/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "IceDefs.h"
#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInst.h"
#include "IceLiveness.h"
#include "IceOperand.h"

void IceLiveness::init(void) {
  // Initialize most of the container sizes.
  uint32_t NumVars = Cfg->getVariables().size();
  uint32_t NumNodes = Cfg->getNumNodes();
  Nodes.resize(NumNodes);
  VarToLiveMap.resize(NumVars);
  if (Mode == IceLiveness_RangesFull)
    LiveRanges.resize(NumVars);

  // Count the number of globals, and the number of locals for each
  // block.
  for (uint32_t i = 0; i < NumVars; ++i) {
    IceVariable *Var = Cfg->getVariables()[i];
    if (Var == NULL)
      continue;
    if (Var->isMultiblockLife()) {
      ++NumGlobals;
    } else {
      uint32_t Index = Var->getLocalUseNode()->getIndex();
      ++Nodes[Index].NumLocals;
    }
  }

  // Resize each IceLivenessNode::LiveToVarMap, and the global
  // LiveToVarMap.  Reset the counts to 0.
  for (uint32_t i = 0; i < NumNodes; ++i) {
    Nodes[i].LiveToVarMap.assign(Nodes[i].NumLocals, NULL);
    Nodes[i].NumLocals = 0;
  }
  LiveToVarMap.assign(NumGlobals, NULL);

  // Sort each variable into the appropriate LiveToVarMap.  Also set
  // VarToLiveMap.
  uint32_t TmpNumGlobals = 0;
  for (uint32_t i = 0; i < NumVars; ++i) {
    IceVariable *Var = Cfg->getVariables()[i];
    if (Var == NULL)
      continue;
    uint32_t VarIndex = Var->getIndex();
    uint32_t LiveIndex;
    if (Var->isMultiblockLife()) {
      LiveIndex = TmpNumGlobals++;
      LiveToVarMap[LiveIndex] = Var;
    } else {
      uint32_t NodeIndex = Var->getLocalUseNode()->getIndex();
      LiveIndex = Nodes[NodeIndex].NumLocals++;
      Nodes[NodeIndex].LiveToVarMap[LiveIndex] = Var;
      LiveIndex += NumGlobals;
    }
    VarToLiveMap[VarIndex] = LiveIndex;
  }
  assert(NumGlobals == TmpNumGlobals);

  // Process each node.
  const IceNodeList &LNodes = Cfg->getLNodes();
  unsigned NumLNodes = LNodes.size();
  for (uint32_t i = 0; i < NumLNodes; ++i) {
    IceLivenessNode &Node = Nodes[LNodes[i]->getIndex()];
    // NumLocals, LiveToVarMap already initialized
    Node.LiveIn.resize(NumGlobals);
    Node.LiveOut.resize(NumGlobals);
    // LiveBegin and LiveEnd are reinitialized before each pass over
    // the block.
  }
}

IceVariable *IceLiveness::getVariable(uint32_t LiveIndex,
                                      const IceCfgNode *Node) const {
  if (LiveIndex < NumGlobals)
    return LiveToVarMap[LiveIndex];
  uint32_t NodeIndex = Node->getIndex();
  return Nodes[NodeIndex].LiveToVarMap[LiveIndex - NumGlobals];
}

uint32_t IceLiveness::getLiveIndex(const IceVariable *Var) const {
  return VarToLiveMap[Var->getIndex()];
}

void IceLiveness::addLiveRange(IceVariable *Var, int Start, int End,
                               uint32_t WeightDelta) {
  IceLiveRange &LiveRange = LiveRanges[Var->getIndex()];
  assert(WeightDelta != IceRegWeight::Inf);
  LiveRange.addSegment(Start, End);
  LiveRange.addWeight(WeightDelta);
}

IceLiveRange &IceLiveness::getLiveRange(IceVariable *Var) {
  return LiveRanges[Var->getIndex()];
}
