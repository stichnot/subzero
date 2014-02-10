/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInst.h"
#include "IceInstX8632.h"
#include "IceOperand.h"
#include "IceRegManager.h"

IceString IceTargetX8632::RegNames[] = { "eax", "ecx", "edx", "ebx",
                                         "esp", "ebp", "esi", "edi", };

const char *OpcodeTypeFromIceType(IceType type) {
  switch (type) {
  default:
    return "U";
  case IceType_i8:
    return "b";
  case IceType_i16:
    return "w";
  case IceType_i32:
    return "l";
  case IceType_i64:
    return "q";
  }
}

llvm::SmallBitVector IceTargetX8632::getRegisterSet(RegSetMask Include,
                                                    RegSetMask Exclude) const {
  // TODO: implement Include/Exclude logic.
  llvm::SmallBitVector Mask(Reg_NUM);
  Mask[Reg_eax] = true;
  Mask[Reg_ecx] = true;
  Mask[Reg_edx] = true;
  Mask[Reg_ebx] = true;
  Mask[Reg_ebp] = true;
  Mask[Reg_esi] = true;
  Mask[Reg_edi] = true;
  return Mask;
}

IceRegManager *IceTargetX8632::makeRegManager(IceCfgNode *Node) {
  const unsigned NumScratchReg = 3; // eax, ecx, edx
  // TODO: Optimize for extended basic blocks.
  RegManager = new IceRegManager(Cfg, Node, NumScratchReg);
  return RegManager;
}

IceInstTarget *IceTargetX8632::makeAssign(IceVariable *Dest, IceOperand *Src) {
  assert(Dest->getRegNum() >= 0);
  return new IceInstX8632Mov(Cfg, Dest, Src);
}

IceVariable *IceTargetX8632::getPhysicalRegister(unsigned RegNum) {
  assert(RegNum < PhysicalRegisters.size());
  IceVariable *Reg = PhysicalRegisters[RegNum];
  if (Reg == NULL) {
    Reg = Cfg->makeVariable(IceType_i32);
    Reg->setRegNum(RegNum);
    PhysicalRegisters[RegNum] = Reg;
  }
  return Reg;
}

void IceTargetX8632::addProlog(IceCfgNode *Node) {
  IceInstList Expansion;
  int InArgsSizeBytes = 0;
  int RetIpSizeBytes = 4;
  int PreservedRegsSizeBytes = 0;
  int LocalsSizeBytes = 0;

  // Determine stack frame offsets for each IceVariable without a
  // register assignment.  This can be done as one variable per stack
  // slot.  Or, do coalescing by running the register allocator again
  // with an infinite set of registers (as a side effect, this gives
  // variables a second chance at physical register assignment).

  llvm::SmallBitVector CalleeSaves =
      getRegisterSet(IceTargetLowering::RegMask_CalleeSave);

  // Prepass.  Compute RegsUsed, PreservedRegsSizeBytes, and
  // LocalsSizeBytes.
  RegsUsed = llvm::SmallBitVector(CalleeSaves.size());
  const IceVarList &Variables = Cfg->getVariables();
  const IceVarList &Args = Cfg->getArgs();
  for (IceVarList::const_iterator I = Variables.begin(), E = Variables.end();
       I != E; ++I) {
    IceVariable *Var = *I;
    if (!Var)
      continue;
    if (Var->getRegNum() >= 0) {
      RegsUsed[Var->getRegNum()] = true;
      continue;
    }
    if (Var->getIsArg())
      continue;
    if (Var->getLiveRange().isEmpty())
      continue;
    LocalsSizeBytes += 4;
  }

  // Add push instructions for preserved registers.
  for (unsigned i = 0; i < CalleeSaves.size(); ++i) {
    if (CalleeSaves[i] && RegsUsed[i]) {
      PreservedRegsSizeBytes += 4;
      Expansion.push_back(new IceInstX8632Push(Cfg, getPhysicalRegister(i)));
    }
  }

  // Generate "push ebp; mov ebp, esp"
  if (IsEbpBasedFrame) {
    assert((RegsUsed & getRegisterSet(IceTargetLowering::RegMask_FramePointer))
               .count() == 0);
    PreservedRegsSizeBytes += 4;
    Expansion.push_back(
        new IceInstX8632Push(Cfg, getPhysicalRegister(Reg_ebp)));
    Expansion.push_back(new IceInstX8632Mov(Cfg, getPhysicalRegister(Reg_ebp),
                                            getPhysicalRegister(Reg_esp)));
  }

  // Generate "sub esp, LocalsSizeBytes"
  if (LocalsSizeBytes)
    Expansion.push_back(
        new IceInstX8632Sub(Cfg, getPhysicalRegister(Reg_esp),
                            Cfg->getConstant(IceType_i32, LocalsSizeBytes)));

  // Fill in stack offsets for locals.
  int NextStackOffset = 0;
  for (IceVarList::const_iterator I = Variables.begin(), E = Variables.end();
       I != E; ++I) {
    IceVariable *Var = *I;
    if (!Var)
      continue;
    if (Var->getRegNum() >= 0) {
      RegsUsed[Var->getRegNum()] = true;
      continue;
    }
    if (Var->getIsArg())
      continue;
    if (Var->getLiveRange().isEmpty())
      continue;
    NextStackOffset += 4;
    if (IsEbpBasedFrame)
      Var->setStackOffset(-NextStackOffset);
    else
      Var->setStackOffset(LocalsSizeBytes - NextStackOffset);
  }
  LocalsSizeBytes = NextStackOffset;
  this->FrameSizeLocals = NextStackOffset;
  this->HasComputedFrame = true;

  // Fill in stack offsets for args, and copy args into registers for
  // those that were register-allocated.  Args are pushed right to
  // left, so Arg[0] is closest to the stack/frame pointer.
  //
  // TODO: Make this right for different width args, calling
  // conventions, etc.  For one thing, args passed in registers will
  // need to be copied/shuffled to their home registers (the
  // IceRegManager code may have some permutation logic to leverage),
  // and if they have no home register, home space will need to be
  // allocated on the stack to copy into.
  IceVariable *FramePtr = getPhysicalRegister(getFrameOrStackReg());
  for (unsigned i = 0; i < Args.size(); ++i) {
    IceVariable *Arg = Args[i];
    if (IsEbpBasedFrame)
      Arg->setStackOffset(PreservedRegsSizeBytes + RetIpSizeBytes + 4 * i);
    else
      Arg->setStackOffset(LocalsSizeBytes + PreservedRegsSizeBytes +
                          RetIpSizeBytes + 4 * i);
    if (Arg->getRegNum() >= 0) {
      IceOperandX8632Mem *Mem = new IceOperandX8632Mem(
          IceType_i32, FramePtr,
          Cfg->getConstant(IceType_i32, Arg->getStackOffset()));
      Expansion.push_back(new IceInstX8632Mov(Cfg, Arg, Mem));
    }
    InArgsSizeBytes += 4;
  }

  // TODO: If esp is adjusted during out-arg writing for a Call, any
  // accesses to stack variables need to have their esp or ebp offsets
  // adjusted accordingly.  This should be tracked by the assembler or
  // emitter.

  if (Cfg->Str.isVerbose(IceV_Frame)) {
    Cfg->Str << "LocalsSizeBytes=" << LocalsSizeBytes << "\n"
             << "PreservedRegsSizeBytes=" << PreservedRegsSizeBytes << "\n";
  }

  Node->insertInsts(Node->getInsts().begin(), Expansion);
}

