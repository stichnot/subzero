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

IceInst::IceInst(IceCfg *Cfg, IceInstType Kind, unsigned MaxSrcs)
    : Kind(Kind), MaxSrcs(MaxSrcs), NumSrcs(0), Deleted(false), Dead(false),
      Dest(NULL), LiveRangesEnded(0) {
  Number = Cfg->newInstNumber();
  Srcs = new IceOperand *[MaxSrcs];
  for (unsigned i = 0; i < MaxSrcs; ++i)
    Srcs[i] = NULL;
}

void IceInst::renumber(IceCfg *Cfg) {
  Number = isDeleted() ? -1 : Cfg->newInstNumber();
}

void IceInst::deleteIfDead(void) {
  if (Dead)
    setDeleted();
}

void IceInst::updateVars(IceCfgNode *Node) {
  // update variables in Dests
  if (Dest)
    Dest->setDefinition(this, Node);
  for (unsigned I = 0; I < getSrcSize(); ++I) {
    if (Srcs[I])
      Srcs[I]->setUse(this, Node);
  }
}

void IceInst::addDest(IceVariable *NewDest) {
  assert(Dest == NULL);
  assert(NewDest);
  Dest = NewDest;
}

void IceInst::addSource(IceOperand *Source) {
  assert(NumSrcs < MaxSrcs);
  Srcs[NumSrcs++] = Source;
}

