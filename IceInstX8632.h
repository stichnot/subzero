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
  IceOperandX8632(IceCfg *Cfg, IceOperandTypeX8632 Kind, IceType Type)
      : IceOperand(Cfg, static_cast<OperandKind>(Kind), Type) {}
};

class IceOperandX8632Mem : public IceOperandX8632 {
public:
  static IceOperandX8632Mem *create(IceCfg *Cfg, IceType Type,
                                    IceVariable *Base, IceConstant *Offset,
                                    IceVariable *Index = NULL,
                                    unsigned Shift = 0) {
    return new IceOperandX8632Mem(Cfg, Type, Base, Offset, Index, Shift);
  }
  IceVariable *getBase(void) const { return Base; }
  IceConstant *getOffset(void) const { return Offset; }
  IceVariable *getIndex(void) const { return Index; }
  unsigned getShift(void) const { return Shift; }
  virtual void dump(IceOstream &Str) const;

  static bool classof(const IceOperand *Operand) {
    return Operand->getKind() == static_cast<OperandKind>(Mem);
  }

private:
  IceOperandX8632Mem(IceCfg *Cfg, IceType Type, IceVariable *Base,
                     IceConstant *Offset, IceVariable *Index, unsigned Shift);
  IceVariable *Base;
  IceConstant *Offset;
  IceVariable *Index;
  unsigned Shift;
};

class IceTargetX8632 : public IceTargetLowering {
public:
  static IceTargetX8632 *create(IceCfg *Cfg) { return new IceTargetX8632(Cfg); }
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
  IceTargetX8632(IceCfg *Cfg)
      : IceTargetLowering(Cfg), IsEbpBasedFrame(false), FrameSizeLocals(0),
        PhysicalRegisters(IceVarList(Reg_NUM)) {}
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
  static IceTargetX8632S *create(IceCfg *Cfg) {
    return new IceTargetX8632S(Cfg);
  }
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
  IceTargetX8632S(IceCfg *Cfg) : IceTargetX8632(Cfg) {}
  enum OperandLegalization {
    Legal_None = 0,
    Legal_Reg = 1 << 0,
    Legal_Imm = 1 << 1,
    Legal_Mem = 1 << 2, // includes [eax+4*ecx] as well as [esp+12]
    Legal_All = ~Legal_None
  };
  typedef uint32_t LegalMask;
  IceOperand *legalizeOperand(IceOperand *From, LegalMask Allowed,
                              IceInstList &Insts, bool AllowOverlap = false,
                              int RegNum = -1);
  IceVariable *legalizeOperandToVar(IceOperand *From, IceInstList &Insts,
                                    bool AllowOverlap = false,
                                    int RegNum = -1) {
    return llvm::cast<IceVariable>(
        legalizeOperand(From, Legal_Reg, Insts, AllowOverlap, RegNum));
  }
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
  IceInstX8632(IceCfg *Cfg, IceInstTypeX8632 Kind, unsigned Maxsrcs,
               IceVariable *Dest)
      : IceInstTarget(Cfg, static_cast<IceInstType>(Kind), Maxsrcs, Dest) {}
  static bool isClassof(const IceInst *Inst, IceInstTypeX8632 MyKind) {
    return Inst->getKind() == static_cast<IceInstType>(MyKind);
  }
};

class IceInstX8632Br : public IceInstX8632 {
public:
  static IceInstX8632Br *create(IceCfg *Cfg, IceCfgNode *TargetTrue,
                                IceCfgNode *TargetFalse,
                                IceInstIcmp::IceICond Condition) {
    return new IceInstX8632Br(Cfg, TargetTrue, TargetFalse, Condition);
  }
  IceCfgNode *getTargetTrue(void) const { return TargetTrue; }
  IceCfgNode *getTargetFalse(void) const { return TargetFalse; }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Br); }

