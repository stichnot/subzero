/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInst.h"
#include "IceLiveness.h"
#include "IceOperand.h"
#include "IceTargetLowering.h"
#include "IceInstX8632.h"

IceCfgNode::IceCfgNode(IceCfg *Cfg, uint32_t LabelNumber, IceString Name)
    : Cfg(Cfg), Number(LabelNumber), Name(Name), ArePhiLoadsPlaced(false),
      ArePhiStoresPlaced(false), HasReturn(false) {}

void IceCfgNode::appendInst(IceInst *Inst) {
  if (IceInstPhi *Phi = llvm::dyn_cast<IceInstPhi>(Inst)) {
    if (ArePhiLoadsPlaced || ArePhiStoresPlaced) {
      Cfg->setError("Phi instruction added after phi lowering");
      return;
    }
    if (!Insts.empty()) {
      Cfg->setError("Phi instruction added to the middle of a block");
      return;
    }
    Phis.push_back(Phi);
  } else {
    Insts.push_back(Inst);
  }
  Inst->updateVars(this);
}

IceString IceCfgNode::getName(void) const {
  if (Name != "")
    return Name;
  char buf[30];
  sprintf(buf, "__%u", getIndex());
  return buf;
}

IceString IceCfgNode::getAsmName(void) const {
  // These are internal labels, no need to apply Cfg name mangling.
  return ".L" + Cfg->getName() + "$" + getName();
}

void IceCfgNode::renumberInstructions(void) {
  for (IcePhiList::const_iterator I = Phis.begin(), E = Phis.end(); I != E;
       ++I) {
    (*I)->renumber(Cfg);
  }
  IceInstList::const_iterator I = Insts.begin(), E = Insts.end();
  while (I != E) {
    IceInst *Inst = *I++;
    Inst->renumber(Cfg);
  }
}

// Inserts this node between the From and To nodes.  Just updates the
// in-edge/out-edge structure without doing anything to the CFG
// linearization.  TODO: Edge splitting is now broken because we are
// missing a terminator instruction in one of the nodes.
void IceCfgNode::splitEdge(IceCfgNode *From, IceCfgNode *To) {
  // Find the out-edge position.
  IceNodeList::iterator Iout = From->OutEdges.begin();
  IceNodeList::iterator Eout = From->OutEdges.end();
  for (; Iout != Eout; ++Iout) {
    if (*Iout == To)
      break;
  }
  assert(Iout != Eout);

  // Find the in-edge position.
  IceNodeList::iterator Iin = To->InEdges.begin();
  IceNodeList::iterator Ein = To->InEdges.end();
  for (; Iin != Ein; ++Iin) {
    if (*Iin == From)
      break;
  }
  assert(Iin != Ein);

  // Update all edges.
  this->OutEdges.push_back(*Iout);
  *Iout = this;
  this->InEdges.push_back(*Iin);
  *Iin = this;
}

void IceCfgNode::registerEdges(void) {
  OutEdges = (*Insts.rbegin())->getTerminatorEdges();
  for (IceNodeList::const_iterator I = OutEdges.begin(), E = OutEdges.end();
       I != E; ++I) {
    IceCfgNode *Node = *I;
    Node->InEdges.push_back(this);
  }
}

// TODO: Make IceInstList into its own class that wraps the
// std::list<IceInst*> and provides its own STL iterators.  Provide
// normal iterators, plus an iterator that automatically skips deleted
// instructions.  This way, target lowering can get full lookahead
// instead of just one instruction lookahead.
// IceTargetLowering::lower() would take Cur and End iterators instead
// of Inst and Next IceInsts, for the purpose of pre-lowering peephole
// optimizations such as compare/branch fusing or load/binop fusing.
static IceInst *getNextInst(IceInstList::iterator I,
                            const IceInstList::iterator &E) {
  while (I != E && (*I)->isDeleted())
    ++I;
  if (I == E)
    return NULL;
  return *I;
}

