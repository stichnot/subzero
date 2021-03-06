//===- subzero/src/IceInstX8632.h - Low-level x86 instructions --*- C++ -*-===//
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
#include "IceInstX8632.def"
#include "IceOperand.h"

namespace Ice {

class TargetX8632;

// OperandX8632 extends the Operand hierarchy.  Its subclasses are
// OperandX8632Mem and VariableSplit.
class OperandX8632 : public Operand {
public:
  enum OperandKindX8632 {
    k__Start = Operand::kTarget,
    kMem,
    kSplit
  };
  virtual void emit(const Cfg *Func) const = 0;
  void dump(const Cfg *Func) const;

protected:
  OperandX8632(OperandKindX8632 Kind, Type Ty)
      : Operand(static_cast<OperandKind>(Kind), Ty) {}
  virtual ~OperandX8632() {}

private:
  OperandX8632(const OperandX8632 &) LLVM_DELETED_FUNCTION;
  OperandX8632 &operator=(const OperandX8632 &) LLVM_DELETED_FUNCTION;
};

// OperandX8632Mem represents the m32 addressing mode, with optional
// base and index registers, a constant offset, and a fixed shift
// value for the index register.
class OperandX8632Mem : public OperandX8632 {
public:
  static OperandX8632Mem *create(Cfg *Func, Type Ty, Variable *Base,
                                 Constant *Offset, Variable *Index = NULL,
                                 uint32_t Shift = 0) {
    return new (Func->allocate<OperandX8632Mem>())
        OperandX8632Mem(Func, Ty, Base, Offset, Index, Shift);
  }
  Variable *getBase() const { return Base; }
  Constant *getOffset() const { return Offset; }
  Variable *getIndex() const { return Index; }
  uint32_t getShift() const { return Shift; }
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;

  static bool classof(const Operand *Operand) {
    return Operand->getKind() == static_cast<OperandKind>(kMem);
  }

private:
  OperandX8632Mem(Cfg *Func, Type Ty, Variable *Base, Constant *Offset,
                  Variable *Index, uint32_t Shift);
  OperandX8632Mem(const OperandX8632Mem &) LLVM_DELETED_FUNCTION;
  OperandX8632Mem &operator=(const OperandX8632Mem &) LLVM_DELETED_FUNCTION;
  virtual ~OperandX8632Mem() {}
  Variable *Base;
  Constant *Offset;
  Variable *Index;
  uint32_t Shift;
};

// VariableSplit is a way to treat an f64 memory location as a pair
// of i32 locations (Low and High).  This is needed for some cases
// of the Bitcast instruction.  Since it's not possible for integer
// registers to access the XMM registers and vice versa, the
// lowering forces the f64 to be spilled to the stack and then
// accesses through the VariableSplit.
class VariableSplit : public OperandX8632 {
public:
  enum Portion {
    Low,
    High
  };
  static VariableSplit *create(Cfg *Func, Variable *Var, Portion Part) {
    return new (Func->allocate<VariableSplit>()) VariableSplit(Func, Var, Part);
  }
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;

