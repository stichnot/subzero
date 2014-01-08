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

IceInst::IceInst(IceInstType Kind, IceType Type) :
  Kind(Kind), Type(Type), Deleted(false) {}

void IceInst::setDeleted(void) {
  removeUse(NULL);
}

void IceInst::updateVars(IceCfgNode *Node) {
  // update variables in Dests
  for (IceVarList::const_iterator I = Dests.begin(), E = Dests.end();
       I != E; ++I) {
    (*I)->setDefinition(this, Node);
  }
  for (IceOpList::const_iterator I = Srcs.begin(), E = Srcs.end();
       I != E; ++I) {
    if (*I == NULL)
      continue;
    (*I)->setUse(this, Node);
  }
}

void IceInst::addDest(IceVariable *Dest) {
  Dests.push_back(Dest);
}

void IceInst::addSource(IceOperand *Source) {
  Srcs.push_back(Source);
}

void IceInst::findAddressOpt(IceCfg *Cfg, const IceCfgNode *Node) {
  if (getKind() != Load && getKind() != Store)
    return;
  IceVariable *Base = Srcs.back()->getVariable();
  IceVariable *BaseOrig = Base;
  if (Base == NULL)
    return;
  IceVariable *Index = NULL;
  int Shift = 0;
  int32_t Offset = 0; // TODO: is int32_t appropriate?
  doAddressOpt(Base, Index, Shift, Offset);
  if (Base == BaseOrig)
    return;

  Cfg->Str << "Found AddressOpt opportunity: BaseOrig=" << BaseOrig
           << " Base=" << Base
           << " Index=" << Index
           << " Shift=" << Shift
           << " Offset=" << Offset << "\n";

  // Replace Srcs.back() with Base+Index+Shift+Offset, updating
  // reference counts and deleting resulting dead instructions.
  IceOpList NewOperands;
  NewOperands.push_back(Base);
  NewOperands.push_back(Index);
  NewOperands.push_back(new IceConstant(Shift));
  NewOperands.push_back(new IceConstant(Offset));
  replaceOperands(Node, Srcs.size() - 1, NewOperands);
}

