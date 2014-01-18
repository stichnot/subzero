/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include <stdio.h> // TODO: only for debugging sprintf

#include <iostream>

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceDefs.h"
#include "IceInst.h"
#include "IceOperand.h"
#include "IceTargetLowering.h"

IceOstream *GlobalStr;

// Provides a mapping between strings and small unsigned integers.
// The small integers can be used for vector indices.
class NameTranslation {
public:
  typedef std::vector<IceString> Container;
  NameTranslation(void) {}
  uint32_t translate(const IceString &Name) {
    for (Container::size_type i = 0; i < Strings.size(); ++i) {
      if (Strings[i] == Name)
        return i;
    }
    Strings.push_back(""); // TODO: gaps added for testing; remove
    uint32_t NewIndex = Strings.size();
    Strings.push_back(Name);
    return NewIndex;
  }
  IceString getName(Container::size_type Index) const {
    if (Index >= Strings.size())
      return "<overflow>";
#if 0
    char buf[20]; // TODO: remove debugging suffix
    sprintf(buf, "_%d", (int)Index); // TODO: use proper cast
    return Strings[Index] + buf;
#else
    return Strings[Index];
#endif
  }
private:
  Container Strings;
};


IceCfg::IceCfg(void) : Str(std::cout, this),
                       Type(IceType_void), Target(NULL), Entry(NULL),
                       VariableTranslation(new NameTranslation),
                       LabelTranslation(new NameTranslation),
                       NextInstNumber(1) {
  GlobalStr = &Str;
}

IceCfg::~IceCfg() {
  delete VariableTranslation;
  delete LabelTranslation;
  // TODO: All ICE data destructors should have proper destructors.
  // However, be careful with delete statements since we'll likely be
  // using arena-based allocation.
}

void IceCfg::makeTarget(IceTargetArch Arch) {
  Target = IceTargetLowering::createLowering(Arch, this);
}

void IceCfg::addArg(IceVariable *Arg) {
  Arg->setIsArg();
  Args.push_back(Arg);
}

void IceCfg::setEntryNode(IceCfgNode *EntryNode) {
  Entry = EntryNode;
}

// We assume that the initial CFG construction calls addNode() in the
// desired topological/linearization order.
void IceCfg::addNode(IceCfgNode *Node, uint32_t LabelIndex) {
  if (Nodes.size() <= LabelIndex)
    Nodes.resize(LabelIndex + 1);
  assert(Nodes[LabelIndex] == NULL);
  Nodes[LabelIndex] = Node;
  LNodes.push_back(Node);
}

IceCfgNode *IceCfg::splitEdge(uint32_t FromNodeIndex, uint32_t ToNodeIndex) {
  IceCfgNode *From = getNode(FromNodeIndex);
  IceCfgNode *To = getNode(ToNodeIndex);
  // Create the new node.
  IceString NewNodeName =
    "s__" + labelName(FromNodeIndex) + "__" + labelName(ToNodeIndex);
  uint32_t NewNodeIndex = translateLabel(NewNodeName);
  IceCfgNode *NewNode = new IceCfgNode(this, NewNodeIndex);
  // TODO: It's ugly that LNodes has to be manipulated this way.
  assert(NewNode == LNodes.back());
  LNodes.pop_back();

  // Decide where "this" should go in the linearization.  The two
  // obvious choices are right after the From node, and right before
  // the To node.  For now, let's do the latter.
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end();
       I != E; ++I) {
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

IceConstant *IceCfg::getConstant(IceType Type, int32_t ConstantInt32) {
  return new IceConstant(ConstantInt32);
}

IceVariable *IceCfg::getVariable(uint32_t Index) const {
  assert(Variables.size() > Index);
  return Variables[Index];
}

IceVariable *IceCfg::getVariable(IceType Type, uint32_t Index) {
  if (Variables.size() <= Index)
    Variables.resize(Index + 1);
  if (Variables[Index] == NULL)
    Variables[Index] = new IceVariable(Type, Index);
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

uint32_t IceCfg::translateVariable(const IceString &Name) {
  return VariableTranslation->translate(Name);
}

uint32_t IceCfg::translateLabel(const IceString &Name) {
  return LabelTranslation->translate(Name);
}

IceString IceCfg::variableName(uint32_t VariableIndex) const {
  return VariableTranslation->getName(VariableIndex);
}

IceString IceCfg::labelName(uint32_t LabelIndex) const {
  return LabelTranslation->getName(LabelIndex);
}

void IceCfg::renumberInstructions(void)
{
  InstNumberRemapping.resize(NextInstNumber);
  NextInstNumber = 0;
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end();
       I != E; ++I) {
    (*I)->renumberInstructions();
  }
  // TODO: Update live ranges and any other data structures that rely
  // on the instruction number, before clearing the remap table.
  InstNumberRemapping.clear();
}

void IceCfg::registerInEdges(void) {
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end();
       I != E; ++I) {
    (*I)->registerInEdges();
  }
}

