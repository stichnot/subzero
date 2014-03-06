/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInst.h"
#include "IceInstX8632.h"
#include "IceTargetLoweringX8632.h"
#include "IceOperand.h"

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

IceOperandX8632Mem::IceOperandX8632Mem(IceCfg *Cfg, IceType Type,
                                       IceVariable *Base, IceConstant *Offset,
                                       IceVariable *Index, unsigned Shift)
    : IceOperandX8632(Cfg, Mem, Type), Base(Base), Offset(Offset), Index(Index),
      Shift(Shift) {
  Vars = NULL;
  NumVars = 0;
  if (Base)
    ++NumVars;
  if (Index)
    ++NumVars;
  if (NumVars) {
    Vars = new IceVariable *[NumVars]; // TODO: use Cfg placement alloc
    unsigned I = 0;
    if (Base)
      Vars[I++] = Base;
    if (Index)
      Vars[I++] = Index;
    assert(I == NumVars);
  }
}

void IceOperandX8632Mem::setUse(const IceInst *Inst, const IceCfgNode *Node) {
  if (getBase())
    getBase()->setUse(Inst, Node);
  if (getOffset())
    getOffset()->setUse(Inst, Node);
  if (getIndex())
    getIndex()->setUse(Inst, Node);
}

IceInstX8632Add::IceInstX8632Add(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Add, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Adc::IceInstX8632Adc(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Adc, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Addss::IceInstX8632Addss(IceCfg *Cfg, IceVariable *Dest,
                                     IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Addss, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Sub::IceInstX8632Sub(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Sub, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Sbb::IceInstX8632Sbb(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Sbb, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Subss::IceInstX8632Subss(IceCfg *Cfg, IceVariable *Dest,
                                     IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Subss, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632And::IceInstX8632And(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::And, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Or::IceInstX8632Or(IceCfg *Cfg, IceVariable *Dest,
                               IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Or, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Xor::IceInstX8632Xor(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Xor, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Imul::IceInstX8632Imul(IceCfg *Cfg, IceVariable *Dest,
                                   IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Imul, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Mul::IceInstX8632Mul(IceCfg *Cfg, IceVariable *Dest,
                                 IceVariable *Source1, IceOperand *Source2)
    : IceInstX8632(Cfg, IceInstX8632::Mul, 2, Dest) {
  addSource(Source1);
  addSource(Source2);
}

IceInstX8632Mulss::IceInstX8632Mulss(IceCfg *Cfg, IceVariable *Dest,
                                     IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Mulss, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Idiv::IceInstX8632Idiv(IceCfg *Cfg, IceVariable *Dest,
                                   IceOperand *Source, IceVariable *Other)
    : IceInstX8632(Cfg, IceInstX8632::Idiv, 3, Dest) {
  addSource(Dest);
  addSource(Source);
  addSource(Other);
}

IceInstX8632Div::IceInstX8632Div(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source, IceVariable *Other)
    : IceInstX8632(Cfg, IceInstX8632::Div, 3, Dest) {
  addSource(Dest);
  addSource(Source);
  addSource(Other);
}

IceInstX8632Divss::IceInstX8632Divss(IceCfg *Cfg, IceVariable *Dest,
                                     IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Divss, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Shl::IceInstX8632Shl(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Shl, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Shld::IceInstX8632Shld(IceCfg *Cfg, IceVariable *Dest,
                                   IceVariable *Source1, IceVariable *Source2)
    : IceInstX8632(Cfg, IceInstX8632::Shld, 3, Dest) {
  addSource(Dest);
  addSource(Source1);
  addSource(Source2);
}

IceInstX8632Shr::IceInstX8632Shr(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Shr, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Shrd::IceInstX8632Shrd(IceCfg *Cfg, IceVariable *Dest,
                                   IceVariable *Source1, IceVariable *Source2)
    : IceInstX8632(Cfg, IceInstX8632::Shrd, 3, Dest) {
  addSource(Dest);
  addSource(Source1);
  addSource(Source2);
}

IceInstX8632Sar::IceInstX8632Sar(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Sar, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Label::IceInstX8632Label(IceCfg *Cfg, IceTargetX8632 *Target)
    : IceInstX8632(Cfg, IceInstX8632::Label, 0, NULL),
      Number(Target->makeNextLabelNumber()) {}

IceString IceInstX8632Label::getName(IceCfg *Cfg) const {
  char buf[30];
  sprintf(buf, "%u", Number);
  return ".L" + Cfg->getName() + "$__" + buf;
}

IceInstX8632Br::IceInstX8632Br(IceCfg *Cfg, IceCfgNode *TargetTrue,
                               IceCfgNode *TargetFalse,
                               IceInstX8632Label *Label,
                               IceInstX8632Br::BrCond Condition)
    : IceInstX8632(Cfg, IceInstX8632::Br, 0, NULL), Condition(Condition),
      TargetTrue(TargetTrue), TargetFalse(TargetFalse), Label(Label) {}

IceInstX8632Call::IceInstX8632Call(IceCfg *Cfg, IceVariable *Dest,
                                   IceOperand *CallTarget, bool Tail)
    : IceInstX8632(Cfg, IceInstX8632::Call, 1, Dest), Tail(Tail) {
  HasSideEffects = true;
  addSource(CallTarget);
}

IceInstX8632Cdq::IceInstX8632Cdq(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Cdq, 1, Dest) {
  assert(Dest->getRegNum() == IceTargetX8632::Reg_edx);
  assert(llvm::isa<IceVariable>(Source));
  assert(llvm::dyn_cast<IceVariable>(Source)->getRegNum() ==
         IceTargetX8632::Reg_eax);
  addSource(Source);
}

IceInstX8632Cvt::IceInstX8632Cvt(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Cvt, 1, Dest) {
  addSource(Source);
}

IceInstX8632Icmp::IceInstX8632Icmp(IceCfg *Cfg, IceOperand *Src0,
                                   IceOperand *Src1)
    : IceInstX8632(Cfg, IceInstX8632::Icmp, 2, NULL) {
  addSource(Src0);
  addSource(Src1);
}

IceInstX8632Ucomiss::IceInstX8632Ucomiss(IceCfg *Cfg, IceOperand *Src0,
                                         IceOperand *Src1)
    : IceInstX8632(Cfg, IceInstX8632::Ucomiss, 2, NULL) {
  addSource(Src0);
  addSource(Src1);
}

IceInstX8632Test::IceInstX8632Test(IceCfg *Cfg, IceOperand *Src1,
                                   IceOperand *Src2)
    : IceInstX8632(Cfg, IceInstX8632::Test, 2, NULL) {
  addSource(Src1);
  addSource(Src2);
}

IceInstX8632Store::IceInstX8632Store(IceCfg *Cfg, IceOperand *Value,
                                     IceOperandX8632Mem *Mem)
    : IceInstX8632(Cfg, IceInstX8632::Store, 2, NULL) {
  addSource(Value);
  addSource(Mem);
}

IceInstX8632Mov::IceInstX8632Mov(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Mov, 1, Dest) {
  addSource(Source);
}

IceInstX8632Movsx::IceInstX8632Movsx(IceCfg *Cfg, IceVariable *Dest,
                                     IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Movsx, 1, Dest) {
  addSource(Source);
}

IceInstX8632Movzx::IceInstX8632Movzx(IceCfg *Cfg, IceVariable *Dest,
                                     IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Movzx, 1, Dest) {
  addSource(Source);
}

IceInstX8632Fld::IceInstX8632Fld(IceCfg *Cfg, IceOperand *Src)
    : IceInstX8632(Cfg, IceInstX8632::Fld, 1, NULL) {
  addSource(Src);
}

IceInstX8632Fstp::IceInstX8632Fstp(IceCfg *Cfg, IceVariable *Dest)
    : IceInstX8632(Cfg, IceInstX8632::Fstp, 0, Dest) {}

IceInstX8632Pop::IceInstX8632Pop(IceCfg *Cfg, IceVariable *Dest)
    : IceInstX8632(Cfg, IceInstX8632::Pop, 1, Dest) {}

IceInstX8632Push::IceInstX8632Push(IceCfg *Cfg, IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Push, 1, NULL) {
  addSource(Source);
}

bool IceInstX8632Mov::isRedundantAssign(void) const {
  int DestRegNum = getDest()->getRegNum();
  if (DestRegNum < 0)
    return false;
  IceVariable *Src = llvm::dyn_cast<IceVariable>(getSrc(0));
  if (Src == NULL)
    return false;
  // TODO: On x86-64, instructions like "mov eax, eax" are used to
  // clear the upper 32 bits of rax.  We need to recognize and
  // preserve these.
  return DestRegNum == Src->getRegNum();
}

IceInstX8632Ret::IceInstX8632Ret(IceCfg *Cfg, IceVariable *Source)
    : IceInstX8632(Cfg, IceInstX8632::Ret, Source ? 1 : 0, NULL) {
  if (Source)
    addSource(Source);
}

// ======================== Dump routines ======================== //

void IceInstX8632::dump(IceOstream &Str) const {
  Str << "[X8632] ";
  IceInst::dump(Str);
}

void IceInstX8632Label::emit(IceOstream &Str, unsigned Option) const {
  dump(Str);
  Str << "\n";
}

void IceInstX8632Label::dump(IceOstream &Str) const {
  Str << getName(Str.Cfg) << ":";
}

void IceInstX8632Br::emit(IceOstream &Str, unsigned Option) const {
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
    Str << "\t" << Label->getName(Str.Cfg) << "\n";
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

void IceInstX8632Br::dump(IceOstream &Str) const {
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
        << (Label ? Label->getName(Str.Cfg) : getTargetFalse()->getName());
    return;
    break;
  }
  if (Label) {
    Str << ", label %" << Label->getName(Str.Cfg);
  } else {
    Str << ", label %" << getTargetTrue()->getName();
    if (getTargetFalse()) {
      Str << ", label %" << getTargetFalse()->getName();
    }
  }
}

void IceInstX8632Call::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 1);
  Str << "\tcall\t";
  getCallTarget()->emit(Str, Option);
  if (Tail)
    Str << "\t# tail";
  Str << "\n";
  Str.Cfg->getTarget()->resetStackAdjustment();
}

void IceInstX8632Call::dump(IceOstream &Str) const {
  if (getDest()) {
    dumpDest(Str);
    Str << " = ";
  }
  if (Tail)
    Str << "tail ";
  Str << "call " << getCallTarget();
}

static void emitTwoAddress(const char *Opcode, const IceInst *Inst,
                           IceOstream &Str, uint32_t Option,
                           bool ShiftHack = false) {
  assert(Inst->getSrcSize() == 2);
  assert(Inst->getDest() == Inst->getSrc(0));
  Str << "\t" << Opcode << "\t";
  Inst->getDest()->emit(Str, Option);
  Str << ", ";
  bool EmittedSrc1 = false;
  if (ShiftHack) {
    IceVariable *ShiftReg = llvm::dyn_cast<IceVariable>(Inst->getSrc(1));
    if (ShiftReg && ShiftReg->getRegNum() == IceTargetX8632::Reg_ecx) {
      Str << "cl";
      EmittedSrc1 = true;
    }
  }
  if (!EmittedSrc1)
    Inst->getSrc(1)->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Add::emit(IceOstream &Str, uint32_t Option) const {
  emitTwoAddress("add", this, Str, Option);
}

void IceInstX8632Add::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = add." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Adc::emit(IceOstream &Str, uint32_t Option) const {
  emitTwoAddress("adc", this, Str, Option);
}

void IceInstX8632Adc::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = adc." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Addss::emit(IceOstream &Str, uint32_t Option) const {
  emitTwoAddress(getDest()->getType() == IceType_f32 ? "addss" : "addsd", this,
                 Str, Option);
}

void IceInstX8632Addss::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = addss." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Sub::emit(IceOstream &Str, unsigned Option) const {
  emitTwoAddress("sub", this, Str, Option);
}

void IceInstX8632Sub::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = sub." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Subss::emit(IceOstream &Str, unsigned Option) const {
  emitTwoAddress(getDest()->getType() == IceType_f32 ? "subss" : "subsd", this,
                 Str, Option);
}

void IceInstX8632Subss::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = subss." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Sbb::emit(IceOstream &Str, unsigned Option) const {
  emitTwoAddress("sbb", this, Str, Option);
}

void IceInstX8632Sbb::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = sbb." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632And::emit(IceOstream &Str, uint32_t Option) const {
  emitTwoAddress("and", this, Str, Option);
}

void IceInstX8632And::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = and." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Or::emit(IceOstream &Str, uint32_t Option) const {
  emitTwoAddress("or", this, Str, Option);
}

void IceInstX8632Or::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = or." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Xor::emit(IceOstream &Str, uint32_t Option) const {
  emitTwoAddress("xor", this, Str, Option);
}

void IceInstX8632Xor::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = xor." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Imul::emit(IceOstream &Str, uint32_t Option) const {
  emitTwoAddress("imul", this, Str, Option);
}

void IceInstX8632Imul::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = imul." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Mul::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 2);
  assert(llvm::isa<IceVariable>(getSrc(0)));
  assert(llvm::dyn_cast<IceVariable>(getSrc(0))->getRegNum() ==
         IceTargetX8632::Reg_eax);
  assert(getDest()->getRegNum() == IceTargetX8632::Reg_eax); // TODO: allow edx?
  Str << "\tmul\t";
  getSrc(1)->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Mul::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = mul." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Mulss::emit(IceOstream &Str, uint32_t Option) const {
  emitTwoAddress(getDest()->getType() == IceType_f32 ? "mulss" : "mulsd", this,
                 Str, Option);
}

void IceInstX8632Mulss::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = mulss." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Idiv::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 3);
  Str << "\tidiv\t";
  getSrc(1)->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Idiv::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = idiv." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Div::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 3);
  Str << "\tdiv\t";
  getSrc(1)->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Div::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = div." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Divss::emit(IceOstream &Str, uint32_t Option) const {
  emitTwoAddress(getDest()->getType() == IceType_f32 ? "divss" : "divsd", this,
                 Str, Option);
}

void IceInstX8632Divss::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = divss." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Shl::emit(IceOstream &Str, uint32_t Option) const {
  bool ShiftHack = true;
  emitTwoAddress("shl", this, Str, Option, ShiftHack);
}

void IceInstX8632Shl::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = shl." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Shld::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 3);
  assert(getDest() == getSrc(0));
  Str << "\tshld\t";
  getDest()->emit(Str, Option);
  Str << ", ";
  getSrc(1)->emit(Str, Option);
  Str << ", ";
  bool ShiftHack = true;
  bool EmittedSrc1 = false;
  if (ShiftHack) {
    IceVariable *ShiftReg = llvm::dyn_cast<IceVariable>(getSrc(2));
    if (ShiftReg && ShiftReg->getRegNum() == IceTargetX8632::Reg_ecx) {
      Str << "cl";
      EmittedSrc1 = true;
    }
  }
  if (!EmittedSrc1)
    getSrc(2)->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Shld::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = shld." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Shr::emit(IceOstream &Str, uint32_t Option) const {
  bool ShiftHack = true;
  emitTwoAddress("shr", this, Str, Option, ShiftHack);
}

void IceInstX8632Shr::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = shr." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Shrd::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 3);
  assert(getDest() == getSrc(0));
  Str << "\tshrd\t";
  getDest()->emit(Str, Option);
  Str << ", ";
  getSrc(1)->emit(Str, Option);
  Str << ", ";
  bool ShiftHack = true;
  bool EmittedSrc1 = false;
  if (ShiftHack) {
    IceVariable *ShiftReg = llvm::dyn_cast<IceVariable>(getSrc(2));
    if (ShiftReg && ShiftReg->getRegNum() == IceTargetX8632::Reg_ecx) {
      Str << "cl";
      EmittedSrc1 = true;
    }
  }
  if (!EmittedSrc1)
    getSrc(2)->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Shrd::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = shrd." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Sar::emit(IceOstream &Str, uint32_t Option) const {
  bool ShiftHack = true;
  emitTwoAddress("sar", this, Str, Option, ShiftHack);
}