void IceInst::doAddressOpt(IceVariable *&Base, IceVariable *&Index,
                           int &Shift, int32_t &Offset) {
  if (Base == NULL) // shouldn't happen
    return;
  // If the Base has more than one use or is live across multiple
  // blocks, then don't go further.  Alternatively (?), never consider
  // a transformation that would change a variable that is currently
  // *not* live across basic block boundaries into one that *is*.
  if (Base->isMultiblockLife() || Base->getUseCount() > 1)
    return;

  // TODO: limit to single-dest instructions (though should be
  // unnecessary for Vanilla ICE).
  while (true) {
    // Base is Base=Var ==>
    //   set Base=Var
    const IceInst *BaseInst = Base->getDefinition();
    IceOperand *BaseOperand0 = BaseInst ? BaseInst->getSrc(0) : NULL;
    IceVariable *BaseVariable0 =
      (BaseOperand0 ? BaseOperand0->getVariable() : NULL);
    if (BaseInst &&
        BaseInst->getKind() == IceInst::Assign &&
        BaseVariable0 &&
        // TODO: ensure BaseVariable0 stays single-BB
        true) {
      Base = BaseVariable0;
      continue;
    }

    // Index is Index=Var ==>
    //   set Index=Var

    // Index==NULL && Base is Base=Var1+Var2 ==>
    //   set Base=Var1, Index=Var2, Shift=0
    IceOperand *BaseOperand1 = BaseInst ? BaseInst->getSrc(1) : NULL;
    IceVariable *BaseVariable1 =
      (BaseOperand1 ? BaseOperand1->getVariable() : NULL);
    if (Index == NULL &&
        BaseInst->getKind() == IceInst::Arithmetic &&
        static_cast<const IceInstArithmetic*>(BaseInst)->getOp() == IceInstArithmetic::Add &&
        BaseVariable0 && BaseVariable1 &&
        // TODO: ensure BaseVariable0 and BaseVariable1 stay single-BB
        true) {
      Base = BaseVariable0;
      Index = BaseVariable1;
      Shift = 0; // should already have been 0
      continue;
    }

    // Index is Index=Var*Const && log2(Const)+Shift<=3 ==>
    //   Index=Var, Shift+=log2(Const)
    const IceInst *IndexInst = Index ? Index->getDefinition() : NULL;
    IceOperand *IndexOperand0 = IndexInst ? IndexInst->getSrc(0) : NULL;
    IceVariable *IndexVariable0 =
      (IndexOperand0 ? IndexOperand0->getVariable() : NULL);
    IceOperand *IndexOperand1 = IndexInst ? IndexInst->getSrc(1) : NULL;
    IceConstant *IndexConstant1 = IndexOperand1 ? IndexOperand1->getConstant() : NULL;
    if (IndexInst &&
        IndexInst->getKind() == IceInst::Arithmetic &&
        static_cast<const IceInstArithmetic*>(IndexInst)->getOp() == IceInstArithmetic::Mul &&
        IndexVariable0 &&
        IndexOperand1->getType() == IceType_i32 &&
        IndexConstant1) {
      uint32_t Mult = IndexConstant1->getIntValue();
      uint32_t LogMult;
      switch (Mult) {
      case 1:
        LogMult = 0;
        break;
      case 2:
        LogMult = 1;
        break;
      case 4:
        LogMult = 2;
        break;
      case 8:
        LogMult = 3;
        break;
      default:
        LogMult = 4;
        break;
      }
      if (Shift + LogMult <= 3) {
        Index = IndexVariable0;
        Shift += LogMult;
        continue;
      }
    }

    // Index is Index=Var<<Const && Const+Shift<=3 ==>
    //   Index=Var, Shift+=Const

    // Index is Index=Const*Var && log2(Const)+Shift<=3 ==>
    //   Index=Var, Shift+=log2(Const)

    // Index && Shift==0 && Base is Base=Var*Const && log2(Const)+Shift<=3 ==>
    //   swap(Index,Base)
    // Similar for Base=Const*Var and Base=Var<<Const

    // Base is Base=Var+Const ==>
    //   set Base=Var, Offset+=Const

    // Base is Base=Const+Var ==>
    //   set Base=Var, Offset+=Const

    // Base is Base=Var-Const ==>
    //   set Base=Var, Offset-=Const

    // Index is Index=Var+Const ==>
    //   set Index=Var, Offset+=(Const<<Shift)

    // Index is Index=Const+Var ==>
    //   set Index=Var, Offset+=(Const<<Shift)

    // Index is Index=Var-Const ==>
    //   set Index=Var, Offset-=(Const<<Shift)

    // TODO: consider overflow issues with respect to Offset.
    // TODO: handle symbolic constants.
    break;
  }

}

void IceInst::replaceOperands(const IceCfgNode *Node, unsigned Index,
                              const IceOpList &NewOperands) {
  // Snapshot and remove the old operand.
  IceOperand *OldOperand = Srcs[Index];
  Srcs.erase(Srcs.begin() + Index);
  // Copy in the new operands.
  Srcs.insert(Srcs.begin() + Index, NewOperands.begin(), NewOperands.end());
  // Increment the reference counts of the new operands.
  for (IceOpList::const_iterator I = NewOperands.begin(), E = NewOperands.end();
       I != E; ++I) {
    (*I)->setUse(this, Node);
  }

  // Decrement the reference count of the old operand, and fully or
  // partially delete the defining instruction if the reference count
  // reaches 0, doing a cascading decrement/delete etc.
  OldOperand->removeUse();
}

void IceInst::removeUse(IceVariable *Variable) {
  if (isDeleted())
    return;
  Deleted = true;
  for (IceOpList::const_iterator I = Srcs.begin(), E = Srcs.end();
       I != E; ++I) {
    if (*I == NULL)
      continue;
    (*I)->removeUse();
  }
}

void IceInst::markLastUses(IceCfg *Cfg) {
  for (IceOpList::const_iterator I = Srcs.begin(), E = Srcs.end();
       I != E; ++I) {
    if (*I == NULL)
      continue;
    Cfg->markLastUse(*I, this);
  }
}

IceInstList IceInst::genCodeX8632(IceCfg *Cfg,
                                  IceRegManager *RegManager,
                                  IceInst *Next,
                                  bool &DeleteCurInst,
                                  bool &DeleteNextInst) {
  return IceX8632::genCode(Cfg, RegManager, this, Next,
                           DeleteCurInst, DeleteNextInst);
}

