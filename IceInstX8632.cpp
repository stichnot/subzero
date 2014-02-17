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
  RegManager = IceRegManager::create(Cfg, Node, NumScratchReg);
  return RegManager;
}

IceInstTarget *IceTargetX8632::makeAssign(IceVariable *Dest, IceOperand *Src) {
  assert(Dest->getRegNum() >= 0);
  return IceInstX8632Mov::create(Cfg, Dest, Src);
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

// Helper function for addProlog().  Sets the frame offset for Arg,
// updates InArgsSizeBytes according to Arg's width, and generates an
// instruction to copy Arg into its assigned register if applicable.
// For an I64 arg that has been split into Low and High components, it
// calls itself recursively on the components, taking care to handle
// Low first because of the little-endian architecture.
void IceTargetX8632::setArgOffsetAndCopy(IceVariable *Arg,
                                         IceVariable *FramePtr,
                                         int BasicFrameOffset,
                                         int &InArgsSizeBytes,
                                         IceInstList &Expansion) {
  IceVariable *Low = Arg->getLow();
  IceVariable *High = Arg->getHigh();
  IceType Type = Arg->getType();
  if (Low && High && Type == IceType_i64) {
    assert(Low->getType() != IceType_i64);  // don't want infinite recursion
    assert(High->getType() != IceType_i64); // don't want infinite recursion
    setArgOffsetAndCopy(Low, FramePtr, BasicFrameOffset, InArgsSizeBytes,
                        Expansion);
    setArgOffsetAndCopy(High, FramePtr, BasicFrameOffset, InArgsSizeBytes,
                        Expansion);
    return;
  }
  Arg->setStackOffset(BasicFrameOffset + InArgsSizeBytes);
  if (Arg->getRegNum() >= 0) {
    // TODO: Uncomment this assert when I64 lowering is complete.
    // assert(Type != IceType_i64);
    IceOperandX8632Mem *Mem = IceOperandX8632Mem::create(
        Cfg, Type, FramePtr,
        Cfg->getConstant(IceType_i32, Arg->getStackOffset()));
    Expansion.push_back(IceInstX8632Mov::create(Cfg, Arg, Mem));
  }
  InArgsSizeBytes += typeWidthOnStack(Type);
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
    LocalsSizeBytes += typeWidthOnStack(Var->getType());
  }

  // Add push instructions for preserved registers.
  for (unsigned i = 0; i < CalleeSaves.size(); ++i) {
    if (CalleeSaves[i] && RegsUsed[i]) {
      PreservedRegsSizeBytes += 4;
      Expansion.push_back(
          IceInstX8632Push::create(Cfg, getPhysicalRegister(i)));
    }
  }

  // Generate "push ebp; mov ebp, esp"
  if (IsEbpBasedFrame) {
    assert((RegsUsed & getRegisterSet(IceTargetLowering::RegMask_FramePointer))
               .count() == 0);
    PreservedRegsSizeBytes += 4;
    Expansion.push_back(
        IceInstX8632Push::create(Cfg, getPhysicalRegister(Reg_ebp)));
    Expansion.push_back(IceInstX8632Mov::create(
        Cfg, getPhysicalRegister(Reg_ebp), getPhysicalRegister(Reg_esp)));
  }

  // Generate "sub esp, LocalsSizeBytes"
  if (LocalsSizeBytes)
    Expansion.push_back(IceInstX8632Sub::create(
        Cfg, getPhysicalRegister(Reg_esp),
        Cfg->getConstant(IceType_i32, LocalsSizeBytes)));

  resetStackAdjustment();

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
    NextStackOffset += typeWidthOnStack(Var->getType());
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
  int BasicFrameOffset = PreservedRegsSizeBytes + RetIpSizeBytes;
  if (!IsEbpBasedFrame)
    BasicFrameOffset += LocalsSizeBytes;
  for (unsigned i = 0; i < Args.size(); ++i) {
    IceVariable *Arg = Args[i];
    setArgOffsetAndCopy(Arg, FramePtr, BasicFrameOffset, InArgsSizeBytes,
                        Expansion);
  }

  // TODO: If esp is adjusted during out-arg writing for a Call, any
  // accesses to stack variables need to have their esp or ebp offsets
  // adjusted accordingly.  This should be tracked by the assembler or
  // emitter.

  if (Cfg->Str.isVerbose(IceV_Frame)) {
    Cfg->Str << "LocalsSizeBytes=" << LocalsSizeBytes << "\n"
             << "InArgsSizeBytes=" << InArgsSizeBytes << "\n"
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
    Expansion.push_back(IceInstX8632Mov::create(
        Cfg, getPhysicalRegister(Reg_esp), getPhysicalRegister(Reg_ebp)));
    // pop ebp
    Expansion.push_back(
        IceInstX8632Pop::create(Cfg, getPhysicalRegister(Reg_ebp)));
  } else {
    // add esp, FrameSizeLocals
    if (LocalsSizeBytes)
      Expansion.push_back(IceInstX8632Add::create(
          Cfg, getPhysicalRegister(Reg_esp),
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
      Expansion.push_back(IceInstX8632Pop::create(Cfg, getPhysicalRegister(j)));
    }
  }

  // Convert the reverse_iterator position into its corresponding
  // (forward) iterator position.
  IceInstList::iterator InsertPoint = RI.base();
  --InsertPoint;
  Node->insertInsts(InsertPoint, Expansion);
}

void IceTargetX8632::split64(IceVariable *Var) {
  switch (Var->getType()) {
  default:
    return;
  case IceType_i64:
  // TODO: Only consider F64 if we need to push each half when
  // passing as an argument to a function call.  Note that each half
  // is still typed as I32.
  case IceType_f64:
    break;
  }
  IceVariable *Low = Var->getLow();
  if (Low) {
    assert(Var->getHigh());
    return;
  }
  Low = Cfg->makeVariable(IceType_i32, -1, Var->getName() + "__lo");
  Var->setLow(Low);
  assert(Var->getHigh() == NULL);
  IceVariable *High =
      Cfg->makeVariable(IceType_i32, -1, Var->getName() + "__hi");
  Var->setHigh(High);
  if (Var->getIsArg()) {
    Low->setIsArg();
    High->setIsArg();
  }
}

IceOperand *IceTargetX8632::makeLowOperand(IceOperand *Operand) {
  assert(Operand->getType() == IceType_i64);
  if (Operand->getType() != IceType_i64)
    return Operand;
  if (IceVariable *Var = llvm::dyn_cast<IceVariable>(Operand)) {
    split64(Var);
    return Var->getLow();
  }
  if (IceConstantInteger *Const = llvm::dyn_cast<IceConstantInteger>(Operand)) {
    uint64_t Mask = (1ul << 32) - 1;
    return Cfg->getConstant(IceType_i32, Const->getIntValue() & Mask);
  }
  if (IceOperandX8632Mem *Mem = llvm::dyn_cast<IceOperandX8632Mem>(Operand)) {
    return IceOperandX8632Mem::create(Cfg, IceType_i32, Mem->getBase(),
                                      Mem->getOffset(), Mem->getIndex(),
                                      Mem->getShift());
  }
  assert(0 && "Unsupported operand type");
  return NULL;
}

IceOperand *IceTargetX8632::makeHighOperand(IceOperand *Operand) {
  assert(Operand->getType() == IceType_i64);
  if (Operand->getType() != IceType_i64)
    return Operand;
  if (IceVariable *Var = llvm::dyn_cast<IceVariable>(Operand)) {
    split64(Var);
    return Var->getHigh();
  }
  if (IceConstantInteger *Const = llvm::dyn_cast<IceConstantInteger>(Operand)) {
    return Cfg->getConstant(IceType_i32, Const->getIntValue() >> 32);
  }
  if (IceOperandX8632Mem *Mem = llvm::dyn_cast<IceOperandX8632Mem>(Operand)) {
    IceConstant *Offset = Mem->getOffset();
    if (Offset == NULL)
      Offset = Cfg->getConstant(IceType_i32, 4);
    else {
      if (IceConstantInteger *IntOffset =
              llvm::dyn_cast<IceConstantInteger>(Offset)) {
        Offset = Cfg->getConstant(IceType_i32, 4 + IntOffset->getIntValue());
      } else {
        assert(0 && "Symbolic integer constant not yet supported");
      }
    }
    return IceOperandX8632Mem::create(Cfg, IceType_i32, Mem->getBase(), Offset,
                                      Mem->getIndex(), Mem->getShift());
  }
  assert(0 && "Unsupported operand type");
  return NULL;
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
  bool LRend0 = Inst->isLastUse(Src0);
  // Prefer Src0 if its live range is ending.
  if (LRend0) {
    Prefer.push_back(Src0);
  }
  if (IceVariable *Variable = llvm::dyn_cast<IceVariable>(Src1))
    Avoid.push_back(Variable);
  Reg = RegManager->getRegister(Dest->getType(), Prefer, Avoid);
  // Create "reg=Src0" if needed.
  if (!RegManager->registerContains(Reg, Src0)) {
    NewInst = IceInstX8632Mov::create(Cfg, Reg, Src0);
    Expansion.push_back(NewInst);
    RegManager->notifyLoad(NewInst);
    NewInst->setRegState(RegManager);
  }
  // NewInst = new IceInstX8632Arithmetic(Cfg, Inst->getOp(), Reg, Src1);
  // TODO: Operator-specific instruction instead of Add.
  NewInst = IceInstX8632Add::create(Cfg, Reg, Src1);
  Expansion.push_back(NewInst);
  RegManager->notifyLoad(NewInst, false);
  NewInst->setRegState(RegManager);
  NewInst = IceInstX8632Mov::create(Cfg, Dest, Reg);
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
    NewInst = IceInstX8632Mov::create(Cfg, Dest, Src0);
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
      NewInst = IceInstX8632Mov::create(Cfg, Reg, Src0);
      Expansion.push_back(NewInst);
      RegManager->notifyLoad(NewInst);
      NewInst->setRegState(RegManager);
    }
    NewInst = IceInstX8632Mov::create(Cfg, Dest, Reg);
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
          NewInst = IceInstX8632Mov::create(Cfg, Reg, Src0);
          Expansion.push_back(NewInst);
          RegManager->notifyLoad(NewInst);
          NewInst->setRegState(RegManager);
        }
      }
    }
    NewInst = IceInstX8632Icmp::create(Cfg, RegSrc, Src1);
    Expansion.push_back(NewInst);
    NewInst->setRegState(RegManager);
    NewInst =
        IceInstX8632Br::create(Cfg, NextBr->getTargetTrue(),
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
  assert(0 && "Remove support for 4-src Load instruction");
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
      NewInst = IceInstX8632Mov::create(Cfg, Reg1, Src0);
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
      NewInst = IceInstX8632Mov::create(Cfg, Reg2, Src1);
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
  NewInst = IceInstX8632Load::create(Cfg, Reg, Reg1, Reg2, Src2, Src3);
  Expansion.push_back(NewInst);
  if (Reg != Dest) // TODO: clean this up
    RegManager->notifyLoad(NewInst, false);
  NewInst->setRegState(RegManager);
  if (Reg != Dest) {
    NewInst = IceInstX8632Mov::create(Cfg, Dest, Reg);
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
      NewInst = IceInstX8632Mov::create(Cfg, Reg, Src0);
      Expansion.push_back(NewInst);
      RegManager->notifyLoad(NewInst);
      NewInst->setRegState(RegManager);
    }
  } else
    Reg = NULL;
  NewInst = IceInstX8632Ret::create(Cfg, Reg);
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