  static bool classof(const Operand *Operand) {
    return Operand->getKind() == static_cast<OperandKind>(kSplit);
  }

private:
  VariableSplit(Cfg *Func, Variable *Var, Portion Part)
      : OperandX8632(kSplit, IceType_i32), Func(Func), Var(Var), Part(Part) {
    assert(Var->getType() == IceType_f64);
    Vars = Func->allocateArrayOf<Variable *>(1);
    Vars[0] = Var;
    NumVars = 1;
  }
  VariableSplit(const VariableSplit &) LLVM_DELETED_FUNCTION;
  VariableSplit &operator=(const VariableSplit &) LLVM_DELETED_FUNCTION;
  virtual ~VariableSplit() { Func->deallocateArrayOf<Variable *>(Vars); }
  Cfg *Func; // Held only for the destructor.
  Variable *Var;
  Portion Part;
};

class InstX8632 : public InstTarget {
public:
  enum InstKindX8632 {
    k__Start = Inst::Target,
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
  static const char *getWidthString(Type Ty);
  virtual void emit(const Cfg *Func) const = 0;
  virtual void dump(const Cfg *Func) const;

protected:
  InstX8632(Cfg *Func, InstKindX8632 Kind, SizeT Maxsrcs, Variable *Dest)
      : InstTarget(Func, static_cast<InstKind>(Kind), Maxsrcs, Dest) {}
  virtual ~InstX8632() {}
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
// Labels L1 and L2 are intra-block labels.  Without knowledge of the
// intra-block control flow, liveness analysis will determine the "mov
// c, x" instruction to be dead.  One way to prevent this is to insert
// a "FakeUse(c)" instruction anywhere between the two "mov c, ..."
// instructions, e.g.:
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
// it may be prevented by running dead code elimination before
// lowering.
class InstX8632Label : public InstX8632 {
public:
  static InstX8632Label *create(Cfg *Func, TargetX8632 *Target) {
    return new (Func->allocate<InstX8632Label>()) InstX8632Label(Func, Target);
  }
  IceString getName(const Cfg *Func) const;
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;

private:
  InstX8632Label(Cfg *Func, TargetX8632 *Target);
  InstX8632Label(const InstX8632Label &) LLVM_DELETED_FUNCTION;
  InstX8632Label &operator=(const InstX8632Label &) LLVM_DELETED_FUNCTION;
  virtual ~InstX8632Label() {}
  SizeT Number; // used only for unique label string generation
};

// Conditional and unconditional branch instruction.
class InstX8632Br : public InstX8632 {
public:
  enum BrCond {
#define X(tag, dump, emit) tag,
    ICEINSTX8632BR_TABLE
#undef X
        Br_None
  };

  // Create a conditional branch to a node.
  static InstX8632Br *create(Cfg *Func, CfgNode *TargetTrue,
                             CfgNode *TargetFalse, BrCond Condition) {
    return new (Func->allocate<InstX8632Br>())
        InstX8632Br(Func, TargetTrue, TargetFalse, NULL, Condition);
  }
  // Create an unconditional branch to a node.
  static InstX8632Br *create(Cfg *Func, CfgNode *Target) {
    return new (Func->allocate<InstX8632Br>())
        InstX8632Br(Func, NULL, Target, NULL, Br_None);
  }
  // Create a non-terminator conditional branch to a node, with a
  // fallthrough to the next instruction in the current node.  This is
  // used for switch lowering.
  static InstX8632Br *create(Cfg *Func, CfgNode *Target, BrCond Condition) {
    return new (Func->allocate<InstX8632Br>())
        InstX8632Br(Func, Target, NULL, NULL, Condition);
  }
  // Create a conditional intra-block branch (or unconditional, if
  // Condition==None) to a label in the current block.
  static InstX8632Br *create(Cfg *Func, InstX8632Label *Label,
                             BrCond Condition) {
    return new (Func->allocate<InstX8632Br>())
        InstX8632Br(Func, NULL, NULL, Label, Condition);
  }
  CfgNode *getTargetTrue() const { return TargetTrue; }
  CfgNode *getTargetFalse() const { return TargetFalse; }
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Br); }

private:
  InstX8632Br(Cfg *Func, CfgNode *TargetTrue, CfgNode *TargetFalse,
              InstX8632Label *Label, BrCond Condition);
  InstX8632Br(const InstX8632Br &) LLVM_DELETED_FUNCTION;
  InstX8632Br &operator=(const InstX8632Br &) LLVM_DELETED_FUNCTION;
  virtual ~InstX8632Br() {}
  BrCond Condition;
  CfgNode *TargetTrue;
  CfgNode *TargetFalse;
  InstX8632Label *Label; // Intra-block branch target
};

// Call instruction.  Arguments should have already been pushed.
class InstX8632Call : public InstX8632 {
public:
  static InstX8632Call *create(Cfg *Func, Variable *Dest, Operand *CallTarget) {
    return new (Func->allocate<InstX8632Call>())
        InstX8632Call(Func, Dest, CallTarget);
  }
  Operand *getCallTarget() const { return getSrc(0); }
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Call); }

private:
  InstX8632Call(Cfg *Func, Variable *Dest, Operand *CallTarget);
  InstX8632Call(const InstX8632Call &) LLVM_DELETED_FUNCTION;
  InstX8632Call &operator=(const InstX8632Call &) LLVM_DELETED_FUNCTION;
  virtual ~InstX8632Call() {}
};

