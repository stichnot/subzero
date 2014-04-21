//===- subzero/src/IceRegManager.h - Simple local reg allocator -*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the IceRegManager class which does very simple
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

class IceRegManagerEntry {
public:
  static IceRegManagerEntry *create(IceCfg *Cfg, Variable *Var,
                                    uint32_t NumReg) {
    return new IceRegManagerEntry(Cfg, Var, NumReg);
  }
  static IceRegManagerEntry *
  create(IceCfg *Cfg, const IceRegManagerEntry &Other, uint32_t NumReg) {
    return new IceRegManagerEntry(Cfg, Other, NumReg);
  }
  void load(Inst *Inst);
  void store(Inst *Inst);
  bool contains(const Operand *Operand) const;
  Variable *getVar() const { return Var; }
  void dump(const IceCfg *Cfg) const;

private:
  IceRegManagerEntry(IceCfg *Cfg, Variable *Var, uint32_t NumReg);
  IceRegManagerEntry(IceCfg *Cfg, const IceRegManagerEntry &Other,
                     uint32_t NumReg);

  // Virtual register.
  Variable *const Var;

  // Set of operands currently available in the virtual register.
  OperandList Available;
};

// TODO: Use some "virtual register" subclass of Variable.
class IceRegManager {
public:
  typedef std::vector<IceRegManagerEntry *> QueueType;
  // Initialize a brand new register manager.
  static IceRegManager *create(IceCfg *Cfg, CfgNode *Node, uint32_t NumReg) {
    return new IceRegManager(Cfg, Node, NumReg);
  }
  // Capture the predecessor's end-of-block state for an extended
  // basic block.
  static IceRegManager *create(const IceRegManager &Other) {
    return new IceRegManager(Other);
  }
  // TODO: Are these Variable instances duplicated across
  // IceRegManager objects?
  Variable *getRegister(IceType Type, const OperandList &Prefer,
                        const VarList &Avoid) const;
  bool registerContains(const Variable *Reg, const Operand *Op) const;
  void notifyLoad(Inst *Inst, bool IsAssign = true);
  void notifyStore(Inst *Inst);
  void dump(const IceCfg *Cfg) const;

private:
  IceRegManager(IceCfg *Cfg, CfgNode *Node, uint32_t NumReg);
  IceRegManager(const IceRegManager &Other);
  const uint32_t NumReg;
  // The LRU register queue.  The front element is the least recently
  // used and the next to be assigned.
  // TODO: Multiple queues by type.
  QueueType Queue;
  IceCfg *Cfg;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEREGMANAGER_H
