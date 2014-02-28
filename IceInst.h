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
  enum IceInstType {
    // Alphabetical order
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
    FakeDef,  // not part of LLVM/PNaCl bitcode
    FakeUse,  // not part of LLVM/PNaCl bitcode
    FakeKill, // not part of LLVM/PNaCl bitcode
    Target    // target-specific low-level ICE
              // Anything >= Target is an IceInstTarget subclass.
  };
  int getNumber(void) const { return Number; }
  void renumber(IceCfg *Cfg);
  IceInstType getKind(void) const { return Kind; }
  IceVariable *getDest(void) const { return Dest; }
  IceOperand *getSrc(unsigned I) const {
    assert(I < getSrcSize());
    return Srcs[I];
  }
  unsigned getSrcSize(void) const { return NumSrcs; }
  virtual IceNodeList getTerminatorEdges(void) const {
    assert(0);
    return IceNodeList();
  }
  bool isDeleted(void) const { return Deleted; }
  bool isLastUse(const IceOperand *Src) const;
  // If an instruction is deleted as a result of replacing it with
  // equivalent instructions, only call setDeleted() *after* inserting
  // the new instructions because of the cascading deletes from
  // reference counting.
  void setDeleted(void) { Deleted = true; }
  void deleteIfDead(void);
  void updateVars(IceCfgNode *Node);
  void liveness(IceLivenessMode Mode, int InstNumber, llvm::BitVector &Live,
                IceLiveness *Liveness, const IceCfgNode *Node);
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  virtual void dumpExtras(IceOstream &Str) const;
  void dumpSources(IceOstream &Str) const;
  void dumpDest(IceOstream &Str) const;
  // isRedundantAssign() is only used for dumping, so (for now) it's
  // OK that it's virtual.
  virtual bool isRedundantAssign(void) const { return false; }

  virtual ~IceInst() {}

protected:
  IceInst(IceCfg *Cfg, IceInstType Kind, unsigned MaxSrcs, IceVariable *Dest);
  void addSource(IceOperand *Src);
  void setLastUse(unsigned VarIndex) {
    if (VarIndex < 8 * sizeof(LiveRangesEnded))
      LiveRangesEnded |= (1u << VarIndex);
  }
  void resetLastUses(void) { LiveRangesEnded = 0; }

  const IceInstType Kind;
  const unsigned MaxSrcs; // only used for assert
  unsigned NumSrcs;
  int Number; // the instruction number for describing live ranges
  // Deleted means irrevocably deleted.
  bool Deleted;
  // Dead means pending deletion after liveness analysis converges.
  bool Dead;
  // TODO: use "IceVariable *const Dest".  The problem is that
  // IceInstPhi::lower() modifies its Dest.
  IceVariable *Dest;
  IceOperand **Srcs;        // TODO: possibly delete[] in destructor
  uint32_t LiveRangesEnded; // only first 32 src operands tracked, sorry
};

IceOstream &operator<<(IceOstream &Str, const IceInst *I);

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
    Xor,
    OpKind_NUM
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
  IceCfgNode *getTargetTrue(void) const { return TargetTrue; }
  IceCfgNode *getTargetFalse(void) const { return TargetFalse; }
  virtual IceNodeList getTerminatorEdges(void) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Br; }

private:
  // Conditional branch
  IceInstBr(IceCfg *Cfg, IceOperand *Source, IceCfgNode *TargetTrue,
            IceCfgNode *TargetFalse);
  // Unconditional branch
  IceInstBr(IceCfg *Cfg, IceCfgNode *Target);

  IceCfgNode *const TargetFalse;
  IceCfgNode *const TargetTrue; // NULL if unconditional branch
};

