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

  // Manage the name and return type of the function being translated.
  void setName(const IceString &FunctionName) { Name = FunctionName; }
  IceString getName(void) const { return Name; }
  void setReturnType(IceType ReturnType) { Type = ReturnType; }

  // When emitting assembly, we allow a string to be prepended to
  // names of translated functions.  This makes it easier to create an
  // execution test against a reference translator like llc, with both
  // translators using the same bitcode as input.
  void setTestPrefix(const IceString &Prefix) { TestPrefix = Prefix; }
  IceString getTestPrefix(void) const { return TestPrefix; }
  IceString mangleName(const IceString &Name) const {
    return getTestPrefix() + Name;
  }

  // Manage the "internal" attribute of the function.
  void setInternal(bool Internal) { IsInternal = Internal; }
  bool getInternal(void) const { return IsInternal; }

  // Translation error flagging.  If support for some construct is
  // known to be missing, instead of an assertion failure, setError()
  // should be called and the error should be propagated back up.
  // This way, we can gracefully fail to translate and let a fallback
  // translator handle the function.
  void setError(const IceString &Message);
  bool hasError(void) const { return HasError; }
  IceString getError(void) const { return ErrorMessage; }

  // Manage nodes (a.k.a. basic blocks, IceCfgNodes).
  void setEntryNode(IceCfgNode *EntryNode) { Entry = EntryNode; }
  IceCfgNode *getEntryNode(void) const { return Entry; }
  // Create a node and append it to the end of the linearized list.
  IceCfgNode *makeNode(const IceString &Name = "");
  uint32_t getNumNodes(void) const { return Nodes.size(); }
  const IceNodeList &getNodes(void) const { return Nodes; }

  // Manage instruction numbering.
  int newInstNumber(void) { return NextInstNumber++; }

  // Manage IceVariables.
  IceVariable *makeVariable(IceType Type, const IceCfgNode *Node,
                            const IceString &Name = "");
  uint32_t getNumVariables(void) const { return Variables.size(); }
  const IceVarList &getVariables(void) const { return Variables; }

  // Manage arguments to the function.
  void addArg(IceVariable *Arg);
  const IceVarList &getArgs(void) const { return Args; }

  // Manage IceConstants.
  // getConstant*() functions are not const because they might add
  // something to the constant pool.
  IceConstant *getConstantInt(IceType Type, uint64_t ConstantInt64);
  IceConstant *getConstantFloat(float Value);
  IceConstant *getConstantDouble(double Value);
  // Returns a symbolic constant.  Handle is currently unused but is
  // reserved to hold something LLVM-specific to facilitate linking.
  IceConstant *getConstantSym(IceType Type, const void *Handle, int64_t Offset,
                              const IceString &Name = "",
                              bool SuppressMangling = false);

  void registerEdges(void);
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

  IceString Name;       // function name
  IceString TestPrefix; // prepended to all symbol names, for testing
  IceType Type;         // return type
  bool IsInternal;      // internal linkage
  bool HasError;
  IceString ErrorMessage;
  IceCfgNode *Entry; // entry basic block
  IceNodeList Nodes; // linearized node list; Entry should be first
  int NextInstNumber;
  IceVarList Variables;
  IceVarList Args; // subset of Variables, in argument order
  class IceConstantPool *ConstantPool;
};

#endif // _IceCfg_h
