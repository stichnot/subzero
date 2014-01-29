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
    : Str(std::cout, this), Type(IceType_void), Target(NULL), Entry(NULL),
      NextInstNumber(1) {
  GlobalStr = &Str;
}

IceCfg::~IceCfg() {
  // TODO: All ICE data destructors should have proper destructors.
  // However, be careful with delete statements since we'll likely be
  // using arena-based allocation.
}

void IceCfg::makeTarget(IceTargetArch Arch) {
  Target = IceTargetLowering::createLowering(Arch, this);
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

IceConstant *IceCfg::getConstant(IceType Type, int32_t ConstantInt32) {
  return new IceConstant(ConstantInt32);
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
    Variables[Index] = new IceVariable(Type, Index, Name);
  return Variables[Index];
}

int IceCfg::newInstNumber(void) {
  int Result = NextInstNumber;
  NextInstNumber += 1;
  return Result;
}

int IceCfg::getNewInstNumber(int OldNumber) {
  assert((int)InstNumberRemapping.size() > OldNumber);
  int NewNumber = newInstNumber();
  InstNumberRemapping[OldNumber] = NewNumber;
  return NewNumber;
}

void IceCfg::renumberInstructions(void) {
  InstNumberRemapping.resize(NextInstNumber);
  NextInstNumber = 0;
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end(); I != E;
       ++I) {
    (*I)->renumberInstructions();
  }
  // TODO: Update live ranges and any other data structures that rely
  // on the instruction number, before clearing the remap table.
  InstNumberRemapping.clear();
}

void IceCfg::registerEdges(void) {
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end(); I != E;
       ++I) {
    (*I)->registerEdges();
  }
}

void IceCfg::findAddressOpt(void) {
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end(); I != E;
       ++I) {
    (*I)->findAddressOpt();
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
  assert(Target && "IceCfg::makeTarget() wasn't called.");
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end(); I != E;
       ++I) {
    (*I)->genCode();
  }
}

// This is a simple dead code elimination based on reference counting.
// If the use count of a variable is zero, then its defining
// instruction can be deleted.  That instruction's source operand
// reference counts can then be decremented, possibly leading to
// cascading deletes.  This is currently too aggressive and might
// delete instructions with side effects.  There are no callers for
// now, but the code is left for reference.
void IceCfg::simpleDCE(void) {
  for (IceVarList::const_iterator I = Variables.begin(), E = Variables.end();
       I != E; ++I) {
    IceVariable *Var = *I;
    if (Var == NULL)
      continue;
    if (Str.isVerbose(IceV_Liveness)) {
      Str << "Var=" << Var << ", UseCount=" << Var->getUseCount() << "\n";
    }
    if (Var->getUseCount())
      continue;
    IceInst *Inst = Var->getDefinition();
    if (Inst && !Inst->isDeleted())
      Inst->setDeleted();
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
  // TODO: This is just testing for a machine with 8 registers, 3 of
  // which are made available for register allocation.
  IceLinearScan LinearScan(this);
  llvm::SmallBitVector RegMask = getTarget()->getRegisterMask();
  LinearScan.scan(RegMask);
}

// Proposed pass list:
//   findAddressOpt()
//   liveness(IceLiveness_LREndLightweight) to clean up address opt
//     Actually no point running this if we're going to do full liveness
//   liveness(IceLiveness_RangesFull) to prepare for linear-scan
//   regAlloc()
//     Run once for each class of register (i64, i32, f32/64, i1?)
//     Limit to callee-save registers and multi-block lifetime
//       Nothing should be pre-colored yet
//     How to avoid going crazy with callee-save registers?
//     Keep track of which registers actually used
//   genCode()
//     But don't lower phi instructions yet?
//     Use local register manager to make use of addressing modes,
//     commutativity, etc.  Guarantee register assignment of new
//     temporaries by giving eventual live ranges high weight.
//   renumberInstructions()
//   liveness(IceLiveness_RangesFull) to prepare for linear-scan
//   regAlloc()
//     Unleash all caller-saves plus the callee-saves already used
//   At some point, lower phis
void IceCfg::translate(IceTargetArch TargetArch) {
  makeTarget(TargetArch);
  Str << "================ Initial CFG ================\n";
  dump();

  findAddressOpt();
  liveness(IceLiveness_RangesFull);
  Str << "================ After x86 address opt ================\n";
  dump();

  placePhiLoads();
  placePhiStores();
  deletePhis();
  renumberInstructions();
  Str << "================ After Phi lowering ================\n";
  dump();

  genCode();
  renumberInstructions();
  liveness(IceLiveness_RangesFull);
  Str << "================ After initial x8632 codegen ================\n";
  dump();

  regAlloc();
  Str << "================ After linear scan regalloc ================\n";
  dump();

  Str.setVerbose(IceV_Instructions);
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
          << " uses=" << Var->getUseCount()
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
