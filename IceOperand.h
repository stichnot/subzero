// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef _IceOperand_h
#define _IceOperand_h

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

class IceOperand {
public:
  enum OperandKind {
    Constant,
    ConstantInteger,
    ConstantFloat,
    ConstantRelocatable,
    Constant_Num,
    Variable,
    VariableVirtualRegister,
    VariablePhysicalRegister,
    Variable_Num,
  };
  IceType getType(void) const { return Type; }
  OperandKind getKind(void) const { return Kind; }
  virtual void setUse(const IceInst *Inst, const IceCfgNode *Node) {}
  virtual void dump(IceOstream &Str) const;

protected:
  IceOperand(OperandKind Kind, IceType Type) : Type(Type), Kind(Kind) {}
  const IceType Type;

private:
  const OperandKind Kind;
};

IceOstream &operator<<(IceOstream &Str, const IceOperand *O);

// TODO: better design of a minimal per-module constant pool,
// including synchronized access for parallel translation.
class IceConstant : public IceOperand {
public:
  virtual void dump(IceOstream &Str) const = 0;

  static bool classof(const IceOperand *Operand) {
    OperandKind Kind = Operand->getKind();
    return Kind >= Constant && Kind <= Constant_Num;
  }

protected:
  IceConstant(OperandKind Kind, IceType Type) : IceOperand(Kind, Type) {}
};

class IceConstantInteger : public IceConstant {
public:
  IceConstantInteger(IceType Type, uint64_t IntValue)
      : IceConstant(ConstantInteger, Type), IntValue(IntValue) {}
  uint64_t getIntValue(void) const { return IntValue; }
  virtual void dump(IceOstream &Str) const;

  static bool classof(const IceOperand *Operand) {
    OperandKind Kind = Operand->getKind();
    return Kind == ConstantInteger;
  }

private:
  const uint64_t IntValue;
};

class IceConstantRelocatable : public IceConstant {
public:
  IceConstantRelocatable(IceType Type, const void *Handle,
                         const IceString &Name = "")
      : IceConstant(ConstantRelocatable, Type), CPIndex(0), Handle(Handle),
        Name(Name) {}
  uint32_t getCPIndex(void) const { return CPIndex; }
  const void *getHandle(void) const { return Handle; }
  IceString getName(void) const { return Name; }
  virtual void dump(IceOstream &Str) const;

  static bool classof(const IceOperand *Operand) {
    OperandKind Kind = Operand->getKind();
    return Kind == ConstantRelocatable;
  }

private:
  const uint32_t CPIndex;   // index into ICE constant pool
  const void *const Handle; // opaque handle e.g. to LLVM
  const IceString Name;     // optional for debug/dump
};

class IceRegWeight {
public:
  IceRegWeight(void) : Weight(0) {}
  IceRegWeight(uint32_t Weight) : Weight(Weight) {}
  const static uint32_t Inf = ~0;
  void addWeight(uint32_t Delta) {
    if (Delta == Inf)
      Weight = Inf;
    else if (Weight != Inf)
      Weight += Delta;
  }
  void addWeight(const IceRegWeight &Other) { addWeight(Other.Weight); }
  void setWeight(uint32_t Val) { Weight = Val; }
  uint32_t getWeight(void) const { return Weight; }
  bool isInf(void) const { return Weight == Inf; }

private:
  uint32_t Weight;
};
IceOstream &operator<<(IceOstream &Str, const IceRegWeight &W);
bool operator<(const IceRegWeight &A, const IceRegWeight &B);
bool operator<=(const IceRegWeight &A, const IceRegWeight &B);

class IceLiveRange {
public:
  IceLiveRange(void) : Weight(0) {}
  int getStart(void) const { return Range.empty() ? -1 : Range.begin()->first; }
  void reset(void) {
    Range.clear();
    Weight.setWeight(0);
  }
  void addSegment(int Start, int End);
  bool endsBefore(const IceLiveRange &Other) const;
  bool overlaps(const IceLiveRange &Other) const;
  bool isEmpty(void) const { return Range.empty(); }
  IceRegWeight getWeight(void) const { return Weight; }
  void setWeight(const IceRegWeight &NewWeight) { Weight = NewWeight; }
  void addWeight(uint32_t Delta) { Weight.addWeight(Delta); }
  void dump(IceOstream &Str) const;
  static void unitTests(void);

private:
  typedef std::pair<int, int> RangeElementType;
  typedef std::set<RangeElementType> RangeType;
  RangeType Range;
  IceRegWeight Weight;
};