void IceTargetX8632::addEpilog(IceCfgNode *Node) {
  IceInstList Expansion;
  IceInstList &Insts = Node->getInsts();
  IceInstList::reverse_iterator RI, E;
  for (RI = Insts.rbegin(), E = Insts.rend(); RI != E; ++RI) {
    if (llvm::isa<IceInstX8632Ret>(*RI))
      break;
  }
  if (RI == E)
    return;

  if (IsEbpBasedFrame) {
    // mov esp, ebp
    Expansion.push_back(new IceInstX8632Mov(Cfg, getPhysicalRegister(Reg_esp),
                                            getPhysicalRegister(Reg_ebp)));
    // pop ebp
    Expansion.push_back(new IceInstX8632Pop(Cfg, getPhysicalRegister(Reg_ebp)));
  } else {
    // add esp, FrameSizeLocals
    if (LocalsSizeBytes)
      Expansion.push_back(
          new IceInstX8632Add(Cfg, getPhysicalRegister(Reg_esp),
                              Cfg->getConstant(IceType_i32, FrameSizeLocals)));
  }

  // Add pop instructions for preserved registers.
  llvm::SmallBitVector CalleeSaves =
      getRegisterSet(IceTargetLowering::RegMask_CalleeSave);
  for (unsigned i = 0; i < CalleeSaves.size(); ++i) {
    unsigned j = CalleeSaves.size() - i - 1;
    if (j == Reg_ebp && IsEbpBasedFrame)
      continue;
    if (CalleeSaves[j] && RegsUsed[j]) {
      Expansion.push_back(new IceInstX8632Pop(Cfg, getPhysicalRegister(j)));
    }
  }

  // Convert the reverse_iterator position into its corresponding
  // (forward) iterator position.
  IceInstList::iterator InsertPoint = RI.base();
  --InsertPoint;
  Node->insertInsts(InsertPoint, Expansion);
}

IceInstList IceTargetX8632::lowerAlloca(const IceInstAlloca *Inst,
                                        const IceInst *Next,
                                        bool &DeleteNextInst) {
  IceInstList Expansion;
  IsEbpBasedFrame = true;
  assert(0); // TODO: implement
  return Expansion;
}

IceInstList IceTargetX8632::lowerArithmetic(const IceInstArithmetic *Inst,
                                            const IceInst *Next,
                                            bool &DeleteNextInst) {
  IceInstList Expansion;
  IceOpList Prefer;
  IceVarList Avoid;
  // TODO: Several instructions require specific physical
  // registers, namely div, rem, shift.  Loading into a physical
  // register requires killing all operands available in all
  // virtual registers, except that "ecx=r1" doesn't need to kill
  // operands available in r1.
  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = Inst->getSrc(0);
  IceOperand *Src1 = Inst->getSrc(1);
  IceVariable *Reg;
  IceInstTarget *NewInst;
  bool LRend0 = Inst->isLastUse(0);
  // Prefer Src0 if its live range is ending.
  if (LRend0) {
    Prefer.push_back(Src0);
  }
  if (IceVariable *Variable = llvm::dyn_cast<IceVariable>(Src1))
    Avoid.push_back(Variable);
  Reg = RegManager->getRegister(Dest->getType(), Prefer, Avoid);
  // Create "reg=Src0" if needed.
  if (!RegManager->registerContains(Reg, Src0)) {
    NewInst = new IceInstX8632Mov(Cfg, Reg, Src0);
    Expansion.push_back(NewInst);
    RegManager->notifyLoad(NewInst);
    NewInst->setRegState(RegManager);
  }
  // NewInst = new IceInstX8632Arithmetic(Cfg, Inst->getOp(), Reg, Src1);
  // TODO: Operator-specific instruction instead of Add.
  NewInst = new IceInstX8632Add(Cfg, Reg, Src1);
  Expansion.push_back(NewInst);
  RegManager->notifyLoad(NewInst, false);
  NewInst->setRegState(RegManager);
  NewInst = new IceInstX8632Mov(Cfg, Dest, Reg);
  Expansion.push_back(NewInst);
  RegManager->notifyStore(NewInst);
  NewInst->setRegState(RegManager);
  return Expansion;
}

IceInstList IceTargetX8632::lowerAssign(const IceInstAssign *Inst,
                                        const IceInst *Next,
                                        bool &DeleteNextInst) {
  IceInstList Expansion;
  IceOpList Prefer;
  IceVarList Avoid;
  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = Inst->getSrc(0);
  IceVariable *Reg;
  IceInstTarget *NewInst;
  Prefer.push_back(Src0);
  Reg = RegManager->getRegister(Dest->getType(), Prefer, Avoid);
  if (Dest->getRegNum() >= 0) {
    if (RegManager->registerContains(Reg, Src0)) {
      Src0 = Reg;
    }
    NewInst = new IceInstX8632Mov(Cfg, Dest, Src0);
    Expansion.push_back(NewInst);
    // TODO: commenting out the notifyLoad() call because Dest
    // doesn't actually belong to the current RegManager and
    // therefore it asserts.  Maybe this is the right thing to do,
    // but we also have to consider the code selection for
    // globally register allocated variables.
    // RegManager->notifyLoad(NewInst);
    NewInst->setRegState(RegManager);
  } else {
    if (!RegManager->registerContains(Reg, Src0)) {
      NewInst = new IceInstX8632Mov(Cfg, Reg, Src0);
      Expansion.push_back(NewInst);
      RegManager->notifyLoad(NewInst);
      NewInst->setRegState(RegManager);
    }
    NewInst = new IceInstX8632Mov(Cfg, Dest, Reg);
    Expansion.push_back(NewInst);
    RegManager->notifyStore(NewInst);
    NewInst->setRegState(RegManager);
  }
  return Expansion;
}

IceInstList IceTargetX8632::lowerBr(const IceInstBr *Inst, const IceInst *Next,
                                    bool &DeleteNextInst) {
  IceInstList Expansion;
  // assert(0); // TODO: implement
  return Expansion;
}

IceInstList IceTargetX8632::lowerCall(const IceInstCall *Inst,
                                      const IceInst *Next,
                                      bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement
  return Expansion;
}

IceInstList IceTargetX8632::lowerCast(const IceInstCast *Inst,
                                      const IceInst *Next,
                                      bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement
  return Expansion;
}

IceInstList IceTargetX8632::lowerFcmp(const IceInstFcmp *Inst,
                                      const IceInst *Next,
                                      bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement
  return Expansion;
}

IceInstList IceTargetX8632::lowerIcmp(const IceInstIcmp *Inst,
                                      const IceInst *Next,
                                      bool &DeleteNextInst) {
  IceInstList Expansion;
  IceInstTarget *NewInst;
  IceOpList Prefer;
  IceVarList Avoid;
  // For now, require that the following instruction is a branch
  // based on the last use of this instruction's Dest operand.
  // TODO: Fix this.
  if (llvm::isa<IceInstBr>(Next) && Inst->getDest() == Next->getSrc(0)) {
    const IceInstBr *NextBr = llvm::cast<IceInstBr>(Next);
    // This is basically identical to an Arithmetic instruction,
    // except there is no Dest variable to store.
    IceOperand *Src0 = Inst->getSrc(0);
    IceOperand *Src1 = Inst->getSrc(1);
    IceOperand *RegSrc = Src0;
    if (!llvm::isa<IceVariable>(Src0) ||
        llvm::dyn_cast<IceVariable>(Src0)->getRegNum() < 0) {
      Prefer.push_back(Src0);
      if (IceVariable *Variable = llvm::dyn_cast<IceVariable>(Src1))
        Avoid.push_back(Variable);
      IceVariable *Reg =
          RegManager->getRegister(Src0->getType(), Prefer, Avoid);
      RegSrc = Reg;
      // Create "reg=Src0" if needed.
      if (!RegManager->registerContains(Reg, Src0)) {
        if (llvm::isa<IceConstant>(Src1)) {
          RegSrc = Src0;
        } else {
          NewInst = new IceInstX8632Mov(Cfg, Reg, Src0);
          Expansion.push_back(NewInst);
          RegManager->notifyLoad(NewInst);
          NewInst->setRegState(RegManager);
        }
      }
    }
    NewInst = new IceInstX8632Icmp(Cfg, RegSrc, Src1);
    Expansion.push_back(NewInst);
    NewInst->setRegState(RegManager);
    NewInst =
        new IceInstX8632Br(Cfg, NextBr->getTargetTrue(),
                           NextBr->getTargetFalse(), Inst->getCondition());
    Expansion.push_back(NewInst);
    DeleteNextInst = true;
  } else {
    assert(0);
  }
  return Expansion;
}

