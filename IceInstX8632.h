// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef _IceInstX8632_h
#define _IceInstX8632_h

#include "IceDefs.h"
#include "IceInst.h"
#include "IceOperand.h"

class IceTargetX8632;

class IceOperandX8632 : public IceOperand {
public:
  enum IceOperandTypeX8632 { __Start = IceOperand::Target, Mem };
  virtual void emit(IceOstream &Str, uint32_t Option) const = 0;
  void dump(IceOstream &Str) const;

protected:
  IceOperandX8632(IceCfg *Cfg, IceOperandTypeX8632 Kind, IceType Type)
      : IceOperand(Cfg, static_cast<OperandKind>(Kind), Type) {}
};

class IceOperandX8632Mem : public IceOperandX8632 {
public:
  static IceOperandX8632Mem *create(IceCfg *Cfg, IceType Type,
                                    IceVariable *Base, IceConstant *Offset,
                                    IceVariable *Index = NULL,
                                    unsigned Shift = 0) {
    return new IceOperandX8632Mem(Cfg, Type, Base, Offset, Index, Shift);
  }
  IceVariable *getBase() const { return Base; }
  IceConstant *getOffset() const { return Offset; }
  IceVariable *getIndex() const { return Index; }
  unsigned getShift() const { return Shift; }
  virtual void setUse(const IceInst *Inst, const IceCfgNode *Node);
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;

  static bool classof(const IceOperand *Operand) {
    return Operand->getKind() == static_cast<OperandKind>(Mem);
  }

private:
  IceOperandX8632Mem(IceCfg *Cfg, IceType Type, IceVariable *Base,
                     IceConstant *Offset, IceVariable *Index, unsigned Shift);
  IceVariable *Base;
  IceConstant *Offset;
  IceVariable *Index;
  unsigned Shift;
};

class IceInstX8632 : public IceInstTarget {
public:
  enum InstKindX8632 {
    __Start = IceInst::Target,
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
  virtual void emit(IceOstream &Str, uint32_t Option) const = 0;
  virtual void dump(IceOstream &Str) const;

protected:
  IceInstX8632(IceCfg *Cfg, InstKindX8632 Kind, unsigned Maxsrcs,
               IceVariable *Dest)
      : IceInstTarget(Cfg, static_cast<InstKind>(Kind), Maxsrcs, Dest) {}
  static bool isClassof(const IceInst *Inst, InstKindX8632 MyKind) {
    return Inst->getKind() == static_cast<InstKind>(MyKind);
  }
};

// IceInstX8632Label represents an intra-block label that is the
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
class IceInstX8632Label : public IceInstX8632 {
public:
  static IceInstX8632Label *create(IceCfg *Cfg, IceTargetX8632 *Target) {
    return new IceInstX8632Label(Cfg, Target);
  }
  IceString getName(IceCfg *Cfg) const;
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;

private:
  IceInstX8632Label(IceCfg *Cfg, IceTargetX8632 *Target);
  uint32_t Number; // used only for unique label string generation
};

class IceInstX8632Br : public IceInstX8632 {
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
  static IceInstX8632Br *create(IceCfg *Cfg, IceCfgNode *TargetTrue,
                                IceCfgNode *TargetFalse, BrCond Condition) {
    return new IceInstX8632Br(Cfg, TargetTrue, TargetFalse, NULL, Condition);
  }
  static IceInstX8632Br *create(IceCfg *Cfg, IceCfgNode *Target) {
    return new IceInstX8632Br(Cfg, NULL, Target, NULL, Br_None);
  }
  static IceInstX8632Br *create(IceCfg *Cfg, IceCfgNode *Target,
                                BrCond Condition) {
    return new IceInstX8632Br(Cfg, Target, NULL, NULL, Condition);
  }
  static IceInstX8632Br *create(IceCfg *Cfg, IceInstX8632Label *Label,
                                BrCond Condition) {
    return new IceInstX8632Br(Cfg, NULL, NULL, Label, Condition);
  }
  IceCfgNode *getTargetTrue() const { return TargetTrue; }
  IceCfgNode *getTargetFalse() const { return TargetFalse; }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Br); }

