/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "IceDefs.h"
#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceInstX8632.h"
#include "IceOperand.h"
#include "IceTargetLoweringX8632.h"

IceTargetX8632::IceTargetX8632(IceCfg *Cfg)
    : IceTargetLowering(Cfg), IsEbpBasedFrame(false), FrameSizeLocals(0),
      LocalsSizeBytes(0), NextLabelNumber(0), ComputedLiveRanges(false),
      PhysicalRegisters(IceVarList(Reg_NUM)) {
  llvm::SmallBitVector IntegerRegisters(Reg_NUM);
  llvm::SmallBitVector IntegerRegistersNonI8(Reg_NUM);
  llvm::SmallBitVector FloatRegisters(Reg_NUM);
  llvm::SmallBitVector InvalidRegisters(Reg_NUM);
  for (unsigned i = Reg_eax; i <= Reg_edi; ++i)
    IntegerRegisters[i] = true;
  IntegerRegistersNonI8[Reg_eax] = true;
  IntegerRegistersNonI8[Reg_ecx] = true;
  IntegerRegistersNonI8[Reg_edx] = true;
  IntegerRegistersNonI8[Reg_ebx] = true;
  for (unsigned i = Reg_xmm0; i <= Reg_xmm7; ++i)
    FloatRegisters[i] = true;
  TypeToRegisterSet[IceType_void] = InvalidRegisters;
  TypeToRegisterSet[IceType_i1] = IntegerRegistersNonI8;
  TypeToRegisterSet[IceType_i8] = IntegerRegistersNonI8;
  TypeToRegisterSet[IceType_i16] = IntegerRegisters;
  TypeToRegisterSet[IceType_i32] = IntegerRegisters;
  TypeToRegisterSet[IceType_i64] = IntegerRegisters;
  TypeToRegisterSet[IceType_f32] = FloatRegisters;
  TypeToRegisterSet[IceType_f64] = FloatRegisters;
  ScratchRegs = FloatRegisters;
  ScratchRegs[Reg_eax] = true;
  ScratchRegs[Reg_ecx] = true;
  ScratchRegs[Reg_edx] = true;
}

void IceTargetX8632::translate(void) {
  IceTimer T_placePhiLoads;
  Cfg->placePhiLoads();
  if (Cfg->hasError())
    return;
  T_placePhiLoads.printElapsedUs(Cfg->Str, "placePhiLoads()");
  IceTimer T_placePhiStores;
  Cfg->placePhiStores();
  if (Cfg->hasError())
    return;
  T_placePhiStores.printElapsedUs(Cfg->Str, "placePhiStores()");
  IceTimer T_deletePhis;
  Cfg->deletePhis();
  if (Cfg->hasError())
    return;
  T_deletePhis.printElapsedUs(Cfg->Str, "deletePhis()");
  IceTimer T_renumber1;
  Cfg->renumberInstructions();
  if (Cfg->hasError())
    return;
  T_renumber1.printElapsedUs(Cfg->Str, "renumberInstructions()");
  if (Cfg->Str.isVerbose())
    Cfg->Str << "================ After Phi lowering ================\n";
  Cfg->dump();

  IceTimer T_doAddressOpt;
  Cfg->doAddressOpt();
  T_doAddressOpt.printElapsedUs(Cfg->Str, "doAddressOpt()");
  // Liveness may be incorrect after address mode optimization.
  IceTimer T_renumber2;
  Cfg->renumberInstructions();
  if (Cfg->hasError())
    return;
  T_renumber2.printElapsedUs(Cfg->Str, "renumberInstructions()");
  // TODO: It should be sufficient to use the fastest livness
  // calculation, i.e. IceLiveness_LREndLightweight.  However,
  // currently this breaks one test (icmp-simple.ll) because with
  // IceLiveness_LREndFull, the problematic instructions get dead-code
  // eliminated.
  IceTimer T_liveness1;
  Cfg->liveness(IceLiveness_LREndFull);
  if (Cfg->hasError())
    return;
  T_liveness1.printElapsedUs(Cfg->Str, "liveness()");
  if (Cfg->Str.isVerbose())
    Cfg->Str
        << "================ After x86 address mode opt ================\n";
  Cfg->dump();
  IceTimer T_genCode;
  Cfg->genCode();
  if (Cfg->hasError())
    return;
  T_genCode.printElapsedUs(Cfg->Str, "genCode()");
  IceTimer T_renumber3;
  Cfg->renumberInstructions();
  if (Cfg->hasError())
    return;
  T_renumber3.printElapsedUs(Cfg->Str, "renumberInstructions()");
  IceTimer T_liveness2;
  Cfg->liveness(IceLiveness_RangesFull);
  if (Cfg->hasError())
    return;
  T_liveness2.printElapsedUs(Cfg->Str, "liveness()");
  ComputedLiveRanges = true;
  if (Cfg->Str.isVerbose())
    Cfg->Str
        << "================ After initial x8632 codegen ================\n";
  Cfg->dump();

  IceTimer T_regAlloc;
  Cfg->regAlloc();
  if (Cfg->hasError())
    return;
  T_regAlloc.printElapsedUs(Cfg->Str, "regAlloc()");
  if (Cfg->Str.isVerbose())
    Cfg->Str
        << "================ After linear scan regalloc ================\n";
  Cfg->dump();

  IceTimer T_genFrame;
  Cfg->genFrame();
  if (Cfg->hasError())
    return;
  T_genFrame.printElapsedUs(Cfg->Str, "genFrame()");
  if (Cfg->Str.isVerbose())
    Cfg->Str << "================ After stack frame mapping ================\n";
  Cfg->dump();
}

IceString IceTargetX8632::RegNames[] = { "eax",  "ecx",  "edx",  "ebx",
                                         "esp",  "ebp",  "esi",  "edi",
                                         "xmm0", "xmm1", "xmm2", "xmm3",
                                         "xmm4", "xmm5", "xmm6", "xmm7" };

IceVariable *IceTargetX8632::getPhysicalRegister(unsigned RegNum) {
  assert(RegNum < PhysicalRegisters.size());
  IceVariable *Reg = PhysicalRegisters[RegNum];
  if (Reg == NULL) {
    IceCfgNode *Node = NULL; // NULL means multi-block lifetime
    Reg = Cfg->makeVariable(IceType_i32, Node);
    Reg->setRegNum(RegNum);
    PhysicalRegisters[RegNum] = Reg;
  }
  return Reg;
}

IceString IceTargetX8632::getRegName(int RegNum, IceType Type) const {
  assert(RegNum >= 0);
  assert(RegNum < Reg_NUM);
  static IceString RegNames8[] = { "al", "cl", "dl", "bl" };
  static IceString RegNames16[] = { "ax", "cx", "dx", "bx",
                                    "sp", "bp", "si", "di" };
  switch (Type) {
  case IceType_i1:
  case IceType_i8:
    assert(static_cast<unsigned>(RegNum) <
           (sizeof(RegNames8) / sizeof(*RegNames8)));
    return RegNames8[RegNum];
  case IceType_i16:
    assert(static_cast<unsigned>(RegNum) <
           (sizeof(RegNames16) / sizeof(*RegNames16)));
    return RegNames16[RegNum];
  default:
    return RegNames[RegNum];
  }
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
                                         IceLoweringContext &Context) {
  IceVariable *Low = Arg->getLow();
  IceVariable *High = Arg->getHigh();
  IceType Type = Arg->getType();
  if (Low && High && Type == IceType_i64) {
    assert(Low->getType() != IceType_i64);  // don't want infinite recursion
    assert(High->getType() != IceType_i64); // don't want infinite recursion
    setArgOffsetAndCopy(Low, FramePtr, BasicFrameOffset, InArgsSizeBytes,
                        Context);
    setArgOffsetAndCopy(High, FramePtr, BasicFrameOffset, InArgsSizeBytes,
                        Context);
    return;
  }
  Arg->setStackOffset(BasicFrameOffset + InArgsSizeBytes);
  if (Arg->getRegNum() >= 0) {
    // TODO: Uncomment this assert when I64 lowering is complete.
    // assert(Type != IceType_i64);
    IceOperandX8632Mem *Mem = IceOperandX8632Mem::create(
        Cfg, Type, FramePtr,
        Cfg->getConstantInt(IceType_i32, Arg->getStackOffset()));
    Context.insert(IceInstX8632Mov::create(Cfg, Arg, Mem));
  }
  InArgsSizeBytes += typeWidthOnStack(Type);
}

void IceTargetX8632::addProlog(IceCfgNode *Node) {
  const bool SimpleCoalescing = true;
  int InArgsSizeBytes = 0;
  int RetIpSizeBytes = 4;
  int PreservedRegsSizeBytes = 0;
  LocalsSizeBytes = 0;
  IceLoweringContext Context(Node);
  Context.Next = Context.Cur;

  // Determine stack frame offsets for each IceVariable without a
  // register assignment.  This can be done as one variable per stack
  // slot.  Or, do coalescing by running the register allocator again
  // with an infinite set of registers (as a side effect, this gives
  // variables a second chance at physical register assignment).
  //
  // A middle ground approach is to leverage sparsity and allocate one
  // block of space on the frame for globals (variables with
  // multi-block lifetime), and one block to share for locals
  // (single-block lifetime).

  llvm::SmallBitVector CalleeSaves =
      getRegisterSet(IceTargetLowering::RegMask_CalleeSave);

  int GlobalsSize = 0;
  std::vector<int> LocalsSize(Cfg->getNumNodes());

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
    if (ComputedLiveRanges && Var->getLiveRange().isEmpty())
      continue;
    int Increment = typeWidthOnStack(Var->getType());
    if (SimpleCoalescing) {
      if (Var->isMultiblockLife()) {
        GlobalsSize += Increment;
      } else {
        unsigned NodeIndex = Var->getLocalUseNode()->getIndex();
        LocalsSize[NodeIndex] += Increment;
        if (LocalsSize[NodeIndex] > LocalsSizeBytes)
          LocalsSizeBytes = LocalsSize[NodeIndex];
      }
    } else {
      LocalsSizeBytes += Increment;
    }
  }
  LocalsSizeBytes += GlobalsSize;

  // Add push instructions for preserved registers.
  for (unsigned i = 0; i < CalleeSaves.size(); ++i) {
    if (CalleeSaves[i] && RegsUsed[i]) {
      PreservedRegsSizeBytes += 4;
      Context.insert(IceInstX8632Push::create(Cfg, getPhysicalRegister(i)));
    }
  }

  // Generate "push ebp; mov ebp, esp"
  if (IsEbpBasedFrame) {
    assert((RegsUsed & getRegisterSet(IceTargetLowering::RegMask_FramePointer))
               .count() == 0);
    PreservedRegsSizeBytes += 4;
    Context.insert(IceInstX8632Push::create(Cfg, getPhysicalRegister(Reg_ebp)));
    Context.insert(IceInstX8632Mov::create(Cfg, getPhysicalRegister(Reg_ebp),
                                           getPhysicalRegister(Reg_esp)));
  }

  // Generate "sub esp, LocalsSizeBytes"
  if (LocalsSizeBytes)
    Context.insert(IceInstX8632Sub::create(
        Cfg, getPhysicalRegister(Reg_esp),
        Cfg->getConstantInt(IceType_i32, LocalsSizeBytes)));

  resetStackAdjustment();

  // Fill in stack offsets for locals.
  int TotalGlobalsSize = GlobalsSize;
  GlobalsSize = 0;
  LocalsSize.assign(LocalsSize.size(), 0);
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
    if (ComputedLiveRanges && Var->getLiveRange().isEmpty())
      continue;
    int Increment = typeWidthOnStack(Var->getType());
    if (SimpleCoalescing) {
      if (Var->isMultiblockLife()) {
        GlobalsSize += Increment;
        NextStackOffset = GlobalsSize;
      } else {
        unsigned NodeIndex = Var->getLocalUseNode()->getIndex();
        LocalsSize[NodeIndex] += Increment;
        NextStackOffset = TotalGlobalsSize + LocalsSize[NodeIndex];
      }
    } else {
      NextStackOffset += Increment;
    }
    if (IsEbpBasedFrame)
      Var->setStackOffset(-NextStackOffset);
    else
      Var->setStackOffset(LocalsSizeBytes - NextStackOffset);
  }
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
                        Context);
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
}

