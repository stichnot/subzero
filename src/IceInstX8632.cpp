//===- subzero/src/InstX8632.cpp - X86-32 instruction implementation ---===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the InstX8632 and OperandX8632 classes,
// primarily the constructors and the dump()/emit() methods.
//
//===----------------------------------------------------------------------===//

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInst.h"
#include "IceInstX8632.h"
#include "IceTargetLoweringX8632.h"
#include "IceOperand.h"

namespace Ice {

// XXX kill this
static const char *OpcodeTypeFromIceType(Type type) {
  switch (type) {
  default:
    return "U";
  case IceType_i1:
  case IceType_i8:
    return "b";
  case IceType_i16:
    return "w";
  case IceType_i32:
    return "l";
  case IceType_i64:
    return "q";
  }
}

OperandX8632Mem::OperandX8632Mem(Cfg *Func, Type Ty, Variable *Base,
                                 Constant *Offset, Variable *Index,
                                 uint32_t Shift)
    : OperandX8632(kMem, Ty), Base(Base), Offset(Offset), Index(Index),
      Shift(Shift) {
  Vars = NULL;
  NumVars = 0;
  if (Base)
    ++NumVars;
  if (Index)
    ++NumVars;
  if (NumVars) {
    Vars = Func->allocateArrayOf<Variable *>(NumVars);
    SizeT I = 0;
    if (Base)
      Vars[I++] = Base;
    if (Index)
      Vars[I++] = Index;
    assert(I == NumVars);
  }
}

InstX8632Mul::InstX8632Mul(Cfg *Func, Variable *Dest, Variable *Source1,
                           Operand *Source2)
    : InstX8632(Func, InstX8632::Mul, 2, Dest) {
  addSource(Source1);
  addSource(Source2);
}

InstX8632Shld::InstX8632Shld(Cfg *Func, Variable *Dest, Variable *Source1,
                             Variable *Source2)
    : InstX8632(Func, InstX8632::Shld, 3, Dest) {
  addSource(Dest);
  addSource(Source1);
  addSource(Source2);
}

InstX8632Shrd::InstX8632Shrd(Cfg *Func, Variable *Dest, Variable *Source1,
                             Variable *Source2)
    : InstX8632(Func, InstX8632::Shrd, 3, Dest) {
  addSource(Dest);
  addSource(Source1);
  addSource(Source2);
}

InstX8632Label::InstX8632Label(Cfg *Func, TargetX8632 *Target)
    : InstX8632(Func, InstX8632::Label, 0, NULL),
      Number(Target->makeNextLabelNumber()) {}

IceString InstX8632Label::getName(const Cfg *Func) const {
  const static size_t BufLen = 30;
  char buf[BufLen];
  snprintf(buf, BufLen, "%u", Number);
  return ".L" + Func->getFunctionName() + "$__" + buf;
}

InstX8632Br::InstX8632Br(Cfg *Func, CfgNode *TargetTrue, CfgNode *TargetFalse,
                         InstX8632Label *Label, InstX8632Br::BrCond Condition)
    : InstX8632(Func, InstX8632::Br, 0, NULL), Condition(Condition),
      TargetTrue(TargetTrue), TargetFalse(TargetFalse), Label(Label) {}

InstX8632Call::InstX8632Call(Cfg *Func, Variable *Dest, Operand *CallTarget,
                             bool Tail)
    : InstX8632(Func, InstX8632::Call, 1, Dest), Tail(Tail) {
  HasSideEffects = true;
  addSource(CallTarget);
}

InstX8632Cdq::InstX8632Cdq(Cfg *Func, Variable *Dest, Operand *Source)
    : InstX8632(Func, InstX8632::Cdq, 1, Dest) {
  assert(Dest->getRegNum() == TargetX8632::Reg_edx);
  assert(llvm::isa<Variable>(Source));
  assert(llvm::dyn_cast<Variable>(Source)->getRegNum() == TargetX8632::Reg_eax);
  addSource(Source);
}

InstX8632Cvt::InstX8632Cvt(Cfg *Func, Variable *Dest, Operand *Source)
    : InstX8632(Func, InstX8632::Cvt, 1, Dest) {
  addSource(Source);
}

InstX8632Icmp::InstX8632Icmp(Cfg *Func, Operand *Src0, Operand *Src1)
    : InstX8632(Func, InstX8632::Icmp, 2, NULL) {
  addSource(Src0);
  addSource(Src1);
}

InstX8632Ucomiss::InstX8632Ucomiss(Cfg *Func, Operand *Src0, Operand *Src1)
    : InstX8632(Func, InstX8632::Ucomiss, 2, NULL) {
  addSource(Src0);
  addSource(Src1);
}

InstX8632Test::InstX8632Test(Cfg *Func, Operand *Src1, Operand *Src2)
    : InstX8632(Func, InstX8632::Test, 2, NULL) {
  addSource(Src1);
  addSource(Src2);
}

InstX8632Store::InstX8632Store(Cfg *Func, Operand *Value, OperandX8632 *Mem)
    : InstX8632(Func, InstX8632::Store, 2, NULL) {
  addSource(Value);
  addSource(Mem);
}

InstX8632Mov::InstX8632Mov(Cfg *Func, Variable *Dest, Operand *Source)
    : InstX8632(Func, InstX8632::Mov, 1, Dest) {
  addSource(Source);
}

InstX8632Movsx::InstX8632Movsx(Cfg *Func, Variable *Dest, Operand *Source)
    : InstX8632(Func, InstX8632::Movsx, 1, Dest) {
  addSource(Source);
}

InstX8632Movzx::InstX8632Movzx(Cfg *Func, Variable *Dest, Operand *Source)
    : InstX8632(Func, InstX8632::Movzx, 1, Dest) {
  addSource(Source);
}

InstX8632Fld::InstX8632Fld(Cfg *Func, Operand *Src)
    : InstX8632(Func, InstX8632::Fld, 1, NULL) {
  addSource(Src);
}

InstX8632Fstp::InstX8632Fstp(Cfg *Func, Variable *Dest)
    : InstX8632(Func, InstX8632::Fstp, 0, Dest) {}

InstX8632Pop::InstX8632Pop(Cfg *Func, Variable *Dest)
    : InstX8632(Func, InstX8632::Pop, 1, Dest) {}

InstX8632Push::InstX8632Push(Cfg *Func, Operand *Source,
                             bool SuppressStackAdjustment)
    : InstX8632(Func, InstX8632::Push, 1, NULL),
      SuppressStackAdjustment(SuppressStackAdjustment) {
  addSource(Source);
}

bool InstX8632Mov::isRedundantAssign() const {
  Variable *Src = llvm::dyn_cast<Variable>(getSrc(0));
  if (Src == NULL)
    return false;
  if (getDest()->hasReg() && getDest()->getRegNum() == Src->getRegNum()) {
    // TODO: On x86-64, instructions like "mov eax, eax" are used to
    // clear the upper 32 bits of rax.  We need to recognize and
    // preserve these.
    return true;
  }
  if (!getDest()->hasReg() && !Src->hasReg() &&
      Dest->getStackOffset() == Src->getStackOffset())
    return true;
  return false;
}

InstX8632Ret::InstX8632Ret(Cfg *Func, Variable *Source)
    : InstX8632(Func, InstX8632::Ret, Source ? 1 : 0, NULL) {
  if (Source)
    addSource(Source);
}

// ======================== Dump routines ======================== //

void InstX8632::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "[X8632] ";
  Inst::dump(Func);
}

