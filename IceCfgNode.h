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

  uint32_t getIndex(void) const { return Number; }
  IceString getName(void) const;

  const IceNodeList &getInEdges(void) const { return InEdges; }
  const IceNodeList &getOutEdges(void) const { return OutEdges; }

  IceInstList &getInsts(void) { return Insts; }
  void appendInst(IceInst *Inst);

  void registerEdges(void);

  void dump(IceOstream &Str) const;

private:
  IceCfgNode(IceCfg *Cfg, uint32_t LabelIndex, IceString Name);
  IceCfg *const Cfg;
  const uint32_t Number; // label index
  IceString Name;        // for dumping only
  IceNodeList InEdges;   // in no particular order
  IceNodeList OutEdges;  // in no particular order
  IcePhiList Phis;       // unordered set of phi instructions
  IceInstList Insts;     // ordered list of non-phi instructions
};

#endif // _IceCfgNode_h
