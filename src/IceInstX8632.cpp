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
static const char *OpcodeTypeFromIceType(IceType type) {
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

OperandX8632Mem::OperandX8632Mem(IceCfg *Cfg, IceType Type, Variable *Base,
                                 Constant *Offset, Variable *Index,
                                 uint32_t Shift)
    : OperandX8632(kMem, Type), Base(Base), Offset(Offset), Index(Index),
      Shift(Shift) {
  Vars = NULL;
  NumVars = 0;
  if (Base)
    ++NumVars;
  if (Index)
    ++NumVars;
  if (NumVars) {
    Vars = Cfg->allocateArrayOf<Variable *>(NumVars);
    uint32_t I = 0;
    if (Base)
      Vars[I++] = Base;
    if (Index)
      Vars[I++] = Index;
    assert(I == NumVars);
  }
}

void OperandX8632Mem::setUse(const Inst *Inst, const CfgNode *Node) {
  if (getBase())
    getBase()->setUse(Inst, Node);
  if (getOffset())
    getOffset()->setUse(Inst, Node);
  if (getIndex())
    getIndex()->setUse(Inst, Node);
}

InstX8632Mul::InstX8632Mul(IceCfg *Cfg, Variable *Dest, Variable *Source1,
                           Operand *Source2)
    : InstX8632(Cfg, InstX8632::Mul, 2, Dest) {
  addSource(Source1);
  addSource(Source2);
}

InstX8632Shld::InstX8632Shld(IceCfg *Cfg, Variable *Dest, Variable *Source1,
                             Variable *Source2)
    : InstX8632(Cfg, InstX8632::Shld, 3, Dest) {
  addSource(Dest);
  addSource(Source1);
  addSource(Source2);
}

InstX8632Shrd::InstX8632Shrd(IceCfg *Cfg, Variable *Dest, Variable *Source1,
                             Variable *Source2)
    : InstX8632(Cfg, InstX8632::Shrd, 3, Dest) {
  addSource(Dest);
  addSource(Source1);
  addSource(Source2);
}

InstX8632Label::InstX8632Label(IceCfg *Cfg, TargetX8632 *Target)
    : InstX8632(Cfg, InstX8632::Label, 0, NULL),
      Number(Target->makeNextLabelNumber()) {}

IceString InstX8632Label::getName(const IceCfg *Cfg) const {
  const static size_t BufLen = 30;
  char buf[BufLen];
  snprintf(buf, BufLen, "%u", Number);
  return ".L" + Cfg->getFunctionName() + "$__" + buf;
}

InstX8632Br::InstX8632Br(IceCfg *Cfg, CfgNode *TargetTrue, CfgNode *TargetFalse,
                         InstX8632Label *Label, InstX8632Br::BrCond Condition)
    : InstX8632(Cfg, InstX8632::Br, 0, NULL), Condition(Condition),
      TargetTrue(TargetTrue), TargetFalse(TargetFalse), Label(Label) {}

InstX8632Call::InstX8632Call(IceCfg *Cfg, Variable *Dest, Operand *CallTarget,
                             bool Tail)
    : InstX8632(Cfg, InstX8632::Call, 1, Dest), Tail(Tail) {
  HasSideEffects = true;
  addSource(CallTarget);
}

InstX8632Cdq::InstX8632Cdq(IceCfg *Cfg, Variable *Dest, Operand *Source)
    : InstX8632(Cfg, InstX8632::Cdq, 1, Dest) {
  assert(Dest->getRegNum() == TargetX8632::Reg_edx);
  assert(llvm::isa<Variable>(Source));
  assert(llvm::dyn_cast<Variable>(Source)->getRegNum() == TargetX8632::Reg_eax);
  addSource(Source);
}

InstX8632Cvt::InstX8632Cvt(IceCfg *Cfg, Variable *Dest, Operand *Source)
    : InstX8632(Cfg, InstX8632::Cvt, 1, Dest) {
  addSource(Source);
}

InstX8632Icmp::InstX8632Icmp(IceCfg *Cfg, Operand *Src0, Operand *Src1)
    : InstX8632(Cfg, InstX8632::Icmp, 2, NULL) {
  addSource(Src0);
  addSource(Src1);
}

InstX8632Ucomiss::InstX8632Ucomiss(IceCfg *Cfg, Operand *Src0, Operand *Src1)
    : InstX8632(Cfg, InstX8632::Ucomiss, 2, NULL) {
  addSource(Src0);
  addSource(Src1);
}

InstX8632Test::InstX8632Test(IceCfg *Cfg, Operand *Src1, Operand *Src2)
    : InstX8632(Cfg, InstX8632::Test, 2, NULL) {
  addSource(Src1);
  addSource(Src2);
}

InstX8632Store::InstX8632Store(IceCfg *Cfg, Operand *Value, OperandX8632 *Mem)
    : InstX8632(Cfg, InstX8632::Store, 2, NULL) {
  addSource(Value);
  addSource(Mem);
}

InstX8632Mov::InstX8632Mov(IceCfg *Cfg, Variable *Dest, Operand *Source)
    : InstX8632(Cfg, InstX8632::Mov, 1, Dest) {
  addSource(Source);
}

InstX8632Movsx::InstX8632Movsx(IceCfg *Cfg, Variable *Dest, Operand *Source)
    : InstX8632(Cfg, InstX8632::Movsx, 1, Dest) {
  addSource(Source);
}

InstX8632Movzx::InstX8632Movzx(IceCfg *Cfg, Variable *Dest, Operand *Source)
    : InstX8632(Cfg, InstX8632::Movzx, 1, Dest) {
  addSource(Source);
}

InstX8632Fld::InstX8632Fld(IceCfg *Cfg, Operand *Src)
    : InstX8632(Cfg, InstX8632::Fld, 1, NULL) {
  addSource(Src);
}

InstX8632Fstp::InstX8632Fstp(IceCfg *Cfg, Variable *Dest)
    : InstX8632(Cfg, InstX8632::Fstp, 0, Dest) {}

InstX8632Pop::InstX8632Pop(IceCfg *Cfg, Variable *Dest)
    : InstX8632(Cfg, InstX8632::Pop, 1, Dest) {}

InstX8632Push::InstX8632Push(IceCfg *Cfg, Operand *Source,
                             bool SuppressStackAdjustment)
    : InstX8632(Cfg, InstX8632::Push, 1, NULL),
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

InstX8632Ret::InstX8632Ret(IceCfg *Cfg, Variable *Source)
    : InstX8632(Cfg, InstX8632::Ret, Source ? 1 : 0, NULL) {
  if (Source)
    addSource(Source);
}

// ======================== Dump routines ======================== //

void InstX8632::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  Str << "[X8632] ";
  Inst::dump(Cfg);
}