void InstX8632Label::emit(const Cfg *Func, uint32_t /*Option*/) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  Str << getName(Func) << ":\n";
}

void InstX8632Label::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  Str << getName(Func) << ":";
}

void InstX8632Br::emit(const Cfg *Func, uint32_t /*Option*/) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  Str << "\t";
#define X(tag, dump, emit)                                                     \
  case tag:                                                                    \
    Str << emit;                                                               \
    break;

  switch (Condition) {
    ICEINSTX8632BR_TABLE
  case Br_None:
    Str << "jmp";
    break;
  }
#undef X

  if (Label) {
    Str << "\t" << Label->getName(Func) << "\n";
  } else {
    if (Condition == Br_None) {
      Str << "\t" << getTargetFalse()->getAsmName() << "\n";
    } else {
      Str << "\t" << getTargetTrue()->getAsmName() << "\n";
      if (getTargetFalse()) {
        Str << "\tjmp\t" << getTargetFalse()->getAsmName() << "\n";
      }
    }
  }
}

void InstX8632Br::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "br ";

#define X(tag, dump, emit)                                                     \
  case tag:                                                                    \
    Str << dump;                                                               \
    break;

  switch (Condition) {
    ICEINSTX8632BR_TABLE
  case Br_None:
    Str << "label %"
        << (Label ? Label->getName(Func) : getTargetFalse()->getName());
    return;
    break;
  }
