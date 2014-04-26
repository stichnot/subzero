//===- subzero/src/IceCfg.cpp - Control flow graph implementation ---------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Cfg class, which manages individual
// functions.
//
//===----------------------------------------------------------------------===//

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceDefs.h"
#include "IceInst.h"
#include "IceLiveness.h"
#include "IceOperand.h"
#include "IceTargetLowering.h"

namespace Ice {

Ostream *GlobalStr = NULL;
bool Cfg::HasEmittedFirstMethod = false;

Cfg::Cfg(GlobalContext *Ctx)
    : Ctx(Ctx), FunctionName(""), ReturnType(IceType_void),
      IsInternalLinkage(false), HasError(false), ErrorMessage(""), Entry(NULL),
      NextInstNumber(1), Live(NULL),
      Target(TargetLowering::createLowering(Ctx->getTargetArch(), this)),
      CurrentNode(NULL) {
  GlobalStr = &Ctx->getStrDump();
}

Cfg::~Cfg() {}

void Cfg::setError(const IceString &Message) {
  HasError = true;
  ErrorMessage = Message;
  Ctx->getStrDump() << "ICE translation error: " << ErrorMessage << "\n";
}

CfgNode *Cfg::makeNode(const IceString &Name) {
  SizeT LabelIndex = Nodes.size();
  CfgNode *Node = CfgNode::create(this, LabelIndex, Name);
  Nodes.push_back(Node);
  return Node;
}

CfgNode *Cfg::splitEdge(CfgNode *From, CfgNode *To) {
  // Create the new node.
  IceString NewNodeName = "s__" + From->getName() + "__" + To->getName();
  CfgNode *NewNode = makeNode(NewNodeName);

  // Decide where "this" should go in the linearization.  The two
  // obvious choices are right after the From node, and right before
  // the To node.  For now, let's do the latter.
  assert(NewNode == Nodes.back());
  Nodes.pop_back();
  for (NodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    CfgNode *Node = *I;
    if (Node == To) {
      Nodes.insert(I, NewNode);
      break;
    }
  }

  // Update edges.
  NewNode->splitEdge(From, To);
  return NewNode;
}

// Create a new Variable with a particular type and an optional
// name.  The Node argument is the node where the variable is defined.
Variable *Cfg::makeVariable(Type Ty, const CfgNode *Node,
                            const IceString &Name) {
  SizeT Index = Variables.size();
  Variables.push_back(Variable::create(this, Ty, Node, Index, Name));
  return Variables[Index];
}

void Cfg::addArg(Variable *Arg) {
  Arg->setIsArg(this);
  Args.push_back(Arg);
}

// Returns whether the stack frame layout has been computed yet.  This
// is used for dumping the stack frame location of Variables.
bool Cfg::hasComputedFrame() const {
  return getTarget() && getTarget()->hasComputedFrame();
}

void Cfg::translate() {
  Ostream &Str = Ctx->getStrDump();
  if (hasError())
    return;

  if (Ctx->isVerbose())
    Str << "================ Initial CFG ================\n";
  dump();

  IceTimer T_translate;
  // The set of translation passes and their order are determined by
  // the target.
  getTarget()->translate();
  T_translate.printElapsedUs(getContext(), "translate()");

  if (Ctx->isVerbose())
    Str << "================ Final output ================\n";
  dump();
}

void Cfg::computePredecessors() {
  for (NodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    (*I)->computePredecessors();
  }
}

void Cfg::renumberInstructions() {
  NextInstNumber = 1;
  for (NodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    (*I)->renumberInstructions();
  }
}

// placePhiLoads() must be called before placePhiStores().
void Cfg::placePhiLoads() {
  for (NodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    (*I)->placePhiLoads();
  }
}

// placePhiStores() must be called after placePhiLoads().
void Cfg::placePhiStores() {
  for (NodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    (*I)->placePhiStores();
  }
}

void Cfg::deletePhis() {
  for (NodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    (*I)->deletePhis();
  }
}

void Cfg::doAddressOpt() {
  for (NodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    (*I)->doAddressOpt();
  }
}

void Cfg::genCode() {
  if (getTarget() == NULL) {
    setError("Cfg::makeTarget() wasn't called.");
    return;
  }
  for (NodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    (*I)->genCode();
  }
}

// Compute the stack frame layout.
void Cfg::genFrame() {
  getTarget()->addProlog(Entry);
  // TODO: Consider folding epilog generation into the final
  // emission/assembly pass to avoid an extra iteration over the node
  // list.  Or keep a separate list of exit nodes.
  for (NodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    CfgNode *Node = *I;
    if (Node->getHasReturn())
      getTarget()->addEpilog(Node);
  }
}

void Cfg::liveness(LivenessMode Mode) {
  if (Mode == Liveness_LREndLightweight) {
    // Lightweight liveness is a quick single pass and doesn't need to
    // iterate until convergence.
    for (NodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
      (*I)->liveness(Mode, getLiveness());
    }
    return;
  }

  Live.reset(new Liveness(this, Mode));
  Live->init();
  llvm::BitVector NeedToProcess(Nodes.size());
  // Mark all nodes as needing to be processed.
  for (NodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    NeedToProcess[(*I)->getIndex()] = true;
  }
  while (NeedToProcess.any()) {
    // Iterate in reverse topological order to speed up convergence.
    for (NodeList::reverse_iterator I = Nodes.rbegin(), E = Nodes.rend();
         I != E; ++I) {
      CfgNode *Node = *I;
      if (NeedToProcess[Node->getIndex()]) {
        NeedToProcess[Node->getIndex()] = false;
        bool Changed = Node->liveness(Mode, getLiveness());
        if (Changed) {
          // If the beginning-of-block liveness changed since the last
          // iteration, mark all in-edges as needing to be processed.
          const NodeList &InEdges = Node->getInEdges();
          for (NodeList::const_iterator I1 = InEdges.begin(),
                                        E1 = InEdges.end();
               I1 != E1; ++I1) {
            CfgNode *Pred = *I1;
            NeedToProcess[Pred->getIndex()] = true;
          }
        }
      }
    }
  }
  if (Mode == Liveness_RangesFull) {
    // Reset each variable's live range.
    for (VarList::const_iterator I = Variables.begin(), E = Variables.end();
         I != E; ++I) {
      if (Variable *Var = *I)
        Var->resetLiveRange();
    }
  }
  IceTimer T_liveRange;
  // Make a final pass over instructions to delete dead instructions
  // and build each Variable's live range.
  for (NodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    (*I)->livenessPostprocess(Mode, getLiveness());
  }
  if (Mode == Liveness_RangesFull) {
    // Special treatment for live in-args.  Their liveness needs to
    // extend beyond the beginning of the function, otherwise an arg
    // whose only use is in the first instruction will end up having
    // the trivial live range [1,1) and will *not* interfere with
    // other arguments.  So if the first instruction of the method is
    // "r=arg1+arg2", both args may be assigned the same register.
    for (SizeT I = 0; I < Args.size(); ++I) {
      Variable *Arg = Args[I];
      if (!Live->getLiveRange(Arg).isEmpty()) {
        // Add live range [-1,0) with weight 0.
        Live->addLiveRange(Arg, -1, 0, 0);
      }
      Variable *Lo = Arg->getLo();
      if (Lo && !Live->getLiveRange(Lo).isEmpty())
        Live->addLiveRange(Lo, -1, 0, 0);
      Variable *Hi = Arg->getHi();
      if (Hi && !Live->getLiveRange(Hi).isEmpty())
        Live->addLiveRange(Hi, -1, 0, 0);
    }
    // Copy Liveness::LiveRanges into individual variables.  TODO:
    // Remove Variable::LiveRange and redirect to
    // Liveness::LiveRanges.  TODO: make sure Variable weights
    // are applied properly.
    SizeT NumVars = Variables.size();
    for (SizeT i = 0; i < NumVars; ++i) {
      Variable *Var = Variables[i];
      Var->setLiveRange(Live->getLiveRange(Var));
      if (Var->getWeight().isInf())
        Var->setLiveRangeInfiniteWeight();
      setCurrentNode(NULL);
    }
    T_liveRange.printElapsedUs(getContext(), "live range construction");
    dump();
    // TODO: validateLiveness() is a heavyweight operation inside an
    // assert().  In a Release build with asserts enabled, we may want
    // to disable this call.
    assert(validateLiveness());
  }
}

// Traverse every Variable of every Inst and verify that it
// appears within the Variable's computed live range.
bool Cfg::validateLiveness() const {
  bool Valid = true;
  for (NodeList::const_iterator I1 = Nodes.begin(), E1 = Nodes.end(); I1 != E1;
       ++I1) {
    CfgNode *Node = *I1;
    InstList &Insts = Node->getInsts();
    for (InstList::const_iterator I2 = Insts.begin(), E2 = Insts.end();
         I2 != E2; ++I2) {
      Inst *Inst = *I2;
      if (Inst->isDeleted())
        continue;
      if (llvm::isa<InstFakeKill>(Inst))
        continue;
      int32_t InstNumber = Inst->getNumber();
      Variable *Dest = Inst->getDest();
      if (Dest) {
        // TODO: This instruction should actually begin Dest's live
        // range, so we could probably test that this instruction is
        // the beginning of some segment of Dest's live range.  But
        // this wouldn't work with non-SSA temporaries during
        // lowering.
        if (!Dest->getLiveRange().containsValue(InstNumber)) {
          Valid = false;
          assert(Valid);
        }
      }
      SizeT VarIndex = 0;
      for (SizeT I = 0; I < Inst->getSrcSize(); ++I) {
        Operand *Src = Inst->getSrc(I);
        SizeT NumVars = Src->getNumVars();
        for (SizeT J = 0; J < NumVars; ++J, ++VarIndex) {
          const Variable *Var = Src->getVar(J);
          if (!Var->getLiveRange().containsValue(InstNumber)) {
            Valid = false;
            assert(Valid);
          }
        }
      }
    }
  }
  return Valid;
}

// ======================== Dump routines ======================== //

void Cfg::emit(uint32_t Option) {
  Ostream &Str = Ctx->getStrEmit();
  IceTimer T_emit;
  if (!HasEmittedFirstMethod) {
    HasEmittedFirstMethod = true;
    // Print a helpful command for assembling the output.
    Str << "# $LLVM_BIN_PATH/llvm-mc"
        << " -arch=x86"
        << " -x86-asm-syntax=intel"
        << " -filetype=obj"
        << " -o=MyObj.o"
        << "\n\n";
  }
  // TODO: have the Target emit the header?
  // TODO: need a per-file emit in addition to per-CFG
  // TODO: emit to a specified file
  Str << "\t.text\n";
  if (!getInternal()) {
    Str << "\t.globl\t" << getContext()->mangleName(getFunctionName()) << "\n";
    Str << "\t.type\t" << getContext()->mangleName(getFunctionName())
        << ",@function\n";
  }
  for (NodeList::const_iterator I = Nodes.begin(), E = Nodes.end(); I != E;
       ++I) {
    (*I)->emit(this, Option);
  }
  Str << "\n";
  // TODO: have the Target emit a footer?
  T_emit.printElapsedUs(Ctx, "emit()");
}

void Cfg::dump() {
  Ostream &Str = Ctx->getStrDump();
  setCurrentNode(getEntryNode());
  // Print function name+args
  if (getContext()->isVerbose(IceV_Instructions)) {
    Str << "define ";
    if (getInternal())
      Str << "internal ";
    Str << ReturnType << " @" << getFunctionName() << "(";
    for (SizeT i = 0; i < Args.size(); ++i) {
      if (i > 0)
        Str << ", ";
      Str << Args[i]->getType() << " ";
      Args[i]->dump(this);
    }
    Str << ") {\n";
  }
  setCurrentNode(NULL);
  if (getContext()->isVerbose(IceV_Liveness)) {
    // Print summary info about variables
    for (VarList::const_iterator I = Variables.begin(), E = Variables.end();
         I != E; ++I) {
      Variable *Var = *I;
      Str << "//"
          << " multiblock=" << Var->isMultiblockLife() << " "
          << " weight=" << Var->getWeight() << " ";
      Var->dump(this);
      Str << " LIVE=" << Var->getLiveRange() << "\n";
    }
  }
  // Print each basic block
  for (NodeList::const_iterator I = Nodes.begin(), E = Nodes.end(); I != E;
       ++I) {
    (*I)->dump(this);
  }
  if (getContext()->isVerbose(IceV_Instructions)) {
    Str << "}\n";
  }
}

} // end of namespace Ice
