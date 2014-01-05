/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include <assert.h>
#include <stdio.h>

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceOperand.h"
#include "IceRegManager.h"
#include "IceTypes.h"

IceRegManagerEntry::IceRegManagerEntry(IceVariable *Var) :
  Var(Var), FirstLoad(NULL), IsFirstLoadValid(false) {}

IceRegManagerEntry::IceRegManagerEntry(const IceRegManagerEntry &Other) :
  Var(Other.Var), Available(Other.Available),
  FirstLoad(Other.FirstLoad), IsFirstLoadValid(Other.IsFirstLoadValid) {}

// An Operand is loaded into this virtual register.  Its Available set
// is cleared and then set to contain the Operand.
void IceRegManagerEntry::load(IceOperand *Operand) {
  Available.clear();
  if (Operand)
    Available.push_back(Operand);
  if (!IsFirstLoadValid) {
    FirstLoad = Operand;
    IsFirstLoadValid = true;
  }
}

// This virtual register is stored into a Variable, and the Variable
// is added to the virtual register's Available set.  If the Variable
// is a physical register managed by this RegManager (e.g., specific
// register requirements for instructions like x86 idiv), then all
// Available sets must be killed.
void IceRegManagerEntry::store(IceVariable *Variable) {
  // TODO: Kill all Available sets when necessary.
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

IceRegManager::IceRegManager(IceCfg *Cfg, IceCfgNode *Node, unsigned NumReg) {
  // TODO: Config flag to use physical registers directly.
  for (unsigned i = 0; i < NumReg; ++i) {
    char Buf[100];
    sprintf(Buf, "r%u_%u", i + 1, Node->getIndex());
    IceVariable *Reg = Cfg->getVariable(IceType_i32, Buf);
    Queue.push_back(new IceRegManagerEntry(Reg));
  }
}

IceRegManager::IceRegManager(IceCfg *Cfg, IceCfgNode *Node,
                             const IceRegManager &Other) :
  Queue(Other.Queue) {}

IceRegManager::IceRegManager(const IceRegManager &Other) {
  for (QueueType::const_iterator I = Other.Queue.begin(), E = Other.Queue.end();
       I != E; ++I) {
    Queue.push_back(new IceRegManagerEntry(**I));
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

void IceRegManager::notifyLoad(IceVariable *Reg, IceOperand *Operand) {
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
  Entry->load(Operand);
}

void IceRegManager::notifyStore(IceVariable *Reg, IceVariable *Variable) {
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
  Entry->store(Variable);
  // TODO: See the TODO in IceRegManagerEntry::store().  For the
  // instruction "a=reg", "a" must be removed from all other Available
  // sets.  But is this actually possible with SSA form?
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
    IceOperand *Operand = (*I)->getFirstLoad();
    if (Operand == NULL)
      continue;
    IceVariable *Var = (*I)->getVar();
    if (!First)
      Str << " ";
    Str << Var << ":" << Operand;
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