#undef X

  if (Label) {
    Str << ", label %" << Label->getName(Func);
  } else {
    Str << ", label %" << getTargetTrue()->getName();
    if (getTargetFalse()) {
      Str << ", label %" << getTargetFalse()->getName();
    }
  }
}

void InstX8632Call::emit(const Cfg *Func, uint32_t Option) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 1);
  Str << "\tcall\t";
  getCallTarget()->emit(Func, Option);
  if (Tail)
    Str << "\t# tail";
  Str << "\n";
  Func->getTarget()->resetStackAdjustment();
}

void InstX8632Call::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  if (getDest()) {
    dumpDest(Func);
    Str << " = ";
  }
  if (Tail)
    Str << "tail ";
  Str << "call ";
  getCallTarget()->dump(Func);
}

void IceEmitTwoAddress(const char *Opcode, const Inst *Inst, const Cfg *Func,
                       uint32_t Option, bool ShiftHack) {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(Inst->getSrcSize() == 2);
  assert(Inst->getDest() == Inst->getSrc(0));
  Str << "\t" << Opcode << "\t";
  Inst->getDest()->emit(Func, Option);
  Str << ", ";
  bool EmittedSrc1 = false;
  if (ShiftHack) {
    Variable *ShiftReg = llvm::dyn_cast<Variable>(Inst->getSrc(1));
    if (ShiftReg && ShiftReg->getRegNum() == TargetX8632::Reg_ecx) {
      Str << "cl";
      EmittedSrc1 = true;
    }
  }
  if (!EmittedSrc1)
    Inst->getSrc(1)->emit(Func, Option);
  Str << "\n";
}

template <> const char *InstX8632Add::Opcode = "add";
template <> const char *InstX8632Adc::Opcode = "adc";
template <> const char *InstX8632Addss::Opcode = "addss";
template <> const char *InstX8632Sub::Opcode = "sub";
template <> const char *InstX8632Subss::Opcode = "subss";
template <> const char *InstX8632Sbb::Opcode = "sbb";
template <> const char *InstX8632And::Opcode = "and";
template <> const char *InstX8632Or::Opcode = "or";
template <> const char *InstX8632Xor::Opcode = "xor";
template <> const char *InstX8632Imul::Opcode = "imul";
template <> const char *InstX8632Mulss::Opcode = "mulss";
template <> const char *InstX8632Div::Opcode = "div";
template <> const char *InstX8632Idiv::Opcode = "idiv";
template <> const char *InstX8632Divss::Opcode = "divss";
template <> const char *InstX8632Shl::Opcode = "shl";
template <> const char *InstX8632Shr::Opcode = "shr";
template <> const char *InstX8632Sar::Opcode = "sar";

template <> void InstX8632Addss::emit(const Cfg *Func, uint32_t Option) const {
  IceEmitTwoAddress(getDest()->getType() == IceType_f32 ? "addss" : "addsd",
                    this, Func, Option);
}

template <> void InstX8632Subss::emit(const Cfg *Func, uint32_t Option) const {
  IceEmitTwoAddress(getDest()->getType() == IceType_f32 ? "subss" : "subsd",
                    this, Func, Option);
}

template <> void InstX8632Mulss::emit(const Cfg *Func, uint32_t Option) const {
  IceEmitTwoAddress(getDest()->getType() == IceType_f32 ? "mulss" : "mulsd",
                    this, Func, Option);
}

template <> void InstX8632Divss::emit(const Cfg *Func, uint32_t Option) const {
  IceEmitTwoAddress(getDest()->getType() == IceType_f32 ? "divss" : "divsd",
                    this, Func, Option);
}

