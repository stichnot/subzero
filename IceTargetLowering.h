// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef _IceTargetLowering_h
#define _IceTargetLowering_h

#include "IceDefs.h"
#include "IceTypes.h"

class IceTargetLowering {
public:
  static IceTargetLowering *createLowering(IceTargetArch Target, IceCfg *Cfg);
  void setRegManager(IceRegManager *R) { RegManager = R; }
  IceInstList lower(const IceInst *Inst, const IceInst *Next,
                    bool &DeleteNextInst);
  virtual IceInstTarget *makeAssign(IceVariable *Dest, IceOperand *Src) = 0;
  virtual IceString *getRegNames(void) const = 0;

protected:
  IceTargetLowering(IceCfg *Cfg) : Cfg(Cfg) {}
  virtual IceInstList lowerAlloca(const IceInst *Inst, const IceInst *Next,
                                  bool &DeleteNextInst) = 0;
  virtual IceInstList lowerArithmetic(const IceInst *Inst, const IceInst *Next,
                                      bool &DeleteNextInst) = 0;
  virtual IceInstList lowerAssign(const IceInst *Inst, const IceInst *Next,
                                  bool &DeleteNextInst) = 0;
  virtual IceInstList lowerBr(const IceInst *Inst, const IceInst *Next,
                              bool &DeleteNextInst) = 0;
  virtual IceInstList lowerCall(const IceInst *Inst, const IceInst *Next,
                                bool &DeleteNextInst) = 0;
  virtual IceInstList lowerConversion(const IceInst *Inst, const IceInst *Next,
                                      bool &DeleteNextInst) = 0;
  virtual IceInstList lowerFcmp(const IceInst *Inst, const IceInst *Next,
                                bool &DeleteNextInst) = 0;
  virtual IceInstList lowerIcmp(const IceInst *Inst, const IceInst *Next,
                                bool &DeleteNextInst) = 0;
  virtual IceInstList lowerLoad(const IceInst *Inst, const IceInst *Next,
                                bool &DeleteNextInst) = 0;
  virtual IceInstList lowerPhi(const IceInst *Inst, const IceInst *Next,
                               bool &DeleteNextInst) = 0;
  virtual IceInstList lowerRet(const IceInst *Inst, const IceInst *Next,
                               bool &DeleteNextInst) = 0;
  virtual IceInstList lowerSelect(const IceInst *Inst, const IceInst *Next,
                                  bool &DeleteNextInst) = 0;
  virtual IceInstList lowerStore(const IceInst *Inst, const IceInst *Next,
                                 bool &DeleteNextInst) = 0;
  virtual IceInstList lowerSwitch(const IceInst *Inst, const IceInst *Next,
                                  bool &DeleteNextInst) = 0;
  IceCfg *const Cfg;
  IceRegManager *RegManager;
};

#endif // _IceTargetLowering_h
