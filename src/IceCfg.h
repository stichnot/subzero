//===- subzero/src/IceCfg.h - Control flow graph ----------------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the IceCfg class, which represents the control
// flow graph and the overall per-function compilation context.
//
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICECFG_H
#define SUBZERO_SRC_ICECFG_H

#include "IceDefs.h"
#include "IceTypes.h"

#include "llvm/Support/Allocator.h"

class IceCfg {
public:
  IceCfg();
  ~IceCfg();

  // Manage the name and return type of the function being translated.
  void setName(const IceString &FunctionName) { Name = FunctionName; }
  IceString getName() const { return Name; }
  void setReturnType(IceType ReturnType) { Type = ReturnType; }

  // When emitting assembly, we allow a string to be prepended to
  // names of translated functions.  This makes it easier to create an
  // execution test against a reference translator like llc, with both
  // translators using the same bitcode as input.
  void setTestPrefix(const IceString &Prefix) { TestPrefix = Prefix; }
  IceString getTestPrefix() const { return TestPrefix; }
  IceString mangleName(const IceString &Name) const;

  // Manage the "internal" attribute of the function.
  void setInternal(bool Internal) { IsInternal = Internal; }
  bool getInternal() const { return IsInternal; }

  // Translation error flagging.  If support for some construct is
  // known to be missing, instead of an assertion failure, setError()
  // should be called and the error should be propagated back up.
  // This way, we can gracefully fail to translate and let a fallback
  // translator handle the function.
  void setError(const IceString &Message);
  bool hasError() const { return HasError; }
  IceString getError() const { return ErrorMessage; }

  // Manage nodes (a.k.a. basic blocks, IceCfgNodes).
  void setEntryNode(IceCfgNode *EntryNode) { Entry = EntryNode; }
  IceCfgNode *getEntryNode() const { return Entry; }
  // Create a node and append it to the end of the linearized list.
  IceCfgNode *makeNode(const IceString &Name = "");
  uint32_t getNumNodes() const { return Nodes.size(); }
  const IceNodeList &getNodes() const { return Nodes; }
  IceCfgNode *splitEdge(IceCfgNode *From, IceCfgNode *To);

  // Manage instruction numbering.
  int32_t newInstNumber() { return NextInstNumber++; }

  // Manage IceVariables.
  IceVariable *makeVariable(IceType Type, const IceCfgNode *Node,
                            const IceString &Name = "");
  uint32_t getNumVariables() const { return Variables.size(); }
  const IceVarList &getVariables() const { return Variables; }

  // Manage arguments to the function.
  void addArg(IceVariable *Arg);
  const IceVarList &getArgs() const { return Args; }

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

  // Miscellaneous accessors.
  IceTargetLowering *getTarget() const { return Target; }
  IceLiveness *getLiveness() const { return Liveness; }
  bool hasComputedFrame() const;

  // Passes over the CFG.
  void translate(IceTargetArch TargetArch);
  void registerEdges();
  void renumberInstructions();
  void placePhiLoads();
  void placePhiStores();
  void deletePhis();
  void doAddressOpt();
  void genCode();
  void genFrame();
  void liveness(IceLivenessMode Mode);
  bool validateLiveness() const;
  void emit(uint32_t Option) const;
  void dump() const;

  // Allocate data of type T using the per-Cfg instruction allocator.
  template <typename T> T *allocate() { return Allocator.Allocate<T>(); }

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
  int32_t NextInstNumber;
  IceVarList Variables;
  IceVarList Args; // subset of Variables, in argument order
  class IceConstantPool *ConstantPool;
  IceTargetLowering *Target;
  IceLiveness *Liveness;

  void makeTarget(IceTargetArch Arch);

  // TODO: This is a hack, and should be moved into a global context
  // guarded with a mutex.  The purpose is to add a header comment at
  // the beginning of emission, doing it once per file rather than
  // once per function.
  static bool HasEmittedFirstMethod;
};

#endif // SUBZERO_SRC_ICECFG_H
