//===- subzero/src/IceInst.h - High-level instructions ----------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the IceInst class and its target-independent
// subclasses, which represent the high-level Vanilla ICE instructions
// and map roughly 1:1 to LLVM instructions.
//
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_ICEINST_H
#define SUBZERO_ICEINST_H

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
    FakeDef,  // not part of LLVM/PNaCl bitcode
    FakeUse,  // not part of LLVM/PNaCl bitcode
    FakeKill, // not part of LLVM/PNaCl bitcode
    Target    // target-specific low-level ICE
              // Anything >= Target is an IceInstTarget subclass.
  };
  InstKind getKind() const { return Kind; }

  int32_t getNumber() const { return Number; }
  void renumber(IceCfg *Cfg);

  bool isDeleted() const { return Deleted; }
  void setDeleted() { Deleted = true; }
  void deleteIfDead();

  bool hasSideEffects() const { return HasSideEffects; }

  IceVariable *getDest() const { return Dest; }

  uint32_t getSrcSize() const { return NumSrcs; }
  IceOperand *getSrc(uint32_t I) const {
    assert(I < getSrcSize());
    return Srcs[I];
  }

  bool isLastUse(const IceOperand *Src) const;

  // Returns a list of out-edges corresponding to a terminator
  // instruction, which is the last instruction of the block.
  virtual IceNodeList getTerminatorEdges() const {
    assert(0);
    return IceNodeList();
  }

  // Updates the status of the IceVariables contained within the
  // instruction.  In particular, it marks where the Dest variable is
  // first assigned, and it tracks whether variables are live across
  // basic blocks, i.e. used in a different block from their definition.
  void updateVars(IceCfgNode *Node);

  void liveness(IceLivenessMode Mode, int32_t InstNumber, llvm::BitVector &Live,
                IceLiveness *Liveness, const IceCfgNode *Node);
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;
  virtual void dumpExtras(IceOstream &Str) const;
  void dumpSources(IceOstream &Str) const;
  void dumpDest(IceOstream &Str) const;
  virtual bool isRedundantAssign() const { return false; }

  virtual ~IceInst() {}

protected:
  IceInst(IceCfg *Cfg, InstKind Kind, uint32_t MaxSrcs, IceVariable *Dest);
  void addSource(IceOperand *Src) {
    assert(Src);
    assert(NumSrcs < MaxSrcs);
    Srcs[NumSrcs++] = Src;
  }
  void setLastUse(uint32_t VarIndex) {
    if (VarIndex < 8 * sizeof(LiveRangesEnded))
      LiveRangesEnded |= (1u << VarIndex);
  }
  void resetLastUses() { LiveRangesEnded = 0; }

  const InstKind Kind;
  // Number is the instruction number for describing live ranges.
  int32_t Number;
  // Deleted means irrevocably deleted.
  bool Deleted;
  // Dead means pending deletion after liveness analysis converges.
  bool Dead;
  // HasSideEffects means the instruction is something like a function
  // call or a volatile load that can't be removed even if its Dest
  // variable is not live.
  bool HasSideEffects;

  IceVariable *Dest;
  const uint32_t MaxSrcs; // only used for assert
  uint32_t NumSrcs;
  IceOperand **Srcs;

  uint32_t LiveRangesEnded; // only first 32 src operands tracked, sorry
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
    Xor,
    OpKind_NUM
  };
  static IceInstArithmetic *create(IceCfg *Cfg, OpKind Op, IceVariable *Dest,
                                   IceOperand *Source1, IceOperand *Source2) {
    return new (Cfg->allocateInst<IceInstArithmetic>())
        IceInstArithmetic(Cfg, Op, Dest, Source1, Source2);
  }
  OpKind getOp() const { return Op; }
  bool isCommutative() const;
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
  bool isUnconditional() const { return getTargetTrue() == NULL; }
  IceCfgNode *getTargetTrue() const { return TargetTrue; }
  IceCfgNode *getTargetFalse() const { return TargetFalse; }
  IceCfgNode *getTargetUnconditional() const {
    assert(isUnconditional());
    return getTargetFalse();
  }
  virtual IceNodeList getTerminatorEdges() const;
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
  IceOperand *getCallTarget() const { return getSrc(0); }
  IceOperand *getArg(uint32_t I) const { return getSrc(I + 1); }
  uint32_t getNumArgs() const { return getSrcSize() - 1; }
  bool isTail() const { return Tail; }
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
    Ptrtoint,
    Inttoptr,
    Bitcast
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
  FCond getCondition() const { return Condition; }
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
    Sle,
    None // not part of LLVM; used for unconditional branch
  };
  static IceInstIcmp *create(IceCfg *Cfg, ICond Condition, IceVariable *Dest,
                             IceOperand *Source1, IceOperand *Source2) {
    return new (Cfg->allocateInst<IceInstIcmp>())
        IceInstIcmp(Cfg, Condition, Dest, Source1, Source2);
  }
  ICond getCondition() const { return Condition; }
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
  IceOperand *getOperandForTarget(IceCfgNode *Target) const;
  void livenessPhiOperand(llvm::BitVector &Live, IceCfgNode *Target,
                          IceLiveness *Liveness);
  IceInst *lower(IceCfg *Cfg, IceCfgNode *Node);
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
  virtual IceNodeList getTerminatorEdges() const { return IceNodeList(); }
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
  IceOperand *getCondition() const { return getSrc(0); }
  IceOperand *getTrueOperand() const { return getSrc(1); }
  IceOperand *getFalseOperand() const { return getSrc(2); }
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
  IceOperand *getAddr() const { return getSrc(1); }
  IceOperand *getData() const { return getSrc(0); }
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
  IceCfgNode *getLabelDefault() const { return LabelDefault; }
  uint32_t getNumCases() const { return NumCases; }
  uint64_t getValue(uint32_t I) const {
    assert(I < NumCases);
    return Values[I];
  }
  IceCfgNode *getLabel(uint32_t I) const {
    assert(I < NumCases);
    return Labels[I];
  }
  void addBranch(uint32_t CaseIndex, uint64_t Value, IceCfgNode *Label);
  virtual IceNodeList getTerminatorEdges() const;
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

