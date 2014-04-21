//===- subzero/src/InstX8632.h - Low-level x86 instructions --*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the InstX8632 and OperandX8632 classes and
// their subclasses.  This represents the machine instructions and
// operands used for x86-32 code selection.
//
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEINSTX8632_H
#define SUBZERO_SRC_ICEINSTX8632_H

#include "IceDefs.h"
#include "IceInst.h"
#include "IceOperand.h"

namespace Ice {

class IceTargetX8632;

class OperandX8632 : public Operand {
public:
  enum OperandTypeX8632 {
    __Start = Operand::Target,
    Mem,
    Split
  };
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const = 0;
  void dump(const IceCfg *Cfg) const;

protected:
  OperandX8632(OperandTypeX8632 Kind, IceType Type)
      : Operand(static_cast<OperandKind>(Kind), Type) {}
  OperandX8632(const OperandX8632 &) LLVM_DELETED_FUNCTION;
  OperandX8632 &operator=(const OperandX8632 &) LLVM_DELETED_FUNCTION;
};

class OperandX8632Mem : public OperandX8632 {
public:
  static OperandX8632Mem *create(IceCfg *Cfg, IceType Type, IceVariable *Base,
                                 IceConstant *Offset, IceVariable *Index = NULL,
                                 uint32_t Shift = 0) {
    return new (Cfg->allocate<OperandX8632Mem>())
        OperandX8632Mem(Cfg, Type, Base, Offset, Index, Shift);
  }
  IceVariable *getBase() const { return Base; }
  IceConstant *getOffset() const { return Offset; }
  IceVariable *getIndex() const { return Index; }
  uint32_t getShift() const { return Shift; }
  virtual void setUse(const Inst *Inst, const CfgNode *Node);
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;

  static bool classof(const Operand *Operand) {
    return Operand->getKind() == static_cast<OperandKind>(Mem);
  }

private:
  OperandX8632Mem(IceCfg *Cfg, IceType Type, IceVariable *Base,
                  IceConstant *Offset, IceVariable *Index, uint32_t Shift);
  OperandX8632Mem(const OperandX8632Mem &) LLVM_DELETED_FUNCTION;
  OperandX8632Mem &operator=(const OperandX8632Mem &) LLVM_DELETED_FUNCTION;
  IceVariable *Base;
  IceConstant *Offset;
  IceVariable *Index;
  uint32_t Shift;
};

class IceVariableSplit : public OperandX8632 {
public:
  enum Portion {
    Low,
    High
  };
  static IceVariableSplit *create(IceCfg *Cfg, IceVariable *Var, Portion Part) {
    return new (Cfg->allocate<IceVariableSplit>())
        IceVariableSplit(Cfg, Var, Part);
  }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;