void IceInstX8632Sar::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = sar." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Cdq::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 1);
  Str << "cdq";
}

void IceInstX8632Cdq::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = cdq." << getSrc(0)->getType() << " ";
  dumpSources(Str);
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

void IceInstX8632Cvt::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 1);
  Str << "\tcvts" << getCvtTypeString(getSrc(0)->getType()) << "2s"
      << getCvtTypeString(getDest()->getType()) << "\t";
  getDest()->emit(Str, Option);
  Str << ", ";
  getSrc(0)->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Cvt::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = cvts" << getCvtTypeString(getSrc(0)->getType()) << "2s"
      << getCvtTypeString(getDest()->getType()) << " ";
  dumpSources(Str);
}

void IceInstX8632Icmp::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 2);
  Str << "\tcmp\t";
  getSrc(0)->emit(Str, Option);
  Str << ", ";
  getSrc(1)->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Icmp::dump(IceOstream &Str) const {
  Str << "cmp." << getSrc(0)->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Ucomiss::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 2);
  if (getSrc(0)->getType() == IceType_f32)
    Str << "\tucomiss\t";
  else
    Str << "\tucomisd\t";
  getSrc(0)->emit(Str, Option);
  Str << ", ";
  getSrc(1)->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Ucomiss::dump(IceOstream &Str) const {
  Str << "ucomiss." << getSrc(0)->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Test::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 2);
  Str << "\ttest\t";
  getSrc(0)->emit(Str, Option);
  Str << ", ";
  getSrc(1)->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Test::dump(IceOstream &Str) const {
  Str << "test." << getSrc(0)->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Store::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 2);
  Str << "\tmov\t";
  getSrc(1)->emit(Str, Option);
  Str << ", ";
  getSrc(0)->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Store::dump(IceOstream &Str) const {
  Str << "mov." << getSrc(0)->getType() << " " << getSrc(1) << ", "
      << getSrc(0);
}

