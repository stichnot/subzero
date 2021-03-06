//===- subzero/src/IceCfg.h - Control flow graph ----------------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Cfg class, which represents the control flow
// graph and the overall per-function compilation context.
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

class Cfg {
public:
  Cfg(GlobalContext *Ctx);
  ~Cfg();

  GlobalContext *getContext() const { return Ctx; }

  // Manage the name and return type of the function being translated.
  void setFunctionName(const IceString &Name) { FunctionName = Name; }
  IceString getFunctionName() const { return FunctionName; }
  void setReturnType(Type Ty) { ReturnType = Ty; }

  // Manage the "internal" attribute of the function.
  void setInternal(bool Internal) { IsInternalLinkage = Internal; }
  bool getInternal() const { return IsInternalLinkage; }

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
  SizeT getNumNodes() const { return Nodes.size(); }
  const NodeList &getNodes() const { return Nodes; }
  CfgNode *splitEdge(CfgNode *From, CfgNode *To);

  // Manage instruction numbering.
  InstNumberT newInstNumber() { return NextInstNumber++; }

  // Manage Variables.
  Variable *makeVariable(Type Ty, const CfgNode *Node,
                         const IceString &Name = "");
  SizeT getNumVariables() const { return Variables.size(); }
  const VarList &getVariables() const { return Variables; }

  // Manage arguments to the function.
  void addArg(Variable *Arg);
  const VarList &getArgs() const { return Args; }

  // Miscellaneous accessors.
  TargetLowering *getTarget() const { return Target.get(); }
  Liveness *getLiveness() const { return Live.get(); }
  bool hasComputedFrame() const;

  // Passes over the CFG.
  void translate();
  // After the CFG is fully constructed, iterate over the nodes and
  // compute the predecessor edges, in the form of
  // CfgNode::InEdges[].
  void computePredecessors();
  void renumberInstructions();
  void placePhiLoads();
  void placePhiStores();
  void deletePhis();
  void doAddressOpt();
  void genCode();
  void genFrame();
  void livenessLightweight();
  void liveness(LivenessMode Mode);
  bool validateLiveness() const;

  // Manage the CurrentNode field, which is used for validating the
  // Variable::DefNode field during dumping/emitting.
  void setCurrentNode(const CfgNode *Node) { CurrentNode = Node; }
  const CfgNode *getCurrentNode() const { return CurrentNode; }

  void emit();
  void dump(const IceString &Message = "");

  // Allocate data of type T using the per-Cfg allocator.
  template <typename T> T *allocate() { return Allocator.Allocate<T>(); }

  // Allocate an instruction of type T using the per-Cfg instruction allocator.
  template <typename T> T *allocateInst() { return Allocator.Allocate<T>(); }

  // Allocate an array of data of type T using the per-Cfg allocator.
  template <typename T> T *allocateArrayOf(size_t NumElems) {
    return Allocator.Allocate<T>(NumElems);
  }

  // Deallocate data that was allocated via allocate<T>().
  template <typename T> void deallocate(T *Object) {
    Allocator.Deallocate(Object);
  }

  // Deallocate data that was allocated via allocateInst<T>().
  template <typename T> void deallocateInst(T *Instr) {
    Allocator.Deallocate(Instr);
  }

  // Deallocate data that was allocated via allocateArrayOf<T>().
  template <typename T> void deallocateArrayOf(T *Array) {
    Allocator.Deallocate(Array);
  }

private:
  // TODO: for now, everything is allocated from the same allocator. In the
  // future we may want to split this to several allocators, for example in
  // order to use a "Recycler" to preserve memory. If we keep all allocation
  // requests from the Cfg exposed via methods, we can always switch the
  // implementation over at a later point.
  llvm::BumpPtrAllocator Allocator;

  GlobalContext *Ctx;
  IceString FunctionName;
  Type ReturnType;
  bool IsInternalLinkage;
  bool HasError;
  IceString ErrorMessage;
  CfgNode *Entry; // entry basic block
  NodeList Nodes; // linearized node list; Entry should be first
  InstNumberT NextInstNumber;
  VarList Variables;
  VarList Args; // subset of Variables, in argument order
  llvm::OwningPtr<Liveness> Live;
  llvm::OwningPtr<TargetLowering> Target;

  // CurrentNode is maintained during dumping/emitting just for
  // validating Variable::DefNode.  Normally, a traversal over
  // CfgNodes maintains this, but before global operations like
  // register allocation, setCurrentNode(NULL) should be called to
  // avoid spurious validation failures.
  const CfgNode *CurrentNode;

  Cfg(const Cfg &) LLVM_DELETED_FUNCTION;
  Cfg &operator=(const Cfg &) LLVM_DELETED_FUNCTION;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICECFG_H
