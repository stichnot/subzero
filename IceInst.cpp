/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInst.h"
#include "IceInstX8632.h"
#include "IceLiveness.h"
#include "IceOperand.h"

IceInst::IceInst(IceCfg *Cfg, InstKind Kind, uint32_t MaxSrcs,
                 IceVariable *Dest)
    : Kind(Kind), Deleted(false), Dead(false), HasSideEffects(false),
      Dest(Dest), MaxSrcs(MaxSrcs), NumSrcs(0), LiveRangesEnded(0) {
  Number = Cfg->newInstNumber();
  Srcs = Cfg->allocateArrayOf<IceOperand *>(MaxSrcs);
}

// Assign the instruction a new number.
void IceInst::renumber(IceCfg *Cfg) {
  Number = isDeleted() ? -1 : Cfg->newInstNumber();
}

// Delete the instruction if its tentative Dead flag is still set
// after liveness analysis.
void IceInst::deleteIfDead(void) {
  if (Dead)
    setDeleted();
}

// If Src is an IceVariable, it returns true if this instruction ends
// Src's live range.  Otherwise, returns false.
bool IceInst::isLastUse(const IceOperand *TestSrc) const {
  if (LiveRangesEnded == 0)
    return false; // early-exit optimization
  if (const IceVariable *TestVar = llvm::dyn_cast<const IceVariable>(TestSrc)) {
    uint32_t VarIndex = 0;
    uint32_t Mask = LiveRangesEnded;
    for (uint32_t I = 0; I < getSrcSize(); ++I) {
      IceOperand *Src = getSrc(I);
      uint32_t NumVars = Src->getNumVars();
      for (uint32_t J = 0; J < NumVars; ++J, ++VarIndex) {
        const IceVariable *Var = Src->getVar(J);
        if (Var == TestVar) {
          // We've found where the variable is used in the instruction.
          return Mask & 1;
        }
        Mask >>= 1;
        if (Mask == 0)
          return false; // another early-exit optimization
      }
    }
  }
  return false;
}

void IceInst::updateVars(IceCfgNode *Node) {
  if (Dest)
    Dest->setDefinition(this, Node);
  for (uint32_t I = 0; I < getSrcSize(); ++I) {
    getSrc(I)->setUse(this, Node);
  }
}

