//===- subzero/src/IceCfgNode.cpp - Basic block (node) implementation -----===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the CfgNode class, including the complexities
// of instruction insertion and in-edge calculation.
//
//===----------------------------------------------------------------------===//

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInst.h"
#include "IceLiveness.h"
#include "IceOperand.h"
#include "IceTargetLowering.h"

namespace Ice {

CfgNode::CfgNode(Cfg *Func, SizeT LabelNumber, IceString Name)
    : Func(Func), Number(LabelNumber), Name(Name), HasReturn(false) {}

// Returns the name the node was created with.  If no name was given,
// it synthesizes a (hopefully) unique name.
IceString CfgNode::getName() const {
  if (!Name.empty())
    return Name;
  char buf[30];
  snprintf(buf, llvm::array_lengthof(buf), "__%u", getIndex());
  return buf;
}

// Adds an instruction to either the Phi list or the regular
// instruction list.  Validates that all Phis are added before all
// regular instructions.
void CfgNode::appendInst(Inst *Inst) {
  if (InstPhi *Phi = llvm::dyn_cast<InstPhi>(Inst)) {
    if (!Insts.empty()) {
      Func->setError("Phi instruction added to the middle of a block");
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
void CfgNode::renumberInstructions() {
  for (PhiList::const_iterator I = Phis.begin(), E = Phis.end(); I != E; ++I) {
    (*I)->renumber(Func);
  }
  InstList::const_iterator I = Insts.begin(), E = Insts.end();
  while (I != E) {
    Inst *Inst = *I++;
    Inst->renumber(Func);
  }
}

// Inserts this node between the From and To nodes.  Just updates the
// in-edge/out-edge structure without doing anything to the CFG
// linearization, as this is handled by the calling function
// Cfg::splitEdge().
void CfgNode::splitEdge(CfgNode *From, CfgNode *To) {
  // Find the out-edge position.
  NodeList::iterator Iout = From->OutEdges.begin();
  NodeList::iterator Eout = From->OutEdges.end();
  for (; Iout != Eout; ++Iout) {
    if (*Iout == To)
      break;
  }
  assert(Iout != Eout);

  // Find the in-edge position.
  NodeList::iterator Iin = To->InEdges.begin();
  NodeList::iterator Ein = To->InEdges.end();
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
// constructed, the computePredecessors() pass finalizes it by
// creating the InEdges list.
void CfgNode::computePredecessors() {
  OutEdges = (*Insts.rbegin())->getTerminatorEdges();
  for (NodeList::const_iterator I = OutEdges.begin(), E = OutEdges.end();
       I != E; ++I) {
    CfgNode *Node = *I;
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
void CfgNode::placePhiLoads() {
  for (PhiList::iterator I = Phis.begin(), E = Phis.end(); I != E; ++I) {
    Inst *Inst = (*I)->lower(Func, this);
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
void CfgNode::placePhiStores() {
  // Find the insertion point.  TODO: After branch/compare fusing is
  // implemented, try not to insert Phi stores between the compare and
  // conditional branch instructions, otherwise the branch/compare
  // pattern matching may fail.  However, the branch/compare sequence
  // will have to be broken if the compare result is read (by the
  // assignment) before it is written (by the compare).
  InstList::iterator InsertionPoint = Insts.end();
  // Every block must end in a terminator instruction.
  assert(InsertionPoint != Insts.begin());
  --InsertionPoint;
  // Confirm that InsertionPoint is a terminator instruction.  Calling
  // getTerminatorEdges() on a non-terminator instruction will cause
  // an llvm_unreachable().
  (void)(*InsertionPoint)->getTerminatorEdges();
  // If the current insertion point is at a conditional branch
  // instruction, and the previous instruction is a compare
  // instruction, then we move the insertion point before the compare
  // instruction so as not to interfere with compare/branch fusing.
  if (InstBr *Branch = llvm::dyn_cast<InstBr>(*InsertionPoint)) {
    if (!Branch->isUnconditional()) {
      if (InsertionPoint != Insts.begin()) {
        --InsertionPoint;
        if (!llvm::isa<InstIcmp>(*InsertionPoint) &&
            !llvm::isa<InstFcmp>(*InsertionPoint)) {
          ++InsertionPoint;
        }
      }
    }
  }

  // Consider every out-edge.
  for (NodeList::const_iterator I1 = OutEdges.begin(), E1 = OutEdges.end();
       I1 != E1; ++I1) {
    CfgNode *Target = *I1;
    // Consider every Phi instruction at the out-edge.
    for (PhiList::const_iterator I2 = Target->Phis.begin(),
                                 E2 = Target->Phis.end();
         I2 != E2; ++I2) {
      Operand *Operand = (*I2)->getOperandForTarget(this);
      assert(Operand);
      Variable *Dest = (*I2)->getDest();
      assert(Dest);
      InstAssign *NewInst = InstAssign::create(Func, Dest, Operand);
      // If Src is a variable, set the Src and Dest variables to
      // prefer each other for register allocation.
      if (Variable *Src = llvm::dyn_cast<Variable>(Operand)) {
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
void CfgNode::deletePhis() {
  for (PhiList::iterator I = Phis.begin(), E = Phis.end(); I != E; ++I) {
    (*I)->setDeleted();
  }
}

// Does address mode optimization.  Pass each instruction to the
// TargetLowering object.  If it returns a new instruction
// (representing the optimized address mode), then insert the new
// instruction and delete the old.
void CfgNode::doAddressOpt() {
  TargetLowering *Target = Func->getTarget();
  LoweringContext &Context = Target->getContext();
  Context.init(this);
  while (!Context.atEnd()) {
    Target->doAddressOpt();
  }
}

// Drives the target lowering.  Passes the current instruction and the
// next non-deleted instruction for target lowering.
void CfgNode::genCode() {
  TargetLowering *Target = Func->getTarget();
  LoweringContext &Context = Target->getContext();
  // Lower only the regular instructions.  Defer the Phi instructions.
  Context.init(this);
  while (!Context.atEnd()) {
    InstList::iterator Orig = Context.getCur();
    if (llvm::isa<InstRet>(*Orig))
      setHasReturn();
    Target->lower();
    // Ensure target lowering actually moved the cursor.
    assert(Context.getCur() != Orig);
  }
}

void CfgNode::livenessLightweight() {
  SizeT NumVars = Func->getNumVariables();
  llvm::BitVector Live(NumVars);
  // Process regular instructions in reverse order.
  for (InstList::const_reverse_iterator I = Insts.rbegin(), E = Insts.rend();
       I != E; ++I) {
    if ((*I)->isDeleted())
      continue;
    (*I)->livenessLightweight(Live);
  }
  for (PhiList::const_iterator I = Phis.begin(), E = Phis.end(); I != E; ++I) {
    if ((*I)->isDeleted())
      continue;
    (*I)->livenessLightweight(Live);
  }
}

// Performs liveness analysis on the block.  Returns true if the
// incoming liveness changed from before, false if it stayed the same.
// (If it changes, the node's predecessors need to be processed
// again.)
bool CfgNode::liveness(Liveness *Liveness) {
  SizeT NumVars = Liveness->getNumVarsInNode(this);
  llvm::BitVector Live(NumVars);
  // Mark the beginning and ending of each variable's live range
  // with the sentinel instruction number 0.
  std::vector<InstNumberT> &LiveBegin = Liveness->getLiveBegin(this);
  std::vector<InstNumberT> &LiveEnd = Liveness->getLiveEnd(this);
  InstNumberT Sentinel = Inst::NumberSentinel;
  LiveBegin.assign(NumVars, Sentinel);
  LiveEnd.assign(NumVars, Sentinel);
  // Initialize Live to be the union of all successors' LiveIn.
  for (NodeList::const_iterator I = OutEdges.begin(), E = OutEdges.end();
       I != E; ++I) {
    CfgNode *Succ = *I;
    Live |= Liveness->getLiveIn(Succ);
    // Mark corresponding argument of phis in successor as live.
    for (PhiList::const_iterator I1 = Succ->Phis.begin(), E1 = Succ->Phis.end();
         I1 != E1; ++I1) {
      (*I1)->livenessPhiOperand(Live, this, Liveness);
    }
  }
  Liveness->getLiveOut(this) = Live;

  // Process regular instructions in reverse order.
  for (InstList::const_reverse_iterator I = Insts.rbegin(), E = Insts.rend();
       I != E; ++I) {
    if ((*I)->isDeleted())
      continue;
    (*I)->liveness((*I)->getNumber(), Live, Liveness, this);
  }
  // Process phis in forward order so that we can override the
  // instruction number to be that of the earliest phi instruction in
  // the block.
  InstNumberT FirstPhiNumber = Inst::NumberSentinel;
  for (PhiList::const_iterator I = Phis.begin(), E = Phis.end(); I != E; ++I) {
    if ((*I)->isDeleted())
      continue;
    if (FirstPhiNumber == Inst::NumberSentinel)
      FirstPhiNumber = (*I)->getNumber();
    (*I)->liveness(FirstPhiNumber, Live, Liveness, this);
  }

  // When using the sparse representation, after traversing the
  // instructions in the block, the Live bitvector should only contain
  // set bits for global variables upon block entry.  We validate this
  // by shrinking the Live vector and then testing it against the
  // pre-shrunk version.  (The shrinking is required, but the
  // validation is not.)
  llvm::BitVector LiveOrig = Live;
  Live.resize(Liveness->getNumGlobalVars());
  // Non-global arguments in the entry node are allowed to be live on
  // entry.
  bool IsEntry = (Func->getEntryNode() == this);
  if (!(IsEntry || Live == LiveOrig)) {
    // This is a fatal liveness consistency error.  Print some
    // diagnostics and abort.
    Ostream &Str = Func->getContext()->getStrDump();
    Func->setCurrentNode(NULL);
    Str << "LiveOrig-Live =";
    for (SizeT i = Live.size(); i < LiveOrig.size(); ++i) {
      if (LiveOrig.test(i)) {
        Str << " ";
        Liveness->getVariable(i, this)->dump(Func);
      }
    }
    Str << "\n";
    llvm_unreachable("Fatal inconsistency in liveness analysis");
  }

  bool Changed = false;
  llvm::BitVector &LiveIn = Liveness->getLiveIn(this);
  // Add in current LiveIn
  Live |= LiveIn;
  // Check result, set LiveIn=Live
  Changed = (Live != LiveIn);
  if (Changed)
    LiveIn = Live;
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
void CfgNode::livenessPostprocess(LivenessMode Mode, Liveness *Liveness) {
  InstNumberT FirstInstNum = Inst::NumberSentinel;
  InstNumberT LastInstNum = Inst::NumberSentinel;
  // Process phis in any order.  Process only Dest operands.
  for (PhiList::const_iterator I = Phis.begin(), E = Phis.end(); I != E; ++I) {
    InstPhi *Inst = *I;
    Inst->deleteIfDead();
    if (Inst->isDeleted())
      continue;
    if (FirstInstNum == Inst::NumberSentinel)
      FirstInstNum = Inst->getNumber();
    assert(Inst->getNumber() > LastInstNum);
    LastInstNum = Inst->getNumber();
  }
  // Process instructions
  for (InstList::const_iterator I = Insts.begin(), E = Insts.end(); I != E;
       ++I) {
    Inst *Inst = *I;
    Inst->deleteIfDead();
    if (Inst->isDeleted())
      continue;
    if (FirstInstNum == Inst::NumberSentinel)
      FirstInstNum = Inst->getNumber();
    assert(Inst->getNumber() > LastInstNum);
    LastInstNum = Inst->getNumber();
    // Create fake live ranges for a Kill instruction, but only if the
    // linked instruction is still alive.
    if (Mode == Liveness_Intervals) {
      if (InstFakeKill *Kill = llvm::dyn_cast<InstFakeKill>(Inst)) {
        if (!Kill->getLinked()->isDeleted()) {
          SizeT NumSrcs = Inst->getSrcSize();
          for (SizeT i = 0; i < NumSrcs; ++i) {
            Variable *Var = llvm::cast<Variable>(Inst->getSrc(i));
            InstNumberT InstNumber = Inst->getNumber();
            Liveness->addLiveRange(Var, InstNumber, InstNumber, 1);
          }
        }
      }
    }
  }
  if (Mode != Liveness_Intervals)
    return;

  SizeT NumVars = Liveness->getNumVarsInNode(this);
  SizeT NumGlobals = Liveness->getNumGlobalVars();
  llvm::BitVector &LiveIn = Liveness->getLiveIn(this);
  llvm::BitVector &LiveOut = Liveness->getLiveOut(this);
  std::vector<InstNumberT> &LiveBegin = Liveness->getLiveBegin(this);
  std::vector<InstNumberT> &LiveEnd = Liveness->getLiveEnd(this);
  for (SizeT i = 0; i < NumVars; ++i) {
    // Deal with the case where the variable is both live-in and
    // live-out, but LiveEnd comes before LiveBegin.  In this case, we
    // need to add two segments to the live range because there is a
    // hole in the middle.  This would typically happen as a result of
    // phi lowering in the presence of loopback edges.
    bool IsGlobal = (i < NumGlobals);
    if (IsGlobal && LiveIn[i] && LiveOut[i] && LiveBegin[i] > LiveEnd[i]) {
      Variable *Var = Liveness->getVariable(i, this);
      Liveness->addLiveRange(Var, FirstInstNum, LiveEnd[i], 1);
      Liveness->addLiveRange(Var, LiveBegin[i], LastInstNum + 1, 1);
      continue;
    }
    InstNumberT Begin = (IsGlobal && LiveIn[i]) ? FirstInstNum : LiveBegin[i];
    InstNumberT End = (IsGlobal && LiveOut[i]) ? LastInstNum + 1 : LiveEnd[i];
    if (Begin == Inst::NumberSentinel && End == Inst::NumberSentinel)
      continue;
    if (Begin <= FirstInstNum)
      Begin = FirstInstNum;
    if (End == Inst::NumberSentinel)
      End = LastInstNum + 1;
    Variable *Var = Liveness->getVariable(i, this);
    Liveness->addLiveRange(Var, Begin, End, 1);
  }
}

// ======================== Dump routines ======================== //

void CfgNode::emit(Cfg *Func) const {
  Func->setCurrentNode(this);
  Ostream &Str = Func->getContext()->getStrEmit();
  if (Func->getEntryNode() == this) {
    Str << Func->getContext()->mangleName(Func->getFunctionName()) << ":\n";
  }
  Str << getAsmName() << ":\n";
  for (PhiList::const_iterator I = Phis.begin(), E = Phis.end(); I != E; ++I) {
    InstPhi *Inst = *I;
    if (Inst->isDeleted())
      continue;
    // Emitting a Phi instruction should cause an error.
    Inst->emit(Func);
  }
  for (InstList::const_iterator I = Insts.begin(), E = Insts.end(); I != E;
       ++I) {
    Inst *Inst = *I;
    if (Inst->isDeleted())
      continue;
    // Here we detect redundant assignments like "mov eax, eax" and
    // suppress them.
    if (Inst->isRedundantAssign())
      continue;
    (*I)->emit(Func);
  }
}

void CfgNode::dump(Cfg *Func) const {
  Func->setCurrentNode(this);
  Ostream &Str = Func->getContext()->getStrDump();
  Liveness *Liveness = Func->getLiveness();
  if (Func->getContext()->isVerbose(IceV_Instructions)) {
    Str << getName() << ":\n";
  }
  // Dump list of predecessor nodes.
  if (Func->getContext()->isVerbose(IceV_Preds) && !InEdges.empty()) {
    Str << "    // preds = ";
    for (NodeList::const_iterator I = InEdges.begin(), E = InEdges.end();
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
  if (Func->getContext()->isVerbose(IceV_Liveness) && !LiveIn.empty()) {
    Str << "    // LiveIn:";
    for (SizeT i = 0; i < LiveIn.size(); ++i) {
      if (LiveIn[i]) {
        Str << " %" << Liveness->getVariable(i, this)->getName();
      }
    }
    Str << "\n";
  }
  // Dump each instruction.
  if (Func->getContext()->isVerbose(IceV_Instructions)) {
    for (PhiList::const_iterator I = Phis.begin(), E = Phis.end(); I != E;
         ++I) {
      const Inst *Inst = *I;
      Inst->dumpDecorated(Func);
    }
    InstList::const_iterator I = Insts.begin(), E = Insts.end();
    while (I != E) {
      Inst *Inst = *I++;
      Inst->dumpDecorated(Func);
    }
  }
  // Dump the live-out variables.
  llvm::BitVector LiveOut;
  if (Liveness)
    LiveOut = Liveness->getLiveOut(this);
  if (Func->getContext()->isVerbose(IceV_Liveness) && !LiveOut.empty()) {
    Str << "    // LiveOut:";
    for (SizeT i = 0; i < LiveOut.size(); ++i) {
      if (LiveOut[i]) {
        Str << " %" << Liveness->getVariable(i, this)->getName();
      }
    }
    Str << "\n";
  }
  // Dump list of successor nodes.
  if (Func->getContext()->isVerbose(IceV_Succs)) {
    Str << "    // succs = ";
    for (NodeList::const_iterator I = OutEdges.begin(), E = OutEdges.end();
         I != E; ++I) {
      if (I != OutEdges.begin())
        Str << ", ";
      Str << "%" << (*I)->getName();
    }
    Str << "\n";
  }
}

} // end of namespace Ice