  static bool classof(const Operand *Operand) {
    return Operand->getKind() == static_cast<OperandKind>(Split);
  }

private:
  IceVariableSplit(IceCfg *Cfg, IceVariable *Var, Portion Part)
      : OperandX8632(Split, IceType_i32), Var(Var), Part(Part) {
    Vars = Cfg->allocateArrayOf<IceVariable *>(1);
    Vars[0] = Var;
    NumVars = 1;
  }
  IceVariableSplit(const IceVariableSplit &) LLVM_DELETED_FUNCTION;
  IceVariableSplit &operator=(const IceVariableSplit &) LLVM_DELETED_FUNCTION;
  IceVariable *Var;
  Portion Part;
};

class InstX8632 : public InstTarget {
public:
  enum InstKindX8632 {
    __Start = Inst::Target,
    Adc,
    Add,
    Addss,
    And,
    Br,
    Call,
    Cdq,
    Cvt,
    Div,
    Divss,
    Fld,
    Fstp,
    Icmp,
    Idiv,
    Imul,
    Label,
    Load,
    Mov,
    Movsx,
    Movzx,
    Mul,
    Mulss,
    Or,
    Pop,
    Push,
    Ret,
    Sar,
    Sbb,
    Shl,
    Shld,
    Shr,
    Shrd,
    Store,
    Sub,
    Subss,
    Test,
    Ucomiss,
    Xor
  };
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const = 0;
  virtual void dump(const IceCfg *Cfg) const;

protected:
  InstX8632(IceCfg *Cfg, InstKindX8632 Kind, uint32_t Maxsrcs,
            IceVariable *Dest)
      : InstTarget(Cfg, static_cast<InstKind>(Kind), Maxsrcs, Dest) {}
  static bool isClassof(const Inst *Inst, InstKindX8632 MyKind) {
    return Inst->getKind() == static_cast<InstKind>(MyKind);
  }

private:
  InstX8632(const InstX8632 &) LLVM_DELETED_FUNCTION;
  InstX8632 &operator=(const InstX8632 &) LLVM_DELETED_FUNCTION;
};

// InstX8632Label represents an intra-block label that is the
// target of an intra-block branch.  These are used for lowering i1
// calculations, Select instructions, and 64-bit compares on a 32-bit
// architecture, without basic block splitting.  Basic block splitting
// is not so desirable for several reasons, one of which is the impact
// on decisions based on whether a variable's live range spans
// multiple basic blocks.
//
// Intra-block control flow must be used with caution.  Consider the
// sequence for "c = (a >= b ? x : y)".
//     cmp a, b
//     br lt, L1
//     mov c, x
//     jmp L2
//   L1:
//     mov c, y
//   L2:
//
// Without knowledge of the intra-block control flow, liveness
// analysis will determine the "mov c, x" instruction to be dead.  One
// way to prevent this is to insert a "FakeUse(c)" instruction
// anywhere between the two "mov c, ..." instructions, e.g.:
//
//     cmp a, b
//     br lt, L1
//     mov c, x
//     jmp L2
//     FakeUse(c)
//   L1:
//     mov c, y
//   L2:
//
// The down-side is that "mov c, x" can never be dead-code eliminated
// even if there are no uses of c.  As unlikely as this situation is,
// it would be prevented by running dead code elimination before
// lowering.
class InstX8632Label : public InstX8632 {
public:
  static InstX8632Label *create(IceCfg *Cfg, IceTargetX8632 *Target) {
    return new (Cfg->allocate<InstX8632Label>()) InstX8632Label(Cfg, Target);
  }
  IceString getName(const IceCfg *Cfg) const;
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;

private:
  InstX8632Label(IceCfg *Cfg, IceTargetX8632 *Target);
  InstX8632Label(const InstX8632Label &) LLVM_DELETED_FUNCTION;
  InstX8632Label &operator=(const InstX8632Label &) LLVM_DELETED_FUNCTION;
  uint32_t Number; // used only for unique label string generation
};

class InstX8632Br : public InstX8632 {
public:
  enum BrCond {
    Br_a,
    Br_ae,
    Br_b,
    Br_be,
    Br_e,
    Br_g,
    Br_ge,
    Br_l,
    Br_le,
    Br_ne,
    Br_np,
    Br_p,
    Br_None
  };
  static InstX8632Br *create(IceCfg *Cfg, CfgNode *TargetTrue,
                             CfgNode *TargetFalse, BrCond Condition) {
    return new (Cfg->allocate<InstX8632Br>())
        InstX8632Br(Cfg, TargetTrue, TargetFalse, NULL, Condition);
  }
  static InstX8632Br *create(IceCfg *Cfg, CfgNode *Target) {
    return new (Cfg->allocate<InstX8632Br>())
        InstX8632Br(Cfg, NULL, Target, NULL, Br_None);
  }
  static InstX8632Br *create(IceCfg *Cfg, CfgNode *Target, BrCond Condition) {
    return new (Cfg->allocate<InstX8632Br>())
        InstX8632Br(Cfg, Target, NULL, NULL, Condition);
  }
  static InstX8632Br *create(IceCfg *Cfg, InstX8632Label *Label,
                             BrCond Condition) {
    return new (Cfg->allocate<InstX8632Br>())
        InstX8632Br(Cfg, NULL, NULL, Label, Condition);
  }
  CfgNode *getTargetTrue() const { return TargetTrue; }
  CfgNode *getTargetFalse() const { return TargetFalse; }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Br); }

