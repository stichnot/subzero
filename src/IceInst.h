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
    // Arbitrary (alphabetical) order, except put Unreachable first.
    Unreachable,
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
              // Anything >= Target is an InstTarget subclass.
  };
  InstKind getKind() const { return Kind; }

  int32_t getNumber() const { return Number; }
  void renumber(Cfg *Func);

  bool isDeleted() const { return Deleted; }
  void setDeleted() { Deleted = true; }
  void deleteIfDead();

  bool hasSideEffects() const { return HasSideEffects; }

  Variable *getDest() const { return Dest; }

  IceSize_t getSrcSize() const { return NumSrcs; }
  Operand *getSrc(IceSize_t I) const {
    assert(I < getSrcSize());
    return Srcs[I];
  }

  bool isLastUse(const Operand *Src) const;

  // Returns a list of out-edges corresponding to a terminator
  // instruction, which is the last instruction of the block.
  virtual NodeList getTerminatorEdges() const {
    // All valid terminator instructions override this method.  For
    // the default implementation, we assert in case some CfgNode
    // is constructed without a terminator instruction at the end.
    assert(0);
    return NodeList();
  }

  // Updates the status of the Variables contained within the
  // instruction.  In particular, it marks where the Dest variable is
  // first assigned, and it tracks whether variables are live across
  // basic blocks, i.e. used in a different block from their definition.
  void updateVars(CfgNode *Node);

  void liveness(LivenessMode Mode, int32_t InstNumber, llvm::BitVector &Live,
                Liveness *Liveness, const CfgNode *Node);
  virtual void emit(const Cfg *Func, uint32_t Option) const;
  virtual void dump(const Cfg *Func) const;
  virtual void dumpExtras(const Cfg *Func) const;
  void dumpDecorated(const Cfg *Func) const;
  void emitSources(const Cfg *Func, uint32_t Option) const;
  void dumpSources(const Cfg *Func) const;
  void dumpDest(const Cfg *Func) const;
  virtual bool isRedundantAssign() const { return false; }

  virtual ~Inst();

protected:
  Inst(Cfg *Func, InstKind Kind, IceSize_t MaxSrcs, Variable *Dest);
  void addSource(Operand *Src) {
    assert(Src);
    assert(NumSrcs < MaxSrcs);
    Srcs[NumSrcs++] = Src;
  }
  void setLastUse(IceSize_t VarIndex) {
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

  Variable *Dest;
  const IceSize_t MaxSrcs; // only used for assert
  IceSize_t NumSrcs;
  Operand **Srcs;

  uint32_t LiveRangesEnded; // only first 32 src operands tracked, sorry

private:
  Inst(const Inst &) LLVM_DELETED_FUNCTION;
  Inst &operator=(const Inst &) LLVM_DELETED_FUNCTION;
};

// Alloca instruction.  This captures the size in bytes as getSrc(0),
// and the alignment.
class InstAlloca : public Inst {
public:
  static InstAlloca *create(Cfg *Func, Operand *ByteCount, uint32_t Align,
                            Variable *Dest) {
    return new (Func->allocateInst<InstAlloca>())
        InstAlloca(Func, ByteCount, Align, Dest);
  }
  uint32_t getAlign() const { return Align; }
  Operand *getSizeInBytes() const { return getSrc(0); }
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Alloca; }

private:
  InstAlloca(Cfg *Func, Operand *ByteCount, uint32_t Align, Variable *Dest);
  InstAlloca(const InstAlloca &) LLVM_DELETED_FUNCTION;
  InstAlloca &operator=(const InstAlloca &) LLVM_DELETED_FUNCTION;
  virtual ~InstAlloca() {}
  const uint32_t Align;
};

#define ICEINSTARITHMETIC_TABLE                                                \
  /* enum value, printable string, commutative */                              \
  X(Add, "add", true) X(Fadd, "fadd", false) X(Sub, "sub", false)              \
      X(Fsub, "fsub", false) X(Mul, "mul", true) X(Fmul, "fmul", false)        \
      X(Udiv, "udiv", false) X(Sdiv, "sdiv", false) X(Fdiv, "fdiv", false)     \
      X(Urem, "urem", false) X(Srem, "srem", false) X(Frem, "frem", false)     \
      X(Shl, "shl", false) X(Lshr, "lshr", false) X(Ashr, "ashr", false)       \
      X(And, "and", true) X(Or, "or", true) X(Xor, "xor", true)

// Binary arithmetic instruction.  The source operands are captured in
// getSrc(0) and getSrc(1).
class InstArithmetic : public Inst {
public:
#define X(tag, str, commutative) tag,