IceInstList IceTargetX8632::lowerLoad(const IceInstLoad *Inst,
                                      const IceInst *Next,
                                      bool &DeleteNextInst) {
  IceInstList Expansion;
  IceInstTarget *NewInst;
  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = Inst->getSrc(0); // Base
  IceOperand *Src1 = Inst->getSrc(1); // Index - could be NULL
  IceOperand *Src2 = Inst->getSrc(2); // Shift - constant
  IceOperand *Src3 = Inst->getSrc(3); // Offset - constant
  IceOpList Prefer;
  IceVarList Avoid;
  IceVariable *Reg1 = llvm::dyn_cast<IceVariable>(Src0);
  if (Reg1 == NULL || Reg1->getRegNum() < 0) {
    Prefer.push_back(Src0);
    if (IceVariable *Variable = llvm::dyn_cast_or_null<IceVariable>(Src1))
      Avoid.push_back(Variable);
    Reg1 = RegManager->getRegister(Dest->getType(), Prefer, Avoid);
    if (!RegManager->registerContains(Reg1, Src0)) {
      NewInst = new IceInstX8632Mov(Cfg, Reg1, Src0);
      Expansion.push_back(NewInst);
      RegManager->notifyLoad(NewInst);
      NewInst->setRegState(RegManager);
    }
  }
  IceVariable *Reg2 = llvm::dyn_cast_or_null<IceVariable>(Src1);
  if (Src1 && (Reg2 == NULL || Reg2->getRegNum() < 0)) {
    Prefer.clear();
    Avoid.clear();
    Prefer.push_back(Src1);
    Avoid.push_back(Reg1);
    Reg2 = RegManager->getRegister(Dest->getType(), Prefer, Avoid);
    if (!RegManager->registerContains(Reg2, Src1)) {
      NewInst = new IceInstX8632Mov(Cfg, Reg2, Src1);
      Expansion.push_back(NewInst);
      RegManager->notifyLoad(NewInst);
      NewInst->setRegState(RegManager);
    }
  }
  IceVariable *Reg = Dest;
  if (Dest->getRegNum() < 0) {
    Prefer.clear();
    Avoid.clear();
    Avoid.push_back(Reg1);
    Avoid.push_back(Reg2);
    Reg = RegManager->getRegister(Dest->getType(), Prefer, Avoid);
  }
  NewInst = new IceInstX8632Load(Cfg, Reg, Reg1, Reg2, Src2, Src3);
  Expansion.push_back(NewInst);
  if (Reg != Dest) // TODO: clean this up
    RegManager->notifyLoad(NewInst, false);
  NewInst->setRegState(RegManager);
  if (Reg != Dest) {
    NewInst = new IceInstX8632Mov(Cfg, Dest, Reg);
    Expansion.push_back(NewInst);
    RegManager->notifyStore(NewInst);
    NewInst->setRegState(RegManager);
  }
  return Expansion;
}

IceInstList IceTargetX8632::lowerPhi(const IceInstPhi *Inst,
                                     const IceInst *Next,
                                     bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement if necessary
  return Expansion;
}

IceInstList IceTargetX8632::lowerRet(const IceInstRet *Inst,
                                     const IceInst *Next,
                                     bool &DeleteNextInst) {
  IceInstList Expansion;
  IceInstTarget *NewInst;
  IceOperand *Src0 = Inst->getSrc(0);
  IceVariable *Reg;
  if (Src0) {
    IceOpList Prefer;
    IceVarList Avoid;
    Prefer.push_back(Src0);
    Reg = RegManager->getRegister(Src0->getType(), Prefer, Avoid);
    if (!RegManager->registerContains(Reg, Src0)) {
      NewInst = new IceInstX8632Mov(Cfg, Reg, Src0);
      Expansion.push_back(NewInst);
      RegManager->notifyLoad(NewInst);
      NewInst->setRegState(RegManager);
    }
  } else
    Reg = NULL;
  NewInst = new IceInstX8632Ret(Cfg, Reg);
  Expansion.push_back(NewInst);
  return Expansion;
}

IceInstList IceTargetX8632::lowerSelect(const IceInstSelect *inst,
                                        const IceInst *Next,
                                        bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement
  return Expansion;
}

IceInstList IceTargetX8632::lowerStore(const IceInstStore *Inst,
                                       const IceInst *Next,
                                       bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement
  return Expansion;
}

IceInstList IceTargetX8632::lowerSwitch(const IceInstSwitch *Inst,
                                        const IceInst *Next,
                                        bool &DeleteNextInst) {
  IceInstList Expansion;
  assert(0); // TODO: implement
  return Expansion;
}

IceOperandX8632Mem::IceOperandX8632Mem(IceType Type, IceVariable *Base,
                                       IceConstant *Offset, IceVariable *Index,
                                       unsigned Shift)
    : IceOperandX8632(Mem, Type), Base(Base), Offset(Offset), Index(Index),
      Shift(Shift) {
  Vars = NULL;
  NumVars = 0;
  if (Base)
    ++NumVars;
  if (Index)
    ++NumVars;
  if (NumVars) {
    Vars = new IceVariable *[NumVars];
    unsigned I = 0;
    if (Base)
      Vars[I++] = Base;
    if (Index)
      Vars[I++] = Index;
    assert(I == NumVars);
  }
}

IceInstX8632Add::IceInstX8632Add(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Add, 2) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Sub::IceInstX8632Sub(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Sub, 2) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
}

IceInstX8632And::IceInstX8632And(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::And, 2) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Or::IceInstX8632Or(IceCfg *Cfg, IceVariable *Dest,
                               IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Or, 2) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Xor::IceInstX8632Xor(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Xor, 2) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Imul::IceInstX8632Imul(IceCfg *Cfg, IceVariable *Dest,
                                   IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Imul, 2) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Idiv::IceInstX8632Idiv(IceCfg *Cfg, IceVariable *Dest,
                                   IceOperand *Source, IceVariable *Other)
    : IceInstX8632(Cfg, IceInstX8632::Idiv, 3) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
  addSource(Other);
}

IceInstX8632Div::IceInstX8632Div(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source, IceVariable *Other)
    : IceInstX8632(Cfg, IceInstX8632::Div, 3) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
  addSource(Other);
}