// See the definition of emitTwoAddress() for a description of
// ShiftHack.
void emitTwoAddress(const char *Opcode, const Inst *Inst, const Cfg *Func,
                    bool ShiftHack = false);

template <InstX8632::InstKindX8632 K, bool ShiftHack = false>
class InstX8632Binop : public InstX8632 {
public:
  // Create an ordinary binary-op instruction like add or sub.
  static InstX8632Binop *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX8632Binop>())
        InstX8632Binop(Func, Dest, Source);
  }
  virtual void emit(const Cfg *Func) const {
    emitTwoAddress(Opcode, this, Func, ShiftHack);
  }
  virtual void dump(const Cfg *Func) const {
    Ostream &Str = Func->getContext()->getStrDump();
    dumpDest(Func);
    Str << " = " << Opcode << "." << getDest()->getType() << " ";
    dumpSources(Func);
  }
  static bool classof(const Inst *Inst) { return isClassof(Inst, K); }

private:
  InstX8632Binop(Cfg *Func, Variable *Dest, Operand *Source)
      : InstX8632(Func, K, 2, Dest) {
    addSource(Dest);
    addSource(Source);
  }
  InstX8632Binop(const InstX8632Binop &) LLVM_DELETED_FUNCTION;
  InstX8632Binop &operator=(const InstX8632Binop &) LLVM_DELETED_FUNCTION;
  virtual ~InstX8632Binop() {}
  static const char *Opcode;
};

template <InstX8632::InstKindX8632 K> class InstX8632Ternop : public InstX8632 {
public:
  // Create a ternary-op instruction like div or idiv.
  static InstX8632Ternop *create(Cfg *Func, Variable *Dest, Operand *Source1,
                                 Operand *Source2) {
    return new (Func->allocate<InstX8632Ternop>())
        InstX8632Ternop(Func, Dest, Source1, Source2);
  }
  virtual void emit(const Cfg *Func) const {
    Ostream &Str = Func->getContext()->getStrEmit();
    assert(getSrcSize() == 3);
    Str << "\t" << Opcode << "\t";
    getSrc(1)->emit(Func);
    Str << "\n";
  }
  virtual void dump(const Cfg *Func) const {
    Ostream &Str = Func->getContext()->getStrDump();
    dumpDest(Func);
    Str << " = " << Opcode << "." << getDest()->getType() << " ";
    dumpSources(Func);
  }
  static bool classof(const Inst *Inst) { return isClassof(Inst, K); }

private:
  InstX8632Ternop(Cfg *Func, Variable *Dest, Operand *Source1, Operand *Source2)
      : InstX8632(Func, K, 3, Dest) {
    addSource(Dest);
    addSource(Source1);
    addSource(Source2);
  }
  InstX8632Ternop(const InstX8632Ternop &) LLVM_DELETED_FUNCTION;
  InstX8632Ternop &operator=(const InstX8632Ternop &) LLVM_DELETED_FUNCTION;
  virtual ~InstX8632Ternop() {}
  static const char *Opcode;
};

typedef InstX8632Binop<InstX8632::Add> InstX8632Add;
typedef InstX8632Binop<InstX8632::Adc> InstX8632Adc;
typedef InstX8632Binop<InstX8632::Addss> InstX8632Addss;
typedef InstX8632Binop<InstX8632::Sub> InstX8632Sub;
typedef InstX8632Binop<InstX8632::Subss> InstX8632Subss;
typedef InstX8632Binop<InstX8632::Sbb> InstX8632Sbb;
typedef InstX8632Binop<InstX8632::And> InstX8632And;
typedef InstX8632Binop<InstX8632::Or> InstX8632Or;
typedef InstX8632Binop<InstX8632::Xor> InstX8632Xor;
typedef InstX8632Binop<InstX8632::Imul> InstX8632Imul;
typedef InstX8632Binop<InstX8632::Mulss> InstX8632Mulss;
typedef InstX8632Binop<InstX8632::Divss> InstX8632Divss;
typedef InstX8632Binop<InstX8632::Shl, true> InstX8632Shl;
typedef InstX8632Binop<InstX8632::Shr, true> InstX8632Shr;
typedef InstX8632Binop<InstX8632::Sar, true> InstX8632Sar;
typedef InstX8632Ternop<InstX8632::Idiv> InstX8632Idiv;
typedef InstX8632Ternop<InstX8632::Div> InstX8632Div;