void InstX8632Label::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  Str << getName(Cfg) << ":\n";
}

void InstX8632Label::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  Str << getName(Cfg) << ":";
}

void InstX8632Br::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  Str << "\t";
  switch (Condition) {
  case Br_a:
    Str << "ja";
    break;
  case Br_ae:
    Str << "jae";
    break;
  case Br_b:
    Str << "jb";
    break;
  case Br_be:
    Str << "jbe";
    break;
  case Br_e:
    Str << "je";
    break;
  case Br_g:
    Str << "jg";
    break;
  case Br_ge:
    Str << "jge";
    break;
  case Br_l:
    Str << "jl";
    break;
  case Br_le:
    Str << "jle";
    break;
  case Br_ne:
    Str << "jne";
    break;
  case Br_np:
    Str << "jnp";
    break;
  case Br_p:
    Str << "jp";
    break;
  case Br_None:
    Str << "jmp";
    break;
  }
  if (Label) {
    Str << "\t" << Label->getName(Cfg) << "\n";
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

void InstX8632Br::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  Str << "br ";
  switch (Condition) {
  case Br_a:
    Str << "a";
    break;
  case Br_ae:
    Str << "ae";
    break;
  case Br_b:
    Str << "b";
    break;
  case Br_be:
    Str << "be";
    break;
  case Br_e:
    Str << "e";
    break;
  case Br_g:
    Str << "g";
    break;
  case Br_ge:
    Str << "ge";
    break;
  case Br_l:
    Str << "l";
    break;
  case Br_le:
    Str << "le";
    break;
  case Br_ne:
    Str << "ne";
    break;
  case Br_np:
    Str << "np";
    break;
  case Br_p:
    Str << "p";
    break;
  case Br_None:
    Str << "label %"
        << (Label ? Label->getName(Cfg) : getTargetFalse()->getName());
    return;
    break;
  }
  if (Label) {
    Str << ", label %" << Label->getName(Cfg);
  } else {
    Str << ", label %" << getTargetTrue()->getName();
    if (getTargetFalse()) {
      Str << ", label %" << getTargetFalse()->getName();
    }
  }
}