void IceInst::liveness(IceLivenessMode Mode, int32_t InstNumber,
                       llvm::BitVector &Live, IceLiveness *Liveness,
                       const IceCfgNode *Node) {
  if (isDeleted())
    return;
  if (llvm::isa<IceInstFakeKill>(this))
    return;

  // For lightweight liveness, do the simple calculation and return.
  if (Mode == IceLiveness_LREndLightweight) {
    resetLastUses();
    uint32_t VarIndex = 0;
    for (uint32_t I = 0; I < getSrcSize(); ++I) {
      IceOperand *Src = getSrc(I);
      uint32_t NumVars = Src->getNumVars();
      for (uint32_t J = 0; J < NumVars; ++J, ++VarIndex) {
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

  std::vector<int32_t> &LiveBegin = Liveness->getLiveBegin(Node);
  std::vector<int32_t> &LiveEnd = Liveness->getLiveEnd(Node);
  Dead = false;
  if (Dest) {
    uint32_t VarNum = Liveness->getLiveIndex(Dest);
    if (Live[VarNum]) {
      Live[VarNum] = false;
      LiveBegin[VarNum] = InstNumber;
    } else {
      if (!hasSideEffects())
        Dead = true;
    }
  }
  if (Dead)
    return;
  // Phi arguments only get added to Live in the predecessor node, but
  // we still need to update LiveRangesEnded.
  bool IsPhi = llvm::isa<IceInstPhi>(this);
  resetLastUses();
  uint32_t VarIndex = 0;
  for (uint32_t I = 0; I < getSrcSize(); ++I) {
    IceOperand *Src = getSrc(I);
    uint32_t NumVars = Src->getNumVars();
    for (uint32_t J = 0; J < NumVars; ++J, ++VarIndex) {
      const IceVariable *Var = Src->getVar(J);
      uint32_t VarNum = Liveness->getLiveIndex(Var);
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
          // setting it only when LiveEnd[VarNum]==0 (sentinel value).
          // Note that it's OK to set LiveBegin multiple times because
          // of the backwards traversal.
          if (LiveEnd[VarNum] == 0) {
            LiveEnd[VarNum] = InstNumber;
          }
        }
      }
    }
  }
}

IceInstAlloca::IceInstAlloca(IceCfg *Cfg, IceOperand *ByteCount, uint32_t Align,
                             IceVariable *Dest)
    : IceInst(Cfg, IceInst::Alloca, 1, Dest), Align(Align) {
  addSource(ByteCount);
}

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

// If TargetTrue==TargetFalse, we turn it into an unconditional
// branch.  This ensures that, along with the 'switch' instruction
// semantics, there is at most one edge from one node to another.
IceInstBr::IceInstBr(IceCfg *Cfg, IceOperand *Source, IceCfgNode *TargetTrue,
                     IceCfgNode *TargetFalse)
    : IceInst(Cfg, IceInst::Br, 1, NULL), TargetFalse(TargetFalse),
      TargetTrue(TargetTrue) {
  if (TargetTrue == TargetFalse) {
    TargetTrue = NULL; // turn into unconditional version
  } else {
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

IceInstCast::IceInstCast(IceCfg *Cfg, OpKind CastKind, IceVariable *Dest,
                         IceOperand *Source)
    : IceInst(Cfg, IceInst::Cast, 1, Dest), CastKind(CastKind) {
  addSource(Source);
}

IceInstFcmp::IceInstFcmp(IceCfg *Cfg, FCond Condition, IceVariable *Dest,
                         IceOperand *Source1, IceOperand *Source2)
    : IceInst(Cfg, IceInst::Fcmp, 2, Dest), Condition(Condition) {
  addSource(Source1);
  addSource(Source2);
}

IceInstIcmp::IceInstIcmp(IceCfg *Cfg, ICond Condition, IceVariable *Dest,
                         IceOperand *Source1, IceOperand *Source2)
    : IceInst(Cfg, IceInst::Icmp, 2, Dest), Condition(Condition) {
  addSource(Source1);
  addSource(Source2);
}

IceInstLoad::IceInstLoad(IceCfg *Cfg, IceVariable *Dest, IceOperand *SourceAddr)
    : IceInst(Cfg, IceInst::Load, 1, Dest) {
  addSource(SourceAddr);
}

IceInstPhi::IceInstPhi(IceCfg *Cfg, uint32_t MaxSrcs, IceVariable *Dest)
    : IceInst(Cfg, Phi, MaxSrcs, Dest) {
  Labels = Cfg->allocateArrayOf<IceCfgNode *>(MaxSrcs);
}

// TODO: A Switch instruction (and maybe others) can add duplicate
// edges.  We may want to de-dup Phis and validate consistency (i.e.,
// the source operands are the same for duplicate edges), though it
// seems the current lowering code is OK with this situation.
void IceInstPhi::addArgument(IceOperand *Source, IceCfgNode *Label) {
  Labels[getSrcSize()] = Label;
  addSource(Source);
}

// Find the source operand corresponding to the incoming edge for the
// given node.  TODO: This uses a linear-time search, which could be
// improved if it becomes a problem.
IceOperand *IceInstPhi::getOperandForTarget(IceCfgNode *Target) const {
  for (uint32_t I = 0; I < getSrcSize(); ++I) {
    if (Labels[I] == Target)
      return getSrc(I);
  }
  assert(0);
  return NULL;
}

// Updates liveness for a particular operand based on the given
// predecessor edge.  Doesn't mark the operand as live if the Phi
// instruction is dead or deleted.
void IceInstPhi::livenessPhiOperand(llvm::BitVector &Live, IceCfgNode *Target,
                                    IceLiveness *Liveness) {
  if (isDeleted() || Dead)
    return;
  for (uint32_t I = 0; I < getSrcSize(); ++I) {
    if (Labels[I] == Target) {
      if (IceVariable *Var = llvm::dyn_cast<IceVariable>(getSrc(I))) {
        uint32_t SrcIndex = Liveness->getLiveIndex(Var);
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

// Change "a=phi(...)" to "a_phi=phi(...)" and return a new
// instruction "a=a_phi".
IceInst *IceInstPhi::lower(IceCfg *Cfg, IceCfgNode *Node) {
  IceVariable *Dest = getDest();
  assert(Dest);
  IceString PhiName = Dest->getName() + "_phi";
  IceVariable *NewSrc = Cfg->makeVariable(Dest->getType(), Node, PhiName);
  this->Dest = NewSrc;
  IceInstAssign *NewInst = IceInstAssign::create(Cfg, Dest, NewSrc);
  // Set Dest and NewSrc to have affinity with each other, as a hint
  // for register allocation.
  Dest->setPreferredRegister(NewSrc, false);
  NewSrc->setPreferredRegister(Dest, false);
  Dest->replaceDefinition(NewInst, Node);
  return NewInst;
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

IceInstStore::IceInstStore(IceCfg *Cfg, IceOperand *Data, IceOperand *Addr)
    : IceInst(Cfg, IceInst::Store, 2, NULL) {
  addSource(Data);
  addSource(Addr);
}

IceInstSwitch::IceInstSwitch(IceCfg *Cfg, uint32_t NumCases, IceOperand *Source,
                             IceCfgNode *LabelDefault)
    : IceInst(Cfg, IceInst::Switch, 1, NULL), LabelDefault(LabelDefault),
      NumCases(NumCases) {
  addSource(Source);
  Values = Cfg->allocateArrayOf<uint64_t>(NumCases);
  Labels = Cfg->allocateArrayOf<IceCfgNode *>(NumCases);
  // Initialize in case buggy code doesn't set all entries
  for (uint32_t I = 0; I < NumCases; ++I) {
    Values[I] = 0;
    Labels[I] = NULL;
  }
}

void IceInstSwitch::addBranch(uint32_t CaseIndex, uint64_t Value,
                              IceCfgNode *Label) {
  assert(CaseIndex < NumCases);
  Values[CaseIndex] = Value;
  Labels[CaseIndex] = Label;
}

IceNodeList IceInstSwitch::getTerminatorEdges(void) const {
  IceNodeList OutEdges;
  OutEdges.push_back(LabelDefault);
  for (uint32_t I = 0; I < NumCases; ++I) {
    OutEdges.push_back(Labels[I]);
  }
  return OutEdges;
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
    int32_t Number = I->getNumber();
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
  Str.Cfg->setError("emit() called on a non-lowered instruction");
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
    uint32_t VarIndex = 0;
    for (uint32_t I = 0; I < getSrcSize(); ++I) {
      IceOperand *Src = getSrc(I);
      uint32_t NumVars = Src->getNumVars();
      for (uint32_t J = 0; J < NumVars; ++J, ++VarIndex) {
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
  for (uint32_t I = 0; I < getSrcSize(); ++I) {
    if (I > 0)
      Str << ", ";
    Str << getSrc(I);
  }
}

void IceInst::dumpDest(IceOstream &Str) const {
  if (getDest())
    Str << getDest();
}

void IceInstAlloca::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = alloca i8, i32 ";
  Str << getSrc(0) << ", align " << Align;
}

void IceInstArithmetic::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = ";
  switch (getOp()) {
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
  if (!isUnconditional()) {
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
  Str << "call ";
  if (getDest())
    Str << getDest()->getType();
  else
    Str << "void";
  Str << " " << getCallTarget() << "(";
  for (uint32_t I = 0; I < getNumArgs(); ++I) {
    if (I > 0)
      Str << ", ";
    Str << getArg(I)->getType() << " " << getArg(I);
  }
  Str << ")";
}

void IceInstCast::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = ";
  switch (getCastKind()) {
  default:
    Str << "UNKNOWN";
    assert(0);
    break;
  case Trunc:
    Str << "trunc";
    break;
  case Zext:
    Str << "zext";
    break;
  case Sext:
    Str << "sext";
    break;
  case Fptrunc:
    Str << "fptrunc";
    break;
  case Fpext:
    Str << "fpext";
    break;
  case Fptoui:
    Str << "fptoui";
    break;
  case Fptosi:
    Str << "fptosi";
    break;
  case Uitofp:
    Str << "uitofp";
    break;
  case Sitofp:
    Str << "sitofp";
    break;
  case Inttoptr:
    Str << "inttoptr";
    break;
  }
  Str << " " << getSrc(0)->getType() << " ";
  dumpSources(Str);
  Str << " to " << getDest()->getType();
  if (getCastKind() == Inttoptr)
    Str << "*";
}

void IceInstIcmp::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = icmp ";
  switch (getCondition()) {
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

void IceInstFcmp::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = fcmp ";

  switch (getCondition()) {
  case False:
    Str << "false";
    break;
  case Oeq:
    Str << "oeq";
    break;
  case Ogt:
    Str << "ogt";
    break;
  case Oge:
    Str << "oge";
    break;
  case Olt:
    Str << "olt";
    break;
  case Ole:
    Str << "ole";
    break;
  case One:
    Str << "one";
    break;
  case Ord:
    Str << "ord";
    break;
  case Ueq:
    Str << "ueq";
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
  case Une:
    Str << "une";
    break;
  case Uno:
    Str << "uno";
    break;
  case True:
    Str << "true";
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
  Str << "store " << Type << " " << getData() << ", " << Type << "* "
      << getAddr() << ", align ";
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
  for (uint32_t I = 0; I < getNumCases(); ++I) {
    Str << "    " << Type << " " << getValue(I) << ", label %"
        << getLabel(I)->getName() << "\n";
  }
  Str << "  ]";
}

void IceInstPhi::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = phi " << getDest()->getType() << " ";
  for (uint32_t I = 0; I < getSrcSize(); ++I) {
    if (I > 0)
      Str << ", ";
    Str << "[ " << getSrc(I) << ", %" << Labels[I]->getName() << " ]";
  }
}

void IceInstRet::dump(IceOstream &Str) const {
  IceType Type = getSrcSize() == 0 ? IceType_void : getSrc(0)->getType();
  Str << "ret " << Type;
  if (getSrcSize()) {
    Str << " ";
    dumpSources(Str);
  }
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
}