void IceCfg::findAddressOpt(void) {
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end();
       I != E; ++I) {
    (*I)->findAddressOpt();
  }
}

void IceCfg::placePhiLoads(void) {
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end();
       I != E; ++I) {
    (*I)->placePhiLoads();
  }
}

void IceCfg::placePhiStores(void) {
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end();
       I != E; ++I) {
    (*I)->placePhiStores();
  }
}

void IceCfg::deletePhis(void) {
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end();
       I != E; ++I) {
    (*I)->deletePhis();
  }
}

void IceCfg::genCode(void) {
  assert(Target && "IceCfg::makeTarget() wasn't called.");
  RegisterNames = Target->getRegNames();
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end();
       I != E; ++I) {
    (*I)->genCode();
  }
}

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

void IceCfg::multiblockRegAlloc(void) {
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end();
       I != E; ++I) {
    (*I)->multiblockRegAlloc();
  }
}

void IceCfg::multiblockCompensation(void) {
  // TODO: Make sure the Node iterator plays nicely with the
  // possibility of adding new blocks from edge splitting.
  for (unsigned i = 0, e = Nodes.size(); i < e; ++i) {
    IceCfgNode *Node = Nodes[i];
    if (Node) {
      Node->multiblockCompensation();
    }
  }
}

void IceCfg::liveness(IceLiveness Mode) {
  if (Mode == IceLiveness_LREndLightweight) {
    for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end();
         I != E; ++I) {
      (*I)->liveness(Mode, true);
    }
    return;
  }

  llvm::BitVector NeedToProcess(Nodes.size());
  // Mark all nodes as needing to be processed
  for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end();
       I != E; ++I) {
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
          const IceEdgeList &InEdges = Node->getInEdges();
          for (IceEdgeList::const_iterator I1 = InEdges.begin(),
                 E1 = InEdges.end(); I1 != E1; ++I1) {
            IceCfgNode *Pred = getNode(*I1);
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
    for (IceNodeList::iterator I = LNodes.begin(), E = LNodes.end();
         I != E; ++I) {
      (*I)->livenessPostprocess(Mode);
    }
  }
}

void IceCfg::translate(void) {
  registerInEdges();

  Str << "================ Initial CFG ================\n";
  dump();

  liveness(IceLiveness_LREndLightweight);
  Str << "================ Liveness test 1 ================\n";
  dump();

  liveness(IceLiveness_LREndFull);
  Str << "================ Liveness test 2 ================\n";
  dump();

  liveness(IceLiveness_RangesFull);
  Str << "================ Liveness test 3 ================\n";
  dump();

  findAddressOpt();
  liveness(IceLiveness_LREndLightweight);
  Str << "================ After x86 address opt ================\n";
  dump();

  placePhiLoads();
  placePhiStores();
  deletePhis();
  Str << "================ After Phi lowering ================\n";
  dump();

  renumberInstructions();
  Str << "================ After instruction renumbering ================\n";
  dump();

  genCode();
  renumberInstructions();
  Str << "================ After initial x8632 codegen ================\n";
  dump();

  simpleDCE();
  Str << "================ After simple DCE ================\n";
  dump();

  multiblockRegAlloc();
  multiblockCompensation();
  simpleDCE();
  Str << "================ After multi-block regalloc ================\n";
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
          << " multiblock=" << Var->isMultiblockLife()
          << " " << Var
          << " LIVE=" << Var->getLiveRange()
          << "\n";
    }
  }
  // Print each basic block
  for (IceNodeList::const_iterator I = LNodes.begin(), E = LNodes.end();
       I != E; ++I) {
    (*I)->dump(Str);
  }
  if (Str.isVerbose(IceV_Instructions)) {
    Str << "}\n";
  }
}
