//===- subzero/src/IceOperand.h - High-level operands -----------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the IceOperand class and its target-independent
// subclasses.  The main classes are IceVariable, which represents an
// LLVM variable that is either register- or stack-allocated, and the
// IceConstant hierarchy, which represents integer, floating-point,
// and/or symbolic constants.
//
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEOPERAND_H
#define SUBZERO_SRC_ICEOPERAND_H

#include "IceDefs.h"
#include "IceTypes.h"

class IceOperand {
public:
  enum OperandKind {
    Constant,
    ConstantInteger,
    ConstantFloat,
    ConstantDouble,
    ConstantRelocatable,
    Constant_Num,
    Variable,
    // Target-specific operand classes use Target as the starting
    // point for their Kind enum space.
    Target
  };
  OperandKind getKind() const { return Kind; }
  IceType getType() const { return Type; }

  // Every IceOperand keeps an array of the IceVariables referenced in
  // the operand.  This is so that the liveness operations can get
  // quick access to the variables of interest, without having to dig
  // so far into the operand.
  uint32_t getNumVars() const { return NumVars; }
  IceVariable *getVar(uint32_t I) const {
    assert(I < getNumVars());
    return Vars[I];
  }
  virtual void setUse(const IceInst *Inst, const IceCfgNode *Node) {}
  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;

  virtual ~IceOperand() {}

protected:
  IceOperand(IceCfg *Cfg, OperandKind Kind, IceType Type)
      : Type(Type), Kind(Kind) {}
  const IceType Type;
  const OperandKind Kind;
  // Vars and NumVars are initialized by the derived class.
  uint32_t NumVars;
  IceVariable **Vars;
};

IceOstream &operator<<(IceOstream &Str, const IceOperand *O);

// IceConstant is the abstract base class for constants.
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

// IceConstantPrimitive<> wraps a primitive type.
template <typename T, IceOperand::OperandKind K>
class IceConstantPrimitive : public IceConstant {
public:
  static IceConstantPrimitive *create(IceCfg *Cfg, IceType Type, T Value) {
    // Use non-placement allocation for constants for now, until
    // global constant pool issues are worked out.
    return new IceConstantPrimitive(Cfg, Type, Value);
  }
  T getValue() const { return Value; }
  virtual void emit(IceOstream &Str, uint32_t Option) const { dump(Str); }
  virtual void dump(IceOstream &Str) const { Str << getValue(); }

  static bool classof(const IceOperand *Operand) {
    return Operand->getKind() == K;
  }

private:
  IceConstantPrimitive(IceCfg *Cfg, IceType Type, T Value)
      : IceConstant(Cfg, K, Type), Value(Value) {}
  const T Value;
};

typedef IceConstantPrimitive<uint64_t, IceOperand::ConstantInteger>
IceConstantInteger;
typedef IceConstantPrimitive<float, IceOperand::ConstantFloat> IceConstantFloat;
typedef IceConstantPrimitive<double, IceOperand::ConstantDouble>
IceConstantDouble;

// IceConstantRelocatable represents a symbolic constant combined with
// a fixed offset.
class IceConstantRelocatable : public IceConstant {
public:
  static IceConstantRelocatable *create(IceCfg *Cfg, uint32_t CPIndex,
                                        IceType Type, const void *Handle,
                                        int64_t Offset,
                                        const IceString &Name = "") {
    // Use non-placement allocation for constants for now, until
    // global constant pool issues are worked out.
    return new IceConstantRelocatable(Cfg, Type, Handle, Offset, Name, CPIndex);
  }
  uint32_t getCPIndex() const { return CPIndex; }
  const void *getHandle() const { return Handle; }
  int64_t getOffset() const { return Offset; }
  IceString getName() const { return Name; }
  void setSuppressMangling(bool Value) { SuppressMangling = Value; }
  bool getSuppressMangling() const { return SuppressMangling; }
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
        Handle(Handle), Offset(Offset), Name(Name), SuppressMangling(false) {}
  const uint32_t CPIndex;   // index into ICE constant pool
  const void *const Handle; // opaque handle e.g. to LLVM
  const int64_t Offset;     // fixed offset to add
  const IceString Name;     // optional for debug/dump
  bool SuppressMangling;
};

