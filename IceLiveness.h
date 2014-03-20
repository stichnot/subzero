// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef _IceLiveness_h
#define _IceLiveness_h

#include "IceDefs.h"
#include "IceTypes.h"

class IceLivenessNode {
public:
  IceLivenessNode() : NumLocals(0) {}
  // NumLocals is the number of IceVariables local to this block.
  uint32_t NumLocals;
  // LiveToVarMap maps a liveness bitvector index to an IceVariable.
  // This is generally just for printing/dumping.  The index should be
  // less than NumLocals + IceLiveness::NumGlobals.
  std::vector<IceVariable *> LiveToVarMap;
  // LiveIn and LiveOut track the in- and out-liveness of the global
  // variables.  The size of each vector is
  // IceLivenessNode::NumGlobals.
  llvm::BitVector LiveIn, LiveOut;
  // LiveBegin and LiveEnd track the instruction numbers of the start
  // and end of each variable's live range within this block.  The
  // size of each vector is NumLocals + IceLiveness::NumGlobals.
  std::vector<int> LiveBegin, LiveEnd;
};

class IceLiveness {
public:
  IceLiveness(IceCfg *Cfg, IceLivenessMode Mode)
      : Cfg(Cfg), Mode(Mode), NumGlobals(0) {}
  void init();
  IceVariable *getVariable(uint32_t LiveIndex, const IceCfgNode *Node) const;
  uint32_t getLiveIndex(const IceVariable *Var) const;
  uint32_t getGlobalSize() const { return NumGlobals; }
  uint32_t getLocalSize(const IceCfgNode *Node) const {
    return NumGlobals + Nodes[Node->getIndex()].NumLocals;
  }
  llvm::BitVector &getLiveIn(const IceCfgNode *Node) {
    return Nodes[Node->getIndex()].LiveIn;
  }
  llvm::BitVector &getLiveOut(const IceCfgNode *Node) {
    return Nodes[Node->getIndex()].LiveOut;
  }
  std::vector<int> &getLiveBegin(const IceCfgNode *Node) {
    return Nodes[Node->getIndex()].LiveBegin;
  }
  std::vector<int> &getLiveEnd(const IceCfgNode *Node) {
    return Nodes[Node->getIndex()].LiveEnd;
  }
  IceLiveRange &getLiveRange(IceVariable *Var);
  void addLiveRange(IceVariable *Var, int Start, int End, uint32_t WeightDelta);

private:
  IceCfg *Cfg;
  IceLivenessMode Mode;
  uint32_t NumGlobals;
  // Size of Nodes is IceCfg::Nodes.size().
  std::vector<IceLivenessNode> Nodes;
  // VarToLiveMap maps an IceVariable's IceVariable::Number to its
  // live index within its basic block.
  std::vector<uint32_t> VarToLiveMap;
  // LiveToVarMap is analogous to IceLivenessNode::LiveToVarMap, but
  // for non-local variables.
  std::vector<IceVariable *> LiveToVarMap;
  // LiveRanges maps an IceVariable::Number to its live range.
  std::vector<IceLiveRange> LiveRanges;
};

#endif // _IceLiveness_h
