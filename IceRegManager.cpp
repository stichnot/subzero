/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include <stdio.h>
#include <string.h>

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInst.h"
#include "IceOperand.h"
#include "IceRegManager.h"
#include "IceTargetLowering.h"
#include "IceTypes.h"

IceRegManagerEntry::IceRegManagerEntry(IceVariable *Var, unsigned NumReg) : Var(Var) {
  FirstLoadInst = NULL;
  IsFirstLoadValid = false;
  MultiblockCandidateWeight = 0;
  PhysicalRegisterVotes = new int[NumReg];
  memset(PhysicalRegisterVotes, 0, NumReg * sizeof(*PhysicalRegisterVotes));
  TotalVotes = 0;
  PhysicalRegister = -1;
}

IceRegManagerEntry::IceRegManagerEntry(const IceRegManagerEntry &Other,
                                       unsigned NumReg) : Var(Other.Var) {
  Available = Other.Available;
  FirstLoadInst = Other.FirstLoadInst;
  IsFirstLoadValid = Other.IsFirstLoadValid;
  MultiblockCandidateWeight = Other.MultiblockCandidateWeight;
  PhysicalRegisterVotes = new int[NumReg];
  memcpy(PhysicalRegisterVotes, Other.PhysicalRegisterVotes, NumReg * sizeof(*PhysicalRegisterVotes));
  TotalVotes = Other.TotalVotes;
  PhysicalRegister = Other.PhysicalRegister;
}

// An Operand is loaded into this virtual register.  Its Available set
// is cleared and then set to contain the Operand.
void IceRegManagerEntry::load(IceInst *Inst) {
  Available.clear();
  IceOperand *Operand = NULL;
  if (Inst)
    Operand = Inst->getSrc(0);
  if (Operand)
    Available.push_back(Operand);
  if (!IsFirstLoadValid) {
    FirstLoadInst = Inst;
    IsFirstLoadValid = true;
  }
}

// This virtual register is stored into a Variable, and the Variable
// is added to the virtual register's Available set.  If the Variable
// is a physical register managed by this RegManager (e.g., specific
// register requirements for instructions like x86 idiv), then all
// Available sets must be killed.
void IceRegManagerEntry::store(IceInst *Inst) {
  // TODO: Kill all Available sets when necessary.
  IceVariable *Variable = Inst->getDest(0);
  assert(Variable);
  Available.push_back(Variable);
}

bool IceRegManagerEntry::contains(const IceOperand *Operand) const {
  if (getVar() == Operand)
    return true;
  for (IceOpList::const_iterator I = Available.begin(), E = Available.end();
       I != E; ++I) {
    if (*I == Operand)
      return true;
    // TODO: Make it work for IceConstant pointers.
  }
  return false;
}

IceRegManager::IceRegManager(IceCfg *Cfg, IceCfgNode *Node, unsigned NumReg) :
  NumReg(NumReg) {
  // TODO: Config flag to use physical registers directly.
  for (unsigned i = 0; i < NumReg; ++i) {
    char Buf[100];
    sprintf(Buf, "r%u_%u", i + 1, Node->getIndex());
    IceVariable *Reg = Cfg->getVariable(IceType_i32, Buf);
    Reg->setNoAutoDelete();
    Queue.push_back(new IceRegManagerEntry(Reg, NumReg));
  }
}

IceRegManager::IceRegManager(const IceRegManager &Other) :
  NumReg(Other.NumReg) {
  for (QueueType::const_iterator I = Other.Queue.begin(), E = Other.Queue.end();
       I != E; ++I) {
    Queue.push_back(new IceRegManagerEntry(**I, NumReg));
  }
}

// Prefer[0] is highest preference, Prefer[1] is second, etc.
IceVariable *IceRegManager::getRegister(IceType Type,
                                        const IceOpList &Prefer,
                                        const IceVarList &Avoid)
// TODO: "Avoid" is actually a set of virtual or physical registers.
// Wait - no it's not.  For an Arithmetic instruction, the load of the
// first operand should avoid using a register that contains the
// second operand.
  const {
  // Check each register in LRU order.  If it's in the Avoid list,
  // continue.  If a non-preferred candidate hasn't been seen, set it
  // to this register.  If its index into the Prefer list is better
  // than the current best, update the current best.
  // TODO: implement this policy
  IceRegManagerEntry *Good = NULL;
  IceRegManagerEntry *Best = NULL;
  for (QueueType::const_iterator I1 = Queue.begin(), E1 = Queue.end();
       I1 != E1; ++I1) {
    IceRegManagerEntry *Entry = *I1;
    bool AvoidEntry = false;
    for (IceVarList::const_iterator I2 = Avoid.begin(), E2 = Avoid.end();
         I2 != E2 ; ++I2) {
      if (Entry->contains(*I2)) {
        AvoidEntry = true;
        break;
      }
    }
    if (AvoidEntry)
      continue;
    if (Good == NULL)
      Good = Entry;
    for (IceOpList::const_iterator I2 = Prefer.begin(), E2 = Prefer.end();
         I2 != E2 ; ++I2) {
      if (Entry->contains(*I2)) {
        // TODO: Only set Best if it is better than the previous Best.
        Best = Entry;
      }
    }
  }
  if (Best == NULL)
    Best = Good;
  if (Best == NULL)
    Best = Queue.front();
  return Best->getVar();
}

