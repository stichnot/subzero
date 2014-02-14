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
  bool hasError(void) const { return HasError; }
  IceString getError(void) const { return ErrorMessage; }
  void setError(const IceString &Message);
  bool hasComputedFrame(void) const;
  void setName(const IceString &FunctionName) { Name = FunctionName; }
  IceString getName(void) const { return Name; }
  void setReturnType(IceType ReturnType) { Type = ReturnType; }
  IceTargetLowering *getTarget(void) const { return Target; }
  void addArg(IceVariable *Arg);
  void setEntryNode(IceCfgNode *EntryNode);
  IceCfgNode *getEntryNode(void) const { return Entry; }
  void registerEdges(void);
  void addNode(IceCfgNode *Node, uint32_t LabelIndex);
  IceCfgNode *splitEdge(IceCfgNode *From, IceCfgNode *To);
  IceCfgNode *getNode(uint32_t LabelIndex) const;
  IceCfgNode *makeNode(uint32_t LabelIndex = -1, IceString Name = "");
  // getConstant() is not const because it might add something to the
  // constant pool.
  IceConstant *getConstant(IceType Type, const void *ConstantBits);
  IceConstant *getConstant(IceType Type, uint64_t ConstantInt64);
  // Returns a symbolic constant.  For now, Handle would refer to
  // something LLVM-specific to facilitate linking.
  IceConstant *getConstant(IceType Type, const void *Handle,
                           const IceString &Name = "");
  IceVariable *getVariable(uint32_t Index) const;
  IceVariable *makeVariable(IceType Type, uint32_t Index = -1,
                            const IceString &Name = "");
  const IceVarList &getVariables(void) const { return Variables; }
  const IceVarList &getArgs(void) const { return Args; }
  unsigned getNumVariables(void) const { return Variables.size(); }
  int newInstNumber(void);

  IceString physicalRegName(int Reg) const { return RegisterNames[Reg]; }
  void translate(IceTargetArch TargetArch = IceTarget_X8632);
  void emit(uint32_t Option) const;
  void dump(void) const;

  mutable IceOstream Str;

private:
  bool HasError;
  IceString ErrorMessage;
  IceString Name; // function name
  IceType Type;   // return type
  IceTargetLowering *Target;
  IceCfgNode *Entry; // entry basic block
  // Difference between Nodes and LNodes.  Nodes is the master list;
  // IceCfgNode::NameIndex is a permanent index into Nodes[]; some
  // entries of Nodes may be NULL; Nodes is ideally a vector
  // container.  LNodes is the linearization; does not contain NULL
  // entries; is a permutation of the non-NULL Nodes entries; is
  // ideally a list container.
  IceNodeList Nodes;  // node list
  IceNodeList LNodes; // linearized node list; Entry should be first
  IceVarList Variables;
  IceVarList Args; // densely packed vector, subset of Variables
  class IceConstantPool *ConstantPool;

  int NextInstNumber;
  // TODO: This goes away when we get target-specific operands with
  // their own dump() methods.
  IceString *RegisterNames;
  void makeTarget(IceTargetArch Arch);
  void renumberInstructions(void);
  void placePhiLoads(void);
  void placePhiStores(void);
  void deletePhis(void);
  void doAddressOpt(void);
  void genCode(void);
  void genFrame(void);
  void liveness(IceLiveness Mode);
  void regAlloc(void);

  // TODO: This is a hack, and should be moved into a global context
  // guarded with a mutex.
  static bool HasEmittedFirstMethod;
};

#endif // _IceCfg_h
