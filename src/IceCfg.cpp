//===- subzero/src/IceCfg.cpp - Control flow graph implementation ---------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the IceCfg class, including constant pool
// management.
//
//===----------------------------------------------------------------------===//

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceDefs.h"
#include "IceInst.h"
#include "IceLiveness.h"
#include "IceOperand.h"
#include "IceTargetLowering.h"

IceOstream *GlobalStr = NULL;
bool IceCfg::HasEmittedFirstMethod = false;

IceCfg::IceCfg(IceGlobalContext *Ctx)
    : Ctx(Ctx), Name(""), Type(IceType_void), IsInternal(false),
      HasError(false), ErrorMessage(""), Entry(NULL), NextInstNumber(1),
      Liveness(NULL),
      Target(IceTargetLowering::createLowering(Ctx->getTargetArch(), this)),
      CurrentNode(NULL) {
  GlobalStr = &Ctx->StrDump;
}

IceCfg::~IceCfg() {}

void IceCfg::setError(const IceString &Message) {
  HasError = true;
  ErrorMessage = Message;
  Ctx->StrDump << "ICE translation error: " << ErrorMessage << "\n";
}

IceCfgNode *IceCfg::makeNode(const IceString &Name) {
  uint32_t LabelIndex = Nodes.size();
  IceCfgNode *Node = IceCfgNode::create(this, LabelIndex, Name);
  Nodes.push_back(Node);
  return Node;
}

IceCfgNode *IceCfg::splitEdge(IceCfgNode *From, IceCfgNode *To) {
  // Create the new node.
  IceString NewNodeName = "s__" + From->getName() + "__" + To->getName();
  IceCfgNode *NewNode = makeNode(NewNodeName);

  // Decide where "this" should go in the linearization.  The two
  // obvious choices are right after the From node, and right before
  // the To node.  For now, let's do the latter.
  assert(NewNode == Nodes.back());
  Nodes.pop_back();
  for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    IceCfgNode *Node = *I;
    if (Node == To) {
      Nodes.insert(I, NewNode);
      break;
    }
  }

  // Update edges.
  NewNode->splitEdge(From, To);
  return NewNode;
}

// Create a new IceVariable with a particular type and an optional
// name.  The Node argument is the node where the variable is defined.
IceVariable *IceCfg::makeVariable(IceType Type, const IceCfgNode *Node,
                                  const IceString &Name) {
  uint32_t Index = Variables.size();
  Variables.push_back(IceVariable::create(this, Type, Node, Index, Name));
  return Variables[Index];
}

void IceCfg::addArg(IceVariable *Arg) {
  Arg->setIsArg(this);
  Args.push_back(Arg);
}

// Returns whether the stack frame layout has been computed yet.  This
// is used for dumping the stack frame location of IceVariables.
bool IceCfg::hasComputedFrame() const {
  return getTarget() && getTarget()->hasComputedFrame();
}

void IceCfg::translate(IceTargetArch TargetArch) {
  IceOstream &Str = Ctx->StrDump;
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

void IceCfg::registerEdges() {
  for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    (*I)->registerEdges();
  }
}

void IceCfg::renumberInstructions() {
  NextInstNumber = 1;
  for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    (*I)->renumberInstructions();
  }
}

// placePhiLoads() must be called before placePhiStores().
void IceCfg::placePhiLoads() {
  for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    (*I)->placePhiLoads();
  }
}

// placePhiStores() must be called after placePhiLoads().
void IceCfg::placePhiStores() {
  for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    (*I)->placePhiStores();
  }
}

void IceCfg::deletePhis() {
  for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    (*I)->deletePhis();
  }
}

void IceCfg::doAddressOpt() {
  for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    (*I)->doAddressOpt();
  }
}

void IceCfg::genCode() {
  if (getTarget() == NULL) {
    setError("IceCfg::makeTarget() wasn't called.");
    return;
  }
  for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    (*I)->genCode();
  }
}

// Compute the stack frame layout.
void IceCfg::genFrame() {
  getTarget()->addProlog(Entry);
  // TODO: Consider folding epilog generation into the final
  // emission/assembly pass to avoid an extra iteration over the node
  // list.  Or keep a separate list of exit nodes.
  for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    IceCfgNode *Node = *I;
    if (Node->getHasReturn())
      getTarget()->addEpilog(Node);
  }
}