void IceTargetX8632::addEpilog(IceCfgNode *Node) {
  IceInstList &Insts = Node->getInsts();
  IceInstList::reverse_iterator RI, E;
  for (RI = Insts.rbegin(), E = Insts.rend(); RI != E; ++RI) {
    if (llvm::isa<IceInstX8632Ret>(*RI))
      break;
  }
  if (RI == E)
    return;

  // Convert the reverse_iterator position into its corresponding
  // (forward) iterator position.
  IceInstList::iterator InsertPoint = RI.base();
  --InsertPoint;
  IceLoweringContext Context(Node);
  Context.Next = InsertPoint;

  if (IsEbpBasedFrame) {
    // mov esp, ebp
    Context.insert(IceInstX8632Mov::create(Cfg, getPhysicalRegister(Reg_esp),
                                           getPhysicalRegister(Reg_ebp)));
    // pop ebp
    Context.insert(IceInstX8632Pop::create(Cfg, getPhysicalRegister(Reg_ebp)));
  } else {
    // add esp, LocalsSizeBytes
    if (LocalsSizeBytes)
      Context.insert(IceInstX8632Add::create(
          Cfg, getPhysicalRegister(Reg_esp),
          Cfg->getConstantInt(IceType_i32, LocalsSizeBytes)));
  }

  // Add pop instructions for preserved registers.
  llvm::SmallBitVector CalleeSaves =
      getRegisterSet(IceTargetLowering::RegMask_CalleeSave);
  for (unsigned i = 0; i < CalleeSaves.size(); ++i) {
    unsigned j = CalleeSaves.size() - i - 1;
    if (j == Reg_ebp && IsEbpBasedFrame)
      continue;
    if (CalleeSaves[j] && RegsUsed[j]) {
      Context.insert(IceInstX8632Pop::create(Cfg, getPhysicalRegister(j)));
    }
  }
}

void IceTargetX8632::split64(IceVariable *Var, IceLoweringContext &Context) {
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
  IceVariable *High = Var->getHigh();
  if (Low) {
    assert(High);
    return;
  }
  assert(High == NULL);
  Low = Cfg->makeVariable(IceType_i32, Context.Node, Var->getName() + "__lo");
  High = Cfg->makeVariable(IceType_i32, Context.Node, Var->getName() + "__hi");
  Var->setLowHigh(Low, High);
  if (Var->getIsArg()) {
    Low->setIsArg(Cfg);
    High->setIsArg(Cfg);
  }
}

IceOperand *IceTargetX8632::makeLowOperand(IceOperand *Operand,
                                           IceLoweringContext &Context) {
  assert(Operand->getType() == IceType_i64);
  if (Operand->getType() != IceType_i64)
    return Operand;
  if (IceVariable *Var = llvm::dyn_cast<IceVariable>(Operand)) {
    split64(Var, Context);
    return Var->getLow();
  }
  if (IceConstantInteger *Const = llvm::dyn_cast<IceConstantInteger>(Operand)) {
    uint64_t Mask = (1ul << 32) - 1;
    return Cfg->getConstantInt(IceType_i32, Const->getValue() & Mask);
  }
  if (IceOperandX8632Mem *Mem = llvm::dyn_cast<IceOperandX8632Mem>(Operand)) {
    return IceOperandX8632Mem::create(Cfg, IceType_i32, Mem->getBase(),
                                      Mem->getOffset(), Mem->getIndex(),
                                      Mem->getShift());
  }
  assert(0 && "Unsupported operand type");
  return NULL;
}

IceOperand *IceTargetX8632::makeHighOperand(IceOperand *Operand,
                                            IceLoweringContext &Context) {
  assert(Operand->getType() == IceType_i64);
  if (Operand->getType() != IceType_i64)
    return Operand;
  if (IceVariable *Var = llvm::dyn_cast<IceVariable>(Operand)) {
    split64(Var, Context);
    return Var->getHigh();
  }
  if (IceConstantInteger *Const = llvm::dyn_cast<IceConstantInteger>(Operand)) {
    return Cfg->getConstantInt(IceType_i32, Const->getValue() >> 32);
  }
  if (IceOperandX8632Mem *Mem = llvm::dyn_cast<IceOperandX8632Mem>(Operand)) {
    IceConstant *Offset = Mem->getOffset();
    if (Offset == NULL)
      Offset = Cfg->getConstantInt(IceType_i32, 4);
    else if (IceConstantInteger *IntOffset =
                 llvm::dyn_cast<IceConstantInteger>(Offset)) {
      Offset = Cfg->getConstantInt(IceType_i32, 4 + IntOffset->getValue());
    } else if (IceConstantRelocatable *SymOffset =
                   llvm::dyn_cast<IceConstantRelocatable>(Offset)) {
      // TODO: This creates a new entry in the constant pool, instead
      // of reusing the existing entry.
      Offset =
          Cfg->getConstantSym(IceType_i32, SymOffset->getHandle(),
                              4 + SymOffset->getOffset(), SymOffset->getName());
    }
    return IceOperandX8632Mem::create(Cfg, IceType_i32, Mem->getBase(), Offset,
                                      Mem->getIndex(), Mem->getShift());
  }
  assert(0 && "Unsupported operand type");
  return NULL;
}

llvm::SmallBitVector IceTargetX8632::getRegisterSet(RegSetMask Include,
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
  Registers[Reg_xmm0] = Scratch;
  Registers[Reg_xmm1] = Scratch;
  Registers[Reg_xmm2] = Scratch;
  Registers[Reg_xmm3] = Scratch;
  Registers[Reg_xmm4] = Scratch;
  Registers[Reg_xmm5] = Scratch;
  Registers[Reg_xmm6] = Scratch;
  Registers[Reg_xmm7] = Scratch;
  return Registers;
}

void IceTargetX8632::lowerAlloca(const IceInstAlloca *Inst,
                                 IceLoweringContext &Context) {
  IsEbpBasedFrame = true;
  // TODO(sehr,stichnot): align allocated memory, keep stack aligned, minimize
  // the number of adjustments of esp, etc.
  IceVariable *Esp = Cfg->getTarget()->getPhysicalRegister(Reg_esp);
  IceOperand *ByteCount = Inst->getSrc(0);
  IceOperand *TotalSize = legalizeOperand(ByteCount, Legal_All, Context);
  Context.insert(IceInstX8632Sub::create(Cfg, Esp, TotalSize));
  Context.insert(IceInstX8632Mov::create(Cfg, Inst->getDest(), Esp));
}