// FakeDef instruction.  This creates a fake definition of a variable,
// which is how we represent the case when an instruction produces
// multiple results.  This doesn't happen with high-level ICE
// instructions, but might with lowered instructions.  For example,
// this would be a way to represent condition flags being modified by
// an instruction.
//
// It's generally useful to set the optional source operand to be the
// dest variable of the instruction that actually produces the FakeDef
// dest.  Otherwise, the original instruction could be dead-code
// eliminated if its dest operand is unused, and therefore the FakeDef
// dest wouldn't be properly initialized.
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

// FakeUse instruction.  This creates a fake use of a variable, to
// keep the instruction that produces that variable from being
// dead-code eliminated.  This is useful in a variety of lowering
// situations.  The FakeUse instruction has no dest, so it can itself
// never be dead-code eliminated.
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

// FakeKill instruction.  This "kills" a set of variables by adding a
// trivial live range at this instruction to each variable.  The
// primary use is to indicate that scratch registers are killed after
// a call, so that the register allocator won't assign a scratch
// register to a variable whose live range spans a call.
//
// The FakeKill instruction also holds a pointer to the instruction
// that kills the set of variables, so that if that linked instruction
// gets dead-code eliminated, the FakeKill instruction will as well.
class IceInstFakeKill : public IceInst {
public:
  static IceInstFakeKill *create(IceCfg *Cfg, const IceVarList &KilledRegs,
                                 const IceInst *Linked) {
    return new (Cfg->allocateInst<IceInstFakeKill>())
        IceInstFakeKill(Cfg, KilledRegs, Linked);
  }
  const IceInst *getLinked() const { return Linked; }
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

// The Target instruction is the base class for all target-specific
// instructions.
class IceInstTarget : public IceInst {
public:
  virtual void emit(IceOstream &Str, uint32_t Option) const = 0;
  virtual void dump(IceOstream &Str) const;
  virtual void dumpExtras(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() >= Target; }

protected:
  IceInstTarget(IceCfg *Cfg, InstKind Kind, uint32_t MaxSrcs, IceVariable *Dest)
      : IceInst(Cfg, Kind, MaxSrcs, Dest) {
    assert(Kind >= Target);
  }
};

#endif // SUBZERO_ICEINST_H