template <> void InstX8632Imul::emit(const Cfg *Func, uint32_t Option) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 2);
  if (getDest()->getType() == IceType_i8) {
    // The 8-bit version of imul only allows the form "imul r/m8".
    Variable *Src0 = llvm::dyn_cast<Variable>(getSrc(0));
    assert(Src0 && Src0->getRegNum() == TargetX8632::Reg_eax);
    Str << "\timul\t";
    getSrc(1)->emit(Func, Option);
    Str << "\n";
  } else if (llvm::isa<Constant>(getSrc(1))) {
    Str << "\timul\t";
    getDest()->emit(Func, Option);
    Str << ", ";
    getSrc(0)->emit(Func, Option);
    Str << ", ";
    getSrc(1)->emit(Func, Option);
    Str << "\n";
  } else {
    IceEmitTwoAddress("imul", this, Func, Option);
  }
}

void InstX8632Mul::emit(const Cfg *Func, uint32_t Option) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 2);
  assert(llvm::isa<Variable>(getSrc(0)));
  assert(llvm::dyn_cast<Variable>(getSrc(0))->getRegNum() ==
         TargetX8632::Reg_eax);
  assert(getDest()->getRegNum() == TargetX8632::Reg_eax); // TODO: allow edx?
  Str << "\tmul\t";
  getSrc(1)->emit(Func, Option);
  Str << "\n";
}

void InstX8632Mul::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  dumpDest(Func);
  Str << " = mul." << getDest()->getType() << " ";
  dumpSources(Func);
}

void InstX8632Shld::emit(const Cfg *Func, uint32_t Option) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 3);
  assert(getDest() == getSrc(0));
  Str << "\tshld\t";
  getDest()->emit(Func, Option);
  Str << ", ";
  getSrc(1)->emit(Func, Option);
  Str << ", ";
  bool ShiftHack = true;
  bool EmittedSrc1 = false;
  if (ShiftHack) {
    Variable *ShiftReg = llvm::dyn_cast<Variable>(getSrc(2));
    if (ShiftReg && ShiftReg->getRegNum() == TargetX8632::Reg_ecx) {
      Str << "cl";
      EmittedSrc1 = true;
    }
  }
  if (!EmittedSrc1)
    getSrc(2)->emit(Func, Option);
  Str << "\n";
}

void InstX8632Shld::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  dumpDest(Func);
  Str << " = shld." << getDest()->getType() << " ";
  dumpSources(Func);
}

void InstX8632Shrd::emit(const Cfg *Func, uint32_t Option) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 3);
  assert(getDest() == getSrc(0));
  Str << "\tshrd\t";
  getDest()->emit(Func, Option);
  Str << ", ";
  getSrc(1)->emit(Func, Option);
  Str << ", ";
  bool ShiftHack = true;
  bool EmittedSrc1 = false;
  if (ShiftHack) {
    Variable *ShiftReg = llvm::dyn_cast<Variable>(getSrc(2));
    if (ShiftReg && ShiftReg->getRegNum() == TargetX8632::Reg_ecx) {
      Str << "cl";
      EmittedSrc1 = true;
    }
  }
  if (!EmittedSrc1)
    getSrc(2)->emit(Func, Option);
  Str << "\n";
}

void InstX8632Shrd::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  dumpDest(Func);
  Str << " = shrd." << getDest()->getType() << " ";
  dumpSources(Func);
}

void InstX8632Cdq::emit(const Cfg *Func, uint32_t /*Option*/) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 1);
  Str << "\tcdq\n";
}

void InstX8632Cdq::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  dumpDest(Func);
  Str << " = cdq." << getSrc(0)->getType() << " ";
  dumpSources(Func);
}

static IceString getCvtTypeString(Type Ty) {
  switch (Ty) {
  case IceType_i1:
  case IceType_i8:
  case IceType_i16:
  case IceType_i32:
  case IceType_i64:
    return "i";
    break;
  case IceType_f32:
    return "s";
    break;
  case IceType_f64:
    return "d";
    break;
  case IceType_void:
  case IceType_NUM:
    assert(0);
    break;
  }
  return "???";
}

void InstX8632Cvt::emit(const Cfg *Func, uint32_t Option) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 1);
  Str << "\tcvts" << getCvtTypeString(getSrc(0)->getType()) << "2s"
      << getCvtTypeString(getDest()->getType()) << "\t";
  getDest()->emit(Func, Option);
  Str << ", ";
  getSrc(0)->emit(Func, Option);
  Str << "\n";
}

