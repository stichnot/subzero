//===- subzero/src/Liveness.cpp - Liveness analysis implementation -----===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides some of the support for the Liveness class.
// In particular, it handles the sparsity representation of the
// mapping between Variables and CfgNodes.  The idea is that
// since most variables are used only within a single basic block, we
// can partition the variables into "local" and "global" sets.
// Instead of sizing and indexing vectors according to
// Variable::Number, we create a mapping such that global variables
// are mapped to low indexes that are common across nodes, and local
// variables are mapped to a higher index space that is shared across
// nodes.
//
//===----------------------------------------------------------------------===//

#include "IceDefs.h"
#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInst.h"
#include "IceLiveness.h"
#include "IceOperand.h"

namespace Ice {

void Liveness::init() {
  // Initialize most of the container sizes.
  IceSize_t NumVars = Cfg->getVariables().size();
  IceSize_t NumNodes = Cfg->getNumNodes();
  Nodes.resize(NumNodes);
  VarToLiveMap.resize(NumVars);
  if (Mode == Liveness_RangesFull)
    LiveRanges.resize(NumVars);

  // Count the number of globals, and the number of locals for each
  // block.
  for (IceSize_t i = 0; i < NumVars; ++i) {
    Variable *Var = Cfg->getVariables()[i];
    if (Var->isMultiblockLife()) {
      ++NumGlobals;
    } else {
      IceSize_t Index = Var->getLocalUseNode()->getIndex();
      ++Nodes[Index].NumLocals;
    }
  }

  // Resize each LivenessNode::LiveToVarMap, and the global
  // LiveToVarMap.  Reset the counts to 0.
  for (IceSize_t i = 0; i < NumNodes; ++i) {
    Nodes[i].LiveToVarMap.assign(Nodes[i].NumLocals, NULL);
    Nodes[i].NumLocals = 0;
  }
  LiveToVarMap.assign(NumGlobals, NULL);

  // Sort each variable into the appropriate LiveToVarMap.  Also set
  // VarToLiveMap.
  IceSize_t TmpNumGlobals = 0;
  for (IceSize_t i = 0; i < NumVars; ++i) {
    Variable *Var = Cfg->getVariables()[i];
    IceSize_t VarIndex = Var->getIndex();
    IceSize_t LiveIndex;
    if (Var->isMultiblockLife()) {
      LiveIndex = TmpNumGlobals++;
      LiveToVarMap[LiveIndex] = Var;
    } else {
      IceSize_t NodeIndex = Var->getLocalUseNode()->getIndex();
      LiveIndex = Nodes[NodeIndex].NumLocals++;
      Nodes[NodeIndex].LiveToVarMap[LiveIndex] = Var;
      LiveIndex += NumGlobals;
    }
    VarToLiveMap[VarIndex] = LiveIndex;
  }
  assert(NumGlobals == TmpNumGlobals);

  // Process each node.
  const NodeList &LNodes = Cfg->getNodes();
  IceSize_t NumLNodes = LNodes.size();
  for (IceSize_t i = 0; i < NumLNodes; ++i) {
    LivenessNode &Node = Nodes[LNodes[i]->getIndex()];
    // NumLocals, LiveToVarMap already initialized
    Node.LiveIn.resize(NumGlobals);
    Node.LiveOut.resize(NumGlobals);
    // LiveBegin and LiveEnd are reinitialized before each pass over
    // the block.
  }
}

Variable *Liveness::getVariable(IceSize_t LiveIndex,
                                const CfgNode *Node) const {
  if (LiveIndex < NumGlobals)
    return LiveToVarMap[LiveIndex];
  IceSize_t NodeIndex = Node->getIndex();
  return Nodes[NodeIndex].LiveToVarMap[LiveIndex - NumGlobals];
}

IceSize_t Liveness::getLiveIndex(const Variable *Var) const {
  return VarToLiveMap[Var->getIndex()];
}

void Liveness::addLiveRange(Variable *Var, int32_t Start, int32_t End,
                            uint32_t WeightDelta) {
  LiveRange &LiveRange = LiveRanges[Var->getIndex()];
  assert(WeightDelta != RegWeight::Inf);
  LiveRange.addSegment(Start, End);
  LiveRange.addWeight(WeightDelta);
}

LiveRange &Liveness::getLiveRange(Variable *Var) {
  return LiveRanges[Var->getIndex()];
}

} // end of namespace Ice