IceInstX8632Shl::IceInstX8632Shl(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Shl, 2) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Shr::IceInstX8632Shr(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Shr, 2) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Sar::IceInstX8632Sar(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Sar, 2) {
  addDest(Dest);
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Br::IceInstX8632Br(IceCfg *Cfg, IceCfgNode *TargetTrue,
                               IceCfgNode *TargetFalse,
                               IceInstIcmp::IceICond Condition)
    : IceInstX8632(Cfg, IceInstX8632::Br, 0), Condition(Condition),
      TargetTrue(TargetTrue), TargetFalse(TargetFalse) {}

IceInstX8632Call::IceInstX8632Call(IceCfg *Cfg, IceVariable *Dest,
                                   IceOperand *CallTarget, bool Tail)
    : IceInstX8632(Cfg, IceInstX8632::Call, 0), CallTarget(CallTarget),
      Tail(Tail) {
  // TODO: CallTarget should be another source operand.
  if (Dest)
    addDest(Dest);
}

IceInstX8632Cdq::IceInstX8632Cdq(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Cdq, 1) {
  assert(Dest->getRegNum() == IceTargetX8632::Reg_edx);
  assert(llvm::isa<IceVariable>(Source));
  assert(llvm::dyn_cast<IceVariable>(Source)->getRegNum() ==
         IceTargetX8632::Reg_eax);
  addDest(Dest);
  addSource(Source);
}

IceInstX8632Icmp::IceInstX8632Icmp(IceCfg *Cfg, IceOperand *Src0,
                                   IceOperand *Src1)
    : IceInstX8632(Cfg, IceInstX8632::Icmp, 2) {
  addSource(Src0);
  addSource(Src1);
}

IceInstX8632Load::IceInstX8632Load(IceCfg *Cfg, IceVariable *Dest,
                                   IceOperand *Src0, IceOperand *Src1,
                                   IceOperand *Src2, IceOperand *Src3)
    : IceInstX8632(Cfg, IceInstX8632::Load, 4) {
  addDest(Dest);
  addSource(Src0);
  addSource(Src1);
  addSource(Src2);
  addSource(Src3);
}

IceInstX8632Store::IceInstX8632Store(IceCfg *Cfg, IceOperand *Value,
                                     IceOperandX8632Mem *Mem)
    : IceInstX8632(Cfg, IceInstX8632::Store, 2) {
  addSource(Value);
  addSource(Mem);
}

IceInstX8632Mov::IceInstX8632Mov(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Mov, 1) {
  addDest(Dest);
  addSource(Source);
}

IceInstX8632Movsx::IceInstX8632Movsx(IceCfg *Cfg, IceVariable *Dest,
                                     IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Movsx, 1) {
  addDest(Dest);
  addSource(Source);
}

IceInstX8632Movzx::IceInstX8632Movzx(IceCfg *Cfg, IceVariable *Dest,
                                     IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Movzx, 1) {
  addDest(Dest);
  addSource(Source);
}

IceInstX8632Pop::IceInstX8632Pop(IceCfg *Cfg, IceVariable *Dest)
    : IceInstX8632(Cfg, IceInstX8632::Pop, 1) {
  addDest(Dest);
}

IceInstX8632Push::IceInstX8632Push(IceCfg *Cfg, IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Push, 1) {
  addSource(Source);
}

bool IceInstX8632Mov::isRedundantAssign(void) const {
  int DestRegNum = getDest()->getRegNum();
  if (DestRegNum < 0)
    return false;
  IceVariable *Src = llvm::dyn_cast<IceVariable>(getSrc(0));
  if (Src == NULL)
    return false;
  return DestRegNum == Src->getRegNum();
}

IceInstX8632Ret::IceInstX8632Ret(IceCfg *Cfg, IceVariable *Source)
    : IceInstX8632(Cfg, IceInstX8632::Ret, Source ? 1 : 0) {
  if (Source)
    addSource(Source);
}

// ======================== Dump routines ======================== //

void IceInstX8632::dump(IceOstream &Str) const {
  Str << "[X8632] ";
  IceInst::dump(Str);
}

void IceInstX8632Br::dump(IceOstream &Str) const {
  Str << "br ";
  switch (Condition) {
  case IceInstIcmp::Eq:
    Str << "eq";
    break;
  case IceInstIcmp::Ne:
    Str << "ne";
    break;
  case IceInstIcmp::Ugt:
    Str << "ugt";
    break;
  case IceInstIcmp::Uge:
    Str << "uge";
    break;
  case IceInstIcmp::Ult:
    Str << "ult";
    break;
  case IceInstIcmp::Ule:
    Str << "ule";
    break;
  case IceInstIcmp::Sgt:
    Str << "sgt";
    break;
  case IceInstIcmp::Sge:
    Str << "sge";
    break;
  case IceInstIcmp::Slt:
    Str << "slt";
    break;
  case IceInstIcmp::Sle:
    Str << "sle";
    break;
  }
  Str << ", label %" << getTargetTrue()->getName() << ", label %"
      << getTargetFalse()->getName();
}

void IceInstX8632Call::dump(IceOstream &Str) const {
  if (getDest()) {
    dumpDest(Str);
    Str << " = ";
  }
  if (Tail)
    Str << "tail ";
  Str << "call " << getCallTarget();
}

void IceInstX8632Add::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = add." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Sub::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = sub." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632And::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = and." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Or::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = or." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Xor::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = xor." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Imul::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = imul." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Idiv::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = idiv." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Div::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = div." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Shl::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = shl." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Shr::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = shr." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Sar::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = sar." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Cdq::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = cdq." << getSrc(0)->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Icmp::dump(IceOstream &Str) const {
  Str << "cmp." << getSrc(0)->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Load::dump(IceOstream &Str) const {
  Str << "mov." << getDest()->getType() << " ";
  dumpDest(Str);
  Str << ", [";
  dumpSources(Str);
  Str << "]";
}

void IceInstX8632Store::dump(IceOstream &Str) const {
  Str << "mov." << getSrc(0)->getType() << " " << getSrc(1) << ", "
      << getSrc(0);
}

void IceInstX8632Mov::dump(IceOstream &Str) const {
  Str << "mov." << getDest()->getType() << " ";
  dumpDest(Str);
  Str << ", ";
  dumpSources(Str);
}

void IceInstX8632Movsx::dump(IceOstream &Str) const {
  Str << "movs" << OpcodeTypeFromIceType(getSrc(0)->getType());
  Str << OpcodeTypeFromIceType(getDest()->getType());
  Str << " ";
  dumpDest(Str);
  Str << ", ";
  dumpSources(Str);
}

void IceInstX8632Movzx::dump(IceOstream &Str) const {
  Str << "movz" << OpcodeTypeFromIceType(getSrc(0)->getType());
  Str << OpcodeTypeFromIceType(getDest()->getType());
  Str << " ";
  dumpDest(Str);
  Str << ", ";
  dumpSources(Str);
}

void IceInstX8632Pop::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = pop." << getDest()->getType() << " ";
}

void IceInstX8632Push::dump(IceOstream &Str) const {
  Str << "push." << getSrc(0)->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Ret::dump(IceOstream &Str) const {
  IceType Type = (getSrcSize() == 0 ? IceType_void : getSrc(0)->getType());
  Str << "ret." << Type << " ";
  dumpSources(Str);
}

void IceOperandX8632::dump(IceOstream &Str) const {
  Str << "<IceOperandX8632>";
}

void IceOperandX8632Mem::dump(IceOstream &Str) const {
  bool Dumped = false;
  Str << "[";
  if (Base) {
    Str << Base;
    Dumped = true;
  }
  if (Index) {
    assert(Base);
    Str << "+";
    if (Shift > 0)
      Str << (1u << Shift) << "*";
    Str << Index;
    Dumped = true;
  }
  // Pretty-print the Offset.
  bool OffsetIsZero = false;
  bool OffsetIsNegative = false;
  if (Offset == NULL) {
    OffsetIsZero = true;
  } else if (IceConstantInteger *CI =
                 llvm::dyn_cast<IceConstantInteger>(Offset)) {
    OffsetIsZero = (CI->getIntValue() == 0);
    OffsetIsNegative = (static_cast<int64_t>(CI->getIntValue()) < 0);
  }
  if (!OffsetIsZero) { // Suppress if Offset is known to be 0
    if (Dumped) {
      if (!OffsetIsNegative) // Suppress if Offset is known to be negative
        Str << "+";
    }
    Str << Offset;
  }
  Str << "]";
}

////////////////////////////////////////////////////////////////

llvm::SmallBitVector IceTargetX8632S::getRegisterSet(RegSetMask Include,
                                                     RegSetMask Exclude) const {
  llvm::SmallBitVector Registers(Reg_NUM);
  bool Scratch = Include & ~Exclude & RegMask_CallerSave;
  bool Preserved = Include & ~Exclude & RegMask_CalleeSave;
  Registers[Reg_eax] = Scratch;
  Registers[Reg_ecx] = Scratch;
  Registers[Reg_edx] = Scratch;
  Registers[Reg_ebx] = Preserved;
  Registers[Reg_esp] = Include & ~Exclude & RegMask_StackPointer;
  // ebp counts as both preserved and frame pointer
  Registers[Reg_ebp] = Include & (RegMask_CalleeSave | RegMask_FramePointer);
  if (Exclude & (RegMask_CalleeSave | RegMask_FramePointer))
    Registers[Reg_ebp] = false;
  Registers[Reg_esi] = Preserved;
  Registers[Reg_edi] = Preserved;
  return Registers;
}

IceInstTarget *IceTargetX8632S::makeAssign(IceVariable *Dest, IceOperand *Src) {
  assert(Dest->getRegNum() >= 0);
  return new IceInstX8632Mov(Cfg, Dest, Src);
}

IceInstList IceTargetX8632S::lowerAlloca(const IceInstAlloca *Inst,
                                         const IceInst *Next,
                                         bool &DeleteNextInst) {
  IceInstList Expansion;
  IsEbpBasedFrame = true;
  // TODO: implement
  Cfg->setError("Alloca lowering not implemented");
  return Expansion;
}

IceInstList IceTargetX8632S::lowerArithmetic(const IceInstArithmetic *Inst,
                                             const IceInst *Next,
                                             bool &DeleteNextInst) {
  IceInstList Expansion;
  /*
    +-----------+-------+----------+-------+--------+-----+--------------+
    | a = b ? c | t1?b  | t0:edx=? | t2?c  | t1?=t2 | a=? | Note         |
    |-----------+-------+----------+-------+--------+-----+--------------|
    | Add       | =     |          | :=    | add    | t1  |              |
    | Sub       | =     |          | :=    | sub    | t1  |              |
    | And       | =     |          | :=    | and    | t1  |              |
    | Or        | =     |          | :=    | or     | t1  |              |
    | Xor       | =     |          | :=    | xor    | t1  |              |
    | Mul c:imm |       |          | :=    | [note] | t1  | t1=imul b,t2 |
    | Mul       | =     |          | :=    | imul   | t1  |              |
    | Sdiv      | :eax= | cdq t1   | :=    | idiv   | t1  |              |
    | Srem      | :eax= | cdq t1   | :=    | idiv   | t0  |              |
    | Udiv      | :eax= | 0        | :=    | div    | t1  |              |
    | Urem      | :eax= | 0        | :=    | div    | t0  |              |
    | Shl       | =     |          | :ecx= | shl    | t1  |              |
    | Lshr      | =     |          | :ecx= | shr    | t1  |              |
    | Ashr      | =     |          | :ecx= | sar    | t1  |              |
    +-----------+-------+----------+-------+--------+-----+--------------+
    TODO: Fadd, Fsub, Fmul, Fdiv
  */

  // TODO: Strength-reduce multiplications by a constant, particularly
  // -1 and powers of 2.  Advanced: use lea to multiply by 3, 5, 9.

  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = Inst->getSrc(0);
  IceOperand *Src1 = Inst->getSrc(1);
  IceVariable *Reg0 = NULL;
  IceVariable *Reg1 = NULL;
  IceOperand *Reg2 = Src1;
  uint64_t Zero = 0;
  bool PrecolorReg1WithEax = false;
  bool ZeroExtendReg1 = false;
  bool SignExtendReg1 = false;
  bool Reg2InEcx = false;
  bool ResultFromReg0 = false;
  IceInstTarget *NewInst;
  switch (Inst->getOp()) {
  case IceInstArithmetic::Add:
  case IceInstArithmetic::And:
  case IceInstArithmetic::Or:
  case IceInstArithmetic::Xor:
  case IceInstArithmetic::Sub:
    break;
  case IceInstArithmetic::Mul:
    // TODO: Optimize for llvm::isa<IceConstant>(Src1)
    break;
  case IceInstArithmetic::Udiv:
    PrecolorReg1WithEax = true;
    ZeroExtendReg1 = true;
    break;
  case IceInstArithmetic::Sdiv:
    PrecolorReg1WithEax = true;
    SignExtendReg1 = true;
    break;
  case IceInstArithmetic::Urem:
    PrecolorReg1WithEax = true;
    ZeroExtendReg1 = true;
    ResultFromReg0 = true;
    break;
  case IceInstArithmetic::Srem:
    PrecolorReg1WithEax = true;
    SignExtendReg1 = true;
    ResultFromReg0 = true;
    break;
  case IceInstArithmetic::Shl:
  case IceInstArithmetic::Lshr:
  case IceInstArithmetic::Ashr:
    if (!llvm::isa<IceConstant>(Src1))
      Reg2InEcx = true;
    break;
  case IceInstArithmetic::Fadd:
  case IceInstArithmetic::Fsub:
  case IceInstArithmetic::Fmul:
  case IceInstArithmetic::Fdiv:
  case IceInstArithmetic::Frem:
    // TODO: implement
    Cfg->setError("FP arithmetic lowering not implemented");
    return Expansion;
    break;
  case IceInstArithmetic::OpKind_NUM:
    assert(0);
    break;
  }

  // Assign t1.
  assert(Dest->getType() == Src0->getType());
  Reg1 = legalizeOperandToVar(Src0, Expansion, false,
                              PrecolorReg1WithEax ? Reg_eax : -1);
  assert(Reg1);

  // Assign t0:edx.
  if (ZeroExtendReg1 || SignExtendReg1) {
    Reg0 = Cfg->makeVariable(IceType_i32);
    Reg0->setRegNum(Reg_edx);
    if (ZeroExtendReg1)
      NewInst =
          new IceInstX8632Mov(Cfg, Reg0, Cfg->getConstant(IceType_i32, Zero));
    else
      NewInst = new IceInstX8632Cdq(Cfg, Reg0, Reg1);
    Expansion.push_back(NewInst);
  }

  // Assign t2.
  if (Reg2InEcx) {
    Reg2 = legalizeOperandToVar(Src1, Expansion, false, Reg_ecx);
  }

  // Generate the arithmetic instruction.
  switch (Inst->getOp()) {
  case IceInstArithmetic::Add:
    NewInst = new IceInstX8632Add(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::And:
    NewInst = new IceInstX8632And(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Or:
    NewInst = new IceInstX8632Or(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Xor:
    NewInst = new IceInstX8632Xor(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Sub:
    NewInst = new IceInstX8632Sub(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Mul:
    // TODO: Optimized for llvm::isa<IceConstant>(Src1)
    NewInst = new IceInstX8632Imul(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Udiv:
    NewInst = new IceInstX8632Div(Cfg, Reg1, Reg2, Reg0);
    break;
  case IceInstArithmetic::Sdiv:
    NewInst = new IceInstX8632Idiv(Cfg, Reg1, Reg2, Reg0);
    break;
  case IceInstArithmetic::Urem:
    NewInst = new IceInstX8632Div(Cfg, Reg0, Reg2, Reg1);
    break;
  case IceInstArithmetic::Srem:
    NewInst = new IceInstX8632Idiv(Cfg, Reg0, Reg2, Reg1);
    break;
  case IceInstArithmetic::Shl:
    NewInst = new IceInstX8632Shl(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Lshr:
    NewInst = new IceInstX8632Shr(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Ashr:
    NewInst = new IceInstX8632Sar(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Fadd:
  case IceInstArithmetic::Fsub:
  case IceInstArithmetic::Fmul:
  case IceInstArithmetic::Fdiv:
  case IceInstArithmetic::Frem:
    // TODO: implement
    Cfg->setError("FP arithmetic lowering not implemented");
    return Expansion;
    break;
  case IceInstArithmetic::OpKind_NUM:
    assert(0);
    break;
  }
  Expansion.push_back(NewInst);

  // Assign Dest.
  NewInst = new IceInstX8632Mov(Cfg, Dest, (ResultFromReg0 ? Reg0 : Reg1));
  Expansion.push_back(NewInst);

  return Expansion;
}

IceInstList IceTargetX8632S::lowerAssign(const IceInstAssign *Inst,
                                         const IceInst *Next,
                                         bool &DeleteNextInst) {
  IceInstList Expansion;
  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = Inst->getSrc(0);
  IceInstTarget *NewInst;
  // a=b ==> t=b; a=t; (link t->b)
  assert(Dest->getType() == Src0->getType());
  IceOperand *Reg =
      legalizeOperand(Src0, Legal_Reg | Legal_Imm, Expansion, true);
  NewInst = new IceInstX8632Mov(Cfg, Dest, Reg);
  Expansion.push_back(NewInst);
  return Expansion;
}

IceInstList IceTargetX8632S::lowerBr(const IceInstBr *Inst, const IceInst *Next,
                                     bool &DeleteNextInst) {
  IceInstList Expansion;
  if (Inst->getTargetTrue())
    Cfg->setError("Conditional branch lowering unimplemented");
  return Expansion;
}

IceInstList IceTargetX8632S::lowerCall(const IceInstCall *Inst,
                                       const IceInst *Next,
                                       bool &DeleteNextInst) {
  // TODO: what to do about tailcalls?
  IceInstList Expansion;
  IceInstTarget *NewInst;
  // Generate a sequence of push instructions, pushing right to left,
  // keeping track of stack offsets in case a push involves a stack
  // operand and we are using an esp-based frame.
  uint32_t StackOffset = 0;
  // TODO: If for some reason the call instruction gets dead-code
  // eliminated after lowering, we would need to ensure that the
  // pre-call push instructions and the post-call esp adjustment get
  // eliminated as well.
  for (unsigned NumArgs = Inst->getSrcSize(), i = 0; i < NumArgs; ++i) {
    IceOperand *Arg = Inst->getSrc(NumArgs - i - 1);
    Arg = legalizeOperand(Arg, Legal_All, Expansion);
    assert(Arg);
    NewInst = new IceInstX8632Push(Cfg, Arg);
    // TODO: Where in the Cfg is StackOffset tracked?  It is needed
    // for instructions that push esp-based stack variables as
    // arguments.
    Expansion.push_back(NewInst);
    StackOffset += iceTypeWidth(Arg->getType());
  }
  // Generate the call instruction.  Assign its result to a temporary
  // with high register allocation weight.
  IceVariable *Dest = Inst->getDest();
  IceVariable *Reg = NULL;
  if (Dest) {
    Reg = Cfg->makeVariable(Dest->getType());
    Reg->setWeightInfinite();
    // TODO: Reg_eax is only for 32-bit integer results.  For floating
    // point, we need the appropriate FP register.  For a 64-bit
    // integer result, after the Kill instruction add a
    // "tmp:edx=FakeDef(Reg)" instruction for a call that can be
    // dead-code eliminated, and "tmp:edx=FakeDef()" for a call that
    // can't be eliminated.
    Reg->setRegNum(Reg_eax);
  }
  NewInst =
      new IceInstX8632Call(Cfg, Reg, Inst->getCallTarget(), Inst->isTail());
  Expansion.push_back(NewInst);
  IceInst *NewCall = NewInst;

  // Insert some sort of register-kill pseudo instruction.
  IceVarList KilledRegs;
  KilledRegs.push_back(Cfg->getTarget()->getPhysicalRegister(Reg_eax));
  KilledRegs.push_back(Cfg->getTarget()->getPhysicalRegister(Reg_ecx));
  KilledRegs.push_back(Cfg->getTarget()->getPhysicalRegister(Reg_edx));
  IceInst *Kill = new IceInstFakeKill(Cfg, KilledRegs, NewCall);
  Expansion.push_back(Kill);

  // Generate a FakeUse to keep the call live if necessary.
  bool HasSideEffects = true;
  // TODO: set HasSideEffects=false if it's a known intrinsic without
  // side effects.
  if (HasSideEffects && Reg) {
    IceInst *FakeUse = new IceInstFakeUse(Cfg, Reg);
    Expansion.push_back(FakeUse);
  }

  // Generate Dest=Reg assignment.
  if (Dest) {
    Dest->setPreferredRegister(Reg, false);
    NewInst = new IceInstX8632Mov(Cfg, Dest, Reg);
    Expansion.push_back(NewInst);
  }

  // Add the appropriate offset to esp.
  if (StackOffset) {
    IceVariable *Esp = Cfg->getTarget()->getPhysicalRegister(Reg_esp);
    NewInst = new IceInstX8632Add(Cfg, Esp,
                                  Cfg->getConstant(IceType_i32, StackOffset));
    Expansion.push_back(NewInst);
  }

  return Expansion;
}

IceInstList IceTargetX8632S::lowerCast(const IceInstCast *Inst,
                                       const IceInst *Next,
                                       bool &DeleteNextInst) {
  // TODO: The current expansion forces the mov[sz]x operand to be in
  // a physical register, which is overly restrictive and prevents a
  // single-instruction expansion "movsx reg, [mem]".  A better
  // expansion is:
  //
  // a = cast(b) ==> t=cast(b); a=t; (link t->b, link a->t, no overlap)
  IceInstList Expansion;
  IceInstCast::IceCastKind CastKind = Inst->getCastKind();
  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = Inst->getSrc(0);
  IceVariable *Reg;
  IceInstTarget *NewInst;
  // cast a=b ==> t=b; mov[sz]x a=t; (link t->b)
  Reg = Cfg->makeVariable(Src0->getType());
  Reg->setWeightInfinite();
  Reg->setPreferredRegister(llvm::dyn_cast<IceVariable>(Src0), true);
  NewInst = new IceInstX8632Mov(Cfg, Reg, Src0);
  Expansion.push_back(NewInst);
  switch (CastKind) {
  default:
    // TODO: implement other sorts of casts.
    Cfg->setError("Cast type not yet supported");
    return Expansion;
    break;
  case IceInstCast::Sext:
    NewInst = new IceInstX8632Movsx(Cfg, Dest, Reg);
    break;
  case IceInstCast::Zext:
    NewInst = new IceInstX8632Movzx(Cfg, Dest, Reg);
    break;
  case IceInstCast::Trunc:
    // It appears that Trunc is purely used to cast down from one integral type
    // to a smaller integral type.  In the generated code this does not seem
    // to be needed.  Treat these as vanilla moves.
    NewInst = new IceInstX8632Mov(Cfg, Dest, Reg);
    break;
  }
  Expansion.push_back(NewInst);
  return Expansion;
}

IceInstList IceTargetX8632S::lowerFcmp(const IceInstFcmp *Inst,
                                       const IceInst *Next,
                                       bool &DeleteNextInst) {
  IceInstList Expansion;
  Cfg->setError("Fcmp lowering unimplemented");
  return Expansion;
}

IceInstList IceTargetX8632S::lowerIcmp(const IceInstIcmp *Inst,
                                       const IceInst *Next,
                                       bool &DeleteNextInst) {
  IceInstList Expansion;
  IceInstTarget *NewInst;
  // For now, require that the following instruction is a branch
  // based on the last use of this instruction's Dest operand.
  // TODO: Fix this.
  if (llvm::isa<IceInstBr>(Next) && Inst->getDest() == Next->getSrc(0)) {
    const IceInstBr *NextBr = llvm::cast<IceInstBr>(Next);
    // This is basically identical to an Arithmetic instruction,
    // except there is no Dest variable to store.
    // cmp a,b ==> mov t,a; cmp t,b
    IceOperand *Src0 = Inst->getSrc(0);
    IceOperand *Src1 = Inst->getSrc(1);
    bool IsImmOrReg = false;
    if (llvm::isa<IceConstant>(Src1))
      IsImmOrReg = true;
    else if (IceVariable *Var = llvm::dyn_cast<IceVariable>(Src1)) {
      if (Var->getRegNum() >= 0)
        IsImmOrReg = true;
    }
    IceOperand *Reg = legalizeOperand(Src0, IsImmOrReg ? Legal_All : Legal_Reg,
                                      Expansion, true);
    NewInst = new IceInstX8632Icmp(Cfg, Reg, Src1);
    Expansion.push_back(NewInst);
    NewInst =
        new IceInstX8632Br(Cfg, NextBr->getTargetTrue(),
                           NextBr->getTargetFalse(), Inst->getCondition());
    Expansion.push_back(NewInst);
    DeleteNextInst = true;
  } else {
    Cfg->setError("Icmp lowering without subsequent Br unimplemented");
  }
  return Expansion;
}

static bool isAssign(const IceInst *Inst) {
  if (Inst == NULL)
    return false;
  if (llvm::isa<IceInstAssign>(Inst))
    return true;
  return false;
}

static bool isAdd(const IceInst *Inst) {
  if (Inst == NULL)
    return false;
  if (const IceInstArithmetic *Arith =
          llvm::dyn_cast<const IceInstArithmetic>(Inst)) {
    return (Arith->getOp() == IceInstArithmetic::Add);
  }
  return false;
}

static void computeAddressOpt(IceCfg *Cfg, IceVariable *&Base,
                              IceVariable *&Index, int &Shift,
                              int32_t &Offset) {
  if (Base == NULL)
    return;
  // If the Base has more than one use or is live across multiple
  // blocks, then don't go further.  Alternatively (?), never consider
  // a transformation that would change a variable that is currently
  // *not* live across basic block boundaries into one that *is*.
  if (Base->isMultiblockLife() /* || Base->getUseCount() > 1*/)
    return;

  while (true) {
    // Base is Base=Var ==>
    //   set Base=Var
    const IceInst *BaseInst = Base->getDefinition();
    IceOperand *BaseOperand0 = BaseInst ? BaseInst->getSrc(0) : NULL;
    IceVariable *BaseVariable0 =
        llvm::dyn_cast_or_null<IceVariable>(BaseOperand0);
    if (isAssign(BaseInst) && BaseVariable0 &&
        // TODO: ensure BaseVariable0 stays single-BB
        true) {
      Base = BaseVariable0;

      continue;
    }

    // Index is Index=Var ==>
    //   set Index=Var

    // Index==NULL && Base is Base=Var1+Var2 ==>
    //   set Base=Var1, Index=Var2, Shift=0
    IceOperand *BaseOperand1 = BaseInst ? BaseInst->getSrc(1) : NULL;
    IceVariable *BaseVariable1 =
        llvm::dyn_cast_or_null<IceVariable>(BaseOperand1);
    if (Index == NULL && isAdd(BaseInst) && BaseVariable0 && BaseVariable1 &&
        // TODO: ensure BaseVariable0 and BaseVariable1 stay single-BB
        true) {
      Base = BaseVariable0;
      Index = BaseVariable1;
      Shift = 0; // should already have been 0
      continue;
    }

    // Index is Index=Var*Const && log2(Const)+Shift<=3 ==>
    //   Index=Var, Shift+=log2(Const)
    const IceInst *IndexInst = Index ? Index->getDefinition() : NULL;
    IceOperand *IndexOperand0 = IndexInst ? IndexInst->getSrc(0) : NULL;
    IceVariable *IndexVariable0 =
        llvm::dyn_cast_or_null<IceVariable>(IndexOperand0);
    IceOperand *IndexOperand1 = IndexInst ? IndexInst->getSrc(1) : NULL;
    IceConstantInteger *IndexConstant1 =
        llvm::dyn_cast_or_null<IceConstantInteger>(IndexOperand1);
    if (IndexInst && llvm::isa<IceInstArithmetic>(IndexInst) &&
        (llvm::cast<IceInstArithmetic>(IndexInst)->getOp() ==
         IceInstArithmetic::Mul) &&
        IndexVariable0 && IndexOperand1->getType() == IceType_i32 &&
        IndexConstant1) {
      uint32_t Mult = IndexConstant1->getIntValue();
      uint32_t LogMult;
      switch (Mult) {
      case 1:
        LogMult = 0;
        break;
      case 2:
        LogMult = 1;
        break;
      case 4:
        LogMult = 2;
        break;
      case 8:
        LogMult = 3;
        break;
      default:
        LogMult = 4;
        break;
      }
      if (Shift + LogMult <= 3) {
        Index = IndexVariable0;
        Shift += LogMult;
        continue;
      }
    }

    // Index is Index=Var<<Const && Const+Shift<=3 ==>
    //   Index=Var, Shift+=Const

    // Index is Index=Const*Var && log2(Const)+Shift<=3 ==>
    //   Index=Var, Shift+=log2(Const)

    // Index && Shift==0 && Base is Base=Var*Const && log2(Const)+Shift<=3 ==>
    //   swap(Index,Base)
    // Similar for Base=Const*Var and Base=Var<<Const

    // Base is Base=Var+Const ==>
    //   set Base=Var, Offset+=Const

    // Base is Base=Const+Var ==>
    //   set Base=Var, Offset+=Const

    // Base is Base=Var-Const ==>
    //   set Base=Var, Offset-=Const

    // Index is Index=Var+Const ==>
    //   set Index=Var, Offset+=(Const<<Shift)

    // Index is Index=Const+Var ==>
    //   set Index=Var, Offset+=(Const<<Shift)

    // Index is Index=Var-Const ==>
    //   set Index=Var, Offset-=(Const<<Shift)

    // TODO: consider overflow issues with respect to Offset.
    // TODO: handle symbolic constants.
    break;
  }
}

IceInstList IceTargetX8632S::lowerLoad(const IceInstLoad *Inst,
                                       const IceInst *Next,
                                       bool &DeleteNextInst) {
  // A Load instruction can be treated the same as an Assign
  // instruction, after the source operand is transformed into an
  // IceOperandX8632Mem operand.  Note that the address mode
  // optimization already creates an IceOperandX8632Mem operand, so it
  // doesn't need another level of transformation.
  IceType Type = Inst->getDest()->getType();
  IceOperand *Src = Inst->getSrc(0);
  if (!llvm::isa<IceOperandX8632Mem>(Src)) {
    IceVariable *Base = llvm::dyn_cast<IceVariable>(Src);
    IceConstant *Offset = llvm::dyn_cast<IceConstant>(Src);
    assert(Base || Offset);
    Src = new IceOperandX8632Mem(Type, Base, Offset);
  }
  // TODO: This instruction leaks.
  IceInstAssign *Assign = new IceInstAssign(Cfg, Inst->getDest(), Src);
  return lowerAssign(Assign, Next, DeleteNextInst);
}

IceInstList IceTargetX8632S::doAddressOptLoad(const IceInstLoad *Inst) {
  IceInstList Expansion;
  IceInst *NewInst;
  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = Inst->getSrc(0);
  IceVariable *Index = NULL;
  int Shift = 0;
  int32_t Offset = 0; // TODO: make IceConstant
  IceVariable *Base = llvm::dyn_cast<IceVariable>(Src0);
  computeAddressOpt(Cfg, Base, Index, Shift, Offset);
  if (Base && Src0 != Base) {
    IceConstant *OffsetOp = Cfg->getConstant(IceType_i32, Offset);
    Src0 =
        new IceOperandX8632Mem(Dest->getType(), Base, OffsetOp, Index, Shift);
    NewInst = new IceInstLoad(Cfg, Dest, Src0);
    Expansion.push_back(NewInst);
  }
  return Expansion;
}

IceInstList IceTargetX8632S::lowerPhi(const IceInstPhi *Inst,
                                      const IceInst *Next,
                                      bool &DeleteNextInst) {
  IceInstList Expansion;
  Cfg->setError("Phi lowering not implemented");
  return Expansion;
}

IceInstList IceTargetX8632S::lowerRet(const IceInstRet *Inst,
                                      const IceInst *Next,
                                      bool &DeleteNextInst) {
  IceInstList Expansion;
  IceInstTarget *NewInst;
  IceOperand *Src0 = Inst->getSrc(0);
  IceVariable *Reg = NULL;
  if (Src0) {
    Reg = legalizeOperandToVar(Src0, Expansion, false, Reg_eax);
  }
  NewInst = new IceInstX8632Ret(Cfg, Reg);
  Expansion.push_back(NewInst);
  // Add a fake use of esp to make sure esp stays alive for the entire
  // function.  Otherwise post-call esp adjustments get dead-code
  // eliminated.  TODO: Are there more places where the fake use
  // should be inserted?  E.g. "void f(int n){while(1) g(n);}" may not
  // have a ret instruction.
  IceVariable *Esp = Cfg->getTarget()->getPhysicalRegister(Reg_esp);
  IceInst *FakeUse = new IceInstFakeUse(Cfg, Esp);
  Expansion.push_back(FakeUse);
  return Expansion;
}

IceInstList IceTargetX8632S::lowerSelect(const IceInstSelect *Inst,
                                         const IceInst *Next,
                                         bool &DeleteNextInst) {
  IceInstList Expansion;
  Cfg->setError("Select lowering not implemented");
  return Expansion;
}

IceInstList IceTargetX8632S::lowerStore(const IceInstStore *Inst,
                                        const IceInst *Next,
                                        bool &DeleteNextInst) {
  IceInstList Expansion;

  IceInstTarget *NewInst;
  IceOperand *Value = Inst->getSrc(0);
  IceOperand *Src1 = Inst->getSrc(1); // Base
  IceOperandX8632Mem *NewSrc;
  if (!(NewSrc = llvm::dyn_cast<IceOperandX8632Mem>(Src1))) {
    IceVariable *Base = llvm::dyn_cast<IceVariable>(Src1);
    IceConstant *Offset = llvm::dyn_cast<IceConstant>(Src1);
    assert(Base || Offset);
    NewSrc = new IceOperandX8632Mem(Src1->getType(), Base, Offset);
  }
  NewSrc = llvm::cast<IceOperandX8632Mem>(
      legalizeOperand(NewSrc, Legal_All, Expansion));
  IceOperand *Reg0 =
      legalizeOperand(Value, Legal_Reg | Legal_Imm, Expansion, true);

  NewInst = new IceInstX8632Store(Cfg, Reg0, NewSrc);
  Expansion.push_back(NewInst);

  return Expansion;
}

IceInstList IceTargetX8632S::doAddressOptStore(const IceInstStore *Inst) {
  return IceInstList();
  IceInstList Expansion;
  IceInst *NewInst;
  IceOperand *Src0 = Inst->getSrc(0);
  IceOperand *Src1 = Inst->getSrc(1);
  IceVariable *Index = NULL;
  int Shift = 0;
  int32_t Offset = 0; // TODO: make IceConstant
  IceVariable *Base = llvm::dyn_cast<IceVariable>(Src1);
  computeAddressOpt(Cfg, Base, Index, Shift, Offset);
  if (Src1 != Base) {
    IceConstant *OffsetOp = Cfg->getConstant(IceType_i32, Offset);
    Src0 =
        new IceOperandX8632Mem(Src0->getType(), Base, OffsetOp, Index, Shift);
    NewInst = new IceInstStore(Cfg, Src1, Src0);
    Expansion.push_back(NewInst);
  }
  return Expansion;
}

IceInstList IceTargetX8632S::lowerSwitch(const IceInstSwitch *Inst,
                                         const IceInst *Next,
                                         bool &DeleteNextInst) {
  IceInstList Expansion;
  Cfg->setError("Switch lowering not implemented");
  return Expansion;
}

IceOperand *IceTargetX8632S::legalizeOperand(IceOperand *From,
                                             LegalMask Allowed,
                                             IceInstList &Insts,
                                             bool AllowOverlap, int RegNum) {
  IceInst *NewInst;
  assert(Allowed & Legal_Reg);
  assert(RegNum < 0 || Allowed == Legal_Reg);
  if (IceOperandX8632Mem *Mem = llvm::dyn_cast<IceOperandX8632Mem>(From)) {
    // From = lowerMemOp(Insts, Mem);
    IceVariable *Base = Mem->getBase();
    IceVariable *Index = Mem->getIndex();
    IceVariable *RegBase = Base;
    IceVariable *RegIndex = Index;
    if (Base) {
      RegBase = legalizeOperandToVar(Base, Insts, true);
    }
    if (Index) {
      RegIndex = legalizeOperandToVar(Index, Insts, true);
    }
    if (Base != RegBase || Index != RegIndex) {
      From = new IceOperandX8632Mem(Mem->getType(), RegBase, Mem->getOffset(),
                                    RegIndex, Mem->getShift());
    }

    if (!(Allowed & Legal_Mem)) {
      IceVariable *Reg = Cfg->makeVariable(From->getType());
      if (RegNum < 0) {
        Reg->setWeightInfinite();
      } else {
        Reg->setRegNum(RegNum);
      }
      NewInst = new IceInstX8632Mov(Cfg, Reg, From);
      Insts.push_back(NewInst);
      From = Reg;
    }
    return From;
  }
  if (IceConstant *Const = llvm::dyn_cast<IceConstant>(From)) {
    assert(Const); // avoid "unused Const" compiler warning
    if (!(Allowed & Legal_Imm)) {
      IceVariable *Reg = Cfg->makeVariable(From->getType());
      if (RegNum < 0) {
        Reg->setWeightInfinite();
        // Reg->setPreferredRegister(Var, AllowOverlap);
      } else {
        Reg->setRegNum(RegNum);
      }
      NewInst = new IceInstX8632Mov(Cfg, Reg, From);
      Insts.push_back(NewInst);
      From = Reg;
    }
    return From;
  }
  if (IceVariable *Var = llvm::dyn_cast<IceVariable>(From)) {
    int CurRegNum = Var->getRegNum();
    // We need a new physical register for the operand if:
    //   Mem is not allowed and CurRegNum is unknown, or
    //   RegNum is required and CurRegNum doesn't match.
    if ((!(Allowed & Legal_Mem) && CurRegNum < 0) ||
        (RegNum >= 0 && RegNum != CurRegNum)) {
      IceVariable *Reg = Cfg->makeVariable(From->getType());
      if (RegNum < 0) {
        Reg->setWeightInfinite();
        Reg->setPreferredRegister(Var, AllowOverlap);
      } else {
        Reg->setRegNum(RegNum);
      }
      NewInst = new IceInstX8632Mov(Cfg, Reg, From);
      Insts.push_back(NewInst);
      From = Reg;
    }
    return From;
  }
  assert(0);
  return From;
}
