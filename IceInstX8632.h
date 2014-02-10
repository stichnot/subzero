// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef _IceInstX8632_h
#define _IceInstX8632_h

#include "IceDefs.h"
#include "IceInst.h"
#include "IceOperand.h"

#include "IceTargetLowering.h"

class IceOperandX8632 : public IceOperand {
public:
  enum IceOperandTypeX8632 { __Start = IceOperand::Target, Mem, };
  void dump(IceOstream &Str) const;

protected:
  IceOperandX8632(IceOperandTypeX8632 Kind, IceType Type)
      : IceOperand(static_cast<OperandKind>(Kind), Type) {}
};

class IceOperandX8632Mem : public IceOperandX8632 {
public:
  IceOperandX8632Mem(IceVariable *Base, IceConstant *Offset,
                     IceVariable *Index = NULL, unsigned Shift = 0);
  IceVariable *getBase(void) const { return Base; }
  IceConstant *getOffset(void) const { return Offset; }
  IceVariable *getIndex(void) const { return Index; }
  unsigned getShift(void) const { return Shift; }
  virtual void dump(IceOstream &Str) const;

  static bool classof(const IceOperand *Operand) {
    return Operand->getKind() == static_cast<OperandKind>(Mem);
  }

private:
  IceVariable *Base;
  IceConstant *Offset;
  IceVariable *Index;
  unsigned Shift;
};

class IceTargetX8632 : public IceTargetLowering {
public:
  IceTargetX8632(IceCfg *Cfg)
      : IceTargetLowering(Cfg), IsEbpBasedFrame(false), FrameSizeLocals(0),
        PhysicalRegisters(IceVarList(Reg_NUM)) {}
  virtual IceRegManager *makeRegManager(IceCfgNode *Node);
  virtual IceInstTarget *makeAssign(IceVariable *Dest, IceOperand *Src);
  virtual IceVariable *getPhysicalRegister(unsigned RegNum);
  virtual IceString *getRegNames(void) const { return RegNames; }
  virtual llvm::SmallBitVector
  getRegisterSet(RegSetMask Include = RegMask_All,
                 RegSetMask Exclude = RegMask_None) const;
  virtual bool hasFramePointer(void) const { return IsEbpBasedFrame; }
  virtual unsigned getFrameOrStackReg(void) const {
    return IsEbpBasedFrame ? Reg_ebp : Reg_esp;
  }
  virtual void addProlog(IceCfgNode *Node);
  virtual void addEpilog(IceCfgNode *Node);
  enum Registers {
    Reg_eax = 0,
    Reg_ecx = 1,
    Reg_edx = 2,
    Reg_ebx = 3,
    Reg_esp = 4,
    Reg_ebp = 5,
    Reg_esi = 6,
    Reg_edi = 7,
    Reg_NUM = 8
  };

protected:
  virtual IceInstList lowerAlloca(const IceInstAlloca *Inst,
                                  const IceInst *Next, bool &DeleteNextInst);
  virtual IceInstList lowerArithmetic(const IceInstArithmetic *Inst,
                                      const IceInst *Next,
                                      bool &DeleteNextInst);
  virtual IceInstList lowerAssign(const IceInstAssign *Inst,
                                  const IceInst *Next, bool &DeleteNextInst);
  virtual IceInstList lowerBr(const IceInstBr *Inst, const IceInst *Next,
                              bool &DeleteNextInst);
  virtual IceInstList lowerCall(const IceInstCall *Inst, const IceInst *Next,
                                bool &DeleteNextInst);
  virtual IceInstList lowerCast(const IceInstCast *Inst, const IceInst *Next,
                                bool &DeleteNextInst);
  virtual IceInstList lowerFcmp(const IceInstFcmp *Inst, const IceInst *Next,
                                bool &DeleteNextInst);
  virtual IceInstList lowerIcmp(const IceInstIcmp *Inst, const IceInst *Next,
                                bool &DeleteNextInst);
  virtual IceInstList lowerLoad(const IceInstLoad *Inst, const IceInst *Next,
                                bool &DeleteNextInst);
  virtual IceInstList lowerPhi(const IceInstPhi *Inst, const IceInst *Next,
                               bool &DeleteNextInst);
  virtual IceInstList lowerRet(const IceInstRet *Inst, const IceInst *Next,
                               bool &DeleteNextInst);
  virtual IceInstList lowerSelect(const IceInstSelect *Inst,
                                  const IceInst *Next, bool &DeleteNextInst);
  virtual IceInstList lowerStore(const IceInstStore *Inst, const IceInst *Next,
                                 bool &DeleteNextInst);
  virtual IceInstList lowerSwitch(const IceInstSwitch *Inst,
                                  const IceInst *Next, bool &DeleteNextInst);

