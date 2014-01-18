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
    Conversion,
    Fcmp,
    Icmp,
    Load,
    Phi,
    Ret,
    Select,
    Store,
    Switch,
    Target // target-specific low-level ICE
  };
  int getNumber(void) const { return Number; }
  void renumber(IceCfg *Cfg);
  IceInstType getKind(void) const { return Kind; }
  IceVariable *getDest(unsigned I) const {
    return I < Dests.size() ? Dests[I] : NULL;
  }
  IceOperand *getSrc(unsigned I) const {
    return I < Srcs.size() ? Srcs[I] : NULL;
  }
  unsigned getDestSize(void) const { return Dests.size(); }
  unsigned getSrcSize(void) const { return Srcs.size(); }
  bool isDeleted(void) const { return Deleted; }
  bool isLastUse(unsigned SrcIndex) const { return LiveRangesEnded[SrcIndex]; }
  // If an instruction is deleted as a result of replacing it with
  // equivalent instructions, only call setDeleted() *after* inserting
  // the new instructions because of the cascading deletes from
  // reference counting.
  void setDeleted(void);
  void deleteIfDead(void);
  void updateVars(IceCfgNode *Node);
  void findAddressOpt(IceCfg *Cfg, const IceCfgNode *Node);
  void doAddressOpt(IceVariable *&Base, IceVariable *&Index,
                    int &Shift, int32_t &Offset);
  void replaceOperands(const IceCfgNode *Node, unsigned Index,
                       const IceOpList &NewOperands);
  virtual void removeUse(IceVariable *Variable);
  void liveness(IceLiveness Mode, llvm::BitVector &Live,
                std::vector<int> &LiveBegin, std::vector<int> &LiveEnd);
  virtual void dump(IceOstream &Str) const;
  virtual void dumpExtras(IceOstream &Str) const;
  void dumpSources(IceOstream &Str) const;
  void dumpDests(IceOstream &Str) const;
protected:
  IceInst(IceCfg *Cfg, IceInstType Kind);
  void addDest(IceVariable *Dest);
  void addSource(IceOperand *Src);
  const IceInstType Kind;
  int Number; // the instruction number for describing live ranges
  // Deleted means irrevocably deleted.
  bool Deleted;
  // Dead means pending deletion after liveness analysis converges.
  bool Dead;
  // TODO: Is there any good reason to allow multiple Dest vars?
  // Maybe x86 idiv which produces div/rem at once?  sincos intrinsic?
  IceVarList Dests;
  IceOpList Srcs;
  llvm::SmallBitVector LiveRangesEnded; // size is Srcs.size()
};

IceOstream& operator<<(IceOstream &Str, const IceInst *I);

class IceInstAlloca : public IceInst {
public:
  IceInstAlloca(IceCfg *Cfg, uint32_t Size, uint32_t Align) :
    IceInst(Cfg, Alloca), Size(Size), Align(Align) {}
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == Alloca;
  }
private:
  const uint32_t Size;
  const uint32_t Align;
};

class IceInstArithmetic : public IceInst {
public:
  enum IceArithmetic {
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
  IceInstArithmetic(IceCfg *Cfg, IceArithmetic Op, IceVariable *Dest,
                    IceOperand *Source1, IceOperand *Source2);
  IceArithmetic getOp(void) const { return Op; }
  bool isCommutative(void) const;
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == Arithmetic;
  }

private:
  const IceArithmetic Op;
};

class IceInstAssign : public IceInst {
public:
  IceInstAssign(IceCfg *Cfg, IceVariable *Dest, IceOperand *Source);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == Assign;
  }
private:
};

class IceInstBr : public IceInst {
public:
  // Conditional branch
  IceInstBr(IceCfg *Cfg, IceCfgNode *Node, IceOperand *Source,
            uint32_t LabelTrue, uint32_t LabelFalse);
  // Unconditional branch.  This kind of instruction is actually
  // redundant in ICE because unconditional branches are represented
  // as IceCfgNode out-edges, and the final decision on whether to
  // emit an unconditional branch depends on the final block layout.
  IceInstBr(IceCfg *Cfg, IceCfgNode *Node, uint32_t Label);
  uint32_t getLabelTrue(void) const;
  // Fall-through
  uint32_t getLabelFalse(void) const;
  const IceCfgNode *getNode(void) const { return Node; }
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == Br;
  }
private:
  const bool IsConditional;
  const IceCfgNode *Node; // Out-edge target list is kept here.
};

// TODO: implement
class IceInstCall : public IceInst {
public:
  IceInstCall(IceCfg *Cfg, bool Tail=false) :
    IceInst(Cfg, Call), Tail(Tail) {}
  virtual void removeUse(IceVariable *Variable) {}
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == Call;
  }
private:
  const bool Tail;
};

// TODO: implement
class IceInstConversion : public IceInst {
public:
  enum IceConvert {
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
  IceInstConversion(IceCfg *Cfg, IceConvert Conversion, IceType Type,
                    IceOperand *Dest, IceOperand *Source);
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == Conversion;
  }
private:
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
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == Fcmp;
  }
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
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == Icmp;
  }
private:
  IceICond Condition;
};

class IceInstLoad : public IceInst {
public:
  IceInstLoad(IceCfg *Cfg, IceVariable *Dest, IceOperand *SourceAddr);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == Load;
  }
private:
};

class IceInstPhi : public IceInst {
public:
  IceInstPhi(IceCfg *Cfg, IceVariable *Dest);
  void addArgument(IceOperand *Source, uint32_t Label);
  IceOperand *getArgument(uint32_t Label) const;
  IceInst *lower(IceCfg *Cfg, IceCfgNode *Node);
  // TODO: delete unused getOperandForTarget()
  IceOperand *getOperandForTarget(uint32_t Target) const;
  void livenessPhiOperand(llvm::BitVector &Live, uint32_t Target);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == Phi;
  }
private:
  IceEdgeList Labels; // corresponding to Srcs
};

class IceInstRet : public IceInst {
public:
  IceInstRet(IceCfg *Cfg, IceOperand *Source = NULL);
  virtual void dump(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == Ret;
  }
private:
};

// TODO: implement
class IceInstSelect : public IceInst {
public:
  IceInstSelect(IceCfg *Cfg, IceType Type, IceOperand *Dest, IceOperand *Condition,
                IceOperand *Source1, IceOperand *Source2);
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == Select;
  }
private:
};

// TODO: implement
// Put SourceData as Srcs[0] and SourceAddr as Srcs[1]
class IceInstStore : public IceInst {
public:
  IceInstStore(IceCfg *Cfg, IceType Type, IceOperand *SourceAddr, IceOperand *SourceData);
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == Store;
  }
private:
};

// TODO: implement
class IceInstSwitch : public IceInst {
public:
  IceInstSwitch(IceCfg *Cfg, IceType Type, IceOperand *Source,
                int32_t LabelDefault);
  void addBranch(IceType Type, IceOperand *Source, int32_t Label);
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == Switch;
  }
private:
};

class IceInstTarget : public IceInst {
public:
  void setRegState(const IceRegManager *State);
  virtual void dump(IceOstream &Str) const;
  virtual void dumpExtras(IceOstream &Str) const;
  static bool classof(const IceInst *Inst) {
    return Inst->getKind() == Target;
  }
protected:
  IceInstTarget(IceCfg *Cfg) : IceInst(Cfg, Target), RegState(NULL) {}
  const IceRegManager *RegState; // used only for debugging/dumping
private:
};

#endif // _IceInst_h