void InstX8632Call::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  assert(getSrcSize() == 1);
  Str << "\tcall\t";
  getCallTarget()->emit(Cfg, Option);
  if (Tail)
    Str << "\t# tail";
  Str << "\n";
  Cfg->getTarget()->resetStackAdjustment();
}

void InstX8632Call::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  if (getDest()) {
    dumpDest(Cfg);
    Str << " = ";
  }
  if (Tail)
    Str << "tail ";
  Str << "call ";
  getCallTarget()->dump(Cfg);
}

void IceEmitTwoAddress(const char *Opcode, const Inst *Inst, const IceCfg *Cfg,
                       uint32_t Option, bool ShiftHack) {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  assert(Inst->getSrcSize() == 2);
  assert(Inst->getDest() == Inst->getSrc(0));
  Str << "\t" << Opcode << "\t";
  Inst->getDest()->emit(Cfg, Option);
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
    Inst->getSrc(1)->emit(Cfg, Option);
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

template <>
void InstX8632Addss::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceEmitTwoAddress(getDest()->getType() == IceType_f32 ? "addss" : "addsd",
                    this, Cfg, Option);
}

template <>
void InstX8632Subss::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceEmitTwoAddress(getDest()->getType() == IceType_f32 ? "subss" : "subsd",
                    this, Cfg, Option);
}

template <>
void InstX8632Mulss::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceEmitTwoAddress(getDest()->getType() == IceType_f32 ? "mulss" : "mulsd",
                    this, Cfg, Option);
}

template <>
void InstX8632Divss::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceEmitTwoAddress(getDest()->getType() == IceType_f32 ? "divss" : "divsd",
                    this, Cfg, Option);
}

template <> void InstX8632Imul::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  assert(getSrcSize() == 2);
  if (getDest()->getType() == IceType_i8) {
    // The 8-bit version of imul only allows the form "imul r/m8".
    Variable *Src0 = llvm::dyn_cast<Variable>(getSrc(0));
    assert(Src0 && Src0->getRegNum() == TargetX8632::Reg_eax);
    Str << "\timul\t";
    getSrc(1)->emit(Cfg, Option);
    Str << "\n";
  } else if (llvm::isa<Constant>(getSrc(1))) {
    Str << "\timul\t";
    getDest()->emit(Cfg, Option);
    Str << ", ";
    getSrc(0)->emit(Cfg, Option);
    Str << ", ";
    getSrc(1)->emit(Cfg, Option);
    Str << "\n";
  } else {
    IceEmitTwoAddress("imul", this, Cfg, Option);
  }
}

void InstX8632Mul::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  assert(getSrcSize() == 2);
  assert(llvm::isa<Variable>(getSrc(0)));
  assert(llvm::dyn_cast<Variable>(getSrc(0))->getRegNum() ==
         TargetX8632::Reg_eax);
  assert(getDest()->getRegNum() == TargetX8632::Reg_eax); // TODO: allow edx?
  Str << "\tmul\t";
  getSrc(1)->emit(Cfg, Option);
  Str << "\n";
}