private:
  InstX8632Br(IceCfg *Cfg, CfgNode *TargetTrue, CfgNode *TargetFalse,
              InstX8632Label *Label, BrCond Condition);
  InstX8632Br(const InstX8632Br &) LLVM_DELETED_FUNCTION;
  InstX8632Br &operator=(const InstX8632Br &) LLVM_DELETED_FUNCTION;
  BrCond Condition;
  CfgNode *TargetTrue;
  CfgNode *TargetFalse;
  InstX8632Label *Label; // Intra-block branch target
};

class InstX8632Call : public InstX8632 {
public:
  static InstX8632Call *create(IceCfg *Cfg, IceVariable *Dest,
                               Operand *CallTarget, bool Tail) {
    return new (Cfg->allocate<InstX8632Call>())
        InstX8632Call(Cfg, Dest, CallTarget, Tail);
  }
  Operand *getCallTarget() const { return getSrc(0); }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Call); }

private:
  InstX8632Call(IceCfg *Cfg, IceVariable *Dest, Operand *CallTarget, bool Tail);
  InstX8632Call(const InstX8632Call &) LLVM_DELETED_FUNCTION;
  InstX8632Call &operator=(const InstX8632Call &) LLVM_DELETED_FUNCTION;
  const bool Tail;
};

void IceEmitTwoAddress(const char *Opcode, const Inst *Inst, const IceCfg *Cfg,
                       uint32_t Option, bool ShiftHack = false);

template <bool ShiftHack, InstX8632::InstKindX8632 K>
class InstX8632Binop : public InstX8632 {
public:
  static InstX8632Binop *create(IceCfg *Cfg, IceVariable *Dest,
                                Operand *Source) {
    return new (Cfg->allocate<InstX8632Binop>())
        InstX8632Binop(Cfg, Dest, Source);
  }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const {
    IceEmitTwoAddress(Opcode, this, Cfg, Option, ShiftHack);
  }
  virtual void dump(const IceCfg *Cfg) const {
    IceOstream &Str = Cfg->getContext()->StrDump;
    dumpDest(Cfg);
    Str << " = " << Opcode << "." << getDest()->getType() << " ";
    dumpSources(Cfg);
  }
  static bool classof(const Inst *Inst) { return isClassof(Inst, K); }

private:
  InstX8632Binop(IceCfg *Cfg, IceVariable *Dest, Operand *Source)
      : InstX8632(Cfg, K, 2, Dest) {
    addSource(Dest);
    addSource(Source);
  }
  InstX8632Binop(const InstX8632Binop &) LLVM_DELETED_FUNCTION;
  InstX8632Binop &operator=(const InstX8632Binop &) LLVM_DELETED_FUNCTION;
  static const char *Opcode;
};

template <InstX8632::InstKindX8632 K> class InstX8632Ternop : public InstX8632 {
public:
  static InstX8632Ternop *create(IceCfg *Cfg, IceVariable *Dest,
                                 Operand *Source1, Operand *Source2) {
    return new (Cfg->allocate<InstX8632Ternop>())
        InstX8632Ternop(Cfg, Dest, Source1, Source2);
  }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const {
    IceOstream &Str = Cfg->getContext()->StrEmit;
    assert(getSrcSize() == 3);
    Str << "\t" << Opcode << "\t";
    getSrc(1)->emit(Cfg, Option);
    Str << "\n";
  }
  virtual void dump(const IceCfg *Cfg) const {
    IceOstream &Str = Cfg->getContext()->StrDump;
    dumpDest(Cfg);
    Str << " = " << Opcode << "." << getDest()->getType() << " ";
    dumpSources(Cfg);
  }
  static bool classof(const Inst *Inst) { return isClassof(Inst, K); }

private:
  InstX8632Ternop(IceCfg *Cfg, IceVariable *Dest, Operand *Source1,
                  Operand *Source2)
      : InstX8632(Cfg, K, 3, Dest) {
    addSource(Dest);
    addSource(Source1);
    addSource(Source2);
  }
  InstX8632Ternop(const InstX8632Ternop &) LLVM_DELETED_FUNCTION;
  InstX8632Ternop &operator=(const InstX8632Ternop &) LLVM_DELETED_FUNCTION;
  static const char *Opcode;
};