bool IceRegManager::registerContains(const IceVariable *Reg,
                                     const IceOperand *Op) const {
  for (QueueType::const_iterator I = Queue.begin(), E = Queue.end();
       I != E; ++I) {
    if ((*I)->getVar() == Reg)
      return (*I)->contains(Op);
  }
  assert(false);
  return false;
}

void IceRegManager::notifyLoad(IceInst *Inst, bool IsAssign) {
  IceVariable *Reg = Inst->getDest(0);
  IceRegManagerEntry *Entry = NULL;
  for (QueueType::iterator I = Queue.begin(), E = Queue.end();
       I != E; ++I) {
    if ((*I)->getVar() == Reg) {
      Entry = *I;
      Queue.erase(I);
      break;
    }
  }
  assert(Entry);
  Queue.push_back(Entry);
  Entry->load(IsAssign ? Inst : NULL);
}

void IceRegManager::notifyStore(IceInst *Inst) {
  IceVariable *Reg = llvm::cast<IceVariable>(Inst->getSrc(0));
  IceVariable *Variable = Inst->getDest(0);
  assert(Variable);
  IceRegManagerEntry *Entry = NULL;
  for (QueueType::iterator I = Queue.begin(), E = Queue.end();
       I != E; ++I) {
    if ((*I)->getVar() == Reg) {
      Entry = *I;
      Queue.erase(I);
      break;
    }
  }
  assert(Entry);
  Queue.push_back(Entry);
  Entry->store(Inst);
  // TODO: See the TODO in IceRegManagerEntry::store().  For the
  // instruction "a=reg", "a" must be removed from all other Available
  // sets.  But is this actually possible with SSA form?
}

void IceRegManager::updateCandidates(const IceRegManager *Pred) {
  for (QueueType::const_iterator I = Queue.begin(), E = Queue.end();
       I != E; ++I) {
    IceInst *LoadInst = (*I)->getFirstLoadInst();
    if (LoadInst == NULL)
      continue;
    assert(LoadInst->getDest(0));
    assert(llvm::cast<IceVariable>(LoadInst->getDest(0)) == (*I)->getVar());
    IceOperand *Operand = LoadInst->getSrc(0);
    if (Pred->getEntryContaining(Operand))
      (*I)->updateCandidateWeight();
  }
}

void IceRegManager::updateVotes(const IceRegManager *Pred) {
  for (QueueType::const_iterator I = Queue.begin(), E = Queue.end();
       I != E; ++I) {
    if (!(*I)->isCandidate())
      continue;
    IceInst *LoadInst = (*I)->getFirstLoadInst();
    if (LoadInst == NULL)
      continue;
    assert(LoadInst->getDest(0));
    assert(llvm::cast<IceVariable>(LoadInst->getDest(0)) == (*I)->getVar());
    IceOperand *Operand = LoadInst->getSrc(0);
    IceRegManagerEntry *PredEntry = Pred->getEntryContaining(Operand);
    if (PredEntry == NULL)
      continue;
    int Reg = PredEntry->getVar()->getRegNum();
    if (Reg < 0)
      continue;
    assert((unsigned)Reg < NumReg);
    (*I)->voteFor(Reg);
  }
}

IceRegManagerEntry *IceRegManager::getEntryContaining(const IceOperand *Operand)
  const {
  for (QueueType::const_iterator I = Queue.begin(), E = Queue.end();
       I != E; ++I) {
    if ((*I)->contains(Operand))
      return *I;
  }
  return NULL;
}

void IceRegManager::makeAssignments(void) {
  // For each entry, ordered by TotalVotes, assign the register
  // corresponding to the largest PhysicalRegisterVotes entry that
  // hasn't already been assigned.
  bool Assigned[NumReg];
  memset(Assigned, 0, NumReg * sizeof(*Assigned));
  while (true) {
    IceRegManagerEntry *BestEntry = NULL;
    for (QueueType::const_iterator I = Queue.begin(), E = Queue.end();
         I != E; ++I) {
      if ((*I)->getVar()->getRegNum() >= 0)
        continue;
      if (BestEntry == NULL ||
          (*I)->getTotalVotes() > BestEntry->getTotalVotes())
        BestEntry = *I;
    }
    if (BestEntry == NULL)
      break;
    int BestReg = -1;
    for (unsigned i = 0; i < NumReg; i++) {
      if (Assigned[i])
        continue;
      if (BestReg < 0 || BestEntry->getVotes(i) > BestEntry->getVotes(BestReg))
        BestReg = i;
    }
    assert(BestReg >= 0);
    BestEntry->getVar()->setRegNum(BestReg);
    Assigned[BestReg] = true;
  }
}

