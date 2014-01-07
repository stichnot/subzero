/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include <assert.h>

#include "IceCfg.h"
#include "IceInst.h"
#include "IceInstX8632.h"
#include "IceOperand.h"
#include "IceRegManager.h"

namespace IceX8632 {
  IceInstList genCode(IceCfg *Cfg, IceRegManager *RegManager,
                      const IceInst *Inst, IceInst *Next,
                      bool &DeleteCurInst, bool &DeleteNextInst) {
    IceInstList Expansion;
    IceOpList Prefer;
    IceVarList Avoid;
    IceVariable *Dest;
    IceOperand *Src0, *Src1, *Src2, *Src3;
    IceVariable *Reg, *Reg1, *Reg2;
    bool LRend0, LRend1;
    IceInstTarget *NewInst;
    DeleteCurInst = true;
    switch (Inst->getKind()) {
    case IceInst::Assign:
      Dest = Inst->getDest(0);
      Src0 = Inst->getSrc(0);
      Prefer.push_back(Src0);
      Reg = RegManager->getRegister(Dest->getType(), Prefer, Avoid);
      if (Dest->getRegNum() >= 0) {
        if (RegManager->registerContains(Reg, Src0)) {
          Src0 = Reg;
        }
        NewInst = new IceInstX8632Mov(Dest->getType(), Dest, Src0);
        Expansion.push_back(NewInst);
        // TODO: commenting out the notifyLoad() call because Dest
        // doesn't actually belong to the current RegManager and
        // therefore it asserts.  Maybe this is the right thing to do,
        // but we also have to consider the code selection for
        // globally register allocated variables.
        //RegManager->notifyLoad(NewInst);
        NewInst->setRegState(RegManager);
      } else {
        if (!RegManager->registerContains(Reg, Src0)) {
          NewInst = new IceInstX8632Mov(Dest->getType(), Reg, Src0);
          Expansion.push_back(NewInst);
          RegManager->notifyLoad(NewInst);
          NewInst->setRegState(RegManager);
        }
        NewInst = new IceInstX8632Mov(Dest->getType(), Dest, Reg);
        Expansion.push_back(NewInst);
        RegManager->notifyStore(NewInst);
        NewInst->setRegState(RegManager);
      }
      break;
    case IceInst::Arithmetic:
      // TODO: Several instructions require specific physical
      // registers, namely div, rem, shift.  Loading into a physical
      // register requires killing all operands available in all
      // virtual registers, except that "ecx=r1" doesn't need to kill
      // operands available in r1.
      Dest = Inst->getDest(0);
      Src0 = Inst->getSrc(0);
      Src1 = Inst->getSrc(1);
      LRend0 = Cfg->isLastUse(Inst, Src0);
      LRend1 = Cfg->isLastUse(Inst, Src1);
      (void)LRend1;
      // Prefer Src0 if its live range is ending.
      if (LRend0) {
        Prefer.push_back(Src0);
      }
      if (Src1->getVariable())
        Avoid.push_back(Src1->getVariable());
      Reg = RegManager->getRegister(Dest->getType(), Prefer, Avoid);
      // Create "reg=Src0" if needed.
      if (!RegManager->registerContains(Reg, Src0)) {
        NewInst = new IceInstX8632Mov(Dest->getType(), Reg, Src0);
        Expansion.push_back(NewInst);
        RegManager->notifyLoad(NewInst);
        NewInst->setRegState(RegManager);
      }
      // TODO: Use a virtual register instead of Src1 if Src1 is
      // available in a virtual register.
      NewInst = new IceInstX8632Arithmetic((IceInstX8632Arithmetic::IceX8632Arithmetic)static_cast<const IceInstArithmetic *>(Inst)->getOp(), Dest->getType(), Reg, Src1);
      Expansion.push_back(NewInst);
      RegManager->notifyLoad(NewInst, false);
      NewInst->setRegState(RegManager);
      NewInst = new IceInstX8632Mov(Dest->getType(), Dest, Reg);
      Expansion.push_back(NewInst);
      RegManager->notifyStore(NewInst);
      NewInst->setRegState(RegManager);
      break;
    case IceInst::Icmp:
      // For now, require that the following instruction is a branch
      // based on the last use of this instruction's Dest operand.
      // TODO: Fix this.
      if (Next->getKind() == IceInst::Br &&
          Inst->getDest(0) == Next->getSrc(0)) {
        // This is basically identical to an Arithmetic instruction,
        // except there is no Dest variable to store.
        Src0 = Inst->getSrc(0);
        Src1 = Inst->getSrc(1);
        Prefer.push_back(Src0);
        if (Src1->getVariable())
          Avoid.push_back(Src1->getVariable());
        Reg = RegManager->getRegister(Dest->getType(), Prefer, Avoid);
        // Create "reg=Src0" if needed.
        if (!RegManager->registerContains(Reg, Src0)) {
          NewInst = new IceInstX8632Mov(Src0->getType(), Reg, Src0);
          Expansion.push_back(NewInst);
          RegManager->notifyLoad(NewInst);
          NewInst->setRegState(RegManager);
        }
        NewInst = new IceInstX8632Icmp(Reg, Src1);
        Expansion.push_back(NewInst);
        NewInst->setRegState(RegManager);
        const IceInstIcmp *InstIcmp = static_cast<const IceInstIcmp*>(Inst);
        IceInstBr *NextBr = static_cast<IceInstBr*>(Next);
        NewInst = new IceInstX8632Br(InstIcmp->getCondition(), NextBr->getLabelTrue(), NextBr->getLabelFalse());
        Expansion.push_back(NewInst);
        //Next->setDeleted();
        DeleteNextInst = true;
      } else {
        assert(0);
      }
      break;
    case IceInst::Load:
      Dest = Inst->getDest(0);
      Src0 = Inst->getSrc(0); // Base
      Src1 = Inst->getSrc(1); // Index - could be NULL
      Src2 = Inst->getSrc(2); // Shift - constant
      Src3 = Inst->getSrc(3); // Offset - constant
      LRend0 = Cfg->isLastUse(Inst, Src0);
      LRend1 = Src1 ? Cfg->isLastUse(Inst, Src1) : false;
      (void)LRend1;
      Prefer.push_back(Src0);
      if (Src1 && Src1->getVariable())
        Avoid.push_back(Src1->getVariable());
      Reg1 = RegManager->getRegister(Dest->getType(), Prefer, Avoid);
      if (!RegManager->registerContains(Reg1, Src0)) {
        NewInst = new IceInstX8632Mov(Dest->getType(), Reg1, Src0);
        Expansion.push_back(NewInst);
        RegManager->notifyLoad(NewInst);
        NewInst->setRegState(RegManager);
      }
      Reg2 = NULL;
      if (Src1) {
        Prefer.clear();
        Avoid.clear();
        Prefer.push_back(Src1);
        Avoid.push_back(Reg1);
        Reg2 = RegManager->getRegister(Dest->getType(), Prefer, Avoid);
        if (!RegManager->registerContains(Reg2, Src1)) {
          NewInst = new IceInstX8632Mov(Dest->getType(), Reg2, Src1);
          Expansion.push_back(NewInst);
          RegManager->notifyLoad(NewInst);
          NewInst->setRegState(RegManager);
        }
      }
      Prefer.clear();
      Avoid.clear();
      Avoid.push_back(Reg1);
      Avoid.push_back(Reg2);
      Reg = RegManager->getRegister(Dest->getType(), Prefer, Avoid);
      NewInst = new IceInstX8632Load(Dest->getType(), Reg,
                                     Reg1, Reg2, Src2, Src3);
      Expansion.push_back(NewInst);
      RegManager->notifyLoad(NewInst, false);
      NewInst->setRegState(RegManager);
      NewInst = new IceInstX8632Mov(Dest->getType(), Dest, Reg);
      Expansion.push_back(NewInst);
      RegManager->notifyStore(NewInst);
      NewInst->setRegState(RegManager);
      break;
    case IceInst::Ret:
      Src0 = Inst->getSrc(0);
      Prefer.push_back(Src0);
      Reg = RegManager->getRegister(Src0->getType(), Prefer, Avoid);
      if (!RegManager->registerContains(Reg, Src0)) {
        NewInst = new IceInstX8632Mov(Src0->getType(), Reg, Src0);
        Expansion.push_back(NewInst);
        RegManager->notifyLoad(NewInst);
        NewInst->setRegState(RegManager);
      }
      NewInst = new IceInstX8632Ret(Src0->getType(), Reg);
      Expansion.push_back(NewInst);
      break;
    default:
      DeleteCurInst = false;
      break;
    }
    return Expansion;
  }
}

