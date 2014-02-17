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
  IceInstList doAddressOpt(const IceInst *Inst);
  IceInstList lower(const IceInst *Inst, const IceInst *Next,
                    bool &DeleteNextInst);
  virtual IceRegManager *makeRegManager(IceCfgNode *Node) { return NULL; }
  virtual IceInstTarget *makeAssign(IceVariable *Dest, IceOperand *Src) = 0;
  virtual IceVariable *getPhysicalRegister(unsigned RegNum) = 0;
  virtual IceString *getRegNames(void) const = 0;
  virtual bool hasFramePointer(void) const { return false; }
  virtual unsigned getFrameOrStackReg(void) const = 0;
  virtual uint32_t typeWidthOnStack(IceType Type) = 0;
  bool hasComputedFrame(void) const { return HasComputedFrame; }
  int getStackAdjustment(void) const { return StackAdjustment; }
  void updateStackAdjustment(int Offset) { StackAdjustment += Offset; }
  void resetStackAdjustment(void) { StackAdjustment = 0; }

  enum RegSet {
    RegMask_None = 0,
    RegMask_CallerSave = 1 << 0,
    RegMask_CalleeSave = 1 << 1,
    RegMask_StackPointer = 1 << 2,
    RegMask_FramePointer = 1 << 3,
    RegMask_All = ~RegMask_None
  };
  typedef uint32_t RegSetMask;
  virtual llvm::SmallBitVector
  getRegisterSet(RegSetMask Include = RegMask_All,
                 RegSetMask Exclude = RegMask_None) const = 0;
  virtual void addProlog(IceCfgNode *Node) = 0;
  virtual void addEpilog(IceCfgNode *Node) = 0;

protected:
  IceTargetLowering(IceCfg *Cfg)
      : Cfg(Cfg), RegManager(NULL), HasComputedFrame(false),
        StackAdjustment(0) {}
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

  virtual IceInstList doAddressOptLoad(const IceInstLoad *Inst) {
    return IceInstList();
  }
  virtual IceInstList doAddressOptStore(const IceInstStore *Inst) {
    return IceInstList();
  }
  IceCfg *const Cfg;
  IceRegManager *RegManager;
  bool HasComputedFrame;
  // StackAdjustment keeps track of the current stack offset from its
  // natural location, as arguments are pushed for a function call.
  int StackAdjustment;
};

#endif // _IceTargetLowering_h
