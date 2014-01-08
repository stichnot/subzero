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
                       Type(IceType_void), Entry(NULL),
                       VariableTranslation(new NameTranslation),
                       LabelTranslation(new NameTranslation) {
  GlobalStr = &Str;
}

IceCfg::~IceCfg() {
  delete VariableTranslation;
  delete LabelTranslation;
  // TODO: All ICE data destructors should have proper destructors.
  // However, be careful with delete statements since we'll likely be
  // using arena-based allocation.
}

void IceCfg::setName(const IceString &FunctionName) {
  Name = FunctionName;
}

void IceCfg::setReturnType(IceType ReturnType) {
  Type = ReturnType;
}

void IceCfg::addArg(IceVariable *Arg) {
  Arg->setIsArg();
  Args.push_back(Arg);
}

void IceCfg::setEntryNode(IceCfgNode *EntryNode) {
  Entry = EntryNode;
}

void IceCfg::addNode(IceCfgNode *Node, uint32_t LabelIndex) {
  if (Nodes.size() <= LabelIndex)
    Nodes.resize(LabelIndex + 1);
  assert(Nodes[LabelIndex] == NULL);
  Nodes[LabelIndex] = Node;
}

IceCfgNode *IceCfg::getNode(uint32_t LabelIndex) const {
  return Nodes[LabelIndex];
}

IceConstant *IceCfg::getConstant(IceType Type, int32_t ConstantInt32) {
  return new IceConstant(ConstantInt32);
}

IceVariable *IceCfg::getVariable(IceType Type, uint32_t Index) {
  if (Variables.size() <= Index)
    Variables.resize(Index + 1);
  if (Variables[Index] == NULL)
    Variables[Index] = new IceVariable(Type, Index);
  return Variables[Index];
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

void IceCfg::registerInEdges(void) {
  for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end();
       I != E; ++I) {
    IceCfgNode *Node = *I;
    if (Node) {
      Node->registerInEdges(this);
    }
  }
}

void IceCfg::findAddressOpt(void) {
  for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end();
       I != E; ++I) {
    IceCfgNode *Node = *I;
    if (Node) {
      Node->findAddressOpt(this);
    }
  }
}

void IceCfg::markLastUses(void) {
  LastUses.clear();
  LastUses.resize(Variables.size(), NULL);
  for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end();
       I != E; ++I) {
    IceCfgNode *Node = *I;
    if (Node) {
      Node->markLastUses(this);
    }
  }
}

void IceCfg::markLastUse(IceOperand *Operand, const IceInst *Inst) {
  IceVariable *Variable = llvm::dyn_cast<IceVariable>(Operand);
  if (Variable == NULL)
    return;
  if (Variable->isMultiblockLife())
    return;
  uint32_t Index = Variable->getIndex();
  if (false && LastUses[Index])
    return;
  LastUses[Index] = Inst;
}

void IceCfg::placePhiLoads(void) {
  for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end();
       I != E; ++I) {
    IceCfgNode *Node = *I;
    if (Node) {
      Node->placePhiLoads(this);
    }
  }
}

void IceCfg::placePhiStores(void) {
  for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end();
       I != E; ++I) {
    IceCfgNode *Node = *I;
    if (Node) {
      Node->placePhiStores(this);
    }
  }
}

void IceCfg::deletePhis(void) {
  for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end();
       I != E; ++I) {
    IceCfgNode *Node = *I;
    if (Node) {
      Node->deletePhis(this);
    }
  }
}

void IceCfg::genCodeX8632(void) {
  static const char *RegNames[] = {
    "eax",
    "ecx",
    "edx",
  };
  RegisterNames = RegNames;
  for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end();
       I != E; ++I) {
    IceCfgNode *Node = *I;
    if (Node) {
      Node->genCodeX8632(this);
    }
  }
}

void IceCfg::simpleDCE(void) {
  for (IceVarList::const_iterator I = Variables.begin(), E = Variables.end();
       I != E; ++I) {
    IceVariable *Var = *I;
    if (Var == NULL)
      continue;
    Str << "Var=" << Var << ", UseCount=" << Var->getUseCount() << "\n";
    if (Var->getUseCount())
      continue;
    IceInst *Inst = Var->getDefinition();
    if (Inst && !Inst->isDeleted())
      Inst->setDeleted();
  }
}

void IceCfg::multiblockRegAlloc(void) {
  for (IceNodeList::iterator I = Nodes.begin(), E = Nodes.end();
       I != E; ++I) {
    IceCfgNode *Node = *I;
    if (Node) {
      Node->multiblockRegAlloc(this);
    }
  }
}

void IceCfg::multiblockCompensation(void) {
  // TODO: Make sure the Node iterator plays nicely with the
  // possibility of adding new blocks from edge splitting.
  for (unsigned i = 0, e = Nodes.size(); i < e; ++i) {
    IceCfgNode *Node = Nodes[i];
    if (Node) {
      Node->multiblockCompensation(this);
    }
  }
}

bool IceCfg::isLastUse(const IceInst *Inst, IceOperand *Operand) const {
  IceVariable *Variable = llvm::dyn_cast<IceVariable>(Operand);
  if (Variable == NULL)
    return false;
  uint32_t Index = Variable->getIndex();
  if (Index >= LastUses.size())
    return false;
  // TODO: We can mark multiple last-use instructions for an operand.
  // E.g., "x=...; if (cond) a=x; else b=x;" has 2 last-uses of x.
  return (Inst == LastUses[Index]);
}

void IceCfg::translate(void) {
  registerInEdges();

  Str << "================ Initial CFG ================\n";
  dump();

  findAddressOpt();
  markLastUses();
  Str << "================ After x86 address opt ================\n";
  dump();

  placePhiLoads();
  placePhiStores();
  deletePhis();
  Str << "================ After Phi load placement ================\n";
  dump();

  genCodeX8632();
  Str << "================ After initial x8632 codegen ================\n";
  dump();

  simpleDCE();
  Str << "================ After simple DCE ================\n";
  dump();

  multiblockRegAlloc();
  multiblockCompensation();
  Str << "================ After multi-block regalloc ================\n";
  dump();

  Str.setVerbose(false);
  Str << "================ Final output ================\n";
  dump();
}

// ======================== Dump routines ======================== //

void IceCfg::dump(void) const {
  // Print function name+args
  Str << "define internal " << Type << " " << Name << "(";
  for (unsigned i = 0; i < Args.size(); ++i) {
    if (i > 0)
      Str << ", ";
    Str << Args[i]->getType() << " " << Args[i];
  }
  Str << ") {\n";
  if (Str.isVerbose()) {
    // Print summary info about variables
    for (IceVarList::const_iterator I = Variables.begin(), E = Variables.end();
         I != E; ++I) {
      IceVariable *Var = *I;
      if (!Var)
        continue;
      Str << "//"
          << " uses=" << Var->getUseCount()
          << " multiblock=" << Var->isMultiblockLife()
          << " " << Var << "\n";
    }
  }
  // Print each basic block
  for (IceNodeList::const_iterator I = Nodes.begin(), E = Nodes.end();
       I != E; ++I) {
    IceCfgNode *Node = *I;
    if (Node) {
      Node->dump(Str);
    }
  }
  Str << "}\n";
}