IceOperandX8632Mem::IceOperandX8632Mem(IceCfg *Cfg, IceType Type,
                                       IceVariable *Base, IceConstant *Offset,
                                       IceVariable *Index, unsigned Shift)
    : IceOperandX8632(Cfg, Mem, Type), Base(Base), Offset(Offset), Index(Index),
      Shift(Shift) {
  Vars = NULL;
  NumVars = 0;
  if (Base)
    ++NumVars;
  if (Index)
    ++NumVars;
  if (NumVars) {
    Vars = new IceVariable *[NumVars]; // TODO: use Cfg placement alloc
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
    : IceInstX8632(Cfg, IceInstX8632::Add, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Sub::IceInstX8632Sub(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Sub, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632And::IceInstX8632And(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::And, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Or::IceInstX8632Or(IceCfg *Cfg, IceVariable *Dest,
                               IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Or, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Xor::IceInstX8632Xor(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Xor, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Imul::IceInstX8632Imul(IceCfg *Cfg, IceVariable *Dest,
                                   IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Imul, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Idiv::IceInstX8632Idiv(IceCfg *Cfg, IceVariable *Dest,
                                   IceOperand *Source, IceVariable *Other)
    : IceInstX8632(Cfg, IceInstX8632::Idiv, 3, Dest) {
  addSource(Dest);
  addSource(Source);
  addSource(Other);
}

IceInstX8632Div::IceInstX8632Div(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source, IceVariable *Other)
    : IceInstX8632(Cfg, IceInstX8632::Div, 3, Dest) {
  addSource(Dest);
  addSource(Source);
  addSource(Other);
}

IceInstX8632Shl::IceInstX8632Shl(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Shl, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Shr::IceInstX8632Shr(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Shr, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Sar::IceInstX8632Sar(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Sar, 2, Dest) {
  addSource(Dest);
  addSource(Source);
}

IceInstX8632Label::IceInstX8632Label(IceCfg *Cfg, IceTargetX8632 *Target)
    : IceInstX8632(Cfg, IceInstX8632::Label, 0, NULL),
      Number(Target->makeNextLabelNumber()) {}

IceString IceInstX8632Label::getName(IceCfg *Cfg) const {
  char buf[30];
  sprintf(buf, "%u", Number);
  return ".L" + Cfg->getName() + "$__" + buf;
}

IceInstX8632Br::IceInstX8632Br(IceCfg *Cfg, IceCfgNode *TargetTrue,
                               IceCfgNode *TargetFalse,
                               IceInstX8632Label *Label,
                               IceInstIcmp::IceICond Condition)
    : IceInstX8632(Cfg, IceInstX8632::Br, 0, NULL), Condition(Condition),
      TargetTrue(TargetTrue), TargetFalse(TargetFalse), Label(Label) {}

IceInstX8632Call::IceInstX8632Call(IceCfg *Cfg, IceVariable *Dest,
                                   IceOperand *CallTarget, bool Tail)
    : IceInstX8632(Cfg, IceInstX8632::Call, 0, Dest), CallTarget(CallTarget),
      Tail(Tail) {
  // TODO: CallTarget should be another source operand.
}

IceInstX8632Cdq::IceInstX8632Cdq(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Cdq, 1, Dest) {
  assert(Dest->getRegNum() == IceTargetX8632::Reg_edx);
  assert(llvm::isa<IceVariable>(Source));
  assert(llvm::dyn_cast<IceVariable>(Source)->getRegNum() ==
         IceTargetX8632::Reg_eax);
  addSource(Source);
}

IceInstX8632Icmp::IceInstX8632Icmp(IceCfg *Cfg, IceOperand *Src0,
                                   IceOperand *Src1)
    : IceInstX8632(Cfg, IceInstX8632::Icmp, 2, NULL) {
  addSource(Src0);
  addSource(Src1);
}

IceInstX8632Load::IceInstX8632Load(IceCfg *Cfg, IceVariable *Dest,
                                   IceOperand *Src0, IceOperand *Src1,
                                   IceOperand *Src2, IceOperand *Src3)
    : IceInstX8632(Cfg, IceInstX8632::Load, 4, Dest) {
  assert(0 && "Remove support for 4-src Load instruction");
  addSource(Src0);
  addSource(Src1);
  addSource(Src2);
  addSource(Src3);
}

IceInstX8632Store::IceInstX8632Store(IceCfg *Cfg, IceOperand *Value,
                                     IceOperandX8632Mem *Mem)
    : IceInstX8632(Cfg, IceInstX8632::Store, 2, NULL) {
  addSource(Value);
  addSource(Mem);
}

IceInstX8632Mov::IceInstX8632Mov(IceCfg *Cfg, IceVariable *Dest,
                                 IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Mov, 1, Dest) {
  addSource(Source);
}

IceInstX8632Movsx::IceInstX8632Movsx(IceCfg *Cfg, IceVariable *Dest,
                                     IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Movsx, 1, Dest) {
  addSource(Source);
}

IceInstX8632Movzx::IceInstX8632Movzx(IceCfg *Cfg, IceVariable *Dest,
                                     IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Movzx, 1, Dest) {
  addSource(Source);
}

IceInstX8632Pop::IceInstX8632Pop(IceCfg *Cfg, IceVariable *Dest)
    : IceInstX8632(Cfg, IceInstX8632::Pop, 1, Dest) {}

IceInstX8632Push::IceInstX8632Push(IceCfg *Cfg, IceOperand *Source)
    : IceInstX8632(Cfg, IceInstX8632::Push, 1, NULL) {
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
    : IceInstX8632(Cfg, IceInstX8632::Ret, Source ? 1 : 0, NULL) {
  if (Source)
    addSource(Source);
}

// ======================== Dump routines ======================== //

void IceInstX8632::dump(IceOstream &Str) const {
  Str << "[X8632] ";
  IceInst::dump(Str);
}

void IceInstX8632Label::emit(IceOstream &Str, unsigned Option) const {
  dump(Str);
  Str << "\n";
}

void IceInstX8632Label::dump(IceOstream &Str) const {
  Str << getName(Str.Cfg) << ":";
}

void IceInstX8632Br::emit(IceOstream &Str, unsigned Option) const {
  Str << "\t";
  switch (Condition) {
  case IceInstIcmp::Eq:
    Str << "je";
    break;
  case IceInstIcmp::Ne:
    Str << "jne";
    break;
  case IceInstIcmp::Ugt:
    Str << "jg";
    break;
  case IceInstIcmp::Uge:
    Str << "jge";
    break;
  case IceInstIcmp::Ult:
    Str << "jl";
    break;
  case IceInstIcmp::Ule:
    Str << "jle";
    break;
  case IceInstIcmp::Sgt:
    Str << "ja";
    break;
  case IceInstIcmp::Sge:
    Str << "jae";
    break;
  case IceInstIcmp::Slt:
    Str << "jb";
    break;
  case IceInstIcmp::Sle:
    Str << "jbe";
    break;
  case IceInstIcmp::None:
    Str << "jmp";
    break;
  }
  if (Label) {
    Str << "\t" << Label->getName(Str.Cfg) << "\n";
  } else {
    if (Condition == IceInstIcmp::None) {
      Str << "\t" << getTargetFalse()->getAsmName() << "\n";
    } else {
      Str << "\t" << getTargetTrue()->getAsmName() << "\n";
      if (getTargetFalse()) {
        Str << "\tjmp\t" << getTargetFalse()->getAsmName() << "\n";
      }
    }
  }
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
  case IceInstIcmp::None:
    Str << "label %"
        << (Label ? Label->getName(Str.Cfg) : getTargetFalse()->getName());
    return;
    break;
  }
  if (Label) {
    Str << ", label %" << Label->getName(Str.Cfg);
  } else {
    Str << ", label %" << getTargetTrue()->getName();
    if (getTargetFalse()) {
      Str << ", label %" << getTargetFalse()->getName();
    }
  }
}

void IceInstX8632Call::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 0);
  Str << "\tcall\t";
  getCallTarget()->emit(Str, Option);
  if (Tail)
    Str << "\t# tail";
  Str << "\n";
  Str.Cfg->getTarget()->resetStackAdjustment();
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

static void emitTwoAddress(const char *Opcode, const IceInst *Inst,
                           IceOstream &Str, uint32_t Option,
                           bool ShiftHack = false) {
  assert(Inst->getSrcSize() == 2);
  assert(Inst->getDest() == Inst->getSrc(0));
  Str << "\t" << Opcode << "\t";
  Inst->getDest()->emit(Str, Option);
  Str << ", ";
  bool EmittedSrc1 = false;
  if (ShiftHack) {
    IceVariable *ShiftReg = llvm::dyn_cast<IceVariable>(Inst->getSrc(1));
    if (ShiftReg && ShiftReg->getRegNum() == IceTargetX8632::Reg_ecx) {
      Str << "cl";
      EmittedSrc1 = true;
    }
  }
  if (!EmittedSrc1)
    Inst->getSrc(1)->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Add::emit(IceOstream &Str, uint32_t Option) const {
  emitTwoAddress("add", this, Str, Option);
}

void IceInstX8632Add::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = add." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Sub::emit(IceOstream &Str, unsigned Option) const {
  emitTwoAddress("sub", this, Str, Option);
}

void IceInstX8632Sub::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = sub." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632And::emit(IceOstream &Str, uint32_t Option) const {
  emitTwoAddress("and", this, Str, Option);
}

void IceInstX8632And::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = and." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Or::emit(IceOstream &Str, uint32_t Option) const {
  emitTwoAddress("or", this, Str, Option);
}

void IceInstX8632Or::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = or." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Xor::emit(IceOstream &Str, uint32_t Option) const {
  emitTwoAddress("xor", this, Str, Option);
}

void IceInstX8632Xor::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = xor." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Imul::emit(IceOstream &Str, uint32_t Option) const {
  emitTwoAddress("imul", this, Str, Option);
}

void IceInstX8632Imul::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = imul." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Idiv::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 3);
  Str << "\tidiv\t";
  getSrc(1)->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Idiv::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = idiv." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Div::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 3);
  Str << "\tdiv\t";
  getSrc(1)->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Div::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = div." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Shl::emit(IceOstream &Str, uint32_t Option) const {
  bool ShiftHack = true;
  emitTwoAddress("shl", this, Str, Option, ShiftHack);
}

void IceInstX8632Shl::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = shl." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Shr::emit(IceOstream &Str, uint32_t Option) const {
  bool ShiftHack = true;
  emitTwoAddress("shr", this, Str, Option, ShiftHack);
}

void IceInstX8632Shr::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = shr." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Sar::emit(IceOstream &Str, uint32_t Option) const {
  bool ShiftHack = true;
  emitTwoAddress("sar", this, Str, Option, ShiftHack);
}

void IceInstX8632Sar::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = sar." << getDest()->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Cdq::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 1);
  Str << "cdq";
}

void IceInstX8632Cdq::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = cdq." << getSrc(0)->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Icmp::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 2);
  Str << "\tcmp\t";
  getSrc(0)->emit(Str, Option);
  Str << ", ";
  getSrc(1)->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Icmp::dump(IceOstream &Str) const {
  Str << "cmp." << getSrc(0)->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Load::emit(IceOstream &Str, uint32_t Option) const {}

void IceInstX8632Load::dump(IceOstream &Str) const {
  Str << "mov." << getDest()->getType() << " ";
  dumpDest(Str);
  Str << ", [";
  dumpSources(Str);
  Str << "]";
}

void IceInstX8632Store::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 2);
  Str << "\tmov\t";
  getSrc(1)->emit(Str, Option);
  Str << ", ";
  getSrc(0)->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Store::dump(IceOstream &Str) const {
  Str << "mov." << getSrc(0)->getType() << " " << getSrc(1) << ", "
      << getSrc(0);
}

void IceInstX8632Mov::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 1);
  Str << "\tmov\t";
  getDest()->emit(Str, Option);
  Str << ", ";
  getSrc(0)->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Mov::dump(IceOstream &Str) const {
  Str << "mov." << getDest()->getType() << " ";
  dumpDest(Str);
  Str << ", ";
  dumpSources(Str);
}

