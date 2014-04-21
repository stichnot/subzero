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
  static CfgNode *create(IceCfg *Cfg, uint32_t LabelIndex,
                         IceString Name = "") {
    return new (Cfg->allocate<CfgNode>()) CfgNode(Cfg, LabelIndex, Name);
  }

  // Access the label number and name for this node.
  uint32_t getIndex() const { return Number; }
  IceString getName() const;
  IceString getAsmName() const {
    return ".L" + Cfg->getName() + "$" + getName();
  }

  // The HasReturn flag indicates that this node contains a return
  // instruction and therefore needs an epilog.
  void setHasReturn() { HasReturn = true; }
  bool getHasReturn() const { return HasReturn; }

  // Access predecessor and successor edge lists.
  const IceNodeList &getInEdges() const { return InEdges; }
  const IceNodeList &getOutEdges() const { return OutEdges; }

  // Manage the instruction list.
  IceInstList &getInsts() { return Insts; }
  void appendInst(IceInst *Inst);
  void renumberInstructions();

  void splitEdge(CfgNode *From, CfgNode *To);
  // Add a predecessor edge to the InEdges list for each of this
  // node's successors.
  void registerEdges();

  void placePhiLoads();
  void placePhiStores();
  void deletePhis();
  void doAddressOpt();
  void genCode();
  bool liveness(IceLivenessMode Mode, IceLiveness *Liveness);
  void livenessPostprocess(IceLivenessMode Mode, IceLiveness *Liveness);
  void emit(IceCfg *Cfg, uint32_t Option) const;
  void dump(IceCfg *Cfg) const;

private:
  CfgNode(IceCfg *Cfg, uint32_t LabelIndex, IceString Name);
  CfgNode(const CfgNode &) LLVM_DELETED_FUNCTION;
  CfgNode &operator=(const CfgNode &) LLVM_DELETED_FUNCTION;
  IceCfg *const Cfg;
  const uint32_t Number; // label index
  IceString Name;        // for dumping only
  bool HasReturn;        // does this block need an epilog?
  IceNodeList InEdges;   // in no particular order
  IceNodeList OutEdges;  // in no particular order
  IcePhiList Phis;       // unordered set of phi instructions
  IceInstList Insts;     // ordered list of non-phi instructions
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICECFGNODE_H
