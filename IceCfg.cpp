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

IceOstream *GlobalStr;

IceCfg::IceCfg(void)
    : Str(std::cout, this), HasError(false), ErrorMessage(""),
      Type(IceType_void), Target(NULL), Entry(NULL), NextInstNumber(1) {
  GlobalStr = &Str;
}

IceCfg::~IceCfg() {
  // TODO: All ICE data destructors should have proper destructors.
  // However, be careful with delete statements since we'll likely be
  // using arena-based allocation.
}

void IceCfg::setError(const IceString &Message) {
  HasError = true;
  ErrorMessage = Message;
  if (Str.isVerbose()) {
    Str << "ICE translation error: " << ErrorMessage << "\n";
  }
}

bool IceCfg::hasComputedFrame(void) const {
  return getTarget() && getTarget()->hasComputedFrame();
}

void IceCfg::makeTarget(IceTargetArch Arch) {
  Target = IceTargetLowering::createLowering(Arch, this);
  if (Target)
    RegisterNames = Target->getRegNames();
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
    IceCfgNode *Node = new IceCfgNode(this, LabelIndex, Name);
    Nodes[LabelIndex] = Node;
    LNodes.push_back(Node);
  }
  return Nodes[LabelIndex];
}

IceConstant *IceCfg::getConstant(IceType Type, uint64_t ConstantInt64) {
  return IceConstantInteger::create(Type, ConstantInt64);
}

IceConstant *IceCfg::getConstant(IceType Type, const void *Handle,
                                 const IceString &Name) {
  // TODO: look up from the constant pool.
  return IceConstantRelocatable::create(Type, Handle, Name);
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
    Variables[Index] = IceVariable::create(Type, Index, Name);
  return Variables[Index];
}

int IceCfg::newInstNumber(void) {
  int Result = NextInstNumber;
  NextInstNumber += 1;
  return Result;
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
      (*I)->liveness(Mode, true);
    }
    return;
  }

  llvm::BitVector NeedToProcess(Nodes.size());
  // Mark all nodes as needing to be processed
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end(); I != E;
       ++I) {
    NeedToProcess[(*I)->getIndex()] = true;
  }
  for (bool First = true; NeedToProcess.any(); First = false) {
    // Iterate in reverse topological order to speed up convergence.
    for (IceNodeList::reverse_iterator I = LNodes.rbegin(), E = LNodes.rend();
         I != E; ++I) {
      IceCfgNode *Node = *I;
      if (NeedToProcess[Node->getIndex()]) {
        NeedToProcess[Node->getIndex()] = false;
        bool Changed = Node->liveness(Mode, First);
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

  Str.setVerbose(IceV_Instructions);
  if (Str.isVerbose())
    Str << "================ Final output ================\n";
  dump();
}

// ======================== Dump routines ======================== //

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
