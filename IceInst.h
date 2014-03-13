// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef _IceInst_h
#define _IceInst_h

#include "IceDefs.h"
#include "IceTypes.h"

class IceInst {
public:
  enum InstKind {
    Alloca,
    Arithmetic,
    Assign, // not part of LLVM/PNaCl bitcode
    Br,
    Call,
    Cast,
    Fcmp,
    Icmp,
    Load,
    Phi,
    Ret,
    Select,
    Store,
    Switch,
  };
  InstKind getKind(void) const { return Kind; }

  int32_t getNumber(void) const { return Number; }

  bool isDeleted(void) const { return Deleted; }
  void setDeleted(void) { Deleted = true; }

  bool hasSideEffects(void) const { return HasSideEffects; }

  IceVariable *getDest(void) const { return Dest; }

  uint32_t getSrcSize(void) const { return NumSrcs; }
  IceOperand *getSrc(uint32_t I) const {
    assert(I < getSrcSize());
    return Srcs[I];
  }

  // Returns a list of out-edges corresponding to a terminator
  // instruction, which is the last instruction of the block.
  virtual IceNodeList getTerminatorEdges(void) const {
    assert(0);
    return IceNodeList();
  }

  // Updates the status of the IceVariables contained within the
  // instruction.  In particular, it marks where the Dest variable is
  // first assigned, and it tracks whether variables are live across
  // basic blocks, i.e. used in a different block from their definition.
  void updateVars(IceCfgNode *Node);

  virtual void dump(IceOstream &Str) const;
  void dumpSources(IceOstream &Str) const;
  void dumpDest(IceOstream &Str) const;

  virtual ~IceInst() {}

protected:
  IceInst(IceCfg *Cfg, InstKind Kind, uint32_t MaxSrcs, IceVariable *Dest);
  void addSource(IceOperand *Src) {
    assert(Src);
    assert(NumSrcs < MaxSrcs);
    Srcs[NumSrcs++] = Src;
  }

  const InstKind Kind;
  // Number is the instruction number for describing live ranges.
  int32_t Number;
  // Deleted means irrevocably deleted.
  bool Deleted;
  // HasSideEffects means the instruction is something like a function
  // call or a volatile load that can't be removed even if its Dest
  // variable is not live.
  bool HasSideEffects;

  IceVariable *Dest;
  const uint32_t MaxSrcs; // only used for assert
  uint32_t NumSrcs;
  IceOperand **Srcs;
};

IceOstream &operator<<(IceOstream &Str, const IceInst *I);

// Alloca instruction.  This captures the size in bytes as getSrc(0),
// and the alignment.
class IceInstAlloca : public IceInst {
public:
  static IceInstAlloca *create(IceCfg *Cfg, IceOperand *ByteCount,
                               uint32_t Align, IceVariable *Dest) {
    return new (Cfg->allocateInst<IceInstAlloca>())
        IceInstAlloca(Cfg, ByteCount, Align, Dest);
  }
  uint32_t getAlign() const { return Align; }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Alloca; }

protected:
private:
  IceInstAlloca(IceCfg *Cfg, IceOperand *ByteCount, uint32_t Align,
                IceVariable *Dest);
  const uint32_t Align;
};

// Binary arithmetic instruction.  The source operands are captured in
// getSrc(0) and getSrc(1).
class IceInstArithmetic : public IceInst {
public:
  enum OpKind {
    // Ordered by http://llvm.org/docs/LangRef.html#binary-operations
    Add,
    Fadd,
    Sub,
    Fsub,
    Mul,
    Fmul,
    Udiv,
    Sdiv,
    Fdiv,
    Urem,
    Srem,
    Frem,
    // Ordered by http://llvm.org/docs/LangRef.html#bitwise-binary-operations
    Shl,
    Lshr,
    Ashr,
    And,
    Or,
    Xor
  };
  static IceInstArithmetic *create(IceCfg *Cfg, OpKind Op, IceVariable *Dest,
                                   IceOperand *Source1, IceOperand *Source2) {
    return new (Cfg->allocateInst<IceInstArithmetic>())
        IceInstArithmetic(Cfg, Op, Dest, Source1, Source2);
  }
  OpKind getOp(void) const { return Op; }
  bool isCommutative(void) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == Arithmetic;
  }

