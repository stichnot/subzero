/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInst.h"
#include "IceInstX8632.h"
#include "IceOperand.h"
#include "IceRegManager.h"

IceString IceTargetX8632::RegNames[] = { "eax", "ecx", "edx", "ebx",
                                         "esp", "ebp", "esi", "edi", };

llvm::SmallBitVector IceTargetX8632::getRegisterMask(void) const {
  llvm::SmallBitVector Mask(Reg_NUM);
  Mask[Reg_eax] = true;
  Mask[Reg_ecx] = true;
  Mask[Reg_edx] = true;
  Mask[Reg_ebx] = true;
  // TODO: Disable ebp if Cfg has an alloca.
  Mask[Reg_ebp] = true;
  Mask[Reg_esi] = true;
  Mask[Reg_edi] = true;
  return Mask;
}

IceRegManager *IceTargetX8632::makeRegManager(IceCfgNode *Node) {
  const unsigned NumScratchReg = 3; // eax, ecx, edx
  // TODO: Optimize for extended basic blocks.
  RegManager = new IceRegManager(Cfg, Node, NumScratchReg);
  return RegManager;
}

IceInstTarget *IceTargetX8632::makeAssign(IceVariable *Dest, IceOperand *Src) {
  assert(Dest->getRegNum() >= 0);
  return new IceInstX8632Mov(Cfg, Dest, Src);
}

IceVariable *IceTargetX8632::getPhysicalRegister(unsigned RegNum) {
  assert(RegNum < PhysicalRegisters.size());
  IceVariable *Reg = PhysicalRegisters[RegNum];
  if (Reg == NULL) {
    Reg = Cfg->makeVariable(IceType_i32);
    Reg->setRegNum(RegNum);
    PhysicalRegisters[RegNum] = Reg;
  }
  return Reg;
}

IceInstList IceTargetX8632::lowerAlloca(const IceInstAlloca *Inst,
                                        const IceInst *Next,
                                        bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement
  return Expansion;
}

IceInstList IceTargetX8632::lowerArithmetic(const IceInstArithmetic *Inst,
                                            const IceInst *Next,
                                            bool &DeleteNextInst) {
  IceInstList Expansion;
  IceOpList Prefer;
  IceVarList Avoid;
  // TODO: Several instructions require specific physical
  // registers, namely div, rem, shift.  Loading into a physical
  // register requires killing all operands available in all
  // virtual registers, except that "ecx=r1" doesn't need to kill
  // operands available in r1.
  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = Inst->getSrc(0);
  IceOperand *Src1 = Inst->getSrc(1);
  IceVariable *Reg;
  IceInstTarget *NewInst;
  bool LRend0 = Inst->isLastUse(0);
  // Prefer Src0 if its live range is ending.
  if (LRend0) {
    Prefer.push_back(Src0);
  }
  if (IceVariable *Variable = llvm::dyn_cast<IceVariable>(Src1))
    Avoid.push_back(Variable);
  Reg = RegManager->getRegister(Dest->getType(), Prefer, Avoid);
  // Create "reg=Src0" if needed.
  if (!RegManager->registerContains(Reg, Src0)) {
    NewInst = new IceInstX8632Mov(Cfg, Reg, Src0);
    Expansion.push_back(NewInst);
    RegManager->notifyLoad(NewInst);
    NewInst->setRegState(RegManager);
  }
  // NewInst = new IceInstX8632Arithmetic(Cfg, Inst->getOp(), Reg, Src1);
  // TODO: Operator-specific instruction instead of Add.
  NewInst = new IceInstX8632Add(Cfg, Reg, Src1);
  Expansion.push_back(NewInst);
  RegManager->notifyLoad(NewInst, false);
  NewInst->setRegState(RegManager);
  NewInst = new IceInstX8632Mov(Cfg, Dest, Reg);
  Expansion.push_back(NewInst);
  RegManager->notifyStore(NewInst);
  NewInst->setRegState(RegManager);
  return Expansion;
}

IceInstList IceTargetX8632::lowerAssign(const IceInstAssign *Inst,
                                        const IceInst *Next,
                                        bool &DeleteNextInst) {
  IceInstList Expansion;
  IceOpList Prefer;
  IceVarList Avoid;
  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = Inst->getSrc(0);
  IceVariable *Reg;
  IceInstTarget *NewInst;
  Prefer.push_back(Src0);
  Reg = RegManager->getRegister(Dest->getType(), Prefer, Avoid);
  if (Dest->getRegNum() >= 0) {
    if (RegManager->registerContains(Reg, Src0)) {
      Src0 = Reg;
    }
    NewInst = new IceInstX8632Mov(Cfg, Dest, Src0);
    Expansion.push_back(NewInst);
    // TODO: commenting out the notifyLoad() call because Dest
    // doesn't actually belong to the current RegManager and
    // therefore it asserts.  Maybe this is the right thing to do,
    // but we also have to consider the code selection for
    // globally register allocated variables.
    // RegManager->notifyLoad(NewInst);
    NewInst->setRegState(RegManager);
  } else {
    if (!RegManager->registerContains(Reg, Src0)) {
      NewInst = new IceInstX8632Mov(Cfg, Reg, Src0);
      Expansion.push_back(NewInst);
      RegManager->notifyLoad(NewInst);
      NewInst->setRegState(RegManager);
    }
    NewInst = new IceInstX8632Mov(Cfg, Dest, Reg);
    Expansion.push_back(NewInst);
    RegManager->notifyStore(NewInst);
    NewInst->setRegState(RegManager);
  }
  return Expansion;
}