// Mul instruction - unsigned multiply.
class InstX8632Mul : public InstX8632 {
public:
  static InstX8632Mul *create(Cfg *Func, Variable *Dest, Variable *Source1,
                              Operand *Source2) {
    return new (Func->allocate<InstX8632Mul>())
        InstX8632Mul(Func, Dest, Source1, Source2);
  }
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Mul); }

private:
  InstX8632Mul(Cfg *Func, Variable *Dest, Variable *Source1, Operand *Source2);
  InstX8632Mul(const InstX8632Mul &) LLVM_DELETED_FUNCTION;
  InstX8632Mul &operator=(const InstX8632Mul &) LLVM_DELETED_FUNCTION;
  virtual ~InstX8632Mul() {}
};

// Shld instruction - shift across a pair of operands.  TODO: Verify
// that the validator accepts the shld instruction.
class InstX8632Shld : public InstX8632 {
public:
  static InstX8632Shld *create(Cfg *Func, Variable *Dest, Variable *Source1,
                               Variable *Source2) {
    return new (Func->allocate<InstX8632Shld>())
        InstX8632Shld(Func, Dest, Source1, Source2);
  }
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Shld); }

private:
  InstX8632Shld(Cfg *Func, Variable *Dest, Variable *Source1,
                Variable *Source2);
  InstX8632Shld(const InstX8632Shld &) LLVM_DELETED_FUNCTION;
  InstX8632Shld &operator=(const InstX8632Shld &) LLVM_DELETED_FUNCTION;
  virtual ~InstX8632Shld() {}
};

// Shrd instruction - shift across a pair of operands.  TODO: Verify
// that the validator accepts the shrd instruction.
class InstX8632Shrd : public InstX8632 {
public:
  static InstX8632Shrd *create(Cfg *Func, Variable *Dest, Variable *Source1,
                               Variable *Source2) {
    return new (Func->allocate<InstX8632Shrd>())
        InstX8632Shrd(Func, Dest, Source1, Source2);
  }
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Shrd); }

private:
  InstX8632Shrd(Cfg *Func, Variable *Dest, Variable *Source1,
                Variable *Source2);
  InstX8632Shrd(const InstX8632Shrd &) LLVM_DELETED_FUNCTION;
  InstX8632Shrd &operator=(const InstX8632Shrd &) LLVM_DELETED_FUNCTION;
  virtual ~InstX8632Shrd() {}
};

// Cdq instruction - sign-extend eax into edx
class InstX8632Cdq : public InstX8632 {
public:
  static InstX8632Cdq *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX8632Cdq>())
        InstX8632Cdq(Func, Dest, Source);
  }
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Cdq); }

private:
  InstX8632Cdq(Cfg *Func, Variable *Dest, Operand *Source);
  InstX8632Cdq(const InstX8632Cdq &) LLVM_DELETED_FUNCTION;
  InstX8632Cdq &operator=(const InstX8632Cdq &) LLVM_DELETED_FUNCTION;
  virtual ~InstX8632Cdq() {}
};

// Cvt instruction - wrapper for cvtsX2sY where X and Y are in {s,d,i}
// as appropriate.  s=float, d=double, i=int.  X and Y are determined
// from dest/src types.  Sign and zero extension on the integer
// operand needs to be done separately.
class InstX8632Cvt : public InstX8632 {
public:
  static InstX8632Cvt *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX8632Cvt>())
        InstX8632Cvt(Func, Dest, Source);
  }
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Cvt); }

private:
  InstX8632Cvt(Cfg *Func, Variable *Dest, Operand *Source);
  InstX8632Cvt(const InstX8632Cvt &) LLVM_DELETED_FUNCTION;
  InstX8632Cvt &operator=(const InstX8632Cvt &) LLVM_DELETED_FUNCTION;
  virtual ~InstX8632Cvt() {}
};

