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

class IceConstantPool {
public:
  IceConstantPool(IceCfg *Cfg) : Cfg(Cfg) {}
  IceConstantRelocatable *getOrAddRelocatable(IceType Type, const void *Handle,
                                              int64_t Offset,
                                              const IceString &Name) {
    uint32_t Index = NameToIndex.translate(
        KeyType(Type, std::pair<IceString, int64_t>(Name, Offset)));
    if (Index >= RelocatablePool.size()) {
      RelocatablePool.resize(Index + 1);
      void *Handle = NULL;
      RelocatablePool[Index] = IceConstantRelocatable::create(
          Cfg, Index, Type, Handle, Offset, Name);
    }
    IceConstantRelocatable *Constant = RelocatablePool[Index];
    assert(Constant);
    return Constant;
  }
  uint32_t getSize() const { return RelocatablePool.size(); }
  IceConstantRelocatable *getEntry(uint32_t Index) const {
    assert(Index < RelocatablePool.size());
    return RelocatablePool[Index];
  }

private:
  // KeyType is a triple of {Type, Name, Offset}.
  typedef std::pair<IceType, std::pair<IceString, int64_t> > KeyType;
  // TODO: Cfg is being captured primarily for arena allocation for
  // new IceConstants.  If IceConstants live beyond a function/Cfg,
  // they need to be allocated from a global arena and there needs to
  // be appropriate locking.
  IceCfg *Cfg;
  // Use IceValueTranslation<> to map (Name,Type) pairs to an index.
  IceValueTranslation<KeyType> NameToIndex;
  std::vector<IceConstantRelocatable *> RelocatablePool;
};

IceOstream *GlobalStr = NULL;
bool IceCfg::HasEmittedFirstMethod = false;

IceCfg::IceCfg()
    : Str(this), Name(""), TestPrefix(""), Type(IceType_void),
      IsInternal(false), HasError(false), ErrorMessage(""), Entry(NULL),
      NextInstNumber(1), Target(NULL), Liveness(NULL) {
  GlobalStr = &Str;
  ConstantPool = new IceConstantPool(this);
}

IceCfg::~IceCfg() {
  // TODO: All ICE data destructors should have proper destructors.
  // However, be careful with delete statements since we'll likely be
  // using arena-based allocation.
  delete ConstantPool;
  delete Liveness;
}

// In this context, name mangling means to rewrite a symbol using a
// given prefix.  For a C++ symbol, we'd like to demangle it, prepend
// the prefix to the original symbol, and remangle it for C++.  For
// other symbols, just prepend the prefix.
IceString IceCfg::mangleName(const IceString &Name) const {
  // TODO: This handles only non-nested C++ symbols, and not ones that
  // begin with "_ZN".  For the latter, we need to rewrite only the
  // last name component.
  if (getTestPrefix() == "")
    return Name;
  IceString Default = getTestPrefix() + Name;
  uint32_t BaseLength = 0;
  char Buffer[1 + Name.length()];
  int ItemsParsed = sscanf(Name.c_str(), "_Z%u%s", &BaseLength, Buffer);
  if (ItemsParsed != 2)
    return Default;
  if (strlen(Buffer) < BaseLength)
    return Default;

  BaseLength += getTestPrefix().length();
  char NewNumber[30 + Name.length() + getTestPrefix().length()];
  sprintf(NewNumber, "_Z%u%s%s", BaseLength, getTestPrefix().c_str(), Buffer);
  return NewNumber;
}