// IceRegWeight is a wrapper for a uint32_t weight value, with a
// special value that represents infinite weight, and an addWeight()
// method that ensures that W+infinity=infinity.
class IceRegWeight {
public:
  IceRegWeight() : Weight(0) {}
  IceRegWeight(uint32_t Weight) : Weight(Weight) {}
  const static uint32_t Inf = ~0; // Force regalloc to give a register
  const static uint32_t Zero = 0; // Force regalloc NOT to give a register
  void addWeight(uint32_t Delta) {
    if (Delta == Inf)
      Weight = Inf;
    else if (Weight != Inf)
      Weight += Delta;
  }
  void addWeight(const IceRegWeight &Other) { addWeight(Other.Weight); }
  void setWeight(uint32_t Val) { Weight = Val; }
  uint32_t getWeight() const { return Weight; }
  bool isInf() const { return Weight == Inf; }

private:
  uint32_t Weight;
};
IceOstream &operator<<(IceOstream &Str, const IceRegWeight &W);
bool operator<(const IceRegWeight &A, const IceRegWeight &B);
bool operator<=(const IceRegWeight &A, const IceRegWeight &B);
bool operator==(const IceRegWeight &A, const IceRegWeight &B);

// IceLiveRange is a set of instruction number intervals representing
// a variable's live range.  Generally there is one interval per basic
// block where the variable is live, but adjacent intervals get
// coalesced into a single interval.  IceLiveRange also includes a
// weight, in case e.g. we want a live range to have higher weight
// inside a loop.
class IceLiveRange {
public:
  IceLiveRange() : Weight(0) {}

  void reset() {
    Range.clear();
    Weight.setWeight(0);
  }
  void addSegment(int32_t Start, int32_t End);

  bool endsBefore(const IceLiveRange &Other) const;
  bool overlaps(const IceLiveRange &Other) const;
  bool containsValue(int32_t Value) const;
  bool isEmpty() const { return Range.empty(); }
  int32_t getStart() const { return Range.empty() ? -1 : Range.begin()->first; }

  IceRegWeight getWeight() const { return Weight; }
  void setWeight(const IceRegWeight &NewWeight) { Weight = NewWeight; }
  void addWeight(uint32_t Delta) { Weight.addWeight(Delta); }
  void dump(IceOstream &Str) const;

  // Defining USE_SET uses std::set to hold the segments instead of
  // std::list.  Using std::list will be slightly faster, but is more
  // restrictive because new segments cannot be added in the middle.

  //#define USE_SET

private:
  typedef std::pair<int32_t, int32_t> RangeElementType;
#ifdef USE_SET
  typedef std::set<RangeElementType> RangeType;
#else
  typedef std::list<RangeElementType> RangeType;
#endif
  RangeType Range;
  IceRegWeight Weight;
};

IceOstream &operator<<(IceOstream &Str, const IceLiveRange &L);

// IceVariable represents an operand that is register-allocated or
// stack-allocated.  If it is register-allocated, it will ultimately
// have a non-negative RegNum field.
class IceVariable : public IceOperand {
public:
  static IceVariable *create(IceCfg *Cfg, IceType Type, const IceCfgNode *Node,
                             uint32_t Index, const IceString &Name) {
    return new (Cfg->allocate<IceVariable>())
        IceVariable(Cfg, Type, Node, Index, Name);
  }

  uint32_t getIndex() const { return Number; }
  IceString getName() const;

  IceInst *getDefinition() const { return DefInst; }
  void setDefinition(IceInst *Inst, const IceCfgNode *Node);
  void replaceDefinition(IceInst *Inst, const IceCfgNode *Node);

  const IceCfgNode *getLocalUseNode() const { return DefNode; }
  bool isMultiblockLife() const { return (DefNode == NULL); }
  void setUse(const IceInst *Inst, const IceCfgNode *Node);

  bool getIsArg() const { return IsArgument; }
  void setIsArg(IceCfg *Cfg);

  int32_t getStackOffset() const { return StackOffset; }
  void setStackOffset(int32_t Offset) { StackOffset = Offset; }

  static const int32_t NoRegister = -1;
  bool hasReg() const { return getRegNum() != NoRegister; }
  int32_t getRegNum() const { return RegNum; }
  void setRegNum(int32_t NewRegNum) {
    // Regnum shouldn't be set more than once.
    assert(!hasReg() || RegNum == NewRegNum);
    RegNum = NewRegNum;
  }
  bool hasRegTmp() const { return getRegNumTmp() != NoRegister; }
  int32_t getRegNumTmp() const { return RegNumTmp; }
  void setRegNumTmp(int32_t NewRegNum) { RegNumTmp = NewRegNum; }

