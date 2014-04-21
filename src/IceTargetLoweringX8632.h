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

namespace Ice {

class IceTargetX8632 : public IceTargetLowering {
public:
  static IceTargetX8632 *create(IceCfg *Cfg) { return new IceTargetX8632(Cfg); }

  virtual void translateOm1();
  virtual void translateO2();

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
  virtual size_t typeWidthInBytesOnStack(IceType Type) {
    return (iceTypeWidthInBytes(Type) + 3) & ~3;
  }
  virtual void addProlog(CfgNode *Node);
  virtual void addEpilog(CfgNode *Node);
  uint32_t makeNextLabelNumber() { return NextLabelNumber++; }
  // Ensure that a 64-bit IceVariable has been split into 2 32-bit
  // IceVariables, creating them if necessary.  This is needed for all
  // I64 operations, and it is needed for pushing F64 arguments for
  // function calls using the 32-bit push instruction (though the
  // latter could be done by directly writing to the stack).
  void split64(IceVariable *Var);
  void setArgOffsetAndCopy(IceVariable *Arg, IceVariable *FramePtr,
                           int32_t BasicFrameOffset, int32_t &InArgsSizeBytes);
  IceOperand *loOperand(IceOperand *Operand);
  IceOperand *hiOperand(IceOperand *Operand);
  enum Registers {
    Reg_eax = 0,
    Reg_ecx = Reg_eax + 1,
    Reg_edx = Reg_eax + 2,
    Reg_ebx = Reg_eax + 3,
    Reg_esp = Reg_eax + 4,
    Reg_ebp = Reg_eax + 5,
    Reg_esi = Reg_eax + 6,
    Reg_edi = Reg_eax + 7,
    Reg_ah, // special hack for 8-bit div instruction
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

  virtual void postLower();

  virtual void lowerAlloca(const InstAlloca *Inst);
  virtual void lowerArithmetic(const InstArithmetic *Inst);
  virtual void lowerAssign(const InstAssign *Inst);
  virtual void lowerBr(const InstBr *Inst);
  virtual void lowerCall(const InstCall *Inst);
  virtual void lowerCast(const InstCast *Inst);
  virtual void lowerFcmp(const InstFcmp *Inst);
  virtual void lowerIcmp(const InstIcmp *Inst);
  virtual void lowerLoad(const InstLoad *Inst);
  virtual void lowerPhi(const InstPhi *Inst);
  virtual void lowerRet(const InstRet *Inst);
  virtual void lowerSelect(const InstSelect *Inst);
  virtual void lowerStore(const InstStore *Inst);
  virtual void lowerSwitch(const InstSwitch *Inst);
  virtual void lowerUnreachable(const InstUnreachable *Inst);
  virtual void doAddressOptLoad();
  virtual void doAddressOptStore();

  enum OperandLegalization {
    Legal_None = 0,
    Legal_Reg = 1 << 0,
    Legal_Imm = 1 << 1,
    Legal_Mem = 1 << 2, // includes [eax+4*ecx] as well as [esp+12]
    Legal_All = ~Legal_None
  };
  typedef uint32_t LegalMask;
  IceOperand *legalize(IceOperand *From, LegalMask Allowed = Legal_All,
                       bool AllowOverlap = false,
                       int32_t RegNum = IceVariable::NoRegister);
  IceVariable *legalizeToVar(IceOperand *From, bool AllowOverlap = false,
                             int32_t RegNum = IceVariable::NoRegister);
  IceVariable *makeReg(IceType Type, int32_t RegNum = IceVariable::NoRegister);
  InstCall *makeHelperCall(const IceString &Name, IceVariable *Dest,
                           uint32_t MaxSrcs) {
    bool SuppressMangling = true;
    bool Tailcall = false;
    IceType Type = Dest ? Dest->getType() : IceType_void;
    IceConstant *CallTarget =
        Ctx->getConstantSym(Type, NULL, 0, Name, SuppressMangling);
    InstCall *Call = InstCall::create(Cfg, MaxSrcs, Dest, CallTarget, Tailcall);
    return Call;
  }