void IceInstX8632Mov::emit(IceOstream &Str, uint32_t Option) const {
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
  getDest()->emit(Str, Option);
  Str << ", ";
  getSrc(0)->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Mov::dump(IceOstream &Str) const {
  Str << "mov." << getDest()->getType() << " ";
  dumpDest(Str);
  Str << ", ";
  dumpSources(Str);
}

void IceInstX8632Movsx::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 1);
  Str << "\tmovsx\t";
  getDest()->emit(Str, Option);
  Str << ", ";
  // TODO: Passing Option+1 is a hack to let the emitter selectively
  // use the proper 8- or 16-bit register name in some contexts but
  // not others.  The right way is to ensure in the lowering code that
  // all integer operands are treated as 32-bit except in very limited
  // cases like load, store, movsx, and movzx.
  getSrc(0)->emit(Str, Option + 1);
  Str << "\n";
}

void IceInstX8632Movsx::dump(IceOstream &Str) const {
  Str << "movs" << OpcodeTypeFromIceType(getSrc(0)->getType());
  Str << OpcodeTypeFromIceType(getDest()->getType());
  Str << " ";
  dumpDest(Str);
  Str << ", ";
  dumpSources(Str);
}

void IceInstX8632Movzx::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 1);
  Str << "\tmovzx\t";
  getDest()->emit(Str, Option);
  Str << ", ";
  getSrc(0)->emit(Str, Option + 1);
  Str << "\n";
}

