//===- subzero/src/Inst.h - High-level instructions ----------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Inst class and its target-independent
// subclasses, which represent the high-level Vanilla ICE instructions
// and map roughly 1:1 to LLVM instructions.
//
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEINST_H
#define SUBZERO_SRC_ICEINST_H

#include "IceDefs.h"
#include "IceTypes.h"

namespace Ice {

class Inst {
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
    Unreachable,
    FakeDef,  // not part of LLVM/PNaCl bitcode
    FakeUse,  // not part of LLVM/PNaCl bitcode
    FakeKill, // not part of LLVM/PNaCl bitcode
    Target    // target-specific low-level ICE
              // Anything >= Target is an InstTarget subclass.
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
    // All valid terminator instructions override this method.  For
    // the default implementation, we assert in case some CfgNode
    // is constructed without a terminator instruction at the end.
    assert(0);
    return IceNodeList();
  }

  // Updates the status of the IceVariables contained within the
  // instruction.  In particular, it marks where the Dest variable is
  // first assigned, and it tracks whether variables are live across
  // basic blocks, i.e. used in a different block from their definition.
  void updateVars(CfgNode *Node);

  void liveness(LivenessMode Mode, int32_t InstNumber, llvm::BitVector &Live,
                Liveness *Liveness, const CfgNode *Node);
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  virtual void dumpExtras(const IceCfg *Cfg) const;
  void dumpDecorated(const IceCfg *Cfg) const;
  void emitSources(const IceCfg *Cfg, uint32_t Option) const;
  void dumpSources(const IceCfg *Cfg) const;
  void dumpDest(const IceCfg *Cfg) const;
  virtual bool isRedundantAssign() const { return false; }

  virtual ~Inst() {}

protected:
  Inst(IceCfg *Cfg, InstKind Kind, uint32_t MaxSrcs, IceVariable *Dest);
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

private:
  Inst(const Inst &) LLVM_DELETED_FUNCTION;
  Inst &operator=(const Inst &) LLVM_DELETED_FUNCTION;
};

// Alloca instruction.  This captures the size in bytes as getSrc(0),
// and the alignment.
class InstAlloca : public Inst {
public:
  static InstAlloca *create(IceCfg *Cfg, IceOperand *ByteCount, uint32_t Align,
                            IceVariable *Dest) {
    return new (Cfg->allocateInst<InstAlloca>())
        InstAlloca(Cfg, ByteCount, Align, Dest);
  }
  uint32_t getAlign() const { return Align; }
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Alloca; }

private:
  InstAlloca(IceCfg *Cfg, IceOperand *ByteCount, uint32_t Align,
             IceVariable *Dest);
  InstAlloca(const InstAlloca &) LLVM_DELETED_FUNCTION;
  InstAlloca &operator=(const InstAlloca &) LLVM_DELETED_FUNCTION;
  const uint32_t Align;
};

// Binary arithmetic instruction.  The source operands are captured in
// getSrc(0) and getSrc(1).
class InstArithmetic : public Inst {
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
  static InstArithmetic *create(IceCfg *Cfg, OpKind Op, IceVariable *Dest,
                                IceOperand *Source1, IceOperand *Source2) {
    return new (Cfg->allocateInst<InstArithmetic>())
        InstArithmetic(Cfg, Op, Dest, Source1, Source2);
  }
  OpKind getOp() const { return Op; }
  bool isCommutative() const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) {
    return Inst->getKind() == Arithmetic;
  }

private:
  InstArithmetic(IceCfg *Cfg, OpKind Op, IceVariable *Dest, IceOperand *Source1,
                 IceOperand *Source2);
  InstArithmetic(const InstArithmetic &) LLVM_DELETED_FUNCTION;
  InstArithmetic &operator=(const InstArithmetic &) LLVM_DELETED_FUNCTION;

  const OpKind Op;
};

