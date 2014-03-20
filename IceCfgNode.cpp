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
    : Cfg(Cfg), Number(LabelNumber), Name(Name), HasReturn(false) {}

// Returns the name the node was created with.  If no name was given,
// it synthesizes a (hopefully) unique name.
IceString IceCfgNode::getName() const {
  if (Name != "")
    return Name;
  char buf[30];
  sprintf(buf, "__%u", getIndex());
  return buf;
}

// Adds an instruction to either the Phi list or the regular
// instruction list.  Validates that all Phis are added before all
// regular instructions.
void IceCfgNode::appendInst(IceInst *Inst) {
  if (IceInstPhi *Phi = llvm::dyn_cast<IceInstPhi>(Inst)) {
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

// Renumbers the non-deleted instructions in the node.  This needs to
// be done in preparation for live range analysis.  The instruction
// numbers in a block must be monotonically increasing.  The range of
// instruction numbers in a block, from lowest to highest, must not
// overlap with the range of any other block.
void IceCfgNode::renumberInstructions() {
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
// linearization, as this is handled by the calling function
// IceCfg::splitEdge().
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

// When a node is created, the OutEdges are immediately knows, but the
// InEdges have to be built up incrementally.  After the CFG has been
// constructed, the registerEdges() pass finalizes it by creating the
// InEdges list.
void IceCfgNode::registerEdges() {
  OutEdges = (*Insts.rbegin())->getTerminatorEdges();
  for (IceNodeList::const_iterator I = OutEdges.begin(), E = OutEdges.end();
       I != E; ++I) {
    IceCfgNode *Node = *I;
    Node->InEdges.push_back(this);
  }
}

// This does part 1 of Phi lowering, by creating a new dest variable
// for each Phi instruction, replacing the Phi instruction's dest with
// that variable, and adding an explicit assignment of the old dest to
// the new dest.  For example,
//   a=phi(...)
// changes to
//   "a_phi=phi(...); a=a_phi".
//
// This is in preparation for part 2 which deletes the Phi
// instructions and appends assignment instructions to predecessor
// blocks.  Note that this transformation preserves SSA form.
void IceCfgNode::placePhiLoads() {
  for (IcePhiList::iterator I = Phis.begin(), E = Phis.end(); I != E; ++I) {
    IceInst *Inst = (*I)->lower(Cfg, this);
    Insts.insert(Insts.begin(), Inst);
    Inst->updateVars(this);
  }
}

// This does part 2 of Phi lowering.  For each Phi instruction at each
// out-edge, create a corresponding assignment instruction, and add
// all the assignments near the end of this block.  They need to be
// added before any branch instruction, and also if the block ends
// with a compare instruction followed by a branch instruction that we
// may want to fuse, it's better to insert the new assignments before
// the compare instruction.
//
// Note that this transformation takes the Phi dest variables out of
// SSA form, as there may be assignments to the dest variable in
// multiple blocks.
//
// TODO: Defer this pass until after register allocation, then split
// critical edges, add the assignments, and lower them.  This should
// reduce the amount of shuffling at the end of each block.
void IceCfgNode::placePhiStores() {
  // Find the insertion point.  TODO: This insertion-point logic is
  // fragile.  It's too closely linked to the branch/compare fusing
  // code in the target lowering.  And it's wrong if the source
  // operand of one of the new assignments is equal to the dest
  // operand of the compare instruction, in which case the compare
  // result is read (by the assignment) before it is written (by the
  // compare).  However, this problem should go away with the edge
  // splitting approach described above.
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

  // Consider every out-edge.
  for (IceNodeList::const_iterator I1 = OutEdges.begin(), E1 = OutEdges.end();
       I1 != E1; ++I1) {
    IceCfgNode *Target = *I1;
    // Consider every Phi instruction at the out-edge.
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
      // If Src is a variable, set the Src and Dest variables to
      // prefer each other for register allocation.
      if (IceVariable *Src = llvm::dyn_cast<IceVariable>(Operand)) {
        bool AllowOverlap = false;
        Dest->setPreferredRegister(Src, AllowOverlap);
        Src->setPreferredRegister(Dest, AllowOverlap);
      }
      Insts.insert(InsertionPoint, NewInst);
      NewInst->updateVars(this);
    }
  }
}

// Deletes the phi instructions after the loads and stores are placed.
void IceCfgNode::deletePhis() {
  for (IcePhiList::iterator I = Phis.begin(), E = Phis.end(); I != E; ++I) {
    (*I)->setDeleted();
  }
}

// Does address mode optimization.  Pass each instruction to the
// IceTargetLowering object.  If it returns a new instruction
// (representing the optimized address mode), then insert the new
// instruction and delete the old.
void IceCfgNode::doAddressOpt() {
  IceTargetLowering *Target = Cfg->getTarget();
  IceLoweringContext Context(this);
  while (Context.Cur != Context.End) {
    Target->doAddressOpt(Context);
  }
}

// Drives the target lowering.  Passes the current instruction and the
// next non-deleted instruction for target lowering.
void IceCfgNode::genCode() {
  IceTargetLowering *Target = Cfg->getTarget();
  // Lower only the regular instructions.  Defer the Phi instructions.
  IceLoweringContext Context(this);
  while (Context.Cur != Context.End) {
    IceInstList::iterator Orig = Context.Cur;
    (void)Orig; // used only in assert()
    if (llvm::isa<IceInstRet>(*Context.Cur))
      setHasReturn();
    Target->lower(Context);
    assert(Context.Cur != Orig);
  }
}

// Performs liveness analysis on the block.  Returns true if the
// incoming liveness changed from before, false if it stayed the same.
// (If it changes, the node's predecessors need to be processed
// again.)
bool IceCfgNode::liveness(IceLivenessMode Mode, IceLiveness *Liveness) {
  uint32_t NumVars;
  if (Mode == IceLiveness_LREndLightweight)
    NumVars = Cfg->getNumVariables();
  else
    NumVars = Liveness->getLocalSize(this);
  llvm::BitVector Live(NumVars);
  if (Mode != IceLiveness_LREndLightweight) {
    // Mark the beginning and ending of each variable's live range
    // with the sentinel instruction number 0.
    std::vector<int> &LiveBegin = Liveness->getLiveBegin(this);
    std::vector<int> &LiveEnd = Liveness->getLiveEnd(this);
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
    Liveness->getLiveOut(this) = Live;
  }

  // Process regular instructions in reverse order.
  for (IceInstList::const_reverse_iterator I = Insts.rbegin(), E = Insts.rend();
       I != E; ++I) {
    (*I)->liveness(Mode, (*I)->getNumber(), Live, Liveness, this);
  }
  // Process phis in forward order so that we can override the
  // instruction number to be that of the earliest phi instruction in
  // the block.
  int FirstPhiNumber = 0; // sentinel value
  for (IcePhiList::const_iterator I = Phis.begin(), E = Phis.end(); I != E;
       ++I) {
    if (FirstPhiNumber <= 0)
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
    // The following block helps debug why the previous assertion
    // failed.
    if (!(IsEntry || Live == LiveOrig)) {
      IceOstream &Str = Cfg->Str;
      Str.setCurrentNode(NULL);
      Str << "LiveOrig-Live =";
      for (uint32_t i = Live.size(); i < LiveOrig.size(); ++i) {
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
  int FirstInstNum = 0;
  int LastInstNum = 0;
  // Process phis in any order.  Process only Dest operands.
  for (IcePhiList::const_iterator I = Phis.begin(), E = Phis.end(); I != E;
       ++I) {
    IceInstPhi *Inst = *I;
    Inst->deleteIfDead();
    if (Inst->isDeleted())
      continue;
    if (FirstInstNum <= 0)
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
    if (FirstInstNum <= 0)
      FirstInstNum = Inst->getNumber();
    assert(Inst->getNumber() > LastInstNum);
    LastInstNum = Inst->getNumber();
    // Create fake live ranges for a Kill instruction, but only if the
    // linked instruction is still alive.
    if (Mode == IceLiveness_RangesFull) {
      if (IceInstFakeKill *Kill = llvm::dyn_cast<IceInstFakeKill>(Inst)) {
        if (!Kill->getLinked()->isDeleted()) {
          uint32_t NumSrcs = Inst->getSrcSize();
          for (uint32_t i = 0; i < NumSrcs; ++i) {
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

  uint32_t NumVars = Liveness->getLocalSize(this);
  uint32_t NumGlobals = Liveness->getGlobalSize();
  llvm::BitVector &LiveIn = Liveness->getLiveIn(this);
  llvm::BitVector &LiveOut = Liveness->getLiveOut(this);
  std::vector<int> &LiveBegin = Liveness->getLiveBegin(this);
  std::vector<int> &LiveEnd = Liveness->getLiveEnd(this);
  for (uint32_t i = 0; i < NumVars; ++i) {
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
  for (IcePhiList::const_iterator I = Phis.begin(), E = Phis.end(); I != E;
       ++I) {
    IceInstPhi *Inst = *I;
    if (Inst->isDeleted())
      continue;
    // Emitting a Phi instruction should cause an error.
    Inst->emit(Str, Option);
  }
  for (IceInstList::const_iterator I = Insts.begin(), E = Insts.end(); I != E;
       ++I) {
    IceInst *Inst = *I;
    if (Inst->isDeleted())
      continue;
    // Here we detect redundant assignments like "mov eax, eax" and
    // suppress them.
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
  // Dump list of predecessor nodes.
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
  // Dump the live-in variables.
  llvm::BitVector LiveIn;
  if (Liveness)
    LiveIn = Liveness->getLiveIn(this);
  if (Str.isVerbose(IceV_Liveness) && !LiveIn.empty()) {
    Str << "    // LiveIn:";
    for (uint32_t i = 0; i < LiveIn.size(); ++i) {
      if (LiveIn[i]) {
        Str << " %" << Liveness->getVariable(i, this)->getName();
      }
    }
    Str << "\n";
  }
  // Dump each instruction.
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
  // Dump the live-out variables.
  llvm::BitVector LiveOut;
  if (Liveness)
    LiveOut = Liveness->getLiveOut(this);
  if (Str.isVerbose(IceV_Liveness) && !LiveOut.empty()) {
    Str << "    // LiveOut:";
    for (uint32_t i = 0; i < LiveOut.size(); ++i) {
      if (LiveOut[i]) {
        Str << " %" << Liveness->getVariable(i, this)->getName();
      }
    }
    Str << "\n";
  }
  // Dump list of successor nodes.
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