void IceTargetX8632::lowerArithmetic(const IceInstArithmetic *Inst,
                                     IceLoweringContext &Context) {
  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = legalizeOperand(Inst->getSrc(0), Legal_All, Context);
  IceOperand *Src1 = legalizeOperand(Inst->getSrc(1), Legal_All, Context);
  IceVariable *Reg0 = NULL;
  IceVariable *Reg1 = NULL;
  IceOperand *Reg2 = Src1;
  bool LowerI64ToI32 = (Dest->getType() == IceType_i64);
  IceVariable *DestLo = NULL, *DestHi = NULL;
  IceVariable *TmpLo = NULL, *TmpHi = NULL;
  IceOperand *Src0Lo = NULL, *Src0Hi = NULL, *Src1Lo = NULL, *Src1Hi = NULL;
  if (LowerI64ToI32) {
    DestLo = llvm::cast<IceVariable>(makeLowOperand(Dest, Context));
    DestHi = llvm::cast<IceVariable>(makeHighOperand(Dest, Context));
    Src0Lo = makeLowOperand(Src0, Context);
    Src0Hi = makeHighOperand(Src0, Context);
    Src1Lo = makeLowOperand(Src1, Context);
    Src1Hi = makeHighOperand(Src1, Context);
  }

  switch (Inst->getOp()) {
  case IceInstArithmetic::Add:
    if (LowerI64ToI32) {
      TmpLo = legalizeOperandToVar(Src0Lo, Context);
      Context.insert(IceInstX8632Add::create(Cfg, TmpLo, Src1Lo));
      Context.insert(IceInstX8632Mov::create(Cfg, DestLo, TmpLo));
      TmpHi = legalizeOperandToVar(Src0Hi, Context);
      Context.insert(IceInstX8632Adc::create(Cfg, TmpHi, Src1Hi));
      Context.insert(IceInstX8632Mov::create(Cfg, DestHi, TmpHi));
    } else {
      Reg1 = legalizeOperandToVar(Src0, Context);
      Reg2 = legalizeOperand(Src1, Legal_All, Context);
      Context.insert(IceInstX8632Add::create(Cfg, Reg1, Reg2));
      Context.insert(IceInstX8632Mov::create(Cfg, Dest, Reg1));
    }
    break;
  case IceInstArithmetic::And:
    if (LowerI64ToI32) {
      TmpLo = legalizeOperandToVar(Src0Lo, Context);
      Context.insert(IceInstX8632And::create(Cfg, TmpLo, Src1Lo));
      Context.insert(IceInstX8632Mov::create(Cfg, DestLo, TmpLo));
      TmpHi = legalizeOperandToVar(Src0Hi, Context);
      Context.insert(IceInstX8632And::create(Cfg, TmpHi, Src1Hi));
      Context.insert(IceInstX8632Mov::create(Cfg, DestHi, TmpHi));
    } else {
      Reg1 = legalizeOperandToVar(Src0, Context);
      Reg2 = legalizeOperand(Src1, Legal_All, Context);
      Context.insert(IceInstX8632And::create(Cfg, Reg1, Reg2));
      Context.insert(IceInstX8632Mov::create(Cfg, Dest, Reg1));
    }
    break;
  case IceInstArithmetic::Or:
    if (LowerI64ToI32) {
      TmpLo = legalizeOperandToVar(Src0Lo, Context);
      Context.insert(IceInstX8632Or::create(Cfg, TmpLo, Src1Lo));
      Context.insert(IceInstX8632Mov::create(Cfg, DestLo, TmpLo));
      TmpHi = legalizeOperandToVar(Src0Hi, Context);
      Context.insert(IceInstX8632Or::create(Cfg, TmpHi, Src1Hi));
      Context.insert(IceInstX8632Mov::create(Cfg, DestHi, TmpHi));
    } else {
      Reg1 = legalizeOperandToVar(Src0, Context);
      Reg2 = legalizeOperand(Src1, Legal_All, Context);
      Context.insert(IceInstX8632Or::create(Cfg, Reg1, Reg2));
      Context.insert(IceInstX8632Mov::create(Cfg, Dest, Reg1));
    }
    break;
  case IceInstArithmetic::Xor:
    if (LowerI64ToI32) {
      TmpLo = legalizeOperandToVar(Src0Lo, Context);
      Context.insert(IceInstX8632Xor::create(Cfg, TmpLo, Src1Lo));
      Context.insert(IceInstX8632Mov::create(Cfg, DestLo, TmpLo));
      TmpHi = legalizeOperandToVar(Src0Hi, Context);
      Context.insert(IceInstX8632Xor::create(Cfg, TmpHi, Src1Hi));
      Context.insert(IceInstX8632Mov::create(Cfg, DestHi, TmpHi));
    } else {
      Reg1 = legalizeOperandToVar(Src0, Context);
      Reg2 = legalizeOperand(Src1, Legal_All, Context);
      Context.insert(IceInstX8632Xor::create(Cfg, Reg1, Reg2));
      Context.insert(IceInstX8632Mov::create(Cfg, Dest, Reg1));
    }
    break;
  case IceInstArithmetic::Sub:
    if (LowerI64ToI32) {
      TmpLo = legalizeOperandToVar(Src0Lo, Context);
      Context.insert(IceInstX8632Sub::create(Cfg, TmpLo, Src1Lo));
      Context.insert(IceInstX8632Mov::create(Cfg, DestLo, TmpLo));
      TmpHi = legalizeOperandToVar(Src0Hi, Context);
      Context.insert(IceInstX8632Sbb::create(Cfg, TmpHi, Src1Hi));
      Context.insert(IceInstX8632Mov::create(Cfg, DestHi, TmpHi));
    } else {
      Reg1 = legalizeOperandToVar(Src0, Context);
      Reg2 = legalizeOperand(Src1, Legal_All, Context);
      Context.insert(IceInstX8632Sub::create(Cfg, Reg1, Reg2));
      Context.insert(IceInstX8632Mov::create(Cfg, Dest, Reg1));
    }
    break;
  case IceInstArithmetic::Mul:
    if (LowerI64ToI32) {
      IceVariable *Tmp1, *Tmp2, *Tmp3;
      IceVariable *Tmp4Lo = Cfg->makeVariable(IceType_i32, Context.Node);
      IceVariable *Tmp4Hi = Cfg->makeVariable(IceType_i32, Context.Node);
      Tmp4Lo->setRegNum(Reg_eax);
      Tmp4Hi->setRegNum(Reg_edx);
      // gcc does the following:
      // a=b*c ==>
      //   t1 = b.hi; t1 *=(imul) c.lo
      //   t2 = c.hi; t2 *=(imul) b.lo
      //   t3:eax = b.lo
      //   t4.hi:edx,t4.lo:eax = t3:eax *(mul) c.lo
      //   a.lo = t4.lo
      //   t4.hi += t1
      //   t4.hi += t2
      //   a.hi = t4.hi
      Tmp1 = legalizeOperandToVar(Src0Hi, Context);
      Context.insert(IceInstX8632Imul::create(Cfg, Tmp1, Src1Lo));
      Tmp2 = legalizeOperandToVar(Src1Hi, Context);
      Context.insert(IceInstX8632Imul::create(Cfg, Tmp2, Src0Lo));
      Tmp3 = legalizeOperandToVar(Src0Lo, Context, false, Reg_eax);
      Context.insert(IceInstX8632Mul::create(Cfg, Tmp4Lo, Tmp3, Src1Lo));
      Context.insert(IceInstX8632Mov::create(Cfg, DestLo, Tmp4Lo));
      Context.insert(IceInstFakeDef::create(Cfg, Tmp4Hi, Tmp4Lo));
      Context.insert(IceInstX8632Add::create(Cfg, Tmp4Hi, Tmp1));
      Context.insert(IceInstX8632Add::create(Cfg, Tmp4Hi, Tmp2));
      Context.insert(IceInstX8632Mov::create(Cfg, DestHi, Tmp4Hi));
    } else {
      // TODO: Optimize for llvm::isa<IceConstant>(Src1)
      // TODO: Strength-reduce multiplications by a constant,
      // particularly -1 and powers of 2.  Advanced: use lea to
      // multiply by 3, 5, 9.
      Reg1 = legalizeOperandToVar(Src0, Context);
      Reg2 = legalizeOperand(Src1, Legal_All, Context);
      Context.insert(IceInstX8632Imul::create(Cfg, Reg1, Reg2));
      Context.insert(IceInstX8632Mov::create(Cfg, Dest, Reg1));
    }
    break;
  case IceInstArithmetic::Shl:
    if (LowerI64ToI32) {
      // TODO: Refactor the similarities between Shl, Lshr, and Ashr.
      // gcc does the following:
      // a=b<<c ==>
      //   t1:ecx = c.lo & 0xff // via movzx, we can probably just t1=c.lo
      //   t2 = b.lo
      //   t3 = b.hi
      //   t3 = shld t3, t2, t1
      //   t2 = shl t2, t1
      //   test t1, 0x20
      //   je L1
      //   use(t3)
      //   t3 = t2
      //   t2 = 0
      // L1:
      //   a.lo = t2
      //   a.hi = t3
      IceVariable *Tmp1, *Tmp2, *Tmp3;
      IceConstant *BitTest = Cfg->getConstantInt(IceType_i32, 0x20);
      IceConstant *Zero = Cfg->getConstantInt(IceType_i32, 0);
      IceInstX8632Label *Label = IceInstX8632Label::create(Cfg, this);
      Tmp1 = legalizeOperandToVar(Src1Lo, Context, false, Reg_ecx);
      Tmp2 = legalizeOperandToVar(Src0Lo, Context);
      Tmp3 = legalizeOperandToVar(Src0Hi, Context);
      Context.insert(IceInstX8632Shld::create(Cfg, Tmp3, Tmp2, Tmp1));
      Context.insert(IceInstX8632Shl::create(Cfg, Tmp2, Tmp1));
      Context.insert(IceInstX8632Test::create(Cfg, Tmp1, BitTest));
      Context.insert(IceInstX8632Br::create(Cfg, Label, IceInstX8632Br::Br_e));
      Context.insert(IceInstFakeUse::create(Cfg, Tmp3));
      Context.insert(IceInstX8632Mov::create(Cfg, Tmp3, Tmp2));
      Context.insert(IceInstX8632Mov::create(Cfg, Tmp2, Zero));
      Context.insert(Label);
      Context.insert(IceInstX8632Mov::create(Cfg, DestLo, Tmp2));
      Context.insert(IceInstX8632Mov::create(Cfg, DestHi, Tmp3));
    } else {
      Reg1 = legalizeOperandToVar(Src0, Context);
      if (!llvm::isa<IceConstant>(Src1))
        Reg2 = legalizeOperandToVar(Src1, Context, false, Reg_ecx);
      Context.insert(IceInstX8632Shl::create(Cfg, Reg1, Reg2));
      Context.insert(IceInstX8632Mov::create(Cfg, Dest, Reg1));
    }
    break;
  case IceInstArithmetic::Lshr:
    if (LowerI64ToI32) {
      // a=b>>c (unsigned) ==>
      //   t1:ecx = c.lo & 0xff // via movzx, we can probably just t1=c.lo
      //   t2 = b.lo
      //   t3 = b.hi
      //   t2 = shrd t2, t3, t1
      //   t3 = shr t3, t1
      //   test t1, 0x20
      //   je L1
      //   use(t2)
      //   t2 = t3
      //   t3 = 0
      // L1:
      //   a.lo = t2
      //   a.hi = t3
      IceVariable *Tmp1, *Tmp2, *Tmp3;
      IceConstant *BitTest = Cfg->getConstantInt(IceType_i32, 0x20);
      IceConstant *Zero = Cfg->getConstantInt(IceType_i32, 0);
      IceInstX8632Label *Label = IceInstX8632Label::create(Cfg, this);
      Tmp1 = legalizeOperandToVar(Src1Lo, Context, false, Reg_ecx);
      Tmp2 = legalizeOperandToVar(Src0Lo, Context);
      Tmp3 = legalizeOperandToVar(Src0Hi, Context);
      Context.insert(IceInstX8632Shrd::create(Cfg, Tmp2, Tmp3, Tmp1));
      Context.insert(IceInstX8632Shr::create(Cfg, Tmp3, Tmp1));
      Context.insert(IceInstX8632Test::create(Cfg, Tmp1, BitTest));
      Context.insert(IceInstX8632Br::create(Cfg, Label, IceInstX8632Br::Br_e));
      Context.insert(IceInstFakeUse::create(Cfg, Tmp2));
      Context.insert(IceInstX8632Mov::create(Cfg, Tmp2, Tmp3));
      Context.insert(IceInstX8632Mov::create(Cfg, Tmp3, Zero));
      Context.insert(Label);
      Context.insert(IceInstX8632Mov::create(Cfg, DestLo, Tmp2));
      Context.insert(IceInstX8632Mov::create(Cfg, DestHi, Tmp3));
    } else {
      Reg1 = legalizeOperandToVar(Src0, Context);
      if (!llvm::isa<IceConstant>(Src1))
        Reg2 = legalizeOperandToVar(Src1, Context, false, Reg_ecx);
      Context.insert(IceInstX8632Shr::create(Cfg, Reg1, Reg2));
      Context.insert(IceInstX8632Mov::create(Cfg, Dest, Reg1));
    }
    break;
  case IceInstArithmetic::Ashr:
    if (LowerI64ToI32) {
      // a=b>>c (signed) ==>
      //   t1:ecx = c.lo & 0xff // via movzx, we can probably just t1=c.lo
      //   t2 = b.lo
      //   t3 = b.hi
      //   t2 = shrd t2, t3, t1
      //   t3 = sar t3, t1
      //   test t1, 0x20
      //   je L1
      //   use(t2)
      //   t2 = t3
      //   t3 = sar t3, 0x1f
      // L1:
      //   a.lo = t2
      //   a.hi = t3
      IceVariable *Tmp1, *Tmp2, *Tmp3;
      IceConstant *BitTest = Cfg->getConstantInt(IceType_i32, 0x20);
      IceConstant *SignExtend = Cfg->getConstantInt(IceType_i32, 0x1f);
      IceInstX8632Label *Label = IceInstX8632Label::create(Cfg, this);
      Tmp1 = legalizeOperandToVar(Src1Lo, Context, false, Reg_ecx);
      Tmp2 = legalizeOperandToVar(Src0Lo, Context);
      Tmp3 = legalizeOperandToVar(Src0Hi, Context);
      Context.insert(IceInstX8632Shrd::create(Cfg, Tmp2, Tmp3, Tmp1));
      Context.insert(IceInstX8632Sar::create(Cfg, Tmp3, Tmp1));
      Context.insert(IceInstX8632Test::create(Cfg, Tmp1, BitTest));
      Context.insert(IceInstX8632Br::create(Cfg, Label, IceInstX8632Br::Br_e));
      Context.insert(IceInstFakeUse::create(Cfg, Tmp2));
      Context.insert(IceInstX8632Mov::create(Cfg, Tmp2, Tmp3));
      // Context.insert(IceInstX8632Mov::create(Cfg, Tmp3, Zero));
      Context.insert(IceInstX8632Sar::create(Cfg, Tmp3, SignExtend));
      Context.insert(Label);
      Context.insert(IceInstX8632Mov::create(Cfg, DestLo, Tmp2));
      Context.insert(IceInstX8632Mov::create(Cfg, DestHi, Tmp3));
    } else {
      Reg1 = legalizeOperandToVar(Src0, Context);
      if (!llvm::isa<IceConstant>(Src1))
        Reg2 = legalizeOperandToVar(Src1, Context, false, Reg_ecx);
      Context.insert(IceInstX8632Sar::create(Cfg, Reg1, Reg2));
      Context.insert(IceInstX8632Mov::create(Cfg, Dest, Reg1));
    }
    break;
  case IceInstArithmetic::Udiv:
    if (LowerI64ToI32) {
      unsigned MaxSrcs = 2;
      // TODO: Figure out how to properly construct CallTarget.
      bool SuppressMangling = true;
      IceConstant *CallTarget = Cfg->getConstantSym(
          IceType_i32, NULL, 0, "__udivdi3", SuppressMangling);
      bool Tailcall = false;
      // TODO: This instruction leaks.
      IceInstCall *Call = IceInstCall::create(Cfg, MaxSrcs, Inst->getDest(),
                                              CallTarget, Tailcall);
      Call->addArg(Inst->getSrc(0));
      Call->addArg(Inst->getSrc(1));
      lowerCall(Call, Context);
      return;
    } else {
      Reg1 = legalizeOperandToVar(Src0, Context, false, Reg_eax);
      Reg0 = Cfg->makeVariable(IceType_i32, Context.Node);
      Reg0->setRegNum(Reg_edx);
      IceConstant *ConstZero = Cfg->getConstantInt(IceType_i32, 0);
      Context.insert(IceInstX8632Mov::create(Cfg, Reg0, ConstZero));
      Reg2 = legalizeOperand(Src1, Legal_All, Context);
      Context.insert(IceInstX8632Div::create(Cfg, Reg1, Reg2, Reg0));
      Context.insert(IceInstX8632Mov::create(Cfg, Dest, Reg1));
    }
    break;
  case IceInstArithmetic::Sdiv:
    if (LowerI64ToI32) {
      unsigned MaxSrcs = 2;
      // TODO: Figure out how to properly construct CallTarget.
      bool SuppressMangling = true;
      IceConstant *CallTarget = Cfg->getConstantSym(
          IceType_i32, NULL, 0, "__divdi3", SuppressMangling);
      bool Tailcall = false;
      // TODO: This instruction leaks.
      IceInstCall *Call = IceInstCall::create(Cfg, MaxSrcs, Inst->getDest(),
                                              CallTarget, Tailcall);
      Call->addArg(Inst->getSrc(0));
      Call->addArg(Inst->getSrc(1));
      lowerCall(Call, Context);
      return;
    } else {
      Reg1 = legalizeOperandToVar(Src0, Context, false, Reg_eax);
      Reg0 = Cfg->makeVariable(IceType_i32, Context.Node);
      Reg0->setRegNum(Reg_edx);
      Context.insert(IceInstX8632Cdq::create(Cfg, Reg0, Reg1));
      Reg2 = legalizeOperand(Src1, Legal_All, Context);
      Context.insert(IceInstX8632Idiv::create(Cfg, Reg1, Reg2, Reg0));
      Context.insert(IceInstX8632Mov::create(Cfg, Dest, Reg1));
    }
    break;
  case IceInstArithmetic::Urem:
    if (LowerI64ToI32) {
      unsigned MaxSrcs = 2;
      // TODO: Figure out how to properly construct CallTarget.
      bool SuppressMangling = true;
      IceConstant *CallTarget = Cfg->getConstantSym(
          IceType_i32, NULL, 0, "__umoddi3", SuppressMangling);
      bool Tailcall = false;
      // TODO: This instruction leaks.
      IceInstCall *Call = IceInstCall::create(Cfg, MaxSrcs, Inst->getDest(),
                                              CallTarget, Tailcall);
      Call->addArg(Inst->getSrc(0));
      Call->addArg(Inst->getSrc(1));
      lowerCall(Call, Context);
      return;
    } else {
      Reg1 = legalizeOperandToVar(Src0, Context, false, Reg_eax);
      Reg0 = Cfg->makeVariable(IceType_i32, Context.Node);
      Reg0->setRegNum(Reg_edx);
      IceConstant *ConstZero = Cfg->getConstantInt(IceType_i32, 0);
      Context.insert(IceInstX8632Mov::create(Cfg, Reg0, ConstZero));
      Reg2 = legalizeOperand(Src1, Legal_All, Context);
      Context.insert(IceInstX8632Div::create(Cfg, Reg0, Reg2, Reg1));
      Reg1 = Reg0;
      Context.insert(IceInstX8632Mov::create(Cfg, Dest, Reg1));
    }
    break;
  case IceInstArithmetic::Srem:
    if (LowerI64ToI32) {
      unsigned MaxSrcs = 2;
      // TODO: Figure out how to properly construct CallTarget.
      bool SuppressMangling = true;
      IceConstant *CallTarget = Cfg->getConstantSym(
          IceType_i32, NULL, 0, "__moddi3", SuppressMangling);
      bool Tailcall = false;
      // TODO: This instruction leaks.
      IceInstCall *Call = IceInstCall::create(Cfg, MaxSrcs, Inst->getDest(),
                                              CallTarget, Tailcall);
      Call->addArg(Inst->getSrc(0));
      Call->addArg(Inst->getSrc(1));
      lowerCall(Call, Context);
      return;
    } else {
      Reg1 = legalizeOperandToVar(Src0, Context, false, Reg_eax);
      Reg0 = Cfg->makeVariable(IceType_i32, Context.Node);
      Reg0->setRegNum(Reg_edx);
      Context.insert(IceInstX8632Cdq::create(Cfg, Reg0, Reg1));
      Reg2 = legalizeOperand(Src1, Legal_All, Context);
      Context.insert(IceInstX8632Idiv::create(Cfg, Reg0, Reg2, Reg1));
      Reg1 = Reg0;
      Context.insert(IceInstX8632Mov::create(Cfg, Dest, Reg1));
    }
    break;
  case IceInstArithmetic::Fadd:
    // t=src0; t=addss/addsd t, src1; dst=movss/movsd t
    Reg1 = legalizeOperandToVar(Src0, Context);
    Context.insert(IceInstX8632Addss::create(Cfg, Reg1, Src1));
    Context.insert(IceInstX8632Mov::create(Cfg, Dest, Reg1));
    break;
  case IceInstArithmetic::Fsub:
    Reg1 = legalizeOperandToVar(Src0, Context);
    Context.insert(IceInstX8632Subss::create(Cfg, Reg1, Src1));
    Context.insert(IceInstX8632Mov::create(Cfg, Dest, Reg1));
    break;
  case IceInstArithmetic::Fmul:
    Reg1 = legalizeOperandToVar(Src0, Context);
    Context.insert(IceInstX8632Mulss::create(Cfg, Reg1, Src1));
    Context.insert(IceInstX8632Mov::create(Cfg, Dest, Reg1));
    break;
  case IceInstArithmetic::Fdiv:
    Reg1 = legalizeOperandToVar(Src0, Context);
    Context.insert(IceInstX8632Divss::create(Cfg, Reg1, Src1));
    Context.insert(IceInstX8632Mov::create(Cfg, Dest, Reg1));
    break;
  case IceInstArithmetic::Frem: {
    unsigned MaxSrcs = 2;
    IceType Type = Dest->getType();
    // TODO: Figure out how to properly construct CallTarget.
    bool SuppressMangling = true;
    IceConstant *CallTarget = Cfg->getConstantSym(
        Type, NULL, 0, Type == IceType_f32 ? "fmodf" : "fmod",
        SuppressMangling);
    bool Tailcall = false;
    // TODO: This instruction leaks.
    IceInstCall *Call = IceInstCall::create(Cfg, MaxSrcs, Inst->getDest(),
                                            CallTarget, Tailcall);
    Call->addArg(Inst->getSrc(0));
    Call->addArg(Inst->getSrc(1));
    return lowerCall(Call, Context);
  } break;
  case IceInstArithmetic::OpKind_NUM:
    assert(0);
    break;
  }
}