void IceCfgNode::placePhiLoads(void) {
  if (ArePhiLoadsPlaced) {
    Cfg->setError("placePhiLoads() called more than once");
    return;
  }
  ArePhiLoadsPlaced = true;
  // Create the phi version of each destination and add it to the phi
  // instruction's Srcs list.
  IceInstList NewPhiLoads;
  for (IcePhiList::iterator I = Phis.begin(), E = Phis.end(); I != E; ++I) {
    // Change "a=phi(...)" to "a_phi=phi(...); a=a_phi".
    IceInstPhi *Phi = *I;
    IceInst *NewPhi = Phi->lower(Cfg, this);
    NewPhiLoads.push_back(NewPhi);
  }
  if (NewPhiLoads.empty())
    return;
  // TODO: Insert each phi right before its destination's first use in
  // the block, but before any instruction with an implicit use of the
  // destination such as a call or integer divide instruction when the
  // function uses setjmp().
  insertInsts(Insts.begin(), NewPhiLoads);
}

void IceCfgNode::placePhiStores(void) {
  if (ArePhiStoresPlaced) {
    Cfg->setError("placePhiStores() called more than once");
    return;
  }
  if (!ArePhiLoadsPlaced) {
    Cfg->setError("placePhiStores() must be called after placePhiLoads()");
    return;
  }
  ArePhiStoresPlaced = true;

  IceInstList NewPhiStores;
  for (IceNodeList::const_iterator I1 = OutEdges.begin(), E1 = OutEdges.end();
       I1 != E1; ++I1) {
    IceCfgNode *Target = *I1;
    assert(Target);
    if (Target == NULL)
      continue;
    for (IcePhiList::const_iterator I2 = Target->Phis.begin(),
                                    E2 = Target->Phis.end();
         I2 != E2; ++I2) {
      IceOperand *Operand = (*I2)->getOperandForTarget(this);
      assert(Operand);
      if (Operand == NULL)
        continue;
      IceVariable *Dest = (*I2)->getDest();
      assert(Dest);
      IceInstAssign *NewInst = IceInstAssign::create(Cfg, Dest, Operand);
      if (IceVariable *Src = llvm::dyn_cast<IceVariable>(Operand)) {
        Dest->setPreferredRegister(Src, false);
        Src->setPreferredRegister(Dest, false);
      }
      NewPhiStores.push_back(NewInst);
    }
  }
  if (NewPhiStores.empty())
    return;
  IceInstList::iterator InsertionPoint = Insts.end();
  if (InsertionPoint != Insts.begin()) {
    --InsertionPoint;
    if (llvm::isa<IceInstBr>(*InsertionPoint)) {
      if (InsertionPoint != Insts.begin()) {
        --InsertionPoint;
        if (!llvm::isa<IceInstIcmp>(*InsertionPoint) &&
            !llvm::isa<IceInstFcmp>(*InsertionPoint)) {
          ++InsertionPoint;
        }
      }
    }
  }
  insertInsts(InsertionPoint, NewPhiStores);
}

void IceCfgNode::deletePhis(void) {
  for (IcePhiList::iterator I = Phis.begin(), E = Phis.end(); I != E; ++I) {
    (*I)->setDeleted();
  }
}

void IceCfgNode::doAddressOpt(void) {
  IceTargetLowering *Target = Cfg->getTarget();
  Target->setCurrentNode(this);
  IceInstList::iterator I = Insts.begin(), E = Insts.end();
  while (I != E) {
    IceInst *Inst = *I++;
    if (Inst->isDeleted())
      continue;
    IceInstList NewInsts = Target->doAddressOpt(Inst);
    if (!NewInsts.empty()) {
      insertInsts(I, NewInsts);
      Inst->setDeleted();
    }
  }
}