IceInstList IceTargetX8632::lowerBr(const IceInstBr *Inst, const IceInst *Next,
                                    bool &DeleteNextInst) {
  IceInstList Expansion;
  // assert(0); // TODO: implement
  return Expansion;
}

IceInstList IceTargetX8632::lowerCall(const IceInstCall *Inst,
                                      const IceInst *Next,
                                      bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement
  return Expansion;
}

IceInstList IceTargetX8632::lowerCast(const IceInstCast *Inst,
                                      const IceInst *Next,
                                      bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement
  return Expansion;
}

IceInstList IceTargetX8632::lowerFcmp(const IceInstFcmp *Inst,
                                      const IceInst *Next,
                                      bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement
  return Expansion;
}

IceInstList IceTargetX8632::lowerIcmp(const IceInstIcmp *Inst,
                                      const IceInst *Next,
                                      bool &DeleteNextInst) {
  IceInstList Expansion;
  IceInstTarget *NewInst;
  IceOpList Prefer;
  IceVarList Avoid;
  // For now, require that the following instruction is a branch
  // based on the last use of this instruction's Dest operand.
  // TODO: Fix this.
  if (llvm::isa<IceInstBr>(Next) && Inst->getDest() == Next->getSrc(0)) {
    const IceInstBr *NextBr = llvm::cast<IceInstBr>(Next);
    // This is basically identical to an Arithmetic instruction,
    // except there is no Dest variable to store.
    IceOperand *Src0 = Inst->getSrc(0);
    IceOperand *Src1 = Inst->getSrc(1);
    IceOperand *RegSrc = Src0;
    if (!llvm::isa<IceVariable>(Src0) ||
        llvm::dyn_cast<IceVariable>(Src0)->getRegNum() < 0) {
      Prefer.push_back(Src0);
      if (IceVariable *Variable = llvm::dyn_cast<IceVariable>(Src1))
        Avoid.push_back(Variable);
      IceVariable *Reg =
          RegManager->getRegister(Src0->getType(), Prefer, Avoid);
      RegSrc = Reg;
      // Create "reg=Src0" if needed.
      if (!RegManager->registerContains(Reg, Src0)) {
        if (llvm::isa<IceConstant>(Src1)) {
          RegSrc = Src0;
        } else {
          NewInst = new IceInstX8632Mov(Cfg, Reg, Src0);
          Expansion.push_back(NewInst);
          RegManager->notifyLoad(NewInst);
          NewInst->setRegState(RegManager);
        }
      }
    }
    NewInst = new IceInstX8632Icmp(Cfg, RegSrc, Src1);
    Expansion.push_back(NewInst);
    NewInst->setRegState(RegManager);
    NewInst =
        new IceInstX8632Br(Cfg, NextBr->getTargetTrue(),
                           NextBr->getTargetFalse(), Inst->getCondition());
    Expansion.push_back(NewInst);
    DeleteNextInst = true;
  } else {
    assert(0);
  }
  return Expansion;
}

IceInstList IceTargetX8632::lowerLoad(const IceInstLoad *Inst,
                                      const IceInst *Next,
                                      bool &DeleteNextInst) {
  IceInstList Expansion;
  IceInstTarget *NewInst;
  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = Inst->getSrc(0); // Base
  IceOperand *Src1 = Inst->getSrc(1); // Index - could be NULL
  IceOperand *Src2 = Inst->getSrc(2); // Shift - constant
  IceOperand *Src3 = Inst->getSrc(3); // Offset - constant
  IceOpList Prefer;
  IceVarList Avoid;
  IceVariable *Reg1 = llvm::dyn_cast<IceVariable>(Src0);
  if (Reg1 == NULL || Reg1->getRegNum() < 0) {
    Prefer.push_back(Src0);
    if (IceVariable *Variable = llvm::dyn_cast_or_null<IceVariable>(Src1))
      Avoid.push_back(Variable);
    Reg1 = RegManager->getRegister(Dest->getType(), Prefer, Avoid);
    if (!RegManager->registerContains(Reg1, Src0)) {
      NewInst = new IceInstX8632Mov(Cfg, Reg1, Src0);
      Expansion.push_back(NewInst);
      RegManager->notifyLoad(NewInst);
      NewInst->setRegState(RegManager);
    }
  }
  IceVariable *Reg2 = llvm::dyn_cast_or_null<IceVariable>(Src1);
  if (Src1 && (Reg2 == NULL || Reg2->getRegNum() < 0)) {
    Prefer.clear();
    Avoid.clear();
    Prefer.push_back(Src1);
    Avoid.push_back(Reg1);
    Reg2 = RegManager->getRegister(Dest->getType(), Prefer, Avoid);
    if (!RegManager->registerContains(Reg2, Src1)) {
      NewInst = new IceInstX8632Mov(Cfg, Reg2, Src1);
      Expansion.push_back(NewInst);
      RegManager->notifyLoad(NewInst);
      NewInst->setRegState(RegManager);
    }
  }
  IceVariable *Reg = Dest;
  if (Dest->getRegNum() < 0) {
    Prefer.clear();
    Avoid.clear();
    Avoid.push_back(Reg1);
    Avoid.push_back(Reg2);
    Reg = RegManager->getRegister(Dest->getType(), Prefer, Avoid);
  }
  NewInst = new IceInstX8632Load(Cfg, Reg, Reg1, Reg2, Src2, Src3);
  Expansion.push_back(NewInst);
  if (Reg != Dest) // TODO: clean this up
    RegManager->notifyLoad(NewInst, false);
  NewInst->setRegState(RegManager);
  if (Reg != Dest) {
    NewInst = new IceInstX8632Mov(Cfg, Dest, Reg);
    Expansion.push_back(NewInst);
    RegManager->notifyStore(NewInst);
    NewInst->setRegState(RegManager);
  }
  return Expansion;
}

