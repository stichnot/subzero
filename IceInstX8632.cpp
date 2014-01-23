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

IceString IceTargetX8632::RegNames[] = { "eax", "ecx", "edx", };

IceInstTarget *IceTargetX8632::makeAssign(IceVariable *Dest, IceOperand *Src) {
  assert(Dest->getRegNum() >= 0);
  return new IceInstX8632Mov(Cfg, Dest, Src);
}

IceInstList IceTargetX8632::lowerAlloca(const IceInst *Inst,
                                        const IceInst *Next,
                                        bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement
  return Expansion;
}

IceInstList IceTargetX8632::lowerArithmetic(const IceInst *Inst,
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
  IceVariable *Dest = Inst->getDest(0);
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
  // TODO: Use a virtual register instead of Src1 if Src1 is
  // available in a virtual register.
  // TODO: De-uglify this.
  NewInst = new IceInstX8632Arithmetic(
      Cfg,
      (IceInstX8632Arithmetic::IceX8632Arithmetic)llvm::cast<IceInstArithmetic>(
          Inst)->getOp(),
      Reg, Src1);
  Expansion.push_back(NewInst);
  RegManager->notifyLoad(NewInst, false);
  NewInst->setRegState(RegManager);
  NewInst = new IceInstX8632Mov(Cfg, Dest, Reg);
  Expansion.push_back(NewInst);
  RegManager->notifyStore(NewInst);
  NewInst->setRegState(RegManager);
  return Expansion;
}