void InstX8632Mul::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  dumpDest(Cfg);
  Str << " = mul." << getDest()->getType() << " ";
  dumpSources(Cfg);
}

void InstX8632Shld::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  assert(getSrcSize() == 3);
  assert(getDest() == getSrc(0));
  Str << "\tshld\t";
  getDest()->emit(Cfg, Option);
  Str << ", ";
  getSrc(1)->emit(Cfg, Option);
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
    getSrc(2)->emit(Cfg, Option);
  Str << "\n";
}

void InstX8632Shld::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  dumpDest(Cfg);
  Str << " = shld." << getDest()->getType() << " ";
  dumpSources(Cfg);
}

void InstX8632Shrd::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  assert(getSrcSize() == 3);
  assert(getDest() == getSrc(0));
  Str << "\tshrd\t";
  getDest()->emit(Cfg, Option);
  Str << ", ";
  getSrc(1)->emit(Cfg, Option);
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
    getSrc(2)->emit(Cfg, Option);
  Str << "\n";
}

void InstX8632Shrd::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  dumpDest(Cfg);
  Str << " = shrd." << getDest()->getType() << " ";
  dumpSources(Cfg);
}

void InstX8632Cdq::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  assert(getSrcSize() == 1);
  Str << "\tcdq\n";
}

void InstX8632Cdq::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  dumpDest(Cfg);
  Str << " = cdq." << getSrc(0)->getType() << " ";
  dumpSources(Cfg);
}

static IceString getCvtTypeString(IceType Type) {
  switch (Type) {
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

void InstX8632Cvt::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  assert(getSrcSize() == 1);
  Str << "\tcvts" << getCvtTypeString(getSrc(0)->getType()) << "2s"
      << getCvtTypeString(getDest()->getType()) << "\t";
  getDest()->emit(Cfg, Option);
  Str << ", ";
  getSrc(0)->emit(Cfg, Option);
  Str << "\n";
}

void InstX8632Cvt::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  dumpDest(Cfg);
  Str << " = cvts" << getCvtTypeString(getSrc(0)->getType()) << "2s"
      << getCvtTypeString(getDest()->getType()) << " ";
  dumpSources(Cfg);
}

void InstX8632Icmp::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  assert(getSrcSize() == 2);
  Str << "\tcmp\t";
  getSrc(0)->emit(Cfg, Option);
  Str << ", ";
  getSrc(1)->emit(Cfg, Option);
  Str << "\n";
}

void InstX8632Icmp::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  Str << "cmp." << getSrc(0)->getType() << " ";
  dumpSources(Cfg);
}

void InstX8632Ucomiss::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  assert(getSrcSize() == 2);
  if (getSrc(0)->getType() == IceType_f32)
    Str << "\tucomiss\t";
  else
    Str << "\tucomisd\t";
  getSrc(0)->emit(Cfg, Option);
  Str << ", ";
  getSrc(1)->emit(Cfg, Option);
  Str << "\n";
}

void InstX8632Ucomiss::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  Str << "ucomiss." << getSrc(0)->getType() << " ";
  dumpSources(Cfg);
}

void InstX8632Test::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  assert(getSrcSize() == 2);
  Str << "\ttest\t";
  getSrc(0)->emit(Cfg, Option);
  Str << ", ";
  getSrc(1)->emit(Cfg, Option);
  Str << "\n";
}

void InstX8632Test::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  Str << "test." << getSrc(0)->getType() << " ";
  dumpSources(Cfg);
}

void InstX8632Store::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  assert(getSrcSize() == 2);
  Str << "\tmov\t";
  getSrc(1)->emit(Cfg, Option);
  Str << ", ";
  getSrc(0)->emit(Cfg, Option);
  Str << "\n";
}

