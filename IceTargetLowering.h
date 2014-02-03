// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef _IceTargetLowering_h
#define _IceTargetLowering_h

#include "IceDefs.h"
#include "IceTypes.h"

#include "IceInst.h" // for the names of the IceInst subtypes

class IceTargetLowering {
public:
  static IceTargetLowering *createLowering(IceTargetArch Target, IceCfg *Cfg);
  void setRegManager(IceRegManager *R) { RegManager = R; }
  IceInstList lower(const IceInst *Inst, const IceInst *Next,
                    bool &DeleteNextInst);
  virtual IceRegManager *makeRegManager(IceCfgNode *Node) { return NULL; }
  virtual IceInstTarget *makeAssign(IceVariable *Dest, IceOperand *Src) = 0;
  virtual IceVariable *getPhysicalRegister(unsigned RegNum) = 0;
  virtual IceString *getRegNames(void) const = 0;
  // TODO: Configure which registers to allow, e.g. caller-save,
  // callee-save, all, all minus one, etc.
  virtual llvm::SmallBitVector getRegisterMask(void) const = 0;

protected:
  IceTargetLowering(IceCfg *Cfg) : Cfg(Cfg) {}
  virtual IceInstList lowerAlloca(const IceInstAlloca *Inst,
                                  const IceInst *Next,
                                  bool &DeleteNextInst) = 0;
  virtual IceInstList lowerArithmetic(const IceInstArithmetic *Inst,
                                      const IceInst *Next,
                                      bool &DeleteNextInst) = 0;
  virtual IceInstList lowerAssign(const IceInstAssign *Inst,
                                  const IceInst *Next,
                                  bool &DeleteNextInst) = 0;
  virtual IceInstList lowerBr(const IceInstBr *Inst, const IceInst *Next,
                              bool &DeleteNextInst) = 0;
  virtual IceInstList lowerCall(const IceInstCall *Inst, const IceInst *Next,
                                bool &DeleteNextInst) = 0;
  virtual IceInstList lowerCast(const IceInstCast *Inst, const IceInst *Next,
                                bool &DeleteNextInst) = 0;
  virtual IceInstList lowerFcmp(const IceInstFcmp *Inst, const IceInst *Next,
                                bool &DeleteNextInst) = 0;
  virtual IceInstList lowerIcmp(const IceInstIcmp *Inst, const IceInst *Next,
                                bool &DeleteNextInst) = 0;
  virtual IceInstList lowerLoad(const IceInstLoad *Inst, const IceInst *Next,
                                bool &DeleteNextInst) = 0;
  virtual IceInstList lowerPhi(const IceInstPhi *Inst, const IceInst *Next,
                               bool &DeleteNextInst) = 0;
  virtual IceInstList lowerRet(const IceInstRet *Inst, const IceInst *Next,
                               bool &DeleteNextInst) = 0;
  virtual IceInstList lowerSelect(const IceInstSelect *Inst,
                                  const IceInst *Next,
                                  bool &DeleteNextInst) = 0;
  virtual IceInstList lowerStore(const IceInstStore *Inst, const IceInst *Next,
                                 bool &DeleteNextInst) = 0;
  virtual IceInstList lowerSwitch(const IceInstSwitch *Inst,
                                  const IceInst *Next,
                                  bool &DeleteNextInst) = 0;
  IceCfg *const Cfg;
  IceRegManager *RegManager;
};

#endif // _IceTargetLowering_h