void InstX8632Cvt::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  dumpDest(Func);
  Str << " = cvts" << getCvtTypeString(getSrc(0)->getType()) << "2s"
      << getCvtTypeString(getDest()->getType()) << " ";
  dumpSources(Func);
}

void InstX8632Icmp::emit(const Cfg *Func, uint32_t Option) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 2);
  Str << "\tcmp\t";
  getSrc(0)->emit(Func, Option);
  Str << ", ";
  getSrc(1)->emit(Func, Option);
  Str << "\n";
}

void InstX8632Icmp::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "cmp." << getSrc(0)->getType() << " ";
  dumpSources(Func);
}

void InstX8632Ucomiss::emit(const Cfg *Func, uint32_t Option) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 2);
  if (getSrc(0)->getType() == IceType_f32)
    Str << "\tucomiss\t";
  else
    Str << "\tucomisd\t";
  getSrc(0)->emit(Func, Option);
  Str << ", ";
  getSrc(1)->emit(Func, Option);
  Str << "\n";
}

void InstX8632Ucomiss::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "ucomiss." << getSrc(0)->getType() << " ";
  dumpSources(Func);
}

void InstX8632Test::emit(const Cfg *Func, uint32_t Option) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 2);
  Str << "\ttest\t";
  getSrc(0)->emit(Func, Option);
  Str << ", ";
  getSrc(1)->emit(Func, Option);
  Str << "\n";
}

void InstX8632Test::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "test." << getSrc(0)->getType() << " ";
  dumpSources(Func);
}

void InstX8632Store::emit(const Cfg *Func, uint32_t Option) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 2);
  Str << "\tmov\t";
  getSrc(1)->emit(Func, Option);
  Str << ", ";
  getSrc(0)->emit(Func, Option);
  Str << "\n";
}

void InstX8632Store::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "mov." << getSrc(0)->getType() << " ";
  getSrc(1)->dump(Func);
  Str << ", ";
  getSrc(0)->dump(Func);
}

void InstX8632Mov::emit(const Cfg *Func, uint32_t Option) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 1);
  Str << "\tmov";
  switch (getDest()->getType()) {
  case IceType_f32:
    Str << "ss"; // movss
    break;
  case IceType_f64:
    Str << "sd"; // movsd
    break;
  default:
    break; // mov
  }
  Str << "\t";
  // For an integer truncation operation, src is wider than dest.
  // Ideally, we use a mov instruction whose data width matches the
  // narrower dest.  This is a problem if e.g. src is a register like
  // esi or si where there is no 8-bit version of the register.  To be
  // safe, we instead widen the dest to match src.  This works even
  // for stack-allocated dest variables because typeWidthOnStack()
  // pads to a 4-byte boundary even if only a lower portion is used.
  assert(Func->getTarget()->typeWidthInBytesOnStack(getDest()->getType()) ==
         Func->getTarget()->typeWidthInBytesOnStack(getSrc(0)->getType()));
  getDest()->asType(getSrc(0)->getType()).emit(Func, Option);
  Str << ", ";
  getSrc(0)->emit(Func, Option);
  Str << "\n";
}

void InstX8632Mov::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "mov." << getDest()->getType() << " ";
  dumpDest(Func);
  Str << ", ";
  dumpSources(Func);
}

void InstX8632Movsx::emit(const Cfg *Func, uint32_t Option) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 1);
  Str << "\tmovsx\t";
  getDest()->emit(Func, Option);
  Str << ", ";
  getSrc(0)->emit(Func, Option);
  Str << "\n";
}

void InstX8632Movsx::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "movs" << OpcodeTypeFromIceType(getSrc(0)->getType());
  Str << OpcodeTypeFromIceType(getDest()->getType());
  Str << " ";
  dumpDest(Func);
  Str << ", ";
  dumpSources(Func);
}

void InstX8632Movzx::emit(const Cfg *Func, uint32_t Option) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 1);
  Str << "\tmovzx\t";
  getDest()->emit(Func, Option);
  Str << ", ";
  getSrc(0)->emit(Func, Option);
  Str << "\n";
}