IceInstArithmetic::IceInstArithmetic(IceArithmetic Op, IceType Type,
                                     IceVariable *Dest,
                                     IceOperand *Source1,
                                     IceOperand *Source2) :
  IceInst(Arithmetic, Type), Op(Op) {
  addDest(Dest);
  addSource(Source1);
  addSource(Source2);
}

bool IceInstArithmetic::isCommutative(void) const {
  switch (getOp()) {
  case Add:
  case Fadd:
  case Mul:
  case Fmul:
  case And:
  case Or:
  case Xor:
    return true;
  default:
    return false;
  }
}

IceInstAssign::IceInstAssign(IceType Type,
                             IceVariable *Dest, IceOperand *Source) :
  IceInst(Assign, Type) {
  addDest(Dest);
  addSource(Source);
}

IceInstBr::IceInstBr(IceCfgNode *Node, IceOperand *Source,
                     uint32_t LabelTrue, uint32_t LabelFalse) :
  IceInst(IceInst::Br, IceType_i1), IsConditional(true), Node(Node) {
  addSource(Source);
  Node->addFallthrough(LabelFalse);
  Node->addNonFallthrough(LabelTrue);
}

IceInstBr::IceInstBr(IceCfgNode *Node, uint32_t Label) :
  IceInst(IceInst::Br, IceType_i1), IsConditional(false), Node(Node) {
  Node->addFallthrough(Label);
}

uint32_t IceInstBr::getLabelTrue(void) const {
  return Node->getNonFallthrough();
}

uint32_t IceInstBr::getLabelFalse(void) const {
  return Node->getFallthrough();
}

IceInstIcmp::IceInstIcmp(IceICond Condition, IceType Type, IceVariable *Dest,
                         IceOperand *Source1, IceOperand *Source2) :
  IceInst(Icmp, Type), Condition(Condition) {
  addDest(Dest);
  addSource(Source1);
  addSource(Source2);
}

IceInstLoad::IceInstLoad(IceType Type,
                         IceVariable *Dest, IceOperand *SourceAddr) :
  IceInst(Load, Type) {
  addDest(Dest);
  addSource(SourceAddr);
}

IceInstPhi::IceInstPhi(IceType Type, IceVariable *Dest) :
  IceInst(Phi, Type) {
  addDest(Dest);
}

void IceInstPhi::addArgument(IceOperand *Source, uint32_t Label) {
  addSource(Source);
  Labels.push_back(Label);
}

// Change "a=phi(...)" to "a_phi=phi(...)" and return a new
// instruction "a=a_phi".
IceInst *IceInstPhi::lower(IceCfg *Cfg, IceCfgNode *Node) {
  assert(Dests.size() == 1);
  IceVariable *Dest = getDest(0);
  IceString PhiName = Cfg->variableName(Dest->getIndex()) + "_phi";
  IceVariable *NewSrc = Cfg->getVariable(Type, PhiName);
  Dests.clear();
  addDest(NewSrc);
  IceInstAssign *NewInst = new IceInstAssign(Type, Dest, NewSrc);
  //NewInst->updateVars(Node);
  Dest->replaceDefinition(NewInst, Node);
  return NewInst;
}

IceOperand *IceInstPhi::getOperandForTarget(uint32_t Target) const {
  IceEdgeList::const_iterator EdgeIter = Labels.begin(), EdgeEnd = Labels.end();
  for (IceOpList::const_iterator I = Srcs.begin(), E = Srcs.end();
       I != E && EdgeIter != EdgeEnd; ++I, ++EdgeIter) {
    if (*EdgeIter == Target)
      return *I;
  }
  return NULL;
}

IceInstRet::IceInstRet(IceType Type, IceOperand *Source) :
  IceInst(Ret, Type) {
  if (Source)
    addSource(Source);
}

void IceInstTarget::setRegState(const IceRegManager *State) {
  RegState = new IceRegManager(*State);
}

// ======================== Dump routines ======================== //

IceOstream& operator<<(IceOstream &Str, const IceInst *I) {
  if (I->isDeleted() && !Str.isVerbose())
    return Str;
  Str << "  ";
  if (I->isDeleted())
    Str << "//";
  I->dump(Str);
  if (Str.isVerbose())
    I->dumpExtras(Str);
  Str << "\n";
  return Str;
}