// Assignment instruction.  The source operand is captured in
// getSrc(0).  This is not part of the LLVM bitcode, but is a useful
// abstraction for some of the lowering.  E.g., if Phi instruction
// lowering happens before target lowering, or for representing an
// Inttoptr instruction, or as an intermediate step for lowering a
// Load instruction.
class InstAssign : public Inst {
public:
  static InstAssign *create(IceCfg *Cfg, IceVariable *Dest,
                            IceOperand *Source) {
    return new (Cfg->allocateInst<InstAssign>()) InstAssign(Cfg, Dest, Source);
  }
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Assign; }

private:
  InstAssign(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
  InstAssign(const InstAssign &) LLVM_DELETED_FUNCTION;
  InstAssign &operator=(const InstAssign &) LLVM_DELETED_FUNCTION;
};

// Branch instruction.  This represents both conditional and
// unconditional branches.
class InstBr : public Inst {
public:
  // Create a conditional branch.  If TargetTrue==TargetFalse, it is
  // optimized to an unconditional branch.
  static InstBr *create(IceCfg *Cfg, IceOperand *Source, CfgNode *TargetTrue,
                        CfgNode *TargetFalse) {
    return new (Cfg->allocateInst<InstBr>())
        InstBr(Cfg, Source, TargetTrue, TargetFalse);
  }
  static InstBr *create(IceCfg *Cfg, CfgNode *Target) {
    return new (Cfg->allocateInst<InstBr>()) InstBr(Cfg, Target);
  }
  bool isUnconditional() const { return getTargetTrue() == NULL; }
  CfgNode *getTargetTrue() const { return TargetTrue; }
  CfgNode *getTargetFalse() const { return TargetFalse; }
  CfgNode *getTargetUnconditional() const {
    assert(isUnconditional());
    return getTargetFalse();
  }
  virtual IceNodeList getTerminatorEdges() const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Br; }

private:
  // Conditional branch
  InstBr(IceCfg *Cfg, IceOperand *Source, CfgNode *TargetTrue,
         CfgNode *TargetFalse);
  // Unconditional branch
  InstBr(IceCfg *Cfg, CfgNode *Target);
  InstBr(const InstBr &) LLVM_DELETED_FUNCTION;
  InstBr &operator=(const InstBr &) LLVM_DELETED_FUNCTION;

  CfgNode *const TargetFalse; // Doubles as unconditional branch target
  CfgNode *const TargetTrue;  // NULL if unconditional branch
};

// Call instruction.  The call target is captured as getSrc(0), and
// arg I is captured as getSrc(I+1).
class InstCall : public Inst {
public:
  // The Tail argument represents the "tail" marker from the original
  // bitcode instruction (which doesn't necessarily mean that this
  // call must be executed as a tail call).
  static InstCall *create(IceCfg *Cfg, uint32_t NumArgs, IceVariable *Dest,
                          IceOperand *CallTarget, bool Tail) {
    return new (Cfg->allocateInst<InstCall>())
        InstCall(Cfg, NumArgs, Dest, CallTarget, Tail);
  }
  void addArg(IceOperand *Arg) { addSource(Arg); }
  IceOperand *getCallTarget() const { return getSrc(0); }
  IceOperand *getArg(uint32_t I) const { return getSrc(I + 1); }
  uint32_t getNumArgs() const { return getSrcSize() - 1; }
  bool isTail() const { return Tail; }
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Call; }

private:
  InstCall(IceCfg *Cfg, uint32_t NumArgs, IceVariable *Dest,
           IceOperand *CallTarget, bool Tail)
      : Inst(Cfg, Inst::Call, NumArgs + 1, Dest), Tail(Tail) {
    // Set HasSideEffects so that the call instruction can't be
    // dead-code eliminated.  Don't set this for a deletable intrinsic
    // call.
    HasSideEffects = true;
    addSource(CallTarget);
  }
  InstCall(const InstCall &) LLVM_DELETED_FUNCTION;
  InstCall &operator=(const InstCall &) LLVM_DELETED_FUNCTION;
  const bool Tail;
};

