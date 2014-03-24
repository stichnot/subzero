//===- subzero/src/IceCfgNode.h - Control flow graph node -------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the IceCfgNode class, which represents a single
// basic block as its instruction list, in-edge list, and out-edge
// list.
//
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICECFGNODE_H
#define SUBZERO_SRC_ICECFGNODE_H

#include "IceDefs.h"

class IceCfgNode {
public:
  static IceCfgNode *create(IceCfg *Cfg, uint32_t LabelIndex,
                            IceString Name = "") {
    return new IceCfgNode(Cfg, LabelIndex, Name);
  }

  uint32_t getIndex() const { return Number; }
  IceString getName() const;
  IceString getAsmName() const {
    return ".L" + Cfg->getName() + "$" + getName();
  }

  // The HasReturn flag indicates that this node contains a return
  // instruction and therefore needs an epilog.
  void setHasReturn() { HasReturn = true; }
  bool getHasReturn() const { return HasReturn; }

  const IceNodeList &getInEdges() const { return InEdges; }
  const IceNodeList &getOutEdges() const { return OutEdges; }

  IceInstList &getInsts() { return Insts; }
  void appendInst(IceInst *Inst);
  void renumberInstructions();

  void splitEdge(IceCfgNode *From, IceCfgNode *To);
  void registerEdges();

  void placePhiLoads();
  void placePhiStores();
  void deletePhis();
  void doAddressOpt();
  void genCode();
  bool liveness(IceLivenessMode Mode, IceLiveness *Liveness);
  void livenessPostprocess(IceLivenessMode Mode, IceLiveness *Liveness);
  void emit(IceOstream &Str, uint32_t Option) const;
  void dump(IceOstream &Str) const;

private:
  IceCfgNode(IceCfg *Cfg, uint32_t LabelIndex, IceString Name);
  IceCfg *const Cfg;
  const uint32_t Number; // label index
  IceString Name;        // for dumping only
  bool HasReturn;        // does this block need an epilog?
  IceNodeList InEdges;   // in no particular order
  IceNodeList OutEdges;  // in no particular order
  IcePhiList Phis;       // unordered set of phi instructions
  IceInstList Insts;     // ordered list of non-phi instructions
};

#endif // SUBZERO_SRC_ICECFGNODE_H
