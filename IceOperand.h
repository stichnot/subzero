// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef _IceOperand_h
#define _IceOperand_h

#include <set>
#include <utility>

#include "IceDefs.h"
#include "IceTypes.h"

/*
  Operand (rvalue)
    Constant
      ActualInteger
      ActualFloat (bits that go into module constant pool)
      Symbolic
      Symbolic+Integer (e.g. address)
    Variable (stack or register - lvalue)
      Virtual register (pre-colored)
      Physical register
  load/store instructions can take a global (constant?)
  or maybe a TLS address
 */

class IceVariable;
class IceConstant;

class IceOperand {
public:
  enum OperandKind {
    Constant,
    ConstantInteger,
    ConstantFloat,
    Constant_Num,
    Variable,
    VariableVirtualRegister,
    VariablePhysicalRegister,
    Variable_Num,
  };
  IceType getType(void) const { return Type; }
  OperandKind getKind(void) const { return Kind; }
  virtual void setUse(const IceInst *Inst, const IceCfgNode *Node) {}
  virtual void removeUse(void) {}
  virtual uint32_t getUseCount(void) const { return 0; }
  virtual void dump(IceOstream &Str) const;
protected:
  IceOperand(OperandKind Kind, IceType Type) : Type(Type), Kind(Kind) {}
  const IceType Type;
private:
  const OperandKind Kind;
};

IceOstream& operator<<(IceOstream &Str, const IceOperand *O);

// TODO: better design of a minimal per-module constant pool,
// including synchronized access for parallel translation.
// TODO: handle symbolic constants.
class IceConstant : public IceOperand {
public:
  IceConstant(int32_t IntValue) : IceOperand(Constant, IceType_i32) {
    Value.I32 = IntValue;
  }
  uint32_t getIntValue(void) const { return Value.I32; }
  virtual void dump(IceOstream &Str) const;

  static bool classof(const IceOperand *Operand) {
    OperandKind Kind = Operand->getKind();
    return Kind >= Constant && Kind <= Constant_Num;
  }
private:
  union {
    uint8_t I1;
    uint8_t I8;
    uint16_t I16;
    uint32_t I32;
    uint64_t I64;
    // TODO: don't store FP constants as float/double due to NaN issues
    float F32;
    double F64;
  } Value;
};

class IceLiveRange {
public:
  void reset(void) { Range.clear(); }
  void addSegment(int Start, int End);
  bool endsBefore(const IceLiveRange &Other) const;
  bool overlaps(const IceLiveRange &Other) const;
  void dump(IceOstream &Str) const;
  static void unitTests(void);
private:
  typedef std::set<std::pair<int, int> > RangeType;
  RangeType Range;
};

IceOstream& operator<<(IceOstream &Str, const IceLiveRange &L);

// Stack operand, or virtual or physical register
class IceVariable : public IceOperand {
public:
  IceVariable(IceType Type, uint32_t Index) :
    IceOperand(Variable, Type), VarIndex(Index), UseCount(0), DefInst(NULL),
    DefOrUseNode(NULL), IsMultiDef(false), IsArgument(false),
    IsMultiblockLife(false), AllowAutoDelete(true), RegNum(-1) {}
  virtual void setUse(const IceInst *Inst, const IceCfgNode *Node);
  virtual void removeUse(void);
  virtual uint32_t getUseCount(void) const { return UseCount; }
  uint32_t getIndex(void) const { return VarIndex; }
  IceInst *getDefinition(void) const { return DefInst; }
  void setDefinition(IceInst *Inst, const IceCfgNode *Node);
  void replaceDefinition(IceInst *Inst, const IceCfgNode *Node);
  // TODO: consider initializing IsArgument in the ctor.
  void setIsArg(void) { IsArgument = true; }
  bool isMultiblockLife(void) const {
    return IsMultiblockLife | IsArgument;
    // TODO: if an argument is only used in the entry node, and there
    // are no back-edges to the entry node, then it doesn't have a
    // multi-block lifetime.
  }
  void setNoAutoDelete(void) { AllowAutoDelete = false; }
  bool canAutoDelete(void) const { return AllowAutoDelete; }
  void setRegNum(int NewRegNum) {
    assert(RegNum < 0); // shouldn't set it more than once
    RegNum = NewRegNum;
  }
  int getRegNum(void) const { return RegNum; }
  void resetLiveRange(void) { LiveRange.reset(); }
  void addLiveRange(int Start, int End) { LiveRange.addSegment(Start, End); }
  const IceLiveRange &getLiveRange(void) const { return LiveRange; }
  virtual void dump(IceOstream &Str) const;

  static bool classof(const IceOperand *Operand) {
    OperandKind Kind = Operand->getKind();
    return Kind >= Variable && Kind <= Variable_Num;
  }
private:
  // operand name for pretty-printing
  const uint32_t VarIndex;
  // decorations
  uint32_t UseCount;
  // TODO: A Phi instruction lowers into several assignment
  // instructions with the same dest.  These should all be tracked
  // here so that they can all be deleted when this variable's use
  // count reaches zero.
  IceInst *DefInst;
  const IceCfgNode *DefOrUseNode; // for detecting IsMultiblockLife
  bool IsMultiDef;
  bool IsArgument;
  bool IsMultiblockLife;
  bool AllowAutoDelete;
  // Allocated register; -1 for no allocation
  int RegNum;
  IceLiveRange LiveRange;
};

#endif // _IceOperand_h