  bool IsEbpBasedFrame;
  int FrameSizeLocals;
  int LocalsSizeBytes;
  llvm::SmallBitVector RegsUsed;

private:
  IceVarList PhysicalRegisters;
  static IceString RegNames[];
};

class IceTargetX8632S : public IceTargetX8632 {
public:
  IceTargetX8632S(IceCfg *Cfg) : IceTargetX8632(Cfg) {}
  virtual IceRegManager *makeRegManager(IceCfgNode *Node) { return NULL; }
  virtual IceInstTarget *makeAssign(IceVariable *Dest, IceOperand *Src);
  virtual llvm::SmallBitVector
  getRegisterSet(RegSetMask Include = RegMask_All,
                 RegSetMask Exclude = RegMask_None) const;

protected:
  virtual IceInstList lowerAlloca(const IceInstAlloca *Inst,
                                  const IceInst *Next, bool &DeleteNextInst);
  virtual IceInstList lowerArithmetic(const IceInstArithmetic *Inst,
                                      const IceInst *Next,
                                      bool &DeleteNextInst);
  virtual IceInstList lowerAssign(const IceInstAssign *Inst,
                                  const IceInst *Next, bool &DeleteNextInst);
  virtual IceInstList lowerBr(const IceInstBr *Inst, const IceInst *Next,
                              bool &DeleteNextInst);
  virtual IceInstList lowerCall(const IceInstCall *Inst, const IceInst *Next,
                                bool &DeleteNextInst);
  virtual IceInstList lowerCast(const IceInstCast *Inst, const IceInst *Next,
                                bool &DeleteNextInst);
  virtual IceInstList lowerFcmp(const IceInstFcmp *Inst, const IceInst *Next,
                                bool &DeleteNextInst);
  virtual IceInstList lowerIcmp(const IceInstIcmp *Inst, const IceInst *Next,
                                bool &DeleteNextInst);
  virtual IceInstList lowerLoad(const IceInstLoad *Inst, const IceInst *Next,
                                bool &DeleteNextInst);
  virtual IceInstList lowerPhi(const IceInstPhi *Inst, const IceInst *Next,
                               bool &DeleteNextInst);
  virtual IceInstList lowerRet(const IceInstRet *Inst, const IceInst *Next,
                               bool &DeleteNextInst);
  virtual IceInstList lowerSelect(const IceInstSelect *Inst,
                                  const IceInst *Next, bool &DeleteNextInst);
  virtual IceInstList lowerStore(const IceInstStore *Inst, const IceInst *Next,
                                 bool &DeleteNextInst);
  virtual IceInstList lowerSwitch(const IceInstSwitch *Inst,
                                  const IceInst *Next, bool &DeleteNextInst);
  virtual IceInstList doAddressOptLoad(const IceInstLoad *Inst);
  virtual IceInstList doAddressOptStore(const IceInstStore *Inst);

private:
  // Lowers a memory operand and returns a new version containing just
  // infinite-weight IceVariables.
  IceOperandX8632Mem *lowerMemOp(IceInstList &Expansion,
                                 IceOperandX8632Mem *Src);
};

class IceInstX8632 : public IceInstTarget {
public:
  enum IceInstTypeX8632 {
    __Start = IceInst::Target,
    Add,
    And,
    Br,
    Call,
    Cdq,
    Div,
    Icmp,
    Idiv,
    Imul,
    Load,
    Mov,
    Movsx,
    Movzx,
    Or,
    Pop,
    Push,
    Ret,
    Sar,
    Shl,
    Shr,
    Store,
    Sub,
    Xor,
  };
  virtual void dump(IceOstream &Str) const;

protected:
  IceInstX8632(IceCfg *Cfg, IceInstTypeX8632 Kind, unsigned Maxsrcs)
      : IceInstTarget(Cfg, static_cast<IceInstType>(Kind), Maxsrcs) {}
  static bool isClassof(const IceInst *Inst, IceInstTypeX8632 MyKind) {
    return Inst->getKind() == static_cast<IceInstType>(MyKind);
  }
};

