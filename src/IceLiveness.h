//===- subzero/src/Liveness.h - Liveness analysis ------------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Liveness and LivenessNode classes,
// which are used for liveness analysis.  The node-specific
// information tracked for each Variable includes whether it is
// live on entry, whether it is live on exit, the instruction number
// that starts its live range, and the instruction number that ends
// its live range.  At the Cfg level, the actual live intervals are
// recorded.
//
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICELIVENESS_H
#define SUBZERO_SRC_ICELIVENESS_H

#include "IceDefs.h"
#include "IceTypes.h"

namespace Ice {

class LivenessNode {
public:
  LivenessNode() : NumLocals(0) {}
  // NumLocals is the number of Variables local to this block.
  IceSize_t NumLocals;
  // LiveToVarMap maps a liveness bitvector index to an Variable.
  // This is generally just for printing/dumping.  The index should be
  // less than NumLocals + Liveness::NumGlobals.
  std::vector<Variable *> LiveToVarMap;
  // LiveIn and LiveOut track the in- and out-liveness of the global
  // variables.  The size of each vector is
  // LivenessNode::NumGlobals.
  llvm::BitVector LiveIn, LiveOut;
  // LiveBegin and LiveEnd track the instruction numbers of the start
  // and end of each variable's live range within this block.  The
  // size of each vector is NumLocals + Liveness::NumGlobals.
  std::vector<int> LiveBegin, LiveEnd;

private:
  // TODO: Disable these constructors when Liveness::Nodes is no
  // longer an STL container.
  // LivenessNode(const LivenessNode &) LLVM_DELETED_FUNCTION;
  // LivenessNode &operator=(const LivenessNode &) LLVM_DELETED_FUNCTION;
};

class Liveness {
public:
  Liveness(Cfg *Func, LivenessMode Mode)
      : Func(Func), Mode(Mode), NumGlobals(0) {}
  void init();
  Variable *getVariable(IceSize_t LiveIndex, const CfgNode *Node) const;
  IceSize_t getLiveIndex(const Variable *Var) const;
  IceSize_t getGlobalSize() const { return NumGlobals; }
  IceSize_t getLocalSize(const CfgNode *Node) const {
    return NumGlobals + Nodes[Node->getIndex()].NumLocals;
  }
  llvm::BitVector &getLiveIn(const CfgNode *Node) {
    return Nodes[Node->getIndex()].LiveIn;
  }
  llvm::BitVector &getLiveOut(const CfgNode *Node) {
    return Nodes[Node->getIndex()].LiveOut;
  }
  std::vector<int> &getLiveBegin(const CfgNode *Node) {
    return Nodes[Node->getIndex()].LiveBegin;
  }
  std::vector<int> &getLiveEnd(const CfgNode *Node) {
    return Nodes[Node->getIndex()].LiveEnd;
  }
  LiveRange &getLiveRange(Variable *Var);
  void addLiveRange(Variable *Var, int32_t Start, int32_t End,
                    uint32_t WeightDelta);

private:
  Cfg *Func;
  LivenessMode Mode;
  IceSize_t NumGlobals;
  // Size of Nodes is Cfg::Nodes.size().
  std::vector<LivenessNode> Nodes;
  // VarToLiveMap maps an Variable's Variable::Number to its
  // live index within its basic block.
  std::vector<IceSize_t> VarToLiveMap;
  // LiveToVarMap is analogous to LivenessNode::LiveToVarMap, but
  // for non-local variables.
  std::vector<Variable *> LiveToVarMap;
  // LiveRanges maps an Variable::Number to its live range.
  std::vector<LiveRange> LiveRanges;
  Liveness(const Liveness &) LLVM_DELETED_FUNCTION;
  Liveness &operator=(const Liveness &) LLVM_DELETED_FUNCTION;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICELIVENESS_H
