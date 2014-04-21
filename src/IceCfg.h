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
#include "IceGlobalContext.h"

#include "llvm/ADT/OwningPtr.h"
#include "llvm/Support/Allocator.h"

namespace Ice {

class IceCfg {
public:
  IceCfg(IceGlobalContext *Ctx);
  ~IceCfg();

  IceGlobalContext *getContext() const { return Ctx; }

  // Manage the name and return type of the function being translated.
  void setName(const IceString &FunctionName) { Name = FunctionName; }
  IceString getName() const { return Name; }
  void setReturnType(IceType ReturnType) { Type = ReturnType; }

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

  // Manage nodes (a.k.a. basic blocks, CfgNodes).
  void setEntryNode(CfgNode *EntryNode) { Entry = EntryNode; }
  CfgNode *getEntryNode() const { return Entry; }
  // Create a node and append it to the end of the linearized list.
  CfgNode *makeNode(const IceString &Name = "");
  uint32_t getNumNodes() const { return Nodes.size(); }
  const IceNodeList &getNodes() const { return Nodes; }
  CfgNode *splitEdge(CfgNode *From, CfgNode *To);

  // Manage instruction numbering.
  int32_t newInstNumber() { return NextInstNumber++; }

  // Manage IceVariables.
  IceVariable *makeVariable(IceType Type, const CfgNode *Node,
                            const IceString &Name = "");
  uint32_t getNumVariables() const { return Variables.size(); }
  const IceVarList &getVariables() const { return Variables; }

  // Manage arguments to the function.
  void addArg(IceVariable *Arg);
  const IceVarList &getArgs() const { return Args; }

  // Miscellaneous accessors.
  IceTargetLowering *getTarget() const { return Target.get(); }
  IceLiveness *getLiveness() const { return Liveness.get(); }
  bool hasComputedFrame() const;

  // Passes over the CFG.
  void translate(IceTargetArch TargetArch);
  // After the CFG is fully constructed, iterate over the nodes and
  // compute the predecessor edges, in the form of
  // CfgNode::InEdges[].
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

  // Manage the CurrentNode field, which is used for validating the
  // Variable::DefNode field during dumping/emitting.
  void setCurrentNode(const CfgNode *Node) { CurrentNode = Node; }
  const CfgNode *getCurrentNode() const { return CurrentNode; }

  void emit(uint32_t Option);
  void dump();

  // Allocate data of type T using the per-Cfg allocator.
  template <typename T> T *allocate() { return Allocator.Allocate<T>(); }

  // Allocate an instruction of type T using the per-Cfg instruction allocator.
  template <typename T> T *allocateInst() { return Allocator.Allocate<T>(); }

  // Allocate an array of data of type T using the per-Cfg allocator.
  template <typename T> T *allocateArrayOf(size_t NumElems) {
    return Allocator.Allocate<T>(NumElems);
  }

private:
  // TODO: for now, everything is allocated from the same allocator. In the
  // future we may want to split this to several allocators, for example in
  // order to use a "Recycler" to preserve memory. If we keep all allocation
  // requests from the Cfg exposed via methods, we can always switch the
  // implementation over at a later point.
  llvm::BumpPtrAllocator Allocator;

  IceGlobalContext *Ctx;
  IceString Name;  // function name
  IceType Type;    // return type
  bool IsInternal; // internal linkage
  bool HasError;
  IceString ErrorMessage;
  CfgNode *Entry;    // entry basic block
  IceNodeList Nodes; // linearized node list; Entry should be first
  int32_t NextInstNumber;
  IceVarList Variables;
  IceVarList Args; // subset of Variables, in argument order
  llvm::OwningPtr<IceLiveness> Liveness;
  llvm::OwningPtr<IceTargetLowering> Target;

  // CurrentNode is maintained during dumping/emitting just for
  // validating IceVariable::DefNode.  Normally, a traversal over
  // CfgNodes maintains this, but before global operations like
  // register allocation, setCurrentNode(NULL) should be called to
  // avoid spurious validation failures.
  const CfgNode *CurrentNode;

  // TODO: This is a hack, and should be moved into a global context
  // guarded with a mutex.  The purpose is to add a header comment at
  // the beginning of emission, doing it once per file rather than
  // once per function.
  static bool HasEmittedFirstMethod;

  IceCfg(const IceCfg &) LLVM_DELETED_FUNCTION;
  IceCfg &operator=(const IceCfg &) LLVM_DELETED_FUNCTION;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICECFG_H
