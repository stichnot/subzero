// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef _IceInstX8632_h
#define _IceInstX8632_h

#include "IceDefs.h"
#include "IceInst.h"

namespace IceX8632 {
  IceInstList genCode(IceCfg *Cfg, IceRegManager *RegManager,
                      const IceInst *Inst, IceInst *Next,
                      bool &DeleteCurInst, bool &DeleteNextInst);
}

// Two-address arithmetic instructions.
class IceInstX8632Arithmetic : public IceInstTarget {
public:
  enum IceX8632Arithmetic { // copied from IceInstArithmetic
    // Ordered by http://llvm.org/docs/LangRef.html#binary-operations
    Add,
    Fadd,
    Sub,
    Fsub,
    Mul,
    Fmul,
    Udiv,
    Sdiv,
    Fdiv,
    Urem,
    Srem,
    Frem,
    // Ordered by http://llvm.org/docs/LangRef.html#bitwise-binary-operations
    Shl,
    Lshr,
    Ashr,
    And,
    Or,
    Xor,
    Invalid,
  };
  IceInstX8632Arithmetic(IceX8632Arithmetic Op, IceType Type,
                         IceVariable *Dest, IceOperand *Source);
  IceX8632Arithmetic getOp(void) const { return Op; }
  bool isCommutative(void) const;
  virtual void dump(IceOstream &Str) const;
private:
  const IceX8632Arithmetic Op;
};

class IceInstX8632Br : public IceInstTarget {
public:
  IceInstX8632Br(IceInstIcmp::IceICond Condition,
                 uint32_t LabelTrue, uint32_t LabelFalse);
  virtual void dump(IceOstream &Str) const;
private:
  IceInstIcmp::IceICond Condition;
  uint32_t LabelIndexFalse; // fall-through
  uint32_t LabelIndexTrue;
};

class IceInstX8632Icmp : public IceInstTarget {
public:
  IceInstX8632Icmp(IceOperand *Src1, IceOperand *Src2);
  virtual void dump(IceOstream &Str) const;
private:
};

// TODO: Are Load and Store really just Assigns?
class IceInstX8632Load : public IceInstTarget {
public:
  IceInstX8632Load(IceType Type, IceVariable *Dest, IceOperand *Base,
                   IceOperand *Index, IceOperand *Shift, IceOperand *Offset);
  virtual void dump(IceOstream &Str) const;
private:
};

class IceInstX8632Mov : public IceInstTarget {
public:
  IceInstX8632Mov(IceType Type, IceVariable *Dest, IceOperand *Source);
  virtual void dump(IceOstream &Str) const;
private:
};

class IceInstX8632Ret : public IceInstTarget {
public:
  IceInstX8632Ret(IceType Type, IceVariable *Source);
  virtual void dump(IceOstream &Str) const;
private:
};

#endif // _IceInstX8632_h