void InstX8632Movzx::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "movz" << OpcodeTypeFromIceType(getSrc(0)->getType());
  Str << OpcodeTypeFromIceType(getDest()->getType());
  Str << " ";
  dumpDest(Func);
  Str << ", ";
  dumpSources(Func);
}

void InstX8632Fld::emit(const Cfg *Func, uint32_t Option) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 1);
  bool isDouble = (getSrc(0)->getType() == IceType_f64);
  Variable *Var = llvm::dyn_cast<Variable>(getSrc(0));
  if (Var && Var->hasReg()) {
    // This is a physical xmm register, so we need to spill it to a
    // temporary stack slot.
    Str << "\tsub\tesp, " << (isDouble ? 8 : 4) << "\n";
    Str << "\tmovs" << (isDouble ? "d" : "s") << "\t" << (isDouble ? "q" : "d")
        << "word ptr [esp], ";
    Var->emit(Func, Option);
    Str << "\n";
    Str << "\tfld\t" << (isDouble ? "q" : "d") << "word ptr [esp]\n";
    Str << "\tadd\tesp, " << (isDouble ? 8 : 4) << "\n";
    return;
  }
  Str << "\tfld\t";
  getSrc(0)->emit(Func, Option);
  Str << "\n";
}

void InstX8632Fld::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "fld." << getSrc(0)->getType() << " ";
  dumpSources(Func);
}

void InstX8632Fstp::emit(const Cfg *Func, uint32_t Option) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 0);
  if (getDest() == NULL) {
    Str << "\tfstp\tst(0)\n";
    return;
  }
  if (!getDest()->hasReg()) {
    Str << "\tfstp\t";
    getDest()->emit(Func, Option);
    Str << "\n";
    return;
  }
  // Dest is a physical (xmm) register, so st(0) needs to go through
  // memory.  Hack this by creating a temporary stack slot, spilling
  // st(0) there, loading it into the xmm register, and deallocating
  // the stack slot.
  size_t Width = typeWidthInBytes(getDest()->getType());
  Str << "\tsub\tesp, " << Width << "\n";
  Str << "\tfstp\t" << (Width == 8 ? "q" : "d") << "word ptr [esp]\n";
  Str << "\tmovs" << (Width == 8 ? "d" : "s") << "\t";
  getDest()->emit(Func, Option);
  Str << ", " << (Width == 8 ? "q" : "d") << "word ptr [esp]\n";
  Str << "\tadd\tesp, " << Width << "\n";
}

void InstX8632Fstp::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  dumpDest(Func);
  Str << " = fstp." << getDest()->getType() << ", st(0)";
  Str << "\n";
}

void InstX8632Pop::emit(const Cfg *Func, uint32_t Option) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 0);
  Str << "\tpop\t";
  getDest()->emit(Func, Option);
  Str << "\n";
}

void InstX8632Pop::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  dumpDest(Func);
  Str << " = pop." << getDest()->getType() << " ";
}

void InstX8632Push::emit(const Cfg *Func, uint32_t Option) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(getSrcSize() == 1);
  Type Ty = getSrc(0)->getType();
  Variable *Var = llvm::dyn_cast<Variable>(getSrc(0));
  if ((Ty == IceType_f32 || Ty == IceType_f64) && Var && Var->hasReg()) {
    // The xmm registers can't be directly pushed, so we fake it by
    // decrementing esp and then storing to [esp].
    Str << "\tsub\tesp, " << typeWidthInBytes(Ty) << "\n";
    if (!SuppressStackAdjustment)
      Func->getTarget()->updateStackAdjustment(typeWidthInBytes(Ty));
    Str << "\tmov" << (Ty == IceType_f32 ? "ss\td" : "sd\tq")
        << "word ptr [esp], ";
    getSrc(0)->emit(Func, Option);
    Str << "\n";
  } else if (Ty == IceType_f64 && (!Var || !Var->hasReg())) {
    // A double on the stack has to be pushed as two halves.  Push the
    // upper half followed by the lower half for little-endian.  TODO:
    // implement.
    assert(0 && "Missing support for pushing doubles from memory");
  } else {
    Str << "\tpush\t";
    getSrc(0)->emit(Func, Option);
    Str << "\n";
    if (!SuppressStackAdjustment)
      Func->getTarget()->updateStackAdjustment(4);
  }
}

