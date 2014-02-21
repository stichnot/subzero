// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef _IceTargetLoweringX8632_h
#define _IceTargetLoweringX8632_h

#include "IceDefs.h"
#include "IceTargetLowering.h"

class IceTargetX8632 : public IceTargetLowering {
public:
  static IceTargetX8632 *create(IceCfg *Cfg) { return new IceTargetX8632(Cfg); }
  virtual IceVariable *getPhysicalRegister(unsigned RegNum);
  virtual IceString *getRegNames(void) const { return RegNames; }
  virtual llvm::SmallBitVector
  getRegisterSet(RegSetMask Include = RegMask_All,
                 RegSetMask Exclude = RegMask_None) const;
  virtual bool hasFramePointer(void) const { return IsEbpBasedFrame; }
  virtual unsigned getFrameOrStackReg(void) const {
    return IsEbpBasedFrame ? Reg_ebp : Reg_esp;
  }
  virtual uint32_t typeWidthOnStack(IceType Type) {
    switch (Type) {
    case IceType_i1:
      return 4;
    default:
      return iceTypeWidth(Type);
    }
  }
  virtual void addProlog(IceCfgNode *Node);
  virtual void addEpilog(IceCfgNode *Node);
  uint32_t makeNextLabelNumber(void) { return NextLabelNumber++; }
  // Ensure that a 64-bit IceVariable has been split into 2 32-bit
  // IceVariables, creating them if necessary.  This is needed for all
  // I64 operations, and it is needed for pushing F64 arguments for
  // function calls using the 32-bit push instruction (though the
  // latter could be done by directly writing to the stack).
  void split64(IceVariable *Var);
  void setArgOffsetAndCopy(IceVariable *Arg, IceVariable *FramePtr,
                           int BasicFrameOffset, int &InArgsSizeBytes,
                           IceInstList &Expansion);
  IceOperand *makeLowOperand(IceOperand *Operand);
  IceOperand *makeHighOperand(IceOperand *Operand);
  enum Registers {
    Reg_eax = 0,
    Reg_ecx = 1,
    Reg_edx = 2,
    Reg_ebx = 3,
    Reg_esp = 4,
    Reg_ebp = 5,
    Reg_esi = 6,
    Reg_edi = 7,
    Reg_NUM = 8
  };

private:
  IceTargetX8632(IceCfg *Cfg)
      : IceTargetLowering(Cfg), IsEbpBasedFrame(false), FrameSizeLocals(0),
        LocalsSizeBytes(0), NextLabelNumber(0),
        PhysicalRegisters(IceVarList(Reg_NUM)) {}

  virtual IceInstList lowerAlloca(const IceInstAlloca *Inst,
                                  const IceInst *Next, bool &DeleteNextInst);
  virtual IceInstList lowerArithmetic(const IceInstArithmetic *Inst,
                                      const IceInst *Next,
                                      bool &DeleteNextInst);
  IceInstList lowerArithmeticI64(const IceInstArithmetic *Inst,
                                 const IceInst *Next, bool &DeleteNextInst);
  virtual IceInstList lowerAssign(const IceInstAssign *Inst,
                                  const IceInst *Next, bool &DeleteNextInst);
  virtual IceInstList lowerBr(const IceInstBr *Inst, const IceInst *Next,
                              bool &DeleteNextInst);
  virtual IceInstList lowerCall(const IceInstCall *Inst, const IceInst *Next,
                                bool &DeleteNextInst);
  virtual IceInstList lowerCast(const IceInstCast *Inst, const IceInst *Next,
                                bool &DeleteNextInst);
  virtual IceInstList lowerFcmp(const IceInstFcmp *Inst, const IceInst *Next,
                                bool &DeleteNextInst);
  virtual IceInstList lowerIcmp(const IceInstIcmp *Inst, const IceInst *Next,
                                bool &DeleteNextInst);
  virtual IceInstList lowerLoad(const IceInstLoad *Inst, const IceInst *Next,
                                bool &DeleteNextInst);
  virtual IceInstList lowerPhi(const IceInstPhi *Inst, const IceInst *Next,
                               bool &DeleteNextInst);
  virtual IceInstList lowerRet(const IceInstRet *Inst, const IceInst *Next,
                               bool &DeleteNextInst);
  virtual IceInstList lowerSelect(const IceInstSelect *Inst,
                                  const IceInst *Next, bool &DeleteNextInst);
  virtual IceInstList lowerStore(const IceInstStore *Inst, const IceInst *Next,
                                 bool &DeleteNextInst);
  virtual IceInstList lowerSwitch(const IceInstSwitch *Inst,
                                  const IceInst *Next, bool &DeleteNextInst);
  virtual IceInstList doAddressOptLoad(const IceInstLoad *Inst);
  virtual IceInstList doAddressOptStore(const IceInstStore *Inst);

  enum OperandLegalization {
    Legal_None = 0,
    Legal_Reg = 1 << 0,
    Legal_Imm = 1 << 1,
    Legal_Mem = 1 << 2, // includes [eax+4*ecx] as well as [esp+12]
    Legal_All = ~Legal_None
  };
  typedef uint32_t LegalMask;
  IceOperand *legalizeOperand(IceOperand *From, LegalMask Allowed,
                              IceInstList &Insts, bool AllowOverlap = false,
                              int RegNum = -1);
  IceVariable *legalizeOperandToVar(IceOperand *From, IceInstList &Insts,
                                    bool AllowOverlap = false, int RegNum = -1);

  bool IsEbpBasedFrame;
  int FrameSizeLocals;
  int LocalsSizeBytes;
  llvm::SmallBitVector RegsUsed;
  uint32_t NextLabelNumber;
  IceVarList PhysicalRegisters;
  static IceString RegNames[];
};

#endif // _IceTargetLoweringX8632_h
