/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

// -*- Mode: c++ -*-
#ifndef _IceRegManager_h
#define _IceRegManager_h

#include "IceDefs.h"
#include "IceTypes.h"

// Maintain LRU allocation order.
// Track which variables are available in which registers.
//   Each register has a bitmap of variables.
// Track which constants are available in which registers.
//   Each register has a constant.
// Track which registers contain which variables.
//   Each variable has a bitmap of registers.
// First variable loaded into each register in the block.
//
// Select register for loading specific variable or constant.
// Pass a list of preferences and a list of anti-preferences.
//
// Notify of a load or other change to a register's contents ("other
// change": e.g. a function call destroys the scratch registers'
// contents).  Record it if this is the first load/change in the block
// to the register.  Remove the old register/operand availabilities
// and add the new.
//
// Notify of a store of the register into a variable.  Add the new
// register/operand availability.
//
//
// Examples:
//   a = b + c
// r1 = b; r1 += c; a = r1;   -or-   r1 = c; r1 += b; a = r1;
// r1 is destroyed in both cases.  So when asking for an r1, our
// preferences are:
// 1. Register holding b if this instruction ends b's live range.
//    (r1==b); r1 += c; a = r1;
// 2. Register holding c if this instruction ends c's live range.
//    And only if the operator is commutative.
//    (r1==c); r1 += b; a = r1;
// 3. Register not holding b or c.
//    r1 = b; r1 += c; a = r1;
// 4. Any register.
//    r1 = b; r1 += c; a = r1;
// May want to prioritize #1 and #2 by LRU.
// #3 and #4 could use the lea instruction if b and c are already in
// registers.
// The cmp/fcmp instruction is like a commutative arithmetic instruction.

class IceRegManagerEntry {
public:
  IceRegManagerEntry(IceVariable *Var);
  IceRegManagerEntry(const IceRegManagerEntry &Other, unsigned NumReg);
  void load(IceInst *Inst);
  void store(IceInst *Inst);
  bool contains(const IceOperand *Operand) const;
  IceVariable *getVar(void) const { return Var; }
  IceInst *getFirstLoadInst(void) const {
    return IsFirstLoadValid ? FirstLoadInst : NULL;
  }
  void updateCandidateWeight(int Incr=1) { MultiblockCandidateWeight += Incr; }
  int getCandidateWeight(void) const { return MultiblockCandidateWeight; }
  void dump(IceOstream &Str) const;
private:
  // Virtual register.
  IceVariable *const Var;

  // Set of operands currently available in the virtual register.
  IceOpList Available;

  // Instruction that loads the first operand into the virtual
  // register.  Used for multiblock register allocation.  Can be NULL
  // if the first load into the virtual register is not a valid
  // candidate for multiblock register allocation, e.g. the Load
  // bitcode.
  IceInst *FirstLoadInst;

  // Whether the first load into the virtual register has been
  // encountered.  TODO: consider saving memory by initializing
  // FirstLoadInst to point to a sentinel instruction.
  bool IsFirstLoadValid;

  // Number of predecessors that have the FirstLoadInst operand in the
  // Available set.  Must be greater than zero to be a candidation for
  // multiblock register allocation.
  unsigned MultiblockCandidateWeight;

  // This is an array indexed by (small) physical register number,
  // giving the number of "votes" for each physical register.  Initial
  // votes are given during initial instruction selection as a result
  // of target instruction constraints.
  int *PhysicalRegisterVotes;

  // The sum of PhysicalRegisterVotes, for convenience.  The virtual
  // register with the highest TotalVotes gets the first pick of
  // physical registers.
  int TotalVotes;

  // If nonnegative, this is the (small) register number of the
  // assigned physical register.  TODO: consider moving this into
  // IceVariable for global register allocation.
  int PhysicalRegister;
};

// TODO: Use some "virtual register" subclass of IceVariable.
class IceRegManager {
public:
  typedef std::vector<IceRegManagerEntry*> QueueType;
  // Initialize a brand new register manager.
  IceRegManager(IceCfg *Cfg, IceCfgNode *Node, unsigned NumReg);
  // Capture the predecessor's end-of-block state for an extended
  // basic block.
  IceRegManager(const IceRegManager &Other);
  // TODO: Are these IceVariable instances duplicated across
  // IceRegManager objects?
  IceVariable *getRegister(IceType Type,
                           const IceOpList &Prefer,
                           const IceVarList &Avoid) const;
  bool registerContains(const IceVariable *Reg, const IceOperand *Op) const;
  void notifyLoad(IceInst *Inst, bool IsAssign = true);
  void notifyStore(IceInst *Inst);
  void updateCandidates(const IceRegManager *Pred);
  bool isAvailable(const IceOperand *Operand) const;
  void dump(IceOstream &Str) const;
  void dumpFirstLoads(IceOstream &Str) const;
private:
  const unsigned NumReg;
  // The LRU register queue.  The front element is the least recently
  // used and the next to be assigned.
  // TODO: Multiple queues by type.
  QueueType Queue;
};

#endif // _IceRegManager_h