  enum OpKind {
    ICEINSTARITHMETIC_TABLE OpKind_NUM
  };
#undef X

  static InstArithmetic *create(Cfg *Func, OpKind Op, Variable *Dest,
                                Operand *Source1, Operand *Source2) {
    return new (Func->allocateInst<InstArithmetic>())
        InstArithmetic(Func, Op, Dest, Source1, Source2);
  }
  OpKind getOp() const { return Op; }
  bool isCommutative() const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) {
    return Inst->getKind() == Arithmetic;
  }

private:
  InstArithmetic(Cfg *Func, OpKind Op, Variable *Dest, Operand *Source1,
                 Operand *Source2);
  InstArithmetic(const InstArithmetic &) LLVM_DELETED_FUNCTION;
  InstArithmetic &operator=(const InstArithmetic &) LLVM_DELETED_FUNCTION;
  virtual ~InstArithmetic() {}

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
  static InstAssign *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocateInst<InstAssign>())
        InstAssign(Func, Dest, Source);
  }
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Assign; }

private:
  InstAssign(Cfg *Func, Variable *Dest, Operand *Source);
  InstAssign(const InstAssign &) LLVM_DELETED_FUNCTION;
  InstAssign &operator=(const InstAssign &) LLVM_DELETED_FUNCTION;
  virtual ~InstAssign() {}
};

// Branch instruction.  This represents both conditional and
// unconditional branches.
class InstBr : public Inst {
public:
  // Create a conditional branch.  If TargetTrue==TargetFalse, it is
  // optimized to an unconditional branch.
  static InstBr *create(Cfg *Func, Operand *Source, CfgNode *TargetTrue,
                        CfgNode *TargetFalse) {
    return new (Func->allocateInst<InstBr>())
        InstBr(Func, Source, TargetTrue, TargetFalse);
  }
  static InstBr *create(Cfg *Func, CfgNode *Target) {
    return new (Func->allocateInst<InstBr>()) InstBr(Func, Target);
  }
  bool isUnconditional() const { return getTargetTrue() == NULL; }
  Operand *getCondition() const {
    assert(!isUnconditional());
    return getSrc(0);
  }
  CfgNode *getTargetTrue() const { return TargetTrue; }
  CfgNode *getTargetFalse() const { return TargetFalse; }
  CfgNode *getTargetUnconditional() const {
    assert(isUnconditional());
    return getTargetFalse();
  }
  virtual NodeList getTerminatorEdges() const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Br; }

private:
  // Conditional branch
  InstBr(Cfg *Func, Operand *Source, CfgNode *TargetTrue,
         CfgNode *TargetFalse);
  // Unconditional branch
  InstBr(Cfg *Func, CfgNode *Target);
  InstBr(const InstBr &) LLVM_DELETED_FUNCTION;
  InstBr &operator=(const InstBr &) LLVM_DELETED_FUNCTION;
  virtual ~InstBr() {}

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
  static InstCall *create(Cfg *Func, IceSize_t NumArgs, Variable *Dest,
                          Operand *CallTarget, bool Tail) {
    return new (Func->allocateInst<InstCall>())
        InstCall(Func, NumArgs, Dest, CallTarget, Tail);
  }
  void addArg(Operand *Arg) { addSource(Arg); }
  Operand *getCallTarget() const { return getSrc(0); }
  Operand *getArg(IceSize_t I) const { return getSrc(I + 1); }
  IceSize_t getNumArgs() const { return getSrcSize() - 1; }
  bool isTail() const { return Tail; }
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Call; }

private:
  InstCall(Cfg *Func, IceSize_t NumArgs, Variable *Dest, Operand *CallTarget,
           bool Tail)
      : Inst(Func, Inst::Call, NumArgs + 1, Dest), Tail(Tail) {
    // Set HasSideEffects so that the call instruction can't be
    // dead-code eliminated.  Don't set this for a deletable intrinsic
    // call.
    HasSideEffects = true;
    addSource(CallTarget);
  }
  InstCall(const InstCall &) LLVM_DELETED_FUNCTION;
  InstCall &operator=(const InstCall &) LLVM_DELETED_FUNCTION;
  virtual ~InstCall() {}
  const bool Tail;
};

#define ICEINSTCAST_TABLE                                                      \
  /* enum value, printable string */                                           \
  X(Trunc, "trunc") X(Zext, "zext") X(Sext, "sext") X(Fptrunc, "fptrunc")      \
      X(Fpext, "fpext") X(Fptoui, "fptoui") X(Fptosi, "fptosi")                \
      X(Uitofp, "uitofp") X(Sitofp, "sitofp") X(Bitcast, "bitcast")