IceInstList IceRegManager::addCompensations(const IceRegManager *Pred,
                                            IceTargetLowering *Target) {
  // Pass 1: Scan candidate list and generate list of assignments for
  // currently unavailable operands.  The assignments can be in any
  // order.  The first register corresponding to a non-candidate or an
  // unavailable operand becomes the cycle-breaking scratch register.
  IceInstList CompsUnavailable;
  IceVariable *Scratch = NULL;
  IceVarList Avoid;
  IceOpList Prefer;
  for (QueueType::const_iterator I = Queue.begin(), E = Queue.end();
       I != E; ++I) {
    IceVariable *Dest = (*I)->getVar();
    if (!(*I)->isCandidate()) {
      if (Scratch == NULL)
        Scratch = Dest;
      continue;
    }
    IceInst *LoadInst = (*I)->getFirstLoadInst();
    assert(LoadInst);
    assert(LoadInst->getDest(0) == Dest);
    IceOperand *Src = LoadInst->getSrc(0);
    Prefer.clear();
    Prefer.push_back(Src);
    IceVariable *Reg = Pred->getRegister(Dest->getType(), Prefer, Avoid);
    bool Available = Pred->registerContains(Reg, Src);
    if (Available)
      continue;
    IceInstTarget *NewInst = Target->makeAssign(Dest, Src);
    CompsUnavailable.push_back(NewInst);
  }

  // Pass 2: Scan candidate list and generate list of assignments for
  // available operands.  The assignment list must not violate any
  // dependencies.  For the assignment "reg1=reg2", insert before any
  // other assignment "reg2=xxx", and after any other assignment
  // "xxx=reg1".  If these conditions can't be satisfied due to a
  // dependency cycle, and there is a scratch register regX, then
  // insert the assignment "regX=reg2" at the early point and insert
  // the assignment "reg1=regX" at the late point.  If there is no
  // scratch register, then remove this candidate altogether (and
  // optionally remove related compensations from already-processed
  // edges).
  // TODO: this ordering is completely unimplemented so far.
  IceInstList CompsAvailable;
  for (QueueType::const_iterator I = Queue.begin(), E = Queue.end();
       I != E; ++I) {
    if (!(*I)->isCandidate())
      continue;
    IceVariable *Dest = (*I)->getVar();
    IceInst *LoadInst = (*I)->getFirstLoadInst();
    assert(LoadInst);
    assert(LoadInst->getDest(0) == Dest);
    IceOperand *Src = LoadInst->getSrc(0);
    Prefer.clear();
    Prefer.push_back(Src);
    IceVariable *Reg = Pred->getRegister(Dest->getType(), Prefer, Avoid);
    bool Available = Pred->registerContains(Reg, Src);
    if (!Available)
      continue;
    assert(Dest->getRegNum() >= 0);
    assert(Reg->getRegNum() >= 0);
    if (Dest->getRegNum() != Reg->getRegNum()) {
      IceInstTarget *NewInst = Target->makeAssign(Dest, Reg);
      // TODO: Find the correct insertion point instead of just
      // appending.
      assert(CompsAvailable.empty() &&
             "RegManager permutation not yet implemented");
      CompsAvailable.push_back(NewInst);
    }
  }

  IceInstList Compensations = CompsAvailable;
  Compensations.insert(Compensations.begin(),
                       CompsUnavailable.begin(), CompsUnavailable.end());
  return Compensations;
}

void IceRegManager::deleteHoists(void) {
  for (QueueType::const_iterator I = Queue.begin(), E = Queue.end();
       I != E; ++I) {
    if (!(*I)->isCandidate())
      continue;
    IceInst *LoadInst = (*I)->getFirstLoadInst();
    if (LoadInst == NULL)
      continue;
    LoadInst->setDeleted();
  }
}

// ======================== Dump routines ======================== //

void IceRegManager::dump(IceOstream &Str) const {
  for (QueueType::const_iterator I = Queue.begin(), E = Queue.end();
       I != E; ++I) {
    (*I)->dump(Str);
  }
}

void IceRegManager::dumpFirstLoads(IceOstream &Str) const {
  bool First = true;
  for (QueueType::const_iterator I = Queue.begin(), E = Queue.end();
       I != E; ++I) {
    IceInst *LoadInst = (*I)->getFirstLoadInst();
    IceOperand *Operand = LoadInst ? LoadInst->getSrc(0) : NULL;
    if (Operand == NULL)
      continue;
    IceVariable *Var = (*I)->getVar();
    int Weight = (*I)->getCandidateWeight();
    if (!First)
      Str << " ";
    Str << Var << ":" << Operand << ":" << Weight;
    First = false;
  }
}

void IceRegManagerEntry::dump(IceOstream &Str) const {
  Str << " " << getVar() << "={";
  for (IceOpList::const_iterator I = Available.begin(), E = Available.end();
       I != E; ++I) {
    if (I != Available.begin())
      Str << ",";
    Str << *I;
  }
  Str << "}";
}
