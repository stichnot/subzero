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

IceInst::IceInst(IceCfg *Cfg, IceInstType Kind, unsigned MaxSrcs,
                 IceVariable *Dest)
    : Kind(Kind), MaxSrcs(MaxSrcs), NumSrcs(0), Deleted(false), Dead(false),
      Dest(Dest), LiveRangesEnded(0) {
  Number = Cfg->newInstNumber();
  Srcs = new IceOperand *[MaxSrcs]; // TODO: use placement alloc from Cfg
}

// If Src is an IceVariable, it returns true if this instruction ends
// Src's live range.  Otherwise, returns false.
bool IceInst::isLastUse(const IceOperand *TestSrc) const {
  if (LiveRangesEnded == 0)
    return false; // early-exit optimization
  if (const IceVariable *TestVar = llvm::dyn_cast<const IceVariable>(TestSrc)) {
    unsigned VarIndex = 0;
    uint32_t Mask = LiveRangesEnded;
    for (unsigned I = 0; I < getSrcSize(); ++I) {
      IceOperand *Src = getSrc(I);
      unsigned NumVars = Src->getNumVars();
      for (unsigned J = 0; J < NumVars; ++J, ++VarIndex) {
        const IceVariable *Var = Src->getVar(J);
        if (Var == TestVar) {
          // We've found where the variable is used in the instruction
          return Mask & 1;
        }
        Mask >>= 1;
        if (Mask == 0)
          return false;
      }
    }
  }
  return false;
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
    if (IceVariable *Var = llvm::dyn_cast<IceVariable>(getSrc(I)))
      Var->setUse(this, Node);
  }
}

