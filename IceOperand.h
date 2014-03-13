// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef _IceOperand_h
#define _IceOperand_h

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
  OperandKind getKind(void) const { return Kind; }
  IceType getType(void) const { return Type; }

  // Every IceOperand keeps an array of the IceVariables referenced in
  // the operand.  This is so that the liveness operations can get
  // quick access to the variables of interest, without having to dig
  // so far into the operand.
  uint32_t getNumVars(void) const { return NumVars; }
  IceVariable *getVar(uint32_t I) const {
    assert(I < getNumVars());
    return Vars[I];
  }
  virtual void setUse(const IceInst *Inst, const IceCfgNode *Node) {}
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
    return new IceConstantPrimitive(Cfg, Type, Value);
  }
  T getValue(void) const { return Value; }
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
    return new IceConstantRelocatable(Cfg, Type, Handle, Offset, Name, CPIndex);
  }
  uint32_t getCPIndex(void) const { return CPIndex; }
  const void *getHandle(void) const { return Handle; }
  int64_t getOffset(void) const { return Offset; }
  IceString getName(void) const { return Name; }
  void setSuppressMangling(bool Value) { SuppressMangling = Value; }
  bool getSuppressMangling(void) const { return SuppressMangling; }
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

// IceVariable represents an operand that is register-allocated or
// stack-allocated.  If it is register-allocated, it will ultimately
// have a non-negative RegNum field.
class IceVariable : public IceOperand {
public:
  static IceVariable *create(IceCfg *Cfg, IceType Type, const IceCfgNode *Node,
                             uint32_t Index, const IceString &Name) {
    return new IceVariable(Cfg, Type, Node, Index, Name);
  }

  uint32_t getIndex(void) const { return Number; }
  IceString getName(void) const;

  IceInst *getDefinition(void) const { return DefInst; }
  void setDefinition(IceInst *Inst, const IceCfgNode *Node);
  void replaceDefinition(IceInst *Inst, const IceCfgNode *Node);

  const IceCfgNode *getLocalUseNode() const { return DefNode; }
  bool isMultiblockLife(void) const { return (DefNode == NULL); }
  void setUse(const IceInst *Inst, const IceCfgNode *Node);

  bool getIsArg(void) const { return IsArgument; }
  void setIsArg(IceCfg *Cfg);

  virtual void dump(IceOstream &Str) const;

  static bool classof(const IceOperand *Operand) {
    return Operand->getKind() == Variable;
  }

private:
  IceVariable(IceCfg *Cfg, IceType Type, const IceCfgNode *Node, uint32_t Index,
              const IceString &Name)
      : IceOperand(Cfg, Variable, Type), Number(Index), Name(Name),
        DefInst(NULL), DefNode(Node), IsArgument(false) {
    Vars = new IceVariable *[1];
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
  // DefOrUseNode is the node where this variable was produced, and is
  // reset to NULL if it is used outside that node.  This is used for
  // detecting isMultiblockLife().
  const IceCfgNode *DefNode;
  bool IsArgument;
};

#endif // _IceOperand_h
