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
  IceInstList &getInsts(void) { return Insts; }
  void appendInst(IceInst *Inst);
  void insertInsts(IceInstList::iterator Location, const IceInstList &NewInsts);
  uint32_t getIndex(void) const { return Number; }
  IceString getName(void) const;
  // The HasReturn flag indicates that this node contains a return
  // instruction and therefore needs an epilog.
  void setHasReturn(void) { HasReturn = true; }
  bool hasReturn(void) const { return HasReturn; }
  const IceNodeList &getInEdges(void) const { return InEdges; }
  const IceNodeList &getOutEdges(void) const { return OutEdges; }
  void renumberInstructions(void);
  void splitEdge(IceCfgNode *From, IceCfgNode *To);
  void registerEdges(void);
  void placePhiLoads(void);
  void placePhiStores(void);
  void deletePhis(void);
  void doAddressOpt(void);
  void genCode(void);
  bool liveness(IceLiveness Mode, bool IsFirst);
  void livenessPostprocess(IceLiveness Mode);
  void dump(IceOstream &Str) const;

private:
  IceCfg *const Cfg;
  const uint32_t Number;                  // label index
  IceString Name;                         // for dumping only
  IceNodeList OutEdges;                   // in no particular order
  IceNodeList InEdges;                    // in no particular order
  std::vector<IceInstList> Compensations; // ordered by InEdges
  // TODO: The Live* vectors are not needed outside liveness analysis,
  // and could be moved outside of IceCfgNode to save memory.
  llvm::BitVector LiveIn, LiveOut;     // TODO: consider llvm::SparseBitVector
  std::vector<int> LiveBegin, LiveEnd; // maps variables to inst numbers
  IcePhiList Phis;                     // unordered set of phi instructions
  IceInstList Insts;                   // ordered list of non-phi instructions
  bool ArePhiLoadsPlaced;
  bool ArePhiStoresPlaced;
  bool HasReturn;
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
};

#endif // _IceCfgNode_h