private:
  IceInstX8632Br(IceCfg *Cfg, IceCfgNode *TargetTrue, IceCfgNode *TargetFalse,
                 IceInstX8632Label *Label, BrCond Condition);
  BrCond Condition;
  IceCfgNode *TargetTrue;
  IceCfgNode *TargetFalse;
  IceInstX8632Label *Label; // Intra-block branch target
};

class IceInstX8632Call : public IceInstX8632 {
public:
  static IceInstX8632Call *create(IceCfg *Cfg, IceVariable *Dest,
                                  IceOperand *CallTarget, bool Tail) {
    return new IceInstX8632Call(Cfg, Dest, CallTarget, Tail);
  }
  IceOperand *getCallTarget() const { return getSrc(0); }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Call); }

private:
  IceInstX8632Call(IceCfg *Cfg, IceVariable *Dest, IceOperand *CallTarget,
                   bool Tail);
  const bool Tail;
};

void IceEmitTwoAddress(const char *Opcode, const IceInst *Inst, IceOstream &Str,
                       uint32_t Option, bool ShiftHack = false);

template <bool ShiftHack, IceInstX8632::InstKindX8632 K>
class IceInstX8632Binop : public IceInstX8632 {
public:
  static IceInstX8632Binop *create(IceCfg *Cfg, IceVariable *Dest,
                                   IceOperand *Source) {
    return new IceInstX8632Binop(Cfg, Dest, Source);
  }
  virtual void emit(IceOstream &Str, uint32_t Option) const {
    IceEmitTwoAddress(Opcode, this, Str, Option, ShiftHack);
  }
  virtual void dump(IceOstream &Str) const {
    dumpDest(Str);
    Str << " = " << Opcode << "." << getDest()->getType() << " ";
    dumpSources(Str);
  }
  static bool classof(const IceInst *Inst) { return isClassof(Inst, K); }

private:
  IceInstX8632Binop(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source)
      : IceInstX8632(Cfg, K, 2, Dest) {
    addSource(Dest);
    addSource(Source);
  }
  static const char *Opcode;
};

template <IceInstX8632::InstKindX8632 K>
class IceInstX8632Ternop : public IceInstX8632 {
public:
  static IceInstX8632Ternop *create(IceCfg *Cfg, IceVariable *Dest,
                                    IceOperand *Source1, IceOperand *Source2) {
    return new IceInstX8632Ternop(Cfg, Dest, Source1, Source2);
  }
  virtual void emit(IceOstream &Str, uint32_t Option) const {
    assert(getSrcSize() == 3);
    Str << "\t" << Opcode << "\t";
    getSrc(1)->emit(Str, Option);
    Str << "\n";
  }
  virtual void dump(IceOstream &Str) const {
    dumpDest(Str);
    Str << " = " << Opcode << "." << getDest()->getType() << " ";
    dumpSources(Str);
  }
  static bool classof(const IceInst *Inst) { return isClassof(Inst, K); }

private:
  IceInstX8632Ternop(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source1,
                     IceOperand *Source2)
      : IceInstX8632(Cfg, K, 3, Dest) {
    addSource(Dest);
    addSource(Source1);
    addSource(Source2);
  }
  static const char *Opcode;
};