IceInstList IceTargetX8632::lowerPhi(const IceInstPhi *Inst,
                                     const IceInst *Next,
                                     bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement if necessary
  return Expansion;
}

IceInstList IceTargetX8632::lowerRet(const IceInstRet *Inst,
                                     const IceInst *Next,
                                     bool &DeleteNextInst) {
  IceInstList Expansion;
  IceInstTarget *NewInst;
  IceOperand *Src0 = Inst->getSrc(0);
  IceVariable *Reg;
  if (Src0) {
    IceOpList Prefer;
    IceVarList Avoid;
    Prefer.push_back(Src0);
    Reg = RegManager->getRegister(Src0->getType(), Prefer, Avoid);
    if (!RegManager->registerContains(Reg, Src0)) {
      NewInst = new IceInstX8632Mov(Cfg, Reg, Src0);
      Expansion.push_back(NewInst);
      RegManager->notifyLoad(NewInst);
      NewInst->setRegState(RegManager);
    }
  } else
    Reg = NULL;
  NewInst = new IceInstX8632Ret(Cfg, Reg);
  Expansion.push_back(NewInst);
  return Expansion;
}

IceInstList IceTargetX8632::lowerSelect(const IceInstSelect *inst,
                                        const IceInst *Next,
                                        bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement
  return Expansion;
}

IceInstList IceTargetX8632::lowerStore(const IceInstStore *Inst,
                                       const IceInst *Next,
                                       bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement
  return Expansion;
}

IceInstList IceTargetX8632::lowerSwitch(const IceInstSwitch *Inst,
                                        const IceInst *Next,
                                        bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement
  return Expansion;
}