  IceRegWeight getWeight() const { return Weight; }
  void setWeight(uint32_t NewWeight) { Weight = NewWeight; }
  void setWeightInfinite() { Weight = IceRegWeight::Inf; }

  IceVariable *getPreferredRegister() const { return RegisterPreference; }
  bool getRegisterOverlap() const { return AllowRegisterOverlap; }
  void setPreferredRegister(IceVariable *Prefer, bool Overlap) {
    RegisterPreference = Prefer;
    AllowRegisterOverlap = Overlap;
  }

  const IceLiveRange &getLiveRange() const { return LiveRange; }
  void setLiveRange(const IceLiveRange &Range) { LiveRange = Range; }
  void resetLiveRange() { LiveRange.reset(); }
  void addLiveRange(int32_t Start, int32_t End, uint32_t WeightDelta) {
    assert(WeightDelta != IceRegWeight::Inf);
    LiveRange.addSegment(Start, End);
    if (Weight.isInf())
      LiveRange.setWeight(IceRegWeight::Inf);
    else
      LiveRange.addWeight(WeightDelta * Weight.getWeight());
  }
  void setLiveRangeInfiniteWeight() { LiveRange.setWeight(IceRegWeight::Inf); }

  IceVariable *getLo() const { return LoVar; }
  IceVariable *getHi() const { return HiVar; }
  void setLoHi(IceVariable *Lo, IceVariable *Hi) {
    assert(LoVar == NULL);
    assert(HiVar == NULL);
    LoVar = Lo;
    HiVar = Hi;
  }
  // Creates a temporary copy of the variable with a different type.
  // Used primarily for syntactic correctness of textual assembly
  // emission.
  IceVariable asType(IceCfg *Cfg, IceType Type);

  virtual void emit(IceOstream &Str, uint32_t Option) const;
  virtual void dump(IceOstream &Str) const;

  static bool classof(const IceOperand *Operand) {
    return Operand->getKind() == Variable;
  }

private:
  IceVariable(IceCfg *Cfg, IceType Type, const IceCfgNode *Node, uint32_t Index,
              const IceString &Name)
      : IceOperand(Cfg, Variable, Type), Number(Index), Name(Name),
        DefInst(NULL), DefNode(Node), IsArgument(false), StackOffset(0),
        RegNum(NoRegister), RegNumTmp(NoRegister), Weight(1),
        RegisterPreference(NULL), AllowRegisterOverlap(false), LoVar(NULL),
        HiVar(NULL) {
    Vars = Cfg->allocateArrayOf<IceVariable *>(1);
    Vars[0] = this;
    NumVars = 1;
  }
  // Number is unique across all variables, and is used as a
  // (bit)vector index for liveness analysis.
  const uint32_t Number;
  // Name is optional.
  const IceString Name;
  // DefInst is the instruction that produces this variable as its
  // dest.
  IceInst *DefInst;
  // DefNode is the node where this variable was produced, and is
  // reset to NULL if it is used outside that node.  This is used for
  // detecting isMultiblockLife().
  const IceCfgNode *DefNode;
  bool IsArgument;
  // StackOffset is the canonical location on stack (only if
  // RegNum<0 || IsArgument).
  int32_t StackOffset;
  // RegNum is the allocated register, or -1 if it isn't
  // register-allocated.
  int32_t RegNum;
  // RegNumTmp is the tentative assignment during register allocation.
  int32_t RegNumTmp;
  IceRegWeight Weight; // Register allocation priority
  // RegisterPreference says that if possible, the register allocator
  // should prefer the register that was assigned to this linked
  // variable.  It also allows a spill slot to share its stack
  // location with another variable, if that variable does not get
  // register-allocated and therefore has a stack location.
  IceVariable *RegisterPreference;
  // AllowRegisterOverlap says that it is OK to honor
  // RegisterPreference and "share" a register even if the two live
  // ranges overlap.
  bool AllowRegisterOverlap;
  IceLiveRange LiveRange;
  // LoVar and HiVar are needed for lowering from 64 to 32 bits.  When
  // lowering from I64 to I32 on a 32-bit architecture, we split the
  // variable into two machine-size pieces.  LoVar is the low-order
  // machine-size portion, and HiVar is the remaining high-order
  // portion.  TODO: It's wasteful to penalize all variables on all
  // targets this way; use a sparser representation.  It's also
  // wasteful for a 64-bit target.
  IceVariable *LoVar;
  IceVariable *HiVar;
};

#endif // SUBZERO_SRC_ICEOPERAND_H