IceInstX8632Arithmetic::IceInstX8632Arithmetic(IceX8632Arithmetic Op,
                                               IceType Type,
                                               IceVariable *Dest,
                                               IceOperand *Source) :
  IceInstTarget(Type), Op(Op) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Br::IceInstX8632Br(IceInstIcmp::IceICond Condition,
                               uint32_t LabelTrue, uint32_t LabelFalse) :
  IceInstTarget(IceType_void), Condition(Condition),
  LabelIndexFalse(LabelFalse), LabelIndexTrue(LabelTrue) {
}

IceInstX8632Icmp::IceInstX8632Icmp(IceOperand *Src0, IceOperand *Src1) :
  IceInstTarget(Src0->getType()) {
  addSource(Src0);
  addSource(Src1);
}

IceInstX8632Load::IceInstX8632Load(IceType Type, IceVariable *Dest,
                                   IceOperand *Src0, IceOperand *Src1,
                                   IceOperand *Src2, IceOperand *Src3) :
  IceInstTarget(Type) {
  addDest(Dest);
  addSource(Src0);
  addSource(Src1);
  addSource(Src2);
  addSource(Src3);
}

IceInstX8632Mov::IceInstX8632Mov(IceType Type,
                                 IceVariable *Dest, IceOperand *Source) :
  IceInstTarget(Type) {
  addDest(Dest);
  addSource(Source);
}

IceInstX8632Ret::IceInstX8632Ret(IceType Type, IceVariable *Source) :
  IceInstTarget(Type) {
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
  Str << "." << Type << " ";
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
  Str << ", label %" << Str.Cfg->labelName(LabelIndexTrue)
      << ", label %" << Str.Cfg->labelName(LabelIndexFalse);
}

void IceInstX8632Icmp::dump(IceOstream &Str) const {
  Str << "cmp." << Type << " ";
  dumpSources(Str);
}

void IceInstX8632Load::dump(IceOstream &Str) const {
  Str << "mov." << Type << " ";
  dumpDests(Str);
  Str << ", [";
  dumpSources(Str);
  Str << "]";
}

void IceInstX8632Mov::dump(IceOstream &Str) const {
  Str << "mov." << Type << " ";
  dumpDests(Str);
  Str << ", ";
  dumpSources(Str);
}

void IceInstX8632Ret::dump(IceOstream &Str) const {
  Str << "ret." << Type << " ";
  dumpSources(Str);
}
