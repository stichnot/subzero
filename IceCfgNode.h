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

#endif // _IceCfgNode_h
