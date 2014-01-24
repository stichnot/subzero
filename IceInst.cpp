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

IceInst::IceInst(IceCfg *Cfg, IceInstType Kind)
    : Kind(Kind), Deleted(false), Dead(false) {
  Number = Cfg->newInstNumber();
}

void IceInst::renumber(IceCfg *Cfg) {
  Number = isDeleted() ? -1 : Cfg->getNewInstNumber(Number);
}

void IceInst::setDeleted(void) { removeUse(NULL); }

void IceInst::deleteIfDead(void) {
  if (Dead)
    setDeleted();
}

void IceInst::updateVars(IceCfgNode *Node) {
  // update variables in Dests
  for (IceVarList::const_iterator I = Dests.begin(), E = Dests.end(); I != E;
       ++I) {
    (*I)->setDefinition(this, Node);
  }
  for (IceOpList::const_iterator I = Srcs.begin(), E = Srcs.end(); I != E;
       ++I) {
    if (*I == NULL)
      continue;
    (*I)->setUse(this, Node);
  }
}

void IceInst::addDest(IceVariable *Dest) {
  Dests.push_back(Dest);
  // TODO: Multi-dest instructions need more testing.
  assert(Dests.size() == 1);
}

void IceInst::addSource(IceOperand *Source) {
  Srcs.push_back(Source);
  LiveRangesEnded.resize(Srcs.size());
}

void IceInst::findAddressOpt(IceCfg *Cfg, const IceCfgNode *Node) {
  if (!llvm::isa<IceInstLoad>(this) && !llvm::isa<IceInstStore>(this))
    return;
  IceVariable *Base = llvm::dyn_cast<IceVariable>(Srcs.back());
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
           << " Base=" << Base << " Index=" << Index << " Shift=" << Shift
           << " Offset=" << Offset << "\n";

  // Replace Srcs.back() with Base+Index+Shift+Offset, updating
  // reference counts and deleting resulting dead instructions.
  IceOpList NewOperands;
  NewOperands.push_back(Base);
  NewOperands.push_back(Index);
  NewOperands.push_back(new IceConstant(Shift));
  NewOperands.push_back(new IceConstant(Offset));
  replaceOperands(Node, Srcs.size() - 1, NewOperands);
  // TODO: See if liveness information can be incrementally corrected
  // without a whole new liveness analysis pass.
}