private:
  IceInstArithmetic(IceCfg *Cfg, OpKind Op, IceVariable *Dest,
                    IceOperand *Source1, IceOperand *Source2);

  const OpKind Op;
};

// Assignment instruction.  The source operand is captured in
// getSrc(0).  This is not part of the LLVM bitcode, but is a useful
// abstraction for some of the lowering.  E.g., if Phi instruction
// lowering happens before target lowering, or for representing an
// Inttoptr instruction, or as an intermediate step for lowering a
// Load instruction.
class IceInstAssign : public IceInst {
public:
  static IceInstAssign *create(IceCfg *Cfg, IceVariable *Dest,
                               IceOperand *Source) {
    return new (Cfg->allocateInst<IceInstAssign>())
        IceInstAssign(Cfg, Dest, Source);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Assign; }

private:
  IceInstAssign(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
};

// Branch instruction.  This represents both conditional and
// unconditional branches.
class IceInstBr : public IceInst {
public:
  static IceInstBr *create(IceCfg *Cfg, IceOperand *Source,
                           IceCfgNode *TargetTrue, IceCfgNode *TargetFalse) {
    return new (Cfg->allocateInst<IceInstBr>())
        IceInstBr(Cfg, Source, TargetTrue, TargetFalse);
  }
  static IceInstBr *create(IceCfg *Cfg, IceCfgNode *Target) {
    return new (Cfg->allocateInst<IceInstBr>()) IceInstBr(Cfg, Target);
  }
  bool isUnconditional(void) const { return getTargetTrue() == NULL; }
  IceCfgNode *getTargetTrue(void) const { return TargetTrue; }
  IceCfgNode *getTargetFalse(void) const { return TargetFalse; }
  IceCfgNode *getTargetUnconditional(void) const {
    assert(isUnconditional());
    return getTargetFalse();
  }
  virtual IceNodeList getTerminatorEdges(void) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Br; }

private:
  // Conditional branch
  IceInstBr(IceCfg *Cfg, IceOperand *Source, IceCfgNode *TargetTrue,
            IceCfgNode *TargetFalse);
  // Unconditional branch
  IceInstBr(IceCfg *Cfg, IceCfgNode *Target);

  IceCfgNode *const TargetFalse; // Doubles as unconditional branch target
  IceCfgNode *const TargetTrue;  // NULL if unconditional branch
};

// Call instruction.  The call target is captured as getSrc(0), and
// arg I is captured as getSrc(I+1).
class IceInstCall : public IceInst {
public:
  static IceInstCall *create(IceCfg *Cfg, uint32_t NumArgs, IceVariable *Dest,
                             IceOperand *CallTarget, bool Tail) {
    return new (Cfg->allocateInst<IceInstCall>())
        IceInstCall(Cfg, NumArgs, Dest, CallTarget, Tail);
  }
  void addArg(IceOperand *Arg) { addSource(Arg); }
  IceOperand *getCallTarget(void) const { return getSrc(0); }
  IceOperand *getArg(uint32_t I) const { return getSrc(I + 1); }
  uint32_t getNumArgs(void) const { return getSrcSize() - 1; }
  bool isTail(void) const { return Tail; }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Call; }

private:
  IceInstCall(IceCfg *Cfg, uint32_t NumArgs, IceVariable *Dest,
              IceOperand *CallTarget, bool Tail)
      : IceInst(Cfg, IceInst::Call, NumArgs + 1, Dest), Tail(Tail) {
    // Set HasSideEffects so that the call instruction can't be
    // dead-code eliminated.  Don't set this for a deletable intrinsic
    // call.
    HasSideEffects = true;
    addSource(CallTarget);
  }
  const bool Tail;
};