void IceCfgNode::genCode(void) {
  IceTargetLowering *Target = Cfg->getTarget();
  Target->setCurrentNode(this);
  // Defer the Phi instructions.
  IceInstList::iterator I = Insts.begin(), E = Insts.end();
  while (I != E) {
    IceInst *Inst = *I++;
    IceInst *Next = getNextInst(I, E);
    if (Inst->isDeleted())
      continue;
    if (llvm::isa<IceInstRet>(Inst))
      setHasReturn();
    bool DeleteNextInst = false;
    IceInstList NewInsts = Target->lower(Inst, Next, DeleteNextInst);
    insertInsts(I, NewInsts);
    Inst->setDeleted();
    if (DeleteNextInst)
      Next->setDeleted();
  }
}

void IceCfgNode::insertInsts(IceInstList::iterator Location,
                             const IceInstList &NewInsts) {
  Insts.insert(Location, NewInsts.begin(), NewInsts.end());
  IceInstList::const_iterator I = NewInsts.begin(), E = NewInsts.end();
  while (I != E) {
    IceInst *Inst = *I++;
    if (Inst->isDeleted())
      continue;
    Inst->updateVars(this);
  }
}

// Returns true if the incoming liveness changed from before, false if
// it stayed the same.
bool IceCfgNode::liveness(IceLivenessMode Mode, IceLiveness *Liveness) {
  unsigned NumVars;
  if (Mode == IceLiveness_LREndLightweight)
    NumVars = Cfg->getNumVariables();
  else
    NumVars = Liveness->getLocalSize(this);
  std::vector<int> &LiveBegin = Liveness->getLiveBegin(this);
  std::vector<int> &LiveEnd = Liveness->getLiveEnd(this);
  llvm::BitVector Live(NumVars);
  if (Mode != IceLiveness_LREndLightweight) {
    LiveBegin.assign(NumVars, 0);
    LiveEnd.assign(NumVars, 0);
    // Initialize Live to be the union of all successors' LiveIn.
    for (IceNodeList::const_iterator I = OutEdges.begin(), E = OutEdges.end();
         I != E; ++I) {
      IceCfgNode *Succ = *I;
      Live |= Liveness->getLiveIn(Succ);
      // Mark corresponding argument of phis in successor as live.
      for (IcePhiList::const_iterator I1 = Succ->Phis.begin(),
                                      E1 = Succ->Phis.end();
           I1 != E1; ++I1) {
        (*I1)->livenessPhiOperand(Live, this, Liveness);
      }
    }
    // TODO: Can LiveOut contain local variables???
    Liveness->getLiveOut(this) = Live;
  }

  // Process instructions in reverse order
  for (IceInstList::const_reverse_iterator I = Insts.rbegin(), E = Insts.rend();
       I != E; ++I) {
    (*I)->liveness(Mode, (*I)->getNumber(), Live, Liveness, this);
  }
  // Process phis in any order
  int FirstPhiNumber = -1;
  for (IcePhiList::const_iterator I = Phis.begin(), E = Phis.end(); I != E;
       ++I) {
    if (FirstPhiNumber < 0)
      FirstPhiNumber = (*I)->getNumber();
    (*I)->liveness(Mode, FirstPhiNumber, Live, Liveness, this);
  }

  // When using the sparse representation, after traversing the
  // instructions in the block, the Live bitvector should only contain
  // set bits for global variables upon block entry.  We validate this
  // by shrinking the Live vector and then testing it against the
  // pre-shrunk version.  (The shrinking is required, but the
  // validation is not.)
  if (Mode != IceLiveness_LREndLightweight) {
    llvm::BitVector LiveOrig = Live;
    Live.resize(Liveness->getGlobalSize());
    // Non-global arguments in the entry node are allowed to be live on
    // entry.
    bool IsEntry = (Cfg->getEntryNode() == this);
    assert(IsEntry || Live == LiveOrig);
    if (!(IsEntry || Live == LiveOrig)) {
      IceOstream &Str = Cfg->Str;
      Str.setCurrentNode(NULL);
      Str << "LiveOrig-Live =";
      for (unsigned i = Live.size(); i < LiveOrig.size(); ++i) {
        if (LiveOrig.test(i))
          Str << " " << Liveness->getVariable(i, this);
      }
      Str << "\n";
    }
  }

  bool Changed = false;
  if (Mode != IceLiveness_LREndLightweight) {
    llvm::BitVector &LiveIn = Liveness->getLiveIn(this);
    // Add in current LiveIn
    Live |= LiveIn;
    // Check result, set LiveIn=Live
    Changed = (Live != LiveIn);
    if (Changed)
      LiveIn = Live;
  }
  return Changed;
}