void IceTargetX8632::lowerAssign(const IceInstAssign *Inst,
                                 IceLoweringContext &Context) {
  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = Inst->getSrc(0);
  if (Dest->getType() == IceType_i64) {
    // TODO: This seems broken if Src and Dest components are both on
    // the stack and not register-allocated.
    IceVariable *DestLo =
        llvm::cast<IceVariable>(makeLowOperand(Dest, Context));
    IceVariable *DestHi =
        llvm::cast<IceVariable>(makeHighOperand(Dest, Context));
    Context.insert(
        IceInstX8632Mov::create(Cfg, DestLo, makeLowOperand(Src0, Context)));
    Context.insert(
        IceInstX8632Mov::create(Cfg, DestHi, makeHighOperand(Src0, Context)));
    return;
  }
  // a=b ==> t=b; a=t; (link t->b)
  assert(Dest->getType() == Src0->getType());
  IceOperand *Reg = legalizeOperand(Src0, Legal_Reg | Legal_Imm, Context, true);
  Context.insert(IceInstX8632Mov::create(Cfg, Dest, Reg));
}

void IceTargetX8632::lowerBr(const IceInstBr *Inst,
                             IceLoweringContext &Context) {
  if (Inst->isUnconditional()) {
    Context.insert(IceInstX8632Br::create(Cfg, Inst->getTargetUnconditional()));
    return;
  }
  // cmp src, 0; br ne, labelTrue; br labelFalse
  IceOperand *Src = legalizeOperand(Inst->getSrc(0), Legal_All, Context);
  IceConstant *OpZero = Cfg->getConstantInt(IceType_i32, 0);
  Context.insert(IceInstX8632Icmp::create(Cfg, Src, OpZero));
  Context.insert(IceInstX8632Br::create(Cfg, Inst->getTargetTrue(),
                                        Inst->getTargetFalse(),
                                        IceInstX8632Br::Br_ne));
}