// Cast instruction (a.k.a. conversion operation).
class InstCast : public Inst {
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
  static InstCast *create(IceCfg *Cfg, OpKind CastKind, IceVariable *Dest,
                          IceOperand *Source) {
    return new (Cfg->allocateInst<InstCast>())
        InstCast(Cfg, CastKind, Dest, Source);
  }
  OpKind getCastKind() const { return CastKind; }
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Cast; }

private:
  InstCast(IceCfg *Cfg, OpKind CastKind, IceVariable *Dest, IceOperand *Source);
  InstCast(const InstCast &) LLVM_DELETED_FUNCTION;
  InstCast &operator=(const InstCast &) LLVM_DELETED_FUNCTION;
  OpKind CastKind;
};

// Floating-point comparison instruction.  The source operands are
// captured in getSrc(0) and getSrc(1).
class InstFcmp : public Inst {
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
  static InstFcmp *create(IceCfg *Cfg, FCond Condition, IceVariable *Dest,
                          IceOperand *Source1, IceOperand *Source2) {
    return new (Cfg->allocateInst<InstFcmp>())
        InstFcmp(Cfg, Condition, Dest, Source1, Source2);
  }
  FCond getCondition() const { return Condition; }
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Fcmp; }

private:
  InstFcmp(IceCfg *Cfg, FCond Condition, IceVariable *Dest, IceOperand *Source1,
           IceOperand *Source2);
  InstFcmp(const InstFcmp &) LLVM_DELETED_FUNCTION;
  InstFcmp &operator=(const InstFcmp &) LLVM_DELETED_FUNCTION;
  FCond Condition;
};

// Integer comparison instruction.  The source operands are captured
// in getSrc(0) and getSrc(1).
class InstIcmp : public Inst {
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
  static InstIcmp *create(IceCfg *Cfg, ICond Condition, IceVariable *Dest,
                          IceOperand *Source1, IceOperand *Source2) {
    return new (Cfg->allocateInst<InstIcmp>())
        InstIcmp(Cfg, Condition, Dest, Source1, Source2);
  }
  ICond getCondition() const { return Condition; }
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Icmp; }

private:
  InstIcmp(IceCfg *Cfg, ICond Condition, IceVariable *Dest, IceOperand *Source1,
           IceOperand *Source2);
  InstIcmp(const InstIcmp &) LLVM_DELETED_FUNCTION;
  InstIcmp &operator=(const InstIcmp &) LLVM_DELETED_FUNCTION;
  ICond Condition;
};

// Load instruction.  The source address is captured in getSrc(0);
class InstLoad : public Inst {
public:
  static InstLoad *create(IceCfg *Cfg, IceVariable *Dest,
                          IceOperand *SourceAddr) {
    return new (Cfg->allocateInst<InstLoad>()) InstLoad(Cfg, Dest, SourceAddr);
  }
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Load; }

private:
  InstLoad(IceCfg *Cfg, IceVariable *Dest, IceOperand *SourceAddr);
  InstLoad(const InstLoad &) LLVM_DELETED_FUNCTION;
  InstLoad &operator=(const InstLoad &) LLVM_DELETED_FUNCTION;
};

// Phi instruction.  For incoming edge I, the node is Labels[I] and
// the Phi source operand is getSrc(I).
class InstPhi : public Inst {
public:
  static InstPhi *create(IceCfg *Cfg, uint32_t MaxSrcs, IceVariable *Dest) {
    return new (Cfg->allocateInst<InstPhi>()) InstPhi(Cfg, MaxSrcs, Dest);
  }
  void addArgument(IceOperand *Source, CfgNode *Label);
  IceOperand *getOperandForTarget(CfgNode *Target) const;
  void livenessPhiOperand(llvm::BitVector &Live, CfgNode *Target,
                          Liveness *Liveness);
  Inst *lower(IceCfg *Cfg, CfgNode *Node);
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Phi; }

