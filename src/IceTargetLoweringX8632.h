//===- subzero/src/IceTargetLoweringX8632.h - x86-32 lowering ---*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the IceTargetLoweringX8632 class, which
// implements the IceTargetLowering interface for the x86-32
// architecture.
//
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICETARGETLOWERINGX8632_H
#define SUBZERO_SRC_ICETARGETLOWERINGX8632_H

#include "IceDefs.h"
#include "IceTargetLowering.h"

class IceTargetX8632 : public IceTargetLowering {
public:
  static IceTargetX8632 *create(IceCfg *Cfg) { return new IceTargetX8632(Cfg); }
  virtual void translate();

  virtual IceVariable *getPhysicalRegister(unsigned RegNum);
  virtual IceString getRegName(int RegNum, IceType Type) const;
  virtual llvm::SmallBitVector getRegisterSet(RegSetMask Include,
                                              RegSetMask Exclude) const;
  virtual const llvm::SmallBitVector &
  getRegisterSetForType(IceType Type) const {
    return TypeToRegisterSet[Type];
  }
  virtual bool hasFramePointer() const { return IsEbpBasedFrame; }
  virtual unsigned getFrameOrStackReg() const {
    return IsEbpBasedFrame ? Reg_ebp : Reg_esp;
  }
  virtual uint32_t typeWidthOnStack(IceType Type) {
    return (iceTypeWidth(Type) + 3) & ~3;
  }
  virtual void addProlog(IceCfgNode *Node);
  virtual void addEpilog(IceCfgNode *Node);
  uint32_t makeNextLabelNumber() { return NextLabelNumber++; }
  // Ensure that a 64-bit IceVariable has been split into 2 32-bit
  // IceVariables, creating them if necessary.  This is needed for all
  // I64 operations, and it is needed for pushing F64 arguments for
  // function calls using the 32-bit push instruction (though the
  // latter could be done by directly writing to the stack).
  void split64(IceVariable *Var, IceLoweringContext &Context);
  void setArgOffsetAndCopy(IceVariable *Arg, IceVariable *FramePtr,
                           int BasicFrameOffset, int &InArgsSizeBytes,
                           IceLoweringContext &Context);
  IceOperand *makeLowOperand(IceOperand *Operand, IceLoweringContext &Context);
  IceOperand *makeHighOperand(IceOperand *Operand, IceLoweringContext &Context);
  enum Registers {
    Reg_eax = 0,
    Reg_ecx = Reg_eax + 1,
    Reg_edx = Reg_eax + 2,
    Reg_ebx = Reg_eax + 3,
    Reg_esp = Reg_eax + 4,
    Reg_ebp = Reg_eax + 5,
    Reg_esi = Reg_eax + 6,
    Reg_edi = Reg_eax + 7,
    Reg_xmm0,
    Reg_xmm1 = Reg_xmm0 + 1,
    Reg_xmm2 = Reg_xmm0 + 2,
    Reg_xmm3 = Reg_xmm0 + 3,
    Reg_xmm4 = Reg_xmm0 + 4,
    Reg_xmm5 = Reg_xmm0 + 5,
    Reg_xmm6 = Reg_xmm0 + 6,
    Reg_xmm7 = Reg_xmm0 + 7,
    Reg_NUM
  };

protected:
  IceTargetX8632(IceCfg *Cfg);

  virtual void lowerAlloca(const IceInstAlloca *Inst,
                           IceLoweringContext &Context);
  virtual void lowerArithmetic(const IceInstArithmetic *Inst,
                               IceLoweringContext &Context);
  virtual void lowerAssign(const IceInstAssign *Inst,
                           IceLoweringContext &Context);
  virtual void lowerBr(const IceInstBr *Inst, IceLoweringContext &Context);
  virtual void lowerCall(const IceInstCall *Inst, IceLoweringContext &Context);
  virtual void lowerCast(const IceInstCast *Inst, IceLoweringContext &Context);
  virtual void lowerFcmp(const IceInstFcmp *Inst, IceLoweringContext &Context);
  virtual void lowerIcmp(const IceInstIcmp *Inst, IceLoweringContext &Context);
  virtual void lowerLoad(const IceInstLoad *Inst, IceLoweringContext &Context);
  virtual void lowerPhi(const IceInstPhi *Inst, IceLoweringContext &Context);
  virtual void lowerRet(const IceInstRet *Inst, IceLoweringContext &Context);
  virtual void lowerSelect(const IceInstSelect *Inst,
                           IceLoweringContext &Context);
  virtual void lowerStore(const IceInstStore *Inst,
                          IceLoweringContext &Context);
  virtual void lowerSwitch(const IceInstSwitch *Inst,
                           IceLoweringContext &Context);
  virtual void lowerUnreachable(const IceInstUnreachable *Inst,
                                IceLoweringContext &Context);
  virtual void doAddressOptLoad(IceLoweringContext &Context);
  virtual void doAddressOptStore(IceLoweringContext &Context);

  enum OperandLegalization {
    Legal_None = 0,
    Legal_Reg = 1 << 0,
    Legal_Imm = 1 << 1,
    Legal_Mem = 1 << 2, // includes [eax+4*ecx] as well as [esp+12]
    Legal_All = ~Legal_None
  };
  typedef uint32_t LegalMask;
  IceOperand *legalizeOperand(IceOperand *From, LegalMask Allowed,
                              IceLoweringContext &Context,
                              bool AllowOverlap = false, int RegNum = -1);
  IceVariable *legalizeOperandToVar(IceOperand *From,
                                    IceLoweringContext &Context,
                                    bool AllowOverlap = false, int RegNum = -1);

  bool IsEbpBasedFrame;
  int FrameSizeLocals;
  int LocalsSizeBytes;
  llvm::SmallBitVector TypeToRegisterSet[IceType_NUM];
  llvm::SmallBitVector ScratchRegs;
  llvm::SmallBitVector RegsUsed;
  uint32_t NextLabelNumber;
  bool ComputedLiveRanges;
  IceVarList PhysicalRegisters;
  static IceString RegNames[];
};

class IceTargetX8632Fast : public IceTargetX8632 {
public:
  static IceTargetX8632Fast *create(IceCfg *Cfg) {
    return new IceTargetX8632Fast(Cfg);
  }
  virtual void translate();

protected:
  IceTargetX8632Fast(IceCfg *Cfg) : IceTargetX8632(Cfg) {}
  virtual void postLower(const IceLoweringContext &Context);
};

#endif // SUBZERO_SRC_ICETARGETLOWERINGX8632_H
