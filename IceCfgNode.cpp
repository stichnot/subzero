/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInst.h"
#include "IceOperand.h"
#include "IceRegManager.h"
#include "IceTargetLowering.h"

IceCfgNode::IceCfgNode(IceCfg *Cfg, uint32_t LabelIndex) :
  Cfg(Cfg), NameIndex(LabelIndex),
  ArePhiLoadsPlaced(false), ArePhiStoresPlaced(false),
  RegManager(NULL) {
  Cfg->addNode(this, LabelIndex);
}

void IceCfgNode::appendInst(IceInst *Inst) {
  Insts.push_back(Inst);
  Inst->updateVars(this);
}

void IceCfgNode::addPhi(IceInstPhi *Inst) {
  assert(!ArePhiLoadsPlaced);
  assert(!ArePhiStoresPlaced);
  assert(Insts.empty());
  Phis.push_back(Inst);
  Inst->updateVars(this);
}

void IceCfgNode::addFallthrough(uint32_t TargetLabel) {
  OutEdges.insert(OutEdges.begin(), TargetLabel);
}

void IceCfgNode::addNonFallthrough(uint32_t TargetLabel) {
  OutEdges.push_back(TargetLabel);
}

void IceCfgNode::renumberInstructions(void) {
  for (IcePhiList::const_iterator I = Phis.begin(), E = Phis.end();
       I != E; ++I) {
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
// linearization.
void IceCfgNode::splitEdge(IceCfgNode *From, IceCfgNode *To) {
  // Find the out-edge position.
  IceEdgeList::iterator Iout = From->OutEdges.begin();
  IceEdgeList::iterator Eout = From->OutEdges.end();
  for (; Iout != Eout; ++Iout) {
    if (*Iout == To->getIndex())
      break;
  }
  assert(Iout != Eout);

  // Find the in-edge position.
  IceEdgeList::iterator Iin = To->InEdges.begin();
  IceEdgeList::iterator Ein = To->InEdges.end();
  for (; Iin != Ein; ++Iin) {
    if (*Iin == From->getIndex())
      break;
  }
  assert(Iin != Ein);

  // Update all edges.
  this->addFallthrough(*Iout);
  *Iout = this->getIndex();
  this->InEdges.push_back(*Iin);
  *Iin = this->getIndex();
}

void IceCfgNode::registerInEdges(void) {
  for (IceEdgeList::const_iterator I = OutEdges.begin(), E = OutEdges.end();
       I != E; ++I) {
    IceCfgNode *Node = Cfg->getNode(*I);
    Node->InEdges.push_back(NameIndex);
  }
}

void IceCfgNode::findAddressOpt(void) {
  // No need to check the Phi instructions.
  IceInstList::const_iterator I = Insts.begin(), E = Insts.end();
  while (I != E) {
    IceInst *Inst = *I++;
    if (Inst->isDeleted())
      continue;
    Inst->findAddressOpt(Cfg, this);
  }
}

void IceCfgNode::markLastUses(void) {
  // No need to check the Phi instructions.
  IceInstList::const_iterator I = Insts.begin(), E = Insts.end();
  while (I != E) {
    IceInst *Inst = *I++;
    if (Inst->isDeleted())
      continue;
    Inst->markLastUses(Cfg);
  }
}

static IceInst *getNextInst(IceInstList::iterator I,
                            const IceInstList::iterator &E) {
  while (I != E && (*I)->isDeleted())
    ++I;
  if (I == E)
    return NULL;
  return *I;
}

void IceCfgNode::placePhiLoads(void) {
  assert(!ArePhiLoadsPlaced);
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
  assert(ArePhiLoadsPlaced);
  assert(!ArePhiStoresPlaced);
  ArePhiStoresPlaced = true;

  IceInstList NewPhiStores;
  for (IceEdgeList::const_iterator I1 = OutEdges.begin(), E1 = OutEdges.end();
       I1 != E1; ++I1) {
    IceCfgNode *Target = Cfg->getNode(*I1);
    assert(Target);
    if (Target == NULL)
      continue;
    for (IcePhiList::const_iterator I2 = Target->Phis.begin(),
           E2 = Target->Phis.end(); I2 != E2; ++I2) {
      IceOperand *Operand = (*I2)->getOperandForTarget(NameIndex);
      assert(Operand);
      if (Operand == NULL)
        continue;
      IceInstAssign *NewInst =
        new IceInstAssign(Cfg, (*I2)->getDest(0), Operand);
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

void IceCfgNode::genCode(void) {
  const unsigned NumScratchReg = 3; // eax, ecx, edx
  IceTargetLowering *Target = Cfg->getTarget();
  // TODO: Disabling extended basic block handling for now.  IIRC,
  // there was a problem when adding compensations.  Revisit when
  // compensations are fixed.
  if (false && InEdges.size() == 1) {
    IceCfgNode *Pred = Cfg->getNode(InEdges[0]);
    assert(Pred);
    // TODO: Use the final RegManager in Pred.
    RegManager = new IceRegManager(*Pred->RegManager);
  } else {
    RegManager = new IceRegManager(Cfg, this, NumScratchReg);
  }
  Target->setRegManager(RegManager);
  // Defer the Phi instructions.
  IceInstList::iterator I = Insts.begin(), E = Insts.end();
  while (I != E) {
    IceInst *Inst = *I++;
    IceInst *Next = getNextInst(I, E);
    if (Inst->isDeleted())
      continue;
    bool DeleteNextInst = false;
    IceInstList NewInsts = Target->lower(Inst, Next, DeleteNextInst);
    insertInsts(I, NewInsts);
    Inst->setDeleted();
    if (DeleteNextInst)
      Next->setDeleted();
  }
}

void IceCfgNode::multiblockRegAlloc(void) {
  // Candidates are the operands for which the operand is in the set
  // of FirstLoad operands, and the operand is in the Available set of
  // at least one predecessor RegManager.
  //
  // Construct the candidate list by checking each predecessor for
  // each FirstLoad operand.
  //
  // Vote on physical register assignment.  For each virtual register,
  // keep a tally of the number of votes for each physical register.
  // A physical register assignment for a candidate in a predecessor
  // block gets one vote.  Same for a successor block.  There is also
  // a vote for preferences in the current block arising from
  // instruction constraints.
  //
  // Add a compensation entry for each in-edge as necessary.  A
  // compensation is either a load of an unavailable operand, or a
  // register move.  A compensation is unnecessary when it is a
  // register move with both the source and dest known to map to the
  // same physical register.
  //
  // After this runs on all blocks, physical register assignment is
  // complete.  multiblockCompensation() then passes over the blocks
  // adds the compensation code, and removes the candidate assignment
  // instructions.

  // Consider each predecessor and update the
  // MultiblockCandidateWeight values.
  for (IceEdgeList::const_iterator I = InEdges.begin(), E = InEdges.end();
       I != E; ++I) {
    IceCfgNode *Pred = Cfg->getNode(*I);
    assert(Pred);
    // TODO: use the final RegManager in Pred.
    RegManager->updateCandidates(Pred->RegManager);
  }

  // Consider each predecessor and update the PhysicalRegisterVotes
  // values.
  for (IceEdgeList::const_iterator I = InEdges.begin(), E = InEdges.end();
       I != E; ++I) {
    IceCfgNode *Pred = Cfg->getNode(*I);
    assert(Pred);
    RegManager->updateVotes(Pred->RegManager);
  }

  // Consider each successor and update the PhysicalRegisterVotes
  // values.
  for (IceEdgeList::const_iterator I = OutEdges.begin(), E = OutEdges.end();
       I != E; ++I) {
    IceCfgNode *Succ = Cfg->getNode(*I);
    assert(Succ);
    // TODO: Implement voting by successors.
    //RegManager->updateVotes(Pred->RegManager);
  }

  // Tally up the votes and assign physical registers.
  RegManager->makeAssignments();

  // Add compensation entries as necessary.
  for (IceEdgeList::const_iterator I = InEdges.begin(), E = InEdges.end();
       I != E; ++I) {
    IceCfgNode *Pred = Cfg->getNode(*I);
    assert(Pred);
    // TODO: use the final RegManager in Pred.
    Compensations.push_back(RegManager->addCompensations(Pred->RegManager,
                                                         Cfg->getTarget()));
  }
}

void IceCfgNode::multiblockCompensation(void) {
  //return; // TODO: This is broken so disable it for now.
  std::vector<IceInstList>::const_iterator I1 = Compensations.begin();
  for (IceEdgeList::const_iterator I = InEdges.begin(), E = InEdges.end();
       I != E; ++I, ++I1) {
    IceInstList Insts = *I1;
    if (Insts.empty())
      continue;
    IceCfgNode *Pred = Cfg->getNode(*I);
    assert(Pred);
    IceCfgNode *Split = Cfg->splitEdge(*I, NameIndex);
    Insts.push_back(new IceInstBr(Cfg, Split, NameIndex));
    Split->InEdges.push_back(*I);
    Split->insertInsts(Split->Insts.end(), Insts);
  }
  RegManager->deleteHoists();
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
// it stayed the same.  IsFirst is set the first time the node is
// processed, and is a signal to initialize LiveIn.
bool IceCfgNode::liveness(bool IsFirst) {
  unsigned NumVars = Cfg->getNumVariables();
  if (IsFirst) {
    LiveIn.clear();
    LiveIn.resize(NumVars);
    LiveOut.clear();
    LiveOut.resize(NumVars);
    LiveBegin.resize(NumVars);
    LiveEnd.resize(NumVars);
  }
  LiveBegin.clear();
  LiveEnd.clear();
  llvm::BitVector Live(NumVars);
  // Initialize Live to be the union of all successors' LiveIn.
  for (IceEdgeList::const_iterator I = OutEdges.begin(), E = OutEdges.end();
       I != E; ++I) {
    IceCfgNode *Succ = Cfg->getNode(*I);
    Live |= Succ->LiveIn;
    // Mark corresponding argument of phis in successor as live.
    for (IcePhiList::const_iterator I1 = Succ->Phis.begin(),
           E1 = Succ->Phis.end(); I1 != E1; ++I1) {
      (*I1)->livenessPhiOperand(Live, getIndex());
    }
  }
  LiveOut = Live;
  // Process instructions in reverse order
  for (IceInstList::const_reverse_iterator I = Insts.rbegin(), E = Insts.rend();
       I != E; ++I) {
    (*I)->liveness(Live, LiveBegin, LiveEnd);
  }
  // Process phis in any order
  for (IcePhiList::const_iterator I = Phis.begin(), E = Phis.end();
       I != E; ++I) {
    (*I)->liveness(Live, LiveBegin, LiveEnd);
  }
  // Add in current LiveIn
  Live |= LiveIn;
  // Check result, set LiveIn=Live
  bool Changed = (Live != LiveIn);
  if (Changed)
    LiveIn = Live;
  return Changed;
}

void IceCfgNode::livenessPostprocess(void) {
  unsigned NumVars = Cfg->getNumVariables();
  int FirstInstNum = -1;
  int LastInstNum = -1;
  // Process phis in any order.  Process only Dest operands.
  for (IcePhiList::const_iterator I = Phis.begin(), E = Phis.end();
       I != E; ++I) {
    IceInstPhi *Inst = *I;
    assert(Inst->getNumber() > LastInstNum);
    LastInstNum = Inst->getNumber();
    Inst->deleteIfDead();
    if (Inst->isDeleted())
      continue;
  }
  // Process instructions
  for (IceInstList::const_iterator I = Insts.begin(), E = Insts.end();
       I != E; ++I) {
    IceInst *Inst = *I;
    if (FirstInstNum < 0)
      FirstInstNum = Inst->getNumber();
    // TODO: What to do if the block contains phi instructions but no
    // regular instructions?  What live range should the phi
    // destinations get in this block?
    assert(Inst->getNumber() > LastInstNum);
    LastInstNum = Inst->getNumber();
    Inst->deleteIfDead();
    if (Inst->isDeleted())
      continue;
  }
  for (unsigned i = 0; i < NumVars; ++i) {
    int Begin = LiveIn[i] ? FirstInstNum : LiveBegin[i];
    int End = LiveOut[i] ? LastInstNum + 1 : LiveEnd[i];
    if (Begin <= 0 && End <= 0)
      continue;
    if (Begin <= FirstInstNum)
      Begin = FirstInstNum;
    if (End <= 0)
      End = LastInstNum + 1;
    IceVariable *Var = Cfg->getVariable(i);
    Var->addLiveRange(Begin, End);
  }
}

// ======================== Dump routines ======================== //

void IceCfgNode::dump(IceOstream &Str) const {
  IceString Name = Str.Cfg->labelName(getIndex());
  if (Str.isVerbose(IceV_Instructions)) {
    Str << Name << ":\n";
  }
  if (Str.isVerbose(IceV_Preds)) {
    Str << "    // preds = ";
    for (IceEdgeList::const_iterator I = InEdges.begin(), E = InEdges.end();
         I != E; ++I) {
      if (I != InEdges.begin())
        Str << ", ";
      Str << "%" << Str.Cfg->labelName(*I);
    }
    Str << "\n";
  }
  if (Str.isVerbose(IceV_Liveness)) {
    Str << "    // LiveIn:";
    for (unsigned i = 0; i < LiveIn.size(); ++i) {
      if (LiveIn[i]) {
        Str << " %" << Str.Cfg->variableName(i);
      }
    }
    Str << "\n";
  }
  if (Str.isVerbose(IceV_RegManager)) {
    if (RegManager) {
      Str << "    // FirstLoads={";
      RegManager->dumpFirstLoads(Str);
      Str << "}\n";
    }
  }
  if (Str.isVerbose(IceV_Instructions)) {
    for (IcePhiList::const_iterator I = Phis.begin(), E = Phis.end();
         I != E; ++I) {
      Str << (*I);
    }
    IceInstList::const_iterator I = Insts.begin(), E = Insts.end();
    while (I != E) {
      IceInst *Inst = *I++;
      Str << Inst;
    }
  }
  if (Str.isVerbose(IceV_RegManager)) {
    if (RegManager) {
      Str << "    // AVAIL:";
      RegManager->dump(Str);
      Str << "\n";
    }
  }
  if (Str.isVerbose(IceV_Succs)) {
    Str << "    // succs = ";
    for (IceEdgeList::const_iterator I = OutEdges.begin(), E = OutEdges.end();
         I != E; ++I) {
      if (I != OutEdges.begin())
        Str << ", ";
      Str << "%" << Str.Cfg->labelName(*I);
    }
    Str << "\n";
  }
}