private:
  IceInstX8632Br(IceCfg *Cfg, IceCfgNode *TargetTrue, IceCfgNode *TargetFalse,
                 IceInstIcmp::IceICond Condition);
  IceInstIcmp::IceICond Condition;
  IceCfgNode *TargetTrue;
  IceCfgNode *TargetFalse;
};

class IceInstX8632Call : public IceInstX8632 {
public:
  static IceInstX8632Call *create(IceCfg *Cfg, IceVariable *Dest,
                                  IceOperand *CallTarget, bool Tail) {
    return new IceInstX8632Call(Cfg, Dest, CallTarget, Tail);
  }
  IceOperand *getCallTarget(void) const { return CallTarget; }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Call); }

private:
  IceInstX8632Call(IceCfg *Cfg, IceVariable *Dest, IceOperand *CallTarget,
                   bool Tail);
  IceOperand *CallTarget;
  const bool Tail;
};

class IceInstX8632Add : public IceInstX8632 {
public:
  static IceInstX8632Add *create(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source) {
    return new IceInstX8632Add(Cfg, Dest, Source);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Add); }

private:
  IceInstX8632Add(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
};

class IceInstX8632Sub : public IceInstX8632 {
public:
  static IceInstX8632Sub *create(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source) {
    return new IceInstX8632Sub(Cfg, Dest, Source);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Sub); }

private:
  IceInstX8632Sub(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
};

class IceInstX8632And : public IceInstX8632 {
public:
  static IceInstX8632And *create(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source) {
    return new IceInstX8632And(Cfg, Dest, Source);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, And); }

private:
  IceInstX8632And(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
};

class IceInstX8632Or : public IceInstX8632 {
public:
  static IceInstX8632Or *create(IceCfg *Cfg, IceVariable *Dest,
                                IceOperand *Source) {
    return new IceInstX8632Or(Cfg, Dest, Source);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Or); }

private:
  IceInstX8632Or(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
};

class IceInstX8632Xor : public IceInstX8632 {
public:
  static IceInstX8632Xor *create(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source) {
    return new IceInstX8632Xor(Cfg, Dest, Source);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Xor); }

private:
  IceInstX8632Xor(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
};

class IceInstX8632Imul : public IceInstX8632 {
public:
  static IceInstX8632Imul *create(IceCfg *Cfg, IceVariable *Dest,
                                  IceOperand *Source) {
    return new IceInstX8632Imul(Cfg, Dest, Source);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Imul); }

private:
  IceInstX8632Imul(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
};

class IceInstX8632Idiv : public IceInstX8632 {
public:
  static IceInstX8632Idiv *create(IceCfg *Cfg, IceVariable *Dest,
                                  IceOperand *Source, IceVariable *Other) {
    return new IceInstX8632Idiv(Cfg, Dest, Source, Other);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Idiv); }

private:
  IceInstX8632Idiv(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source,
                   IceVariable *Other);
};
class IceInstX8632Div : public IceInstX8632 {
public:
  static IceInstX8632Div *create(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source, IceVariable *Other) {
    return new IceInstX8632Div(Cfg, Dest, Source, Other);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Div); }

private:
  IceInstX8632Div(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source,
                  IceVariable *Other);
};

class IceInstX8632Shl : public IceInstX8632 {
public:
  static IceInstX8632Shl *create(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source) {
    return new IceInstX8632Shl(Cfg, Dest, Source);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Shl); }

private:
  IceInstX8632Shl(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
};

class IceInstX8632Shr : public IceInstX8632 {
public:
  static IceInstX8632Shr *create(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source) {
    return new IceInstX8632Shr(Cfg, Dest, Source);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Shr); }

private:
  IceInstX8632Shr(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
};

class IceInstX8632Sar : public IceInstX8632 {
public:
  static IceInstX8632Sar *create(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source) {
    return new IceInstX8632Sar(Cfg, Dest, Source);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Sar); }

private:
  IceInstX8632Sar(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
};

