//===- subzero/src/Inst.cpp - High-level instruction implementation ----===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Inst class, primarily the various
// subclass constructors and dump routines.
//
//===----------------------------------------------------------------------===//

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInst.h"
#include "IceInstX8632.h"
#include "IceLiveness.h"
#include "IceOperand.h"

namespace Ice {

Inst::Inst(IceCfg *Cfg, InstKind Kind, uint32_t MaxSrcs, Variable *Dest)
    : Kind(Kind), Deleted(false), Dead(false), HasSideEffects(false),
      Dest(Dest), MaxSrcs(MaxSrcs), NumSrcs(0), LiveRangesEnded(0) {
  Number = Cfg->newInstNumber();
  Srcs = Cfg->allocateArrayOf<Operand *>(MaxSrcs);
}

// Assign the instruction a new number.
void Inst::renumber(IceCfg *Cfg) {
  Number = isDeleted() ? -1 : Cfg->newInstNumber();
}

// Delete the instruction if its tentative Dead flag is still set
// after liveness analysis.
void Inst::deleteIfDead() {
  if (Dead)
    setDeleted();
}

// If Src is an Variable, it returns true if this instruction ends
// Src's live range.  Otherwise, returns false.
bool Inst::isLastUse(const Operand *TestSrc) const {
  if (LiveRangesEnded == 0)
    return false; // early-exit optimization
  if (const Variable *TestVar = llvm::dyn_cast<const Variable>(TestSrc)) {
    uint32_t VarIndex = 0;
    uint32_t Mask = LiveRangesEnded;
    for (uint32_t I = 0; I < getSrcSize(); ++I) {
      Operand *Src = getSrc(I);
      uint32_t NumVars = Src->getNumVars();
      for (uint32_t J = 0; J < NumVars; ++J, ++VarIndex) {
        const Variable *Var = Src->getVar(J);
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

void Inst::updateVars(CfgNode *Node) {
  if (Dest)
    Dest->setDefinition(this, Node);
  for (uint32_t I = 0; I < getSrcSize(); ++I) {
    getSrc(I)->setUse(this, Node);
  }
}

void Inst::liveness(LivenessMode Mode, int32_t InstNumber,
                    llvm::BitVector &Live, Liveness *Liveness,
                    const CfgNode *Node) {
  if (isDeleted())
    return;
  if (llvm::isa<InstFakeKill>(this))
    return;

  // For lightweight liveness, do the simple calculation and return.
  if (Mode == Liveness_LREndLightweight) {
    resetLastUses();
    uint32_t VarIndex = 0;
    for (uint32_t I = 0; I < getSrcSize(); ++I) {
      Operand *Src = getSrc(I);
      uint32_t NumVars = Src->getNumVars();
      for (uint32_t J = 0; J < NumVars; ++J, ++VarIndex) {
        const Variable *Var = Src->getVar(J);
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
  bool IsPhi = llvm::isa<InstPhi>(this);
  resetLastUses();
  uint32_t VarIndex = 0;
  for (uint32_t I = 0; I < getSrcSize(); ++I) {
    Operand *Src = getSrc(I);
    uint32_t NumVars = Src->getNumVars();
    for (uint32_t J = 0; J < NumVars; ++J, ++VarIndex) {
      const Variable *Var = Src->getVar(J);
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
          // InstFakeKill instruction that can appear multiple
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

InstAlloca::InstAlloca(IceCfg *Cfg, Operand *ByteCount, uint32_t Align,
                       Variable *Dest)
    : Inst(Cfg, Inst::Alloca, 1, Dest), Align(Align) {
  addSource(ByteCount);
}

InstArithmetic::InstArithmetic(IceCfg *Cfg, OpKind Op, Variable *Dest,
                               Operand *Source1, Operand *Source2)
    : Inst(Cfg, Inst::Arithmetic, 2, Dest), Op(Op) {
  addSource(Source1);
  addSource(Source2);
}

bool InstArithmetic::isCommutative() const {
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

InstAssign::InstAssign(IceCfg *Cfg, Variable *Dest, Operand *Source)
    : Inst(Cfg, Inst::Assign, 1, Dest) {
  addSource(Source);
}

// If TargetTrue==TargetFalse, we turn it into an unconditional
// branch.  This ensures that, along with the 'switch' instruction
// semantics, there is at most one edge from one node to another.
InstBr::InstBr(IceCfg *Cfg, Operand *Source, CfgNode *TargetTrue,
               CfgNode *TargetFalse)
    : Inst(Cfg, Inst::Br, 1, NULL), TargetFalse(TargetFalse),
      TargetTrue(TargetTrue) {
  if (TargetTrue == TargetFalse) {
    TargetTrue = NULL; // turn into unconditional version
  } else {
    addSource(Source);
  }
}

InstBr::InstBr(IceCfg *Cfg, CfgNode *Target)
    : Inst(Cfg, Inst::Br, 0, NULL), TargetFalse(Target), TargetTrue(NULL) {}

NodeList InstBr::getTerminatorEdges() const {
  NodeList OutEdges;
  OutEdges.push_back(TargetFalse);
  if (TargetTrue)
    OutEdges.push_back(TargetTrue);
  return OutEdges;
}

InstCast::InstCast(IceCfg *Cfg, OpKind CastKind, Variable *Dest,
                   Operand *Source)
    : Inst(Cfg, Inst::Cast, 1, Dest), CastKind(CastKind) {
  addSource(Source);
}

InstFcmp::InstFcmp(IceCfg *Cfg, FCond Condition, Variable *Dest,
                   Operand *Source1, Operand *Source2)
    : Inst(Cfg, Inst::Fcmp, 2, Dest), Condition(Condition) {
  addSource(Source1);
  addSource(Source2);
}

InstIcmp::InstIcmp(IceCfg *Cfg, ICond Condition, Variable *Dest,
                   Operand *Source1, Operand *Source2)
    : Inst(Cfg, Inst::Icmp, 2, Dest), Condition(Condition) {
  addSource(Source1);
  addSource(Source2);
}

InstLoad::InstLoad(IceCfg *Cfg, Variable *Dest, Operand *SourceAddr)
    : Inst(Cfg, Inst::Load, 1, Dest) {
  addSource(SourceAddr);
}

InstPhi::InstPhi(IceCfg *Cfg, uint32_t MaxSrcs, Variable *Dest)
    : Inst(Cfg, Phi, MaxSrcs, Dest) {
  Labels = Cfg->allocateArrayOf<CfgNode *>(MaxSrcs);
}

// TODO: A Switch instruction (and maybe others) can add duplicate
// edges.  We may want to de-dup Phis and validate consistency (i.e.,
// the source operands are the same for duplicate edges), though it
// seems the current lowering code is OK with this situation.
void InstPhi::addArgument(Operand *Source, CfgNode *Label) {
  Labels[getSrcSize()] = Label;
  addSource(Source);
}

// Find the source operand corresponding to the incoming edge for the
// given node.  TODO: This uses a linear-time search, which could be
// improved if it becomes a problem.
Operand *InstPhi::getOperandForTarget(CfgNode *Target) const {
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
void InstPhi::livenessPhiOperand(llvm::BitVector &Live, CfgNode *Target,
                                 Liveness *Liveness) {
  if (isDeleted() || Dead)
    return;
  for (uint32_t I = 0; I < getSrcSize(); ++I) {
    if (Labels[I] == Target) {
      if (Variable *Var = llvm::dyn_cast<Variable>(getSrc(I))) {
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
Inst *InstPhi::lower(IceCfg *Cfg, CfgNode *Node) {
  Variable *Dest = getDest();
  assert(Dest);
  IceString PhiName = Dest->getName() + "_phi";
  Variable *NewSrc = Cfg->makeVariable(Dest->getType(), Node, PhiName);
  this->Dest = NewSrc;
  InstAssign *NewInst = InstAssign::create(Cfg, Dest, NewSrc);
  // Set Dest and NewSrc to have affinity with each other, as a hint
  // for register allocation.
  Dest->setPreferredRegister(NewSrc, false);
  NewSrc->setPreferredRegister(Dest, false);
  Dest->replaceDefinition(NewInst, Node);
  return NewInst;
}

InstRet::InstRet(IceCfg *Cfg, Operand *Source)
    : Inst(Cfg, Ret, Source ? 1 : 0, NULL) {
  if (Source)
    addSource(Source);
}

InstSelect::InstSelect(IceCfg *Cfg, Variable *Dest, Operand *Condition,
                       Operand *SourceTrue, Operand *SourceFalse)
    : Inst(Cfg, Inst::Select, 3, Dest) {
  assert(Condition->getType() == IceType_i1);
  addSource(Condition);
  addSource(SourceTrue);
  addSource(SourceFalse);
}

InstStore::InstStore(IceCfg *Cfg, Operand *Data, Operand *Addr)
    : Inst(Cfg, Inst::Store, 2, NULL) {
  addSource(Data);
  addSource(Addr);
}

InstSwitch::InstSwitch(IceCfg *Cfg, uint32_t NumCases, Operand *Source,
                       CfgNode *LabelDefault)
    : Inst(Cfg, Inst::Switch, 1, NULL), LabelDefault(LabelDefault),
      NumCases(NumCases) {
  addSource(Source);
  Values = Cfg->allocateArrayOf<uint64_t>(NumCases);
  Labels = Cfg->allocateArrayOf<CfgNode *>(NumCases);
  // Initialize in case buggy code doesn't set all entries
  for (uint32_t I = 0; I < NumCases; ++I) {
    Values[I] = 0;
    Labels[I] = NULL;
  }
}

void InstSwitch::addBranch(uint32_t CaseIndex, uint64_t Value, CfgNode *Label) {
  assert(CaseIndex < NumCases);
  Values[CaseIndex] = Value;
  Labels[CaseIndex] = Label;
}

NodeList InstSwitch::getTerminatorEdges() const {
  NodeList OutEdges;
  OutEdges.push_back(LabelDefault);
  for (uint32_t I = 0; I < NumCases; ++I) {
    OutEdges.push_back(Labels[I]);
  }
  return OutEdges;
}

InstUnreachable::InstUnreachable(IceCfg *Cfg)
    : Inst(Cfg, Inst::Unreachable, 0, NULL) {}

InstFakeDef::InstFakeDef(IceCfg *Cfg, Variable *Dest, Variable *Src)
    : Inst(Cfg, Inst::FakeDef, Src ? 1 : 0, Dest) {
  assert(Dest);
  if (Src)
    addSource(Src);
}

InstFakeUse::InstFakeUse(IceCfg *Cfg, Variable *Src)
    : Inst(Cfg, Inst::FakeUse, 1, NULL) {
  assert(Src);
  addSource(Src);
}

InstFakeKill::InstFakeKill(IceCfg *Cfg, const VarList &KilledRegs,
                           const Inst *Linked)
    : Inst(Cfg, Inst::FakeKill, KilledRegs.size(), NULL), Linked(Linked) {
  for (VarList::const_iterator I = KilledRegs.begin(), E = KilledRegs.end();
       I != E; ++I) {
    Variable *Var = *I;
    addSource(Var);
  }
}

// ======================== Dump routines ======================== //

void Inst::dumpDecorated(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  if (!Cfg->getContext()->isVerbose(IceV_Deleted) &&
      (isDeleted() || isRedundantAssign()))
    return;
  if (Cfg->getContext()->isVerbose(IceV_InstNumbers)) {
    const static size_t BufLen = 30;
    char buf[BufLen];
    int32_t Number = getNumber();
    if (Number < 0)
      snprintf(buf, BufLen, "[XXX]");
    else
      snprintf(buf, BufLen, "[%3d]", Number);
    Str << buf;
  }
  Str << "  ";
  if (isDeleted())
    Str << "  //";
  dump(Cfg);
  dumpExtras(Cfg);
  Str << "\n";
}

void Inst::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->getStrEmit();
  Str << "??? ";
  dump(Cfg);
  Str << "\n";
  assert(0 && "emit() called on a non-lowered instruction");
  // Ideally, Cfg->setError() would be called, but Cfg is a const
  // pointer and setError() changes its contents.
  // Cfg->setError("emit() called on a non-lowered instruction");
}

void Inst::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  dumpDest(Cfg);
  Str << " =~ ";
  dumpSources(Cfg);
}

void Inst::dumpExtras(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  bool First = true;
  // Print "LIVEEND={a,b,c}" for all source operands whose live ranges
  // are known to end at this instruction.
  if (Cfg->getContext()->isVerbose(IceV_Liveness)) {
    uint32_t VarIndex = 0;
    for (uint32_t I = 0; I < getSrcSize(); ++I) {
      Operand *Src = getSrc(I);
      uint32_t NumVars = Src->getNumVars();
      for (uint32_t J = 0; J < NumVars; ++J, ++VarIndex) {
        const Variable *Var = Src->getVar(J);
        if (isLastUse(Var)) {
          if (First)
            Str << " // LIVEEND={";
          else
            Str << ",";
          Var->dump(Cfg);
          First = false;
        }
      }
    }
    if (!First)
      Str << "}";
  }
}

void Inst::dumpSources(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  for (uint32_t I = 0; I < getSrcSize(); ++I) {
    if (I > 0)
      Str << ", ";
    getSrc(I)->dump(Cfg);
  }
}

void Inst::emitSources(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->getStrEmit();
  for (uint32_t I = 0; I < getSrcSize(); ++I) {
    if (I > 0)
      Str << ", ";
    getSrc(I)->emit(Cfg, Option);
  }
}

void Inst::dumpDest(const IceCfg *Cfg) const {
  if (getDest())
    getDest()->dump(Cfg);
}

void InstAlloca::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  dumpDest(Cfg);
  Str << " = alloca i8, i32 ";
  getSrc(0)->dump(Cfg);
  Str << ", align " << Align;
}

void InstArithmetic::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  dumpDest(Cfg);
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
  dumpSources(Cfg);
}

void InstAssign::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  dumpDest(Cfg);
  Str << " = " << getDest()->getType() << " ";
  dumpSources(Cfg);
}

void InstBr::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  dumpDest(Cfg);
  Str << "br ";
  if (!isUnconditional()) {
    Str << "i1 ";
    getSrc(0)->dump(Cfg);
    Str << ", label %" << getTargetTrue()->getName() << ", ";
  }
  Str << "label %" << getTargetFalse()->getName();
}

void InstCall::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  if (getDest()) {
    dumpDest(Cfg);
    Str << " = ";
  }
  if (Tail)
    Str << "tail ";
  Str << "call ";
  if (getDest())
    Str << getDest()->getType();
  else
    Str << "void";
  Str << " ";
  getCallTarget()->dump(Cfg);
  Str << "(";
  for (uint32_t I = 0; I < getNumArgs(); ++I) {
    if (I > 0)
      Str << ", ";
    Str << getArg(I)->getType() << " ";
    getArg(I)->dump(Cfg);
  }
  Str << ")";
}

void InstCast::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  dumpDest(Cfg);
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
  case Bitcast:
    Str << "bitcast";
    break;
  }
  Str << " " << getSrc(0)->getType() << " ";
  dumpSources(Cfg);
  Str << " to " << getDest()->getType();
  if (getCastKind() == Inttoptr)
    Str << "*";
}

void InstIcmp::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  dumpDest(Cfg);
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
  dumpSources(Cfg);
}

void InstFcmp::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  dumpDest(Cfg);
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
  dumpSources(Cfg);
}

void InstLoad::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  dumpDest(Cfg);
  IceType Type = getDest()->getType();
  Str << " = load " << Type << "* ";
  dumpSources(Cfg);
  switch (Type) {
  case IceType_f32:
    Str << ", align 4";
    break;
  case IceType_f64:
    Str << ", align 8";
    break;
  default:
    Str << ", align 1";
    break;
  }
}

void InstStore::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  IceType Type = getData()->getType();
  Str << "store " << Type << " ";
  getData()->dump(Cfg);
  Str << ", " << Type << "* ";
  getAddr()->dump(Cfg);
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

void InstSwitch::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  IceType Type = getSrc(0)->getType();
  Str << "switch " << Type << " ";
  getSrc(0)->dump(Cfg);
  Str << ", label %" << getLabelDefault()->getName() << " [\n";
  for (uint32_t I = 0; I < getNumCases(); ++I) {
    Str << "    " << Type << " " << getValue(I) << ", label %"
        << getLabel(I)->getName() << "\n";
  }
  Str << "  ]";
}

void InstPhi::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  dumpDest(Cfg);
  Str << " = phi " << getDest()->getType() << " ";
  for (uint32_t I = 0; I < getSrcSize(); ++I) {
    if (I > 0)
      Str << ", ";
    Str << "[ ";
    getSrc(I)->dump(Cfg);
    Str << ", %" << Labels[I]->getName() << " ]";
  }
}

void InstRet::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  IceType Type = getSrcSize() == 0 ? IceType_void : getSrc(0)->getType();
  Str << "ret " << Type;
  if (getSrcSize()) {
    Str << " ";
    dumpSources(Cfg);
  }
}

void InstSelect::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  dumpDest(Cfg);
  Operand *Condition = getCondition();
  Operand *TrueOp = getTrueOperand();
  Operand *FalseOp = getFalseOperand();
  Str << " = select " << Condition->getType() << " ";
  Condition->dump(Cfg);
  Str << ", " << TrueOp->getType() << " ";
  TrueOp->dump(Cfg);
  Str << ", " << FalseOp->getType() << " ";
  FalseOp->dump(Cfg);
}

void InstUnreachable::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  Str << "unreachable";
}

void InstFakeDef::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->getStrEmit();
  Str << "\t# ";
  getDest()->emit(Cfg, Option);
  Str << " = def.pseudo ";
  emitSources(Cfg, Option);
  Str << "\n";
}

void InstFakeDef::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  dumpDest(Cfg);
  Str << " = def.pseudo ";
  dumpSources(Cfg);
}

void InstFakeUse::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->getStrEmit();
  Str << "\t# ";
  Str << "use.pseudo ";
  emitSources(Cfg, Option);
  Str << "\n";
}

void InstFakeUse::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  Str << "use.pseudo ";
  dumpSources(Cfg);
}

void InstFakeKill::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->getStrEmit();
  Str << "\t# ";
  if (Linked->isDeleted())
    Str << "// ";
  Str << "kill.pseudo ";
  emitSources(Cfg, Option);
  Str << "\n";
}

void InstFakeKill::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  if (Linked->isDeleted())
    Str << "// ";
  Str << "kill.pseudo ";
  dumpSources(Cfg);
}

void InstTarget::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  Str << "[TARGET] ";
  Inst::dump(Cfg);
}

void InstTarget::dumpExtras(const IceCfg *Cfg) const { Inst::dumpExtras(Cfg); }

} // end of namespace Ice