private:
  InstPhi(IceCfg *Cfg, uint32_t MaxSrcs, IceVariable *Dest);
  InstPhi(const InstPhi &) LLVM_DELETED_FUNCTION;
  InstPhi &operator=(const InstPhi &) LLVM_DELETED_FUNCTION;
  // Labels[] duplicates the InEdges[] information in the enclosing
  // CfgNode, but the Phi instruction is created before InEdges[]
  // is available, so it's more complicated to share the list.
  CfgNode **Labels;
};

// Ret instruction.  The return value is captured in getSrc(0), but if
// there is no return value (void-type function), then
// getSrcSize()==0.
class InstRet : public Inst {
public:
  static InstRet *create(IceCfg *Cfg, IceOperand *Source = NULL) {
    return new (Cfg->allocateInst<InstRet>()) InstRet(Cfg, Source);
  }
  virtual IceNodeList getTerminatorEdges() const { return IceNodeList(); }
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Ret; }

private:
  InstRet(IceCfg *Cfg, IceOperand *Source);
  InstRet(const InstRet &) LLVM_DELETED_FUNCTION;
  InstRet &operator=(const InstRet &) LLVM_DELETED_FUNCTION;
};

// Select instruction.  The condition, true, and false operands are captured.
class InstSelect : public Inst {
public:
  static InstSelect *create(IceCfg *Cfg, IceVariable *Dest,
                            IceOperand *Condition, IceOperand *SourceTrue,
                            IceOperand *SourceFalse) {
    return new (Cfg->allocateInst<InstSelect>())
        InstSelect(Cfg, Dest, Condition, SourceTrue, SourceFalse);
  }
  IceOperand *getCondition() const { return getSrc(0); }
  IceOperand *getTrueOperand() const { return getSrc(1); }
  IceOperand *getFalseOperand() const { return getSrc(2); }
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Select; }

private:
  InstSelect(IceCfg *Cfg, IceVariable *Dest, IceOperand *Condition,
             IceOperand *Source1, IceOperand *Source2);
  InstSelect(const InstSelect &) LLVM_DELETED_FUNCTION;
  InstSelect &operator=(const InstSelect &) LLVM_DELETED_FUNCTION;
};

// Store instruction.  The address operand is captured, along with the
// data operand to be stored into the address.
class InstStore : public Inst {
public:
  static InstStore *create(IceCfg *Cfg, IceOperand *Data, IceOperand *Addr) {
    return new (Cfg->allocateInst<InstStore>()) InstStore(Cfg, Data, Addr);
  }
  IceOperand *getAddr() const { return getSrc(1); }
  IceOperand *getData() const { return getSrc(0); }
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Store; }

private:
  InstStore(IceCfg *Cfg, IceOperand *Data, IceOperand *Addr);
  InstStore(const InstStore &) LLVM_DELETED_FUNCTION;
  InstStore &operator=(const InstStore &) LLVM_DELETED_FUNCTION;
};

// Switch instruction.  The single source operand is captured as
// getSrc(0).
class InstSwitch : public Inst {
public:
  static InstSwitch *create(IceCfg *Cfg, uint32_t NumCases, IceOperand *Source,
                            CfgNode *LabelDefault) {
    return new (Cfg->allocateInst<InstSwitch>())
        InstSwitch(Cfg, NumCases, Source, LabelDefault);
  }
  CfgNode *getLabelDefault() const { return LabelDefault; }
  uint32_t getNumCases() const { return NumCases; }
  uint64_t getValue(uint32_t I) const {
    assert(I < NumCases);
    return Values[I];
  }
  CfgNode *getLabel(uint32_t I) const {
    assert(I < NumCases);
    return Labels[I];
  }
  void addBranch(uint32_t CaseIndex, uint64_t Value, CfgNode *Label);
  virtual IceNodeList getTerminatorEdges() const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Switch; }

private:
  InstSwitch(IceCfg *Cfg, uint32_t NumCases, IceOperand *Source,
             CfgNode *LabelDefault);
  InstSwitch(const InstSwitch &) LLVM_DELETED_FUNCTION;
  InstSwitch &operator=(const InstSwitch &) LLVM_DELETED_FUNCTION;
  CfgNode *LabelDefault;
  uint32_t NumCases; // not including the default case
  uint64_t *Values;  // size is NumCases
  CfgNode **Labels;  // size is NumCases
};

