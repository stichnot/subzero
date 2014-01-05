/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

// -*- Mode: c++ -*-
#ifndef _IceCfgNode_h
#define _IceCfgNode_h

#include <list>
#include <vector>

#include "IceDefs.h"

class IceCfgNode {
public:
  IceCfgNode(IceCfg *Cfg, uint32_t LabelIndex);
  void appendInst(IceInst *Inst);
  void addPhi(IceInstPhi *Phi);
  uint32_t getIndex(void) const { return NameIndex; }
  void addFallthrough(uint32_t TargetLabel);
  void addNonFallthrough(uint32_t TargetLabel);
  void registerInEdges(IceCfg *Cfg);
  void findAddressOpt(IceCfg *Cfg);
  void markLastUses(IceCfg *Cfg);
  void placePhiLoads(IceCfg *Cfg);
  void placePhiStores(IceCfg *Cfg);
  void deletePhis(IceCfg *Cfg);
  void genCodeX8632(IceCfg *Cfg);
  void dump(IceOstream &Str) const;
private:
  const uint32_t NameIndex; // label
  IceEdgeList OutEdges; // first is default/fallthrough
  IceEdgeList InEdges; // in no particular order
  std::vector<IceInstPhi *> Phis; // unordered set of phi instructions
  IceInstList Insts; // ordered list of non-phi instructions
  bool ArePhiLoadsPlaced;
  bool ArePhiStoresPlaced;
  // arena allocator for the function?
  // edge-split list for successors
  // node-specific local register manager
  IceRegManager *RegManager;
  // list of live operands on entry (unsure if this will be necessary)
  // virtual<-->physical register mappings (REG)
  // multi-block regalloc stuff:
  //   virtual register <--> operand mappings for first assignment
  //   multi-block regalloc candidates (CAND)
  //   set of compensating register shuffles (COMPREG)
  //   set of compensating loads (COMPLD)
  void insertInsts(IceInstList::iterator Location, const IceInstList &NewInsts);
};

#endif // _IceCfgNode_h
