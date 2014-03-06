// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef _IceCfg_h
#define _IceCfg_h

#include "IceDefs.h"
#include "IceTypes.h"

#include "llvm/Support/Allocator.h"

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
  void setInternal(bool Internal) { IsInternal = Internal; }
  bool getInternal(void) const { return IsInternal; }
  void setTestPrefix(const IceString &Prefix) { TestPrefix = Prefix; }
  IceString getTestPrefix(void) const { return TestPrefix; }
  IceString mangleName(const IceString &Name) const;
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
  const IceNodeList &getLNodes(void) const { return LNodes; }
  unsigned getNumNodes(void) const { return Nodes.size(); }
  // getConstant() is not const because it might add something to the
  // constant pool.
  IceConstant *getConstantBits(IceType Type, const void *ConstantBits);
  IceConstant *getConstantInt(IceType Type, uint64_t ConstantInt64);
  IceConstant *getConstantFloat(float Value);
  IceConstant *getConstantDouble(double Value);
  // Returns a symbolic constant.  For now, Handle would refer to
  // something LLVM-specific to facilitate linking.
  IceConstant *getConstant(IceType Type, const void *Handle, int64_t Offset,
                           const IceString &Name = "");
  IceVariable *getVariable(uint32_t Index) const;
  IceVariable *makeVariable(IceType Type, const IceCfgNode *Node,
                            uint32_t Index = -1, const IceString &Name = "");
  const IceVarList &getVariables(void) const { return Variables; }
  const IceVarList &getArgs(void) const { return Args; }
  unsigned getNumVariables(void) const { return Variables.size(); }
  IceLiveness *getLiveness(void) const { return Liveness; }
  int newInstNumber(void);

  IceString physicalRegName(int Reg, IceType Type = IceType_void,
                            uint32_t Option = 0) const;
  void translate(IceTargetArch TargetArch);
  void renumberInstructions(void);
  void placePhiLoads(void);
  void placePhiStores(void);
  void deletePhis(void);
  void doAddressOpt(void);
  void genCode(void);
  void genFrame(void);
  void liveness(IceLivenessMode Mode);
  bool validateLiveness(void) const;
  void regAlloc(void);
  void emit(uint32_t Option) const;
  void dump(void) const;

  // Allocate an instruction of type T using the per-Cfg instruction allocator.
  template <typename T> T *allocateInst() { return Allocator.Allocate<T>(); }

  // Allocate an array of data of type T using the per-Cfg allocator.
  template <typename T> T *allocateArrayOf(size_t NumElems) {
    return Allocator.Allocate<T>(NumElems);
  }

  mutable IceOstream Str;

private:
  // TODO: for now, everything is allocated from the same allocator. In the
  // future we may want to split this to several allocators, for example in
  // order to use a "Recycler" to preserve memory. If we keep all allocation
  // requests from the Cfg exposed via methods, we can always switch the
  // implementation over at a later point.
  llvm::BumpPtrAllocator Allocator;

  bool HasError;
  IceString ErrorMessage;
  IceString Name;       // function name
  bool IsInternal;      // internal linkage
  IceString TestPrefix; // prepended to all symbol names, for testing
  IceType Type;         // return type
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
  IceLiveness *Liveness;

  int NextInstNumber;
  void makeTarget(IceTargetArch Arch);

  // TODO: This is a hack, and should be moved into a global context
  // guarded with a mutex.
  static bool HasEmittedFirstMethod;
};

#endif // _IceCfg_h
