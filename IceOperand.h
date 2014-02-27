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
    // Target-specific operand classes can use Target as their
    // starting type.
    Target
  };
  IceType getType(void) const { return Type; }
  OperandKind getKind(void) const { return Kind; }
  IceVariable *getVar(unsigned I) const {
    assert(I < getNumVars());
    return Vars[I];
  }
  unsigned getNumVars(void) const { return NumVars; }
  virtual void setUse(const IceInst *Inst, const IceCfgNode *Node) {}
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;

  virtual ~IceOperand() {}

protected:
  IceOperand(IceCfg *Cfg, OperandKind Kind, IceType Type)
      : Type(Type), Kind(Kind) {}
  const IceType Type;
  const OperandKind Kind;
  IceVariable **Vars;
  unsigned NumVars;
};

IceOstream &operator<<(IceOstream &Str, const IceOperand *O);

// TODO: better design of a minimal per-module constant pool,
// including synchronized access for parallel translation.
class IceConstant : public IceOperand {
public:
  virtual void emit(IceOstream &Str, uint32_t Option) const = 0;
  virtual void dump(IceOstream &Str) const = 0;

  static bool classof(const IceOperand *Operand) {
    OperandKind Kind = Operand->getKind();
    return Kind >= Constant && Kind <= Constant_Num;
  }

protected:
  IceConstant(IceCfg *Cfg, OperandKind Kind, IceType Type)
      : IceOperand(Cfg, Kind, Type) {
    Vars = NULL;
    NumVars = 0;
  }
};

class IceConstantInteger : public IceConstant {
public:
  static IceConstantInteger *create(IceCfg *Cfg, IceType Type,
                                    uint64_t IntValue) {
    return new IceConstantInteger(Cfg, Type, IntValue);
  }
  uint64_t getIntValue(void) const { return IntValue; }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;

  static bool classof(const IceOperand *Operand) {
    OperandKind Kind = Operand->getKind();
    return Kind == ConstantInteger;
  }

private:
  IceConstantInteger(IceCfg *Cfg, IceType Type, uint64_t IntValue)
      : IceConstant(Cfg, ConstantInteger, Type), IntValue(IntValue) {}
  const uint64_t IntValue;
};

class IceConstantRelocatable : public IceConstant {
public:
  static IceConstantRelocatable *create(IceCfg *Cfg, uint32_t CPIndex,
                                        IceType Type, const void *Handle,
                                        int64_t Offset,
                                        const IceString &Name = "") {
    return new IceConstantRelocatable(Cfg, Type, Handle, Offset, Name, CPIndex);
  }
  uint32_t getCPIndex(void) const { return CPIndex; }
  const void *getHandle(void) const { return Handle; }
  int64_t getOffset(void) const { return Offset; }
  IceString getName(void) const { return Name; }
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;

  static bool classof(const IceOperand *Operand) {
    OperandKind Kind = Operand->getKind();
    return Kind == ConstantRelocatable;
  }

private:
  IceConstantRelocatable(IceCfg *Cfg, IceType Type, const void *Handle,
                         int64_t Offset, const IceString &Name,
                         uint32_t CPIndex)
      : IceConstant(Cfg, ConstantRelocatable, Type), CPIndex(CPIndex),
        Handle(Handle), Offset(Offset), Name(Name) {}
  const uint32_t CPIndex;   // index into ICE constant pool
  const void *const Handle; // opaque handle e.g. to LLVM
  const int64_t Offset;     // fixed offset to add
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
  bool containsValue(int Value) const;
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
  static IceVariable *create(IceCfg *Cfg, IceType Type, const IceCfgNode *Node,
                             uint32_t Index, const IceString &Name) {
    return new IceVariable(Cfg, Type, Node, Index, Name);
  }
  void setUse(const IceInst *Inst, const IceCfgNode *Node);
  uint32_t getIndex(void) const { return Number; }
  IceInst *getDefinition(void) const { return DefInst; }
  void setDefinition(IceInst *Inst, const IceCfgNode *Node);
  void replaceDefinition(IceInst *Inst, const IceCfgNode *Node);
  // TODO: consider initializing IsArgument in the ctor.
  bool getIsArg(void) const { return IsArgument; }
  void setIsArg(void) { IsArgument = true; }
  IceVariable *getLow(void) const { return LowVar; }
  IceVariable *getHigh(void) const { return HighVar; }
  void setLow(IceVariable *Low) {
    assert(LowVar == NULL);
    LowVar = Low;
  }
  void setHigh(IceVariable *High) {
    assert(HighVar == NULL);
    HighVar = High;
  }
  bool isMultiblockLife(void) const { return (DefOrUseNode == NULL); }
  const IceCfgNode *getLocalUseNode() const { return DefOrUseNode; }
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
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;

  static bool classof(const IceOperand *Operand) {
    return Operand->getKind() == Variable;
  }

private:
  IceVariable(IceCfg *Cfg, IceType Type, const IceCfgNode *Node, uint32_t Index,
              const IceString &Name)
      : IceOperand(Cfg, Variable, Type), Number(Index), Name(Name),
        DefInst(NULL), DefOrUseNode(Node), IsArgument(false), StackOffset(0),
        RegNum(-1), RegNumTmp(-1), Weight(1), RegisterPreference(NULL),
        AllowRegisterOverlap(false), LowVar(NULL), HighVar(NULL) {
    Vars = new IceVariable *[1];
    Vars[0] = this;
    NumVars = 1;
  }
  const uint32_t Number;
  const IceString Name;
  // TODO: A Phi instruction lowers into several assignment
  // instructions with the same dest.  These should all be tracked
  // here so that they can all be deleted when this variable's use
  // count reaches zero.
  IceInst *DefInst;
  const IceCfgNode *DefOrUseNode; // for detecting isMultiblockLife()
  bool IsArgument;
  int
  StackOffset; // Canonical location on stack (only if RegNum==-1 || IsArgument)
  int RegNum;  // Allocated register; -1 for no allocation
  int RegNumTmp;       // Tentative assignment during register allocation
  IceRegWeight Weight; // Register allocation priority
  IceVariable *RegisterPreference;
  bool AllowRegisterOverlap;
  IceLiveRange LiveRange;
  // When lowering from I64 to I32 on a 32-bit architecture, we split
  // the variable into two machine-size pieces.  LowVar is the
  // low-order machine-size portion, and HighVar is the remaining
  // high-order portion.  TODO: It's wasteful to penalize all
  // variables on all targets this way; use a sparser representation.
  IceVariable *LowVar;
  IceVariable *HighVar;
};

#endif // _IceOperand_h