IceInstList IceTargetX8632::lowerAssign(const IceInst *Inst,
                                        const IceInst *Next,
                                        bool &DeleteNextInst) {
  IceInstList Expansion;
  IceOpList Prefer;
  IceVarList Avoid;
  IceVariable *Dest = Inst->getDest(0);
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

IceInstList IceTargetX8632::lowerBr(const IceInst *Inst, const IceInst *Next,
                                    bool &DeleteNextInst) {
  IceInstList Expansion;
  // assert(0); // TODO: implement
  return Expansion;
}

IceInstList IceTargetX8632::lowerCall(const IceInst *Inst, const IceInst *Next,
                                      bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement
  return Expansion;
}

IceInstList IceTargetX8632::lowerConversion(const IceInst *Inst,
                                            const IceInst *Next,
                                            bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement
  return Expansion;
}

IceInstList IceTargetX8632::lowerFcmp(const IceInst *Inst, const IceInst *Next,
                                      bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement
  return Expansion;
}

IceInstList IceTargetX8632::lowerIcmp(const IceInst *Inst, const IceInst *Next,
                                      bool &DeleteNextInst) {
  IceInstList Expansion;
  IceInstTarget *NewInst;
  IceOpList Prefer;
  IceVarList Avoid;
  // For now, require that the following instruction is a branch
  // based on the last use of this instruction's Dest operand.
  // TODO: Fix this.
  if (llvm::isa<IceInstBr>(Next) && Inst->getDest(0) == Next->getSrc(0)) {
    const IceInstIcmp *InstIcmp = llvm::cast<IceInstIcmp>(Inst);
    const IceInstBr *NextBr = llvm::cast<IceInstBr>(Next);
    // This is basically identical to an Arithmetic instruction,
    // except there is no Dest variable to store.
    IceOperand *Src0 = Inst->getSrc(0);
    IceOperand *Src1 = Inst->getSrc(1);
    Prefer.push_back(Src0);
    if (IceVariable *Variable = llvm::dyn_cast<IceVariable>(Src1))
      Avoid.push_back(Variable);
    IceVariable *Reg = RegManager->getRegister(Src0->getType(), Prefer, Avoid);
    IceOperand *RegSrc = Reg;
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
    NewInst = new IceInstX8632Icmp(Cfg, RegSrc, Src1);
    Expansion.push_back(NewInst);
    NewInst->setRegState(RegManager);
    NewInst =
        new IceInstX8632Br(Cfg, NextBr->getNode(), InstIcmp->getCondition());
    Expansion.push_back(NewInst);
    DeleteNextInst = true;
  } else {
    assert(0);
  }
  return Expansion;
}

IceInstList IceTargetX8632::lowerLoad(const IceInst *Inst, const IceInst *Next,
                                      bool &DeleteNextInst) {
  IceInstList Expansion;
  IceInstTarget *NewInst;
  IceVariable *Dest = Inst->getDest(0);
  IceOperand *Src0 = Inst->getSrc(0); // Base
  IceOperand *Src1 = Inst->getSrc(1); // Index - could be NULL
  IceOperand *Src2 = Inst->getSrc(2); // Shift - constant
  IceOperand *Src3 = Inst->getSrc(3); // Offset - constant
  IceOpList Prefer;
  IceVarList Avoid;
  Prefer.push_back(Src0);
  if (IceVariable *Variable = llvm::dyn_cast_or_null<IceVariable>(Src1))
    Avoid.push_back(Variable);
  IceVariable *Reg1 = RegManager->getRegister(Dest->getType(), Prefer, Avoid);
  if (!RegManager->registerContains(Reg1, Src0)) {
    NewInst = new IceInstX8632Mov(Cfg, Reg1, Src0);
    Expansion.push_back(NewInst);
    RegManager->notifyLoad(NewInst);
    NewInst->setRegState(RegManager);
  }
  IceVariable *Reg2 = NULL;
  if (Src1) {
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
  Prefer.clear();
  Avoid.clear();
  Avoid.push_back(Reg1);
  Avoid.push_back(Reg2);
  IceVariable *Reg = RegManager->getRegister(Dest->getType(), Prefer, Avoid);
  NewInst = new IceInstX8632Load(Cfg, Reg, Reg1, Reg2, Src2, Src3);
  Expansion.push_back(NewInst);
  RegManager->notifyLoad(NewInst, false);
  NewInst->setRegState(RegManager);
  NewInst = new IceInstX8632Mov(Cfg, Dest, Reg);
  Expansion.push_back(NewInst);
  RegManager->notifyStore(NewInst);
  NewInst->setRegState(RegManager);
  return Expansion;
}

IceInstList IceTargetX8632::lowerPhi(const IceInst *Inst, const IceInst *Next,
                                     bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement if necessary
  return Expansion;
}

IceInstList IceTargetX8632::lowerRet(const IceInst *Inst, const IceInst *Next,
                                     bool &DeleteNextInst) {
  IceInstList Expansion;
  IceInstTarget *NewInst;
  IceOperand *Src0 = Inst->getSrc(0);
  IceVariable *Reg;
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
  NewInst = new IceInstX8632Ret(Cfg, Reg);
  Expansion.push_back(NewInst);
  return Expansion;
}

IceInstList IceTargetX8632::lowerSelect(const IceInst *Inst,
                                        const IceInst *Next,
                                        bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement
  return Expansion;
}

IceInstList IceTargetX8632::lowerStore(const IceInst *Inst, const IceInst *Next,
                                       bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement
  return Expansion;
}

IceInstList IceTargetX8632::lowerSwitch(const IceInst *Inst,
                                        const IceInst *Next,
                                        bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement
  return Expansion;
}

IceInstX8632Arithmetic::IceInstX8632Arithmetic(IceCfg *Cfg,
                                               IceX8632Arithmetic Op,
                                               IceVariable *Dest,
                                               IceOperand *Source)
    : IceInstTarget(Cfg), Op(Op) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Br::IceInstX8632Br(IceCfg *Cfg, const IceCfgNode *Node,
                               IceInstIcmp::IceICond Condition)
    : IceInstTarget(Cfg), Condition(Condition), Node(Node) {}

uint32_t IceInstX8632Br::getLabelTrue(void) const {
  return Node->getNonFallthrough();
}

uint32_t IceInstX8632Br::getLabelFalse(void) const {
  return Node->getFallthrough();
}

IceInstX8632Icmp::IceInstX8632Icmp(IceCfg *Cfg, IceOperand *Src0,
                                   IceOperand *Src1)
    : IceInstTarget(Cfg) {
  addSource(Src0);
  addSource(Src1);
}

IceInstX8632Load::IceInstX8632Load(IceCfg *Cfg, IceVariable *Dest,
                                   IceOperand *Src0, IceOperand *Src1,
                                   IceOperand *Src2, IceOperand *Src3)
    : IceInstTarget(Cfg) {
  addDest(Dest);
  addSource(Src0);
  addSource(Src1);
  addSource(Src2);
  addSource(Src3);
}

IceInstX8632Mov::IceInstX8632Mov(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstTarget(Cfg) {
  addDest(Dest);
  addSource(Source);
}

IceInstX8632Ret::IceInstX8632Ret(IceCfg *Cfg, IceVariable *Source)
    : IceInstTarget(Cfg) {
  addSource(Source);
}

// ======================== Dump routines ======================== //

void IceInstX8632Arithmetic::dump(IceOstream &Str) const {
  switch (Op) {
  case Add:
    Str << "add";
    break;
  case Fadd:
    Str << "fadd";
    break;
  case Sub:
    Str << "sub";
    break;
  case Fsub:
    Str << "fsub";
    break;
  case Mul:
    Str << "mul";
    break;
  case Fmul:
    Str << "fmul";
    break;
  case Udiv:
    Str << "udiv";
    break;
  case Sdiv:
    Str << "sdiv";
    break;
  case Fdiv:
    Str << "fdiv";
    break;
  case Urem:
    Str << "urem";
    break;
  case Srem:
    Str << "srem";
    break;
  case Frem:
    Str << "frem";
    break;
  case Shl:
    Str << "shl";
    break;
  case Lshr:
    Str << "lshr";
    break;
  case Ashr:
    Str << "ashr";
    break;
  case And:
    Str << "and";
    break;
  case Or:
    Str << "or";
    break;
  case Xor:
    Str << "xor";
    break;
  case Invalid:
    Str << "invalid";
    break;
  }
  Str << "." << getDest(0)->getType() << " ";
  dumpSources(Str);
}

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
  Str << ", label %" << Str.Cfg->getNode(getLabelTrue())->getName()
      << ", label %" << Str.Cfg->getNode(getLabelFalse())->getName();
}

void IceInstX8632Icmp::dump(IceOstream &Str) const {
  Str << "cmp." << getSrc(0)->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Load::dump(IceOstream &Str) const {
  Str << "mov." << getDest(0)->getType() << " ";
  dumpDests(Str);
  Str << ", [";
  dumpSources(Str);
  Str << "]";
}

void IceInstX8632Mov::dump(IceOstream &Str) const {
  Str << "mov." << getDest(0)->getType() << " ";
  dumpDests(Str);
  Str << ", ";
  dumpSources(Str);
}

void IceInstX8632Ret::dump(IceOstream &Str) const {
  IceType Type = Srcs.empty() ? IceType_void : getSrc(0)->getType();
  Str << "ret." << Type << " ";
  dumpSources(Str);
}
