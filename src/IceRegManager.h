//===- subzero/src/RegManager.h - Simple local reg allocator -*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the RegManager class which does very simple
// register allocation across a single basic block.
//
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEREGMANAGER_H
#define SUBZERO_SRC_ICEREGMANAGER_H

#include "IceDefs.h"
#include "IceTypes.h"

namespace Ice {

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

class RegManagerEntry {
public:
  static RegManagerEntry *create(Cfg *Func, Variable *Var, SizeT NumReg) {
    return new RegManagerEntry(Func, Var, NumReg);
  }
  static RegManagerEntry *create(Cfg *Func, const RegManagerEntry &Other,
                                 SizeT NumReg) {
    return new RegManagerEntry(Func, Other, NumReg);
  }
  void load(Inst *Inst);
  void store(Inst *Inst);
  bool contains(const Operand *Operand) const;
  Variable *getVar() const { return Var; }
  void dump(const Cfg *Func) const;

private:
  RegManagerEntry(Cfg *Func, Variable *Var, SizeT NumReg);
  RegManagerEntry(Cfg *Func, const RegManagerEntry &Other, SizeT NumReg);

  // Virtual register.
  Variable *const Var;

  // Set of operands currently available in the virtual register.
  OperandList Available;
};

// TODO: Use some "virtual register" subclass of Variable.
class RegManager {
public:
  typedef std::vector<RegManagerEntry *> QueueType;
  // Initialize a brand new register manager.
  static RegManager *create(Cfg *Func, CfgNode *Node, SizeT NumReg) {
    return new RegManager(Func, Node, NumReg);
  }
  // Capture the predecessor's end-of-block state for an extended
  // basic block.
  static RegManager *create(const RegManager &Other) {
    return new RegManager(Other);
  }
  // TODO: Are these Variable instances duplicated across
  // RegManager objects?
  Variable *getRegister(IceType Type, const OperandList &Prefer,
                        const VarList &Avoid) const;
  bool registerContains(const Variable *Reg, const Operand *Op) const;
  void notifyLoad(Inst *Inst, bool IsAssign = true);
  void notifyStore(Inst *Inst);
  void dump(const Cfg *Func) const;

private:
  RegManager(Cfg *Func, CfgNode *Node, SizeT NumReg);
  RegManager(const RegManager &Other);
  const SizeT NumReg;
  // The LRU register queue.  The front element is the least recently
  // used and the next to be assigned.
  // TODO: Multiple queues by type.
  QueueType Queue;
  Cfg *Func;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEREGMANAGER_H