class IceInstCall : public IceInst {
public:
  static IceInstCall *create(IceCfg *Cfg, unsigned NumArgs, IceVariable *Dest,
                             IceOperand *CallTarget, bool Tail) {
    return new (Cfg->allocateInst<IceInstCall>())
        IceInstCall(Cfg, NumArgs, Dest, CallTarget, Tail);
  }
  void addArg(IceOperand *Arg) { addSource(Arg); }
  IceOperand *getCallTarget(void) const { return getSrc(0); }
  IceOperand *getArg(unsigned I) const { return getSrc(I + 1); }
  unsigned getNumArgs(void) const { return getSrcSize() - 1; }
  bool isTail(void) const { return Tail; }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Call; }

private:
  IceInstCall(IceCfg *Cfg, unsigned NumArgs, IceVariable *Dest,
              IceOperand *CallTarget, bool Tail)
      : IceInst(Cfg, IceInst::Call, NumArgs + 1, Dest), Tail(Tail) {
    addSource(CallTarget);
  }
  const bool Tail;
};

class IceInstCast : public IceInst {
public:
  enum IceCastKind {
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
    Ptrtoint,
    Inttoptr,
    Bitcast
  };
  static IceInstCast *create(IceCfg *Cfg, IceCastKind CastKind,
                             IceVariable *Dest, IceOperand *Source) {
    return new (Cfg->allocateInst<IceInstCast>())
        IceInstCast(Cfg, CastKind, Dest, Source);
  }
  IceCastKind getCastKind() const { return CastKind; }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Cast; }

private:
  IceInstCast(IceCfg *Cfg, IceCastKind CastKind, IceVariable *Dest,
              IceOperand *Source);
  IceCastKind CastKind;
};

// TODO: implement
class IceInstFcmp : public IceInst {
public:
  enum IceFCond {
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
  static IceInstFcmp *create(IceCfg *Cfg, IceFCond Condition, IceType Type,
                             IceOperand *Dest, IceOperand *Source1,
                             IceOperand *Source2) {
    return new (Cfg->allocateInst<IceInstFcmp>())
        IceInstFcmp(Cfg, Condition, Type, Dest, Source1, Source2);
  }
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Fcmp; }

private:
  IceInstFcmp(IceCfg *Cfg, IceFCond Condition, IceType Type, IceOperand *Dest,
              IceOperand *Source1, IceOperand *Source2);
  IceFCond Condition;
};

class IceInstIcmp : public IceInst {
public:
  enum IceICond {
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
    Sle,
    None // not part of LLVM; used for unconditional branch
  };
  static IceInstIcmp *create(IceCfg *Cfg, IceICond Condition, IceVariable *Dest,
                             IceOperand *Source1, IceOperand *Source2) {
    return new (Cfg->allocateInst<IceInstIcmp>())
        IceInstIcmp(Cfg, Condition, Dest, Source1, Source2);
  }
  IceICond getCondition(void) const { return Condition; }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Icmp; }

private:
  IceInstIcmp(IceCfg *Cfg, IceICond Condition, IceVariable *Dest,
              IceOperand *Source1, IceOperand *Source2);
  IceICond Condition;
};

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

class IceInstPhi : public IceInst {
public:
  static IceInstPhi *create(IceCfg *Cfg, unsigned MaxSrcs, IceVariable *Dest) {
    return new (Cfg->allocateInst<IceInstPhi>()) IceInstPhi(Cfg, MaxSrcs, Dest);
  }
  void addArgument(IceOperand *Source, IceCfgNode *Label);
  IceOperand *getArgument(IceCfgNode *Label) const;
  IceInst *lower(IceCfg *Cfg, IceCfgNode *Node);
  IceOperand *getOperandForTarget(IceCfgNode *Target) const;
  void livenessPhiOperand(llvm::BitVector &Live, IceCfgNode *Target,
                          IceLiveness *Liveness);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Phi; }

private:
  IceInstPhi(IceCfg *Cfg, unsigned MaxSrcs, IceVariable *Dest);
  IceNodeList Labels; // corresponding to Srcs
};

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