// cmp - Integer compare instruction.
class InstX8632Icmp : public InstX8632 {
public:
  static InstX8632Icmp *create(Cfg *Func, Operand *Src1, Operand *Src2) {
    return new (Func->allocate<InstX8632Icmp>())
        InstX8632Icmp(Func, Src1, Src2);
  }
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Icmp); }

private:
  InstX8632Icmp(Cfg *Func, Operand *Src1, Operand *Src2);
  InstX8632Icmp(const InstX8632Icmp &) LLVM_DELETED_FUNCTION;
  InstX8632Icmp &operator=(const InstX8632Icmp &) LLVM_DELETED_FUNCTION;
  virtual ~InstX8632Icmp() {}
};

// ucomiss/ucomisd - floating-point compare instruction.
class InstX8632Ucomiss : public InstX8632 {
public:
  static InstX8632Ucomiss *create(Cfg *Func, Operand *Src1, Operand *Src2) {
    return new (Func->allocate<InstX8632Ucomiss>())
        InstX8632Ucomiss(Func, Src1, Src2);
  }
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Ucomiss); }

private:
  InstX8632Ucomiss(Cfg *Func, Operand *Src1, Operand *Src2);
  InstX8632Ucomiss(const InstX8632Ucomiss &) LLVM_DELETED_FUNCTION;
  InstX8632Ucomiss &operator=(const InstX8632Ucomiss &) LLVM_DELETED_FUNCTION;
  virtual ~InstX8632Ucomiss() {}
};

// Test instruction.
class InstX8632Test : public InstX8632 {
public:
  static InstX8632Test *create(Cfg *Func, Operand *Source1, Operand *Source2) {
    return new (Func->allocate<InstX8632Test>())
        InstX8632Test(Func, Source1, Source2);
  }
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Test); }

private:
  InstX8632Test(Cfg *Func, Operand *Source1, Operand *Source2);
  InstX8632Test(const InstX8632Test &) LLVM_DELETED_FUNCTION;
  InstX8632Test &operator=(const InstX8632Test &) LLVM_DELETED_FUNCTION;
  virtual ~InstX8632Test() {}
};

// This is essentially a "mov" instruction with an OperandX8632Mem
// operand instead of Variable as the destination.  It's important
// for liveness that there is no Dest operand.
class InstX8632Store : public InstX8632 {
public:
  static InstX8632Store *create(Cfg *Func, Operand *Value, OperandX8632 *Mem) {
    return new (Func->allocate<InstX8632Store>())
        InstX8632Store(Func, Value, Mem);
  }
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Store); }

private:
  InstX8632Store(Cfg *Func, Operand *Value, OperandX8632 *Mem);
  InstX8632Store(const InstX8632Store &) LLVM_DELETED_FUNCTION;
  InstX8632Store &operator=(const InstX8632Store &) LLVM_DELETED_FUNCTION;
  virtual ~InstX8632Store() {}
};

// Move/assignment instruction - wrapper for mov/movss/movsd.
class InstX8632Mov : public InstX8632 {
public:
  static InstX8632Mov *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX8632Mov>())
        InstX8632Mov(Func, Dest, Source);
  }
  virtual bool isRedundantAssign() const;
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Mov); }

private:
  InstX8632Mov(Cfg *Func, Variable *Dest, Operand *Source);
  InstX8632Mov(const InstX8632Mov &) LLVM_DELETED_FUNCTION;
  InstX8632Mov &operator=(const InstX8632Mov &) LLVM_DELETED_FUNCTION;
  virtual ~InstX8632Mov() {}
};

// Movsx - copy from a narrower integer type to a wider integer
// type, with sign extension.
class InstX8632Movsx : public InstX8632 {
public:
  static InstX8632Movsx *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX8632Movsx>())
        InstX8632Movsx(Func, Dest, Source);
  }
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Movsx); }

private:
  InstX8632Movsx(Cfg *Func, Variable *Dest, Operand *Source);
  InstX8632Movsx(const InstX8632Movsx &) LLVM_DELETED_FUNCTION;
  InstX8632Movsx &operator=(const InstX8632Movsx &) LLVM_DELETED_FUNCTION;
  virtual ~InstX8632Movsx() {}
};