void IceCfg::setError(const IceString &Message) {
  HasError = true;
  ErrorMessage = Message;
  Str << "ICE translation error: " << ErrorMessage << "\n";
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

IceConstant *IceCfg::getConstantInt(IceType Type, uint64_t ConstantInt64) {
  return IceConstantInteger::create(this, Type, ConstantInt64);
}

// TODO: Add float and double constants to the global constant pool,
// instead of creating a new instance each time.
IceConstant *IceCfg::getConstantFloat(float ConstantFloat) {
  return IceConstantFloat::create(this, IceType_f32, ConstantFloat);
}

IceConstant *IceCfg::getConstantDouble(double ConstantDouble) {
  return IceConstantDouble::create(this, IceType_f64, ConstantDouble);
}

IceConstant *IceCfg::getConstantSym(IceType Type, const void *Handle,
                                    int64_t Offset, const IceString &Name,
                                    bool SuppressMangling) {
  IceConstantRelocatable *Const =
      ConstantPool->getOrAddRelocatable(Type, Handle, Offset, Name);
  Const->setSuppressMangling(SuppressMangling);
  return Const;
}

// Returns whether the stack frame layout has been computed yet.  This
// is used for dumping the stack frame location of IceVariables.
bool IceCfg::hasComputedFrame() const {
  return getTarget() && getTarget()->hasComputedFrame();
}

void IceCfg::translate(IceTargetArch TargetArch) {
  makeTarget(TargetArch);
  if (hasError())
    return;

  if (Str.isVerbose())
    Str << "================ Initial CFG ================\n";
  dump();

  IceTimer T_translate;
  // The set of translation passes and their order are determined by
  // the target.
  getTarget()->translate();
  T_translate.printElapsedUs(Str, "translate()");

  if (Str.isVerbose())
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
  if (Target == NULL) {
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
  delete Liveness;
  Liveness = NULL;
  if (Mode == IceLiveness_LREndLightweight) {
    // Lightweight liveness is a quick single pass and doesn't need to
    // iterate until convergence.
    for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end(); I != E;
         ++I) {
      (*I)->liveness(Mode, Liveness);
    }
    return;
  }

  Liveness = new IceLiveness(this, Mode);
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
        bool Changed = Node->liveness(Mode, Liveness);
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
    (*I)->livenessPostprocess(Mode, Liveness);
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
      IceVariable *Low = Arg->getLow();
      if (Low && !Liveness->getLiveRange(Low).isEmpty())
        Liveness->addLiveRange(Low, -1, 0, 0);
      IceVariable *High = Arg->getHigh();
      if (High && !Liveness->getLiveRange(High).isEmpty())
        Liveness->addLiveRange(High, -1, 0, 0);
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
      Str.setCurrentNode(NULL);
    }
    T_liveRange.printElapsedUs(Str, "live range construction");
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

void IceCfg::makeTarget(IceTargetArch Arch) {
  Target = IceTargetLowering::createLowering(Arch, this);
}

// ======================== Dump routines ======================== //

void IceCfg::emit(uint32_t Option) const {
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
    Str << "\t.globl\t" << mangleName(Name) << "\n";
    Str << "\t.type\t" << mangleName(Name) << ",@function\n";
  }
  for (IceNodeList::const_iterator I = Nodes.begin(), E = Nodes.end(); I != E;
       ++I) {
    (*I)->emit(Str, Option);
  }
  Str << "\n";
  // TODO: have the Target emit a footer?
  T_emit.printElapsedUs(Str, "emit()");
}

void IceCfg::dump() const {
  Str.setCurrentNode(getEntryNode());
  // Print function name+args
  if (Str.isVerbose(IceV_Instructions)) {
    Str << "define ";
    if (getInternal())
      Str << "internal ";
    Str << Type << " @" << Name << "(";
    for (uint32_t i = 0; i < Args.size(); ++i) {
      if (i > 0)
        Str << ", ";
      Str << Args[i]->getType() << " " << Args[i];
    }
    Str << ") {\n";
  }
  Str.setCurrentNode(NULL);
  if (Str.isVerbose(IceV_Liveness)) {
    // Print summary info about variables
    for (IceVarList::const_iterator I = Variables.begin(), E = Variables.end();
         I != E; ++I) {
      IceVariable *Var = *I;
      if (!Var)
        continue;
      Str << "//"
          << " multiblock=" << Var->isMultiblockLife() << " "
          << " weight=" << Var->getWeight() << " " << Var
          << " LIVE=" << Var->getLiveRange() << "\n";
    }
  }
  // Print each basic block
  for (IceNodeList::const_iterator I = Nodes.begin(), E = Nodes.end(); I != E;
       ++I) {
    (*I)->dump(Str);
  }
  if (Str.isVerbose(IceV_Instructions)) {
    Str << "}\n";
  }
}