void InstX8632Store::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  Str << "mov." << getSrc(0)->getType() << " ";
  getSrc(1)->dump(Cfg);
  Str << ", ";
  getSrc(0)->dump(Cfg);
}

void InstX8632Mov::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
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
  assert(Cfg->getTarget()->typeWidthInBytesOnStack(getDest()->getType()) ==
         Cfg->getTarget()->typeWidthInBytesOnStack(getSrc(0)->getType()));
  getDest()->asType(getSrc(0)->getType()).emit(Cfg, Option);
  Str << ", ";
  getSrc(0)->emit(Cfg, Option);
  Str << "\n";
}

void InstX8632Mov::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  Str << "mov." << getDest()->getType() << " ";
  dumpDest(Cfg);
  Str << ", ";
  dumpSources(Cfg);
}

void InstX8632Movsx::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  assert(getSrcSize() == 1);
  Str << "\tmovsx\t";
  getDest()->emit(Cfg, Option);
  Str << ", ";
  getSrc(0)->emit(Cfg, Option);
  Str << "\n";
}

void InstX8632Movsx::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  Str << "movs" << OpcodeTypeFromIceType(getSrc(0)->getType());
  Str << OpcodeTypeFromIceType(getDest()->getType());
  Str << " ";
  dumpDest(Cfg);
  Str << ", ";
  dumpSources(Cfg);
}

void InstX8632Movzx::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  assert(getSrcSize() == 1);
  Str << "\tmovzx\t";
  getDest()->emit(Cfg, Option);
  Str << ", ";
  getSrc(0)->emit(Cfg, Option);
  Str << "\n";
}

void InstX8632Movzx::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  Str << "movz" << OpcodeTypeFromIceType(getSrc(0)->getType());
  Str << OpcodeTypeFromIceType(getDest()->getType());
  Str << " ";
  dumpDest(Cfg);
  Str << ", ";
  dumpSources(Cfg);
}

void InstX8632Fld::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  assert(getSrcSize() == 1);
  bool isDouble = (getSrc(0)->getType() == IceType_f64);
  Variable *Var = llvm::dyn_cast<Variable>(getSrc(0));
  if (Var && Var->hasReg()) {
    // This is a physical xmm register, so we need to spill it to a
    // temporary stack slot.
    Str << "\tsub\tesp, " << (isDouble ? 8 : 4) << "\n";
    Str << "\tmovs" << (isDouble ? "d" : "s") << "\t" << (isDouble ? "q" : "d")
        << "word ptr [esp], ";
    Var->emit(Cfg, Option);
    Str << "\n";
    Str << "\tfld\t" << (isDouble ? "q" : "d") << "word ptr [esp]\n";
    Str << "\tadd\tesp, " << (isDouble ? 8 : 4) << "\n";
    return;
  }
  Str << "\tfld\t";
  getSrc(0)->emit(Cfg, Option);
  Str << "\n";
}

void InstX8632Fld::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  Str << "fld." << getSrc(0)->getType() << " ";
  dumpSources(Cfg);
}

void InstX8632Fstp::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  assert(getSrcSize() == 0);
  if (getDest() == NULL) {
    Str << "\tfstp\tst(0)\n";
    return;
  }
  if (!getDest()->hasReg()) {
    Str << "\tfstp\t";
    getDest()->emit(Cfg, Option);
    Str << "\n";
    return;
  }
  // Dest is a physical (xmm) register, so st(0) needs to go through
  // memory.  Hack this by creating a temporary stack slot, spilling
  // st(0) there, loading it into the xmm register, and deallocating
  // the stack slot.
  uint32_t Width = iceTypeWidthInBytes(getDest()->getType());
  Str << "\tsub\tesp, " << Width << "\n";
  Str << "\tfstp\t" << (Width == 8 ? "q" : "d") << "word ptr [esp]\n";
  Str << "\tmovs" << (Width == 8 ? "d" : "s") << "\t";
  getDest()->emit(Cfg, Option);
  Str << ", " << (Width == 8 ? "q" : "d") << "word ptr [esp]\n";
  Str << "\tadd\tesp, " << Width << "\n";
}