void IceTargetX8632::lowerCall(const IceInstCall *Inst,
                               IceLoweringContext &Context) {
  // TODO: what to do about tailcalls?
  // Generate a sequence of push instructions, pushing right to left,
  // keeping track of stack offsets in case a push involves a stack
  // operand and we are using an esp-based frame.
  uint32_t StackOffset = 0;
  // TODO: If for some reason the call instruction gets dead-code
  // eliminated after lowering, we would need to ensure that the
  // pre-call push instructions and the post-call esp adjustment get
  // eliminated as well.
  for (unsigned NumArgs = Inst->getNumArgs(), i = 0; i < NumArgs; ++i) {
    IceOperand *Arg = Inst->getArg(NumArgs - i - 1);
    Arg = legalizeOperand(Arg, Legal_All, Context);
    assert(Arg);
    if (Arg->getType() == IceType_i64) {
      Context.insert(
          IceInstX8632Push::create(Cfg, makeHighOperand(Arg, Context)));
      Context.insert(
          IceInstX8632Push::create(Cfg, makeLowOperand(Arg, Context)));
    } else if (Arg->getType() == IceType_f64) {
      // If the Arg turns out to be a memory operand, we need to push
      // 8 bytes, which requires two push instructions.  This ends up
      // being somewhat clumsy in the current IR, so we use a
      // workaround.  Force the operand into a (xmm) register, and
      // then push the register.  An xmm register push is actually not
      // possible in x86, but the Push instruction emitter handles
      // this by decrementing the stack pointer and directly writing
      // the xmm register value.
      IceVariable *Var = legalizeOperandToVar(Arg, Context);
      Context.insert(IceInstX8632Push::create(Cfg, Var));
    } else {
      Context.insert(IceInstX8632Push::create(Cfg, Arg));
    }
    StackOffset += typeWidthOnStack(Arg->getType());
  }
  // Generate the call instruction.  Assign its result to a temporary
  // with high register allocation weight.
  IceVariable *Dest = Inst->getDest();
  IceVariable *Reg = NULL; // doubles as RegLo as necessary
  IceVariable *RegHi = NULL;
  if (Dest) {
    switch (Dest->getType()) {
    case IceType_NUM:
      assert(0);
      break;
    case IceType_void:
      break;
    case IceType_i1:
    case IceType_i8:
    case IceType_i16:
    case IceType_i32:
      Reg = Cfg->makeVariable(Dest->getType(), Context.Node);
      Reg->setRegNum(Reg_eax);
      break;
    case IceType_i64:
      Reg = Cfg->makeVariable(IceType_i32, Context.Node);
      Reg->setRegNum(Reg_eax);
      RegHi = Cfg->makeVariable(IceType_i32, Context.Node);
      RegHi->setRegNum(Reg_edx);
      break;
    case IceType_f32:
    case IceType_f64:
      Reg = NULL;
      // Leave Reg==NULL, and capture the result with the fstp
      // instruction.
      break;
    }
  }
  IceOperand *CallTarget =
      legalizeOperand(Inst->getCallTarget(), Legal_All, Context);
  IceInst *NewCall =
      IceInstX8632Call::create(Cfg, Reg, CallTarget, Inst->isTail());
  Context.insert(NewCall);
  if (RegHi)
    Context.insert(IceInstFakeDef::create(Cfg, RegHi));

  // Insert a register-kill pseudo instruction.
  IceVarList KilledRegs;
  for (unsigned i = 0; i < ScratchRegs.size(); ++i) {
    if (ScratchRegs[i])
      KilledRegs.push_back(Cfg->getTarget()->getPhysicalRegister(i));
  }
  if (!KilledRegs.empty()) {
    IceInst *Kill = IceInstFakeKill::create(Cfg, KilledRegs, NewCall);
    Context.insert(Kill);
  }

  // Generate a FakeUse to keep the call live if necessary.
  if (Inst->hasSideEffects() && Reg) {
    IceInst *FakeUse = IceInstFakeUse::create(Cfg, Reg);
    Context.insert(FakeUse);
  }

  // Generate Dest=Reg assignment.
  if (Dest && Reg) {
    if (RegHi) {
      IceVariable *DestLo = Dest->getLow();
      IceVariable *DestHi = Dest->getHigh();
      DestLo->setPreferredRegister(Reg, false);
      Context.insert(IceInstX8632Mov::create(Cfg, DestLo, Reg));
      DestHi->setPreferredRegister(RegHi, false);
      Context.insert(IceInstX8632Mov::create(Cfg, DestHi, RegHi));
    } else {
      Dest->setPreferredRegister(Reg, false);
      Context.insert(IceInstX8632Mov::create(Cfg, Dest, Reg));
    }
  }

  // Special treatment for an FP function which returns its result in
  // st(0).
  if (Dest &&
      (Dest->getType() == IceType_f32 || Dest->getType() == IceType_f64)) {
    Context.insert(IceInstX8632Fstp::create(Cfg, Dest));
    // If Dest ends up being a physical xmm register, the fstp emit
    // code will route st(0) through a temporary stack slot.
  }

  // Add the appropriate offset to esp.
  if (StackOffset) {
    IceVariable *Esp = Cfg->getTarget()->getPhysicalRegister(Reg_esp);
    Context.insert(IceInstX8632Add::create(
        Cfg, Esp, Cfg->getConstantInt(IceType_i32, StackOffset)));
  }
}

void IceTargetX8632::lowerCast(const IceInstCast *Inst,
                               IceLoweringContext &Context) {
  // a = cast(b) ==> t=cast(b); a=t; (link t->b, link a->t, no overlap)
  IceInstCast::OpKind CastKind = Inst->getCastKind();
  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = Inst->getSrc(0);
  IceOperand *Reg = legalizeOperand(Src0, Legal_Reg | Legal_Mem, Context, true);
  switch (CastKind) {
  default:
    // TODO: implement other sorts of casts.
    Cfg->setError("Cast type not yet supported");
    return;
    break;
  case IceInstCast::Sext:
    if (Dest->getType() == IceType_i64) {
      // t1=movsx src; t2=t1; t2=sar t2, 31; dst.lo=t1; dst.hi=t2
      IceVariable *DestLo =
          llvm::cast<IceVariable>(makeLowOperand(Dest, Context));
      IceVariable *DestHi =
          llvm::cast<IceVariable>(makeHighOperand(Dest, Context));
      if (Reg->getType() == IceType_i32)
        Context.insert(IceInstX8632Mov::create(Cfg, DestLo, Reg));
      else
        Context.insert(IceInstX8632Movsx::create(Cfg, DestLo, Reg));
      IceVariable *RegHi = Cfg->makeVariable(IceType_i32, Context.Node);
      IceConstant *Shift = Cfg->getConstantInt(IceType_i32, 31);
      Context.insert(IceInstX8632Mov::create(Cfg, RegHi, Reg));
      Context.insert(IceInstX8632Sar::create(Cfg, RegHi, Shift));
      Context.insert(IceInstX8632Mov::create(Cfg, DestHi, RegHi));
    } else {
      // TODO: Sign-extend an i1 via "shl reg, 31; sar reg, 31", and
      // also copy to the high operand of a 64-bit variable.
      Context.insert(IceInstX8632Movsx::create(Cfg, Dest, Reg));
    }
    break;
  case IceInstCast::Zext:
    if (Dest->getType() == IceType_i64) {
      // t1=movzx src; dst.lo=t1; dst.hi=0
      IceConstant *Zero = Cfg->getConstantInt(IceType_i32, 0);
      IceVariable *DestLo =
          llvm::cast<IceVariable>(makeLowOperand(Dest, Context));
      IceVariable *DestHi =
          llvm::cast<IceVariable>(makeHighOperand(Dest, Context));
      if (Reg->getType() == IceType_i32)
        Context.insert(IceInstX8632Mov::create(Cfg, DestLo, Reg));
      else
        Context.insert(IceInstX8632Movzx::create(Cfg, DestLo, Reg));
      Context.insert(IceInstX8632Mov::create(Cfg, DestHi, Zero));
    } else if (Reg->getType() == IceType_i1) {
      // t = Reg; t &= 1; Dest = t
      IceOperand *ConstOne = Cfg->getConstantInt(IceType_i32, 1);
      IceVariable *Tmp = Cfg->makeVariable(IceType_i32, Context.Node);
      Tmp->setWeightInfinite();
      Context.insert(IceInstX8632Movzx::create(Cfg, Tmp, Reg));
      Context.insert(IceInstX8632And::create(Cfg, Tmp, ConstOne));
      Context.insert(IceInstX8632Mov::create(Cfg, Dest, Tmp));
    } else {
      Context.insert(IceInstX8632Movzx::create(Cfg, Dest, Reg));
    }
    break;
  case IceInstCast::Trunc: {
    if (Reg->getType() == IceType_i64)
      Reg = makeLowOperand(Reg, Context);
    // t1 = Reg; t2 = trunc t1; Dest = t2
    IceVariable *Tmp1 = legalizeOperandToVar(Reg, Context);
    IceVariable *Tmp2 = Cfg->makeVariable(Dest->getType(), Context.Node);
    Tmp2->setWeightInfinite();
    Tmp2->setPreferredRegister(Tmp1, true);
    Context.insert(IceInstX8632Mov::create(Cfg, Tmp2, Tmp1));
    Context.insert(IceInstX8632Mov::create(Cfg, Dest, Tmp2));
    break;
  }
  case IceInstCast::Fptrunc:
  case IceInstCast::Fpext:
    Context.insert(IceInstX8632Cvt::create(Cfg, Dest, Reg));
    break;
  case IceInstCast::Fptosi:
    if (Dest->getType() == IceType_i64) {
      // Use a helper for converting floating-point values to 64-bit
      // integers.  SSE2 appears to have no way to convert from xmm
      // registers to something like the edx:eax register pair, and
      // gcc and clang both want to use x87 instructions complete with
      // temporary manipulation of the status word.  This helper is
      // not needed for x86-64.
      split64(Dest, Context);
      unsigned MaxSrcs = 1;
      IceType SrcType = Inst->getSrc(0)->getType();
      // TODO: Figure out how to properly construct CallTarget.
      bool SuppressMangling = true;
      IceConstant *CallTarget = Cfg->getConstantSym(
          IceType_i64, NULL, 0,
          SrcType == IceType_f32 ? "cvtftosi64" : "cvtdtosi64",
          SuppressMangling);
      bool Tailcall = false;
      // TODO: This instruction leaks.
      IceInstCall *Call = IceInstCall::create(Cfg, MaxSrcs, Inst->getDest(),
                                              CallTarget, Tailcall);
      Call->addArg(Inst->getSrc(0));
      lowerCall(Call, Context);
      return;
    } else {
      // t1.i32 = cvt Reg; t2.dest_type = t1; Dest = t2.dest_type
      IceVariable *Tmp1 = Cfg->makeVariable(IceType_i32, Context.Node);
      Tmp1->setWeightInfinite();
      Context.insert(IceInstX8632Cvt::create(Cfg, Tmp1, Reg));
      IceVariable *Tmp2 = Cfg->makeVariable(Dest->getType(), Context.Node);
      Tmp2->setWeightInfinite();
      Tmp2->setPreferredRegister(Tmp1, true);
      Context.insert(IceInstX8632Mov::create(Cfg, Tmp2, Tmp1));
      Context.insert(IceInstX8632Mov::create(Cfg, Dest, Tmp2));
      // Sign-extend the result if necessary.
    }
    break;
  case IceInstCast::Fptoui:
    if (Dest->getType() == IceType_i64 || Dest->getType() == IceType_i32) {
      // Use a helper for both x86-32 and x86-64.
      split64(Dest, Context);
      unsigned MaxSrcs = 1;
      IceType DestType = Inst->getDest()->getType();
      IceType SrcType = Inst->getSrc(0)->getType();
      IceString DstSubstring = (DestType == IceType_i64 ? "64" : "32");
      IceString SrcSubstring = (SrcType == IceType_f32 ? "f" : "d");
      // Possibilities are cvtftoui32, cvtdtoui32, cvtftoui64, cvtdtoui64
      IceString TargetString = "cvt" + SrcSubstring + "toui" + DstSubstring;
      // TODO: Figure out how to properly construct CallTarget.
      bool SuppressMangling = true;
      IceConstant *CallTarget = Cfg->getConstantSym(
          IceType_i64, NULL, 0, TargetString, SuppressMangling);
      bool Tailcall = false;
      // TODO: This instruction leaks.
      IceInstCall *Call = IceInstCall::create(Cfg, MaxSrcs, Inst->getDest(),
                                              CallTarget, Tailcall);
      Call->addArg(Inst->getSrc(0));
      lowerCall(Call, Context);
      return;
    } else {
      // t1.i32 = cvt Reg; t2.dest_type = t1; Dest = t2.dest_type
      IceVariable *Tmp1 = Cfg->makeVariable(IceType_i32, Context.Node);
      Tmp1->setWeightInfinite();
      Context.insert(IceInstX8632Cvt::create(Cfg, Tmp1, Reg));
      IceVariable *Tmp2 = Cfg->makeVariable(Dest->getType(), Context.Node);
      Tmp2->setWeightInfinite();
      Tmp2->setPreferredRegister(Tmp1, true);
      Context.insert(IceInstX8632Mov::create(Cfg, Tmp2, Tmp1));
      Context.insert(IceInstX8632Mov::create(Cfg, Dest, Tmp2));
      // Zero-extend the result if necessary.
    }
    break;
  case IceInstCast::Sitofp:
    if (Reg->getType() == IceType_i64) {
      // Use a helper for x86-32.
      unsigned MaxSrcs = 1;
      IceType DestType = Inst->getDest()->getType();
      // TODO: Figure out how to properly construct CallTarget.
      bool SuppressMangling = true;
      IceConstant *CallTarget = Cfg->getConstantSym(
          DestType, NULL, 0,
          DestType == IceType_f32 ? "cvtsi64tof" : "cvtsi64tod",
          SuppressMangling);
      bool Tailcall = false;
      // TODO: This instruction leaks.
      IceInstCall *Call = IceInstCall::create(Cfg, MaxSrcs, Inst->getDest(),
                                              CallTarget, Tailcall);
      Call->addArg(Inst->getSrc(0));
      lowerCall(Call, Context);
      return;
    } else {
      // Sign-extend the operand.
      // t1.i32 = movsx Reg; t2 = Cvt t1.i32; Dest = t2
      IceVariable *Tmp1 = Cfg->makeVariable(IceType_i32, Context.Node);
      Tmp1->setWeightInfinite();
      if (Reg->getType() == IceType_i32)
        Context.insert(IceInstX8632Mov::create(Cfg, Tmp1, Reg));
      else
        Context.insert(IceInstX8632Movsx::create(Cfg, Tmp1, Reg));
      IceVariable *Tmp2 = Cfg->makeVariable(Dest->getType(), Context.Node);
      Tmp2->setWeightInfinite();
      Context.insert(IceInstX8632Cvt::create(Cfg, Tmp2, Tmp1));
      Context.insert(IceInstX8632Mov::create(Cfg, Dest, Tmp2));
    }
    break;
  case IceInstCast::Uitofp:
    if (Reg->getType() == IceType_i64 || Reg->getType() == IceType_i32) {
      // Use a helper for x86-32 and x86-64.  Also use a helper for
      // i32 on x86-32.
      unsigned MaxSrcs = 1;
      IceType DestType = Inst->getDest()->getType();
      IceString SrcSubstring = (Reg->getType() == IceType_i64 ? "64" : "32");
      IceString DstSubstring = (DestType == IceType_f32 ? "f" : "d");
      // Possibilities are cvtui32tof, cvtui32tod, cvtui64tof, cvtui64tod
      IceString TargetString = "cvtui" + SrcSubstring + "to" + DstSubstring;
      // TODO: Figure out how to properly construct CallTarget.
      bool SuppressMangling = true;
      IceConstant *CallTarget = Cfg->getConstantSym(
          DestType, NULL, 0, TargetString, SuppressMangling);
      bool Tailcall = false;
      // TODO: This instruction leaks.
      IceInstCall *Call = IceInstCall::create(Cfg, MaxSrcs, Inst->getDest(),
                                              CallTarget, Tailcall);
      Call->addArg(Inst->getSrc(0));
      lowerCall(Call, Context);
      return;
    } else {
      // Zero-extend the operand.
      // t1.i32 = movzx Reg; t2 = Cvt t1.i32; Dest = t2
      IceVariable *Tmp1 = Cfg->makeVariable(IceType_i32, Context.Node);
      Tmp1->setWeightInfinite();
      if (Reg->getType() == IceType_i32)
        Context.insert(IceInstX8632Mov::create(Cfg, Tmp1, Reg));
      else
        Context.insert(IceInstX8632Movzx::create(Cfg, Tmp1, Reg));
      IceVariable *Tmp2 = Cfg->makeVariable(Dest->getType(), Context.Node);
      Tmp2->setWeightInfinite();
      Context.insert(IceInstX8632Cvt::create(Cfg, Tmp2, Tmp1));
      Context.insert(IceInstX8632Mov::create(Cfg, Dest, Tmp2));
    }
    break;
  }
}

