// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef _IceInst_h
#define _IceInst_h

#include "IceDefs.h"
#include "IceTypes.h"

class IceInst {
public:
  enum IceInstType {
    // Alphabetical order
    Alloca,
    Arithmetic,
    Assign, // not part of LLVM/PNaCl bitcode
    Br,
    Call,
    Cast,
    Fcmp,
    Icmp,
    Load,
    Phi,
    Ret,
    Select,
    Store,
    Switch,
    FakeDef,  // not part of LLVM/PNaCl bitcode
    FakeUse,  // not part of LLVM/PNaCl bitcode
    FakeKill, // not part of LLVM/PNaCl bitcode
    Target    // target-specific low-level ICE
  };
  int getNumber(void) const { return Number; }
  void renumber(IceCfg *Cfg);
  IceInstType getKind(void) const { return Kind; }
  IceVariable *getDest(void) const { return Dest; }
  IceOperand *getSrc(unsigned I) const {
    return I < Srcs.size() ? Srcs[I] : NULL;
  }
  unsigned getSrcSize(void) const { return Srcs.size(); }
  virtual IceNodeList getTerminatorEdges(void) const {
    assert(0);
    return IceNodeList();
  }
  bool isDeleted(void) const { return Deleted; }
  bool isLastUse(unsigned SrcIndex) const {
    if (SrcIndex >= 8 * sizeof(LiveRangesEnded))
      return false;
    return LiveRangesEnded & (1u << SrcIndex);
  }
  // If an instruction is deleted as a result of replacing it with
  // equivalent instructions, only call setDeleted() *after* inserting
  // the new instructions because of the cascading deletes from
  // reference counting.
  void setDeleted(void) { Deleted = true; }
  void deleteIfDead(void);
  void updateVars(IceCfgNode *Node);
  void doAddressOpt(IceVariable *&Base, IceVariable *&Index, int &Shift,
                    int32_t &Offset) const;
  void liveness(IceLiveness Mode, int InstNumber, llvm::BitVector &Live,
                std::vector<int> &LiveBegin, std::vector<int> &LiveEnd);
  virtual void dump(IceOstream &Str) const;
  virtual void dumpExtras(IceOstream &Str) const;
  void dumpSources(IceOstream &Str) const;
  void dumpDest(IceOstream &Str) const;
  // isRedundantAssign() is only used for dumping, so (for now) it's
  // OK that it's virtual.
  virtual bool isRedundantAssign(void) const { return false; }

protected:
  IceInst(IceCfg *Cfg, IceInstType Kind);
  void addDest(IceVariable *Dest);
  void addSource(IceOperand *Src);
  void setLastUse(unsigned SrcIndex) {
    if (SrcIndex < 8 * sizeof(LiveRangesEnded))
      LiveRangesEnded |= (1u << SrcIndex);
  }
  void resetLastUses(void) { LiveRangesEnded = 0; }

  const IceInstType Kind;
  int Number; // the instruction number for describing live ranges
  // Deleted means irrevocably deleted.
  bool Deleted;
  // Dead means pending deletion after liveness analysis converges.
  bool Dead;
  IceVariable *Dest;
  IceOpList Srcs;
  uint32_t LiveRangesEnded; // only first 32 src operands tracked, sorry
};

IceOstream &operator<<(IceOstream &Str, const IceInst *I);

class IceInstAlloca : public IceInst {
public:
  IceInstAlloca(IceCfg *Cfg, uint32_t Size, uint32_t Align)
      : IceInst(Cfg, IceInst::Alloca), Size(Size), Align(Align) {}
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Alloca; }

private:
  const uint32_t Size;
  const uint32_t Align;
};

class IceInstArithmetic : public IceInst {
public:
  enum OpKind {
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
    OpKind_NUM,
  };
  IceInstArithmetic(IceCfg *Cfg, OpKind Op, IceVariable *Dest,
                    IceOperand *Source1, IceOperand *Source2);
  OpKind getOp(void) const { return Op; }
  bool isCommutative(void) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == Arithmetic;
  }

private:
  const OpKind Op;
};

class IceInstAssign : public IceInst {
public:
  IceInstAssign(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Assign; }

private:
};

class IceInstBr : public IceInst {
public:
  // Conditional branch
  IceInstBr(IceCfg *Cfg, IceOperand *Source, IceCfgNode *TargetTrue,
            IceCfgNode *TargetFalse);
  // Unconditional branch
  IceInstBr(IceCfg *Cfg, IceCfgNode *Target);
  IceCfgNode *getTargetTrue(void) const { return TargetTrue; }
  IceCfgNode *getTargetFalse(void) const { return TargetFalse; }
  virtual IceNodeList getTerminatorEdges(void) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Br; }

private:
  IceCfgNode *const TargetFalse;
  IceCfgNode *const TargetTrue; // NULL if unconditional branch
};

class IceInstCall : public IceInst {
public:
  IceInstCall(IceCfg *Cfg, IceVariable *Dest, IceOperand *CallTarget,
              bool Tail = false)
      : IceInst(Cfg, IceInst::Call), CallTarget(CallTarget), Tail(Tail) {
    if (Dest)
      addDest(Dest);
  }
  void addArg(IceOperand *Arg) { addSource(Arg); }
  IceOperand *getCallTarget(void) const { return CallTarget; }
  bool isTail(void) const { return Tail; }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Call; }

