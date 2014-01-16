// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef _IceCfg_h
#define _IceCfg_h

#include "IceDefs.h"
#include "IceTypes.h"

class IceCfg {
public:
  IceCfg(void);
  ~IceCfg();
  void setName(const IceString &FunctionName) { Name = FunctionName; }
  void setReturnType(IceType ReturnType) { Type = ReturnType; }
  void makeTarget(IceTargetArch Arch);
  IceTargetLowering *getTarget(void) const { return Target; }
  void addArg(IceVariable *Arg);
  void setEntryNode(IceCfgNode *EntryNode);
  void addNode(IceCfgNode *Node, uint32_t LabelIndex);
  IceCfgNode *splitEdge(uint32_t FromNodeIndex, uint32_t ToNodeIndex);
  IceCfgNode *getNode(uint32_t LabelIndex) const;
  // getConstant() is not const because it might add something to the
  // constant pool.
  IceConstant *getConstant(IceType Type, const void *ConstantBits);
  IceConstant *getConstant(IceType Type, int32_t ConstantInt32);
  // Look up a variable used as an rvalue.  The variable might not yet
  // have a definition if its definition is in a predecessor block
  // that hasn't yet been processed, e.g. via a phi for a loopback
  // edge.
  IceVariable *getVariable(IceType Type, uint32_t Index);
  IceVariable *getVariable(IceType Type, const IceString &Name) {
    return getVariable(Type, translateVariable(Name));
  }
  unsigned getNumVariables(void) const { return Variables.size(); }
  int newInstNumber(void);
  int getNewInstNumber(int OldNumber);

  uint32_t translateVariable(const IceString &Name);
  uint32_t translateLabel(const IceString &Name);
  IceString variableName(uint32_t VariableIndex) const;
  IceString labelName(uint32_t LabelIndex) const;
  IceString physicalRegName(int Reg) const { return RegisterNames[Reg]; }
  void translate(void);
  void dump(void) const;
  void markLastUse(IceOperand *Operand, const IceInst *Inst);
  bool isLastUse(const IceInst *Inst, IceOperand *Operand) const;

  mutable IceOstream Str;
private:
  IceString Name; // function name
  IceType Type; // return type
  IceTargetLowering *Target;
  IceCfgNode *Entry; // entry basic block
  // Difference between Nodes and LNodes.  Nodes is the master list;
  // IceCfgNode::NameIndex is a permanent index into Nodes[]; some
  // entries of Nodes may be NULL; Nodes is ideally a vector
  // container.  LNodes is the linearization; does not contain NULL
  // entries; is a permutation of the non-NULL Nodes entries; is
  // ideally a list container.
  IceNodeList Nodes; // node list
  IceNodeList LNodes; // linearized node list; Entry should be first
  IceVarList Variables;
  std::vector<const IceInst *> LastUses; // instruction ending each variable's live range
  IceVarList Args; // densely packed vector, subset of Variables
  std::vector<int> InstNumberRemapping;

  class NameTranslation *VariableTranslation;
  class NameTranslation *LabelTranslation;
  int NextInstNumber;
  // TODO: This goes away when we get target-specific operands with
  // their own dump() methods.
  IceString *RegisterNames;
  // arena allocator for the function
  // list of exit IceCfgNode* for liveness analysis
  // operand pool - set of IceOperand
  // whether there is an alloca (frame ptr optimization)
  void renumberInstructions(void);
  void registerInEdges(void);
  void findAddressOpt(void);
  void markLastUses(void);
  void placePhiLoads(void);
  void placePhiStores(void);
  void deletePhis(void);
  void genCode(void);
  void simpleDCE(void);
  void multiblockRegAlloc(void);
  void multiblockCompensation(void);
  void liveness(void);
};

#endif // _IceCfg_h