static struct {
  IceInstFcmp::FCond Cond;
  unsigned Default;
  bool SwapOperands;
  IceInstX8632Br::BrCond C1, C2;
} TableFcmp[] = {
    { IceInstFcmp::False, 0, false, IceInstX8632Br::Br_None,
      IceInstX8632Br::Br_None },
    { IceInstFcmp::Oeq, 0, false, IceInstX8632Br::Br_ne, IceInstX8632Br::Br_p },
    { IceInstFcmp::Ogt, 1, false, IceInstX8632Br::Br_a,
      IceInstX8632Br::Br_None },
    { IceInstFcmp::Oge, 1, false, IceInstX8632Br::Br_ae,
      IceInstX8632Br::Br_None },
    { IceInstFcmp::Olt, 1, true, IceInstX8632Br::Br_a,
      IceInstX8632Br::Br_None },
    { IceInstFcmp::Ole, 1, true, IceInstX8632Br::Br_ae,
      IceInstX8632Br::Br_None },
    { IceInstFcmp::One, 1, false, IceInstX8632Br::Br_ne,
      IceInstX8632Br::Br_None },
    { IceInstFcmp::Ord, 1, false, IceInstX8632Br::Br_np,
      IceInstX8632Br::Br_None },
    { IceInstFcmp::Ueq, 1, false, IceInstX8632Br::Br_e,
      IceInstX8632Br::Br_None },
    { IceInstFcmp::Ugt, 1, true, IceInstX8632Br::Br_b,
      IceInstX8632Br::Br_None },
    { IceInstFcmp::Uge, 1, true, IceInstX8632Br::Br_be,
      IceInstX8632Br::Br_None },
    { IceInstFcmp::Ult, 1, false, IceInstX8632Br::Br_b,
      IceInstX8632Br::Br_None },
    { IceInstFcmp::Ule, 1, false, IceInstX8632Br::Br_be,
      IceInstX8632Br::Br_None },
    { IceInstFcmp::Une, 1, false, IceInstX8632Br::Br_ne, IceInstX8632Br::Br_p },
    { IceInstFcmp::Uno, 1, false, IceInstX8632Br::Br_p,
      IceInstX8632Br::Br_None },
    { IceInstFcmp::True, 1, false, IceInstX8632Br::Br_None,
      IceInstX8632Br::Br_None },
  };
const static unsigned TableFcmpSize = sizeof(TableFcmp) / sizeof(*TableFcmp);

void IceTargetX8632::lowerFcmp(const IceInstFcmp *Inst,
                               IceLoweringContext &Context) {
  IceOperand *Src0 = Inst->getSrc(0);
  IceOperand *Src1 = Inst->getSrc(1);
  IceVariable *Dest = Inst->getDest();
  // Lowering a = fcmp cond, b, c
  // ucomiss b, c (only if C1 != Br_None)
  //   (but swap b,c order if SwapOperands==true)
  // mov a, <default>
  // j<C1> label (only if C1 != Br_None)
  // j<C2> label (only if C2 != Br_None)
  // FakeUse(a) (only if C1 != Br_None)
  // mov a, !<default> (only if C1 != Br_None)
  // label: (only if C1 != Br_None)
  IceInstFcmp::FCond Condition = Inst->getCondition();
  unsigned Index = static_cast<unsigned>(Condition);
  assert(Index < TableFcmpSize);
  assert(TableFcmp[Index].Cond == Condition);
  if (TableFcmp[Index].SwapOperands) {
    IceOperand *Tmp = Src0;
    Src0 = Src1;
    Src1 = Tmp;
  }
  bool HasC1 = (TableFcmp[Index].C1 != IceInstX8632Br::Br_None);
  bool HasC2 = (TableFcmp[Index].C2 != IceInstX8632Br::Br_None);
  if (HasC1) {
    Src0 = legalizeOperandToVar(Src0, Context);
    Src1 = legalizeOperand(Src1, Legal_All, Context);
    Context.insert(IceInstX8632Ucomiss::create(Cfg, Src0, Src1));
  }
  IceConstant *Default =
      Cfg->getConstantInt(IceType_i32, TableFcmp[Index].Default);
  Context.insert(IceInstX8632Mov::create(Cfg, Dest, Default));
  if (HasC1) {
    IceInstX8632Label *Label = IceInstX8632Label::create(Cfg, this);
    Context.insert(IceInstX8632Br::create(Cfg, Label, TableFcmp[Index].C1));
    if (HasC2) {
      Context.insert(IceInstX8632Br::create(Cfg, Label, TableFcmp[Index].C2));
    }
    Context.insert(IceInstFakeUse::create(Cfg, Dest));
    IceConstant *NonDefault =
        Cfg->getConstantInt(IceType_i32, !TableFcmp[Index].Default);
    Context.insert(IceInstX8632Mov::create(Cfg, Dest, NonDefault));
    Context.insert(Label);
  }
}

static struct {
  IceInstIcmp::ICond Cond;
  IceInstX8632Br::BrCond C1, C2, C3;
} TableIcmp64[] = { { IceInstIcmp::Eq }, { IceInstIcmp::Ne },
                    { IceInstIcmp::Ugt, IceInstX8632Br::Br_a,
                      IceInstX8632Br::Br_b, IceInstX8632Br::Br_a },
                    { IceInstIcmp::Uge, IceInstX8632Br::Br_a,
                      IceInstX8632Br::Br_b, IceInstX8632Br::Br_ae },
                    { IceInstIcmp::Ult, IceInstX8632Br::Br_b,
                      IceInstX8632Br::Br_a, IceInstX8632Br::Br_b },
                    { IceInstIcmp::Ule, IceInstX8632Br::Br_b,
                      IceInstX8632Br::Br_a, IceInstX8632Br::Br_be },
                    { IceInstIcmp::Sgt, IceInstX8632Br::Br_g,
                      IceInstX8632Br::Br_l, IceInstX8632Br::Br_a },
                    { IceInstIcmp::Sge, IceInstX8632Br::Br_g,
                      IceInstX8632Br::Br_l, IceInstX8632Br::Br_ae },
                    { IceInstIcmp::Slt, IceInstX8632Br::Br_l,
                      IceInstX8632Br::Br_g, IceInstX8632Br::Br_b },
                    { IceInstIcmp::Sle, IceInstX8632Br::Br_l,
                      IceInstX8632Br::Br_g, IceInstX8632Br::Br_be }, };
const static unsigned TableIcmp64Size =
    sizeof(TableIcmp64) / sizeof(*TableIcmp64);

static struct {
  IceInstIcmp::ICond Cond;
  IceInstX8632Br::BrCond Mapping;
} TableIcmp32[] = { { IceInstIcmp::Eq, IceInstX8632Br::Br_e },
                    { IceInstIcmp::Ne, IceInstX8632Br::Br_ne },
                    { IceInstIcmp::Ugt, IceInstX8632Br::Br_a },
                    { IceInstIcmp::Uge, IceInstX8632Br::Br_ae },
                    { IceInstIcmp::Ult, IceInstX8632Br::Br_b },
                    { IceInstIcmp::Ule, IceInstX8632Br::Br_be },
                    { IceInstIcmp::Sgt, IceInstX8632Br::Br_g },
                    { IceInstIcmp::Sge, IceInstX8632Br::Br_ge },
                    { IceInstIcmp::Slt, IceInstX8632Br::Br_l },
                    { IceInstIcmp::Sle, IceInstX8632Br::Br_le }, };
const static unsigned TableIcmp32Size =
    sizeof(TableIcmp32) / sizeof(*TableIcmp32);

static IceInstX8632Br::BrCond getIcmp32Mapping(IceInstIcmp::ICond Cond) {
  unsigned Index = static_cast<unsigned>(Cond);
  assert(Index < TableIcmp32Size);
  assert(TableIcmp32[Index].Cond == Cond);
  return TableIcmp32[Index].Mapping;
}