IceInstX8632Add::IceInstX8632Add(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstTarget(Cfg, 2) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Sub::IceInstX8632Sub(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstTarget(Cfg, 2) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
}

IceInstX8632And::IceInstX8632And(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstTarget(Cfg, 2) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Or::IceInstX8632Or(IceCfg *Cfg, IceVariable *Dest,
                               IceOperand *Source)
    : IceInstTarget(Cfg, 2) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Xor::IceInstX8632Xor(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstTarget(Cfg, 2) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Imul::IceInstX8632Imul(IceCfg *Cfg, IceVariable *Dest,
                                   IceOperand *Source)
    : IceInstTarget(Cfg, 2) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Idiv::IceInstX8632Idiv(IceCfg *Cfg, IceVariable *Dest,
                                   IceOperand *Source, IceVariable *Other)
    : IceInstTarget(Cfg, 3) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
  addSource(Other);
}

IceInstX8632Div::IceInstX8632Div(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source, IceVariable *Other)
    : IceInstTarget(Cfg, 3) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
  addSource(Other);
}

IceInstX8632Shl::IceInstX8632Shl(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstTarget(Cfg, 2) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Shr::IceInstX8632Shr(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstTarget(Cfg, 2) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Sar::IceInstX8632Sar(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstTarget(Cfg, 2) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Br::IceInstX8632Br(IceCfg *Cfg, IceCfgNode *TargetTrue,
                               IceCfgNode *TargetFalse,
                               IceInstIcmp::IceICond Condition)
    : IceInstTarget(Cfg, 0), Condition(Condition), TargetTrue(TargetTrue),
      TargetFalse(TargetFalse) {}

IceInstX8632Call::IceInstX8632Call(IceCfg *Cfg, IceVariable *Dest,
                                   IceOperand *CallTarget, bool Tail)
    : IceInstTarget(Cfg, 0), CallTarget(CallTarget), Tail(Tail) {
  // TODO: CallTarget should be another source operand.
  if (Dest)
    addDest(Dest);
}

IceInstX8632Cdq::IceInstX8632Cdq(IceCfg *Cfg, IceVariable *Dest,
                                 IceVariable *Source)
    : IceInstTarget(Cfg, 1) {
  assert(Dest->getRegNum() == IceTargetX8632::Reg_edx);
  assert(Source->getRegNum() == IceTargetX8632::Reg_eax);
  addDest(Dest);
  addSource(Source);
}

IceInstX8632Icmp::IceInstX8632Icmp(IceCfg *Cfg, IceOperand *Src0,
                                   IceOperand *Src1)
    : IceInstTarget(Cfg, 2) {
  addSource(Src0);
  addSource(Src1);
}

IceInstX8632Load::IceInstX8632Load(IceCfg *Cfg, IceVariable *Dest,
                                   IceOperand *Src0, IceOperand *Src1,
                                   IceOperand *Src2, IceOperand *Src3)
    : IceInstTarget(Cfg, 4) {
  addDest(Dest);
  addSource(Src0);
  addSource(Src1);
  addSource(Src2);
  addSource(Src3);
}

IceInstX8632Store::IceInstX8632Store(IceCfg *Cfg, IceOperand *Value,
                                     IceOperand *Base, IceOperand *Index,
                                     IceOperand *Shift, IceOperand *Offset)
    : IceInstTarget(Cfg, 5) {
  addSource(Value);
  addSource(Base);
  addSource(Index);
  addSource(Shift);
  addSource(Offset);
}

IceInstX8632Mov::IceInstX8632Mov(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstTarget(Cfg, 1) {
  addDest(Dest);
  addSource(Source);
}

IceInstX8632Push::IceInstX8632Push(IceCfg *Cfg, IceOperand *Source)
    : IceInstTarget(Cfg, 1) {
  addSource(Source);
}

bool IceInstX8632Mov::isRedundantAssign(void) const {
  int DestRegNum = getDest()->getRegNum();
  if (DestRegNum < 0)
    return false;
  IceVariable *Src = llvm::dyn_cast<IceVariable>(getSrc(0));
  if (Src == NULL)
    return false;
  return DestRegNum == Src->getRegNum();
}

IceInstX8632Ret::IceInstX8632Ret(IceCfg *Cfg, IceVariable *Source)
    : IceInstTarget(Cfg, Source ? 1 : 0) {
  if (Source)
    addSource(Source);
}

// ======================== Dump routines ======================== //

void IceInstX8632Br::dump(IceOstream &Str) const {
  Str << "br ";
  switch (Condition) {
  case IceInstIcmp::Eq:
    Str << "eq";
    break;
  case IceInstIcmp::Ne:
    Str << "ne";
    break;
  case IceInstIcmp::Ugt:
    Str << "ugt";
    break;
  case IceInstIcmp::Uge:
    Str << "uge";
    break;
  case IceInstIcmp::Ult:
    Str << "ult";
    break;
  case IceInstIcmp::Ule:
    Str << "ule";
    break;
  case IceInstIcmp::Sgt:
    Str << "sgt";
    break;
  case IceInstIcmp::Sge:
    Str << "sge";
    break;
  case IceInstIcmp::Slt:
    Str << "slt";
    break;
  case IceInstIcmp::Sle:
    Str << "sle";
    break;
  }
  Str << ", label %" << getTargetTrue()->getName() << ", label %"
      << getTargetFalse()->getName();
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

void IceInstX8632Add::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = add." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Sub::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = sub." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632And::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = and." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Or::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = or." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Xor::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = xor." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Imul::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = imul." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Idiv::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = idiv." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Div::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = div." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Shl::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = shl." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Shr::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = shr." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Sar::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = sar." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Cdq::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = cdq." << getSrc(0)->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Icmp::dump(IceOstream &Str) const {
  Str << "cmp." << getSrc(0)->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Load::dump(IceOstream &Str) const {
  Str << "mov." << getDest()->getType() << " ";
  dumpDest(Str);
  Str << ", [";
  dumpSources(Str);
  Str << "]";
}

void IceInstX8632Store::dump(IceOstream &Str) const {
  Str << "mov." << getSrc(0)->getType() << " ";
  Str << "[";
  Str << getSrc(1);
  Str << ", ";
  Str << getSrc(2);
  Str << ", ";
  Str << getSrc(3);
  Str << ", ";
  Str << getSrc(4);
  Str << "], ";
  Str << getSrc(0);
}

void IceInstX8632Mov::dump(IceOstream &Str) const {
  Str << "mov." << getDest()->getType() << " ";
  dumpDest(Str);
  Str << ", ";
  dumpSources(Str);
}

void IceInstX8632Push::dump(IceOstream &Str) const {
  Str << "push." << getSrc(0)->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Ret::dump(IceOstream &Str) const {
  IceType Type = (getSrcSize() == 0 ? IceType_void : getSrc(0)->getType());
  Str << "ret." << Type << " ";
  dumpSources(Str);
}

////////////////////////////////////////////////////////////////

llvm::SmallBitVector IceTargetX8632S::getRegisterMask(void) const {
  llvm::SmallBitVector Mask(Reg_NUM);
  Mask[Reg_eax] = true;
  Mask[Reg_ecx] = true;
  Mask[Reg_edx] = true;
  Mask[Reg_ebx] = true;
  // TODO: Disable ebp if Cfg has an alloca.
  Mask[Reg_ebp] = true;
  Mask[Reg_esi] = true;
  Mask[Reg_edi] = true;
  return Mask;
}

IceInstTarget *IceTargetX8632S::makeAssign(IceVariable *Dest, IceOperand *Src) {
  assert(Dest->getRegNum() >= 0);
  return new IceInstX8632Mov(Cfg, Dest, Src);
}

IceInstList IceTargetX8632S::lowerAlloca(const IceInstAlloca *Inst,
                                         const IceInst *Next,
                                         bool &DeleteNextInst) {
  IceInstList Expansion;
  // TODO: implement
  Cfg->setError("Alloca lowering not implemented");
  return Expansion;
}

IceInstList IceTargetX8632S::lowerArithmetic(const IceInstArithmetic *Inst,
                                             const IceInst *Next,
                                             bool &DeleteNextInst) {
  IceInstList Expansion;
  /*
    +-----------+-------+----------+-------+--------+-----+--------------+
    | a = b ? c | t1?b  | t0:edx=? | t2?c  | t1?=t2 | a=? | Note         |
    |-----------+-------+----------+-------+--------+-----+--------------|
    | Add       | =     |          | :=    | add    | t1  |              |
    | Sub       | =     |          | :=    | sub    | t1  |              |
    | And       | =     |          | :=    | and    | t1  |              |
    | Or        | =     |          | :=    | or     | t1  |              |
    | Xor       | =     |          | :=    | xor    | t1  |              |
    | Mul c:imm |       |          | :=    | [note] | t1  | t1=imul b,t2 |
    | Mul       | =     |          | :=    | imul   | t1  |              |
    | Sdiv      | :eax= | cdq t1   | :=    | idiv   | t1  |              |
    | Srem      | :eax= | cdq t1   | :=    | idiv   | t0  |              |
    | Udiv      | :eax= | 0        | :=    | div    | t1  |              |
    | Urem      | :eax= | 0        | :=    | div    | t0  |              |
    | Shl       | =     |          | :ecx= | shl    | t1  |              |
    | Lshr      | =     |          | :ecx= | shr    | t1  |              |
    | Ashr      | =     |          | :ecx= | sar    | t1  |              |
    +-----------+-------+----------+-------+--------+-----+--------------+
    TODO: Fadd, Fsub, Fmul, Fdiv
  */

  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = Inst->getSrc(0);
  IceOperand *Src1 = Inst->getSrc(1);
  IceVariable *Reg0 = NULL;
  IceVariable *Reg1 = NULL;
  IceOperand *Reg2 = Src1;
  uint64_t Zero = 0;
  bool PrecolorReg1WithEax = false;
  bool ZeroExtendReg1 = false;
  bool SignExtendReg1 = false;
  bool Reg2InEcx = false;
  bool ResultFromReg0 = false;
  IceInstTarget *NewInst;
  switch (Inst->getOp()) {
  case IceInstArithmetic::Add:
  case IceInstArithmetic::And:
  case IceInstArithmetic::Or:
  case IceInstArithmetic::Xor:
  case IceInstArithmetic::Sub:
    break;
  case IceInstArithmetic::Mul:
    // TODO: Optimized for llvm::isa<IceConstant>(Src1)
    break;
  case IceInstArithmetic::Udiv:
    PrecolorReg1WithEax = true;
    ZeroExtendReg1 = true;
    break;
  case IceInstArithmetic::Sdiv:
    PrecolorReg1WithEax = true;
    SignExtendReg1 = true;
    break;
  case IceInstArithmetic::Urem:
    PrecolorReg1WithEax = true;
    ZeroExtendReg1 = true;
    ResultFromReg0 = true;
    break;
  case IceInstArithmetic::Srem:
    PrecolorReg1WithEax = true;
    SignExtendReg1 = true;
    ResultFromReg0 = true;
    break;
  case IceInstArithmetic::Shl:
  case IceInstArithmetic::Lshr:
  case IceInstArithmetic::Ashr:
    Reg2InEcx = true;
    break;
  case IceInstArithmetic::Fadd:
  case IceInstArithmetic::Fsub:
  case IceInstArithmetic::Fmul:
  case IceInstArithmetic::Fdiv:
  case IceInstArithmetic::Frem:
    // TODO: implement
    Cfg->setError("FP arithmetic lowering not implemented");
    return Expansion;
    break;
  case IceInstArithmetic::OpKind_NUM:
    assert(0);
    break;
  }

  // Assign t1.
  Reg1 = Cfg->makeVariable(Dest->getType());
  Reg1->setWeightInfinite();
  if (PrecolorReg1WithEax) {
    Reg1->setRegNum(Reg_eax);
  } else {
    Reg1->setPreferredRegister(llvm::dyn_cast<IceVariable>(Src0), false);
  }
  NewInst = new IceInstX8632Mov(Cfg, Reg1, Src0);
  Expansion.push_back(NewInst);

  // Assign t0:edx.
  if (ZeroExtendReg1 || SignExtendReg1) {
    Reg0 = Cfg->makeVariable(IceType_i32);
    Reg0->setRegNum(Reg_edx);
    if (ZeroExtendReg1)
      NewInst =
          new IceInstX8632Mov(Cfg, Reg0, Cfg->getConstant(IceType_i32, Zero));
    else
      NewInst = new IceInstX8632Cdq(Cfg, Reg0, Reg1);
    Expansion.push_back(NewInst);
  }

  // Assign t2.
  if (Reg2InEcx) {
    IceVariable *Reg = Cfg->makeVariable(Src1->getType());
    Reg->setRegNum(Reg_ecx);
    NewInst = new IceInstX8632Mov(Cfg, Reg, Src1);
    Expansion.push_back(NewInst);
    Reg2 = Reg;
  }

  // Generate the arithmetic instruction.
  switch (Inst->getOp()) {
  case IceInstArithmetic::Add:
    NewInst = new IceInstX8632Add(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::And:
    NewInst = new IceInstX8632And(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Or:
    NewInst = new IceInstX8632Or(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Xor:
    NewInst = new IceInstX8632Xor(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Sub:
    NewInst = new IceInstX8632Sub(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Mul:
    // TODO: Optimized for llvm::isa<IceConstant>(Src1)
    NewInst = new IceInstX8632Imul(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Udiv:
    NewInst = new IceInstX8632Div(Cfg, Reg1, Reg2, Reg0);
    break;
  case IceInstArithmetic::Sdiv:
    NewInst = new IceInstX8632Idiv(Cfg, Reg1, Reg2, Reg0);
    break;
  case IceInstArithmetic::Urem:
    NewInst = new IceInstX8632Div(Cfg, Reg0, Reg2, Reg1);
    break;
  case IceInstArithmetic::Srem:
    NewInst = new IceInstX8632Idiv(Cfg, Reg0, Reg2, Reg1);
    break;
  case IceInstArithmetic::Shl:
    NewInst = new IceInstX8632Shl(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Lshr:
    NewInst = new IceInstX8632Shr(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Ashr:
    NewInst = new IceInstX8632Sar(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Fadd:
  case IceInstArithmetic::Fsub:
  case IceInstArithmetic::Fmul:
  case IceInstArithmetic::Fdiv:
  case IceInstArithmetic::Frem:
    // TODO: implement
    Cfg->setError("FP arithmetic lowering not implemented");
    return Expansion;
    break;
  case IceInstArithmetic::OpKind_NUM:
    assert(0);
    break;
  }
  Expansion.push_back(NewInst);

  // Assign Dest.
  NewInst = new IceInstX8632Mov(Cfg, Dest, (ResultFromReg0 ? Reg0 : Reg1));
  Expansion.push_back(NewInst);

  return Expansion;
}

IceInstList IceTargetX8632S::lowerAssign(const IceInstAssign *Inst,
                                         const IceInst *Next,
                                         bool &DeleteNextInst) {
  IceInstList Expansion;
  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = Inst->getSrc(0);
  IceVariable *Reg;
  IceInstTarget *NewInst;
  // a=b ==> t=b; a=t; (link t->b)
  Reg = Cfg->makeVariable(Dest->getType());
  Reg->setWeightInfinite();
  Reg->setPreferredRegister(llvm::dyn_cast<IceVariable>(Src0), true);
  NewInst = new IceInstX8632Mov(Cfg, Reg, Src0);
  Expansion.push_back(NewInst);
  NewInst = new IceInstX8632Mov(Cfg, Dest, Reg);
  Expansion.push_back(NewInst);
  return Expansion;
}

IceInstList IceTargetX8632S::lowerBr(const IceInstBr *Inst, const IceInst *Next,
                                     bool &DeleteNextInst) {
  IceInstList Expansion;
  if (Inst->getTargetTrue())
    Cfg->setError("Conditional branch lowering unimplemented");
  return Expansion;
}

IceInstList IceTargetX8632S::lowerCall(const IceInstCall *Inst,
                                       const IceInst *Next,
                                       bool &DeleteNextInst) {
  // TODO: what to do about tailcalls?
  IceInstList Expansion;
  IceInstTarget *NewInst;
  // Generate a sequence of push instructions, pushing right to left,
  // keeping track of stack offsets in case a push involves a stack
  // operand and we are using an esp-based frame.
  uint32_t StackOffset = 0;
  // TODO: If for some reason the call instruction gets dead-code
  // eliminated after lowering, we would need to ensure that the
  // pre-call push instructions and the post-call esp adjustment get
  // eliminated as well.
  for (unsigned NumArgs = Inst->getSrcSize(), i = 0; i < NumArgs; ++i) {
    IceOperand *Arg = Inst->getSrc(NumArgs - i - 1);
    assert(Arg);
    NewInst = new IceInstX8632Push(Cfg, Arg);
    // TODO: Where in the Cfg is StackOffset tracked?  It is needed
    // for instructions that push esp-based stack variables as
    // arguments.
    Expansion.push_back(NewInst);
    StackOffset += iceTypeWidth(Arg->getType());
  }
  // Generate the call instruction.  Assign its result to a temporary
  // with high register allocation weight.
  IceVariable *Dest = Inst->getDest();
  IceVariable *Reg = NULL;
  if (Dest) {
    Reg = Cfg->makeVariable(Dest->getType());
    Reg->setWeightInfinite();
    // TODO: Reg_eax is only for 32-bit integer results.  For floating
    // point, we need the appropriate FP register.  For a 64-bit
    // integer result, after the Kill instruction add a
    // "tmp:edx=FakeDef(Reg)" instruction for a call that can be
    // dead-code eliminated, and "tmp:edx=FakeDef()" for a call that
    // can't be eliminated.
    Reg->setRegNum(Reg_eax);
  }
  NewInst =
      new IceInstX8632Call(Cfg, Reg, Inst->getCallTarget(), Inst->isTail());
  Expansion.push_back(NewInst);
  IceInst *NewCall = NewInst;

  // Insert some sort of register-kill pseudo instruction.
  IceVarList KilledRegs;
  KilledRegs.push_back(Cfg->getTarget()->getPhysicalRegister(Reg_eax));
  KilledRegs.push_back(Cfg->getTarget()->getPhysicalRegister(Reg_ecx));
  KilledRegs.push_back(Cfg->getTarget()->getPhysicalRegister(Reg_edx));
  IceInst *Kill = new IceInstFakeKill(Cfg, KilledRegs, NewCall);
  Expansion.push_back(Kill);

  // Generate a FakeUse to keep the call live if necessary.
  bool HasSideEffects = true;
  // TODO: set HasSideEffects=false if it's a known intrinsic without
  // side effects.
  if (HasSideEffects && Reg) {
    IceInst *FakeUse = new IceInstFakeUse(Cfg, Reg);
    Expansion.push_back(FakeUse);
  }

  // Generate Dest=Reg assignment.
  if (Dest) {
    Dest->setPreferredRegister(Reg, false);
    NewInst = new IceInstX8632Mov(Cfg, Dest, Reg);
    Expansion.push_back(NewInst);
  }

  // Add the appropriate offset to esp.
  if (StackOffset) {
    IceVariable *Esp = Cfg->getTarget()->getPhysicalRegister(Reg_esp);
    NewInst = new IceInstX8632Add(Cfg, Esp,
                                  Cfg->getConstant(IceType_i32, StackOffset));
    Expansion.push_back(NewInst);
  }

  return Expansion;
}

IceInstList IceTargetX8632S::lowerCast(const IceInstCast *Inst,
                                       const IceInst *Next,
                                       bool &DeleteNextInst) {
  IceInstList Expansion;
  Cfg->setError("Cast lowering unimplemented");
  return Expansion;
}

IceInstList IceTargetX8632S::lowerFcmp(const IceInstFcmp *Inst,
                                       const IceInst *Next,
                                       bool &DeleteNextInst) {
  IceInstList Expansion;
  Cfg->setError("Fcmp lowering unimplemented");
  return Expansion;
}

IceInstList IceTargetX8632S::lowerIcmp(const IceInstIcmp *Inst,
                                       const IceInst *Next,
                                       bool &DeleteNextInst) {
  IceInstList Expansion;
  IceInstTarget *NewInst;
  // For now, require that the following instruction is a branch
  // based on the last use of this instruction's Dest operand.
  // TODO: Fix this.
  if (llvm::isa<IceInstBr>(Next) && Inst->getDest() == Next->getSrc(0)) {
    const IceInstBr *NextBr = llvm::cast<IceInstBr>(Next);
    // This is basically identical to an Arithmetic instruction,
    // except there is no Dest variable to store.
    // cmp a,b ==> mov t,a; cmp t,b
    IceOperand *Src0 = Inst->getSrc(0);
    IceOperand *Src1 = Inst->getSrc(1);
    IceVariable *Reg = Cfg->makeVariable(Src0->getType());
    Reg->setWeightInfinite();
    Reg->setPreferredRegister(llvm::dyn_cast<IceVariable>(Src0), true);
    NewInst = new IceInstX8632Mov(Cfg, Reg, Src0);
    Expansion.push_back(NewInst);
    NewInst = new IceInstX8632Icmp(Cfg, Reg, Src1);
    Expansion.push_back(NewInst);
    NewInst =
        new IceInstX8632Br(Cfg, NextBr->getTargetTrue(),
                           NextBr->getTargetFalse(), Inst->getCondition());
    Expansion.push_back(NewInst);
    DeleteNextInst = true;
  } else {
    Cfg->setError("Icmp lowering without subsequent Br unimplemented");
  }
  return Expansion;
}

IceInstList IceTargetX8632S::lowerLoad(const IceInstLoad *Inst,
                                       const IceInst *Next,
                                       bool &DeleteNextInst) {
  IceInstList Expansion;
  IceInstTarget *NewInst;
  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = Inst->getSrc(0); // Base
  IceOperand *Src1 = Inst->getSrc(1); // Index - could be NULL
  IceOperand *Src2 = Inst->getSrc(2); // Shift - constant
  IceOperand *Src3 = Inst->getSrc(3); // Offset - constant
  assert(Src0 != NULL /* && llvm::isa<IceVariable>(Src0)*/);
  assert(Src1 == NULL || llvm::isa<IceVariable>(Src1));
  assert(Src2 == NULL || llvm::isa<IceConstant>(Src2));
  assert(Src3 == NULL || llvm::isa<IceConstant>(Src3));
  // dest=load[base,index,shift,offset] ==>
  // t0=base; t1=index; t2=load[base,index,shift,offset]; dest=t2
  IceVariable *Reg0 = Cfg->makeVariable(Src0->getType());
  Reg0->setWeightInfinite();
  Reg0->setPreferredRegister(llvm::dyn_cast<IceVariable>(Src0), true);
  NewInst = new IceInstX8632Mov(Cfg, Reg0, Src0);
  Expansion.push_back(NewInst);
  IceVariable *Reg1 = NULL;
  if (Src1) {
    Reg1 = Cfg->makeVariable(Src1->getType());
    Reg1->setWeightInfinite();
    Reg1->setPreferredRegister(llvm::dyn_cast<IceVariable>(Src1), true);
    NewInst = new IceInstX8632Mov(Cfg, Reg1, Src1);
    Expansion.push_back(NewInst);
  }
  IceVariable *Reg2 = Cfg->makeVariable(Dest->getType());
  Reg2->setWeightInfinite();
  NewInst = new IceInstX8632Load(Cfg, Reg2, Reg0, Reg1, Src2, Src3);
  Expansion.push_back(NewInst);
  NewInst = new IceInstX8632Mov(Cfg, Dest, Reg2);
  Expansion.push_back(NewInst);

  return Expansion;
}

IceInstList IceTargetX8632S::lowerPhi(const IceInstPhi *Inst,
                                      const IceInst *Next,
                                      bool &DeleteNextInst) {
  IceInstList Expansion;
  Cfg->setError("Phi lowering not implemented");
  return Expansion;
}

IceInstList IceTargetX8632S::lowerRet(const IceInstRet *Inst,
                                      const IceInst *Next,
                                      bool &DeleteNextInst) {
  IceInstList Expansion;
  IceInstTarget *NewInst;
  IceOperand *Src0 = Inst->getSrc(0);
  IceVariable *Reg = NULL;
  if (Src0) {
    Reg = Cfg->makeVariable(Src0->getType());
    Reg->setWeightInfinite();
    Reg->setRegNum(Reg_eax);
    NewInst = new IceInstX8632Mov(Cfg, Reg, Src0);
    Expansion.push_back(NewInst);
  }
  NewInst = new IceInstX8632Ret(Cfg, Reg);
  Expansion.push_back(NewInst);
  // Add a fake use of esp to make sure esp stays alive for the entire
  // function.  Otherwise post-call esp adjustments get dead-code
  // eliminated.  TODO: Are there more places where the fake use
  // should be inserted?  E.g. "void f(int n){while(1) g(n);}" may not
  // have a ret instruction.
  IceVariable *Esp = Cfg->getTarget()->getPhysicalRegister(Reg_esp);
  IceInst *FakeUse = new IceInstFakeUse(Cfg, Esp);
  Expansion.push_back(FakeUse);
  return Expansion;
}

IceInstList IceTargetX8632S::lowerSelect(const IceInstSelect *Inst,
                                         const IceInst *Next,
                                         bool &DeleteNextInst) {
  IceInstList Expansion;
  Cfg->setError("Select lowering not implemented");
  return Expansion;
}

IceInstList IceTargetX8632S::lowerStore(const IceInstStore *Inst,
                                        const IceInst *Next,
                                        bool &DeleteNextInst) {
  IceInstList Expansion;

  IceInstTarget *NewInst;
  IceOperand *Value = Inst->getSrc(0);
  IceOperand *Src1 = Inst->getSrc(1); // Base
  IceOperand *Src2 = Inst->getSrc(2); // Index - could be NULL
  IceOperand *Src3 = Inst->getSrc(3); // Shift - constant
  IceOperand *Src4 = Inst->getSrc(4); // Offset - constant
  assert(Src1 != NULL /* && llvm::isa<IceVariable>(Src1)*/);
  assert(Src2 == NULL || llvm::isa<IceVariable>(Src2));
  assert(Src2 == NULL || llvm::isa<IceConstant>(Src3));
  assert(Src4 == NULL || llvm::isa<IceConstant>(Src4));

  // store val, [base,index,shift,offset] ==>
  // t0=val; t1=base; t2=index; store[base,index,shift,offset]
  IceVariable *Reg0 = Cfg->makeVariable(Value->getType());
  Reg0->setWeightInfinite();
  Reg0->setPreferredRegister(llvm::dyn_cast<IceVariable>(Value), true);
  NewInst = new IceInstX8632Mov(Cfg, Reg0, Value);
  Expansion.push_back(NewInst);

  IceVariable *Reg1 = Cfg->makeVariable(Src1->getType());
  Reg1->setWeightInfinite();
  Reg1->setPreferredRegister(llvm::dyn_cast<IceVariable>(Src1), true);
  NewInst = new IceInstX8632Mov(Cfg, Reg1, Src1);
  Expansion.push_back(NewInst);
  IceVariable *Reg2 = NULL;
  if (Src2) {
    Reg2 = Cfg->makeVariable(Src2->getType());
    Reg2->setWeightInfinite();
    Reg2->setPreferredRegister(llvm::dyn_cast<IceVariable>(Src2), true);
    NewInst = new IceInstX8632Mov(Cfg, Reg2, Src2);
    Expansion.push_back(NewInst);
  }

  NewInst = new IceInstX8632Store(Cfg, Reg0, Reg1, Reg2, Src3, Src4);
  Expansion.push_back(NewInst);

  return Expansion;
}

IceInstList IceTargetX8632S::lowerSwitch(const IceInstSwitch *Inst,
                                         const IceInst *Next,
                                         bool &DeleteNextInst) {
  IceInstList Expansion;
  Cfg->setError("Switch lowering not implemented");
  return Expansion;
}