void InstX8632Fstp::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  dumpDest(Cfg);
  Str << " = fstp." << getDest()->getType() << ", st(0)";
  Str << "\n";
}

void InstX8632Pop::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  assert(getSrcSize() == 0);
  Str << "\tpop\t";
  getDest()->emit(Cfg, Option);
  Str << "\n";
}

void InstX8632Pop::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  dumpDest(Cfg);
  Str << " = pop." << getDest()->getType() << " ";
}

void InstX8632Push::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  assert(getSrcSize() == 1);
  IceType Type = getSrc(0)->getType();
  Variable *Var = llvm::dyn_cast<Variable>(getSrc(0));
  if ((Type == IceType_f32 || Type == IceType_f64) && Var && Var->hasReg()) {
    // The xmm registers can't be directly pushed, so we fake it by
    // decrementing esp and then storing to [esp].
    Str << "\tsub\tesp, " << iceTypeWidthInBytes(Type) << "\n";
    if (!SuppressStackAdjustment)
      Cfg->getTarget()->updateStackAdjustment(iceTypeWidthInBytes(Type));
    Str << "\tmov" << (Type == IceType_f32 ? "ss\td" : "sd\tq")
        << "word ptr [esp], ";
    getSrc(0)->emit(Cfg, Option);
    Str << "\n";
  } else if (Type == IceType_f64 && (!Var || !Var->hasReg())) {
    // A double on the stack has to be pushed as two halves.  Push the
    // upper half followed by the lower half for little-endian.  TODO:
    // implement.
    assert(0 && "Missing support for pushing doubles from memory");
  } else {
    Str << "\tpush\t";
    getSrc(0)->emit(Cfg, Option);
    Str << "\n";
    if (!SuppressStackAdjustment)
      Cfg->getTarget()->updateStackAdjustment(4);
  }
}

void InstX8632Push::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  Str << "push." << getSrc(0)->getType() << " ";
  dumpSources(Cfg);
}

void InstX8632Ret::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  Str << "\tret\n";
}

void InstX8632Ret::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  IceType Type = (getSrcSize() == 0 ? IceType_void : getSrc(0)->getType());
  Str << "ret." << Type << " ";
  dumpSources(Cfg);
}

void OperandX8632::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  Str << "<OperandX8632>";
}

void OperandX8632Mem::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
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
    Base->emit(Cfg, Option);
    Dumped = true;
  }
  if (Index) {
    assert(Base);
    Str << "+";
    if (Shift > 0)
      Str << (1u << Shift) << "*";
    Index->emit(Cfg, Option);
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
    Offset->emit(Cfg, Option);
  }
  Str << "]";
}

void OperandX8632Mem::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  bool Dumped = false;
  Str << "[";
  if (Base) {
    Base->dump(Cfg);
    Dumped = true;
  }
  if (Index) {
    assert(Base);
    Str << "+";
    if (Shift > 0)
      Str << (1u << Shift) << "*";
    Index->dump(Cfg);
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
    Offset->dump(Cfg);
  }
  Str << "]";
}

void VariableSplit::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  assert(Var->getLocalUseNode() == NULL ||
         Var->getLocalUseNode() == Cfg->getCurrentNode());
  assert(!Var->hasReg());
  // The following is copied/adapted from Variable::emit().
  Str << "dword ptr ["
      << Cfg->getTarget()->getRegName(Cfg->getTarget()->getFrameOrStackReg(),
                                      IceType_i32);
  int32_t Offset =
      Var->getStackOffset() + Cfg->getTarget()->getStackAdjustment();
  if (Part == High)
    Offset += 4;
  if (Offset) {
    if (Offset > 0)
      Str << "+";
    Str << Offset;
  }
  Str << "]";
}

void VariableSplit::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
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
  Var->dump(Cfg);
  Str << ")";
}

} // end of namespace Ice
