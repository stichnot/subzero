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
  static IceCfgNode *create(IceCfg *Cfg, uint32_t LabelIndex,
                            IceString Name = "") {
    return new IceCfgNode(Cfg, LabelIndex, Name);
  }
  IceInstList &getInsts(void) { return Insts; }
  void appendInst(IceInst *Inst);
  void insertInsts(IceInstList::iterator Location, const IceInstList &NewInsts);
  uint32_t getIndex(void) const { return Number; }
  IceString getName(void) const;
  IceString getAsmName(void) const;
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
  bool liveness(IceLivenessMode Mode, IceLiveness *Liveness);
  void livenessPostprocess(IceLivenessMode Mode, IceLiveness *Liveness);
  void emit(IceOstream &Str, uint32_t Option) const;
  void dump(IceOstream &Str) const;

private:
  IceCfgNode(IceCfg *Cfg, uint32_t LabelIndex, IceString Name);
  IceCfg *const Cfg;
  const uint32_t Number; // label index
  IceString Name;        // for dumping only
  IceNodeList OutEdges;  // in no particular order
  IceNodeList InEdges;   // in no particular order
  IcePhiList Phis;       // unordered set of phi instructions
  IceInstList Insts;     // ordered list of non-phi instructions
  bool ArePhiLoadsPlaced;
  bool ArePhiStoresPlaced;
  bool HasReturn;
};

#endif // _IceCfgNode_h
