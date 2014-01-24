// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef _IceCfgNode_h
#define _IceCfgNode_h

#include "IceDefs.h"

class IceCfgNode {
public:
  IceCfgNode(IceCfg *Cfg, uint32_t LabelIndex, IceString Name = "");
  void appendInst(IceInst *Inst);
  void addPhi(IceInstPhi *Phi);
  uint32_t getIndex(void) const { return NameIndex; }
  IceString getName(void) const;
  const IceEdgeList &getInEdges(void) const { return InEdges; }
  const IceEdgeList &getOutEdges(void) const { return OutEdges; }
  void renumberInstructions(void);
  void splitEdge(IceCfgNode *From, IceCfgNode *To);
  void registerEdges(void);
  void findAddressOpt(void);
  void placePhiLoads(void);
  void placePhiStores(void);
  void deletePhis(void);
  void genCode(void);
  void multiblockRegAlloc(void);
  void multiblockCompensation(void);
  bool liveness(IceLiveness Mode, bool IsFirst);
  void livenessPostprocess(IceLiveness Mode);
  void dump(IceOstream &Str) const;

private:
  IceCfg *const Cfg;
  const uint32_t NameIndex;               // label
  IceString Name;                         // for dumping only
  IceEdgeList OutEdges;                   // in no particular order
  IceEdgeList InEdges;                    // in no particular order
  std::vector<IceInstList> Compensations; // ordered by InEdges
  // TODO: The Live* vectors are not needed outside liveness analysis,
  // and could be moved outside of IceCfgNode to save memory.
  llvm::BitVector LiveIn, LiveOut;     // TODO: consider llvm::SparseBitVector
  std::vector<int> LiveBegin, LiveEnd; // maps variables to inst numbers
  IcePhiList Phis;                     // unordered set of phi instructions
  IceInstList Insts;                   // ordered list of non-phi instructions
  bool ArePhiLoadsPlaced;
  bool ArePhiStoresPlaced;
  // TODO: Allow the block to have a list of RegManager objects.  A
  // mid-block call instruction kills all scratch registers at once
  // and there is no relationship between pre-and post-call
  // availability.  This has two advantages.  First, physical register
  // assignment can depend on either incoming or outgoing edges but
  // not both, leading to less compensation code.  Second, especially
  // for blocks containing multiple call instructions, it increases
  // the amount of randomness possible without affecting code quality.
  // The multi-block register allocation would need access to the
  // first and last RegManager objects on the list for computing
  // preferences.
  IceRegManager *RegManager;
  void insertInsts(IceInstList::iterator Location, const IceInstList &NewInsts);
};

#endif // _IceCfgNode_h