void IceInstX8632Movsx::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 1);
  Str << "\tmovs" << OpcodeTypeFromIceType(getSrc(0)->getType());
  Str << OpcodeTypeFromIceType(getDest()->getType());
  Str << "\t";
  getDest()->emit(Str, Option);
  Str << ", ";
  getSrc(0)->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Movsx::dump(IceOstream &Str) const {
  Str << "movs" << OpcodeTypeFromIceType(getSrc(0)->getType());
  Str << OpcodeTypeFromIceType(getDest()->getType());
  Str << " ";
  dumpDest(Str);
  Str << ", ";
  dumpSources(Str);
}

void IceInstX8632Movzx::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 1);
  Str << "\tmovz" << OpcodeTypeFromIceType(getSrc(0)->getType());
  Str << OpcodeTypeFromIceType(getDest()->getType());
  Str << "\t";
  getDest()->emit(Str, Option);
  Str << ", ";
  getSrc(0)->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Movzx::dump(IceOstream &Str) const {
  Str << "movz" << OpcodeTypeFromIceType(getSrc(0)->getType());
  Str << OpcodeTypeFromIceType(getDest()->getType());
  Str << " ";
  dumpDest(Str);
  Str << ", ";
  dumpSources(Str);
}

void IceInstX8632Pop::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 0);
  Str << "\tpop\t";
  getDest()->emit(Str, Option);
  Str << "\n";
}