// Now that basic liveness is complete, remove dead instructions that
// were tentatively marked as dead, and compute actual live ranges.
// It is assumed that within a single basic block, a live range begins
// at most once and ends at most once.  This is certainly true for
// pure SSA form.  It is also true once phis are lowered, since each
// assignment to the phi-based temporary is in a different basic
// block, and there is a single read that ends the live in the basic
// block that contained the actual phi instruction.
void IceCfgNode::livenessPostprocess(IceLivenessMode Mode,
                                     IceLiveness *Liveness) {
  int FirstInstNum = -1;
  int LastInstNum = -1;
  // Process phis in any order.  Process only Dest operands.
  for (IcePhiList::const_iterator I = Phis.begin(), E = Phis.end(); I != E;
       ++I) {
    IceInstPhi *Inst = *I;
    Inst->deleteIfDead();
    if (Inst->isDeleted())
      continue;
    if (FirstInstNum < 0)
      FirstInstNum = Inst->getNumber();
    assert(Inst->getNumber() > LastInstNum);
    LastInstNum = Inst->getNumber();
  }
  // Process instructions
  for (IceInstList::const_iterator I = Insts.begin(), E = Insts.end(); I != E;
       ++I) {
    IceInst *Inst = *I;
    Inst->deleteIfDead();
    if (Inst->isDeleted())
      continue;
    if (FirstInstNum < 0)
      FirstInstNum = Inst->getNumber();
    // TODO: What to do if the block contains phi instructions but no
    // regular instructions?  What live range should the phi
    // destinations get in this block?
    assert(Inst->getNumber() > LastInstNum);
    LastInstNum = Inst->getNumber();
    // Create fake live ranges for a Kill instruction, but only if the
    // linked instruction is still alive.
    if (Mode == IceLiveness_RangesFull) {
      if (IceInstFakeKill *Kill = llvm::dyn_cast<IceInstFakeKill>(Inst)) {
        if (!Kill->getLinked()->isDeleted()) {
          unsigned NumSrcs = Inst->getSrcSize();
          for (unsigned i = 0; i < NumSrcs; ++i) {
            IceVariable *Var = llvm::cast<IceVariable>(Inst->getSrc(i));
            int InstNumber = Inst->getNumber();
            Liveness->addLiveRange(Var, InstNumber, InstNumber, 1);
          }
        }
      }
    }
  }
  if (Mode != IceLiveness_RangesFull)
    return;

  unsigned NumVars = Liveness->getLocalSize(this);
  unsigned NumGlobals = Liveness->getGlobalSize();
  llvm::BitVector &LiveIn = Liveness->getLiveIn(this);
  llvm::BitVector &LiveOut = Liveness->getLiveOut(this);
  std::vector<int> &LiveBegin = Liveness->getLiveBegin(this);
  std::vector<int> &LiveEnd = Liveness->getLiveEnd(this);
  for (unsigned i = 0; i < NumVars; ++i) {
    // Deal with the case where the variable is both live-in and
    // live-out, but LiveEnd comes before LiveBegin.  In this case, we
    // need to add two segments to the live range because there is a
    // hole in the middle.  This would typically happen as a result of
    // phi lowering in the presence of loopback edges.
    bool IsGlobal = (i < NumGlobals);
    if (IsGlobal && LiveIn[i] && LiveOut[i] && LiveBegin[i] > LiveEnd[i]) {
      IceVariable *Var = Liveness->getVariable(i, this);
      Liveness->addLiveRange(Var, FirstInstNum, LiveEnd[i], 1);
      Liveness->addLiveRange(Var, LiveBegin[i], LastInstNum + 1, 1);
      continue;
    }
    int Begin = (IsGlobal && LiveIn[i]) ? FirstInstNum : LiveBegin[i];
    int End = (IsGlobal && LiveOut[i]) ? LastInstNum + 1 : LiveEnd[i];
    if (Begin <= 0 && End <= 0)
      continue;
    if (Begin <= FirstInstNum)
      Begin = FirstInstNum;
    if (End <= 0)
      End = LastInstNum + 1;
    IceVariable *Var = Liveness->getVariable(i, this);
    Liveness->addLiveRange(Var, Begin, End, 1);
  }
}

