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

class IceLoweringContext {
public:
  IceLoweringContext(IceCfgNode *Node);
  IceInst *getNextInst(void) const {
    if (Next == End)
      return NULL;
    return *Next;
  }
  void insert(IceInst *Inst) {
    Insts.insert(Next, Inst);
    Inst->updateVars(Node);
  }
  void advanceCur(void) { advance(Cur); }
  void advanceNext(void) { advance(Next); }
  // Node is the argument to IceInst::updateVars().
  IceCfgNode *const Node;
  // Insts is a reference to the container, for inserting new
  // instructions.
  IceInstList &Insts;
  // Cur points to the current instruction being considered.  It is
  // guaranteed to point to a non-deleted instruction, or to be End.
  IceInstList::iterator Cur;
  // Next doubles as a pointer to the next valid instruction (if any),
  // and the new-instruction insertion point.  It is also updated for
  // the caller in case the lowering consumes more than one high-level
  // instruction.  It is guaranteed to point to a non-deleted
  // instruction after Cur, or to be End.  TODO: Consider separating
  // the notion of "next valid instruction" and "new instruction
  // insertion point", to avoid confusion when previously-deleted
  // instructions come between the two points.
  IceInstList::iterator Next;
  // End is a copy of Insts.end(), used if Next needs to be advanced.
  const IceInstList::iterator End;

private:
  void skipDeleted(IceInstList::iterator &I);
  void advance(IceInstList::iterator &I);
};

class IceTargetLowering {
public:
  static IceTargetLowering *createLowering(IceTargetArch Target, IceCfg *Cfg);
  virtual void translate(void) {
    Cfg->setError("Target doesn't specify lowering steps.");
  }

  IceInstList doAddressOpt(const IceInst *Inst);
  void lower(IceLoweringContext &Context);
  virtual IceVariable *getPhysicalRegister(unsigned RegNum) = 0;
  virtual IceString getRegName(int RegNum, IceType Type) const = 0;
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
  virtual const llvm::SmallBitVector &
  getRegisterSetForType(IceType Type) const = 0;
  virtual void addProlog(IceCfgNode *Node) = 0;
  virtual void addEpilog(IceCfgNode *Node) = 0;

  virtual ~IceTargetLowering() {}

protected:
  IceTargetLowering(IceCfg *Cfg)
      : Cfg(Cfg), HasComputedFrame(false), StackAdjustment(0) {}
  virtual void lowerAlloca(const IceInstAlloca *Inst,
                           IceLoweringContext &Context) = 0;
  virtual void lowerArithmetic(const IceInstArithmetic *Inst,
                               IceLoweringContext &Context) = 0;
  virtual void lowerAssign(const IceInstAssign *Inst,
                           IceLoweringContext &Context) = 0;
  virtual void lowerBr(const IceInstBr *Inst, IceLoweringContext &Context) = 0;
  virtual void lowerCall(const IceInstCall *Inst,
                         IceLoweringContext &Context) = 0;
  virtual void lowerCast(const IceInstCast *Inst,
                         IceLoweringContext &Context) = 0;
  virtual void lowerFcmp(const IceInstFcmp *Inst,
                         IceLoweringContext &Context) = 0;
  virtual void lowerIcmp(const IceInstIcmp *Inst,
                         IceLoweringContext &Context) = 0;
  virtual void lowerLoad(const IceInstLoad *Inst,
                         IceLoweringContext &Context) = 0;
  virtual void lowerPhi(const IceInstPhi *Inst,
                        IceLoweringContext &Context) = 0;
  virtual void lowerRet(const IceInstRet *Inst,
                        IceLoweringContext &Context) = 0;
  virtual void lowerSelect(const IceInstSelect *Inst,
                           IceLoweringContext &Context) = 0;
  virtual void lowerStore(const IceInstStore *Inst,
                          IceLoweringContext &Context) = 0;
  virtual void lowerSwitch(const IceInstSwitch *Inst,
                           IceLoweringContext &Context) = 0;

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
  virtual void postLower(const IceLoweringContext &Context) {}

  IceCfg *const Cfg;
  bool HasComputedFrame;
  // StackAdjustment keeps track of the current stack offset from its
  // natural location, as arguments are pushed for a function call.
  int StackAdjustment;
};

#endif // _IceTargetLowering_h