class IceInstX8632Br : public IceInstX8632 {
public:
  IceInstX8632Br(IceCfg *Cfg, IceCfgNode *TargetTrue, IceCfgNode *TargetFalse,
                 IceInstIcmp::IceICond Condition);
  IceCfgNode *getTargetTrue(void) const { return TargetTrue; }
  IceCfgNode *getTargetFalse(void) const { return TargetFalse; }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Br); }

private:
  IceInstIcmp::IceICond Condition;
  IceCfgNode *TargetTrue;
  IceCfgNode *TargetFalse;
};

class IceInstX8632Call : public IceInstX8632 {
public:
  IceInstX8632Call(IceCfg *Cfg, IceVariable *Dest, IceOperand *CallTarget,
                   bool Tail);
  IceOperand *getCallTarget(void) const { return CallTarget; }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Call); }

private:
  IceOperand *CallTarget;
  const bool Tail;
};

class IceInstX8632Add : public IceInstX8632 {
public:
  IceInstX8632Add(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Add); }
};

class IceInstX8632Sub : public IceInstX8632 {
public:
  IceInstX8632Sub(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Sub); }
};

class IceInstX8632And : public IceInstX8632 {
public:
  IceInstX8632And(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, And); }
};

class IceInstX8632Or : public IceInstX8632 {
public:
  IceInstX8632Or(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Or); }
};

class IceInstX8632Xor : public IceInstX8632 {
public:
  IceInstX8632Xor(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Xor); }
};

class IceInstX8632Imul : public IceInstX8632 {
public:
  IceInstX8632Imul(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Imul); }
};

class IceInstX8632Idiv : public IceInstX8632 {
public:
  IceInstX8632Idiv(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source,
                   IceVariable *Other);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Idiv); }
};

class IceInstX8632Div : public IceInstX8632 {
public:
  IceInstX8632Div(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source,
                  IceVariable *Other);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Div); }
};

class IceInstX8632Shl : public IceInstX8632 {
public:
  IceInstX8632Shl(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Shl); }
};

class IceInstX8632Shr : public IceInstX8632 {
public:
  IceInstX8632Shr(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Shr); }
};

class IceInstX8632Sar : public IceInstX8632 {
public:
  IceInstX8632Sar(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Sar); }
};

// Sign-extend eax into edx
class IceInstX8632Cdq : public IceInstX8632 {
public:
  IceInstX8632Cdq(IceCfg *Cfg, IceVariable *Dest, IceVariable *Source);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Cdq); }
};

class IceInstX8632Icmp : public IceInstX8632 {
public:
  IceInstX8632Icmp(IceCfg *Cfg, IceOperand *Src1, IceOperand *Src2);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Icmp); }

private:
};

// TODO: Are Load and Store really just Assigns?
class IceInstX8632Load : public IceInstX8632 {
public:
  IceInstX8632Load(IceCfg *Cfg, IceVariable *Dest, IceOperand *Base,
                   IceOperand *Index, IceOperand *Shift, IceOperand *Offset);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Load); }

private:
};

// This is essentially a "mov" instruction with an IceOperandX8632Mem
// operand instead of IceVariable as the destination.  It's important
// for liveness that there is no Dest operand.
class IceInstX8632Store : public IceInstX8632 {
public:
  IceInstX8632Store(IceCfg *Cfg, IceOperand *Value, IceOperandX8632Mem *Mem);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Store); }

private:
};

class IceInstX8632Mov : public IceInstX8632 {
public:
  IceInstX8632Mov(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
  virtual bool isRedundantAssign(void) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Mov); }

private:
};

class IceInstX8632Movsx : public IceInstX8632 {
public:
  IceInstX8632Movsx(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Movsx); }

private:
};

class IceInstX8632Movzx : public IceInstX8632 {
public:
  IceInstX8632Movzx(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Movzx); }

private:
};

class IceInstX8632Pop : public IceInstX8632 {
public:
  IceInstX8632Pop(IceCfg *Cfg, IceVariable *Dest);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Pop); }
};

class IceInstX8632Push : public IceInstX8632 {
public:
  IceInstX8632Push(IceCfg *Cfg, IceOperand *Source);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Push); }
};

class IceInstX8632Ret : public IceInstX8632 {
public:
  IceInstX8632Ret(IceCfg *Cfg, IceVariable *Source = NULL);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Ret); }

private:
};

#endif // _IceInstX8632_h