  // The following are helpers that insert lowered x86 instructions
  // with minimal syntactic overhead, so that the lowering code can
  // look as close to assembly as practical.
  void _adc(IceVariable *Dest, IceOperand *Src0) {
    Context.insert(InstX8632Adc::create(Cfg, Dest, Src0));
  }
  void _add(IceVariable *Dest, IceOperand *Src0) {
    Context.insert(InstX8632Add::create(Cfg, Dest, Src0));
  }
  void _addss(IceVariable *Dest, IceOperand *Src0) {
    Context.insert(InstX8632Addss::create(Cfg, Dest, Src0));
  }
  void _and(IceVariable *Dest, IceOperand *Src0) {
    Context.insert(InstX8632And::create(Cfg, Dest, Src0));
  }
  void _br(InstX8632Br::BrCond Condition, CfgNode *TargetTrue,
           CfgNode *TargetFalse) {
    Context.insert(
        InstX8632Br::create(Cfg, TargetTrue, TargetFalse, Condition));
  }
  void _br(CfgNode *Target) {
    Context.insert(InstX8632Br::create(Cfg, Target));
  }
  void _br(InstX8632Br::BrCond Condition, CfgNode *Target) {
    Context.insert(InstX8632Br::create(Cfg, Target, Condition));
  }
  void _br(InstX8632Br::BrCond Condition, InstX8632Label *Label) {
    Context.insert(InstX8632Br::create(Cfg, Label, Condition));
  }
  void _cdq(IceVariable *Dest, IceOperand *Src0) {
    Context.insert(InstX8632Cdq::create(Cfg, Dest, Src0));
  }
  void _cmp(IceOperand *Src0, IceOperand *Src1) {
    Context.insert(InstX8632Icmp::create(Cfg, Src0, Src1));
  }
  void _cvt(IceVariable *Dest, IceOperand *Src0) {
    Context.insert(InstX8632Cvt::create(Cfg, Dest, Src0));
  }
  void _div(IceVariable *Dest, IceOperand *Src0, IceOperand *Src1) {
    Context.insert(InstX8632Div::create(Cfg, Dest, Src0, Src1));
  }
  void _divss(IceVariable *Dest, IceOperand *Src0) {
    Context.insert(InstX8632Divss::create(Cfg, Dest, Src0));
  }
  void _fld(IceOperand *Src0) {
    Context.insert(InstX8632Fld::create(Cfg, Src0));
  }
  void _fstp(IceVariable *Dest) {
    Context.insert(InstX8632Fstp::create(Cfg, Dest));
  }
  void _idiv(IceVariable *Dest, IceOperand *Src0, IceOperand *Src1) {
    Context.insert(InstX8632Idiv::create(Cfg, Dest, Src0, Src1));
  }
  void _imul(IceVariable *Dest, IceOperand *Src0) {
    Context.insert(InstX8632Imul::create(Cfg, Dest, Src0));
  }
  // If Dest=NULL is passed in, then a new variable is created, marked
  // as infinite register allocation weight, and returned through the
  // in/out Dest argument.
  void _mov(IceVariable *&Dest, IceOperand *Src0,
            int32_t RegNum = IceVariable::NoRegister) {
    if (Dest == NULL) {
      Dest = legalizeToVar(Src0, false, RegNum);
    } else {
      Context.insert(InstX8632Mov::create(Cfg, Dest, Src0));
    }
  }
  void _movsx(IceVariable *Dest, IceOperand *Src0) {
    Context.insert(InstX8632Movsx::create(Cfg, Dest, Src0));
  }
  void _movzx(IceVariable *Dest, IceOperand *Src0) {
    Context.insert(InstX8632Movzx::create(Cfg, Dest, Src0));
  }
  void _mul(IceVariable *Dest, IceVariable *Src0, IceOperand *Src1) {
    Context.insert(InstX8632Mul::create(Cfg, Dest, Src0, Src1));
  }
  void _mulss(IceVariable *Dest, IceOperand *Src0) {
    Context.insert(InstX8632Mulss::create(Cfg, Dest, Src0));
  }
  void _or(IceVariable *Dest, IceOperand *Src0) {
    Context.insert(InstX8632Or::create(Cfg, Dest, Src0));
  }
  void _pop(IceVariable *Dest) {
    Context.insert(InstX8632Pop::create(Cfg, Dest));
  }
  void _push(IceOperand *Src0, bool SuppressStackAdjustment = false) {
    Context.insert(InstX8632Push::create(Cfg, Src0, SuppressStackAdjustment));
  }
  void _ret(IceVariable *Src0 = NULL) {
    Context.insert(InstX8632Ret::create(Cfg, Src0));
  }
  void _sar(IceVariable *Dest, IceOperand *Src0) {
    Context.insert(InstX8632Sar::create(Cfg, Dest, Src0));
  }
  void _sbb(IceVariable *Dest, IceOperand *Src0) {
    Context.insert(InstX8632Sbb::create(Cfg, Dest, Src0));
  }
  void _shl(IceVariable *Dest, IceOperand *Src0) {
    Context.insert(InstX8632Shl::create(Cfg, Dest, Src0));
  }
  void _shld(IceVariable *Dest, IceVariable *Src0, IceVariable *Src1) {
    Context.insert(InstX8632Shld::create(Cfg, Dest, Src0, Src1));
  }
  void _shr(IceVariable *Dest, IceOperand *Src0) {
    Context.insert(InstX8632Shr::create(Cfg, Dest, Src0));
  }
  void _shrd(IceVariable *Dest, IceVariable *Src0, IceVariable *Src1) {
    Context.insert(InstX8632Shrd::create(Cfg, Dest, Src0, Src1));
  }
  void _store(IceOperand *Value, IceOperandX8632 *Mem) {
    Context.insert(InstX8632Store::create(Cfg, Value, Mem));
  }
  void _sub(IceVariable *Dest, IceOperand *Src0) {
    Context.insert(InstX8632Sub::create(Cfg, Dest, Src0));
  }
  void _subss(IceVariable *Dest, IceOperand *Src0) {
    Context.insert(InstX8632Subss::create(Cfg, Dest, Src0));
  }
  void _test(IceOperand *Src0, IceOperand *Src1) {
    Context.insert(InstX8632Test::create(Cfg, Src0, Src1));
  }
  void _ucomiss(IceOperand *Src0, IceOperand *Src1) {
    Context.insert(InstX8632Ucomiss::create(Cfg, Src0, Src1));
  }
  void _xor(IceVariable *Dest, IceOperand *Src0) {
    Context.insert(InstX8632Xor::create(Cfg, Dest, Src0));
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

private:
  IceTargetX8632(const IceTargetX8632 &) LLVM_DELETED_FUNCTION;
  IceTargetX8632 &operator=(const IceTargetX8632 &) LLVM_DELETED_FUNCTION;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICETARGETLOWERINGX8632_H
