//===- subzero/src/RegManager.cpp - Simple reg alloc implementation ----===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the RegManager class, which is a very
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

RegManagerEntry::RegManagerEntry(Cfg * /*Func*/, Variable *Var,
                                 SizeT /*NumReg*/)
    : Var(Var) {}

RegManagerEntry::RegManagerEntry(Cfg * /*Func*/, const RegManagerEntry &Other,
                                 SizeT /*NumReg*/)
    : Var(Other.Var), Available(Other.Available) {}

// An Operand is loaded into this virtual register.  Its Available set
// is cleared and then set to contain the Operand.
void RegManagerEntry::load(Inst *Inst) {
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
void RegManagerEntry::store(Inst *Inst) {
  // TODO: Kill all Available sets when necessary.
  Variable *Variable = Inst->getDest();
  assert(Variable);
  Available.push_back(Variable);
}

bool RegManagerEntry::contains(const Operand *Operand) const {
  if (getVar() == Operand)
    return true;
  for (OperandList::const_iterator I = Available.begin(), E = Available.end();
       I != E; ++I) {
    if (*I == Operand)
      return true;
    // TODO: Make it work for Constant pointers.
  }
  return false;
}

RegManager::RegManager(Cfg *Func, CfgNode *Node, SizeT NumReg)
    : NumReg(NumReg), Func(Func) {
  // TODO: Config flag to use physical registers directly.
  for (SizeT i = 0; i < NumReg; ++i) {
    const static size_t BufLen = 100;
    char Buf[BufLen];
    snprintf(Buf, BufLen, "r%u_%u", i + 1, Node->getIndex());
    Variable *Reg = Func->makeVariable(IceType_i32, Node, Buf);
    Queue.push_back(RegManagerEntry::create(Func, Reg, NumReg));
  }
}

RegManager::RegManager(const RegManager &Other)
    : NumReg(Other.NumReg), Func(Other.Func) {
  for (QueueType::const_iterator I = Other.Queue.begin(), E = Other.Queue.end();
       I != E; ++I) {
    Queue.push_back(RegManagerEntry::create(Func, **I, NumReg));
  }
}

// Prefer[0] is highest preference, Prefer[1] is second, etc.
Variable *RegManager::getRegister(IceType /*Type*/, const OperandList &Prefer,
                                  const VarList &Avoid)
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
  RegManagerEntry *Good = NULL;
  RegManagerEntry *Best = NULL;
  for (QueueType::const_iterator I1 = Queue.begin(), E1 = Queue.end(); I1 != E1;
       ++I1) {
    RegManagerEntry *Entry = *I1;
    bool AvoidEntry = false;
    for (VarList::const_iterator I2 = Avoid.begin(), E2 = Avoid.end(); I2 != E2;
         ++I2) {
      if (Entry->contains(*I2)) {
        AvoidEntry = true;
        break;
      }
    }
    if (AvoidEntry)
      continue;
    if (Good == NULL)
      Good = Entry;
    for (OperandList::const_iterator I2 = Prefer.begin(), E2 = Prefer.end();
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

bool RegManager::registerContains(const Variable *Reg,
                                  const Operand *Op) const {
  for (QueueType::const_iterator I = Queue.begin(), E = Queue.end(); I != E;
       ++I) {
    if ((*I)->getVar() == Reg)
      return (*I)->contains(Op);
  }
  assert(false);
  return false;
}

void RegManager::notifyLoad(Inst *Inst, bool IsAssign) {
  Variable *Reg = Inst->getDest();
  RegManagerEntry *Entry = NULL;
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

void RegManager::notifyStore(Inst *Inst) {
  Variable *Reg = llvm::cast<Variable>(Inst->getSrc(0));
  RegManagerEntry *Entry = NULL;
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
  // TODO: See the TODO in RegManagerEntry::store().  For the
  // instruction "a=reg", "a" must be removed from all other Available
  // sets.  But is this actually possible with SSA form?
}

// ======================== Dump routines ======================== //

void RegManager::dump(const Cfg *Func) const {
  for (QueueType::const_iterator I = Queue.begin(), E = Queue.end(); I != E;
       ++I) {
    (*I)->dump(Func);
  }
}

void RegManagerEntry::dump(const Cfg *Func) const {
  Ostream &Str = Func->getContext()->getStrDump();
  Str << " " << getVar() << "={";
  for (OperandList::const_iterator I = Available.begin(), E = Available.end();
       I != E; ++I) {
    if (I != Available.begin())
      Str << ",";
    Str << *I;
  }
  Str << "}";
}

} // end of namespace Ice