void IceInst::doAddressOpt(IceVariable *&Base, IceVariable *&Index, int &Shift,
                           int32_t &Offset) {
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
        llvm::dyn_cast_or_null<IceVariable>(BaseOperand0);
    if (BaseInst && llvm::isa<IceInstAssign>(BaseInst) && BaseVariable0 &&
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
        llvm::dyn_cast_or_null<IceVariable>(BaseOperand1);
    if (Index == NULL && llvm::isa<IceInstArithmetic>(BaseInst) &&
        (llvm::cast<IceInstArithmetic>(BaseInst)->getOp() ==
         IceInstArithmetic::Add) &&
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
        llvm::dyn_cast_or_null<IceVariable>(IndexOperand0);
    IceOperand *IndexOperand1 = IndexInst ? IndexInst->getSrc(1) : NULL;
    IceConstant *IndexConstant1 =
        llvm::dyn_cast_or_null<IceConstant>(IndexOperand1);
    if (IndexInst && llvm::isa<IceInstArithmetic>(IndexInst) &&
        (llvm::cast<IceInstArithmetic>(IndexInst)->getOp() ==
         IceInstArithmetic::Mul) &&
        IndexVariable0 && IndexOperand1->getType() == IceType_i32 &&
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
  LiveRangesEnded.resize(Srcs.size());
}

void IceInst::removeUse(IceVariable *Variable) {
  if (isDeleted())
    return;
  Deleted = true;
  for (IceOpList::const_iterator I = Srcs.begin(), E = Srcs.end(); I != E;
       ++I) {
    if (*I == NULL)
      continue;
    (*I)->removeUse();
  }
}

void IceInst::liveness(IceLiveness Mode, int InstNumber, llvm::BitVector &Live,
                       std::vector<int> &LiveBegin, std::vector<int> &LiveEnd) {
  if (isDeleted())
    return;

  // For lightweight liveness, do the simple calculation and return.
  if (Mode == IceLiveness_LREndLightweight) {
    int OpNum = 0;
    LiveRangesEnded.reset();
    for (IceOpList::const_iterator I = Srcs.begin(), E = Srcs.end(); I != E;
         ++I, ++OpNum) {
      if (IceVariable *Var = llvm::dyn_cast_or_null<IceVariable>(*I)) {
        if (Var->isMultiblockLife())
          continue;
        uint32_t Index = Var->getIndex();
        if (Live[Index])
          continue;
        Live[Index] = true;
        LiveRangesEnded[OpNum] = true;
      }
    }
    return;
  }

  // TODO: if all dest operands are dead, consider marking the entire
  // instruction as dead, which also means not marking its source
  // operands as live.
  // Don't delete a dest-less instruction.
  Dead = !Dests.empty();
  for (IceVarList::const_iterator I = Dests.begin(), E = Dests.end(); I != E;
       ++I) {
    if (*I) {
      unsigned VarNum = (*I)->getIndex();
      if (Live[VarNum]) {
        Dead = false;
        Live[VarNum] = false;
        LiveBegin[VarNum] = InstNumber;
      }
    }
  }
  // TODO: Make sure it's OK to delete the instruction and its
  // potential side effects.
  if (Dead)
    return;
  // Phi arguments only get added to Live in the predecessor node, but
  // we still need to update LiveRangesEnded.
  bool IsPhi = llvm::isa<IceInstPhi>(this);
  assert(LiveRangesEnded.size() == Srcs.size());
  int Index = 0;
  LiveRangesEnded.reset();
  // TODO: For a 3-address arithmetic instruction on a 2-address
  // architecture, we need to indicate that the latter source
  // operand's live range *does* overlap with the dest operand's live
  // range.  This is because an instruction like "a=b-c" should be
  // expanded like "a=b; a-=c" and so we want to express "a" and "c"
  // as interfering.  If this instruction is the latter operand's last
  // use, we can handle this by using InstNumber+1 instead of
  // InstNumber for its LiveEnd value.  Note that for "a=b+c" where
  // c's live range ends, b's live range does not end, and the
  // operator is commutable, we can reduce register pressure by
  // commuting the operation.
  for (IceOpList::const_iterator I = Srcs.begin(), E = Srcs.end(); I != E;
       ++I, ++Index) {
    if (IceVariable *Var = llvm::dyn_cast_or_null<IceVariable>(*I)) {
      uint32_t VarNum = Var->getIndex();
      if (!Live[VarNum]) {
        LiveRangesEnded[Index] = true;
        if (!IsPhi) {
          Live[VarNum] = true;
          LiveEnd[VarNum] = InstNumber;
          if (Index == 1 && getKind() == Arithmetic) {
            // TODO: Do the same for target-specific Arithmetic
            // instructions, and also optimize for commutativity.
            // Also, consider moving this special logic into
            // IceCfgNode::livenessPostprocess().
            LiveEnd[VarNum] = InstNumber + 1;
          }
        }
      }
    }
  }
}

IceInstArithmetic::IceInstArithmetic(IceCfg *Cfg, IceArithmetic Op,
                                     IceVariable *Dest, IceOperand *Source1,
                                     IceOperand *Source2)
    : IceInst(Cfg, Arithmetic), Op(Op) {
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

IceInstAssign::IceInstAssign(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source)
    : IceInst(Cfg, Assign) {
  addDest(Dest);
  addSource(Source);
}

// If LabelTrue==LabelFalse, we turn it into an unconditional branch.
// This ensures that, along with the 'switch' instruction semantics,
// there is at most one edge from one node to another.
IceInstBr::IceInstBr(IceCfg *Cfg, IceOperand *Source, IceCfgNode *TargetTrue,
                     IceCfgNode *TargetFalse)
    : IceInst(Cfg, IceInst::Br), TargetFalse(TargetFalse),
      TargetTrue(TargetTrue) {
  if (TargetTrue != TargetFalse) {
    addSource(Source);
  }
}

IceInstBr::IceInstBr(IceCfg *Cfg, IceCfgNode *Target)
    : IceInst(Cfg, IceInst::Br), TargetFalse(Target), TargetTrue(NULL) {}

IceNodeList IceInstBr::getTerminatorEdges(void) const {
  IceNodeList OutEdges;
  OutEdges.push_back(TargetFalse);
  if (TargetTrue)
    OutEdges.push_back(TargetTrue);
  return OutEdges;
}

IceInstIcmp::IceInstIcmp(IceCfg *Cfg, IceICond Condition, IceVariable *Dest,
                         IceOperand *Source1, IceOperand *Source2)
    : IceInst(Cfg, Icmp), Condition(Condition) {
  addDest(Dest);
  addSource(Source1);
  addSource(Source2);
}

IceInstLoad::IceInstLoad(IceCfg *Cfg, IceVariable *Dest, IceOperand *SourceAddr)
    : IceInst(Cfg, Load) {
  addDest(Dest);
  addSource(SourceAddr);
}

IceInstPhi::IceInstPhi(IceCfg *Cfg, IceVariable *Dest) : IceInst(Cfg, Phi) {
  addDest(Dest);
}

void IceInstPhi::addArgument(IceOperand *Source, IceCfgNode *Label) {
  addSource(Source);
  Labels.push_back(Label);
}

IceOperand *IceInstPhi::getArgument(IceCfgNode *Label) const {
  assert(Labels.size() == Srcs.size());
  IceNodeList::const_iterator EdgeIter = Labels.begin(), EdgeEnd = Labels.end();
  for (IceOpList::const_iterator I = Srcs.begin(), E = Srcs.end();
       I != E && EdgeIter != EdgeEnd; ++I, ++EdgeIter) {
    if (*EdgeIter == Label)
      return *I;
  }
  assert(0);
  return NULL;
}

// Change "a=phi(...)" to "a_phi=phi(...)" and return a new
// instruction "a=a_phi".
IceInst *IceInstPhi::lower(IceCfg *Cfg, IceCfgNode *Node) {
  assert(Dests.size() == 1);
  IceVariable *Dest = getDest(0);
  IceString PhiName = Dest->getName() + "_phi";
  IceVariable *NewSrc = Cfg->makeVariable(Dest->getType(), -1, PhiName);
  Dests.clear();
  addDest(NewSrc);
  IceInstAssign *NewInst = new IceInstAssign(Cfg, Dest, NewSrc);
  // NewInst->updateVars(Node);
  Dest->replaceDefinition(NewInst, Node);
  return NewInst;
}

IceOperand *IceInstPhi::getOperandForTarget(IceCfgNode *Target) const {
  IceNodeList::const_iterator EdgeIter = Labels.begin(), EdgeEnd = Labels.end();
  for (IceOpList::const_iterator I = Srcs.begin(), E = Srcs.end();
       I != E && EdgeIter != EdgeEnd; ++I, ++EdgeIter) {
    if (*EdgeIter == Target)
      return *I;
  }
  return NULL;
}

// Updates liveness for a particular operand based on the given
// predecessor edge.  Doesn't mark the operand as live if the Phi
// instruction is dead or deleted.  TODO: Make sure liveness
// convergence works correctly for dead instructions.
void IceInstPhi::livenessPhiOperand(llvm::BitVector &Live, IceCfgNode *Target) {
  if (isDeleted() || Dead)
    return;
  IceNodeList::const_iterator EdgeIter = Labels.begin(), EdgeEnd = Labels.end();
  uint32_t Index = 0;
  for (IceOpList::const_iterator I = Srcs.begin(), E = Srcs.end();
       I != E && EdgeIter != EdgeEnd; ++I, ++EdgeIter, ++Index) {
    if (*EdgeIter == Target) {
      if (IceVariable *Var = llvm::dyn_cast_or_null<IceVariable>(*I)) {
        uint32_t SrcIndex = Var->getIndex();
        if (!Live[SrcIndex]) {
          LiveRangesEnded[Index] = true;
          Live[SrcIndex] = true;
        }
      }
      return;
    }
  }
  assert(0);
}

IceInstRet::IceInstRet(IceCfg *Cfg, IceOperand *Source) : IceInst(Cfg, Ret) {
  if (Source)
    addSource(Source);
}

void IceInstTarget::setRegState(const IceRegManager *State) {
  RegState = new IceRegManager(*State);
}

// ======================== Dump routines ======================== //

IceOstream &operator<<(IceOstream &Str, const IceInst *I) {
  if (I->isDeleted() && !Str.isVerbose(IceV_Deleted))
    return Str;
  if (Str.isVerbose(IceV_InstNumbers)) {
    char buf[30];
    int Number = I->getNumber();
    if (Number < 0)
      sprintf(buf, "[XXX]");
    else
      sprintf(buf, "[%3d]", I->getNumber());
    Str << buf;
  }
  Str << "  ";
  if (I->isDeleted())
    Str << "  //";
  I->dump(Str);
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
  if (Str.isVerbose(IceV_Liveness)) {
    unsigned Index = 0;
    for (IceOpList::const_iterator I = Srcs.begin(), E = Srcs.end(); I != E;
         ++I, ++Index) {
      if (*I == NULL)
        continue;
      if (LiveRangesEnded[Index]) {
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
}

void IceInst::dumpSources(IceOstream &Str) const {
  for (IceOpList::const_iterator I = Srcs.begin(), E = Srcs.end(); I != E;
       ++I) {
    if (*I == NULL)
      continue;
    if (I != Srcs.begin())
      Str << ", ";
    Str << *I;
  }
}

void IceInst::dumpDests(IceOstream &Str) const {
  for (IceVarList::const_iterator I = Dests.begin(), E = Dests.end(); I != E;
       ++I) {
    if (I != Dests.begin())
      Str << " ";
    Str << *I;
  }
}

void IceInstAlloca::dump(IceOstream &Str) const {
  dumpDests(Str);
  Str << " = alloca " << IceType_i8 << ", i32 " << Size << ", align " << Align;
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
  Str << " " << getDest(0)->getType() << " ";
  dumpSources(Str);
}

void IceInstAssign::dump(IceOstream &Str) const {
  dumpDests(Str);
  Str << " = " << getDest(0)->getType() << " ";
  dumpSources(Str);
}

void IceInstBr::dump(IceOstream &Str) const {
  dumpDests(Str);
  Str << "br ";
  if (getTargetTrue()) {
    Str << "i1 " << Srcs[0] << ", label %" << getTargetTrue()->getName()
        << ", ";
  }
  Str << "label %" << getTargetFalse()->getName();
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
  Str << " " << getSrc(0)->getType() << " ";
  dumpSources(Str);
}

void IceInstLoad::dump(IceOstream &Str) const {
  dumpDests(Str);
  IceType Type = getDest(0)->getType();
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
  Str << " = phi " << getDest(0)->getType() << " ";
  IceNodeList::const_iterator EdgeIter = Labels.begin(), EdgeEnd = Labels.end();
  for (IceOpList::const_iterator I = Srcs.begin(), E = Srcs.end();
       I != E && EdgeIter != EdgeEnd; ++I, ++EdgeIter) {
    if (I != Srcs.begin())
      Str << ", ";
    Str << "[ " << *I << ", %" << (*EdgeIter)->getName() << " ]";
  }
}

void IceInstRet::dump(IceOstream &Str) const {
  IceType Type = Srcs.empty() ? IceType_void : getSrc(0)->getType();
  Str << "ret " << Type << " ";
  dumpSources(Str);
}

void IceInstTarget::dump(IceOstream &Str) const {
  Str << "[TARGET] ";
  IceInst::dump(Str);
}

void IceInstTarget::dumpExtras(IceOstream &Str) const {
  IceInst::dumpExtras(Str);
  if (Str.isVerbose(IceV_RegManager)) {
    if (RegState) {
      Str << " //";
      RegState->dump(Str);
    }
  }
}