void IceInstX8632Pop::dump(IceOstream &Str) const {
  dumpDest(Str);
  Str << " = pop." << getDest()->getType() << " ";
}

void IceInstX8632Push::emit(IceOstream &Str, uint32_t Option) const {
  assert(getSrcSize() == 1);
  Str << "\tpush\t";
  getSrc(0)->emit(Str, Option);
  Str << "\n";
  Str.Cfg->getTarget()->updateStackAdjustment(4);
}

void IceInstX8632Push::dump(IceOstream &Str) const {
  Str << "push." << getSrc(0)->getType() << " ";
  dumpSources(Str);
}

void IceInstX8632Ret::emit(IceOstream &Str, uint32_t Option) const {
  Str << "\tret\n";
}

void IceInstX8632Ret::dump(IceOstream &Str) const {
  IceType Type = (getSrcSize() == 0 ? IceType_void : getSrc(0)->getType());
  Str << "ret." << Type << " ";
  dumpSources(Str);
}

void IceOperandX8632::dump(IceOstream &Str) const {
  Str << "<IceOperandX8632>";
}

void IceOperandX8632Mem::emit(IceOstream &Str, uint32_t Option) const {
  switch (getType()) {
  case IceType_i8:
    Str << "byte ptr ";
    break;
  case IceType_i16:
    Str << "word ptr ";
    break;
  case IceType_i32:
    Str << "dword ptr ";
    break;
  case IceType_i64:
    Str << "qword ptr ";
    break;
  default:
    Str << "??? ";
    break;
  }
  // TODO: The following is an almost verbatim paste of dump().
  bool Dumped = false;
  Str << "[";
  if (Base) {
    Base->emit(Str, Option);
    Dumped = true;
  }
  if (Index) {
    assert(Base);
    Str << "+";
    if (Shift > 0)
      Str << (1u << Shift) << "*";
    Index->emit(Str, Option);
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
    Offset->emit(Str, Option);
  }
  Str << "]";
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
  return IceInstX8632Mov::create(Cfg, Dest, Src);
}