void IceInst::doAddressOpt(IceVariable *&Base, IceVariable *&Index, int &Shift,
                           int32_t &Offset) const {
  if (Base == NULL) // shouldn't happen
    return;
  // If the Base has more than one use or is live across multiple
  // blocks, then don't go further.  Alternatively (?), never consider
  // a transformation that would change a variable that is currently
  // *not* live across basic block boundaries into one that *is*.
  if (Base->isMultiblockLife() /* || Base->getUseCount() > 1*/)
    return;

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
    if (Index == NULL && BaseInst && llvm::isa<IceInstArithmetic>(BaseInst) &&
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
    IceConstantInteger *IndexConstant1 =
        llvm::dyn_cast_or_null<IceConstantInteger>(IndexOperand1);
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

void IceInst::liveness(IceLiveness Mode, int InstNumber, llvm::BitVector &Live,
                       std::vector<int> &LiveBegin, std::vector<int> &LiveEnd) {
  if (isDeleted())
    return;
  if (llvm::isa<IceInstFakeKill>(this))
    return;

  // For lightweight liveness, do the simple calculation and return.
  if (Mode == IceLiveness_LREndLightweight) {
    resetLastUses();
    for (unsigned I = 0; I < getSrcSize(); ++I) {
      if (IceVariable *Var = llvm::dyn_cast_or_null<IceVariable>(Srcs[I])) {
        if (Var->isMultiblockLife())
          continue;
        uint32_t Index = Var->getIndex();
        if (Live[Index])
          continue;
        Live[Index] = true;
        setLastUse(I);
      }
    }
    return;
  }

  Dead = false;
  if (Dest) {
    unsigned VarNum = Dest->getIndex();
    if (Live[VarNum]) {
      Live[VarNum] = false;
      LiveBegin[VarNum] = InstNumber;
    } else {
      Dead = true;
    }
  }
  if (Dead)
    return;
  // Phi arguments only get added to Live in the predecessor node, but
  // we still need to update LiveRangesEnded.
  bool IsPhi = llvm::isa<IceInstPhi>(this);
  resetLastUses();
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
  for (unsigned I = 0; I < getSrcSize(); ++I) {
    if (IceVariable *Var = llvm::dyn_cast_or_null<IceVariable>(Srcs[I])) {
      uint32_t VarNum = Var->getIndex();
      if (!Live[VarNum]) {
        setLastUse(I);
        if (!IsPhi) {
          Live[VarNum] = true;
          // For a variable in SSA form, its live range can end at
          // most once in a basic block.  However, after lowering to
          // two-address instructions, we end up with sequences like
          // "t=b;t+=c;a=t" where t's live range begins and ends
          // twice.  ICE only allows a variable to have a single
          // liveness interval in a basic block (except for blocks
          // where a variable is live-in and live-out but there is a
          // gap in the middle, and except for the special
          // IceInstFakeKill instruction that can appear multiple
          // times in the same block).  Therefore, this lowered
          // sequence needs to represent a single conservative live
          // range for t.  Since the instructions are being traversed
          // backwards, we make sure LiveEnd is only set once by
          // setting it only when LiveEnd[VarNum]==0.  Note that it's
          // OK to set LiveBegin multiple times because of the
          // backwards traversal.
          if (LiveEnd[VarNum] == 0) {
            LiveEnd[VarNum] = InstNumber;
            if (I == 1 && getKind() == Arithmetic) {
              // TODO: Do the same for target-specific Arithmetic
              // instructions, and also optimize for commutativity.
              // Also, consider moving this special logic into
              // IceCfgNode::livenessPostprocess().
              LiveEnd[VarNum] = InstNumber /* + 1*/;
            }
          }
        }
      }
    }
  }
}

IceInstArithmetic::IceInstArithmetic(IceCfg *Cfg, OpKind Op, IceVariable *Dest,
                                     IceOperand *Source1, IceOperand *Source2)
    : IceInst(Cfg, IceInst::Arithmetic, 2), Op(Op) {
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
    : IceInst(Cfg, IceInst::Assign, 1) {
  addDest(Dest);
  addSource(Source);
}

// If LabelTrue==LabelFalse, we turn it into an unconditional branch.
// This ensures that, along with the 'switch' instruction semantics,
// there is at most one edge from one node to another.
IceInstBr::IceInstBr(IceCfg *Cfg, IceOperand *Source, IceCfgNode *TargetTrue,
                     IceCfgNode *TargetFalse)
    : IceInst(Cfg, IceInst::Br, 1), TargetFalse(TargetFalse),
      TargetTrue(TargetTrue) {
  if (TargetTrue != TargetFalse) {
    addSource(Source);
  }
}

IceInstBr::IceInstBr(IceCfg *Cfg, IceCfgNode *Target)
    : IceInst(Cfg, IceInst::Br, 0), TargetFalse(Target), TargetTrue(NULL) {}

IceNodeList IceInstBr::getTerminatorEdges(void) const {
  IceNodeList OutEdges;
  OutEdges.push_back(TargetFalse);
  if (TargetTrue)
    OutEdges.push_back(TargetTrue);
  return OutEdges;
}

IceInstCast::IceInstCast(IceCfg *Cfg, IceCastKind CastKind, IceVariable *Dest,
                         IceOperand *Source)
    : IceInst(Cfg, IceInst::Cast, 1), CastKind(CastKind) {
  addDest(Dest);
  addSource(Source);
}

IceInstIcmp::IceInstIcmp(IceCfg *Cfg, IceICond Condition, IceVariable *Dest,
                         IceOperand *Source1, IceOperand *Source2)
    : IceInst(Cfg, IceInst::Icmp, 2), Condition(Condition) {
  addDest(Dest);
  addSource(Source1);
  addSource(Source2);
}

IceInstLoad::IceInstLoad(IceCfg *Cfg, IceVariable *Dest, IceOperand *SourceAddr)
    : IceInst(Cfg, IceInst::Load, 4) {
  addDest(Dest);
  IceVariable *Index = NULL;
  int Shift = 0;
  int32_t Offset = 0;
  if (IceVariable *Base = llvm::dyn_cast<IceVariable>(SourceAddr)) {
    doAddressOpt(Base, Index, Shift, Offset);
    if (Base != SourceAddr) {
      if (Cfg->Str.isVerbose())
        Cfg->Str << "Found AddressOpt opportunity: BaseOrig=" << SourceAddr
                 << " Base=" << Base << " Index=" << Index << " Shift=" << Shift
                 << " Offset=" << Offset << "\n";
      addSource(Base);
      addSource(Index);
      addSource(new IceConstantInteger(IceType_i32, Shift));
      addSource(new IceConstantInteger(IceType_i32, Offset));
      return;
    }
  }
  addSource(SourceAddr);
}

IceInstStore::IceInstStore(IceCfg *Cfg, IceOperand *Val, IceOperand *Addr)
    : IceInst(Cfg, IceInst::Store, 5) {
  addSource(Val);
  IceVariable *Index = NULL;
  int Shift = 0;
  int32_t Offset = 0;
  if (IceVariable *Base = llvm::dyn_cast<IceVariable>(Addr)) {
    doAddressOpt(Base, Index, Shift, Offset);
    if (Base != Addr) {
      if (Cfg->Str.isVerbose())
        Cfg->Str << "Found AddressOpt opportunity: BaseOrig=" << Addr
                 << " Base=" << Base << " Index=" << Index << " Shift=" << Shift
                 << " Offset=" << Offset << "\n";
      addSource(Base);
      addSource(Index);
      addSource(new IceConstantInteger(IceType_i32, Shift));
      addSource(new IceConstantInteger(IceType_i32, Offset));
      return;
    }
  }
  addSource(Addr);
}

IceInstPhi::IceInstPhi(IceCfg *Cfg, unsigned MaxSrcs, IceVariable *Dest)
    : IceInst(Cfg, Phi, MaxSrcs) {
  addDest(Dest);
}

void IceInstPhi::addArgument(IceOperand *Source, IceCfgNode *Label) {
  addSource(Source);
  Labels.push_back(Label);
}

IceOperand *IceInstPhi::getArgument(IceCfgNode *Label) const {
  assert(Labels.size() == getSrcSize());
  IceNodeList::const_iterator EdgeIter = Labels.begin(), EdgeEnd = Labels.end();
  for (unsigned I = 0; I < getSrcSize() && EdgeIter != EdgeEnd;
       ++I, ++EdgeIter) {
    if (*EdgeIter == Label)
      return Srcs[I];
  }
  assert(0);
  return NULL;
}

// Change "a=phi(...)" to "a_phi=phi(...)" and return a new
// instruction "a=a_phi".
IceInst *IceInstPhi::lower(IceCfg *Cfg, IceCfgNode *Node) {
  assert(getDest());
  IceVariable *Dest = getDest();
  IceString PhiName = Dest->getName() + "_phi";
  IceVariable *NewSrc = Cfg->makeVariable(Dest->getType(), -1, PhiName);
  this->Dest = NewSrc;
  IceInstAssign *NewInst = new IceInstAssign(Cfg, Dest, NewSrc);
  Dest->setPreferredRegister(NewSrc, false);
  NewSrc->setPreferredRegister(Dest, false);
  Dest->replaceDefinition(NewInst, Node);
  return NewInst;
}

IceOperand *IceInstPhi::getOperandForTarget(IceCfgNode *Target) const {
  IceNodeList::const_iterator EdgeIter = Labels.begin(), EdgeEnd = Labels.end();
  for (unsigned I = 0; I < getSrcSize() && EdgeIter != EdgeEnd;
       ++I, ++EdgeIter) {
    if (*EdgeIter == Target)
      return Srcs[I];
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
  for (uint32_t I = 0; I < getSrcSize() && EdgeIter != EdgeEnd;
       ++EdgeIter, ++I) {
    if (*EdgeIter == Target) {
      if (IceVariable *Var = llvm::dyn_cast_or_null<IceVariable>(Srcs[I])) {
        uint32_t SrcIndex = Var->getIndex();
        if (!Live[SrcIndex]) {
          setLastUse(I);
          Live[SrcIndex] = true;
        }
      }
      return;
    }
  }
  assert(0);
}

IceInstRet::IceInstRet(IceCfg *Cfg, IceOperand *Source)
    : IceInst(Cfg, Ret, Source ? 1 : 0) {
  if (Source)
    addSource(Source);
}

void IceInstTarget::setRegState(const IceRegManager *State) {
  RegState = new IceRegManager(*State);
}

IceInstFakeDef::IceInstFakeDef(IceCfg *Cfg, IceVariable *Dest, IceVariable *Src)
    : IceInst(Cfg, IceInst::FakeDef, Src ? 1 : 0) {
  assert(Dest);
  addDest(Dest);
  if (Src)
    addSource(Src);
}

IceInstFakeUse::IceInstFakeUse(IceCfg *Cfg, IceVariable *Src)
    : IceInst(Cfg, IceInst::FakeUse, 1) {
  assert(Src);
  addSource(Src);
}

IceInstFakeKill::IceInstFakeKill(IceCfg *Cfg, const IceVarList &KilledRegs,
                                 const IceInst *Linked)
    : IceInst(Cfg, IceInst::FakeKill, KilledRegs.size()), Linked(Linked) {
  for (IceVarList::const_iterator I = KilledRegs.begin(), E = KilledRegs.end();
       I != E; ++I) {
    IceVariable *Var = *I;
    addSource(Var);
  }
}

// ======================== Dump routines ======================== //

IceOstream &operator<<(IceOstream &Str, const IceInst *I) {
  if (!Str.isVerbose(IceV_Deleted) &&
      (I->isDeleted() || I->isRedundantAssign()))
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
  dumpDest(Str);
  Str << " =~ ";
  dumpSources(Str);
}

void IceInst::dumpExtras(IceOstream &Str) const {
  bool First = true;
  // Print "LIVEEND={a,b,c}" for all source operands whose live ranges
  // are known to end at this instruction.
  if (Str.isVerbose(IceV_Liveness)) {
    for (unsigned I = 0; I < getSrcSize(); ++I) {
      if (Srcs[I] == NULL)
        continue;
      if (isLastUse(I)) {
        if (First)
          Str << " // LIVEEND={";
        else
          Str << ",";
        Str << Srcs[I];
        First = false;
      }
    }
    if (!First)
      Str << "}";
  }
}

void IceInst::dumpSources(IceOstream &Str) const {
  for (unsigned I = 0; I < getSrcSize(); ++I) {
    if (I > 0)
      Str << ", ";
    Str << Srcs[I];
  }
}

void IceInst::dumpDest(IceOstream &Str) const {
  if (Dest)
    Str << Dest;
}

void IceInstAlloca::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = alloca " << IceType_i8 << ", i32 " << Size << ", align " << Align;
}

void IceInstArithmetic::dump(IceOstream &Str) const {
  dumpDest(Str);
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
  case OpKind_NUM:
    Str << "invalid";
    break;
  }
  Str << " " << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstAssign::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = " << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstBr::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << "br ";
  if (getTargetTrue()) {
    Str << "i1 " << Srcs[0] << ", label %" << getTargetTrue()->getName()
        << ", ";
  }
  Str << "label %" << getTargetFalse()->getName();
}

void IceInstCall::dump(IceOstream &Str) const {
  if (getDest()) {
    dumpDest(Str);
    Str << " = ";
  }
  if (Tail)
    Str << "tail ";
  Str << "call " << getCallTarget() << "(";
  dumpSources(Str);
  Str << ")";
}

void IceInstCast::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = ";
  switch (CastKind) {
  default:
    Str << "UNKNOWN";
    break;
  case Sext:
    Str << "sext";
    break;
  case Zext:
    Str << "zext";
    break;
  case Trunc:
    Str << "trunc";
    break;
  }
  Str << " " << getSrc(0)->getType() << " ";
  dumpSources(Str);
  Str << " to " << getDest()->getType();
}

void IceInstIcmp::dump(IceOstream &Str) const {
  dumpDest(Str);
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
  dumpDest(Str);
  IceType Type = getDest()->getType();
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

void IceInstStore::dump(IceOstream &Str) const {
  IceType Type = getSrc(0)->getType();
  Str << "store " << Type << "* ";
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
  dumpDest(Str);
  Str << " = phi " << getDest()->getType() << " ";
  IceNodeList::const_iterator EdgeIter = Labels.begin(), EdgeEnd = Labels.end();
  for (unsigned I = 0; I < getSrcSize() && EdgeIter != EdgeEnd;
       ++I, ++EdgeIter) {
    if (I > 0)
      Str << ", ";
    Str << "[ " << Srcs[I] << ", %" << (*EdgeIter)->getName() << " ]";
  }
}

void IceInstRet::dump(IceOstream &Str) const {
  IceType Type = getSrcSize() == 0 ? IceType_void : getSrc(0)->getType();
  Str << "ret " << Type << " ";
  dumpSources(Str);
}

void IceInstFakeDef::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = def.pseudo ";
  dumpSources(Str);
}

void IceInstFakeUse::dump(IceOstream &Str) const {
  Str << "use.pseudo ";
  dumpSources(Str);
}

void IceInstFakeKill::dump(IceOstream &Str) const {
  if (Linked->isDeleted())
    Str << "// ";
  Str << "kill.pseudo ";
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