// Sign-extend eax into edx
class IceInstX8632Cdq : public IceInstX8632 {
public:
  static IceInstX8632Cdq *create(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source) {
    return new IceInstX8632Cdq(Cfg, Dest, Source);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Cdq); }

private:
  IceInstX8632Cdq(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
};

class IceInstX8632Icmp : public IceInstX8632 {
public:
  static IceInstX8632Icmp *create(IceCfg *Cfg, IceOperand *Src1,
                                  IceOperand *Src2) {
    return new IceInstX8632Icmp(Cfg, Src1, Src2);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Icmp); }

private:
  IceInstX8632Icmp(IceCfg *Cfg, IceOperand *Src1, IceOperand *Src2);
};

// TODO: Load is basically equivalent to Assign
class IceInstX8632Load : public IceInstX8632 {
public:
  static IceInstX8632Load *create(IceCfg *Cfg, IceVariable *Dest,
                                  IceOperand *Base, IceOperand *Index,
                                  IceOperand *Shift, IceOperand *Offset) {
    return new IceInstX8632Load(Cfg, Dest, Base, Index, Shift, Offset);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Load); }

private:
  IceInstX8632Load(IceCfg *Cfg, IceVariable *Dest, IceOperand *Base,
                   IceOperand *Index, IceOperand *Shift, IceOperand *Offset);
};

// This is essentially a "mov" instruction with an IceOperandX8632Mem
// operand instead of IceVariable as the destination.  It's important
// for liveness that there is no Dest operand.
class IceInstX8632Store : public IceInstX8632 {
public:
  static IceInstX8632Store *create(IceCfg *Cfg, IceOperand *Value,
                                   IceOperandX8632Mem *Mem) {
    return new IceInstX8632Store(Cfg, Value, Mem);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Store); }

private:
  IceInstX8632Store(IceCfg *Cfg, IceOperand *Value, IceOperandX8632Mem *Mem);
};

class IceInstX8632Mov : public IceInstX8632 {
public:
  static IceInstX8632Mov *create(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source) {
    return new IceInstX8632Mov(Cfg, Dest, Source);
  }
  virtual bool isRedundantAssign(void) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Mov); }

private:
  IceInstX8632Mov(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
};

class IceInstX8632Movsx : public IceInstX8632 {
public:
  static IceInstX8632Movsx *create(IceCfg *Cfg, IceVariable *Dest,
                                   IceOperand *Source) {
    return new IceInstX8632Movsx(Cfg, Dest, Source);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Movsx); }

private:
  IceInstX8632Movsx(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
};

class IceInstX8632Movzx : public IceInstX8632 {
public:
  static IceInstX8632Movzx *create(IceCfg *Cfg, IceVariable *Dest,
                                   IceOperand *Source) {
    return new IceInstX8632Movzx(Cfg, Dest, Source);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Movzx); }

private:
  IceInstX8632Movzx(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
};

class IceInstX8632Pop : public IceInstX8632 {
public:
  static IceInstX8632Pop *create(IceCfg *Cfg, IceVariable *Dest) {
    return new IceInstX8632Pop(Cfg, Dest);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Pop); }

private:
  IceInstX8632Pop(IceCfg *Cfg, IceVariable *Dest);
};

class IceInstX8632Push : public IceInstX8632 {
public:
  static IceInstX8632Push *create(IceCfg *Cfg, IceOperand *Source) {
    return new IceInstX8632Push(Cfg, Source);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Push); }

private:
  IceInstX8632Push(IceCfg *Cfg, IceOperand *Source);
};

class IceInstX8632Ret : public IceInstX8632 {
public:
  static IceInstX8632Ret *create(IceCfg *Cfg, IceVariable *Source = NULL) {
    return new IceInstX8632Ret(Cfg, Source);
  }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return isClassof(Inst, Ret); }

private:
  IceInstX8632Ret(IceCfg *Cfg, IceVariable *Source);
};

#endif // _IceInstX8632_h
