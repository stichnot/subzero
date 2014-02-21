/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include <iostream> // std::cout

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceDefs.h"
#include "IceInst.h"
#include "IceOperand.h"
#include "IceRegAlloc.h"
#include "IceTargetLowering.h"

class IceConstantPool {
public:
  IceConstantPool(IceCfg *Cfg) : Cfg(Cfg) {}
  IceConstantRelocatable *getOrAddRelocatable(IceType Type, const void *Handle,
                                              int64_t Offset,
                                              const IceString &Name) {
    uint32_t Index = NameToIndex.translate(KeyType(Name, Type));
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
  uint32_t getSize(void) const { return RelocatablePool.size(); }
  IceConstantRelocatable *getEntry(uint32_t Index) const {
    assert(Index < RelocatablePool.size());
    return RelocatablePool[Index];
  }

private:
  typedef std::pair<IceString, IceType> KeyType;
  // TODO: Cfg is being captured primarily for arena allocation for
  // new IceConstants.  If IceConstants live beyond a function/Cfg,
  // they need to be allocated from a global arena and there needs to
  // be appropriate locking.
  IceCfg *Cfg;
  // Use IceValueTranslation<> to map (Name,Type) pairs to an index.
  IceValueTranslation<KeyType> NameToIndex;
  std::vector<IceConstantRelocatable *> RelocatablePool;
};

IceOstream *GlobalStr;

IceCfg::IceCfg(void)
    : Str(std::cout, this), HasError(false), ErrorMessage(""),
      Type(IceType_void), Target(NULL), Entry(NULL), NextInstNumber(1) {
  GlobalStr = &Str;
  ConstantPool = new IceConstantPool(this);
}

IceCfg::~IceCfg() {
  // TODO: All ICE data destructors should have proper destructors.
  // However, be careful with delete statements since we'll likely be
  // using arena-based allocation.
  delete ConstantPool;
}

void IceCfg::setError(const IceString &Message) {
  HasError = true;
  ErrorMessage = Message;
  if (true || Str.isVerbose()) {
    Str << "ICE translation error: " << ErrorMessage << "\n";
  }
}

bool IceCfg::hasComputedFrame(void) const {
  return getTarget() && getTarget()->hasComputedFrame();
}

void IceCfg::makeTarget(IceTargetArch Arch) {
  Target = IceTargetLowering::createLowering(Arch, this);
}

void IceCfg::addArg(IceVariable *Arg) {
  Arg->setIsArg();
  Args.push_back(Arg);
}

void IceCfg::setEntryNode(IceCfgNode *EntryNode) { Entry = EntryNode; }

// We assume that the initial CFG construction calls addNode() in the
// desired topological/linearization order.
void IceCfg::addNode(IceCfgNode *Node, uint32_t LabelIndex) {
  if (Nodes.size() <= LabelIndex)
    Nodes.resize(LabelIndex + 1);
  assert(Nodes[LabelIndex] == NULL);
  Nodes[LabelIndex] = Node;
  LNodes.push_back(Node);
}

IceCfgNode *IceCfg::splitEdge(IceCfgNode *From, IceCfgNode *To) {
  // Create the new node.
  IceString NewNodeName = "s__" + From->getName() + "__" + To->getName();
  IceCfgNode *NewNode = makeNode(-1, NewNodeName);
  // TODO: It's ugly that LNodes has to be manipulated this way.
  assert(NewNode == LNodes.back());
  LNodes.pop_back();

  // Decide where "this" should go in the linearization.  The two
  // obvious choices are right after the From node, and right before
  // the To node.  For now, let's do the latter.
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end(); I != E;
       ++I) {
    IceCfgNode *Node = *I;
    if (Node == To) {
      LNodes.insert(I, NewNode);
      break;
    }
  }

  // Update edges.
  NewNode->splitEdge(From, To);
  return NewNode;
}

IceCfgNode *IceCfg::getNode(uint32_t LabelIndex) const {
  assert(LabelIndex < Nodes.size());
  return Nodes[LabelIndex];
}

IceCfgNode *IceCfg::makeNode(uint32_t LabelIndex, IceString Name) {
  if (LabelIndex == (uint32_t) - 1)
    LabelIndex = Nodes.size();
  if (Nodes.size() <= LabelIndex)
    Nodes.resize(LabelIndex + 1);
  if (Nodes[LabelIndex] == NULL) {
    IceCfgNode *Node = IceCfgNode::create(this, LabelIndex, Name);
    Nodes[LabelIndex] = Node;
    // TODO: This ends up creating LNodes in the order that nodes are
    // resolved, not the compacted order they end up in Nodes.  It
    // would be a good idea to reconstruct LNodes right after initial
    // ICE formation.
    LNodes.push_back(Node);
  }
  return Nodes[LabelIndex];
}