// ======================== Dump routines ======================== //

void IceCfgNode::emit(IceOstream &Str, uint32_t Option) const {
  Str.setCurrentNode(this);
  if (Cfg->getEntryNode() == this) {
    Str << Cfg->mangleName(Cfg->getName()) << ":\n";
  }
  Str << getAsmName() << ":\n";
  // TODO: emit() should blow up in some way if any live phi
  // instructions remain.
  for (IcePhiList::const_iterator I = Phis.begin(), E = Phis.end(); I != E;
       ++I) {
    IceInstPhi *Inst = *I;
    if (Inst->isDeleted())
      continue;
    Inst->emit(Str, Option);
  }
  for (IceInstList::const_iterator I = Insts.begin(), E = Insts.end(); I != E;
       ++I) {
    IceInst *Inst = *I;
    if (Inst->isDeleted())
      continue;
    if (Inst->isRedundantAssign())
      continue;
    (*I)->emit(Str, Option);
  }
}

void IceCfgNode::dump(IceOstream &Str) const {
  Str.setCurrentNode(this);
  IceLiveness *Liveness = Str.Cfg->getLiveness();
  if (Str.isVerbose(IceV_Instructions)) {
    Str << getName() << ":\n";
  }
  if (Str.isVerbose(IceV_Preds) && !InEdges.empty()) {
    Str << "    // preds = ";
    for (IceNodeList::const_iterator I = InEdges.begin(), E = InEdges.end();
         I != E; ++I) {
      if (I != InEdges.begin())
        Str << ", ";
      Str << "%" << (*I)->getName();
    }
    Str << "\n";
  }
  llvm::BitVector LiveIn;
  if (Liveness)
    LiveIn = Liveness->getLiveIn(this);
  if (Str.isVerbose(IceV_Liveness) && !LiveIn.empty()) {
    Str << "    // LiveIn:";
    for (unsigned i = 0; i < LiveIn.size(); ++i) {
      if (LiveIn[i]) {
        Str << " %" << Liveness->getVariable(i, this)->getName();
      }
    }
    Str << "\n";
  }
  if (Str.isVerbose(IceV_Instructions)) {
    for (IcePhiList::const_iterator I = Phis.begin(), E = Phis.end(); I != E;
         ++I) {
      Str << (*I);
    }
    IceInstList::const_iterator I = Insts.begin(), E = Insts.end();
    while (I != E) {
      IceInst *Inst = *I++;
      Str << Inst;
    }
  }
  llvm::BitVector LiveOut;
  if (Liveness)
    LiveOut = Liveness->getLiveOut(this);
  if (Str.isVerbose(IceV_Liveness) && !LiveOut.empty()) {
    Str << "    // LiveOut:";
    for (unsigned i = 0; i < LiveOut.size(); ++i) {
      if (LiveOut[i]) {
        Str << " %" << Liveness->getVariable(i, this)->getName();
      }
    }
    Str << "\n";
  }
  if (Str.isVerbose(IceV_Succs)) {
    Str << "    // succs = ";
    for (IceNodeList::const_iterator I = OutEdges.begin(), E = OutEdges.end();
         I != E; ++I) {
      if (I != OutEdges.begin())
        Str << ", ";
      Str << "%" << (*I)->getName();
    }
    Str << "\n";
  }
}