void IceTargetX8632::lowerIcmp(const IceInstIcmp *Inst,
                               IceLoweringContext &Context) {
  IceOperand *Src0 = Inst->getSrc(0);
  IceOperand *Src1 = Inst->getSrc(1);
  IceVariable *Dest = Inst->getDest();
  if (IceInstBr *NextBr =
          llvm::dyn_cast_or_null<IceInstBr>(Context.getNextInst())) {
    if (Src0->getType() != IceType_i64 && !NextBr->isUnconditional() &&
        Dest == NextBr->getSrc(0) && NextBr->isLastUse(Dest)) {
      // This is basically identical to an Arithmetic instruction,
      // except there is no Dest variable to store.
      // cmp a,b ==> mov t,a; cmp t,b
      bool IsImmOrReg = false;
      if (llvm::isa<IceConstant>(Src1))
        IsImmOrReg = true;
      else if (IceVariable *Var = llvm::dyn_cast<IceVariable>(Src1)) {
        if (Var->getRegNum() >= 0)
          IsImmOrReg = true;
      }
      IceOperand *Reg = legalizeOperand(
          Src0, IsImmOrReg ? Legal_All : Legal_Reg, Context, true);
      Context.insert(IceInstX8632Icmp::create(Cfg, Reg, Src1));
      Context.insert(IceInstX8632Br::create(
          Cfg, NextBr->getTargetTrue(), NextBr->getTargetFalse(),
          getIcmp32Mapping(Inst->getCondition())));
      NextBr->setDeleted();
      Context.advanceNext();
      return;
    }
  }

  // a=icmp cond, b, c ==> cmp b,c; a=1; br cond,L1; FakeUse(a); a=0; L1:
  //
  // Alternative without intra-block branch: cmp b,c; a=0; a=set<cond> {a}
  IceOperand *ConstZero = Cfg->getConstantInt(IceType_i32, 0);
  IceOperand *ConstOne = Cfg->getConstantInt(IceType_i32, 1);
  if (Src0->getType() == IceType_i64) {
    IceInstIcmp::ICond Condition = Inst->getCondition();
    unsigned Index = static_cast<unsigned>(Condition);
    assert(Index < TableIcmp64Size);
    assert(TableIcmp64[Index].Cond == Condition);
    IceInstX8632Label *LabelFalse = IceInstX8632Label::create(Cfg, this);
    IceInstX8632Label *LabelTrue = IceInstX8632Label::create(Cfg, this);
    Src0 = legalizeOperand(Src0, Legal_All, Context);
    Src1 = legalizeOperand(Src1, Legal_All, Context);
    if (Condition == IceInstIcmp::Eq || Condition == IceInstIcmp::Ne) {
      Context.insert(IceInstX8632Mov::create(
          Cfg, Dest, (Condition == IceInstIcmp::Eq ? ConstZero : ConstOne)));
      IceOperand *RegHi = legalizeOperand(makeHighOperand(Src1, Context),
                                          Legal_Reg | Legal_Imm, Context);
      Context.insert(
          IceInstX8632Icmp::create(Cfg, makeHighOperand(Src0, Context), RegHi));
      Context.insert(
          IceInstX8632Br::create(Cfg, LabelFalse, IceInstX8632Br::Br_ne));
      IceOperand *RegLo = legalizeOperand(makeLowOperand(Src1, Context),
                                          Legal_Reg | Legal_Imm, Context);
      Context.insert(
          IceInstX8632Icmp::create(Cfg, makeLowOperand(Src0, Context), RegLo));
      Context.insert(
          IceInstX8632Br::create(Cfg, LabelFalse, IceInstX8632Br::Br_ne));
      Context.insert(LabelTrue);
      Context.insert(IceInstFakeUse::create(Cfg, Dest));
      Context.insert(IceInstX8632Mov::create(
          Cfg, Dest, (Condition == IceInstIcmp::Eq ? ConstOne : ConstZero)));
      Context.insert(LabelFalse);
    } else {
      Context.insert(IceInstX8632Mov::create(Cfg, Dest, ConstOne));
      IceOperand *RegHi = legalizeOperand(makeHighOperand(Src1, Context),
                                          Legal_Reg | Legal_Imm, Context);
      Context.insert(
          IceInstX8632Icmp::create(Cfg, makeHighOperand(Src0, Context), RegHi));
      Context.insert(
          IceInstX8632Br::create(Cfg, LabelTrue, TableIcmp64[Index].C1));
      Context.insert(
          IceInstX8632Br::create(Cfg, LabelFalse, TableIcmp64[Index].C2));
      IceOperand *RegLo = legalizeOperand(makeLowOperand(Src1, Context),
                                          Legal_Reg | Legal_Imm, Context);
      Context.insert(
          IceInstX8632Icmp::create(Cfg, makeLowOperand(Src0, Context), RegLo));
      Context.insert(
          IceInstX8632Br::create(Cfg, LabelTrue, TableIcmp64[Index].C3));
      Context.insert(LabelFalse);
      Context.insert(IceInstFakeUse::create(Cfg, Dest));
      Context.insert(IceInstX8632Mov::create(Cfg, Dest, ConstZero));
      Context.insert(LabelTrue);
    }
    return;
  }
  // cmp b, c
  bool IsImmOrReg = false;
  if (llvm::isa<IceConstant>(Src1))
    IsImmOrReg = true;
  else if (IceVariable *Var = llvm::dyn_cast<IceVariable>(Src1)) {
    if (Var->getRegNum() >= 0)
      IsImmOrReg = true;
  }
  IceOperand *Reg =
      legalizeOperand(Src0, IsImmOrReg ? Legal_All : Legal_Reg, Context, true);
  Context.insert(IceInstX8632Icmp::create(Cfg, Reg, Src1));

  // a = 1;
  Context.insert(
      IceInstX8632Mov::create(Cfg, Dest, Cfg->getConstantInt(IceType_i32, 1)));

  // create Label
  IceInstX8632Label *Label = IceInstX8632Label::create(Cfg, this);

  // br cond, Label
  Context.insert(IceInstX8632Br::create(
      Cfg, Label, getIcmp32Mapping(Inst->getCondition())));

  // FakeUse(a)
  IceInst *FakeUse = IceInstFakeUse::create(Cfg, Dest);
  Context.insert(FakeUse);

  // a = 0
  Context.insert(
      IceInstX8632Mov::create(Cfg, Dest, Cfg->getConstantInt(IceType_i32, 0)));

  // Label:
  Context.insert(Label);
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
        uint32_t Mult = IndexConstant1->getValue();
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

void IceTargetX8632::lowerLoad(const IceInstLoad *Inst,
                               IceLoweringContext &Context) {
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
  if (IceInstArithmetic *Arith =
          llvm::dyn_cast_or_null<IceInstArithmetic>(Context.getNextInst())) {
    IceVariable *DestLoad = Inst->getDest();
    IceVariable *Src0Arith = llvm::dyn_cast<IceVariable>(Arith->getSrc(0));
    IceVariable *Src1Arith = llvm::dyn_cast<IceVariable>(Arith->getSrc(1));
    if (Src1Arith == DestLoad && Arith->isLastUse(Src1Arith) &&
        DestLoad != Src0Arith) {
      // TODO: This instruction leaks.
      IceInstArithmetic *NewArith = IceInstArithmetic::create(
          Cfg, Arith->getOp(), Arith->getDest(), Arith->getSrc(0), Src);
      NewArith->setDeleted();
      Context.advanceNext();
      lowerArithmetic(NewArith, Context);
      return;
    } else if (Src0Arith == DestLoad && Arith->isCommutative() &&
               Arith->isLastUse(Src0Arith) && DestLoad != Src1Arith) {
      // TODO: This instruction leaks.
      IceInstArithmetic *NewArith = IceInstArithmetic::create(
          Cfg, Arith->getOp(), Arith->getDest(), Arith->getSrc(1), Src);
      NewArith->setDeleted();
      Context.advanceNext();
      lowerArithmetic(NewArith, Context);
      return;
    }
  }

  // TODO: This instruction leaks.
  IceInstAssign *Assign = IceInstAssign::create(Cfg, Inst->getDest(), Src);
  lowerAssign(Assign, Context);
}

void IceTargetX8632::doAddressOptLoad(IceLoweringContext &Context) {
  IceInst *Inst = *Context.Cur;
  IceVariable *Dest = Inst->getDest();
  IceOperand *Addr = Inst->getSrc(0);
  IceVariable *Index = NULL;
  int Shift = 0;
  int32_t Offset = 0; // TODO: make IceConstant
  IceVariable *Base = llvm::dyn_cast<IceVariable>(Addr);
  computeAddressOpt(Cfg, Base, Index, Shift, Offset);
  if (Base && Addr != Base) {
    IceConstant *OffsetOp = Cfg->getConstantInt(IceType_i32, Offset);
    Addr = IceOperandX8632Mem::create(Cfg, Dest->getType(), Base, OffsetOp,
                                      Index, Shift);
    Inst->setDeleted();
    Context.insert(IceInstLoad::create(Cfg, Dest, Addr));
  }
}

void IceTargetX8632::lowerPhi(const IceInstPhi *Inst,
                              IceLoweringContext &Context) {
  Cfg->setError("Phi lowering not implemented");
}

void IceTargetX8632::lowerRet(const IceInstRet *Inst,
                              IceLoweringContext &Context) {
  IceVariable *Reg = NULL;
  if (Inst->getSrcSize()) {
    IceOperand *Src0 = legalizeOperand(Inst->getSrc(0), Legal_All, Context);
    if (Src0->getType() == IceType_i64) {
      IceVariable *Src0Low = legalizeOperandToVar(makeLowOperand(Src0, Context),
                                                  Context, false, Reg_eax);
      IceVariable *Src0High = legalizeOperandToVar(
          makeHighOperand(Src0, Context), Context, false, Reg_edx);
      Reg = Src0Low;
      Context.insert(IceInstFakeUse::create(Cfg, Src0High));
    } else if (Src0->getType() == IceType_f32 ||
               Src0->getType() == IceType_f64) {
      Context.insert(IceInstX8632Fld::create(Cfg, Src0));
    } else {
      Reg = legalizeOperandToVar(Src0, Context, false, Reg_eax);
    }
  }
  Context.insert(IceInstX8632Ret::create(Cfg, Reg));
  // Add a fake use of esp to make sure esp stays alive for the entire
  // function.  Otherwise post-call esp adjustments get dead-code
  // eliminated.  TODO: Are there more places where the fake use
  // should be inserted?  E.g. "void f(int n){while(1) g(n);}" may not
  // have a ret instruction.
  IceVariable *Esp = Cfg->getTarget()->getPhysicalRegister(Reg_esp);
  IceInst *FakeUse = IceInstFakeUse::create(Cfg, Esp);
  Context.insert(FakeUse);
}

void IceTargetX8632::lowerSelect(const IceInstSelect *Inst,
                                 IceLoweringContext &Context) {
  // a=d?b:c ==> cmp d,0; a=b; jne L1; FakeUse(a); a=c; L1:
  //
  // Alternative if a is reg and c is not imm: cmp d,0; a=b; a=cmoveq c {a}
  IceOperand *Condition =
      legalizeOperand(Inst->getCondition(), Legal_All, Context);
  IceConstant *OpZero = Cfg->getConstantInt(IceType_i32, 0);
  Context.insert(IceInstX8632Icmp::create(Cfg, Condition, OpZero));

  IceVariable *Dest = Inst->getDest();
  bool IsI64 = (Dest->getType() == IceType_i64);
  IceVariable *DestLo = NULL;
  IceVariable *DestHi = NULL;
  IceOperand *SrcTrue = Inst->getTrueOperand();
  if (IsI64) {
    DestLo = llvm::cast<IceVariable>(makeLowOperand(Dest, Context));
    DestHi = llvm::cast<IceVariable>(makeHighOperand(Dest, Context));
    IceOperand *SrcTrueHi = makeHighOperand(SrcTrue, Context);
    IceOperand *SrcTrueLo = makeLowOperand(SrcTrue, Context);
    IceOperand *RegHi =
        legalizeOperand(SrcTrueHi, Legal_Reg | Legal_Imm, Context, true);
    Context.insert(IceInstX8632Mov::create(Cfg, DestHi, RegHi));
    IceOperand *RegLo =
        legalizeOperand(SrcTrueLo, Legal_Reg | Legal_Imm, Context, true);
    Context.insert(IceInstX8632Mov::create(Cfg, DestLo, RegLo));
  } else {
    SrcTrue = legalizeOperand(SrcTrue, Legal_Reg | Legal_Imm, Context, true);
    Context.insert(IceInstX8632Mov::create(Cfg, Dest, SrcTrue));
  }

  // create Label
  IceInstX8632Label *Label = IceInstX8632Label::create(Cfg, this);

  Context.insert(IceInstX8632Br::create(Cfg, Label, IceInstX8632Br::Br_ne));

  // FakeUse(a)
  if (IsI64) {
    Context.insert(IceInstFakeUse::create(Cfg, DestLo));
    Context.insert(IceInstFakeUse::create(Cfg, DestHi));
  } else {
    Context.insert(IceInstFakeUse::create(Cfg, Dest));
  }

  IceOperand *SrcFalse = Inst->getFalseOperand();
  if (IsI64) {
    IceOperand *SrcFalseHi = makeHighOperand(SrcFalse, Context);
    IceOperand *SrcFalseLo = makeLowOperand(SrcFalse, Context);
    IceOperand *RegHi =
        legalizeOperand(SrcFalseHi, Legal_Reg | Legal_Imm, Context, true);
    Context.insert(IceInstX8632Mov::create(Cfg, DestHi, RegHi));
    IceOperand *RegLo =
        legalizeOperand(SrcFalseLo, Legal_Reg | Legal_Imm, Context, true);
    Context.insert(IceInstX8632Mov::create(Cfg, DestLo, RegLo));
  } else {
    SrcFalse = legalizeOperand(SrcFalse, Legal_Reg | Legal_Imm, Context, true);
    Context.insert(IceInstX8632Mov::create(Cfg, Dest, SrcFalse));
  }

  // Label:
  Context.insert(Label);
}

void IceTargetX8632::lowerStore(const IceInstStore *Inst,
                                IceLoweringContext &Context) {
  IceOperand *Value = Inst->getData();
  IceOperand *Addr = Inst->getAddr();
  IceOperandX8632Mem *NewAddr = llvm::dyn_cast<IceOperandX8632Mem>(Addr);
  if (!NewAddr) {
    IceVariable *Base = llvm::dyn_cast<IceVariable>(Addr);
    IceConstant *Offset = llvm::dyn_cast<IceConstant>(Addr);
    assert(Base || Offset);
    NewAddr = IceOperandX8632Mem::create(Cfg, Value->getType(), Base, Offset);
  }
  NewAddr = llvm::cast<IceOperandX8632Mem>(
      legalizeOperand(NewAddr, Legal_All, Context));

  if (NewAddr->getType() == IceType_i64) {
    Value = legalizeOperand(Value, Legal_All, Context);
    IceOperand *ValueHi = makeHighOperand(Value, Context);
    IceOperand *ValueLo = makeLowOperand(Value, Context);
    Value = legalizeOperand(Value, Legal_Reg | Legal_Imm, Context, true);
    Context.insert(IceInstX8632Store::create(
        Cfg, ValueHi,
        llvm::cast<IceOperandX8632Mem>(makeHighOperand(NewAddr, Context))));
    Context.insert(IceInstX8632Store::create(
        Cfg, ValueLo,
        llvm::cast<IceOperandX8632Mem>(makeLowOperand(NewAddr, Context))));
  } else {
    Value = legalizeOperand(Value, Legal_Reg | Legal_Imm, Context, true);
    Context.insert(IceInstX8632Store::create(Cfg, Value, NewAddr));
  }
}

void IceTargetX8632::doAddressOptStore(IceLoweringContext &Context) {
  IceInstStore *Inst = llvm::cast<IceInstStore>(*Context.Cur);
  IceOperand *Data = Inst->getData();
  IceOperand *Addr = Inst->getAddr();
  IceVariable *Index = NULL;
  int Shift = 0;
  int32_t Offset = 0; // TODO: make IceConstant
  IceVariable *Base = llvm::dyn_cast<IceVariable>(Addr);
  computeAddressOpt(Cfg, Base, Index, Shift, Offset);
  if (Base && Addr != Base) {
    IceConstant *OffsetOp = Cfg->getConstantInt(IceType_i32, Offset);
    Addr = IceOperandX8632Mem::create(Cfg, Data->getType(), Base, OffsetOp,
                                      Index, Shift);
    Inst->setDeleted();
    Context.insert(IceInstStore::create(Cfg, Data, Addr));
  }
}

void IceTargetX8632::lowerSwitch(const IceInstSwitch *Inst,
                                 IceLoweringContext &Context) {
  // This implements the most naive possible lowering.
  // cmp a,val[0]; jeq label[0]; cmp a,val[1]; jeq label[1]; ... jmp default
  IceOperand *Src = Inst->getSrc(0);
  unsigned NumCases = Inst->getNumCases();
  // OK, we'll be slightly less naive by forcing Src into a physical
  // register if there are 2 or more uses.
  if (NumCases >= 2)
    Src = legalizeOperandToVar(Src, Context, true);
  else
    Src = legalizeOperand(Src, Legal_All, Context, true);
  for (unsigned I = 0; I < NumCases; ++I) {
    IceOperand *Value = Cfg->getConstantInt(IceType_i32, Inst->getValue(I));
    Context.insert(IceInstX8632Icmp::create(Cfg, Src, Value));
    Context.insert(
        IceInstX8632Br::create(Cfg, Inst->getLabel(I), IceInstX8632Br::Br_e));
  }

  Context.insert(IceInstX8632Br::create(Cfg, Inst->getLabelDefault()));
}

IceOperand *IceTargetX8632::legalizeOperand(IceOperand *From, LegalMask Allowed,
                                            IceLoweringContext &Context,
                                            bool AllowOverlap, int RegNum) {
  assert(Allowed & Legal_Reg);
  assert(RegNum < 0 || Allowed == Legal_Reg);
  if (IceOperandX8632Mem *Mem = llvm::dyn_cast<IceOperandX8632Mem>(From)) {
    IceVariable *Base = Mem->getBase();
    IceVariable *Index = Mem->getIndex();
    IceVariable *RegBase = Base;
    IceVariable *RegIndex = Index;
    if (Base) {
      RegBase = legalizeOperandToVar(Base, Context, true);
    }
    if (Index) {
      RegIndex = legalizeOperandToVar(Index, Context, true);
    }
    if (Base != RegBase || Index != RegIndex) {
      From = IceOperandX8632Mem::create(Cfg, Mem->getType(), RegBase,
                                        Mem->getOffset(), RegIndex,
                                        Mem->getShift());
    }

    if (!(Allowed & Legal_Mem)) {
      IceVariable *Reg = Cfg->makeVariable(From->getType(), Context.Node);
      if (RegNum < 0) {
        Reg->setWeightInfinite();
      } else {
        Reg->setRegNum(RegNum);
      }
      Context.insert(IceInstX8632Mov::create(Cfg, Reg, From));
      From = Reg;
    }
    return From;
  }
  if (llvm::isa<IceConstant>(From)) {
    if (!(Allowed & Legal_Imm)) {
      IceVariable *Reg = Cfg->makeVariable(From->getType(), Context.Node);
      if (RegNum < 0) {
        Reg->setWeightInfinite();
      } else {
        Reg->setRegNum(RegNum);
      }
      Context.insert(IceInstX8632Mov::create(Cfg, Reg, From));
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
      IceVariable *Reg = Cfg->makeVariable(From->getType(), Context.Node);
      if (RegNum < 0) {
        Reg->setWeightInfinite();
        Reg->setPreferredRegister(Var, AllowOverlap);
      } else {
        Reg->setRegNum(RegNum);
      }
      Context.insert(IceInstX8632Mov::create(Cfg, Reg, From));
      From = Reg;
    }
    return From;
  }
  assert(0);
  return From;
}

IceVariable *IceTargetX8632::legalizeOperandToVar(IceOperand *From,
                                                  IceLoweringContext &Context,
                                                  bool AllowOverlap,
                                                  int RegNum) {
  return llvm::cast<IceVariable>(
      legalizeOperand(From, Legal_Reg, Context, AllowOverlap, RegNum));
}

////////////////////////////////////////////////////////////////

void IceTargetX8632Fast::translate(void) {
  IceTimer T_placePhiLoads;
  Cfg->placePhiLoads();
  if (Cfg->hasError())
    return;
  T_placePhiLoads.printElapsedUs(Cfg->Str, "placePhiLoads()");
  IceTimer T_placePhiStores;
  Cfg->placePhiStores();
  if (Cfg->hasError())
    return;
  T_placePhiStores.printElapsedUs(Cfg->Str, "placePhiStores()");
  IceTimer T_deletePhis;
  Cfg->deletePhis();
  if (Cfg->hasError())
    return;
  T_deletePhis.printElapsedUs(Cfg->Str, "deletePhis()");
  if (Cfg->Str.isVerbose())
    Cfg->Str << "================ After Phi lowering ================\n";
  Cfg->dump();

  IceTimer T_genCode;
  Cfg->genCode();
  if (Cfg->hasError())
    return;
  T_genCode.printElapsedUs(Cfg->Str, "genCode()");
  if (Cfg->Str.isVerbose())
    Cfg->Str
        << "================ After initial x8632 codegen ================\n";
  Cfg->dump();

  IceTimer T_genFrame;
  Cfg->genFrame();
  if (Cfg->hasError())
    return;
  T_genFrame.printElapsedUs(Cfg->Str, "genFrame()");
  if (Cfg->Str.isVerbose())
    Cfg->Str << "================ After stack frame mapping ================\n";
  Cfg->dump();
}

void IceTargetX8632Fast::postLower(const IceLoweringContext &Context) {
  llvm::SmallBitVector AvailableRegisters = getRegisterSet(RegMask_All);
  // Make one pass to black-list pre-colored registers.  TODO: If
  // there was some prior register allocation pass that made register
  // assignments, those registers need to be black-listed here as
  // well.
  for (IceInstList::iterator I = Context.Cur, E = Context.End; I != E; ++I) {
    const IceInst *Inst = *I;
    unsigned VarIndex = 0;
    for (unsigned SrcNum = 0; SrcNum < Inst->getSrcSize(); ++SrcNum) {
      IceOperand *Src = Inst->getSrc(SrcNum);
      unsigned NumVars = Src->getNumVars();
      for (unsigned J = 0; J < NumVars; ++J, ++VarIndex) {
        const IceVariable *Var = Src->getVar(J);
        int RegNum = Var->getRegNum();
        if (RegNum < 0)
          continue;
        AvailableRegisters[RegNum] = false;
      }
    }
  }
  // The second pass colors infinite-weight variables.
  for (IceInstList::iterator I = Context.Cur, E = Context.End; I != E; ++I) {
    const IceInst *Inst = *I;
    unsigned VarIndex = 0;
    for (unsigned SrcNum = 0; SrcNum < Inst->getSrcSize(); ++SrcNum) {
      IceOperand *Src = Inst->getSrc(SrcNum);
      unsigned NumVars = Src->getNumVars();
      for (unsigned J = 0; J < NumVars; ++J, ++VarIndex) {
        IceVariable *Var = Src->getVar(J);
        int RegNum = Var->getRegNum();
        if (RegNum >= 0)
          continue;
        if (!Var->getWeight().isInf())
          continue;
        llvm::SmallBitVector AvailableTypedRegisters =
            AvailableRegisters & getRegisterSetForType(Var->getType());
        assert(!AvailableTypedRegisters.empty());
        RegNum = AvailableTypedRegisters.find_first();
        Var->setRegNum(RegNum);
        AvailableRegisters[RegNum] = false;
      }
    }
  }
}