IceConstant *IceCfg::getConstant(IceType Type, uint64_t ConstantInt64) {
  return IceConstantInteger::create(this, Type, ConstantInt64);
}

IceConstant *IceCfg::getConstant(IceType Type, const void *Handle,
                                 int64_t Offset, const IceString &Name) {
  return ConstantPool->getOrAddRelocatable(Type, Handle, Offset, Name);
}

IceVariable *IceCfg::getVariable(uint32_t Index) const {
  assert(Variables.size() > Index);
  assert(Variables[Index]);
  return Variables[Index];
}

IceVariable *IceCfg::makeVariable(IceType Type, uint32_t Index,
                                  const IceString &Name) {
  if (Index == (uint32_t) - 1)
    Index = Variables.size();
  if (Variables.size() <= Index)
    Variables.resize(Index + 1);
  if (Variables[Index] == NULL)
    Variables[Index] = IceVariable::create(this, Type, Index, Name);
  return Variables[Index];
}

int IceCfg::newInstNumber(void) {
  int Result = NextInstNumber;
  NextInstNumber += 1;
  return Result;
}

IceString IceCfg::physicalRegName(int Reg) const {
  assert(getTarget());
  return getTarget()->getRegName(Reg);
}

void IceCfg::renumberInstructions(void) {
  NextInstNumber = 1;
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end(); I != E;
       ++I) {
    (*I)->renumberInstructions();
  }
}

void IceCfg::registerEdges(void) {
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end(); I != E;
       ++I) {
    (*I)->registerEdges();
  }
}

void IceCfg::placePhiLoads(void) {
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end(); I != E;
       ++I) {
    (*I)->placePhiLoads();
  }
}

void IceCfg::placePhiStores(void) {
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end(); I != E;
       ++I) {
    (*I)->placePhiStores();
  }
}

void IceCfg::deletePhis(void) {
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end(); I != E;
       ++I) {
    (*I)->deletePhis();
  }
}

void IceCfg::genCode(void) {
  if (Target == NULL) {
    setError("IceCfg::makeTarget() wasn't called.");
    return;
  }
  // TODO: Query whether the Target supports address mode
  // optimization, and skip the pass if not.
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end(); I != E;
       ++I) {
    (*I)->doAddressOpt();
  }
  // Liveness may be incorrect after address mode optimization.
  renumberInstructions();
  if (hasError())
    return;
  // TODO: It should be sufficient to use the fastest livness
  // calculation, i.e. IceLiveness_LREndLightweight.  However,
  // currently this breaks one test (icmp-simple.ll) because with
  // IceLiveness_LREndFull, the problematic instructions get dead-code
  // eliminated.
  liveness(IceLiveness_LREndFull);
  if (hasError())
    return;
  if (Str.isVerbose())
    Str << "================ After x86 address mode opt ================\n";
  dump();
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end(); I != E;
       ++I) {
    (*I)->genCode();
  }
}

