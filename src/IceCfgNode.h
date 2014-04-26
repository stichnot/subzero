//===- subzero/src/CfgNode.h - Control flow graph node -------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the CfgNode class, which represents a single
// basic block as its instruction list, in-edge list, and out-edge
// list.
//
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICECFGNODE_H
#define SUBZERO_SRC_ICECFGNODE_H

#include "IceDefs.h"

namespace Ice {

class CfgNode {
public:
  static CfgNode *create(IceCfg *Func, IceSize_t LabelIndex,
                         IceString Name = "") {
    return new (Func->allocate<CfgNode>()) CfgNode(Func, LabelIndex, Name);
  }

  // Access the label number and name for this node.
  IceSize_t getIndex() const { return Number; }
  IceString getName() const;
  IceString getAsmName() const {
    return ".L" + Func->getFunctionName() + "$" + getName();
  }

  // The HasReturn flag indicates that this node contains a return
  // instruction and therefore needs an epilog.
  void setHasReturn() { HasReturn = true; }
  bool getHasReturn() const { return HasReturn; }

  // Access predecessor and successor edge lists.
  const NodeList &getInEdges() const { return InEdges; }
  const NodeList &getOutEdges() const { return OutEdges; }

  // Manage the instruction list.
  InstList &getInsts() { return Insts; }
  void appendInst(Inst *Inst);
  void renumberInstructions();

  void splitEdge(CfgNode *From, CfgNode *To);
  // Add a predecessor edge to the InEdges list for each of this
  // node's successors.
  void computePredecessors();

  void placePhiLoads();
  void placePhiStores();
  void deletePhis();
  void doAddressOpt();
  void genCode();
  bool liveness(LivenessMode Mode, Liveness *Liveness);
  void livenessPostprocess(LivenessMode Mode, Liveness *Liveness);
  void emit(IceCfg *Func, uint32_t Option) const;
  void dump(IceCfg *Func) const;

private:
  CfgNode(IceCfg *Func, IceSize_t LabelIndex, IceString Name);
  CfgNode(const CfgNode &) LLVM_DELETED_FUNCTION;
  CfgNode &operator=(const CfgNode &) LLVM_DELETED_FUNCTION;
  IceCfg *const Func;
  const IceSize_t Number; // label index
  IceString Name;         // for dumping only
  bool HasReturn;         // does this block need an epilog?
  NodeList InEdges;       // in no particular order
  NodeList OutEdges;      // in no particular order
  PhiList Phis;           // unordered set of phi instructions
  InstList Insts;         // ordered list of non-phi instructions
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICECFGNODE_H
