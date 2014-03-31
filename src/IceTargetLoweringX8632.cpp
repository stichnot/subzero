//===- subzero/src/IceTargetLoweringX8632.cpp - x86-32 lowering -----------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the IceTargetLoweringX8632 class, which
// consists almost entirely of the lowering sequence for each
// high-level instruction.  It also implements
// IceTargetX8632Fast::postLower() which does the simplest possible
// register allocation for the "fast" target.
//
//===----------------------------------------------------------------------===//

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
  for (uint32_t i = Reg_eax; i <= Reg_edi; ++i)
    IntegerRegisters[i] = true;
  IntegerRegistersNonI8[Reg_eax] = true;
  IntegerRegistersNonI8[Reg_ecx] = true;
  IntegerRegistersNonI8[Reg_edx] = true;
  IntegerRegistersNonI8[Reg_ebx] = true;
  for (uint32_t i = Reg_xmm0; i <= Reg_xmm7; ++i)
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

void IceTargetX8632::translate() {
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
  // TODO: It should be sufficient to use the fastest liveness
  // calculation, i.e. IceLiveness_LREndLightweight.  However, for
  // some reason that slows down the rest of the translation.
  // Investigate.
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
  regAlloc();
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

IceVariable *IceTargetX8632::getPhysicalRegister(uint32_t RegNum) {
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

IceString IceTargetX8632::getRegName(uint32_t RegNum, IceType Type) const {
  assert(RegNum < Reg_NUM);
  static IceString RegNames8[] = { "al", "cl", "dl", "bl" };
  static IceString RegNames16[] = { "ax", "cx", "dx", "bx",
                                    "sp", "bp", "si", "di" };
  switch (Type) {
  case IceType_i1:
  case IceType_i8:
    assert(RegNum < (sizeof(RegNames8) / sizeof(*RegNames8)));
    return RegNames8[RegNum];
  case IceType_i16:
    assert(RegNum < (sizeof(RegNames16) / sizeof(*RegNames16)));
    return RegNames16[RegNum];
  default:
    return RegNames[RegNum];
  }
}

// Helper function for addProlog().  Sets the frame offset for Arg,
// updates InArgsSizeBytes according to Arg's width, and generates an
// instruction to copy Arg into its assigned register if applicable.
// For an I64 arg that has been split into Lo and Hi components, it
// calls itself recursively on the components, taking care to handle
// Lo first because of the little-endian architecture.
void IceTargetX8632::setArgOffsetAndCopy(IceLoweringContext &C,
                                         IceVariable *Arg,
                                         IceVariable *FramePtr,
                                         int32_t BasicFrameOffset,
                                         int32_t &InArgsSizeBytes) {
  IceVariable *Lo = Arg->getLo();
  IceVariable *Hi = Arg->getHi();
  IceType Type = Arg->getType();
  if (Lo && Hi && Type == IceType_i64) {
    assert(Lo->getType() != IceType_i64); // don't want infinite recursion
    assert(Hi->getType() != IceType_i64); // don't want infinite recursion
    setArgOffsetAndCopy(C, Lo, FramePtr, BasicFrameOffset, InArgsSizeBytes);
    setArgOffsetAndCopy(C, Hi, FramePtr, BasicFrameOffset, InArgsSizeBytes);
    return;
  }
  Arg->setStackOffset(BasicFrameOffset + InArgsSizeBytes);
  if (Arg->hasReg()) {
    assert(Type != IceType_i64);
    IceOperandX8632Mem *Mem = IceOperandX8632Mem::create(
        Cfg, Type, FramePtr,
        Cfg->getConstantInt(IceType_i32, Arg->getStackOffset()));
    _mov(C, Arg, Mem);
  }
  InArgsSizeBytes += typeWidthOnStack(Type);
}

void IceTargetX8632::addProlog(IceCfgNode *Node) {
  const bool SimpleCoalescing = true;
  int32_t InArgsSizeBytes = 0;
  int32_t RetIpSizeBytes = 4;
  int32_t PreservedRegsSizeBytes = 0;
  LocalsSizeBytes = 0;
  IceLoweringContext C(Node);
  C.Next = C.Cur;

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
      getRegisterSet(RegSet_CalleeSave, RegSet_None);

  int32_t GlobalsSize = 0;
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
    if (Var->hasReg()) {
      RegsUsed[Var->getRegNum()] = true;
      continue;
    }
    // An argument passed on the stack already has a stack slot.
    if (Var->getIsArg())
      continue;
    // An unreferenced variable doesn't need a stack slot.
    if (ComputedLiveRanges && Var->getLiveRange().isEmpty())
      continue;
    // A spill slot linked to a variable with a stack slot should reuse
    // that stack slot.
    if (Var->getWeight() == IceRegWeight::Zero && Var->getRegisterOverlap()) {
      if (IceVariable *Linked = Var->getPreferredRegister()) {
        if (!Linked->hasReg())
          continue;
      }
    }
    int32_t Increment = typeWidthOnStack(Var->getType());
    if (SimpleCoalescing) {
      if (Var->isMultiblockLife()) {
        GlobalsSize += Increment;
      } else {
        uint32_t NodeIndex = Var->getLocalUseNode()->getIndex();
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
  for (uint32_t i = 0; i < CalleeSaves.size(); ++i) {
    if (CalleeSaves[i] && RegsUsed[i]) {
      PreservedRegsSizeBytes += 4;
      _push(C, getPhysicalRegister(i));
    }
  }

  // Generate "push ebp; mov ebp, esp"
  if (IsEbpBasedFrame) {
    assert((RegsUsed & getRegisterSet(RegSet_FramePointer, RegSet_None))
               .count() == 0);
    PreservedRegsSizeBytes += 4;
    IceVariable *ebp = getPhysicalRegister(Reg_ebp);
    IceVariable *esp = getPhysicalRegister(Reg_esp);
    _push(C, ebp);
    _mov(C, ebp, esp);
  }

  // Generate "sub esp, LocalsSizeBytes"
  if (LocalsSizeBytes)
    _sub(C, getPhysicalRegister(Reg_esp),
         Cfg->getConstantInt(IceType_i32, LocalsSizeBytes));

  resetStackAdjustment();

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
  int32_t BasicFrameOffset = PreservedRegsSizeBytes + RetIpSizeBytes;
  if (!IsEbpBasedFrame)
    BasicFrameOffset += LocalsSizeBytes;
  for (uint32_t i = 0; i < Args.size(); ++i) {
    IceVariable *Arg = Args[i];
    setArgOffsetAndCopy(C, Arg, FramePtr, BasicFrameOffset, InArgsSizeBytes);
  }

  // Fill in stack offsets for locals.
  int32_t TotalGlobalsSize = GlobalsSize;
  GlobalsSize = 0;
  LocalsSize.assign(LocalsSize.size(), 0);
  int32_t NextStackOffset = 0;
  for (IceVarList::const_iterator I = Variables.begin(), E = Variables.end();
       I != E; ++I) {
    IceVariable *Var = *I;
    if (!Var)
      continue;
    if (Var->hasReg()) {
      RegsUsed[Var->getRegNum()] = true;
      continue;
    }
    if (Var->getIsArg())
      continue;
    if (ComputedLiveRanges && Var->getLiveRange().isEmpty())
      continue;
    if (Var->getWeight() == IceRegWeight::Zero && Var->getRegisterOverlap()) {
      if (IceVariable *Linked = Var->getPreferredRegister()) {
        if (!Linked->hasReg()) {
          // TODO: Make sure Linked has already been assigned a stack
          // slot.
          Var->setStackOffset(Linked->getStackOffset());
          continue;
        }
      }
    }
    int32_t Increment = typeWidthOnStack(Var->getType());
    if (SimpleCoalescing) {
      if (Var->isMultiblockLife()) {
        GlobalsSize += Increment;
        NextStackOffset = GlobalsSize;
      } else {
        uint32_t NodeIndex = Var->getLocalUseNode()->getIndex();
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
  IceLoweringContext C(Node);
  C.Next = InsertPoint;

  IceVariable *esp = getPhysicalRegister(Reg_esp);
  if (IsEbpBasedFrame) {
    IceVariable *ebp = getPhysicalRegister(Reg_ebp);
    _mov(C, esp, ebp);
    _pop(C, ebp);
  } else {
    // add esp, LocalsSizeBytes
    if (LocalsSizeBytes)
      _add(C, esp, Cfg->getConstantInt(IceType_i32, LocalsSizeBytes));
  }

  // Add pop instructions for preserved registers.
  llvm::SmallBitVector CalleeSaves =
      getRegisterSet(RegSet_CalleeSave, RegSet_None);
  for (uint32_t i = 0; i < CalleeSaves.size(); ++i) {
    uint32_t j = CalleeSaves.size() - i - 1;
    if (j == Reg_ebp && IsEbpBasedFrame)
      continue;
    if (CalleeSaves[j] && RegsUsed[j]) {
      _pop(C, getPhysicalRegister(j));
    }
  }
}

void IceTargetX8632::split64(IceLoweringContext &C, IceVariable *Var) {
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
  IceVariable *Lo = Var->getLo();
  IceVariable *Hi = Var->getHi();
  if (Lo) {
    assert(Hi);
    return;
  }
  assert(Hi == NULL);
  Lo = Cfg->makeVariable(IceType_i32, C.Node, Var->getName() + "__lo");
  Hi = Cfg->makeVariable(IceType_i32, C.Node, Var->getName() + "__hi");
  Var->setLoHi(Lo, Hi);
  if (Var->getIsArg()) {
    Lo->setIsArg(Cfg);
    Hi->setIsArg(Cfg);
  }
}

IceOperand *IceTargetX8632::loOperand(IceLoweringContext &C,
                                      IceOperand *Operand) {
  assert(Operand->getType() == IceType_i64);
  if (Operand->getType() != IceType_i64)
    return Operand;
  if (IceVariable *Var = llvm::dyn_cast<IceVariable>(Operand)) {
    split64(C, Var);
    return Var->getLo();
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

IceOperand *IceTargetX8632::hiOperand(IceLoweringContext &C,
                                      IceOperand *Operand) {
  assert(Operand->getType() == IceType_i64);
  if (Operand->getType() != IceType_i64)
    return Operand;
  if (IceVariable *Var = llvm::dyn_cast<IceVariable>(Operand)) {
    split64(C, Var);
    return Var->getHi();
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
  bool Scratch = Include & ~Exclude & RegSet_CallerSave;
  bool Preserved = Include & ~Exclude & RegSet_CalleeSave;
  Registers[Reg_eax] = Scratch;
  Registers[Reg_ecx] = Scratch;
  Registers[Reg_edx] = Scratch;
  Registers[Reg_ebx] = Preserved;
  Registers[Reg_esp] = Include & ~Exclude & RegSet_StackPointer;
  // ebp counts as both preserved and frame pointer
  Registers[Reg_ebp] = Include & (RegSet_CalleeSave | RegSet_FramePointer);
  if (Exclude & (RegSet_CalleeSave | RegSet_FramePointer))
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
                                 IceLoweringContext &C) {
  IsEbpBasedFrame = true;
  // TODO(sehr,stichnot): align allocated memory, keep stack aligned, minimize
  // the number of adjustments of esp, etc.
  IceVariable *esp = getPhysicalRegister(Reg_esp);
  IceOperand *TotalSize = legalizeOperand(C, Inst->getSrc(0), Legal_All);
  IceVariable *Dest = Inst->getDest();
  _sub(C, esp, TotalSize);
  _mov(C, Dest, esp);
}

void IceTargetX8632::lowerArithmetic(const IceInstArithmetic *Inst,
                                     IceLoweringContext &C) {
  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = legalizeOperand(C, Inst->getSrc(0), Legal_All);
  IceOperand *Src1 = legalizeOperand(C, Inst->getSrc(1), Legal_All);
  if (Dest->getType() == IceType_i64) {
    IceVariable *DestLo = llvm::cast<IceVariable>(loOperand(C, Dest));
    IceVariable *DestHi = llvm::cast<IceVariable>(hiOperand(C, Dest));
    IceOperand *Src0Lo = loOperand(C, Src0);
    IceOperand *Src0Hi = hiOperand(C, Src0);
    IceOperand *Src1Lo = loOperand(C, Src1);
    IceOperand *Src1Hi = hiOperand(C, Src1);
    IceVariable *TmpLo = NULL, *TmpHi = NULL;
    switch (Inst->getOp()) {
    case IceInstArithmetic::Add:
      _mov(C, TmpLo, Src0Lo);
      _add(C, TmpLo, Src1Lo);
      _mov(C, DestLo, TmpLo);
      _mov(C, TmpHi, Src0Hi);
      _adc(C, TmpHi, Src1Hi);
      _mov(C, DestHi, TmpHi);
      break;
    case IceInstArithmetic::And:
      _mov(C, TmpLo, Src0Lo);
      _and(C, TmpLo, Src1Lo);
      _mov(C, DestLo, TmpLo);
      _mov(C, TmpHi, Src0Hi);
      _and(C, TmpHi, Src1Hi);
      _mov(C, DestHi, TmpHi);
      break;
    case IceInstArithmetic::Or:
      _mov(C, TmpLo, Src0Lo);
      _or(C, TmpLo, Src1Lo);
      _mov(C, DestLo, TmpLo);
      _mov(C, TmpHi, Src0Hi);
      _or(C, TmpHi, Src1Hi);
      _mov(C, DestHi, TmpHi);
      break;
    case IceInstArithmetic::Xor:
      _mov(C, TmpLo, Src0Lo);
      _xor(C, TmpLo, Src1Lo);
      _mov(C, DestLo, TmpLo);
      _mov(C, TmpHi, Src0Hi);
      _xor(C, TmpHi, Src1Hi);
      _mov(C, DestHi, TmpHi);
      break;
    case IceInstArithmetic::Sub:
      _mov(C, TmpLo, Src0Lo);
      _sub(C, TmpLo, Src1Lo);
      _mov(C, DestLo, TmpLo);
      _mov(C, TmpHi, Src0Hi);
      _sbb(C, TmpHi, Src1Hi);
      _mov(C, DestHi, TmpHi);
      break;
    case IceInstArithmetic::Mul: {
      IceVariable *Tmp1 = NULL, *Tmp2 = NULL, *Tmp3 = NULL;
      IceVariable *Tmp4Lo = makeVariableWithReg(C, IceType_i32, Reg_eax);
      IceVariable *Tmp4Hi = makeVariableWithReg(C, IceType_i32, Reg_edx);
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
      _mov(C, Tmp1, Src0Hi);
      _imul(C, Tmp1, Src1Lo);
      _mov(C, Tmp2, Src1Hi);
      _imul(C, Tmp2, Src0Lo);
      Tmp3 = legalizeOperandToVar(C, Src0Lo, false, Reg_eax);
      _mul(C, Tmp4Lo, Tmp3, Src1Lo);
      _mov(C, DestLo, Tmp4Lo);
      C.insert(IceInstFakeDef::create(Cfg, Tmp4Hi, Tmp4Lo));
      _add(C, Tmp4Hi, Tmp1);
      _add(C, Tmp4Hi, Tmp2);
      _mov(C, DestHi, Tmp4Hi);
    } break;
    case IceInstArithmetic::Shl: {
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
      IceVariable *Tmp1 = NULL, *Tmp2 = NULL, *Tmp3 = NULL;
      IceConstant *BitTest = Cfg->getConstantInt(IceType_i32, 0x20);
      IceConstant *Zero = Cfg->getConstantInt(IceType_i32, 0);
      IceInstX8632Label *Label = IceInstX8632Label::create(Cfg, this);
      Tmp1 = legalizeOperandToVar(C, Src1Lo, false, Reg_ecx);
      _mov(C, Tmp2, Src0Lo);
      _mov(C, Tmp3, Src0Hi);
      _shld(C, Tmp3, Tmp2, Tmp1);
      _shl(C, Tmp2, Tmp1);
      _test(C, Tmp1, BitTest);
      _br(C, Label, IceInstX8632Br::Br_e);
      C.insert(IceInstFakeUse::create(Cfg, Tmp3));
      _mov(C, Tmp3, Tmp2);
      _mov(C, Tmp2, Zero);
      C.insert(Label);
      _mov(C, DestLo, Tmp2);
      _mov(C, DestHi, Tmp3);
    } break;
    case IceInstArithmetic::Lshr: {
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
      IceVariable *Tmp1 = NULL, *Tmp2 = NULL, *Tmp3 = NULL;
      IceConstant *BitTest = Cfg->getConstantInt(IceType_i32, 0x20);
      IceConstant *Zero = Cfg->getConstantInt(IceType_i32, 0);
      IceInstX8632Label *Label = IceInstX8632Label::create(Cfg, this);
      Tmp1 = legalizeOperandToVar(C, Src1Lo, false, Reg_ecx);
      _mov(C, Tmp2, Src0Lo);
      _mov(C, Tmp3, Src0Hi);
      _shrd(C, Tmp2, Tmp3, Tmp1);
      _shr(C, Tmp3, Tmp1);
      _test(C, Tmp1, BitTest);
      _br(C, Label, IceInstX8632Br::Br_e);
      C.insert(IceInstFakeUse::create(Cfg, Tmp2));
      _mov(C, Tmp2, Tmp3);
      _mov(C, Tmp3, Zero);
      C.insert(Label);
      _mov(C, DestLo, Tmp2);
      _mov(C, DestHi, Tmp3);
    } break;
    case IceInstArithmetic::Ashr: {
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
      IceVariable *Tmp1 = NULL, *Tmp2 = NULL, *Tmp3 = NULL;
      IceConstant *BitTest = Cfg->getConstantInt(IceType_i32, 0x20);
      IceConstant *SignExtend = Cfg->getConstantInt(IceType_i32, 0x1f);
      IceInstX8632Label *Label = IceInstX8632Label::create(Cfg, this);
      Tmp1 = legalizeOperandToVar(C, Src1Lo, false, Reg_ecx);
      _mov(C, Tmp2, Src0Lo);
      _mov(C, Tmp3, Src0Hi);
      _shrd(C, Tmp2, Tmp3, Tmp1);
      _sar(C, Tmp3, Tmp1);
      _test(C, Tmp1, BitTest);
      _br(C, Label, IceInstX8632Br::Br_e);
      C.insert(IceInstFakeUse::create(Cfg, Tmp2));
      _mov(C, Tmp2, Tmp3);
      _sar(C, Tmp3, SignExtend);
      C.insert(Label);
      _mov(C, DestLo, Tmp2);
      _mov(C, DestHi, Tmp3);
    } break;
    case IceInstArithmetic::Udiv: {
      uint32_t MaxSrcs = 2;
      IceInstCall *Call =
          makeHelperCall("__udivdi3", IceType_i32, Dest, MaxSrcs);
      Call->addArg(Inst->getSrc(0));
      Call->addArg(Inst->getSrc(1));
      lowerCall(Call, C);
    } break;
    case IceInstArithmetic::Sdiv: {
      uint32_t MaxSrcs = 2;
      IceInstCall *Call =
          makeHelperCall("__divdi3", IceType_i32, Dest, MaxSrcs);
      Call->addArg(Inst->getSrc(0));
      Call->addArg(Inst->getSrc(1));
      lowerCall(Call, C);
    } break;
    case IceInstArithmetic::Urem: {
      uint32_t MaxSrcs = 2;
      IceInstCall *Call =
          makeHelperCall("__umoddi3", IceType_i32, Dest, MaxSrcs);
      Call->addArg(Inst->getSrc(0));
      Call->addArg(Inst->getSrc(1));
      lowerCall(Call, C);
    } break;
    case IceInstArithmetic::Srem: {
      uint32_t MaxSrcs = 2;
      IceInstCall *Call =
          makeHelperCall("__moddi3", IceType_i32, Dest, MaxSrcs);
      Call->addArg(Inst->getSrc(0));
      Call->addArg(Inst->getSrc(1));
      lowerCall(Call, C);
    } break;
    case IceInstArithmetic::Fadd:
    case IceInstArithmetic::Fsub:
    case IceInstArithmetic::Fmul:
    case IceInstArithmetic::Fdiv:
    case IceInstArithmetic::Frem:
    case IceInstArithmetic::OpKind_NUM:
      assert(0);
      break;
    }
  } else { // Dest->getType() != IceType_i64
    IceVariable *Reg0 = NULL;
    IceVariable *Reg1 = NULL;
    switch (Inst->getOp()) {
    case IceInstArithmetic::Add:
      _mov(C, Reg1, Src0);
      _add(C, Reg1, Src1);
      _mov(C, Dest, Reg1);
      break;
    case IceInstArithmetic::And:
      _mov(C, Reg1, Src0);
      _and(C, Reg1, Src1);
      _mov(C, Dest, Reg1);
      break;
    case IceInstArithmetic::Or:
      _mov(C, Reg1, Src0);
      _or(C, Reg1, Src1);
      _mov(C, Dest, Reg1);
      break;
    case IceInstArithmetic::Xor:
      _mov(C, Reg1, Src0);
      _xor(C, Reg1, Src1);
      _mov(C, Dest, Reg1);
      break;
    case IceInstArithmetic::Sub:
      _mov(C, Reg1, Src0);
      _sub(C, Reg1, Src1);
      _mov(C, Dest, Reg1);
      break;
    case IceInstArithmetic::Mul:
      // TODO: Optimize for llvm::isa<IceConstant>(Src1)
      // TODO: Strength-reduce multiplications by a constant,
      // particularly -1 and powers of 2.  Advanced: use lea to
      // multiply by 3, 5, 9.
      _mov(C, Reg1, Src0);
      _imul(C, Reg1, Src1);
      _mov(C, Dest, Reg1);
      break;
    case IceInstArithmetic::Shl:
      _mov(C, Reg1, Src0);
      if (!llvm::isa<IceConstant>(Src1))
        Src1 = legalizeOperandToVar(C, Src1, false, Reg_ecx);
      _shl(C, Reg1, Src1);
      _mov(C, Dest, Reg1);
      break;
    case IceInstArithmetic::Lshr:
      _mov(C, Reg1, Src0);
      if (!llvm::isa<IceConstant>(Src1))
        Src1 = legalizeOperandToVar(C, Src1, false, Reg_ecx);
      _shr(C, Reg1, Src1);
      _mov(C, Dest, Reg1);
      break;
    case IceInstArithmetic::Ashr:
      _mov(C, Reg1, Src0);
      if (!llvm::isa<IceConstant>(Src1))
        Src1 = legalizeOperandToVar(C, Src1, false, Reg_ecx);
      _sar(C, Reg1, Src1);
      _mov(C, Dest, Reg1);
      break;
    case IceInstArithmetic::Udiv: {
      Reg1 = legalizeOperandToVar(C, Src0, false, Reg_eax);
      Reg0 = makeVariableWithReg(C, IceType_i32, Reg_edx);
      IceConstant *ConstZero = Cfg->getConstantInt(IceType_i32, 0);
      _mov(C, Reg0, ConstZero);
      _div(C, Reg1, Src1, Reg0);
      _mov(C, Dest, Reg1);
    } break;
    case IceInstArithmetic::Sdiv:
      Reg1 = legalizeOperandToVar(C, Src0, false, Reg_eax);
      Reg0 = makeVariableWithReg(C, IceType_i32, Reg_edx);
      _cdq(C, Reg0, Reg1);
      _idiv(C, Reg1, Src1, Reg0);
      _mov(C, Dest, Reg1);
      break;
    case IceInstArithmetic::Urem: {
      Reg1 = legalizeOperandToVar(C, Src0, false, Reg_eax);
      Reg0 = makeVariableWithReg(C, IceType_i32, Reg_edx);
      IceConstant *ConstZero = Cfg->getConstantInt(IceType_i32, 0);
      _mov(C, Reg0, ConstZero);
      _div(C, Reg0, Src1, Reg1);
      Reg1 = Reg0;
      _mov(C, Dest, Reg1);
    } break;
    case IceInstArithmetic::Srem:
      Reg1 = legalizeOperandToVar(C, Src0, false, Reg_eax);
      Reg0 = makeVariableWithReg(C, IceType_i32, Reg_edx);
      _cdq(C, Reg0, Reg1);
      _idiv(C, Reg0, Src1, Reg1);
      Reg1 = Reg0;
      _mov(C, Dest, Reg1);
      break;
    case IceInstArithmetic::Fadd:
      // t=src0; t=addss/addsd t, src1; dst=movss/movsd t
      _mov(C, Reg1, Src0);
      _addss(C, Reg1, Src1);
      _mov(C, Dest, Reg1);
      break;
    case IceInstArithmetic::Fsub:
      _mov(C, Reg1, Src0);
      _subss(C, Reg1, Src1);
      _mov(C, Dest, Reg1);
      break;
    case IceInstArithmetic::Fmul:
      _mov(C, Reg1, Src0);
      _mulss(C, Reg1, Src1);
      _mov(C, Dest, Reg1);
      break;
    case IceInstArithmetic::Fdiv:
      _mov(C, Reg1, Src0);
      _divss(C, Reg1, Src1);
      _mov(C, Dest, Reg1);
      break;
    case IceInstArithmetic::Frem: {
      uint32_t MaxSrcs = 2;
      IceType Type = Dest->getType();
      IceInstCall *Call = makeHelperCall(Type == IceType_f32 ? "fmodf" : "fmod",
                                         Type, Dest, MaxSrcs);
      Call->addArg(Src0);
      Call->addArg(Src1);
      return lowerCall(Call, C);
    } break;
    case IceInstArithmetic::OpKind_NUM:
      assert(0);
      break;
    }
  }
}

void IceTargetX8632::lowerAssign(const IceInstAssign *Inst,
                                 IceLoweringContext &C) {
  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = Inst->getSrc(0);
  if (Dest->getType() == IceType_i64) {
    // TODO: This seems broken if Src and Dest components are both on
    // the stack and not register-allocated.
    IceVariable *DestLo = llvm::cast<IceVariable>(loOperand(C, Dest));
    IceVariable *DestHi = llvm::cast<IceVariable>(hiOperand(C, Dest));
    _mov(C, DestLo, loOperand(C, Src0));
    _mov(C, DestHi, hiOperand(C, Src0));
    return;
  }
  // a=b ==> t=b; a=t; (link t->b)
  assert(Dest->getType() == Src0->getType());
  IceOperand *Reg = legalizeOperand(C, Src0, Legal_Reg | Legal_Imm, true);
  _mov(C, Dest, Reg);
}

void IceTargetX8632::lowerBr(const IceInstBr *Inst, IceLoweringContext &C) {
  if (Inst->isUnconditional()) {
    _br(C, Inst->getTargetUnconditional());
    return;
  }
  // cmp src, 0; br ne, labelTrue; br labelFalse
  IceOperand *Src = legalizeOperand(C, Inst->getSrc(0), Legal_All);
  IceConstant *OpZero = Cfg->getConstantInt(IceType_i32, 0);
  _icmp(C, Src, OpZero);
  _br(C, Inst->getTargetTrue(), Inst->getTargetFalse(), IceInstX8632Br::Br_ne);
}

void IceTargetX8632::lowerCall(const IceInstCall *Inst, IceLoweringContext &C) {
  // TODO: what to do about tailcalls?
  // Generate a sequence of push instructions, pushing right to left,
  // keeping track of stack offsets in case a push involves a stack
  // operand and we are using an esp-based frame.
  uint32_t StackOffset = 0;
  // TODO: If for some reason the call instruction gets dead-code
  // eliminated after lowering, we would need to ensure that the
  // pre-call push instructions and the post-call esp adjustment get
  // eliminated as well.
  for (uint32_t NumArgs = Inst->getNumArgs(), i = 0; i < NumArgs; ++i) {
    IceOperand *Arg = Inst->getArg(NumArgs - i - 1);
    Arg = legalizeOperand(C, Arg, Legal_All);
    assert(Arg);
    if (Arg->getType() == IceType_i64) {
      _push(C, hiOperand(C, Arg));
      _push(C, loOperand(C, Arg));
    } else if (Arg->getType() == IceType_f64) {
      // If the Arg turns out to be a memory operand, we need to push
      // 8 bytes, which requires two push instructions.  This ends up
      // being somewhat clumsy in the current IR, so we use a
      // workaround.  Force the operand into a (xmm) register, and
      // then push the register.  An xmm register push is actually not
      // possible in x86, but the Push instruction emitter handles
      // this by decrementing the stack pointer and directly writing
      // the xmm register value.
      IceVariable *Var = NULL;
      _mov(C, Var, Arg);
      _push(C, Var);
    } else {
      _push(C, Arg);
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
      Reg = makeVariableWithReg(C, Dest->getType(), Reg_eax);
      break;
    case IceType_i64:
      Reg = makeVariableWithReg(C, IceType_i32, Reg_eax);
      RegHi = makeVariableWithReg(C, IceType_i32, Reg_edx);
      break;
    case IceType_f32:
    case IceType_f64:
      Reg = NULL;
      // Leave Reg==NULL, and capture the result with the fstp
      // instruction.
      break;
    }
  }
  IceOperand *CallTarget = legalizeOperand(C, Inst->getCallTarget(), Legal_All);
  IceInst *NewCall =
      IceInstX8632Call::create(Cfg, Reg, CallTarget, Inst->isTail());
  C.insert(NewCall);
  if (RegHi)
    C.insert(IceInstFakeDef::create(Cfg, RegHi));

  // Add the appropriate offset to esp.
  if (StackOffset) {
    IceVariable *esp = Cfg->getTarget()->getPhysicalRegister(Reg_esp);
    _add(C, esp, Cfg->getConstantInt(IceType_i32, StackOffset));
  }

  // Insert a register-kill pseudo instruction.
  IceVarList KilledRegs;
  for (uint32_t i = 0; i < ScratchRegs.size(); ++i) {
    if (ScratchRegs[i])
      KilledRegs.push_back(Cfg->getTarget()->getPhysicalRegister(i));
  }
  if (!KilledRegs.empty()) {
    IceInst *Kill = IceInstFakeKill::create(Cfg, KilledRegs, NewCall);
    C.insert(Kill);
  }

  // Generate a FakeUse to keep the call live if necessary.
  if (Inst->hasSideEffects() && Reg) {
    IceInst *FakeUse = IceInstFakeUse::create(Cfg, Reg);
    C.insert(FakeUse);
  }

  // Generate Dest=Reg assignment.
  if (Dest && Reg) {
    if (RegHi) {
      IceVariable *DestLo = Dest->getLo();
      IceVariable *DestHi = Dest->getHi();
      DestLo->setPreferredRegister(Reg, false);
      _mov(C, DestLo, Reg);
      DestHi->setPreferredRegister(RegHi, false);
      _mov(C, DestHi, RegHi);
    } else {
      Dest->setPreferredRegister(Reg, false);
      _mov(C, Dest, Reg);
    }
  }

  // Special treatment for an FP function which returns its result in
  // st(0).
  if (Dest &&
      (Dest->getType() == IceType_f32 || Dest->getType() == IceType_f64)) {
    _fstp(C, Dest);
    // If Dest ends up being a physical xmm register, the fstp emit
    // code will route st(0) through a temporary stack slot.
  }
}

void IceTargetX8632::lowerCast(const IceInstCast *Inst, IceLoweringContext &C) {
  // a = cast(b) ==> t=cast(b); a=t; (link t->b, link a->t, no overlap)
  IceInstCast::OpKind CastKind = Inst->getCastKind();
  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = Inst->getSrc(0);
  IceOperand *Reg = legalizeOperand(C, Src0, Legal_Reg | Legal_Mem, true);
  // TODO: Consider allowing Immediates for Bitcast.
  switch (CastKind) {
  default:
    Cfg->setError("Cast type not yet supported");
    return;
    break;
  case IceInstCast::Sext:
    if (Dest->getType() == IceType_i64) {
      // t1=movsx src; t2=t1; t2=sar t2, 31; dst.lo=t1; dst.hi=t2
      IceVariable *DestLo = llvm::cast<IceVariable>(loOperand(C, Dest));
      IceVariable *DestHi = llvm::cast<IceVariable>(hiOperand(C, Dest));
      IceVariable *Tmp = makeVariableWithReg(C, DestLo->getType());
      if (Reg->getType() == IceType_i32)
        _mov(C, Tmp, Reg);
      else
        _movsx(C, Tmp, Reg);
      _mov(C, DestLo, Tmp);
      IceVariable *RegHi = makeVariableWithReg(C, IceType_i32);
      IceConstant *Shift = Cfg->getConstantInt(IceType_i32, 31);
      _mov(C, RegHi, Tmp);
      _sar(C, RegHi, Shift);
      _mov(C, DestHi, RegHi);
    } else {
      // TODO: Sign-extend an i1 via "shl reg, 31; sar reg, 31", and
      // also copy to the high operand of a 64-bit variable.
      // t1 = movsx src; dst = t1
      IceVariable *Tmp = makeVariableWithReg(C, Dest->getType());
      _movsx(C, Tmp, Reg);
      _mov(C, Dest, Tmp);
    }
    break;
  case IceInstCast::Zext:
    if (Dest->getType() == IceType_i64) {
      // t1=movzx src; dst.lo=t1; dst.hi=0
      IceConstant *Zero = Cfg->getConstantInt(IceType_i32, 0);
      IceVariable *DestLo = llvm::cast<IceVariable>(loOperand(C, Dest));
      IceVariable *DestHi = llvm::cast<IceVariable>(hiOperand(C, Dest));
      IceVariable *Tmp = makeVariableWithReg(C, DestLo->getType());
      if (Reg->getType() == IceType_i32)
        _mov(C, Tmp, Reg);
      else
        _movzx(C, Tmp, Reg);
      _mov(C, DestLo, Tmp);
      _mov(C, DestHi, Zero);
    } else if (Reg->getType() == IceType_i1) {
      // t = Reg; t &= 1; Dest = t
      IceOperand *ConstOne = Cfg->getConstantInt(IceType_i32, 1);
      IceVariable *Tmp = makeVariableWithReg(C, IceType_i32);
      _movzx(C, Tmp, Reg);
      _and(C, Tmp, ConstOne);
      _mov(C, Dest, Tmp);
    } else {
      // t1 = movzx src; dst = t1
      IceVariable *Tmp = makeVariableWithReg(C, Dest->getType());
      _movzx(C, Tmp, Reg);
      _mov(C, Dest, Tmp);
    }
    break;
  case IceInstCast::Trunc: {
    if (Reg->getType() == IceType_i64)
      Reg = loOperand(C, Reg);
    // t1 = trunc Reg; Dest = t1
    IceOperand *Tmp1 = legalizeOperand(C, Reg, Legal_All);
    IceVariable *Tmp2 = makeVariableWithReg(C, Dest->getType());
    _mov(C, Tmp2, Tmp1);
    _mov(C, Dest, Tmp2);
    break;
  }
  case IceInstCast::Fptrunc:
  case IceInstCast::Fpext: {
    // t1 = cvt Reg; Dest = t1
    IceVariable *Tmp = makeVariableWithReg(C, Dest->getType());
    _cvt(C, Tmp, Reg);
    _mov(C, Dest, Tmp);
    break;
  }
  case IceInstCast::Fptosi:
    if (Dest->getType() == IceType_i64) {
      // Use a helper for converting floating-point values to 64-bit
      // integers.  SSE2 appears to have no way to convert from xmm
      // registers to something like the edx:eax register pair, and
      // gcc and clang both want to use x87 instructions complete with
      // temporary manipulation of the status word.  This helper is
      // not needed for x86-64.
      split64(C, Dest);
      uint32_t MaxSrcs = 1;
      IceType SrcType = Inst->getSrc(0)->getType();
      IceInstCall *Call =
          makeHelperCall(SrcType == IceType_f32 ? "cvtftosi64" : "cvtdtosi64",
                         IceType_f32, Dest, MaxSrcs);
      Call->addArg(Inst->getSrc(0));
      lowerCall(Call, C);
      return;
    } else {
      // t1.i32 = cvt Reg; t2.dest_type = t1; Dest = t2.dest_type
      IceVariable *Tmp1 = makeVariableWithReg(C, IceType_i32);
      _cvt(C, Tmp1, Reg);
      IceVariable *Tmp2 = makeVariableWithReg(C, Dest->getType());
      Tmp2->setPreferredRegister(Tmp1, true);
      _mov(C, Tmp2, Tmp1);
      _mov(C, Dest, Tmp2);
      // Sign-extend the result if necessary.
    }
    break;
  case IceInstCast::Fptoui:
    if (Dest->getType() == IceType_i64 || Dest->getType() == IceType_i32) {
      // Use a helper for both x86-32 and x86-64.
      split64(C, Dest);
      uint32_t MaxSrcs = 1;
      IceType DestType = Dest->getType();
      IceType SrcType = Src0->getType();
      IceString DstSubstring = (DestType == IceType_i64 ? "64" : "32");
      IceString SrcSubstring = (SrcType == IceType_f32 ? "f" : "d");
      // Possibilities are cvtftoui32, cvtdtoui32, cvtftoui64, cvtdtoui64
      IceString TargetString = "cvt" + SrcSubstring + "toui" + DstSubstring;
      IceInstCall *Call =
          makeHelperCall(TargetString, IceType_i64, Dest, MaxSrcs);
      Call->addArg(Inst->getSrc(0));
      lowerCall(Call, C);
      return;
    } else {
      // t1.i32 = cvt Reg; t2.dest_type = t1; Dest = t2.dest_type
      IceVariable *Tmp1 = makeVariableWithReg(C, IceType_i32);
      _cvt(C, Tmp1, Reg);
      IceVariable *Tmp2 = makeVariableWithReg(C, Dest->getType());
      Tmp2->setPreferredRegister(Tmp1, true);
      _mov(C, Tmp2, Tmp1);
      _mov(C, Dest, Tmp2);
      // Zero-extend the result if necessary.
    }
    break;
  case IceInstCast::Sitofp:
    if (Reg->getType() == IceType_i64) {
      // Use a helper for x86-32.
      uint32_t MaxSrcs = 1;
      IceType DestType = Dest->getType();
      IceInstCall *Call =
          makeHelperCall(DestType == IceType_f32 ? "cvtsi64tof" : "cvtsi64tod",
                         DestType, Dest, MaxSrcs);
      Call->addArg(Inst->getSrc(0));
      lowerCall(Call, C);
      return;
    } else {
      // Sign-extend the operand.
      // t1.i32 = movsx Reg; t2 = Cvt t1.i32; Dest = t2
      IceVariable *Tmp1 = makeVariableWithReg(C, IceType_i32);
      if (Reg->getType() == IceType_i32)
        _mov(C, Tmp1, Reg);
      else
        _movsx(C, Tmp1, Reg);
      IceVariable *Tmp2 = makeVariableWithReg(C, Dest->getType());
      _cvt(C, Tmp2, Tmp1);
      _mov(C, Dest, Tmp2);
    }
    break;
  case IceInstCast::Uitofp:
    if (Reg->getType() == IceType_i64 || Reg->getType() == IceType_i32) {
      // Use a helper for x86-32 and x86-64.  Also use a helper for
      // i32 on x86-32.
      uint32_t MaxSrcs = 1;
      IceType DestType = Dest->getType();
      IceString SrcSubstring = (Reg->getType() == IceType_i64 ? "64" : "32");
      IceString DstSubstring = (DestType == IceType_f32 ? "f" : "d");
      // Possibilities are cvtui32tof, cvtui32tod, cvtui64tof, cvtui64tod
      IceString TargetString = "cvtui" + SrcSubstring + "to" + DstSubstring;
      IceInstCall *Call = makeHelperCall(TargetString, DestType, Dest, MaxSrcs);
      Call->addArg(Inst->getSrc(0));
      lowerCall(Call, C);
      return;
    } else {
      // Zero-extend the operand.
      // t1.i32 = movzx Reg; t2 = Cvt t1.i32; Dest = t2
      IceVariable *Tmp1 = makeVariableWithReg(C, IceType_i32);
      if (Reg->getType() == IceType_i32)
        _mov(C, Tmp1, Reg);
      else
        _movzx(C, Tmp1, Reg);
      IceVariable *Tmp2 = makeVariableWithReg(C, Dest->getType());
      _cvt(C, Tmp2, Tmp1);
      _mov(C, Dest, Tmp2);
    }
    break;
  case IceInstCast::Bitcast:
    switch (Dest->getType()) {
    default:
      assert(0 && "Unexpected Bitcast dest type");
    case IceType_i32:
    case IceType_f32: {
      IceType DestType = Dest->getType();
      IceType SrcType = Reg->getType();
      assert((DestType == IceType_i32 && SrcType == IceType_f32) ||
             (DestType == IceType_f32 && SrcType == IceType_i32));
      // a.i32 = bitcast b.f32 ==>
      //   t.f32 = b.f32
      //   s.f32 = spill t.f32
      //   a.i32 = s.f32
      IceVariable *Tmp = makeVariableWithReg(C, SrcType);
      _mov(C, Tmp, Reg);
      IceVariable *Spill = Cfg->makeVariable(SrcType, C.Node);
      Spill->setWeight(IceRegWeight::Zero);
      Spill->setPreferredRegister(Dest, true);
      _mov(C, Spill, Tmp);
      _mov(C, Dest, Spill);
    } break;
    case IceType_i64: {
      assert(Reg->getType() == IceType_f64);
      // a.i64 = bitcast b.f64 ==>
      //   s.f64 = spill b.f64
      //   t_lo.i32 = lo(s.f64)
      //   a_lo.i32 = t_lo.i32
      //   t_hi.i32 = hi(s.f64)
      //   a_hi.i32 = t_hi.i32
      IceVariable *Spill = Cfg->makeVariable(IceType_f64, C.Node);
      Spill->setWeight(IceRegWeight::Zero);
      Spill->setPreferredRegister(llvm::dyn_cast<IceVariable>(Reg), true);
      _mov(C, Spill, Reg);

      IceVariable *TmpLo = makeVariableWithReg(C, IceType_i32);
      IceVariableSplit *SpillLo =
          IceVariableSplit::create(Cfg, Spill, IceVariableSplit::Low);
      _mov(C, TmpLo, SpillLo);
      IceVariable *DestLo = llvm::cast<IceVariable>(loOperand(C, Dest));
      _mov(C, DestLo, TmpLo);

      IceVariable *TmpHi = makeVariableWithReg(C, IceType_i32);
      IceVariableSplit *SpillHi =
          IceVariableSplit::create(Cfg, Spill, IceVariableSplit::High);
      _mov(C, TmpHi, SpillHi);
      IceVariable *DestHi = llvm::cast<IceVariable>(hiOperand(C, Dest));
      _mov(C, DestHi, TmpHi);
    } break;
    case IceType_f64: {
      assert(Reg->getType() == IceType_i64);
      // a.f64 = bitcast b.i64 ==>
      //   t_lo.i32 = b_lo.i32
      //   lo(s.f64) = t_lo.i32
      //   FakeUse(s.f64)
      //   t_hi.i32 = b_hi.i32
      //   hi(s.f64) = t_hi.i32
      //   a.f64 = s.f64
      IceVariable *Spill = Cfg->makeVariable(IceType_f64, C.Node);
      Spill->setWeight(IceRegWeight::Zero);
      Spill->setPreferredRegister(Dest, true);

      C.insert(IceInstFakeDef::create(Cfg, Spill));

      IceVariable *TmpLo = makeVariableWithReg(C, IceType_i32);
      _mov(C, TmpLo, loOperand(C, Reg));
      IceVariableSplit *SpillLo =
          IceVariableSplit::create(Cfg, Spill, IceVariableSplit::Low);
      _store(C, TmpLo, SpillLo);

      IceVariable *TmpHi = makeVariableWithReg(C, IceType_i32);
      _mov(C, TmpHi, hiOperand(C, Reg));
      IceVariableSplit *SpillHi =
          IceVariableSplit::create(Cfg, Spill, IceVariableSplit::High);
      _store(C, TmpHi, SpillHi);

      _mov(C, Dest, Spill);
    } break;
    }
    break;
  }
}

static struct {
  IceInstFcmp::FCond Cond;
  uint32_t Default;
  bool SwapOperands;
  IceInstX8632Br::BrCond C1, C2;
} TableFcmp[] = { { IceInstFcmp::False,      0,                      false,
                    IceInstX8632Br::Br_None, IceInstX8632Br::Br_None },
                  { IceInstFcmp::Oeq,      0,                   false,
                    IceInstX8632Br::Br_ne, IceInstX8632Br::Br_p },
                  { IceInstFcmp::Ogt,     1,                      false,
                    IceInstX8632Br::Br_a, IceInstX8632Br::Br_None },
                  { IceInstFcmp::Oge,      1,                      false,
                    IceInstX8632Br::Br_ae, IceInstX8632Br::Br_None },
                  { IceInstFcmp::Olt,     1,                      true,
                    IceInstX8632Br::Br_a, IceInstX8632Br::Br_None },
                  { IceInstFcmp::Ole,      1,                      true,
                    IceInstX8632Br::Br_ae, IceInstX8632Br::Br_None },
                  { IceInstFcmp::One,      1,                      false,
                    IceInstX8632Br::Br_ne, IceInstX8632Br::Br_None },
                  { IceInstFcmp::Ord,      1,                      false,
                    IceInstX8632Br::Br_np, IceInstX8632Br::Br_None },
                  { IceInstFcmp::Ueq,     1,                      false,
                    IceInstX8632Br::Br_e, IceInstX8632Br::Br_None },
                  { IceInstFcmp::Ugt,     1,                      true,
                    IceInstX8632Br::Br_b, IceInstX8632Br::Br_None },
                  { IceInstFcmp::Uge,      1,                      true,
                    IceInstX8632Br::Br_be, IceInstX8632Br::Br_None },
                  { IceInstFcmp::Ult,     1,                      false,
                    IceInstX8632Br::Br_b, IceInstX8632Br::Br_None },
                  { IceInstFcmp::Ule,      1,                      false,
                    IceInstX8632Br::Br_be, IceInstX8632Br::Br_None },
                  { IceInstFcmp::Une,      1,                   false,
                    IceInstX8632Br::Br_ne, IceInstX8632Br::Br_p },
                  { IceInstFcmp::Uno,     1,                      false,
                    IceInstX8632Br::Br_p, IceInstX8632Br::Br_None },
                  { IceInstFcmp::True,       1,                      false,
                    IceInstX8632Br::Br_None, IceInstX8632Br::Br_None }, };
const static uint32_t TableFcmpSize = sizeof(TableFcmp) / sizeof(*TableFcmp);

void IceTargetX8632::lowerFcmp(const IceInstFcmp *Inst, IceLoweringContext &C) {
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
  uint32_t Index = static_cast<uint32_t>(Condition);
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
    Src1 = legalizeOperand(C, Src1, Legal_All);
    IceVariable *Tmp = NULL;
    _mov(C, Tmp, Src0);
    _ucomiss(C, Tmp, Src1);
  }
  IceConstant *Default =
      Cfg->getConstantInt(IceType_i32, TableFcmp[Index].Default);
  _mov(C, Dest, Default);
  if (HasC1) {
    IceInstX8632Label *Label = IceInstX8632Label::create(Cfg, this);
    _br(C, Label, TableFcmp[Index].C1);
    if (HasC2) {
      _br(C, Label, TableFcmp[Index].C2);
    }
    C.insert(IceInstFakeUse::create(Cfg, Dest));
    IceConstant *NonDefault =
        Cfg->getConstantInt(IceType_i32, !TableFcmp[Index].Default);
    _mov(C, Dest, NonDefault);
    C.insert(Label);
  }
}

static struct {
  IceInstIcmp::ICond Cond;
  IceInstX8632Br::BrCond C1, C2, C3;
} TableIcmp64[] = { { IceInstIcmp::Eq }, { IceInstIcmp::Ne },
                    { IceInstIcmp::Ugt,     IceInstX8632Br::Br_a,
                      IceInstX8632Br::Br_b, IceInstX8632Br::Br_a },
                    { IceInstIcmp::Uge,     IceInstX8632Br::Br_a,
                      IceInstX8632Br::Br_b, IceInstX8632Br::Br_ae },
                    { IceInstIcmp::Ult,     IceInstX8632Br::Br_b,
                      IceInstX8632Br::Br_a, IceInstX8632Br::Br_b },
                    { IceInstIcmp::Ule,     IceInstX8632Br::Br_b,
                      IceInstX8632Br::Br_a, IceInstX8632Br::Br_be },
                    { IceInstIcmp::Sgt,     IceInstX8632Br::Br_g,
                      IceInstX8632Br::Br_l, IceInstX8632Br::Br_a },
                    { IceInstIcmp::Sge,     IceInstX8632Br::Br_g,
                      IceInstX8632Br::Br_l, IceInstX8632Br::Br_ae },
                    { IceInstIcmp::Slt,     IceInstX8632Br::Br_l,
                      IceInstX8632Br::Br_g, IceInstX8632Br::Br_b },
                    { IceInstIcmp::Sle,     IceInstX8632Br::Br_l,
                      IceInstX8632Br::Br_g, IceInstX8632Br::Br_be }, };
const static uint32_t TableIcmp64Size =
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
const static uint32_t TableIcmp32Size =
    sizeof(TableIcmp32) / sizeof(*TableIcmp32);

static IceInstX8632Br::BrCond getIcmp32Mapping(IceInstIcmp::ICond Cond) {
  uint32_t Index = static_cast<uint32_t>(Cond);
  assert(Index < TableIcmp32Size);
  assert(TableIcmp32[Index].Cond == Cond);
  return TableIcmp32[Index].Mapping;
}

void IceTargetX8632::lowerIcmp(const IceInstIcmp *Inst, IceLoweringContext &C) {
  IceOperand *Src0 = Inst->getSrc(0);
  IceOperand *Src1 = Inst->getSrc(1);
  IceVariable *Dest = Inst->getDest();
  if (IceInstBr *NextBr = llvm::dyn_cast_or_null<IceInstBr>(C.getNextInst())) {
    if (Src0->getType() != IceType_i64 && !NextBr->isUnconditional() &&
        Dest == NextBr->getSrc(0) && NextBr->isLastUse(Dest)) {
      // This is basically identical to an Arithmetic instruction,
      // except there is no Dest variable to store.
      // cmp a,b ==> mov t,a; cmp t,b
      bool IsImmOrReg = false;
      if (llvm::isa<IceConstant>(Src1))
        IsImmOrReg = true;
      else if (IceVariable *Var = llvm::dyn_cast<IceVariable>(Src1)) {
        if (Var->hasReg())
          IsImmOrReg = true;
      }
      IceOperand *Reg =
          legalizeOperand(C, Src0, IsImmOrReg ? Legal_All : Legal_Reg, true);
      _icmp(C, Reg, Src1);
      _br(C, NextBr->getTargetTrue(), NextBr->getTargetFalse(),
          getIcmp32Mapping(Inst->getCondition()));
      NextBr->setDeleted();
      C.advanceNext();
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
    uint32_t Index = static_cast<uint32_t>(Condition);
    assert(Index < TableIcmp64Size);
    assert(TableIcmp64[Index].Cond == Condition);
    IceInstX8632Label *LabelFalse = IceInstX8632Label::create(Cfg, this);
    IceInstX8632Label *LabelTrue = IceInstX8632Label::create(Cfg, this);
    Src0 = legalizeOperand(C, Src0, Legal_All);
    Src1 = legalizeOperand(C, Src1, Legal_All);
    if (Condition == IceInstIcmp::Eq || Condition == IceInstIcmp::Ne) {
      _mov(C, Dest, (Condition == IceInstIcmp::Eq ? ConstZero : ConstOne));
      IceOperand *RegHi =
          legalizeOperand(C, hiOperand(C, Src1), Legal_Reg | Legal_Imm);
      _icmp(C, hiOperand(C, Src0), RegHi);
      _br(C, LabelFalse, IceInstX8632Br::Br_ne);
      IceOperand *RegLo =
          legalizeOperand(C, loOperand(C, Src1), Legal_Reg | Legal_Imm);
      _icmp(C, loOperand(C, Src0), RegLo);
      _br(C, LabelFalse, IceInstX8632Br::Br_ne);
      C.insert(LabelTrue);
      C.insert(IceInstFakeUse::create(Cfg, Dest));
      _mov(C, Dest, (Condition == IceInstIcmp::Eq ? ConstOne : ConstZero));
      C.insert(LabelFalse);
    } else {
      _mov(C, Dest, ConstOne);
      IceOperand *RegHi =
          legalizeOperand(C, hiOperand(C, Src1), Legal_Reg | Legal_Imm);
      _icmp(C, hiOperand(C, Src0), RegHi);
      _br(C, LabelTrue, TableIcmp64[Index].C1);
      _br(C, LabelFalse, TableIcmp64[Index].C2);
      IceOperand *RegLo =
          legalizeOperand(C, loOperand(C, Src1), Legal_Reg | Legal_Imm);
      _icmp(C, loOperand(C, Src0), RegLo);
      _br(C, LabelTrue, TableIcmp64[Index].C3);
      C.insert(LabelFalse);
      C.insert(IceInstFakeUse::create(Cfg, Dest));
      _mov(C, Dest, ConstZero);
      C.insert(LabelTrue);
    }
    return;
  }
  // cmp b, c
  bool IsImmOrReg = false;
  if (llvm::isa<IceConstant>(Src1))
    IsImmOrReg = true;
  else if (IceVariable *Var = llvm::dyn_cast<IceVariable>(Src1)) {
    if (Var->hasReg())
      IsImmOrReg = true;
  }
  IceOperand *Reg =
      legalizeOperand(C, Src0, IsImmOrReg ? Legal_All : Legal_Reg, true);
  _icmp(C, Reg, Src1);

  // a = 1;
  C.insert(
      IceInstX8632Mov::create(Cfg, Dest, Cfg->getConstantInt(IceType_i32, 1)));

  // create Label
  IceInstX8632Label *Label = IceInstX8632Label::create(Cfg, this);

  // br cond, Label
  _br(C, Label, getIcmp32Mapping(Inst->getCondition()));

  // FakeUse(a)
  IceInst *FakeUse = IceInstFakeUse::create(Cfg, Dest);
  C.insert(FakeUse);

  // a = 0
  C.insert(
      IceInstX8632Mov::create(Cfg, Dest, Cfg->getConstantInt(IceType_i32, 0)));

  // Label:
  C.insert(Label);
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
                              IceVariable *&Index, int32_t &Shift,
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

void IceTargetX8632::lowerLoad(const IceInstLoad *Inst, IceLoweringContext &C) {
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
          llvm::dyn_cast_or_null<IceInstArithmetic>(C.getNextInst())) {
    IceVariable *DestLoad = Inst->getDest();
    IceVariable *Src0Arith = llvm::dyn_cast<IceVariable>(Arith->getSrc(0));
    IceVariable *Src1Arith = llvm::dyn_cast<IceVariable>(Arith->getSrc(1));
    if (Src1Arith == DestLoad && Arith->isLastUse(Src1Arith) &&
        DestLoad != Src0Arith) {
      IceInstArithmetic *NewArith = IceInstArithmetic::create(
          Cfg, Arith->getOp(), Arith->getDest(), Arith->getSrc(0), Src);
      NewArith->setDeleted();
      C.advanceNext();
      lowerArithmetic(NewArith, C);
      return;
    } else if (Src0Arith == DestLoad && Arith->isCommutative() &&
               Arith->isLastUse(Src0Arith) && DestLoad != Src1Arith) {
      IceInstArithmetic *NewArith = IceInstArithmetic::create(
          Cfg, Arith->getOp(), Arith->getDest(), Arith->getSrc(1), Src);
      NewArith->setDeleted();
      C.advanceNext();
      lowerArithmetic(NewArith, C);
      return;
    }
  }

  IceInstAssign *Assign = IceInstAssign::create(Cfg, Inst->getDest(), Src);
  lowerAssign(Assign, C);
}

void IceTargetX8632::doAddressOptLoad(IceLoweringContext &C) {
  IceInst *Inst = *C.Cur;
  IceVariable *Dest = Inst->getDest();
  IceOperand *Addr = Inst->getSrc(0);
  IceVariable *Index = NULL;
  int32_t Shift = 0;
  int32_t Offset = 0; // TODO: make IceConstant
  IceVariable *Base = llvm::dyn_cast<IceVariable>(Addr);
  computeAddressOpt(Cfg, Base, Index, Shift, Offset);
  if (Base && Addr != Base) {
    IceConstant *OffsetOp = Cfg->getConstantInt(IceType_i32, Offset);
    Addr = IceOperandX8632Mem::create(Cfg, Dest->getType(), Base, OffsetOp,
                                      Index, Shift);
    Inst->setDeleted();
    C.insert(IceInstLoad::create(Cfg, Dest, Addr));
  }
}

void IceTargetX8632::lowerPhi(const IceInstPhi *Inst, IceLoweringContext &C) {
  Cfg->setError("Phi lowering not implemented");
}

void IceTargetX8632::lowerRet(const IceInstRet *Inst, IceLoweringContext &C) {
  IceVariable *Reg = NULL;
  if (Inst->getSrcSize()) {
    IceOperand *Src0 = legalizeOperand(C, Inst->getSrc(0), Legal_All);
    if (Src0->getType() == IceType_i64) {
      IceVariable *Src0Lo =
          legalizeOperandToVar(C, loOperand(C, Src0), false, Reg_eax);
      IceVariable *Src0Hi =
          legalizeOperandToVar(C, hiOperand(C, Src0), false, Reg_edx);
      Reg = Src0Lo;
      C.insert(IceInstFakeUse::create(Cfg, Src0Hi));
    } else if (Src0->getType() == IceType_f32 ||
               Src0->getType() == IceType_f64) {
      _fld(C, Src0);
    } else {
      Reg = legalizeOperandToVar(C, Src0, false, Reg_eax);
    }
  }
  _ret(C, Reg);
  // Add a fake use of esp to make sure esp stays alive for the entire
  // function.  Otherwise post-call esp adjustments get dead-code
  // eliminated.  TODO: Are there more places where the fake use
  // should be inserted?  E.g. "void f(int n){while(1) g(n);}" may not
  // have a ret instruction.
  IceVariable *esp = Cfg->getTarget()->getPhysicalRegister(Reg_esp);
  IceInst *FakeUse = IceInstFakeUse::create(Cfg, esp);
  C.insert(FakeUse);
}

void IceTargetX8632::lowerSelect(const IceInstSelect *Inst,
                                 IceLoweringContext &C) {
  // a=d?b:c ==> cmp d,0; a=b; jne L1; FakeUse(a); a=c; L1:
  //
  // Alternative if a is reg and c is not imm: cmp d,0; a=b; a=cmoveq c {a}
  IceOperand *Condition = legalizeOperand(C, Inst->getCondition(), Legal_All);
  IceConstant *OpZero = Cfg->getConstantInt(IceType_i32, 0);
  _icmp(C, Condition, OpZero);

  IceVariable *Dest = Inst->getDest();
  bool IsI64 = (Dest->getType() == IceType_i64);
  IceVariable *DestLo = NULL;
  IceVariable *DestHi = NULL;
  IceOperand *SrcTrue = Inst->getTrueOperand();
  if (IsI64) {
    DestLo = llvm::cast<IceVariable>(loOperand(C, Dest));
    DestHi = llvm::cast<IceVariable>(hiOperand(C, Dest));
    IceOperand *SrcTrueHi = hiOperand(C, SrcTrue);
    IceOperand *SrcTrueLo = loOperand(C, SrcTrue);
    IceOperand *RegHi =
        legalizeOperand(C, SrcTrueHi, Legal_Reg | Legal_Imm, true);
    _mov(C, DestHi, RegHi);
    IceOperand *RegLo =
        legalizeOperand(C, SrcTrueLo, Legal_Reg | Legal_Imm, true);
    _mov(C, DestLo, RegLo);
  } else {
    SrcTrue = legalizeOperand(C, SrcTrue, Legal_Reg | Legal_Imm, true);
    _mov(C, Dest, SrcTrue);
  }

  // create Label
  IceInstX8632Label *Label = IceInstX8632Label::create(Cfg, this);

  _br(C, Label, IceInstX8632Br::Br_ne);

  // FakeUse(a)
  if (IsI64) {
    C.insert(IceInstFakeUse::create(Cfg, DestLo));
    C.insert(IceInstFakeUse::create(Cfg, DestHi));
  } else {
    C.insert(IceInstFakeUse::create(Cfg, Dest));
  }

  IceOperand *SrcFalse = Inst->getFalseOperand();
  if (IsI64) {
    IceOperand *SrcFalseHi = hiOperand(C, SrcFalse);
    IceOperand *SrcFalseLo = loOperand(C, SrcFalse);
    IceOperand *RegHi =
        legalizeOperand(C, SrcFalseHi, Legal_Reg | Legal_Imm, true);
    _mov(C, DestHi, RegHi);
    IceOperand *RegLo =
        legalizeOperand(C, SrcFalseLo, Legal_Reg | Legal_Imm, true);
    _mov(C, DestLo, RegLo);
  } else {
    SrcFalse = legalizeOperand(C, SrcFalse, Legal_Reg | Legal_Imm, true);
    _mov(C, Dest, SrcFalse);
  }

  // Label:
  C.insert(Label);
}

void IceTargetX8632::lowerStore(const IceInstStore *Inst,
                                IceLoweringContext &C) {
  IceOperand *Value = Inst->getData();
  IceOperand *Addr = Inst->getAddr();
  IceOperandX8632Mem *NewAddr = llvm::dyn_cast<IceOperandX8632Mem>(Addr);
  if (!NewAddr) {
    IceVariable *Base = llvm::dyn_cast<IceVariable>(Addr);
    IceConstant *Offset = llvm::dyn_cast<IceConstant>(Addr);
    assert(Base || Offset);
    NewAddr = IceOperandX8632Mem::create(Cfg, Value->getType(), Base, Offset);
  }
  NewAddr =
      llvm::cast<IceOperandX8632Mem>(legalizeOperand(C, NewAddr, Legal_All));

  if (NewAddr->getType() == IceType_i64) {
    Value = legalizeOperand(C, Value, Legal_All);
    IceOperand *ValueHi = hiOperand(C, Value);
    IceOperand *ValueLo = loOperand(C, Value);
    Value = legalizeOperand(C, Value, Legal_Reg | Legal_Imm, true);
    _store(C, ValueHi, llvm::cast<IceOperandX8632Mem>(hiOperand(C, NewAddr)));
    _store(C, ValueLo, llvm::cast<IceOperandX8632Mem>(loOperand(C, NewAddr)));
  } else {
    Value = legalizeOperand(C, Value, Legal_Reg | Legal_Imm, true);
    _store(C, Value, NewAddr);
  }
}

void IceTargetX8632::doAddressOptStore(IceLoweringContext &C) {
  IceInstStore *Inst = llvm::cast<IceInstStore>(*C.Cur);
  IceOperand *Data = Inst->getData();
  IceOperand *Addr = Inst->getAddr();
  IceVariable *Index = NULL;
  int32_t Shift = 0;
  int32_t Offset = 0; // TODO: make IceConstant
  IceVariable *Base = llvm::dyn_cast<IceVariable>(Addr);
  computeAddressOpt(Cfg, Base, Index, Shift, Offset);
  if (Base && Addr != Base) {
    IceConstant *OffsetOp = Cfg->getConstantInt(IceType_i32, Offset);
    Addr = IceOperandX8632Mem::create(Cfg, Data->getType(), Base, OffsetOp,
                                      Index, Shift);
    Inst->setDeleted();
    C.insert(IceInstStore::create(Cfg, Data, Addr));
  }
}

void IceTargetX8632::lowerSwitch(const IceInstSwitch *Inst,
                                 IceLoweringContext &C) {
  // This implements the most naive possible lowering.
  // cmp a,val[0]; jeq label[0]; cmp a,val[1]; jeq label[1]; ... jmp default
  IceOperand *Src = Inst->getSrc(0);
  uint32_t NumCases = Inst->getNumCases();
  // OK, we'll be slightly less naive by forcing Src into a physical
  // register if there are 2 or more uses.
  if (NumCases >= 2)
    Src = legalizeOperandToVar(C, Src, true);
  else
    Src = legalizeOperand(C, Src, Legal_All, true);
  for (uint32_t I = 0; I < NumCases; ++I) {
    IceOperand *Value = Cfg->getConstantInt(IceType_i32, Inst->getValue(I));
    _icmp(C, Src, Value);
    C.insert(
        IceInstX8632Br::create(Cfg, Inst->getLabel(I), IceInstX8632Br::Br_e));
  }

  _br(C, Inst->getLabelDefault());
}

void IceTargetX8632::lowerUnreachable(const IceInstUnreachable *Inst,
                                      IceLoweringContext &C) {
  uint32_t MaxSrcs = 0;
  IceVariable *Dest = NULL;
  IceInstCall *Call =
      makeHelperCall("ice_unreachable", IceType_void, Dest, MaxSrcs);
  lowerCall(Call, C);
}

IceOperand *IceTargetX8632::legalizeOperand(IceLoweringContext &C,
                                            IceOperand *From, LegalMask Allowed,
                                            bool AllowOverlap, int32_t RegNum) {
  assert(Allowed & Legal_Reg);
  assert(RegNum == IceVariable::NoRegister || Allowed == Legal_Reg);
  if (IceOperandX8632Mem *Mem = llvm::dyn_cast<IceOperandX8632Mem>(From)) {
    IceVariable *Base = Mem->getBase();
    IceVariable *Index = Mem->getIndex();
    IceVariable *RegBase = Base;
    IceVariable *RegIndex = Index;
    if (Base) {
      RegBase = legalizeOperandToVar(C, Base, true);
    }
    if (Index) {
      RegIndex = legalizeOperandToVar(C, Index, true);
    }
    if (Base != RegBase || Index != RegIndex) {
      From = IceOperandX8632Mem::create(Cfg, Mem->getType(), RegBase,
                                        Mem->getOffset(), RegIndex,
                                        Mem->getShift());
    }

    if (!(Allowed & Legal_Mem)) {
      IceVariable *Reg = makeVariableWithReg(C, From->getType(), RegNum);
      _mov(C, Reg, From);
      From = Reg;
    }
    return From;
  }
  if (llvm::isa<IceConstant>(From)) {
    if (!(Allowed & Legal_Imm)) {
      IceVariable *Reg = makeVariableWithReg(C, From->getType(), RegNum);
      _mov(C, Reg, From);
      From = Reg;
    }
    return From;
  }
  if (IceVariable *Var = llvm::dyn_cast<IceVariable>(From)) {
    // We need a new physical register for the operand if:
    //   Mem is not allowed and Var->getRegNum() is unknown, or
    //   RegNum is required and Var->getRegNum() doesn't match.
    if ((!(Allowed & Legal_Mem) && !Var->hasReg()) ||
        (RegNum != IceVariable::NoRegister && RegNum != Var->getRegNum())) {
      IceVariable *Reg = makeVariableWithReg(C, From->getType(), RegNum);
      if (RegNum == IceVariable::NoRegister) {
        Reg->setPreferredRegister(Var, AllowOverlap);
      }
      _mov(C, Reg, From);
      From = Reg;
    }
    return From;
  }
  assert(0);
  return From;
}

IceVariable *IceTargetX8632::legalizeOperandToVar(IceLoweringContext &C,
                                                  IceOperand *From,
                                                  bool AllowOverlap,
                                                  int32_t RegNum) {
  return llvm::cast<IceVariable>(
      legalizeOperand(C, From, Legal_Reg, AllowOverlap, RegNum));
}

IceVariable *IceTargetX8632::makeVariableWithReg(IceLoweringContext &C,
                                                 IceType Type, int32_t RegNum) {
  IceVariable *Reg = Cfg->makeVariable(Type, C.Node);
  if (RegNum == IceVariable::NoRegister)
    Reg->setWeightInfinite();
  else
    Reg->setRegNum(RegNum);
  return Reg;
}

////////////////////////////////////////////////////////////////

void IceTargetX8632Fast::translate() {
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

void IceTargetX8632Fast::postLower(const IceLoweringContext &C) {
  llvm::SmallBitVector WhiteList = getRegisterSet(RegSet_All, RegSet_None);
  // Make one pass to black-list pre-colored registers.  TODO: If
  // there was some prior register allocation pass that made register
  // assignments, those registers need to be black-listed here as
  // well.
  for (IceInstList::iterator I = C.Cur, E = C.End; I != E; ++I) {
    const IceInst *Inst = *I;
    if (Inst->isDeleted())
      continue;
    if (llvm::isa<IceInstFakeKill>(Inst))
      continue;
    uint32_t VarIndex = 0;
    for (uint32_t SrcNum = 0; SrcNum < Inst->getSrcSize(); ++SrcNum) {
      IceOperand *Src = Inst->getSrc(SrcNum);
      uint32_t NumVars = Src->getNumVars();
      for (uint32_t J = 0; J < NumVars; ++J, ++VarIndex) {
        const IceVariable *Var = Src->getVar(J);
        if (!Var->hasReg())
          continue;
        WhiteList[Var->getRegNum()] = false;
      }
    }
  }
  // The second pass colors infinite-weight variables.
  llvm::SmallBitVector AvailableRegisters = WhiteList;
  for (IceInstList::iterator I = C.Cur, E = C.End; I != E; ++I) {
    const IceInst *Inst = *I;
    if (Inst->isDeleted())
      continue;
    uint32_t VarIndex = 0;
    for (uint32_t SrcNum = 0; SrcNum < Inst->getSrcSize(); ++SrcNum) {
      IceOperand *Src = Inst->getSrc(SrcNum);
      uint32_t NumVars = Src->getNumVars();
      for (uint32_t J = 0; J < NumVars; ++J, ++VarIndex) {
        IceVariable *Var = Src->getVar(J);
        if (Var->hasReg())
          continue;
        if (!Var->getWeight().isInf())
          continue;
        llvm::SmallBitVector AvailableTypedRegisters =
            AvailableRegisters & getRegisterSetForType(Var->getType());
        if (!AvailableTypedRegisters.any()) {
          // This is a hack in case we run out of physical registers
          // due to an excessive number of "push" instructions from
          // lowering a call.
          AvailableRegisters = WhiteList;
          AvailableTypedRegisters =
              AvailableRegisters & getRegisterSetForType(Var->getType());
        }
        assert(AvailableTypedRegisters.any());
        int32_t RegNum = AvailableTypedRegisters.find_first();
        Var->setRegNum(RegNum);
        AvailableRegisters[RegNum] = false;
      }
    }
  }
}
