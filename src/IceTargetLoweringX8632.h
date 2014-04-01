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
#include "IceInstX8632.h"

class IceTargetX8632 : public IceTargetLowering {
public:
  static IceTargetX8632 *create(IceCfg *Cfg) { return new IceTargetX8632(Cfg); }
  virtual void translate();

  virtual IceVariable *getPhysicalRegister(uint32_t RegNum);
  virtual IceString getRegName(uint32_t RegNum, IceType Type) const;
  virtual llvm::SmallBitVector getRegisterSet(RegSetMask Include,
                                              RegSetMask Exclude) const;
  virtual const llvm::SmallBitVector &
  getRegisterSetForType(IceType Type) const {
    return TypeToRegisterSet[Type];
  }
  virtual bool hasFramePointer() const { return IsEbpBasedFrame; }
  virtual uint32_t getFrameOrStackReg() const {
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
  void split64(IceLoweringContext &Context, IceVariable *Var);
  void setArgOffsetAndCopy(IceLoweringContext &Context, IceVariable *Arg,
                           IceVariable *FramePtr, int32_t BasicFrameOffset,
                           int32_t &InArgsSizeBytes);
  IceOperand *loOperand(IceLoweringContext &Context, IceOperand *Operand);
  IceOperand *hiOperand(IceLoweringContext &Context, IceOperand *Operand);
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
  IceOperand *legalizeOperand(IceLoweringContext &C, IceOperand *From,
                              LegalMask Allowed, bool AllowOverlap = false,
                              int32_t RegNum = IceVariable::NoRegister);
  IceVariable *legalizeOperandToVar(IceLoweringContext &C, IceOperand *From,
                                    bool AllowOverlap = false,
                                    int32_t RegNum = IceVariable::NoRegister);
  IceVariable *makeVariableWithReg(IceLoweringContext &C, IceType Type,
                                   int32_t RegNum = IceVariable::NoRegister);
  IceInstCall *makeHelperCall(const IceString &Name, IceType Type,
                              IceVariable *Dest, uint32_t MaxSrcs) {
    bool SuppressMangling = true;
    bool Tailcall = false;
    IceConstant *CallTarget =
        Cfg->getConstantSym(Type, NULL, 0, Name, SuppressMangling);
    IceInstCall *Call =
        IceInstCall::create(Cfg, MaxSrcs, Dest, CallTarget, Tailcall);
    return Call;
  }

  // The following are helpers that insert lowered x86 instructions
  // with minimal syntactic overhead, so that the lowering code can
  // look as close to assembly as practical.
  void _adc(IceLoweringContext &C, IceVariable *Dest, IceOperand *Src0) {
    C.insert(IceInstX8632Adc::create(Cfg, Dest, Src0));
  }
  void _add(IceLoweringContext &C, IceVariable *Dest, IceOperand *Src0) {
    C.insert(IceInstX8632Add::create(Cfg, Dest, Src0));
  }
  void _addss(IceLoweringContext &C, IceVariable *Dest, IceOperand *Src0) {
    C.insert(IceInstX8632Addss::create(Cfg, Dest, Src0));
  }
  void _and(IceLoweringContext &C, IceVariable *Dest, IceOperand *Src0) {
    C.insert(IceInstX8632And::create(Cfg, Dest, Src0));
  }
  void _br(IceLoweringContext &C, IceCfgNode *TargetTrue,
           IceCfgNode *TargetFalse, IceInstX8632Br::BrCond Condition) {
    C.insert(IceInstX8632Br::create(Cfg, TargetTrue, TargetFalse, Condition));
  }
  void _br(IceLoweringContext &C, IceCfgNode *Target) {
    C.insert(IceInstX8632Br::create(Cfg, Target));
  }
  void _br(IceLoweringContext &C, IceCfgNode *Target,
           IceInstX8632Br::BrCond Condition) {
    C.insert(IceInstX8632Br::create(Cfg, Target, Condition));
  }
  void _br(IceLoweringContext &C, IceInstX8632Label *Label,
           IceInstX8632Br::BrCond Condition) {
    C.insert(IceInstX8632Br::create(Cfg, Label, Condition));
  }
  void _cdq(IceLoweringContext &C, IceVariable *Dest, IceOperand *Src0) {
    C.insert(IceInstX8632Cdq::create(Cfg, Dest, Src0));
  }
  void _cvt(IceLoweringContext &C, IceVariable *Dest, IceOperand *Src0) {
    C.insert(IceInstX8632Cvt::create(Cfg, Dest, Src0));
  }
  void _div(IceLoweringContext &C, IceVariable *Dest, IceOperand *Src0,
            IceOperand *Src1) {
    C.insert(IceInstX8632Div::create(Cfg, Dest, Src0, Src1));
  }
  void _divss(IceLoweringContext &C, IceVariable *Dest, IceOperand *Src0) {
    C.insert(IceInstX8632Divss::create(Cfg, Dest, Src0));
  }
  void _fld(IceLoweringContext &C, IceOperand *Src0) {
    C.insert(IceInstX8632Fld::create(Cfg, Src0));
  }
  void _fstp(IceLoweringContext &C, IceVariable *Dest) {
    C.insert(IceInstX8632Fstp::create(Cfg, Dest));
  }
  void _icmp(IceLoweringContext &C, IceOperand *Src0, IceOperand *Src1) {
    C.insert(IceInstX8632Icmp::create(Cfg, Src0, Src1));
  }
  void _idiv(IceLoweringContext &C, IceVariable *Dest, IceOperand *Src0,
             IceOperand *Src1) {
    C.insert(IceInstX8632Idiv::create(Cfg, Dest, Src0, Src1));
  }
  void _imul(IceLoweringContext &C, IceVariable *Dest, IceOperand *Src0) {
    C.insert(IceInstX8632Imul::create(Cfg, Dest, Src0));
  }
  // If Dest=NULL is passed in, then a new variable is created, marked
  // as infinite register allocation weight, and returned through the
  // in/out Dest argument.
  void _mov(IceLoweringContext &C, IceVariable *&Dest, IceOperand *Src0,
            int32_t RegNum = IceVariable::NoRegister) {
    if (Dest == NULL) {
      Dest = legalizeOperandToVar(C, Src0, false, RegNum);
    } else {
      C.insert(IceInstX8632Mov::create(Cfg, Dest, Src0));
    }
  }
  void _movsx(IceLoweringContext &C, IceVariable *Dest, IceOperand *Src0) {
    C.insert(IceInstX8632Movsx::create(Cfg, Dest, Src0));
  }
  void _movzx(IceLoweringContext &C, IceVariable *Dest, IceOperand *Src0) {
    C.insert(IceInstX8632Movzx::create(Cfg, Dest, Src0));
  }
  void _mul(IceLoweringContext &C, IceVariable *Dest, IceVariable *Src0,
            IceOperand *Src1) {
    C.insert(IceInstX8632Mul::create(Cfg, Dest, Src0, Src1));
  }
  void _mulss(IceLoweringContext &C, IceVariable *Dest, IceOperand *Src0) {
    C.insert(IceInstX8632Mulss::create(Cfg, Dest, Src0));
  }
  void _or(IceLoweringContext &C, IceVariable *Dest, IceOperand *Src0) {
    C.insert(IceInstX8632Or::create(Cfg, Dest, Src0));
  }
  void _pop(IceLoweringContext &C, IceVariable *Dest) {
    C.insert(IceInstX8632Pop::create(Cfg, Dest));
  }
  void _push(IceLoweringContext &C, IceOperand *Src0) {
    C.insert(IceInstX8632Push::create(Cfg, Src0));
  }
  void _ret(IceLoweringContext &C, IceVariable *Src0 = NULL) {
    C.insert(IceInstX8632Ret::create(Cfg, Src0));
  }
  void _sar(IceLoweringContext &C, IceVariable *Dest, IceOperand *Src0) {
    C.insert(IceInstX8632Sar::create(Cfg, Dest, Src0));
  }
  void _sbb(IceLoweringContext &C, IceVariable *Dest, IceOperand *Src0) {
    C.insert(IceInstX8632Sbb::create(Cfg, Dest, Src0));
  }
  void _shl(IceLoweringContext &C, IceVariable *Dest, IceOperand *Src0) {
    C.insert(IceInstX8632Shl::create(Cfg, Dest, Src0));
  }
  void _shld(IceLoweringContext &C, IceVariable *Dest, IceVariable *Src0,
             IceVariable *Src1) {
    C.insert(IceInstX8632Shld::create(Cfg, Dest, Src0, Src1));
  }
  void _shr(IceLoweringContext &C, IceVariable *Dest, IceOperand *Src0) {
    C.insert(IceInstX8632Shr::create(Cfg, Dest, Src0));
  }
  void _shrd(IceLoweringContext &C, IceVariable *Dest, IceVariable *Src0,
             IceVariable *Src1) {
    C.insert(IceInstX8632Shrd::create(Cfg, Dest, Src0, Src1));
  }
  void _store(IceLoweringContext &C, IceOperand *Value, IceOperandX8632 *Mem) {
    C.insert(IceInstX8632Store::create(Cfg, Value, Mem));
  }
  void _sub(IceLoweringContext &C, IceVariable *Dest, IceOperand *Src0) {
    C.insert(IceInstX8632Sub::create(Cfg, Dest, Src0));
  }
  void _subss(IceLoweringContext &C, IceVariable *Dest, IceOperand *Src0) {
    C.insert(IceInstX8632Subss::create(Cfg, Dest, Src0));
  }
  void _test(IceLoweringContext &C, IceOperand *Src0, IceOperand *Src1) {
    C.insert(IceInstX8632Test::create(Cfg, Src0, Src1));
  }
  void _ucomiss(IceLoweringContext &C, IceOperand *Src0, IceOperand *Src1) {
    C.insert(IceInstX8632Ucomiss::create(Cfg, Src0, Src1));
  }
  void _xor(IceLoweringContext &C, IceVariable *Dest, IceOperand *Src0) {
    C.insert(IceInstX8632Xor::create(Cfg, Dest, Src0));
  }

  bool IsEbpBasedFrame;
  int32_t FrameSizeLocals;
  int32_t LocalsSizeBytes;
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