IceOstream &operator<<(IceOstream &Str, const IceLiveRange &L);

// Stack operand, or virtual or physical register
class IceVariable : public IceOperand {
public:
  IceVariable(IceType Type, uint32_t Index, const IceString &Name)
      : IceOperand(Variable, Type), Number(Index), Name(Name), DefInst(NULL),
        DefOrUseNode(NULL), IsArgument(false), IsMultiblockLife(false),
        StackOffset(0), RegNum(-1), RegNumTmp(-1), Weight(1),
        RegisterPreference(NULL), AllowRegisterOverlap(false) {}
  virtual void setUse(const IceInst *Inst, const IceCfgNode *Node);
  uint32_t getIndex(void) const { return Number; }
  IceInst *getDefinition(void) const { return DefInst; }
  void setDefinition(IceInst *Inst, const IceCfgNode *Node);
  void replaceDefinition(IceInst *Inst, const IceCfgNode *Node);
  // TODO: consider initializing IsArgument in the ctor.
  bool getIsArg(void) const { return IsArgument; }
  void setIsArg(void) { IsArgument = true; }
  bool isMultiblockLife(void) const {
    return IsMultiblockLife | IsArgument;
    // TODO: if an argument is only used in the entry node, and there
    // are no back-edges to the entry node, then it doesn't have a
    // multi-block lifetime.
  }
  void setRegNum(int NewRegNum) {
    assert(RegNum < 0 ||
           RegNum == NewRegNum); // shouldn't set it more than once
    RegNum = NewRegNum;
  }
  int getRegNum(void) const { return RegNum; }
  void setRegNumTmp(int NewRegNum) { RegNumTmp = NewRegNum; }
  int getRegNumTmp(void) const { return RegNumTmp; }
  void setStackOffset(int Offset) { StackOffset = Offset; }
  int getStackOffset(void) const { return StackOffset; }
  void setWeight(uint32_t NewWeight) { Weight = NewWeight; }
  void setWeightInfinite(void) { Weight = IceRegWeight::Inf; }
  IceRegWeight getWeight(void) const { return Weight; }
  void setPreferredRegister(IceVariable *Prefer, bool Overlap) {
    RegisterPreference = Prefer;
    AllowRegisterOverlap = Overlap;
  }
  IceVariable *getPreferredRegister(void) const { return RegisterPreference; }
  bool getRegisterOverlap(void) const { return AllowRegisterOverlap; }
  void resetLiveRange(void) { LiveRange.reset(); }
  void addLiveRange(int Start, int End, uint32_t WeightDelta) {
    assert(WeightDelta != IceRegWeight::Inf);
    LiveRange.addSegment(Start, End);
    if (Weight.isInf())
      LiveRange.setWeight(IceRegWeight::Inf);
    else
      LiveRange.addWeight(WeightDelta * Weight.getWeight());
  }
  const IceLiveRange &getLiveRange(void) const { return LiveRange; }
  void setLiveRangeInfiniteWeight(void) {
    LiveRange.setWeight(IceRegWeight::Inf);
  }
  IceString getName(void) const;
  virtual void dump(IceOstream &Str) const;

  static bool classof(const IceOperand *Operand) {
    OperandKind Kind = Operand->getKind();
    return Kind >= Variable && Kind <= Variable_Num;
  }

private:
  const uint32_t Number;
  const IceString Name;
  // TODO: A Phi instruction lowers into several assignment
  // instructions with the same dest.  These should all be tracked
  // here so that they can all be deleted when this variable's use
  // count reaches zero.
  IceInst *DefInst;
  const IceCfgNode *DefOrUseNode; // for detecting IsMultiblockLife
  bool IsArgument;
  bool IsMultiblockLife;
  int
  StackOffset; // Canonical location on stack (only if RegNum==-1 || IsArgument)
  int RegNum;  // Allocated register; -1 for no allocation
  int RegNumTmp;       // Tentative assignment during register allocation
  IceRegWeight Weight; // Register allocation priority
  IceVariable *RegisterPreference;
  bool AllowRegisterOverlap;
  IceLiveRange LiveRange;
};

#endif // _IceOperand_h