void IceInstX8632Movzx::dump(IceOstream &Str) const {
  Str << "movz" << OpcodeTypeFromIceType(getSrc(0)->getType());
  Str << OpcodeTypeFromIceType(getDest()->getType());
  Str << " ";
  dumpDest(Str);
  Str << ", ";
  dumpSources(Str);
}

void IceInstX8632Fld::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 1);
  bool isDouble = (getSrc(0)->getType() == IceType_f64);
  IceVariable *Var = llvm::dyn_cast<IceVariable>(getSrc(0));
  if (Var && Var->getRegNum() >= 0) {
    // This is a physical xmm register, so we need to spill it to a
    // temporary stack slot.
    Str << "\tsub\tesp, " << (isDouble ? 8 : 4) << "\n";
    Str << "\tmovs" << (isDouble ? "d" : "s") << "\t" << (isDouble ? "q" : "d")
        << "word ptr [esp], ";
    Var->emit(Str, Option);
    Str << "\n";
    Str << "\tfld\t" << (isDouble ? "q" : "d") << "word ptr [esp]\n";
    Str << "\tadd\tesp, " << (isDouble ? 8 : 4) << "\n";
    return;
  }
  Str << "\tfld\t";
  getSrc(0)->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Fld::dump(IceOstream &Str) const {
  Str << "fld." << getSrc(0)->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Fstp::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 0);
  if (getDest() == NULL) {
    Str << "\tfstp\tst(0)\n";
    return;
  }
  if (getDest()->getRegNum() < 0) {
    Str << "\tfstp\t";
    getDest()->emit(Str, Option);
    Str << "\n";
    return;
  }
  // Dest is a physical (xmm) register, so st(0) needs to go through
  // memory.  Hack this by creating a temporary stack slot, spilling
  // st(0) there, loading it into the xmm register, and deallocating
  // the stack slot.
  unsigned Width = iceTypeWidth(getDest()->getType());
  Str << "\tsub\tesp, " << Width << "\n";
  Str << "\tfstp\t" << (Width == 8 ? "q" : "d") << "word ptr [esp]\n";
  Str << "\tmovs" << (Width == 8 ? "d" : "s") << "\t";
  getDest()->emit(Str, Option);
  Str << ", " << (Width == 8 ? "q" : "d") << "word ptr [esp]\n";
  Str << "\tadd\tesp, " << Width << "\n";
}