void IceCfg::liveness(IceLivenessMode Mode) {
  if (Mode == IceLiveness_LREndLightweight) {
    // Lightweight liveness is a quick single pass and doesn't need to
    // iterate until convergence.
    for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E;
         ++I) {
      (*I)->liveness(Mode, getLiveness());
    }
    return;
  }

  Liveness.reset(new IceLiveness(this, Mode));
  Liveness->init();
  llvm::BitVector NeedToProcess(Nodes.size());
  // Mark all nodes as needing to be processed.
  for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    NeedToProcess[(*I)->getIndex()] = true;
  }
  while (NeedToProcess.any()) {
    // Iterate in reverse topological order to speed up convergence.
    for (IceNodeList::reverse_iterator I = Nodes.rbegin(), E = Nodes.rend();
         I != E; ++I) {
      IceCfgNode *Node = *I;
      if (NeedToProcess[Node->getIndex()]) {
        NeedToProcess[Node->getIndex()] = false;
        bool Changed = Node->liveness(Mode, getLiveness());
        if (Changed) {
          // If the beginning-of-block liveness changed since the last
          // iteration, mark all in-edges as needing to be processed.
          const IceNodeList &InEdges = Node->getInEdges();
          for (IceNodeList::const_iterator I1 = InEdges.begin(),
                                           E1 = InEdges.end();
               I1 != E1; ++I1) {
            IceCfgNode *Pred = *I1;
            NeedToProcess[Pred->getIndex()] = true;
          }
        }
      }
    }
  }
  if (Mode == IceLiveness_RangesFull) {
    // Reset each variable's live range.
    for (IceVarList::const_iterator I = Variables.begin(), E = Variables.end();
         I != E; ++I) {
      if (IceVariable *Var = *I)
        Var->resetLiveRange();
    }
  }
  IceTimer T_liveRange;
  // Make a final pass over instructions to delete dead instructions
  // and build each IceVariable's live range.
  for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E; ++I) {
    (*I)->livenessPostprocess(Mode, getLiveness());
  }
  if (Mode == IceLiveness_RangesFull) {
    // Special treatment for live in-args.  Their liveness needs to
    // extend beyond the beginning of the function, otherwise an arg
    // whose only use is in the first instruction will end up having
    // the trivial live range [1,1) and will *not* interfere with
    // other arguments.  So if the first instruction of the method is
    // "r=arg1+arg2", both args may be assigned the same register.
    for (uint32_t I = 0; I < Args.size(); ++I) {
      IceVariable *Arg = Args[I];
      if (!Liveness->getLiveRange(Arg).isEmpty()) {
        // Add live range [-1,0) with weight 0.
        Liveness->addLiveRange(Arg, -1, 0, 0);
      }
      IceVariable *Lo = Arg->getLo();
      if (Lo && !Liveness->getLiveRange(Lo).isEmpty())
        Liveness->addLiveRange(Lo, -1, 0, 0);
      IceVariable *Hi = Arg->getHi();
      if (Hi && !Liveness->getLiveRange(Hi).isEmpty())
        Liveness->addLiveRange(Hi, -1, 0, 0);
    }
    // Copy IceLiveness::LiveRanges into individual variables.  TODO:
    // Remove IceVariable::LiveRange and redirect to
    // IceLiveness::LiveRanges.  TODO: make sure IceVariable weights
    // are applied properly.
    uint32_t NumVars = Variables.size();
    for (uint32_t i = 0; i < NumVars; ++i) {
      IceVariable *Var = Variables[i];
      Var->setLiveRange(Liveness->getLiveRange(Var));
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

// Traverse every IceVariable of every IceInst and verify that it
// appears within the IceVariable's computed live range.
bool IceCfg::validateLiveness() const {
  bool Valid = true;
  for (IceNodeList::const_iterator I1 = Nodes.begin(), E1 = Nodes.end();
       I1 != E1; ++I1) {
    IceCfgNode *Node = *I1;
    IceInstList &Insts = Node->getInsts();
    for (IceInstList::const_iterator I2 = Insts.begin(), E2 = Insts.end();
         I2 != E2; ++I2) {
      IceInst *Inst = *I2;
      if (Inst->isDeleted())
        continue;
      if (llvm::isa<IceInstFakeKill>(Inst))
        continue;
      int32_t InstNumber = Inst->getNumber();
      IceVariable *Dest = Inst->getDest();
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
      uint32_t VarIndex = 0;
      for (uint32_t I = 0; I < Inst->getSrcSize(); ++I) {
        IceOperand *Src = Inst->getSrc(I);
        uint32_t NumVars = Src->getNumVars();
        for (uint32_t J = 0; J < NumVars; ++J, ++VarIndex) {
          const IceVariable *Var = Src->getVar(J);
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

void IceCfg::emit(uint32_t Option) {
  IceOstream &Str = Ctx->StrEmit;
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
    Str << "\t.globl\t" << getContext()->mangleName(Name) << "\n";
    Str << "\t.type\t" << getContext()->mangleName(Name) << ",@function\n";
  }
  for (IceNodeList::const_iterator I = Nodes.begin(), E = Nodes.end(); I != E;
       ++I) {
    (*I)->emit(this, Option);
  }
  Str << "\n";
  // TODO: have the Target emit a footer?
  T_emit.printElapsedUs(Ctx, "emit()");
}

void IceCfg::dump() {
  IceOstream &Str = Ctx->StrDump;
  setCurrentNode(getEntryNode());
  // Print function name+args
  if (getContext()->isVerbose(IceV_Instructions)) {
    Str << "define ";
    if (getInternal())
      Str << "internal ";
    Str << Type << " @" << Name << "(";
    for (uint32_t i = 0; i < Args.size(); ++i) {
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
    for (IceVarList::const_iterator I = Variables.begin(), E = Variables.end();
         I != E; ++I) {
      IceVariable *Var = *I;
      Str << "//"
          << " multiblock=" << Var->isMultiblockLife() << " "
          << " weight=" << Var->getWeight() << " ";
      Var->dump(this);
      Str << " LIVE=" << Var->getLiveRange() << "\n";
    }
  }
  // Print each basic block
  for (IceNodeList::const_iterator I = Nodes.begin(), E = Nodes.end(); I != E;
       ++I) {
    (*I)->dump(this);
  }
  if (getContext()->isVerbose(IceV_Instructions)) {
    Str << "}\n";
  }
}