// Unreachable instruction.  This is a terminator instruction with no
// operands.
class InstUnreachable : public Inst {
public:
  static InstUnreachable *create(IceCfg *Cfg) {
    return new (Cfg->allocateInst<InstUnreachable>()) InstUnreachable(Cfg);
  }
  virtual IceNodeList getTerminatorEdges() const { return IceNodeList(); }
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) {
    return Inst->getKind() == Unreachable;
  }

private:
  InstUnreachable(IceCfg *Cfg);
  InstUnreachable(const InstUnreachable &) LLVM_DELETED_FUNCTION;
  InstUnreachable &operator=(const InstUnreachable &) LLVM_DELETED_FUNCTION;
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
class InstFakeDef : public Inst {
public:
  static InstFakeDef *create(IceCfg *Cfg, IceVariable *Dest,
                             IceVariable *Src = NULL) {
    return new (Cfg->allocateInst<InstFakeDef>()) InstFakeDef(Cfg, Dest, Src);
  }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == FakeDef; }

private:
  InstFakeDef(IceCfg *Cfg, IceVariable *Dest, IceVariable *Src);
  InstFakeDef(const InstFakeDef &) LLVM_DELETED_FUNCTION;
  InstFakeDef &operator=(const InstFakeDef &) LLVM_DELETED_FUNCTION;
};

// FakeUse instruction.  This creates a fake use of a variable, to
// keep the instruction that produces that variable from being
// dead-code eliminated.  This is useful in a variety of lowering
// situations.  The FakeUse instruction has no dest, so it can itself
// never be dead-code eliminated.
class InstFakeUse : public Inst {
public:
  static InstFakeUse *create(IceCfg *Cfg, IceVariable *Src) {
    return new (Cfg->allocateInst<InstFakeUse>()) InstFakeUse(Cfg, Src);
  }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == FakeUse; }

private:
  InstFakeUse(IceCfg *Cfg, IceVariable *Src);
  InstFakeUse(const InstFakeUse &) LLVM_DELETED_FUNCTION;
  InstFakeUse &operator=(const InstFakeUse &) LLVM_DELETED_FUNCTION;
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
class InstFakeKill : public Inst {
public:
  static InstFakeKill *create(IceCfg *Cfg, const IceVarList &KilledRegs,
                              const Inst *Linked) {
    return new (Cfg->allocateInst<InstFakeKill>())
        InstFakeKill(Cfg, KilledRegs, Linked);
  }
  const Inst *getLinked() const { return Linked; }
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const;
  virtual void dump(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == FakeKill; }

private:
  InstFakeKill(IceCfg *Cfg, const IceVarList &KilledRegs, const Inst *Linked);
  InstFakeKill(const InstFakeKill &) LLVM_DELETED_FUNCTION;
  InstFakeKill &operator=(const InstFakeKill &) LLVM_DELETED_FUNCTION;
  // This instruction is ignored if Linked->isDeleted() is true.
  const Inst *Linked;
};

// The Target instruction is the base class for all target-specific
// instructions.
class InstTarget : public Inst {
public:
  virtual void emit(const IceCfg *Cfg, uint32_t Option) const = 0;
  virtual void dump(const IceCfg *Cfg) const;
  virtual void dumpExtras(const IceCfg *Cfg) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() >= Target; }

protected:
  InstTarget(IceCfg *Cfg, InstKind Kind, uint32_t MaxSrcs, IceVariable *Dest)
      : Inst(Cfg, Kind, MaxSrcs, Dest) {
    assert(Kind >= Target);
  }
  InstTarget(const InstTarget &) LLVM_DELETED_FUNCTION;
  InstTarget &operator=(const InstTarget &) LLVM_DELETED_FUNCTION;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEINST_H