typedef InstX8632Binop<false, InstX8632::Add> InstX8632Add;
typedef InstX8632Binop<false, InstX8632::Adc> InstX8632Adc;
typedef InstX8632Binop<false, InstX8632::Addss> InstX8632Addss;
typedef InstX8632Binop<false, InstX8632::Sub> InstX8632Sub;
typedef InstX8632Binop<false, InstX8632::Subss> InstX8632Subss;
typedef InstX8632Binop<false, InstX8632::Sbb> InstX8632Sbb;
typedef InstX8632Binop<false, InstX8632::And> InstX8632And;
typedef InstX8632Binop<false, InstX8632::Or> InstX8632Or;
typedef InstX8632Binop<false, InstX8632::Xor> InstX8632Xor;
typedef InstX8632Binop<false, InstX8632::Imul> InstX8632Imul;
typedef InstX8632Binop<false, InstX8632::Mulss> InstX8632Mulss;
typedef InstX8632Binop<false, InstX8632::Divss> InstX8632Divss;
typedef InstX8632Binop<true, InstX8632::Shl> InstX8632Shl;
typedef InstX8632Binop<true, InstX8632::Shr> InstX8632Shr;
typedef InstX8632Binop<true, InstX8632::Sar> InstX8632Sar;
typedef InstX8632Ternop<InstX8632::Idiv> InstX8632Idiv;
typedef InstX8632Ternop<InstX8632::Div> InstX8632Div;

class InstX8632Mul : public InstX8632 {
public:
  static InstX8632Mul *create(IceCfg *Cfg, IceVariable *Dest,
                              IceVariable *Source1, Operand *Source2) {
    return new (Cfg->allocate<InstX8632Mul>())
        InstX8632Mul(Cfg, Dest, Source1, Source2);
  }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Mul); }

private:
  InstX8632Mul(IceCfg *Cfg, IceVariable *Dest, IceVariable *Source1,
               Operand *Source2);
  InstX8632Mul(const InstX8632Mul &) LLVM_DELETED_FUNCTION;
  InstX8632Mul &operator=(const InstX8632Mul &) LLVM_DELETED_FUNCTION;
};

class InstX8632Shld : public InstX8632 {
public:
  static InstX8632Shld *create(IceCfg *Cfg, IceVariable *Dest,
                               IceVariable *Source1, IceVariable *Source2) {
    return new (Cfg->allocate<InstX8632Shld>())
        InstX8632Shld(Cfg, Dest, Source1, Source2);
  }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Shld); }

private:
  InstX8632Shld(IceCfg *Cfg, IceVariable *Dest, IceVariable *Source1,
                IceVariable *Source2);
  InstX8632Shld(const InstX8632Shld &) LLVM_DELETED_FUNCTION;
  InstX8632Shld &operator=(const InstX8632Shld &) LLVM_DELETED_FUNCTION;
};

class InstX8632Shrd : public InstX8632 {
public:
  static InstX8632Shrd *create(IceCfg *Cfg, IceVariable *Dest,
                               IceVariable *Source1, IceVariable *Source2) {
    return new (Cfg->allocate<InstX8632Shrd>())
        InstX8632Shrd(Cfg, Dest, Source1, Source2);
  }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Shrd); }

private:
  InstX8632Shrd(IceCfg *Cfg, IceVariable *Dest, IceVariable *Source1,
                IceVariable *Source2);
  InstX8632Shrd(const InstX8632Shrd &) LLVM_DELETED_FUNCTION;
  InstX8632Shrd &operator=(const InstX8632Shrd &) LLVM_DELETED_FUNCTION;
};

// Sign-extend eax into edx
class InstX8632Cdq : public InstX8632 {
public:
  static InstX8632Cdq *create(IceCfg *Cfg, IceVariable *Dest, Operand *Source) {
    return new (Cfg->allocate<InstX8632Cdq>()) InstX8632Cdq(Cfg, Dest, Source);
  }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Cdq); }

private:
  InstX8632Cdq(IceCfg *Cfg, IceVariable *Dest, Operand *Source);
  InstX8632Cdq(const InstX8632Cdq &) LLVM_DELETED_FUNCTION;
  InstX8632Cdq &operator=(const InstX8632Cdq &) LLVM_DELETED_FUNCTION;
};