void IceCfg::liveness(IceLiveness Mode) {
  if (Mode == IceLiveness_LREndLightweight) {
    for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end(); I != E;
         ++I) {
      (*I)->liveness(Mode);
    }
    return;
  }

  llvm::BitVector NeedToProcess(Nodes.size());
  // Mark all nodes as needing to be processed
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end(); I != E;
       ++I) {
    NeedToProcess[(*I)->getIndex()] = true;
    (*I)->livenessInit(Mode);
  }
  while (NeedToProcess.any()) {
    // Iterate in reverse topological order to speed up convergence.
    for (IceNodeList::reverse_iterator I = LNodes.rbegin(), E = LNodes.rend();
         I != E; ++I) {
      IceCfgNode *Node = *I;
      if (NeedToProcess[Node->getIndex()]) {
        NeedToProcess[Node->getIndex()] = false;
        bool Changed = Node->liveness(Mode);
        if (Changed) {
          // Mark all in-edges as needing to be processed
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
  if (Mode != IceLiveness_LREndLightweight) {
    // Make a final pass over instructions to delete dead instructions
    // and build each IceVariable's live range.
    for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end(); I != E;
         ++I) {
      (*I)->livenessPostprocess(Mode);
    }
    // Special treatment for live in-args.  Their liveness needs to
    // extend beyond the beginning of the function, otherwise an arg
    // whose only use is in the first instruction will end up having
    // the trivial live range [1,1) and will *not* interfere with
    // other arguments.  So if the first instruction of the method is
    // "r=arg1+arg2", both args may be assigned the same register.
    for (unsigned I = 0; I < Args.size(); ++I) {
      IceVariable *Arg = Args[I];
      if (!Arg->getLiveRange().isEmpty()) {
        // Add live range [-1,0) with weight 0.
        Arg->addLiveRange(-1, 0, 0);
      }
      IceVariable *Low = Arg->getLow();
      if (Low && !Low->getLiveRange().isEmpty())
        Low->addLiveRange(-1, 0, 0);
      IceVariable *High = Arg->getHigh();
      if (High && !High->getLiveRange().isEmpty())
        High->addLiveRange(-1, 0, 0);
    }
  }
}

void IceCfg::regAlloc(void) {
  IceLinearScan LinearScan(this);
  IceTargetLowering::RegSetMask RegInclude = 0, RegExclude = 0;
  RegInclude |= IceTargetLowering::RegMask_CallerSave;
  RegInclude |= IceTargetLowering::RegMask_CalleeSave;
  RegExclude |= IceTargetLowering::RegMask_StackPointer;
  if (getTarget() && getTarget()->hasFramePointer())
    RegExclude |= IceTargetLowering::RegMask_FramePointer;
  llvm::SmallBitVector RegMask =
      getTarget()->getRegisterSet(RegInclude, RegExclude);
  LinearScan.scan(RegMask);
}

// Compute the stack frame layout.
void IceCfg::genFrame(void) {
  getTarget()->addProlog(Entry);
  // TODO: Consider folding epilog generation into the final
  // emission/assembly pass to avoid an extra iteration over the node
  // list.  Or keep a separate list of exit nodes.
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end(); I != E;
       ++I) {
    IceCfgNode *Node = *I;
    if (Node->hasReturn())
      getTarget()->addEpilog(Node);
  }
}

void IceCfg::translate(IceTargetArch TargetArch) {
  makeTarget(TargetArch);
  if (hasError())
    return;

  if (Str.isVerbose())
    Str << "================ Initial CFG ================\n";
  dump();

  placePhiLoads();
  if (hasError())
    return;
  placePhiStores();
  if (hasError())
    return;
  deletePhis();
  if (hasError())
    return;
  renumberInstructions();
  if (hasError())
    return;
  if (Str.isVerbose())
    Str << "================ After Phi lowering ================\n";
  dump();

  genCode();
  if (hasError())
    return;
  renumberInstructions();
  if (hasError())
    return;
  liveness(IceLiveness_RangesFull);
  if (hasError())
    return;
  if (Str.isVerbose())
    Str << "================ After initial x8632 codegen ================\n";
  dump();

  regAlloc();
  if (hasError())
    return;
  if (Str.isVerbose())
    Str << "================ After linear scan regalloc ================\n";
  dump();

  genFrame();
  if (hasError())
    return;
  if (Str.isVerbose())
    Str << "================ After stack frame mapping ================\n";
  dump();

  if (Str.isVerbose())
    Str << "================ Final output ================\n";
  dump();
}

// ======================== Dump routines ======================== //

void IceCfg::emit(uint32_t Option) const {
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
  Str << "\t.globl\t" << Name << "\n";
  Str << "\t.type\t" << Name << ",@function\n";
  uint32_t NumConsts = ConstantPool->getSize();
  for (uint32_t i = 0; i < NumConsts; ++i) {
    IceConstantRelocatable *Const = ConstantPool->getEntry(i);
    if (Const == NULL)
      continue;
    Str << "\t.type\t" << Const->getName() << ",@object\n";
    // TODO: .comm is necessary only when defining vs. declaring?
    uint32_t Width = iceTypeWidth(Const->getType());
    Str << "\t.comm\t" << Const->getName() << "," << Width << "," << Width
        << "\n";
  }
  for (IceNodeList::const_iterator I = LNodes.begin(), E = LNodes.end(); I != E;
       ++I) {
    (*I)->emit(Str, Option);
  }
  Str << "\n";
  // TODO: have the Target emit a footer?
}

void IceCfg::dump(void) const {
  // Print function name+args
  if (Str.isVerbose(IceV_Instructions)) {
    Str << "define internal " << Type << " " << Name << "(";
    for (unsigned i = 0; i < Args.size(); ++i) {
      if (i > 0)
        Str << ", ";
      Str << Args[i]->getType() << " " << Args[i];
    }
    Str << ") {\n";
  }
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
  for (IceNodeList::const_iterator I = LNodes.begin(), E = LNodes.end(); I != E;
       ++I) {
    (*I)->dump(Str);
  }
  if (Str.isVerbose(IceV_Instructions)) {
    Str << "}\n";
  }
}

bool IceCfg::HasEmittedFirstMethod = false;