IceInstList IceTargetX8632S::lowerAlloca(const IceInstAlloca *Inst,
                                         const IceInst *Next,
                                         bool &DeleteNextInst) {
  IceInstList Expansion;
  IceInst *NewInst;
  IsEbpBasedFrame = true;
  // TODO(sehr,stichnot): align allocated memory, keep stack aligned, minimize
  // the number of adjustments of esp, etc.
  IceVariable *Esp = Cfg->getTarget()->getPhysicalRegister(Reg_esp);
  uint32_t Size = Inst->getSize();
  NewInst =
      IceInstX8632Sub::create(Cfg, Esp, Cfg->getConstant(IceType_i32, Size));
  Expansion.push_back(NewInst);
  NewInst = IceInstX8632Mov::create(Cfg, Inst->getDest(), Esp);
  Expansion.push_back(NewInst);
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
      NewInst = IceInstX8632Mov::create(Cfg, Reg0,
                                        Cfg->getConstant(IceType_i32, Zero));
    else
      NewInst = IceInstX8632Cdq::create(Cfg, Reg0, Reg1);
    Expansion.push_back(NewInst);
  }

  // Assign t2.
  if (Reg2InEcx) {
    Reg2 = legalizeOperandToVar(Src1, Expansion, false, Reg_ecx);
  }

  // Generate the arithmetic instruction.
  switch (Inst->getOp()) {
  case IceInstArithmetic::Add:
    NewInst = IceInstX8632Add::create(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::And:
    NewInst = IceInstX8632And::create(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Or:
    NewInst = IceInstX8632Or::create(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Xor:
    NewInst = IceInstX8632Xor::create(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Sub:
    NewInst = IceInstX8632Sub::create(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Mul:
    // TODO: Optimized for llvm::isa<IceConstant>(Src1)
    NewInst = IceInstX8632Imul::create(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Udiv:
    NewInst = IceInstX8632Div::create(Cfg, Reg1, Reg2, Reg0);
    break;
  case IceInstArithmetic::Sdiv:
    NewInst = IceInstX8632Idiv::create(Cfg, Reg1, Reg2, Reg0);
    break;
  case IceInstArithmetic::Urem:
    NewInst = IceInstX8632Div::create(Cfg, Reg0, Reg2, Reg1);
    break;
  case IceInstArithmetic::Srem:
    NewInst = IceInstX8632Idiv::create(Cfg, Reg0, Reg2, Reg1);
    break;
  case IceInstArithmetic::Shl:
    NewInst = IceInstX8632Shl::create(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Lshr:
    NewInst = IceInstX8632Shr::create(Cfg, Reg1, Reg2);
    break;
  case IceInstArithmetic::Ashr:
    NewInst = IceInstX8632Sar::create(Cfg, Reg1, Reg2);
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
  NewInst = IceInstX8632Mov::create(Cfg, Dest, (ResultFromReg0 ? Reg0 : Reg1));
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
  NewInst = IceInstX8632Mov::create(Cfg, Dest, Reg);
  Expansion.push_back(NewInst);
  return Expansion;
}

IceInstList IceTargetX8632S::lowerBr(const IceInstBr *Inst, const IceInst *Next,
                                     bool &DeleteNextInst) {
  IceInstList Expansion;
  if (Inst->getTargetTrue() == NULL) { // unconditional branch
    IceInstTarget *NewInst =
        IceInstX8632Br::create(Cfg, Inst->getTargetFalse());
    Expansion.push_back(NewInst);
    return Expansion;
  }
  // cmp src, 0; br ne, labelTrue; br labelFalse
  IceInstTarget *NewInst;
  IceOperand *Src = legalizeOperand(Inst->getSrc(0), Legal_All, Expansion);
  uint64_t Zero = 0;
  IceConstant *OpZero = Cfg->getConstant(IceType_i32, Zero);
  NewInst = IceInstX8632Icmp::create(Cfg, Src, OpZero);
  Expansion.push_back(NewInst);
  NewInst = IceInstX8632Br::create(Cfg, Inst->getTargetTrue(),
                                   Inst->getTargetFalse(), IceInstIcmp::Ne);
  Expansion.push_back(NewInst);
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
    if (Arg->getType() == IceType_i64) {
      Expansion.push_back(IceInstX8632Push::create(Cfg, makeHighOperand(Arg)));
      Expansion.push_back(IceInstX8632Push::create(Cfg, makeLowOperand(Arg)));
    } else {
      NewInst = IceInstX8632Push::create(Cfg, Arg);
      // TODO: Where in the Cfg is StackOffset tracked?  It is needed
      // for instructions that push esp-based stack variables as
      // arguments.
      Expansion.push_back(NewInst);
    }
    StackOffset += typeWidthOnStack(Arg->getType());
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
      IceInstX8632Call::create(Cfg, Reg, Inst->getCallTarget(), Inst->isTail());
  Expansion.push_back(NewInst);
  IceInst *NewCall = NewInst;

  // Insert some sort of register-kill pseudo instruction.
  IceVarList KilledRegs;
  KilledRegs.push_back(Cfg->getTarget()->getPhysicalRegister(Reg_eax));
  KilledRegs.push_back(Cfg->getTarget()->getPhysicalRegister(Reg_ecx));
  KilledRegs.push_back(Cfg->getTarget()->getPhysicalRegister(Reg_edx));
  IceInst *Kill = IceInstFakeKill::create(Cfg, KilledRegs, NewCall);
  Expansion.push_back(Kill);

  // Generate a FakeUse to keep the call live if necessary.
  bool HasSideEffects = true;
  // TODO: set HasSideEffects=false if it's a known intrinsic without
  // side effects.
  if (HasSideEffects && Reg) {
    IceInst *FakeUse = IceInstFakeUse::create(Cfg, Reg);
    Expansion.push_back(FakeUse);
  }

  // Generate Dest=Reg assignment.
  if (Dest) {
    Dest->setPreferredRegister(Reg, false);
    NewInst = IceInstX8632Mov::create(Cfg, Dest, Reg);
    Expansion.push_back(NewInst);
  }

  // Add the appropriate offset to esp.
  if (StackOffset) {
    IceVariable *Esp = Cfg->getTarget()->getPhysicalRegister(Reg_esp);
    NewInst = IceInstX8632Add::create(
        Cfg, Esp, Cfg->getConstant(IceType_i32, StackOffset));
    Expansion.push_back(NewInst);
  }

  return Expansion;
}

IceInstList IceTargetX8632S::lowerCast(const IceInstCast *Inst,
                                       const IceInst *Next,
                                       bool &DeleteNextInst) {
  // a = cast(b) ==> t=cast(b); a=t; (link t->b, link a->t, no overlap)
  IceInstList Expansion;
  IceInstCast::IceCastKind CastKind = Inst->getCastKind();
  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = Inst->getSrc(0);
  IceInstTarget *NewInst;
  IceOperand *Reg =
      legalizeOperand(Src0, Legal_Reg | Legal_Mem, Expansion, true);
  switch (CastKind) {
  default:
    // TODO: implement other sorts of casts.
    Cfg->setError("Cast type not yet supported");
    return Expansion;
    break;
  case IceInstCast::Sext:
    NewInst = IceInstX8632Movsx::create(Cfg, Dest, Reg);
    break;
  case IceInstCast::Zext:
    NewInst = IceInstX8632Movzx::create(Cfg, Dest, Reg);
    break;
  case IceInstCast::Trunc:
    // It appears that Trunc is purely used to cast down from one integral type
    // to a smaller integral type.  In the generated code this does not seem
    // to be needed.  Treat these as vanilla moves.
    NewInst = IceInstX8632Mov::create(Cfg, Dest, Reg);
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
    NewInst = IceInstX8632Icmp::create(Cfg, Reg, Src1);
    Expansion.push_back(NewInst);
    NewInst =
        IceInstX8632Br::create(Cfg, NextBr->getTargetTrue(),
                               NextBr->getTargetFalse(), Inst->getCondition());
    Expansion.push_back(NewInst);
    DeleteNextInst = true;
    return Expansion;
  }

  // a=icmp cond, b, c ==> cmp b,c; a=1; br cond,L1; FakeUse(a); a=0; L1:
  //
  // Alternative without intra-block branch: cmp b,c; a=0; a=set<cond> {a}
  IceOperand *Src0 = Inst->getSrc(0);
  IceOperand *Src1 = Inst->getSrc(1);
  IceVariable *Dest = Inst->getDest();
  // cmp b, c
  bool IsImmOrReg = false;
  if (llvm::isa<IceConstant>(Src1))
    IsImmOrReg = true;
  else if (IceVariable *Var = llvm::dyn_cast<IceVariable>(Src1)) {
    if (Var->getRegNum() >= 0)
      IsImmOrReg = true;
  }
  IceOperand *Reg = legalizeOperand(Src0, IsImmOrReg ? Legal_All : Legal_Reg,
                                    Expansion, true);
  NewInst = IceInstX8632Icmp::create(Cfg, Reg, Src1);
  Expansion.push_back(NewInst);

  // a = 1;
  uint64_t Zero = 0;
  uint64_t One = 1;
  NewInst =
      IceInstX8632Mov::create(Cfg, Dest, Cfg->getConstant(IceType_i32, One));
  Expansion.push_back(NewInst);

  // create Label
  IceInstX8632Label *Label = IceInstX8632Label::create(Cfg, this);

  // br cond, Label
  NewInst = IceInstX8632Br::create(Cfg, Label, Inst->getCondition());
  Expansion.push_back(NewInst);

  // FakeUse(a)
  IceInst *FakeUse = IceInstFakeUse::create(Cfg, Dest);
  Expansion.push_back(FakeUse);

  // a = 0
  NewInst =
      IceInstX8632Mov::create(Cfg, Dest, Cfg->getConstant(IceType_i32, Zero));
  Expansion.push_back(NewInst);

  // Label:
  Expansion.push_back(Label);

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
    if (const IceInstArithmetic *ArithInst =
            llvm::dyn_cast_or_null<IceInstArithmetic>(IndexInst)) {
      IceOperand *IndexOperand0 = ArithInst->getSrc(0);
      IceVariable *IndexVariable0 = llvm::dyn_cast<IceVariable>(IndexOperand0);
      IceOperand *IndexOperand1 = ArithInst->getSrc(1);
      IceConstantInteger *IndexConstant1 =
          llvm::dyn_cast<IceConstantInteger>(IndexOperand1);
      if (ArithInst->getOp() == IceInstArithmetic::Mul && IndexVariable0 &&
          IndexOperand1->getType() == IceType_i32 && IndexConstant1) {
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
    Src = IceOperandX8632Mem::create(Cfg, Type, Base, Offset);
  }
  // Fuse this load with a subsequent Arithmetic instruction in the
  // following situations:
  //   a=[mem]; c=b+a ==> c=b+[mem] if last use of a and a not in b
  //   a=[mem]; c=a+b ==> c=b+[mem] if commutative and above is true
  //
  // TODO: Clean up and test thoroughly.
  //
  // TODO: Why limit to Arithmetic instructions?  This could probably
  // be applied to most any instruction type.
  if (const IceInstArithmetic *Arith =
          llvm::dyn_cast_or_null<const IceInstArithmetic>(Next)) {
    IceVariable *DestLoad = Inst->getDest();
    IceVariable *Src0Arith = llvm::dyn_cast<IceVariable>(Arith->getSrc(0));
    IceVariable *Src1Arith = llvm::dyn_cast<IceVariable>(Arith->getSrc(1));
    if (Src1Arith == DestLoad && Arith->isLastUse(Src1Arith) &&
        DestLoad != Src0Arith) {
      // TODO: This instruction leaks.
      IceInstArithmetic *NewArith = IceInstArithmetic::create(
          Cfg, Arith->getOp(), Arith->getDest(), Arith->getSrc(0), Src);
      DeleteNextInst = true;
      return lowerArithmetic(NewArith, NULL, DeleteNextInst);
    } else if (Src0Arith == DestLoad && Arith->isCommutative() &&
               Arith->isLastUse(Src0Arith) && DestLoad != Src1Arith) {
      // TODO: This instruction leaks.
      IceInstArithmetic *NewArith = IceInstArithmetic::create(
          Cfg, Arith->getOp(), Arith->getDest(), Arith->getSrc(1), Src);
      DeleteNextInst = true;
      return lowerArithmetic(NewArith, NULL, DeleteNextInst);
    }
  }

  // TODO: This instruction leaks.
  IceInstAssign *Assign = IceInstAssign::create(Cfg, Inst->getDest(), Src);
  return lowerAssign(Assign, Next, DeleteNextInst);
}

IceInstList IceTargetX8632S::doAddressOptLoad(const IceInstLoad *Inst) {
  IceInstList Expansion;
  IceInst *NewInst;
  IceVariable *Dest = Inst->getDest();
  IceOperand *Addr = Inst->getSrc(0);
  IceVariable *Index = NULL;
  int Shift = 0;
  int32_t Offset = 0; // TODO: make IceConstant
  IceVariable *Base = llvm::dyn_cast<IceVariable>(Addr);
  computeAddressOpt(Cfg, Base, Index, Shift, Offset);
  if (Base && Addr != Base) {
    IceConstant *OffsetOp = Cfg->getConstant(IceType_i32, Offset);
    Addr = IceOperandX8632Mem::create(Cfg, Dest->getType(), Base, OffsetOp,
                                      Index, Shift);
    NewInst = IceInstLoad::create(Cfg, Dest, Addr);
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
  IceVariable *Reg = NULL;
  if (Inst->getSrcSize()) {
    IceOperand *Src0 = Inst->getSrc(0);
    if (Src0->getType() == IceType_i64) {
      Src0 = legalizeOperand(Src0, Legal_All, Expansion);
      IceVariable *Src0Low =
          legalizeOperandToVar(makeLowOperand(Src0), Expansion, false, Reg_eax);
      IceVariable *Src0High = legalizeOperandToVar(makeHighOperand(Src0),
                                                   Expansion, false, Reg_edx);
      Reg = Src0Low;
      Expansion.push_back(IceInstFakeUse::create(Cfg, Src0High));
    } else {
      Reg = legalizeOperandToVar(Src0, Expansion, false, Reg_eax);
    }
  }
  NewInst = IceInstX8632Ret::create(Cfg, Reg);
  Expansion.push_back(NewInst);
  // Add a fake use of esp to make sure esp stays alive for the entire
  // function.  Otherwise post-call esp adjustments get dead-code
  // eliminated.  TODO: Are there more places where the fake use
  // should be inserted?  E.g. "void f(int n){while(1) g(n);}" may not
  // have a ret instruction.
  IceVariable *Esp = Cfg->getTarget()->getPhysicalRegister(Reg_esp);
  IceInst *FakeUse = IceInstFakeUse::create(Cfg, Esp);
  Expansion.push_back(FakeUse);
  return Expansion;
}

IceInstList IceTargetX8632S::lowerSelect(const IceInstSelect *Inst,
                                         const IceInst *Next,
                                         bool &DeleteNextInst) {
  // a=d?b:c ==> cmp d,0; a=b; jne L1; FakeUse(a); a=c; L1:
  //
  // Alternative if a is reg and c is not imm: cmp d,0; a=b; a=cmoveq c {a}
  IceInstList Expansion;
  IceInstTarget *NewInst;
  IceOperand *Condition =
      legalizeOperand(Inst->getCondition(), Legal_All, Expansion);
  uint64_t Zero = 0;
  IceConstant *OpZero = Cfg->getConstant(IceType_i32, Zero);
  NewInst = IceInstX8632Icmp::create(Cfg, Condition, OpZero);
  Expansion.push_back(NewInst);

  IceVariable *Dest = Inst->getDest();
  IceOperand *SrcTrue = legalizeOperand(Inst->getTrueOperand(),
                                        Legal_Reg | Legal_Imm, Expansion, true);
  NewInst = IceInstX8632Mov::create(Cfg, Dest, SrcTrue);
  Expansion.push_back(NewInst);

  // create Label
  IceInstX8632Label *Label = IceInstX8632Label::create(Cfg, this);

  NewInst = IceInstX8632Br::create(Cfg, Label, IceInstIcmp::Ne);
  Expansion.push_back(NewInst);

  // FakeUse(a)
  IceInst *FakeUse = IceInstFakeUse::create(Cfg, Dest);
  Expansion.push_back(FakeUse);

  IceOperand *SrcFalse = legalizeOperand(
      Inst->getFalseOperand(), Legal_Reg | Legal_Imm, Expansion, true);
  NewInst = IceInstX8632Mov::create(Cfg, Dest, SrcFalse);
  Expansion.push_back(NewInst);

  // Label:
  Expansion.push_back(Label);

  return Expansion;
}

IceInstList IceTargetX8632S::lowerStore(const IceInstStore *Inst,
                                        const IceInst *Next,
                                        bool &DeleteNextInst) {
  IceInstList Expansion;

  IceInstTarget *NewInst;
  IceOperand *Value = Inst->getData();
  IceOperand *Src1 = Inst->getAddr();
  IceOperandX8632Mem *NewSrc;
  if (!(NewSrc = llvm::dyn_cast<IceOperandX8632Mem>(Src1))) {
    IceVariable *Base = llvm::dyn_cast<IceVariable>(Src1);
    IceConstant *Offset = llvm::dyn_cast<IceConstant>(Src1);
    assert(Base || Offset);
    NewSrc = IceOperandX8632Mem::create(Cfg, Value->getType(), Base, Offset);
  }
  NewSrc = llvm::cast<IceOperandX8632Mem>(
      legalizeOperand(NewSrc, Legal_All, Expansion));
  IceOperand *Reg0 =
      legalizeOperand(Value, Legal_Reg | Legal_Imm, Expansion, true);

  NewInst = IceInstX8632Store::create(Cfg, Reg0, NewSrc);
  Expansion.push_back(NewInst);

  return Expansion;
}

IceInstList IceTargetX8632S::doAddressOptStore(const IceInstStore *Inst) {
  IceInstList Expansion;
  IceInst *NewInst;
  IceOperand *Data = Inst->getData();
  IceOperand *Addr = Inst->getAddr();
  IceVariable *Index = NULL;
  int Shift = 0;
  int32_t Offset = 0; // TODO: make IceConstant
  IceVariable *Base = llvm::dyn_cast<IceVariable>(Addr);
  computeAddressOpt(Cfg, Base, Index, Shift, Offset);
  if (Base && Addr != Base) {
    IceConstant *OffsetOp = Cfg->getConstant(IceType_i32, Offset);
    Addr = IceOperandX8632Mem::create(Cfg, Data->getType(), Base, OffsetOp,
                                      Index, Shift);
    NewInst = IceInstStore::create(Cfg, Data, Addr);
    Expansion.push_back(NewInst);
  }
  return Expansion;
}

IceInstList IceTargetX8632S::lowerSwitch(const IceInstSwitch *Inst,
                                         const IceInst *Next,
                                         bool &DeleteNextInst) {
  // This implements the most naive possible lowering.
  // cmp a,val[0]; jeq label[0]; cmp a,val[1]; jeq label[1]; ... jmp default
  IceInstList Expansion;
  IceInst *NewInst;
  IceOperand *Src = Inst->getSrc(0);
  unsigned NumCases = Inst->getNumCases();
  // OK, we'll be slightly less naive by forcing Src into a physical
  // register if there are 2 or more uses.
  if (NumCases >= 2)
    Src = legalizeOperandToVar(Src, Expansion, true);
  else
    Src = legalizeOperand(Src, Legal_All, Expansion, true);
  for (unsigned I = 0; I < NumCases; ++I) {
    IceOperand *Value = Cfg->getConstant(IceType_i32, Inst->getValue(I));
    NewInst = IceInstX8632Icmp::create(Cfg, Src, Value);
    Expansion.push_back(NewInst);
    NewInst = IceInstX8632Br::create(Cfg, Inst->getLabel(I), IceInstIcmp::Eq);
    Expansion.push_back(NewInst);
  }

  NewInst = IceInstX8632Br::create(Cfg, Inst->getLabelDefault());
  Expansion.push_back(NewInst);

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
      From = IceOperandX8632Mem::create(Cfg, Mem->getType(), RegBase,
                                        Mem->getOffset(), RegIndex,
                                        Mem->getShift());
    }

    if (!(Allowed & Legal_Mem)) {
      IceVariable *Reg = Cfg->makeVariable(From->getType());
      if (RegNum < 0) {
        Reg->setWeightInfinite();
      } else {
        Reg->setRegNum(RegNum);
      }
      NewInst = IceInstX8632Mov::create(Cfg, Reg, From);
      Insts.push_back(NewInst);
      From = Reg;
    }
    return From;
  }
  if (llvm::isa<IceConstant>(From)) {
    if (!(Allowed & Legal_Imm)) {
      IceVariable *Reg = Cfg->makeVariable(From->getType());
      if (RegNum < 0) {
        Reg->setWeightInfinite();
      } else {
        Reg->setRegNum(RegNum);
      }
      NewInst = IceInstX8632Mov::create(Cfg, Reg, From);
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
      NewInst = IceInstX8632Mov::create(Cfg, Reg, From);
      Insts.push_back(NewInst);
      From = Reg;
    }
    return From;
  }
  assert(0);
  return From;
}