// Cvt instruction - wrapper for cvtsX2sY where X and Y are in {s,d,i}
// as appropriate.  s=float, d=double, i=int.  X and Y are determined
// from dest/src types.  Sign and zero extension on the integer
// operand needs to be done separately.
class InstX8632Cvt : public InstX8632 {
public:
  static InstX8632Cvt *create(IceCfg *Cfg, IceVariable *Dest, Operand *Source) {
    return new (Cfg->allocate<InstX8632Cvt>()) InstX8632Cvt(Cfg, Dest, Source);
  }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Cvt); }

private:
  InstX8632Cvt(IceCfg *Cfg, IceVariable *Dest, Operand *Source);
  InstX8632Cvt(const InstX8632Cvt &) LLVM_DELETED_FUNCTION;
  InstX8632Cvt &operator=(const InstX8632Cvt &) LLVM_DELETED_FUNCTION;
};

class InstX8632Icmp : public InstX8632 {
public:
  static InstX8632Icmp *create(IceCfg *Cfg, Operand *Src1, Operand *Src2) {
    return new (Cfg->allocate<InstX8632Icmp>()) InstX8632Icmp(Cfg, Src1, Src2);
  }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Icmp); }

private:
  InstX8632Icmp(IceCfg *Cfg, Operand *Src1, Operand *Src2);
  InstX8632Icmp(const InstX8632Icmp &) LLVM_DELETED_FUNCTION;
  InstX8632Icmp &operator=(const InstX8632Icmp &) LLVM_DELETED_FUNCTION;
};

class InstX8632Ucomiss : public InstX8632 {
public:
  static InstX8632Ucomiss *create(IceCfg *Cfg, Operand *Src1, Operand *Src2) {
    return new (Cfg->allocate<InstX8632Ucomiss>())
        InstX8632Ucomiss(Cfg, Src1, Src2);
  }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Ucomiss); }

private:
  InstX8632Ucomiss(IceCfg *Cfg, Operand *Src1, Operand *Src2);
  InstX8632Ucomiss(const InstX8632Ucomiss &) LLVM_DELETED_FUNCTION;
  InstX8632Ucomiss &operator=(const InstX8632Ucomiss &) LLVM_DELETED_FUNCTION;
};

class InstX8632Test : public InstX8632 {
public:
  static InstX8632Test *create(IceCfg *Cfg, Operand *Source1,
                               Operand *Source2) {
    return new (Cfg->allocate<InstX8632Test>())
        InstX8632Test(Cfg, Source1, Source2);
  }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Test); }

private:
  InstX8632Test(IceCfg *Cfg, Operand *Source1, Operand *Source2);
  InstX8632Test(const InstX8632Test &) LLVM_DELETED_FUNCTION;
  InstX8632Test &operator=(const InstX8632Test &) LLVM_DELETED_FUNCTION;
};

// This is essentially a "mov" instruction with an OperandX8632Mem
// operand instead of IceVariable as the destination.  It's important
// for liveness that there is no Dest operand.
class InstX8632Store : public InstX8632 {
public:
  static InstX8632Store *create(IceCfg *Cfg, Operand *Value,
                                OperandX8632 *Mem) {
    return new (Cfg->allocate<InstX8632Store>())
        InstX8632Store(Cfg, Value, Mem);
  }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Store); }

private:
  InstX8632Store(IceCfg *Cfg, Operand *Value, OperandX8632 *Mem);
  InstX8632Store(const InstX8632Store &) LLVM_DELETED_FUNCTION;
  InstX8632Store &operator=(const InstX8632Store &) LLVM_DELETED_FUNCTION;
};

class InstX8632Mov : public InstX8632 {
public:
  static InstX8632Mov *create(IceCfg *Cfg, IceVariable *Dest, Operand *Source) {
    return new (Cfg->allocate<InstX8632Mov>()) InstX8632Mov(Cfg, Dest, Source);
  }
  virtual bool isRedundantAssign() const;
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Mov); }

private:
  InstX8632Mov(IceCfg *Cfg, IceVariable *Dest, Operand *Source);
  InstX8632Mov(const InstX8632Mov &) LLVM_DELETED_FUNCTION;
  InstX8632Mov &operator=(const InstX8632Mov &) LLVM_DELETED_FUNCTION;
};