// Cast instruction (a.k.a. conversion operation).
class InstCast : public Inst {
public:
#define X(tag, str) tag,

  enum OpKind {
    ICEINSTCAST_TABLE
  };
#undef X

  static InstCast *create(Cfg *Func, OpKind CastKind, Variable *Dest,
                          Operand *Source) {
    return new (Func->allocateInst<InstCast>())
        InstCast(Func, CastKind, Dest, Source);
  }
  OpKind getCastKind() const { return CastKind; }
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Cast; }

private:
  InstCast(Cfg *Func, OpKind CastKind, Variable *Dest, Operand *Source);
  InstCast(const InstCast &) LLVM_DELETED_FUNCTION;
  InstCast &operator=(const InstCast &) LLVM_DELETED_FUNCTION;
  virtual ~InstCast() {}
  const OpKind CastKind;
};

#define ICEINSTFCMP_TABLE                                                      \
  /* enum value, printable string */                                           \
  X(False, "false") X(Oeq, "oeq") X(Ogt, "ogt") X(Oge, "oge") X(Olt, "olt")    \
      X(Ole, "ole") X(One, "one") X(Ord, "ord") X(Ueq, "ueq") X(Ugt, "ugt")    \
      X(Uge, "uge") X(Ult, "ult") X(Ule, "ule") X(Une, "une") X(Uno, "uno")    \
      X(True, "true")

// Floating-point comparison instruction.  The source operands are
// captured in getSrc(0) and getSrc(1).
class InstFcmp : public Inst {
public:
#define X(tag, str) tag,

  enum FCond {
    ICEINSTFCMP_TABLE
  };
#undef X

  static InstFcmp *create(Cfg *Func, FCond Condition, Variable *Dest,
                          Operand *Source1, Operand *Source2) {
    return new (Func->allocateInst<InstFcmp>())
        InstFcmp(Func, Condition, Dest, Source1, Source2);
  }
  FCond getCondition() const { return Condition; }
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Fcmp; }

private:
  InstFcmp(Cfg *Func, FCond Condition, Variable *Dest, Operand *Source1,
           Operand *Source2);
  InstFcmp(const InstFcmp &) LLVM_DELETED_FUNCTION;
  InstFcmp &operator=(const InstFcmp &) LLVM_DELETED_FUNCTION;
  virtual ~InstFcmp() {}
  const FCond Condition;
};

#define ICEINSTICMP_TABLE                                                      \
  /* enum value, printable string */                                           \
  X(Eq, "eq") X(Ne, "ne") X(Ugt, "ugt") X(Uge, "uge") X(Ult, "ult")            \
      X(Ule, "ule") X(Sgt, "sgt") X(Sge, "sge") X(Slt, "slt") X(Sle, "sle")

// Integer comparison instruction.  The source operands are captured
// in getSrc(0) and getSrc(1).
class InstIcmp : public Inst {
public:
#define X(tag, str) tag,

  enum ICond {
    ICEINSTICMP_TABLE None // not part of LLVM; used for unconditional branch
  };
#undef X

  static InstIcmp *create(Cfg *Func, ICond Condition, Variable *Dest,
                          Operand *Source1, Operand *Source2) {
    return new (Func->allocateInst<InstIcmp>())
        InstIcmp(Func, Condition, Dest, Source1, Source2);
  }
  ICond getCondition() const { return Condition; }
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Icmp; }

private:
  InstIcmp(Cfg *Func, ICond Condition, Variable *Dest, Operand *Source1,
           Operand *Source2);
  InstIcmp(const InstIcmp &) LLVM_DELETED_FUNCTION;
  InstIcmp &operator=(const InstIcmp &) LLVM_DELETED_FUNCTION;
  virtual ~InstIcmp() {}
  const ICond Condition;
};

// Load instruction.  The source address is captured in getSrc(0);
class InstLoad : public Inst {
public:
  static InstLoad *create(Cfg *Func, Variable *Dest, Operand *SourceAddr) {
    return new (Func->allocateInst<InstLoad>())
        InstLoad(Func, Dest, SourceAddr);
  }
  Operand *getSourceAddress() const { return getSrc(0); }
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Load; }

private:
  InstLoad(Cfg *Func, Variable *Dest, Operand *SourceAddr);
  InstLoad(const InstLoad &) LLVM_DELETED_FUNCTION;
  InstLoad &operator=(const InstLoad &) LLVM_DELETED_FUNCTION;
  virtual ~InstLoad() {}
};