void IceInst::addSource(IceOperand *Source) {
  assert(Source);
  assert(NumSrcs < MaxSrcs);
  Srcs[NumSrcs++] = Source;
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
    unsigned VarIndex = 0;
    for (unsigned I = 0; I < getSrcSize(); ++I) {
      IceOperand *Src = getSrc(I);
      unsigned NumVars = Src->getNumVars();
      for (unsigned J = 0; J < NumVars; ++J, ++VarIndex) {
        const IceVariable *Var = Src->getVar(J);
        if (Var->isMultiblockLife())
          continue;
        uint32_t Index = Var->getIndex();
        if (Live[Index])
          continue;
        Live[Index] = true;
        setLastUse(VarIndex);
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
  unsigned VarIndex = 0;
  for (unsigned I = 0; I < getSrcSize(); ++I) {
    IceOperand *Src = getSrc(I);
    unsigned NumVars = Src->getNumVars();
    for (unsigned J = 0; J < NumVars; ++J, ++VarIndex) {
      const IceVariable *Var = Src->getVar(J);
      uint32_t VarNum = Var->getIndex();
      if (!Live[VarNum]) {
        setLastUse(VarIndex);
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

IceInstAlloca::IceInstAlloca(IceCfg *Cfg, uint32_t Size, uint32_t Align,
                             IceVariable *Dest)
    : IceInst(Cfg, IceInst::Alloca, 0, Dest), Size(Size), Align(Align) {}

IceInstArithmetic::IceInstArithmetic(IceCfg *Cfg, OpKind Op, IceVariable *Dest,
                                     IceOperand *Source1, IceOperand *Source2)
    : IceInst(Cfg, IceInst::Arithmetic, 2, Dest), Op(Op) {
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
    : IceInst(Cfg, IceInst::Assign, 1, Dest) {
  addSource(Source);
}

// If LabelTrue==LabelFalse, we turn it into an unconditional branch.
// This ensures that, along with the 'switch' instruction semantics,
// there is at most one edge from one node to another.
IceInstBr::IceInstBr(IceCfg *Cfg, IceOperand *Source, IceCfgNode *TargetTrue,
                     IceCfgNode *TargetFalse)
    : IceInst(Cfg, IceInst::Br, 1, NULL), TargetFalse(TargetFalse),
      TargetTrue(TargetTrue) {
  if (TargetTrue != TargetFalse) {
    addSource(Source);
  }
}

IceInstBr::IceInstBr(IceCfg *Cfg, IceCfgNode *Target)
    : IceInst(Cfg, IceInst::Br, 0, NULL), TargetFalse(Target),
      TargetTrue(NULL) {}

IceNodeList IceInstBr::getTerminatorEdges(void) const {
  IceNodeList OutEdges;
  OutEdges.push_back(TargetFalse);
  if (TargetTrue)
    OutEdges.push_back(TargetTrue);
  return OutEdges;
}

IceInstCast::IceInstCast(IceCfg *Cfg, IceCastKind CastKind, IceVariable *Dest,
                         IceOperand *Source)
    : IceInst(Cfg, IceInst::Cast, 1, Dest), CastKind(CastKind) {
  addSource(Source);
}

IceInstIcmp::IceInstIcmp(IceCfg *Cfg, IceICond Condition, IceVariable *Dest,
                         IceOperand *Source1, IceOperand *Source2)
    : IceInst(Cfg, IceInst::Icmp, 2, Dest), Condition(Condition) {
  addSource(Source1);
  addSource(Source2);
}

IceInstLoad::IceInstLoad(IceCfg *Cfg, IceVariable *Dest, IceOperand *SourceAddr)
    : IceInst(Cfg, IceInst::Load, 1, Dest) {
  addSource(SourceAddr);
}

IceInstStore::IceInstStore(IceCfg *Cfg, IceOperand *Val, IceOperand *Addr)
    : IceInst(Cfg, IceInst::Store, 2, NULL) {
  addSource(Val);
  addSource(Addr);
}

IceInstSwitch::IceInstSwitch(IceCfg *Cfg, unsigned NumCases, IceOperand *Source,
                             IceCfgNode *LabelDefault)
    : IceInst(Cfg, IceInst::Switch, 1, NULL), LabelDefault(LabelDefault),
      NumCases(NumCases) {
  addSource(Source);
  Values = new uint64_t[NumCases];
  Labels = new IceCfgNode *[NumCases];
  // Initialize in case buggy code doesn't set all entries
  for (unsigned I = 0; I < NumCases; ++I) {
    Values[I] = 0;
    Labels[I] = NULL;
  }
}

void IceInstSwitch::addBranch(unsigned CaseIndex, uint64_t Value,
                              IceCfgNode *Label) {
  assert(CaseIndex < NumCases);
  assert(Labels[CaseIndex] == NULL);
  Values[CaseIndex] = Value;
  Labels[CaseIndex] = Label;
}

IceNodeList IceInstSwitch::getTerminatorEdges(void) const {
  IceNodeList OutEdges;
  OutEdges.push_back(LabelDefault);
  for (unsigned I = 0; I < NumCases; ++I) {
    OutEdges.push_back(Labels[I]);
  }
  return OutEdges;
}

IceInstPhi::IceInstPhi(IceCfg *Cfg, unsigned MaxSrcs, IceVariable *Dest)
    : IceInst(Cfg, Phi, MaxSrcs, Dest) {}

// TODO: A Switch instruction (and maybe others) can add duplicate
// edges.  We may want to de-dup Phis and validate consistency, though
// it seems the current lowering code is OK with this situation.
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
      return getSrc(I);
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
  IceInstAssign *NewInst = IceInstAssign::create(Cfg, Dest, NewSrc);
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
      return getSrc(I);
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
      if (IceVariable *Var = llvm::dyn_cast<IceVariable>(getSrc(I))) {
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
    : IceInst(Cfg, Ret, Source ? 1 : 0, NULL) {
  if (Source)
    addSource(Source);
}

IceInstSelect::IceInstSelect(IceCfg *Cfg, IceVariable *Dest,
                             IceOperand *Condition, IceOperand *SourceTrue,
                             IceOperand *SourceFalse)
    : IceInst(Cfg, IceInst::Select, 3, Dest) {
  assert(Condition->getType() == IceType_i1);
  addSource(Condition);
  addSource(SourceTrue);
  addSource(SourceFalse);
}

void IceInstTarget::setRegState(const IceRegManager *State) {
  RegState = IceRegManager::create(*State);
}

IceInstFakeDef::IceInstFakeDef(IceCfg *Cfg, IceVariable *Dest, IceVariable *Src)
    : IceInst(Cfg, IceInst::FakeDef, Src ? 1 : 0, Dest) {
  assert(Dest);
  if (Src)
    addSource(Src);
}

IceInstFakeUse::IceInstFakeUse(IceCfg *Cfg, IceVariable *Src)
    : IceInst(Cfg, IceInst::FakeUse, 1, NULL) {
  assert(Src);
  addSource(Src);
}

IceInstFakeKill::IceInstFakeKill(IceCfg *Cfg, const IceVarList &KilledRegs,
                                 const IceInst *Linked)
    : IceInst(Cfg, IceInst::FakeKill, KilledRegs.size(), NULL), Linked(Linked) {
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

void IceInst::emit(IceOstream &Str, uint32_t Option) const {
  Str << "??? ";
  dump(Str);
  Str << "\n";
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
    unsigned VarIndex = 0;
    for (unsigned I = 0; I < getSrcSize(); ++I) {
      IceOperand *Src = getSrc(I);
      unsigned NumVars = Src->getNumVars();
      for (unsigned J = 0; J < NumVars; ++J, ++VarIndex) {
        const IceVariable *Var = Src->getVar(J);
        if (isLastUse(Var)) {
          if (First)
            Str << " // LIVEEND={";
          else
            Str << ",";
          Str << Var;
          First = false;
        }
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
    Str << getSrc(I);
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
    Str << "i1 " << getSrc(0) << ", label %" << getTargetTrue()->getName()
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
  case None: // shouldn't happen
    Str << "<none>";
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
  IceType Type = getData()->getType();
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

void IceInstSwitch::dump(IceOstream &Str) const {
  IceType Type = getSrc(0)->getType();
  Str << "switch " << Type << " " << getSrc(0) << ", label %"
      << getLabelDefault()->getName() << " [\n";
  for (unsigned I = 0; I < getNumCases(); ++I) {
    Str << "    " << Type << " " << getValue(I) << ", label %"
        << getLabel(I)->getName() << "\n";
  }
  Str << "  ]";
}

void IceInstPhi::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = phi " << getDest()->getType() << " ";
  IceNodeList::const_iterator EdgeIter = Labels.begin(), EdgeEnd = Labels.end();
  for (unsigned I = 0; I < getSrcSize() && EdgeIter != EdgeEnd;
       ++I, ++EdgeIter) {
    if (I > 0)
      Str << ", ";
    Str << "[ " << getSrc(I) << ", %" << (*EdgeIter)->getName() << " ]";
  }
}

void IceInstRet::dump(IceOstream &Str) const {
  IceType Type = getSrcSize() == 0 ? IceType_void : getSrc(0)->getType();
  Str << "ret " << Type << " ";
  dumpSources(Str);
}

void IceInstSelect::dump(IceOstream &Str) const {
  dumpDest(Str);
  IceOperand *Condition = getCondition();
  IceOperand *TrueOp = getTrueOperand();
  IceOperand *FalseOp = getFalseOperand();
  Str << " = select " << Condition->getType() << " " << Condition << ", "
      << TrueOp->getType() << " " << TrueOp << ", " << FalseOp->getType() << " "
      << FalseOp;
}

void IceInstFakeDef::emit(IceOstream &Str, uint32_t Option) const {
  Str << "\t# ";
  dump(Str);
  Str << "\n";
}

void IceInstFakeDef::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = def.pseudo ";
  dumpSources(Str);
}

void IceInstFakeUse::emit(IceOstream &Str, uint32_t Option) const {
  Str << "\t# ";
  dump(Str);
  Str << "\n";
}

void IceInstFakeUse::dump(IceOstream &Str) const {
  Str << "use.pseudo ";
  dumpSources(Str);
}

void IceInstFakeKill::emit(IceOstream &Str, uint32_t Option) const {
  Str << "\t# ";
  dump(Str);
  Str << "\n";
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