// Cast instruction (a.k.a. conversion operation).
class IceInstCast : public IceInst {
public:
  enum OpKind {
    // Ordered by http://llvm.org/docs/LangRef.html#conversion-operations
    Trunc,
    Zext,
    Sext,
    Fptrunc,
    Fpext,
    Fptoui,
    Fptosi,
    Uitofp,
    Sitofp,
  };
  static IceInstCast *create(IceCfg *Cfg, OpKind CastKind, IceVariable *Dest,
                             IceOperand *Source) {
    return new (Cfg->allocateInst<IceInstCast>())
        IceInstCast(Cfg, CastKind, Dest, Source);
  }
  OpKind getCastKind() const { return CastKind; }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Cast; }

private:
  IceInstCast(IceCfg *Cfg, OpKind CastKind, IceVariable *Dest,
              IceOperand *Source);
  OpKind CastKind;
};

// Floating-point comparison instruction.  The source operands are
// captured in getSrc(0) and getSrc(1).
class IceInstFcmp : public IceInst {
public:
  enum FCond {
    // Ordered by http://llvm.org/docs/LangRef.html#id254
    False,
    Oeq,
    Ogt,
    Oge,
    Olt,
    Ole,
    One,
    Ord,
    Ueq,
    Ugt,
    Uge,
    Ult,
    Ule,
    Une,
    Uno,
    True
  };
  static IceInstFcmp *create(IceCfg *Cfg, FCond Condition, IceVariable *Dest,
                             IceOperand *Source1, IceOperand *Source2) {
    return new (Cfg->allocateInst<IceInstFcmp>())
        IceInstFcmp(Cfg, Condition, Dest, Source1, Source2);
  }
  FCond getCondition(void) const { return Condition; }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Fcmp; }

private:
  IceInstFcmp(IceCfg *Cfg, FCond Condition, IceVariable *Dest,
              IceOperand *Source1, IceOperand *Source2);
  FCond Condition;
};

// Integer comparison instruction.  The source operands are captured
// in getSrc(0) and getSrc(1).
class IceInstIcmp : public IceInst {
public:
  enum ICond {
    // Ordered by http://llvm.org/docs/LangRef.html#id249
    Eq,
    Ne,
    Ugt,
    Uge,
    Ult,
    Ule,
    Sgt,
    Sge,
    Slt,
    Sle
  };
  static IceInstIcmp *create(IceCfg *Cfg, ICond Condition, IceVariable *Dest,
                             IceOperand *Source1, IceOperand *Source2) {
    return new (Cfg->allocateInst<IceInstIcmp>())
        IceInstIcmp(Cfg, Condition, Dest, Source1, Source2);
  }
  ICond getCondition(void) const { return Condition; }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Icmp; }

private:
  IceInstIcmp(IceCfg *Cfg, ICond Condition, IceVariable *Dest,
              IceOperand *Source1, IceOperand *Source2);
  ICond Condition;
};

// Load instruction.  The source address is captured in getSrc(0);
class IceInstLoad : public IceInst {
public:
  static IceInstLoad *create(IceCfg *Cfg, IceVariable *Dest,
                             IceOperand *SourceAddr) {
    return new (Cfg->allocateInst<IceInstLoad>())
        IceInstLoad(Cfg, Dest, SourceAddr);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Load; }

private:
  IceInstLoad(IceCfg *Cfg, IceVariable *Dest, IceOperand *SourceAddr);
};

// Phi instruction.  For incoming edge I, the node is Labels[I] and
// the Phi source operand is getSrc(I).
class IceInstPhi : public IceInst {
public:
  static IceInstPhi *create(IceCfg *Cfg, uint32_t MaxSrcs, IceVariable *Dest) {
    return new (Cfg->allocateInst<IceInstPhi>()) IceInstPhi(Cfg, MaxSrcs, Dest);
  }
  void addArgument(IceOperand *Source, IceCfgNode *Label);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Phi; }