// Movsx - copy from a narrower integer type to a wider integer
// type, with zero extension.
class InstX8632Movzx : public InstX8632 {
public:
  static InstX8632Movzx *create(Cfg *Func, Variable *Dest, Operand *Source) {
    return new (Func->allocate<InstX8632Movzx>())
        InstX8632Movzx(Func, Dest, Source);
  }
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Movzx); }

private:
  InstX8632Movzx(Cfg *Func, Variable *Dest, Operand *Source);
  InstX8632Movzx(const InstX8632Movzx &) LLVM_DELETED_FUNCTION;
  InstX8632Movzx &operator=(const InstX8632Movzx &) LLVM_DELETED_FUNCTION;
  virtual ~InstX8632Movzx() {}
};

// Fld - load a value onto the x87 FP stack.
class InstX8632Fld : public InstX8632 {
public:
  static InstX8632Fld *create(Cfg *Func, Operand *Src) {
    return new (Func->allocate<InstX8632Fld>()) InstX8632Fld(Func, Src);
  }
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Fld); }

private:
  InstX8632Fld(Cfg *Func, Operand *Src);
  InstX8632Fld(const InstX8632Fld &) LLVM_DELETED_FUNCTION;
  InstX8632Fld &operator=(const InstX8632Fld &) LLVM_DELETED_FUNCTION;
  virtual ~InstX8632Fld() {}
};

// Fstp - store x87 st(0) into memory and pop st(0).
class InstX8632Fstp : public InstX8632 {
public:
  static InstX8632Fstp *create(Cfg *Func, Variable *Dest) {
    return new (Func->allocate<InstX8632Fstp>()) InstX8632Fstp(Func, Dest);
  }
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Fstp); }

private:
  InstX8632Fstp(Cfg *Func, Variable *Dest);
  InstX8632Fstp(const InstX8632Fstp &) LLVM_DELETED_FUNCTION;
  InstX8632Fstp &operator=(const InstX8632Fstp &) LLVM_DELETED_FUNCTION;
  virtual ~InstX8632Fstp() {}
};

class InstX8632Pop : public InstX8632 {
public:
  static InstX8632Pop *create(Cfg *Func, Variable *Dest) {
    return new (Func->allocate<InstX8632Pop>()) InstX8632Pop(Func, Dest);
  }
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Pop); }

private:
  InstX8632Pop(Cfg *Func, Variable *Dest);
  InstX8632Pop(const InstX8632Pop &) LLVM_DELETED_FUNCTION;
  InstX8632Pop &operator=(const InstX8632Pop &) LLVM_DELETED_FUNCTION;
  virtual ~InstX8632Pop() {}
};

class InstX8632Push : public InstX8632 {
public:
  static InstX8632Push *create(Cfg *Func, Operand *Source,
                               bool SuppressStackAdjustment) {
    return new (Func->allocate<InstX8632Push>())
        InstX8632Push(Func, Source, SuppressStackAdjustment);
  }
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Push); }

private:
  InstX8632Push(Cfg *Func, Operand *Source, bool SuppressStackAdjustment);
  InstX8632Push(const InstX8632Push &) LLVM_DELETED_FUNCTION;
  InstX8632Push &operator=(const InstX8632Push &) LLVM_DELETED_FUNCTION;
  bool SuppressStackAdjustment;
  virtual ~InstX8632Push() {}
};

// Ret instruction.  Currently only supports the "ret" version that
// does not pop arguments.  This instruction takes a Source operand
// (for non-void returning functions) for liveness analysis, though
// a FakeUse before the ret would do just as well.
class InstX8632Ret : public InstX8632 {
public:
  static InstX8632Ret *create(Cfg *Func, Variable *Source = NULL) {
    return new (Func->allocate<InstX8632Ret>()) InstX8632Ret(Func, Source);
  }
  virtual void emit(const Cfg *Func) const;
  virtual void dump(const Cfg *Func) const;
  static bool classof(const Inst *Inst) { return isClassof(Inst, Ret); }

private:
  InstX8632Ret(Cfg *Func, Variable *Source);
  InstX8632Ret(const InstX8632Ret &) LLVM_DELETED_FUNCTION;
  InstX8632Ret &operator=(const InstX8632Ret &) LLVM_DELETED_FUNCTION;
  virtual ~InstX8632Ret() {}
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEINSTX8632_H
