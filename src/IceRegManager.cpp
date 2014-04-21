//===- subzero/src/IceRegManager.cpp - Simple reg alloc implementation ----===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the IceRegManager class, which is a very
// simple register allocator that uses operand availability along with
// a LRU replacement policy that allocates across a single or extended
// basic block.
//
//===----------------------------------------------------------------------===//

#include <string.h>

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInst.h"
#include "IceOperand.h"
#include "IceRegManager.h"
#include "IceTargetLowering.h"
#include "IceTypes.h"

namespace Ice {

IceRegManagerEntry::IceRegManagerEntry(IceCfg *Cfg, IceVariable *Var,
                                       uint32_t NumReg)
    : Var(Var) {}

IceRegManagerEntry::IceRegManagerEntry(IceCfg *Cfg,
                                       const IceRegManagerEntry &Other,
                                       uint32_t NumReg)
    : Var(Other.Var), Available(Other.Available) {}

// An Operand is loaded into this virtual register.  Its Available set
// is cleared and then set to contain the Operand.
void IceRegManagerEntry::load(Inst *Inst) {
  Available.clear();
  if (Inst) {
    Operand *Operand = Inst->getSrc(0);
    Available.push_back(Operand);
  }
}

// This virtual register is stored into a Variable, and the Variable
// is added to the virtual register's Available set.  If the Variable
// is a physical register managed by this RegManager (e.g., specific
// register requirements for instructions like x86 idiv), then all
// Available sets must be killed.
void IceRegManagerEntry::store(Inst *Inst) {
  // TODO: Kill all Available sets when necessary.
  IceVariable *Variable = Inst->getDest();
  assert(Variable);
  Available.push_back(Variable);
}

bool IceRegManagerEntry::contains(const Operand *Operand) const {
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

IceRegManager::IceRegManager(IceCfg *Cfg, CfgNode *Node, uint32_t NumReg)
    : NumReg(NumReg), Cfg(Cfg) {
  // TODO: Config flag to use physical registers directly.
  for (uint32_t i = 0; i < NumReg; ++i) {
    const static size_t BufLen = 100;
    char Buf[BufLen];
    snprintf(Buf, BufLen, "r%u_%u", i + 1, Node->getIndex());
    IceVariable *Reg = Cfg->makeVariable(IceType_i32, Node, Buf);
    Queue.push_back(IceRegManagerEntry::create(Cfg, Reg, NumReg));
  }
}

IceRegManager::IceRegManager(const IceRegManager &Other)
    : NumReg(Other.NumReg), Cfg(Other.Cfg) {
  for (QueueType::const_iterator I = Other.Queue.begin(), E = Other.Queue.end();
       I != E; ++I) {
    Queue.push_back(IceRegManagerEntry::create(Cfg, **I, NumReg));
  }
}

// Prefer[0] is highest preference, Prefer[1] is second, etc.
IceVariable *IceRegManager::getRegister(IceType Type, const IceOpList &Prefer,
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
  for (QueueType::const_iterator I1 = Queue.begin(), E1 = Queue.end(); I1 != E1;
       ++I1) {
    IceRegManagerEntry *Entry = *I1;
    bool AvoidEntry = false;
    for (IceVarList::const_iterator I2 = Avoid.begin(), E2 = Avoid.end();
         I2 != E2; ++I2) {
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
         I2 != E2; ++I2) {
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
                                     const Operand *Op) const {
  for (QueueType::const_iterator I = Queue.begin(), E = Queue.end(); I != E;
       ++I) {
    if ((*I)->getVar() == Reg)
      return (*I)->contains(Op);
  }
  assert(false);
  return false;
}

void IceRegManager::notifyLoad(Inst *Inst, bool IsAssign) {
  IceVariable *Reg = Inst->getDest();
  IceRegManagerEntry *Entry = NULL;
  for (QueueType::iterator I = Queue.begin(), E = Queue.end(); I != E; ++I) {
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

void IceRegManager::notifyStore(Inst *Inst) {
  IceVariable *Reg = llvm::cast<IceVariable>(Inst->getSrc(0));
  IceRegManagerEntry *Entry = NULL;
  for (QueueType::iterator I = Queue.begin(), E = Queue.end(); I != E; ++I) {
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

// ======================== Dump routines ======================== //

void IceRegManager::dump(const IceCfg *Cfg) const {
  for (QueueType::const_iterator I = Queue.begin(), E = Queue.end(); I != E;
       ++I) {
    (*I)->dump(Cfg);
  }
}

void IceRegManagerEntry::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  Str << " " << getVar() << "={";
  for (IceOpList::const_iterator I = Available.begin(), E = Available.end();
       I != E; ++I) {
    if (I != Available.begin())
      Str << ",";
    Str << *I;
  }
  Str << "}";
}

} // end of namespace Ice