typedef IceInstX8632Binop<false, IceInstX8632::Add> IceInstX8632Add;
typedef IceInstX8632Binop<false, IceInstX8632::Adc> IceInstX8632Adc;
typedef IceInstX8632Binop<false, IceInstX8632::Addss> IceInstX8632Addss;
typedef IceInstX8632Binop<false, IceInstX8632::Sub> IceInstX8632Sub;
typedef IceInstX8632Binop<false, IceInstX8632::Subss> IceInstX8632Subss;
typedef IceInstX8632Binop<false, IceInstX8632::Sbb> IceInstX8632Sbb;
typedef IceInstX8632Binop<false, IceInstX8632::And> IceInstX8632And;
typedef IceInstX8632Binop<false, IceInstX8632::Or> IceInstX8632Or;
typedef IceInstX8632Binop<false, IceInstX8632::Xor> IceInstX8632Xor;
typedef IceInstX8632Binop<false, IceInstX8632::Imul> IceInstX8632Imul;
typedef IceInstX8632Binop<false, IceInstX8632::Mulss> IceInstX8632Mulss;
typedef IceInstX8632Binop<false, IceInstX8632::Divss> IceInstX8632Divss;
typedef IceInstX8632Binop<true, IceInstX8632::Shl> IceInstX8632Shl;
typedef IceInstX8632Binop<true, IceInstX8632::Shr> IceInstX8632Shr;
typedef IceInstX8632Binop<true, IceInstX8632::Sar> IceInstX8632Sar;
typedef IceInstX8632Ternop<IceInstX8632::Idiv> IceInstX8632Idiv;
typedef IceInstX8632Ternop<IceInstX8632::Div> IceInstX8632Div;

class IceInstX8632Mul : public IceInstX8632 {
public:
  static IceInstX8632Mul *create(IceCfg *Cfg, IceVariable *Dest,
                                 IceVariable *Source1, IceOperand *Source2) {
    return new IceInstX8632Mul(Cfg, Dest, Source1, Source2);
  }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Mul); }

private:
  IceInstX8632Mul(IceCfg *Cfg, IceVariable *Dest, IceVariable *Source1,
                  IceOperand *Source2);
};

class IceInstX8632Shld : public IceInstX8632 {
public:
  static IceInstX8632Shld *create(IceCfg *Cfg, IceVariable *Dest,
                                  IceVariable *Source1, IceVariable *Source2) {
    return new IceInstX8632Shld(Cfg, Dest, Source1, Source2);
  }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Shld); }

private:
  IceInstX8632Shld(IceCfg *Cfg, IceVariable *Dest, IceVariable *Source1,
                   IceVariable *Source2);
};

class IceInstX8632Shrd : public IceInstX8632 {
public:
  static IceInstX8632Shrd *create(IceCfg *Cfg, IceVariable *Dest,
                                  IceVariable *Source1, IceVariable *Source2) {
    return new IceInstX8632Shrd(Cfg, Dest, Source1, Source2);
  }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Shrd); }

private:
  IceInstX8632Shrd(IceCfg *Cfg, IceVariable *Dest, IceVariable *Source1,
                   IceVariable *Source2);
};

// Sign-extend eax into edx
class IceInstX8632Cdq : public IceInstX8632 {
public:
  static IceInstX8632Cdq *create(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source) {
    return new IceInstX8632Cdq(Cfg, Dest, Source);
  }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Cdq); }

private:
  IceInstX8632Cdq(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
};

// Cvt instruction - wrapper for cvtsX2sY where X and Y are in {s,d,i}
// as appropriate.  s=float, d=double, i=int.  X and Y are determined
// from dest/src types.  Sign and zero extension on the integer
// operand needs to be done separately.
class IceInstX8632Cvt : public IceInstX8632 {
public:
  static IceInstX8632Cvt *create(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source) {
    return new IceInstX8632Cvt(Cfg, Dest, Source);
  }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Cvt); }

private:
  IceInstX8632Cvt(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
};

class IceInstX8632Icmp : public IceInstX8632 {
public:
  static IceInstX8632Icmp *create(IceCfg *Cfg, IceOperand *Src1,
                                  IceOperand *Src2) {
    return new IceInstX8632Icmp(Cfg, Src1, Src2);
  }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Icmp); }

private:
  IceInstX8632Icmp(IceCfg *Cfg, IceOperand *Src1, IceOperand *Src2);
};

class IceInstX8632Ucomiss : public IceInstX8632 {
public:
  static IceInstX8632Ucomiss *create(IceCfg *Cfg, IceOperand *Src1,
                                     IceOperand *Src2) {
    return new IceInstX8632Ucomiss(Cfg, Src1, Src2);
  }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Ucomiss); }