// Phi instruction.  For incoming edge I, the node is Labels[I] and
// the Phi source operand is getSrc(I).
class InstPhi : public Inst {
public:
  static InstPhi *create(Cfg *Func, IceSize_t MaxSrcs, Variable *Dest) {
    return new (Func->allocateInst<InstPhi>()) InstPhi(Func, MaxSrcs, Dest);
  }
  void addArgument(Operand *Source, CfgNode *Label);
  Operand *getOperandForTarget(CfgNode *Target) const;
  void livenessPhiOperand(llvm::BitVector &Live, CfgNode *Target,
                          Liveness *Liveness);
  Inst *lower(Cfg *Func, CfgNode *Node);
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Phi; }

private:
  InstPhi(Cfg *Func, IceSize_t MaxSrcs, Variable *Dest);
  InstPhi(const InstPhi &) LLVM_DELETED_FUNCTION;
  InstPhi &operator=(const InstPhi &) LLVM_DELETED_FUNCTION;
  virtual ~InstPhi() {
#ifdef ICE_NO_ARENAS
    delete[] Labels;
#endif // ICE_NO_ARENAS
  }

  // Labels[] duplicates the InEdges[] information in the enclosing
  // CfgNode, but the Phi instruction is created before InEdges[]
  // is available, so it's more complicated to share the list.
  CfgNode **Labels;
};

// Ret instruction.  The return value is captured in getSrc(0), but if
// there is no return value (void-type function), then
// getSrcSize()==0 and hasRetValue()==false.
class InstRet : public Inst {
public:
  static InstRet *create(Cfg *Func, Operand *Source = NULL) {
    return new (Func->allocateInst<InstRet>()) InstRet(Func, Source);
  }
  bool hasRetValue() const { return getSrcSize(); }
  Operand *getRetValue() const {
    assert(hasRetValue());
    return getSrc(0);
  }
  virtual NodeList getTerminatorEdges() const { return NodeList(); }
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Ret; }

private:
  InstRet(Cfg *Func, Operand *Source);
  InstRet(const InstRet &) LLVM_DELETED_FUNCTION;
  InstRet &operator=(const InstRet &) LLVM_DELETED_FUNCTION;
  virtual ~InstRet() {}
};

// Select instruction.  The condition, true, and false operands are captured.
class InstSelect : public Inst {
public:
  static InstSelect *create(Cfg *Func, Variable *Dest, Operand *Condition,
                            Operand *SourceTrue, Operand *SourceFalse) {
    return new (Func->allocateInst<InstSelect>())
        InstSelect(Func, Dest, Condition, SourceTrue, SourceFalse);
  }
  Operand *getCondition() const { return getSrc(0); }
  Operand *getTrueOperand() const { return getSrc(1); }
  Operand *getFalseOperand() const { return getSrc(2); }
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Select; }

private:
  InstSelect(Cfg *Func, Variable *Dest, Operand *Condition, Operand *Source1,
             Operand *Source2);
  InstSelect(const InstSelect &) LLVM_DELETED_FUNCTION;
  InstSelect &operator=(const InstSelect &) LLVM_DELETED_FUNCTION;
  virtual ~InstSelect() {}
};

// Store instruction.  The address operand is captured, along with the
// data operand to be stored into the address.
class InstStore : public Inst {
public:
  static InstStore *create(Cfg *Func, Operand *Data, Operand *Addr) {
    return new (Func->allocateInst<InstStore>()) InstStore(Func, Data, Addr);
  }
  Operand *getAddr() const { return getSrc(1); }
  Operand *getData() const { return getSrc(0); }
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Store; }

private:
  InstStore(Cfg *Func, Operand *Data, Operand *Addr);
  InstStore(const InstStore &) LLVM_DELETED_FUNCTION;
  InstStore &operator=(const InstStore &) LLVM_DELETED_FUNCTION;
  virtual ~InstStore() {}
};

// Switch instruction.  The single source operand is captured as
// getSrc(0).
class InstSwitch : public Inst {
public:
  static InstSwitch *create(Cfg *Func, IceSize_t NumCases, Operand *Source,
                            CfgNode *LabelDefault) {
    return new (Func->allocateInst<InstSwitch>())
        InstSwitch(Func, NumCases, Source, LabelDefault);
  }
  Operand *getComparison() const { return getSrc(0); }
  CfgNode *getLabelDefault() const { return LabelDefault; }
  IceSize_t getNumCases() const { return NumCases; }
  uint64_t getValue(IceSize_t I) const {
    assert(I < NumCases);
    return Values[I];
  }
  CfgNode *getLabel(IceSize_t I) const {
    assert(I < NumCases);
    return Labels[I];
  }
  void addBranch(IceSize_t CaseIndex, uint64_t Value, CfgNode *Label);
  virtual NodeList getTerminatorEdges() const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == Switch; }