void IceInst::dump(IceOstream &Str) const {
  dumpDests(Str);
  Str << " =~ ";
  dumpSources(Str);
}

void IceInst::dumpExtras(IceOstream &Str) const {
  bool First = true;
  // Print "LIVEEND={a,b,c}" for all source operands whose live ranges
  // are known to end at this instruction.
  for (IceOpList::const_iterator I = Srcs.begin(), E = Srcs.end();
       I != E; ++I) {
    if (*I == NULL)
      continue;
    if (Str.Cfg->isLastUse(this, *I)) {
      if (First)
        Str << " // LIVEEND={";
      else
        Str << ",";
      Str << *I;
      First = false;
    }
  }
  if (!First)
    Str << "}";
}

void IceInst::dumpSources(IceOstream &Str) const {
  for (IceOpList::const_iterator I = Srcs.begin(), E = Srcs.end();
       I != E; ++I) {
    if (*I == NULL)
      continue;
    if (I != Srcs.begin())
      Str << ", ";
    Str << *I;
  }
}

void IceInst::dumpDests(IceOstream &Str) const {
  for (IceVarList::const_iterator I = Dests.begin(), E = Dests.end();
       I != E; ++I) {
    if (I != Dests.begin())
      Str << " ";
    Str << *I;
  }
}

void IceInstAlloca::dump(IceOstream &Str) const {
  dumpDests(Str);
  Str << " = alloca " << Type
      << ", i32 " << Size
      << ", align " << Align;
}

void IceInstArithmetic::dump(IceOstream &Str) const {
  dumpDests(Str);
  Str << " = ";
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
  Str << " " << Type << " ";
  dumpSources(Str);
}

void IceInstAssign::dump(IceOstream &Str) const {
  dumpDests(Str);
  Str << " = " << Type << " ";
  dumpSources(Str);
}

void IceInstBr::dump(IceOstream &Str) const {
  dumpDests(Str);
  Str << "br ";
  if (IsConditional) {
    Str << "i1 " << Srcs[0]
        << ", label %" << Str.Cfg->labelName(getLabelTrue())
        << ", ";
  }
  Str << "label %" << Str.Cfg->labelName(getLabelFalse());
}

void IceInstIcmp::dump(IceOstream &Str) const {
  dumpDests(Str);
  Str << " = icmp ";
  switch (Condition) {
  case Eq:
    Str << "eq";
    break;
  case Ne:
    Str << "ne";
    break;
  case Ugt:
    Str << "ugt";
    break;
  case Uge:
    Str << "uge";
    break;
  case Ult:
    Str << "ult";
    break;
  case Ule:
    Str << "ule";
    break;
  case Sgt:
    Str << "sgt";
    break;
  case Sge:
    Str << "sge";
    break;
  case Slt:
    Str << "slt";
    break;
  case Sle:
    Str << "sle";
    break;
  }
  Str << " " << Type << " ";
  dumpSources(Str);
}

void IceInstLoad::dump(IceOstream &Str) const {
  dumpDests(Str);
  Str << " = load " << Type << "* ";
  dumpSources(Str);
  Str << ", align ";
  switch (Type) {
  case IceType_f32:
    Str << "4";
    break;
  case IceType_f64:
    Str << "8";
    break;
  default:
    Str << "1";
    break;
  }
}

void IceInstPhi::dump(IceOstream &Str) const {
  dumpDests(Str);
  Str << " = phi " << Type << " ";
  IceEdgeList::const_iterator EdgeIter = Labels.begin(), EdgeEnd = Labels.end();
  for (IceOpList::const_iterator I = Srcs.begin(), E = Srcs.end();
       I != E && EdgeIter != EdgeEnd; ++I, ++EdgeIter) {
    if (I != Srcs.begin())
      Str << ", ";
    Str << "[ " << *I << ", %" << Str.Cfg->labelName(*EdgeIter) << " ]";
  }
}

void IceInstRet::dump(IceOstream &Str) const {
  Str << "ret " << Type << " ";
  dumpSources(Str);
}

void IceInstTarget::dump(IceOstream &Str) const {
  Str << "[TARGET] ";
  IceInst::dump(Str);
}

void IceInstTarget::dumpExtras(IceOstream &Str) const {
  IceInst::dumpExtras(Str);
  if (RegState) {
    Str << " //";
    RegState->dump(Str);
  }
}
