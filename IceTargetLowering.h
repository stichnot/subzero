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
  virtual void translate(void) {
    Cfg->setError("Target doesn't specify lowering steps.");
  }

  IceInstList doAddressOpt(const IceInst *Inst);
  IceInstList lower(const IceInst *Inst, const IceInst *Next,
                    bool &DeleteNextInst);
  virtual IceVariable *getPhysicalRegister(unsigned RegNum) = 0;
  virtual IceString getRegName(int RegNum, IceType Type,
                               uint32_t Option) const = 0;
  virtual bool hasFramePointer(void) const { return false; }
  virtual unsigned getFrameOrStackReg(void) const = 0;
  virtual uint32_t typeWidthOnStack(IceType Type) = 0;
  bool hasComputedFrame(void) const { return HasComputedFrame; }
  int getStackAdjustment(void) const { return StackAdjustment; }
  void updateStackAdjustment(int Offset) { StackAdjustment += Offset; }
  void resetStackAdjustment(void) { StackAdjustment = 0; }
  void setCurrentNode(IceCfgNode *Node) { CurrentNode = Node; }

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
  virtual const llvm::SmallBitVector &
  getRegisterSetForType(IceType Type) const = 0;
  virtual void addProlog(IceCfgNode *Node) = 0;
  virtual void addEpilog(IceCfgNode *Node) = 0;

  virtual ~IceTargetLowering() {}

protected:
  IceTargetLowering(IceCfg *Cfg)
      : Cfg(Cfg), HasComputedFrame(false), StackAdjustment(0),
        CurrentNode(NULL) {}
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
  // This gives the target an opportunity to post-process the lowered
  // expansion before returning.  The primary intention is to do some
  // Register Manager activity as necessary, specifically to eagerly
  // allocate registers based on affinity and other factors.  The
  // simplest lowering does nothing here and leaves it all to a
  // subsequent global register allocation pass.
  virtual void postLower(const IceInstList &Expansion) {}

  IceCfg *const Cfg;
  bool HasComputedFrame;
  // StackAdjustment keeps track of the current stack offset from its
  // natural location, as arguments are pushed for a function call.
  int StackAdjustment;
  IceCfgNode *CurrentNode;
};

#endif // _IceTargetLowering_h