private:
  InstSwitch(Cfg *Func, IceSize_t NumCases, Operand *Source,
             CfgNode *LabelDefault);
  InstSwitch(const InstSwitch &) LLVM_DELETED_FUNCTION;
  InstSwitch &operator=(const InstSwitch &) LLVM_DELETED_FUNCTION;
  virtual ~InstSwitch() {
#ifdef ICE_NO_ARENAS
    delete[] Values;
    delete[] Labels;
#endif // ICE_NO_ARENAS
  }

  CfgNode *LabelDefault;
  IceSize_t NumCases; // not including the default case
  uint64_t *Values;   // size is NumCases
  CfgNode **Labels;   // size is NumCases
};

// Unreachable instruction.  This is a terminator instruction with no
// operands.
class InstUnreachable : public Inst {
public:
  static InstUnreachable *create(Cfg *Func) {
    return new (Func->allocateInst<InstUnreachable>()) InstUnreachable(Func);
  }
  virtual NodeList getTerminatorEdges() const { return NodeList(); }
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) {
    return Inst->getKind() == Unreachable;
  }

private:
  InstUnreachable(Cfg *Func);
  InstUnreachable(const InstUnreachable &) LLVM_DELETED_FUNCTION;
  InstUnreachable &operator=(const InstUnreachable &) LLVM_DELETED_FUNCTION;
  virtual ~InstUnreachable() {}
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
  static InstFakeDef *create(Cfg *Func, Variable *Dest,
                             Variable *Src = NULL) {
    return new (Func->allocateInst<InstFakeDef>()) InstFakeDef(Func, Dest, Src);
  }
  virtual void emit(const Cfg *Func, uint32_t Option) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == FakeDef; }

private:
  InstFakeDef(Cfg *Func, Variable *Dest, Variable *Src);
  InstFakeDef(const InstFakeDef &) LLVM_DELETED_FUNCTION;
  InstFakeDef &operator=(const InstFakeDef &) LLVM_DELETED_FUNCTION;
  virtual ~InstFakeDef() {}
};

// FakeUse instruction.  This creates a fake use of a variable, to
// keep the instruction that produces that variable from being
// dead-code eliminated.  This is useful in a variety of lowering
// situations.  The FakeUse instruction has no dest, so it can itself
// never be dead-code eliminated.
class InstFakeUse : public Inst {
public:
  static InstFakeUse *create(Cfg *Func, Variable *Src) {
    return new (Func->allocateInst<InstFakeUse>()) InstFakeUse(Func, Src);
  }
  virtual void emit(const Cfg *Func, uint32_t Option) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == FakeUse; }

private:
  InstFakeUse(Cfg *Func, Variable *Src);
  InstFakeUse(const InstFakeUse &) LLVM_DELETED_FUNCTION;
  InstFakeUse &operator=(const InstFakeUse &) LLVM_DELETED_FUNCTION;
  virtual ~InstFakeUse() {}
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
  static InstFakeKill *create(Cfg *Func, const VarList &KilledRegs,
                              const Inst *Linked) {
    return new (Func->allocateInst<InstFakeKill>())
        InstFakeKill(Func, KilledRegs, Linked);
  }
  const Inst *getLinked() const { return Linked; }
  virtual void emit(const Cfg *Func, uint32_t Option) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() == FakeKill; }

private:
  InstFakeKill(Cfg *Func, const VarList &KilledRegs, const Inst *Linked);
  InstFakeKill(const InstFakeKill &) LLVM_DELETED_FUNCTION;
  InstFakeKill &operator=(const InstFakeKill &) LLVM_DELETED_FUNCTION;
  virtual ~InstFakeKill() {}

  // This instruction is ignored if Linked->isDeleted() is true.
  const Inst *Linked;
};

// The Target instruction is the base class for all target-specific
// instructions.
class InstTarget : public Inst {
public:
  virtual void emit(const Cfg *Func, uint32_t Option) const = 0;
  virtual void dump(const Cfg *Func) const;
  virtual void dumpExtras(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return Inst->getKind() >= Target; }

protected:
  InstTarget(Cfg *Func, InstKind Kind, IceSize_t MaxSrcs, Variable *Dest)
      : Inst(Func, Kind, MaxSrcs, Dest) {
    assert(Kind >= Target);
  }
  InstTarget(const InstTarget &) LLVM_DELETED_FUNCTION;
  InstTarget &operator=(const InstTarget &) LLVM_DELETED_FUNCTION;
  virtual ~InstTarget() {}
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEINST_H