void IceInstX8632Fstp::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = fstp." << getDest()->getType() << ", st(0)";
  Str << "\n";
}

void IceInstX8632Pop::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 0);
  Str << "\tpop\t";
  getDest()->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Pop::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = pop." << getDest()->getType() << " ";
}

void IceInstX8632Push::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 1);
  IceType Type = getSrc(0)->getType();
  IceVariable *Var = llvm::dyn_cast<IceVariable>(getSrc(0));
  if ((Type == IceType_f32 || Type == IceType_f64) && Var &&
      Var->getRegNum() >= 0) {
    // The xmm registers can't be directly pushed, so we fake it by
    // decrementing esp and then storing to [esp].
    Str << "\tsub\tesp, " << iceTypeWidth(Type) << "\n";
    Str.Cfg->getTarget()->updateStackAdjustment(iceTypeWidth(Type));
    Str << "\tmov" << (Type == IceType_f32 ? "ss" : "sd") << "\t[esp], ";
    getSrc(0)->emit(Str, Option);
    Str << "\n";
  } else if (Type == IceType_f64 && (!Var || Var->getRegNum() < 0)) {
    // A double on the stack has to be pushed as two halves.  Push the
    // upper half followed by the lower half for little-endian.  TODO:
    // implement.
    assert(0 && "Missing support for pushing doubles from memory");
  } else {
    Str << "\tpush\t";
    getSrc(0)->emit(Str, Option);
    Str << "\n";
    Str.Cfg->getTarget()->updateStackAdjustment(4);
  }
}