private:
  IceOperand *CallTarget;
  const bool Tail;
};

// TODO: implement
class IceInstCast : public IceInst {
public:
  enum IceCastKind {
    // Ordered by http://llvm.org/docs/LangRef.html#conversion-operations
    Trunc,
    Zext,
    Sext,
    Fptrunc,
    Fpext,
    Fptoui,
    Fptosi,
    Uitofp,
    Sitofp,
    Ptrtoint,
    Inttoptr,
    Bitcast,
  };
  IceInstCast(IceCfg *Cfg, IceCastKind CastKind, IceVariable *Dest,
              IceOperand *Source);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Cast; }

private:
  IceCastKind CastKind;
};

// TODO: implement
class IceInstFcmp : public IceInst {
public:
  enum IceFCond {
    // Ordered by http://llvm.org/docs/LangRef.html#id254
    False,
    Oeq,
    Ogt,
    Oge,
    Olt,
    Ole,
    One,
    Ord,
    Ueq,
    Ugt,
    Uge,
    Ult,
    Ule,
    Une,
    Uno,
    True,
  };
  IceInstFcmp(IceCfg *Cfg, IceFCond Condition, IceType Type, IceOperand *Dest,
              IceOperand *Source1, IceOperand *Source2);
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Fcmp; }

private:
  IceFCond Condition;
};

class IceInstIcmp : public IceInst {
public:
  enum IceICond {
    // Ordered by http://llvm.org/docs/LangRef.html#id249
    Eq,
    Ne,
    Ugt,
    Uge,
    Ult,
    Ule,
    Sgt,
    Sge,
    Slt,
    Sle,
  };
  IceInstIcmp(IceCfg *Cfg, IceICond Condition, IceVariable *Dest,
              IceOperand *Source1, IceOperand *Source2);
  IceICond getCondition(void) const { return Condition; }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Icmp; }

private:
  IceICond Condition;
};

class IceInstLoad : public IceInst {
public:
  IceInstLoad(IceCfg *Cfg, IceVariable *Dest, IceOperand *SourceAddr);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Load; }

private:
};

class IceInstPhi : public IceInst {
public:
  IceInstPhi(IceCfg *Cfg, IceVariable *Dest);
  void addArgument(IceOperand *Source, IceCfgNode *Label);
  IceOperand *getArgument(IceCfgNode *Label) const;
  IceInst *lower(IceCfg *Cfg, IceCfgNode *Node);
  // TODO: delete unused getOperandForTarget()
  IceOperand *getOperandForTarget(IceCfgNode *Target) const;
  void livenessPhiOperand(llvm::BitVector &Live, IceCfgNode *Target);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Phi; }

private:
  IceNodeList Labels; // corresponding to Srcs
};

class IceInstRet : public IceInst {
public:
  IceInstRet(IceCfg *Cfg, IceOperand *Source = NULL);
  virtual IceNodeList getTerminatorEdges(void) const { return IceNodeList(); }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Ret; }

private:
};

// TODO: implement
class IceInstSelect : public IceInst {
public:
  IceInstSelect(IceCfg *Cfg, IceType Type, IceOperand *Dest,
                IceOperand *Condition, IceOperand *Source1,
                IceOperand *Source2);
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Select; }

private:
};

// SourceData as Srcs[0] and SourceAddr as Srcs[1]
class IceInstStore : public IceInst {
public:
  IceInstStore(IceCfg *Cfg, IceOperand *SourceAddr, IceOperand *SourceData);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Store; }

private:
};

// TODO: implement
class IceInstSwitch : public IceInst {
public:
  IceInstSwitch(IceCfg *Cfg, IceType Type, IceOperand *Source,
                int32_t LabelDefault);
  void addBranch(IceType Type, IceOperand *Source, int32_t Label);
  virtual IceNodeList getTerminatorEdges(void) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Switch; }

private:
};

class IceInstFakeDef : public IceInst {
public:
  IceInstFakeDef(IceCfg *Cfg, IceVariable *Dest, IceVariable *Src = NULL);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == FakeDef;
  }
};

class IceInstFakeUse : public IceInst {
public:
  IceInstFakeUse(IceCfg *Cfg, IceVariable *Src);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == FakeUse;
  }
};

class IceInstFakeKill : public IceInst {
public:
  IceInstFakeKill(IceCfg *Cfg, const IceVarList &KilledRegs,
                  const IceInst *Linked);
  const IceInst *getLinked(void) const { return Linked; }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == FakeKill;
  }

private:
  // This instruction is ignored if Linked->isDeleted() is true.
  const IceInst *Linked;
};

class IceInstTarget : public IceInst {
public:
  void setRegState(const IceRegManager *State);
  virtual void dump(IceOstream &Str) const;
  virtual void dumpExtras(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) { return Inst->getKind() == Target; }

protected:
  IceInstTarget(IceCfg *Cfg) : IceInst(Cfg, IceInst::Target), RegState(NULL) {}
  const IceRegManager *RegState; // used only for debugging/dumping
private:
};

#endif // _IceInst_h