class InstX8632Movsx : public InstX8632 {
public:
  static InstX8632Movsx *create(IceCfg *Cfg, IceVariable *Dest,
                                Operand *Source) {
    return new (Cfg->allocate<InstX8632Movsx>())
        InstX8632Movsx(Cfg, Dest, Source);
  }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Movsx); }

private:
  InstX8632Movsx(IceCfg *Cfg, IceVariable *Dest, Operand *Source);
  InstX8632Movsx(const InstX8632Movsx &) LLVM_DELETED_FUNCTION;
  InstX8632Movsx &operator=(const InstX8632Movsx &) LLVM_DELETED_FUNCTION;
};

class InstX8632Movzx : public InstX8632 {
public:
  static InstX8632Movzx *create(IceCfg *Cfg, IceVariable *Dest,
                                Operand *Source) {
    return new (Cfg->allocate<InstX8632Movzx>())
        InstX8632Movzx(Cfg, Dest, Source);
  }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Movzx); }

private:
  InstX8632Movzx(IceCfg *Cfg, IceVariable *Dest, Operand *Source);
  InstX8632Movzx(const InstX8632Movzx &) LLVM_DELETED_FUNCTION;
  InstX8632Movzx &operator=(const InstX8632Movzx &) LLVM_DELETED_FUNCTION;
};

class InstX8632Fld : public InstX8632 {
public:
  static InstX8632Fld *create(IceCfg *Cfg, Operand *Src) {
    return new (Cfg->allocate<InstX8632Fld>()) InstX8632Fld(Cfg, Src);
  }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Fld); }

private:
  InstX8632Fld(IceCfg *Cfg, Operand *Src);
  InstX8632Fld(const InstX8632Fld &) LLVM_DELETED_FUNCTION;
  InstX8632Fld &operator=(const InstX8632Fld &) LLVM_DELETED_FUNCTION;
};

class InstX8632Fstp : public InstX8632 {
public:
  static InstX8632Fstp *create(IceCfg *Cfg, IceVariable *Dest) {
    return new (Cfg->allocate<InstX8632Fstp>()) InstX8632Fstp(Cfg, Dest);
  }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Fstp); }

private:
  InstX8632Fstp(IceCfg *Cfg, IceVariable *Dest);
  InstX8632Fstp(const InstX8632Fstp &) LLVM_DELETED_FUNCTION;
  InstX8632Fstp &operator=(const InstX8632Fstp &) LLVM_DELETED_FUNCTION;
};

class InstX8632Pop : public InstX8632 {
public:
  static InstX8632Pop *create(IceCfg *Cfg, IceVariable *Dest) {
    return new (Cfg->allocate<InstX8632Pop>()) InstX8632Pop(Cfg, Dest);
  }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Pop); }

private:
  InstX8632Pop(IceCfg *Cfg, IceVariable *Dest);
  InstX8632Pop(const InstX8632Pop &) LLVM_DELETED_FUNCTION;
  InstX8632Pop &operator=(const InstX8632Pop &) LLVM_DELETED_FUNCTION;
};

class InstX8632Push : public InstX8632 {
public:
  static InstX8632Push *create(IceCfg *Cfg, Operand *Source,
                               bool SuppressStackAdjustment) {
    return new (Cfg->allocate<InstX8632Push>())
        InstX8632Push(Cfg, Source, SuppressStackAdjustment);
  }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Push); }

private:
  InstX8632Push(IceCfg *Cfg, Operand *Source, bool SuppressStackAdjustment);
  InstX8632Push(const InstX8632Push &) LLVM_DELETED_FUNCTION;
  InstX8632Push &operator=(const InstX8632Push &) LLVM_DELETED_FUNCTION;
  bool SuppressStackAdjustment;
};

class InstX8632Ret : public InstX8632 {
public:
  static InstX8632Ret *create(IceCfg *Cfg, IceVariable *Source = NULL) {
    return new (Cfg->allocate<InstX8632Ret>()) InstX8632Ret(Cfg, Source);
  }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Ret); }

private:
  InstX8632Ret(IceCfg *Cfg, IceVariable *Source);
  InstX8632Ret(const InstX8632Ret &) LLVM_DELETED_FUNCTION;
  InstX8632Ret &operator=(const InstX8632Ret &) LLVM_DELETED_FUNCTION;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEINSTX8632_H