private:
  IceInstPhi(IceCfg *Cfg, uint32_t MaxSrcs, IceVariable *Dest);
  // Labels[] duplicates the InEdges[] information in the enclosing
  // IceCfgNode, but the Phi instruction is created before InEdges[]
  // is available, so it's more complicated to share the list.
  IceCfgNode **Labels;
};

// Ret instruction.  The return value is captured in getSrc(0), but if
// there is no return value (void-type function), then
// getSrcSize()==0.
class IceInstRet : public IceInst {
public:
  static IceInstRet *create(IceCfg *Cfg, IceOperand *Source = NULL) {
    return new (Cfg->allocateInst<IceInstRet>()) IceInstRet(Cfg, Source);
  }
  virtual IceNodeList getTerminatorEdges(void) const { return IceNodeList(); }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Ret; }

private:
  IceInstRet(IceCfg *Cfg, IceOperand *Source);
};

// Select instruction.  The condition, true, and false operands are captured.
class IceInstSelect : public IceInst {
public:
  static IceInstSelect *create(IceCfg *Cfg, IceVariable *Dest,
                               IceOperand *Condition, IceOperand *SourceTrue,
                               IceOperand *SourceFalse) {
    return new (Cfg->allocateInst<IceInstSelect>())
        IceInstSelect(Cfg, Dest, Condition, SourceTrue, SourceFalse);
  }
  IceOperand *getCondition(void) const { return getSrc(0); }
  IceOperand *getTrueOperand(void) const { return getSrc(1); }
  IceOperand *getFalseOperand(void) const { return getSrc(2); }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Select; }

private:
  IceInstSelect(IceCfg *Cfg, IceVariable *Dest, IceOperand *Condition,
                IceOperand *Source1, IceOperand *Source2);
};

// Store instruction.  The address operand is captured, along with the
// data operand to be stored into the address.
class IceInstStore : public IceInst {
public:
  static IceInstStore *create(IceCfg *Cfg, IceOperand *Data, IceOperand *Addr) {
    return new (Cfg->allocateInst<IceInstStore>())
        IceInstStore(Cfg, Data, Addr);
  }
  IceOperand *getAddr(void) const { return getSrc(1); }
  IceOperand *getData(void) const { return getSrc(0); }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Store; }

private:
  IceInstStore(IceCfg *Cfg, IceOperand *Data, IceOperand *Addr);
};

// Switch instruction.  The single source operand is captured as
// getSrc(0).
class IceInstSwitch : public IceInst {
public:
  static IceInstSwitch *create(IceCfg *Cfg, uint32_t NumCases,
                               IceOperand *Source, IceCfgNode *LabelDefault) {
    return new (Cfg->allocateInst<IceInstSwitch>())
        IceInstSwitch(Cfg, NumCases, Source, LabelDefault);
  }
  IceCfgNode *getLabelDefault(void) const { return LabelDefault; }
  uint32_t getNumCases(void) const { return NumCases; }
  uint64_t getValue(uint32_t I) const {
    assert(I < NumCases);
    return Values[I];
  }
  IceCfgNode *getLabel(uint32_t I) const {
    assert(I < NumCases);
    return Labels[I];
  }
  void addBranch(uint32_t CaseIndex, uint64_t Value, IceCfgNode *Label);
  virtual IceNodeList getTerminatorEdges(void) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Switch; }

private:
  IceInstSwitch(IceCfg *Cfg, uint32_t NumCases, IceOperand *Source,
                IceCfgNode *LabelDefault);
  IceCfgNode *LabelDefault;
  uint32_t NumCases;   // not including the default case
  uint64_t *Values;    // size is NumCases
  IceCfgNode **Labels; // size is NumCases
};

#endif // _IceInst_h
