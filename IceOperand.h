// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef _IceOperand_h
#define _IceOperand_h

#include <stdint.h>

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
  IceType getType(void) const { return Type; }
  virtual void setUse(const IceInst *Inst, const IceCfgNode *Node) {}
  virtual void removeUse(void) {}
  virtual uint32_t getUseCount(void) const { return 0; }
  // TODO: can we make getVariable() all const?
  virtual IceVariable *getVariable(void) { return NULL; }
  virtual IceConstant *getConstant(void) { return NULL; }
  virtual void dump(IceOstream &Str) const;
protected:
  IceOperand(IceType Type) : Type(Type) {}
  const IceType Type;
private:
};

IceOstream& operator<<(IceOstream &Str, const IceOperand *O);

// TODO: better design of a minimal per-module constant pool,
// including synchronized access for parallel translation.
// TODO: handle symbolic constants.
class IceConstant : public IceOperand {
public:
  IceConstant(int32_t IntValue) : IceOperand(IceType_i32) {
    Value.I32 = IntValue;
  }
  virtual IceConstant *getConstant(void) { return this; }
  uint32_t getIntValue(void) const { return Value.I32; }
  virtual void dump(IceOstream &Str) const;
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

// Stack operand, or virtual or physical register
class IceVariable : public IceOperand {
public:
  IceVariable(IceType Type, uint32_t Index) :
    IceOperand(Type), VarIndex(Index), UseCount(0), DefInst(NULL),
    DefOrUseNode(NULL), IsMultiDef(false), IsArgument(false),
    IsMultiblockLife(false), RegNum(-1) {}
  virtual void setUse(const IceInst *Inst, const IceCfgNode *Node);
  virtual void removeUse(void);
  virtual uint32_t getUseCount(void) const { return UseCount; }
  virtual IceVariable *getVariable(void) { return this; }
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
  void setRegNum(int NewRegNum) {
    assert(RegNum < 0); // shouldn't set it more than once
    RegNum = NewRegNum;
  }
  int getRegNum(void) const { return RegNum; }
  virtual void dump(IceOstream &Str) const;
private:
  // operand name for pretty-printing
  const uint32_t VarIndex;
  // decorations
  uint32_t UseCount;
  IceInst *DefInst;
  const IceCfgNode *DefOrUseNode; // for detecting IsMultiblockLife
  bool IsMultiDef;
  bool IsArgument;
  bool IsMultiblockLife;
  // Allocated register; -1 for no allocation
  int RegNum;
};

#endif // _IceOperand_h