void InstX8632Push::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "push." << getSrc(0)->getType() << " ";
  dumpSources(Func);
}

void InstX8632Ret::emit(const Cfg *Func, uint32_t /*Option*/) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  Str << "\tret\n";
}

void InstX8632Ret::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  Type Ty = (getSrcSize() == 0 ? IceType_void : getSrc(0)->getType());
  Str << "ret." << Ty << " ";
  dumpSources(Func);
}

void OperandX8632::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  Str << "<OperandX8632>";
}

void OperandX8632Mem::emit(const Cfg *Func, uint32_t Option) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  switch (getType()) {
  case IceType_i1:
  case IceType_i8:
    Str << "byte ptr ";
    break;
  case IceType_i16:
    Str << "word ptr ";
    break;
  case IceType_i32:
  case IceType_f32:
    Str << "dword ptr ";
    break;
  case IceType_i64:
  case IceType_f64:
    Str << "qword ptr ";
    break;
  default:
    Str << "??? ";
    break;
  }
  // TODO: The following is an almost verbatim paste of dump().
  bool Dumped = false;
  Str << "[";
  if (Base) {
    Base->emit(Func, Option);
    Dumped = true;
  }
  if (Index) {
    assert(Base);
    Str << "+";
    if (Shift > 0)
      Str << (1u << Shift) << "*";
    Index->emit(Func, Option);
    Dumped = true;
  }
  // Pretty-print the Offset.
  bool OffsetIsZero = false;
  bool OffsetIsNegative = false;
  if (Offset == NULL) {
    OffsetIsZero = true;
  } else if (ConstantInteger *CI = llvm::dyn_cast<ConstantInteger>(Offset)) {
    OffsetIsZero = (CI->getValue() == 0);
    OffsetIsNegative = (static_cast<int64_t>(CI->getValue()) < 0);
  }
  if (!OffsetIsZero) { // Suppress if Offset is known to be 0
    if (Dumped) {
      if (!OffsetIsNegative) // Suppress if Offset is known to be negative
        Str << "+";
    }
    Offset->emit(Func, Option);
  }
  Str << "]";
}

void OperandX8632Mem::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  bool Dumped = false;
  Str << "[";
  if (Base) {
    Base->dump(Func);
    Dumped = true;
  }
  if (Index) {
    assert(Base);
    Str << "+";
    if (Shift > 0)
      Str << (1u << Shift) << "*";
    Index->dump(Func);
    Dumped = true;
  }
  // Pretty-print the Offset.
  bool OffsetIsZero = false;
  bool OffsetIsNegative = false;
  if (Offset == NULL) {
    OffsetIsZero = true;
  } else if (ConstantInteger *CI = llvm::dyn_cast<ConstantInteger>(Offset)) {
    OffsetIsZero = (CI->getValue() == 0);
    OffsetIsNegative = (static_cast<int64_t>(CI->getValue()) < 0);
  }
  if (!OffsetIsZero) { // Suppress if Offset is known to be 0
    if (Dumped) {
      if (!OffsetIsNegative) // Suppress if Offset is known to be negative
        Str << "+";
    }
    Offset->dump(Func);
  }
  Str << "]";
}

void VariableSplit::emit(const Cfg *Func, uint32_t /*Option*/) const {
  Ostream &Str = Func->getContext()->getStrEmit();
  assert(Var->getLocalUseNode() == NULL ||
         Var->getLocalUseNode() == Func->getCurrentNode());
  assert(!Var->hasReg());
  // The following is copied/adapted from Variable::emit().
  Str << "dword ptr ["
      << Func->getTarget()->getRegName(Func->getTarget()->getFrameOrStackReg(),
                                       IceType_i32);
  int32_t Offset =
      Var->getStackOffset() + Func->getTarget()->getStackAdjustment();
  if (Part == High)
    Offset += 4;
  if (Offset) {
    if (Offset > 0)
      Str << "+";
    Str << Offset;
  }
  Str << "]";
}

void VariableSplit::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  switch (Part) {
  case Low:
    Str << "low";
    break;
  case High:
    Str << "high";
    break;
  default:
    Str << "???";
    break;
  }
  Str << "(";
  Var->dump(Func);
  Str << ")";
}

} // end of namespace Ice