private:
  IceInstX8632Ucomiss(IceCfg *Cfg, IceOperand *Src1, IceOperand *Src2);
};

class IceInstX8632Test : public IceInstX8632 {
public:
  static IceInstX8632Test *create(IceCfg *Cfg, IceOperand *Source1,
                                  IceOperand *Source2) {
    return new IceInstX8632Test(Cfg, Source1, Source2);
  }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Test); }

private:
  IceInstX8632Test(IceCfg *Cfg, IceOperand *Source1, IceOperand *Source2);
};

// This is essentially a "mov" instruction with an IceOperandX8632Mem
// operand instead of IceVariable as the destination.  It's important
// for liveness that there is no Dest operand.
class IceInstX8632Store : public IceInstX8632 {
public:
  static IceInstX8632Store *create(IceCfg *Cfg, IceOperand *Value,
                                   IceOperandX8632Mem *Mem) {
    return new IceInstX8632Store(Cfg, Value, Mem);
  }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Store); }

private:
  IceInstX8632Store(IceCfg *Cfg, IceOperand *Value, IceOperandX8632Mem *Mem);
};

class IceInstX8632Mov : public IceInstX8632 {
public:
  static IceInstX8632Mov *create(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source) {
    return new IceInstX8632Mov(Cfg, Dest, Source);
  }
  virtual bool isRedundantAssign() const;
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Mov); }

private:
  IceInstX8632Mov(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
};

class IceInstX8632Movsx : public IceInstX8632 {
public:
  static IceInstX8632Movsx *create(IceCfg *Cfg, IceVariable *Dest,
                                   IceOperand *Source) {
    return new IceInstX8632Movsx(Cfg, Dest, Source);
  }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Movsx); }

private:
  IceInstX8632Movsx(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
};

class IceInstX8632Movzx : public IceInstX8632 {
public:
  static IceInstX8632Movzx *create(IceCfg *Cfg, IceVariable *Dest,
                                   IceOperand *Source) {
    return new IceInstX8632Movzx(Cfg, Dest, Source);
  }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Movzx); }

private:
  IceInstX8632Movzx(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
};

class IceInstX8632Fld : public IceInstX8632 {
public:
  static IceInstX8632Fld *create(IceCfg *Cfg, IceOperand *Src) {
    return new IceInstX8632Fld(Cfg, Src);
  }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Fld); }

private:
  IceInstX8632Fld(IceCfg *Cfg, IceOperand *Src);
};

class IceInstX8632Fstp : public IceInstX8632 {
public:
  static IceInstX8632Fstp *create(IceCfg *Cfg, IceVariable *Dest) {
    return new IceInstX8632Fstp(Cfg, Dest);
  }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Fstp); }

private:
  IceInstX8632Fstp(IceCfg *Cfg, IceVariable *Dest);
};

class IceInstX8632Pop : public IceInstX8632 {
public:
  static IceInstX8632Pop *create(IceCfg *Cfg, IceVariable *Dest) {
    return new IceInstX8632Pop(Cfg, Dest);
  }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Pop); }

private:
  IceInstX8632Pop(IceCfg *Cfg, IceVariable *Dest);
};

class IceInstX8632Push : public IceInstX8632 {
public:
  static IceInstX8632Push *create(IceCfg *Cfg, IceOperand *Source) {
    return new IceInstX8632Push(Cfg, Source);
  }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Push); }

private:
  IceInstX8632Push(IceCfg *Cfg, IceOperand *Source);
};

class IceInstX8632Ret : public IceInstX8632 {
public:
  static IceInstX8632Ret *create(IceCfg *Cfg, IceVariable *Source = NULL) {
    return new IceInstX8632Ret(Cfg, Source);
  }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Ret); }

private:
  IceInstX8632Ret(IceCfg *Cfg, IceVariable *Source);
};

#endif // _IceInstX8632_h
