// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef _IceInstX8632_h
#define _IceInstX8632_h

#include "IceDefs.h"
#include "IceInst.h"

#include "IceTargetLowering.h"

class IceTargetX8632 : public IceTargetLowering {
public:
  IceTargetX8632(IceCfg *Cfg) : IceTargetLowering(Cfg) {}
  virtual IceInstTarget *makeAssign(IceVariable *Dest, IceOperand *Src);
  virtual IceString *getRegNames(void) const { return RegNames; }
protected:
  virtual IceInstList lowerAlloca(const IceInst *Inst, const IceInst *Next,
                                  bool &DeleteNextInst);
  virtual IceInstList lowerArithmetic(const IceInst *Inst, const IceInst *Next,
                                      bool &DeleteNextInst);
  virtual IceInstList lowerAssign(const IceInst *Inst, const IceInst *Next,
                                  bool &DeleteNextInst);
  virtual IceInstList lowerBr(const IceInst *Inst, const IceInst *Next,
                              bool &DeleteNextInst);
  virtual IceInstList lowerCall(const IceInst *Inst, const IceInst *Next,
                                bool &DeleteNextInst);
  virtual IceInstList lowerConversion(const IceInst *Inst, const IceInst *Next,
                                      bool &DeleteNextInst);
  virtual IceInstList lowerFcmp(const IceInst *Inst, const IceInst *Next,
                                bool &DeleteNextInst);
  virtual IceInstList lowerIcmp(const IceInst *Inst, const IceInst *Next,
                                bool &DeleteNextInst);
  virtual IceInstList lowerLoad(const IceInst *Inst, const IceInst *Next,
                                bool &DeleteNextInst);
  virtual IceInstList lowerPhi(const IceInst *Inst, const IceInst *Next,
                               bool &DeleteNextInst);
  virtual IceInstList lowerRet(const IceInst *Inst, const IceInst *Next,
                               bool &DeleteNextInst);
  virtual IceInstList lowerSelect(const IceInst *Inst, const IceInst *Next,
                                  bool &DeleteNextInst);
  virtual IceInstList lowerStore(const IceInst *Inst, const IceInst *Next,
                                 bool &DeleteNextInst);
  virtual IceInstList lowerSwitch(const IceInst *Inst, const IceInst *Next,
                                  bool &DeleteNextInst);
private:
  static IceString RegNames[];
};

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
  IceInstX8632Arithmetic(IceCfg *Cfg, IceX8632Arithmetic Op,
                         IceVariable *Dest, IceOperand *Source);
  IceX8632Arithmetic getOp(void) const { return Op; }
  bool isCommutative(void) const;
  virtual void dump(IceOstream &Str) const;
private:
  const IceX8632Arithmetic Op;
};

class IceInstX8632Br : public IceInstTarget {
public:
  IceInstX8632Br(IceCfg *Cfg, const IceCfgNode *Node, IceInstIcmp::IceICond Condition);
  uint32_t getLabelTrue(void) const;
  // Fall-through
  uint32_t getLabelFalse(void) const;
  virtual void dump(IceOstream &Str) const;
private:
  IceInstIcmp::IceICond Condition;
  const IceCfgNode *Node; // Out-edge target list is kept here.
};

class IceInstX8632Icmp : public IceInstTarget {
public:
  IceInstX8632Icmp(IceCfg *Cfg, IceOperand *Src1, IceOperand *Src2);
  virtual void dump(IceOstream &Str) const;
private:
};

// TODO: Are Load and Store really just Assigns?
class IceInstX8632Load : public IceInstTarget {
public:
  IceInstX8632Load(IceCfg *Cfg, IceVariable *Dest, IceOperand *Base,
                   IceOperand *Index, IceOperand *Shift, IceOperand *Offset);
  virtual void dump(IceOstream &Str) const;
private:
};

class IceInstX8632Mov : public IceInstTarget {
public:
  IceInstX8632Mov(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
  virtual void dump(IceOstream &Str) const;
private:
};

class IceInstX8632Ret : public IceInstTarget {
public:
  IceInstX8632Ret(IceCfg *Cfg, IceVariable *Source);
  virtual void dump(IceOstream &Str) const;
private:
};

#endif // _IceInstX8632_h