void IceInstX8632Push::dump(IceOstream &Str) const {
  Str << "push." << getSrc(0)->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Ret::emit(IceOstream &Str, uint32_t Option) const {
  Str << "\tret\n";
}

void IceInstX8632Ret::dump(IceOstream &Str) const {
  IceType Type = (getSrcSize() == 0 ? IceType_void : getSrc(0)->getType());
  Str << "ret." << Type << " ";
  dumpSources(Str);
}

void IceOperandX8632::dump(IceOstream &Str) const {
  Str << "<IceOperandX8632>";
}

void IceOperandX8632Mem::emit(IceOstream &Str, uint32_t Option) const {
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
    Base->emit(Str, Option);
    Dumped = true;
  }
  if (Index) {
    assert(Base);
    Str << "+";
    if (Shift > 0)
      Str << (1u << Shift) << "*";
    Index->emit(Str, Option);
    Dumped = true;
  }
  // Pretty-print the Offset.
  bool OffsetIsZero = false;
  bool OffsetIsNegative = false;
  if (Offset == NULL) {
    OffsetIsZero = true;
  } else if (IceConstantInteger *CI =
                 llvm::dyn_cast<IceConstantInteger>(Offset)) {
    OffsetIsZero = (CI->getIntValue() == 0);
    OffsetIsNegative = (static_cast<int64_t>(CI->getIntValue()) < 0);
  }
  if (!OffsetIsZero) { // Suppress if Offset is known to be 0
    if (Dumped) {
      if (!OffsetIsNegative) // Suppress if Offset is known to be negative
        Str << "+";
    }
    Offset->emit(Str, Option);
  }
  Str << "]";
}

void IceOperandX8632Mem::dump(IceOstream &Str) const {
  bool Dumped = false;
  Str << "[";
  if (Base) {
    Str << Base;
    Dumped = true;
  }
  if (Index) {
    assert(Base);
    Str << "+";
    if (Shift > 0)
      Str << (1u << Shift) << "*";
    Str << Index;
    Dumped = true;
  }
  // Pretty-print the Offset.
  bool OffsetIsZero = false;
  bool OffsetIsNegative = false;
  if (Offset == NULL) {
    OffsetIsZero = true;
  } else if (IceConstantInteger *CI =
                 llvm::dyn_cast<IceConstantInteger>(Offset)) {
    OffsetIsZero = (CI->getIntValue() == 0);
    OffsetIsNegative = (static_cast<int64_t>(CI->getIntValue()) < 0);
  }
  if (!OffsetIsZero) { // Suppress if Offset is known to be 0
    if (Dumped) {
      if (!OffsetIsNegative) // Suppress if Offset is known to be negative
        Str << "+";
    }
    Str << Offset;
  }
  Str << "]";
}