// SourceData as Srcs[0] and SourceAddr as Srcs[1]
class IceInstStore : public IceInst {
public:
  static IceInstStore *create(IceCfg *Cfg, IceOperand *SourceData,
                              IceOperand *SourceAddr) {
    return new (Cfg->allocateInst<IceInstStore>())
        IceInstStore(Cfg, SourceData, SourceAddr);
  }
  IceOperand *getAddr(void) const { return getSrc(1); }
  IceOperand *getData(void) const { return getSrc(0); }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Store; }

private:
  IceInstStore(IceCfg *Cfg, IceOperand *SourceData, IceOperand *SourceAddr);
};

class IceInstSwitch : public IceInst {
public:
  static IceInstSwitch *create(IceCfg *Cfg, unsigned NumCases,
                               IceOperand *Source, IceCfgNode *LabelDefault) {
    return new (Cfg->allocateInst<IceInstSwitch>())
        IceInstSwitch(Cfg, NumCases, Source, LabelDefault);
  }
  IceCfgNode *getLabelDefault(void) const { return LabelDefault; }
  unsigned getNumCases(void) const { return NumCases; }
  uint64_t getValue(unsigned I) const {
    assert(I < NumCases);
    return Values[I];
  }
  IceCfgNode *getLabel(unsigned I) const {
    assert(I < NumCases);
    return Labels[I];
  }
  void addBranch(unsigned CaseIndex, uint64_t Value, IceCfgNode *Label);
  virtual IceNodeList getTerminatorEdges(void) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Switch; }

private:
  IceInstSwitch(IceCfg *Cfg, unsigned NumCases, IceOperand *Source,
                IceCfgNode *LabelDefault);
  IceCfgNode *LabelDefault;
  unsigned NumCases;   // not including the default case
  uint64_t *Values;    // size is NumCases
  IceCfgNode **Labels; // size is NumCases
};

class IceInstFakeDef : public IceInst {
public:
  static IceInstFakeDef *create(IceCfg *Cfg, IceVariable *Dest,
                                IceVariable *Src = NULL) {
    return new (Cfg->allocateInst<IceInstFakeDef>())
        IceInstFakeDef(Cfg, Dest, Src);
  }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == FakeDef;
  }

private:
  IceInstFakeDef(IceCfg *Cfg, IceVariable *Dest, IceVariable *Src);
};

class IceInstFakeUse : public IceInst {
public:
  static IceInstFakeUse *create(IceCfg *Cfg, IceVariable *Src) {
    return new (Cfg->allocateInst<IceInstFakeUse>()) IceInstFakeUse(Cfg, Src);
  }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == FakeUse;
  }

private:
  IceInstFakeUse(IceCfg *Cfg, IceVariable *Src);
};

class IceInstFakeKill : public IceInst {
public:
  static IceInstFakeKill *create(IceCfg *Cfg, const IceVarList &KilledRegs,
                                 const IceInst *Linked) {
    return new (Cfg->allocateInst<IceInstFakeKill>())
        IceInstFakeKill(Cfg, KilledRegs, Linked);
  }
  const IceInst *getLinked(void) const { return Linked; }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == FakeKill;
  }

private:
  IceInstFakeKill(IceCfg *Cfg, const IceVarList &KilledRegs,
                  const IceInst *Linked);
  // This instruction is ignored if Linked->isDeleted() is true.
  const IceInst *Linked;
};

class IceInstTarget : public IceInst {
public:
  virtual void emit(IceOstream &Str, uint32_t Option) const = 0;
  virtual void dump(IceOstream &Str) const;
  virtual void dumpExtras(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() >= Target; }

protected:
  IceInstTarget(IceCfg *Cfg, IceInstType Kind, unsigned MaxSrcs,
                IceVariable *Dest)
      : IceInst(Cfg, Kind, MaxSrcs, Dest) {
    assert(Kind >= Target);
  }

private:
};

#endif // _IceInst_h
