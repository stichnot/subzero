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

namespace Ice {

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

void IceTargetX8632::translateO2() {
  IceGlobalContext *Context = Cfg->getContext();
  IceOstream &Str = Context->StrDump;
  IceTimer T_placePhiLoads;
  Cfg->placePhiLoads();
  if (Cfg->hasError())
    return;
  T_placePhiLoads.printElapsedUs(Context, "placePhiLoads()");
  IceTimer T_placePhiStores;
  Cfg->placePhiStores();
  if (Cfg->hasError())
    return;
  T_placePhiStores.printElapsedUs(Context, "placePhiStores()");
  IceTimer T_deletePhis;
  Cfg->deletePhis();
  if (Cfg->hasError())
    return;
  T_deletePhis.printElapsedUs(Context, "deletePhis()");
  IceTimer T_renumber1;
  Cfg->renumberInstructions();
  if (Cfg->hasError())
    return;
  T_renumber1.printElapsedUs(Context, "renumberInstructions()");
  if (Context->isVerbose())
    Str << "================ After Phi lowering ================\n";
  Cfg->dump();

  IceTimer T_doAddressOpt;
  Cfg->doAddressOpt();
  T_doAddressOpt.printElapsedUs(Context, "doAddressOpt()");
  // Liveness may be incorrect after address mode optimization.
  IceTimer T_renumber2;
  Cfg->renumberInstructions();
  if (Cfg->hasError())
    return;
  T_renumber2.printElapsedUs(Context, "renumberInstructions()");
  // TODO: It should be sufficient to use the fastest liveness
  // calculation, i.e. IceLiveness_LREndLightweight.  However, for
  // some reason that slows down the rest of the translation.
  // Investigate.
  IceTimer T_liveness1;
  Cfg->liveness(IceLiveness_LREndFull);
  if (Cfg->hasError())
    return;
  T_liveness1.printElapsedUs(Context, "liveness()");
  if (Context->isVerbose())
    Str << "================ After x86 address mode opt ================\n";
  Cfg->dump();
  IceTimer T_genCode;
  Cfg->genCode();
  if (Cfg->hasError())
    return;
  T_genCode.printElapsedUs(Context, "genCode()");
  IceTimer T_renumber3;
  Cfg->renumberInstructions();
  if (Cfg->hasError())
    return;
  T_renumber3.printElapsedUs(Context, "renumberInstructions()");
  IceTimer T_liveness2;
  Cfg->liveness(IceLiveness_RangesFull);
  if (Cfg->hasError())
    return;
  T_liveness2.printElapsedUs(Context, "liveness()");
  ComputedLiveRanges = true;
  if (Context->isVerbose())
    Str << "================ After initial x8632 codegen ================\n";
  Cfg->dump();

  IceTimer T_regAlloc;
  regAlloc();
  if (Cfg->hasError())
    return;
  T_regAlloc.printElapsedUs(Context, "regAlloc()");
  if (Context->isVerbose())
    Str << "================ After linear scan regalloc ================\n";
  Cfg->dump();

  IceTimer T_genFrame;
  Cfg->genFrame();
  if (Cfg->hasError())
    return;
  T_genFrame.printElapsedUs(Context, "genFrame()");
  if (Context->isVerbose())
    Str << "================ After stack frame mapping ================\n";
  Cfg->dump();
}

void IceTargetX8632::translateOm1() {
  IceGlobalContext *Context = Cfg->getContext();
  IceOstream &Str = Context->StrDump;
  IceTimer T_placePhiLoads;
  Cfg->placePhiLoads();
  if (Cfg->hasError())
    return;
  T_placePhiLoads.printElapsedUs(Context, "placePhiLoads()");
  IceTimer T_placePhiStores;
  Cfg->placePhiStores();
  if (Cfg->hasError())
    return;
  T_placePhiStores.printElapsedUs(Context, "placePhiStores()");
  IceTimer T_deletePhis;
  Cfg->deletePhis();
  if (Cfg->hasError())
    return;
  T_deletePhis.printElapsedUs(Context, "deletePhis()");
  if (Context->isVerbose())
    Str << "================ After Phi lowering ================\n";
  Cfg->dump();

  IceTimer T_genCode;
  Cfg->genCode();
  if (Cfg->hasError())
    return;
  T_genCode.printElapsedUs(Context, "genCode()");
  if (Context->isVerbose())
    Str << "================ After initial x8632 codegen ================\n";
  Cfg->dump();

  IceTimer T_genFrame;
  Cfg->genFrame();
  if (Cfg->hasError())
    return;
  T_genFrame.printElapsedUs(Context, "genFrame()");
  if (Context->isVerbose())
    Str << "================ After stack frame mapping ================\n";
  Cfg->dump();
}

IceString IceTargetX8632::RegNames[] = { "eax",  "ecx",  "edx",  "ebx",  "esp",
                                         "ebp",  "esi",  "edi",  "???",  "xmm0",
                                         "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                                         "xmm6", "xmm7" };

IceVariable *IceTargetX8632::getPhysicalRegister(uint32_t RegNum) {
  assert(RegNum < PhysicalRegisters.size());
  IceVariable *Reg = PhysicalRegisters[RegNum];
  if (Reg == NULL) {
    CfgNode *Node = NULL; // NULL means multi-block lifetime
    Reg = Cfg->makeVariable(IceType_i32, Node);
    Reg->setRegNum(RegNum);
    PhysicalRegisters[RegNum] = Reg;
  }
  return Reg;
}

IceString IceTargetX8632::getRegName(uint32_t RegNum, IceType Type) const {
  assert(RegNum < Reg_NUM);
  static IceString RegNames8[] = { "al", "cl", "dl", "bl", "??",
                                   "??", "??", "??", "ah" };
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
void IceTargetX8632::setArgOffsetAndCopy(IceVariable *Arg,
                                         IceVariable *FramePtr,
                                         int32_t BasicFrameOffset,
                                         int32_t &InArgsSizeBytes) {
  IceVariable *Lo = Arg->getLo();
  IceVariable *Hi = Arg->getHi();
  IceType Type = Arg->getType();
  if (Lo && Hi && Type == IceType_i64) {
    assert(Lo->getType() != IceType_i64); // don't want infinite recursion
    assert(Hi->getType() != IceType_i64); // don't want infinite recursion
    setArgOffsetAndCopy(Lo, FramePtr, BasicFrameOffset, InArgsSizeBytes);
    setArgOffsetAndCopy(Hi, FramePtr, BasicFrameOffset, InArgsSizeBytes);
    return;
  }
  Arg->setStackOffset(BasicFrameOffset + InArgsSizeBytes);
  if (Arg->hasReg()) {
    assert(Type != IceType_i64);
    IceOperandX8632Mem *Mem = IceOperandX8632Mem::create(
        Cfg, Type, FramePtr,
        Ctx->getConstantInt(IceType_i32, Arg->getStackOffset()));
    _mov(Arg, Mem);
  }
  InArgsSizeBytes += typeWidthInBytesOnStack(Type);
}

void IceTargetX8632::addProlog(CfgNode *Node) {
  // If SimpleCoalescing is false, each variable without a register
  // gets its own unique stack slot, which leads to large stack
  // frames.  If SimpleCoalescing is true, then each "global" variable
  // without a register gets its own slot, but "local" variable slots
  // are reused across basic blocks.  E.g., if A and B are local to
  // block 1 and C is local to block 2, then C may share a slot with A
  // or B.
  const bool SimpleCoalescing = true;
  int32_t InArgsSizeBytes = 0;
  int32_t RetIpSizeBytes = 4;
  int32_t PreservedRegsSizeBytes = 0;
  LocalsSizeBytes = 0;
  Context.init(Node);
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
    int32_t Increment = typeWidthInBytesOnStack(Var->getType());
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
      bool SuppressStackAdjustment = true;
      _push(getPhysicalRegister(i), SuppressStackAdjustment);
    }
  }

  // Generate "push ebp; mov ebp, esp"
  if (IsEbpBasedFrame) {
    assert((RegsUsed & getRegisterSet(RegSet_FramePointer, RegSet_None))
               .count() == 0);
    PreservedRegsSizeBytes += 4;
    IceVariable *ebp = getPhysicalRegister(Reg_ebp);
    IceVariable *esp = getPhysicalRegister(Reg_esp);
    bool SuppressStackAdjustment = true;
    _push(ebp, SuppressStackAdjustment);
    _mov(ebp, esp);
  }

  // Generate "sub esp, LocalsSizeBytes"
  if (LocalsSizeBytes)
    _sub(getPhysicalRegister(Reg_esp),
         Ctx->getConstantInt(IceType_i32, LocalsSizeBytes));

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
    setArgOffsetAndCopy(Arg, FramePtr, BasicFrameOffset, InArgsSizeBytes);
  }

  // Fill in stack offsets for locals.
  int32_t TotalGlobalsSize = GlobalsSize;
  GlobalsSize = 0;
  LocalsSize.assign(LocalsSize.size(), 0);
  int32_t NextStackOffset = 0;
  for (IceVarList::const_iterator I = Variables.begin(), E = Variables.end();
       I != E; ++I) {
    IceVariable *Var = *I;
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
    int32_t Increment = typeWidthInBytesOnStack(Var->getType());
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

  if (Cfg->getContext()->isVerbose(IceV_Frame)) {
    Cfg->getContext()->StrDump << "LocalsSizeBytes=" << LocalsSizeBytes << "\n"
                               << "InArgsSizeBytes=" << InArgsSizeBytes << "\n"
                               << "PreservedRegsSizeBytes="
                               << PreservedRegsSizeBytes << "\n";
  }
}

void IceTargetX8632::addEpilog(CfgNode *Node) {
  InstList &Insts = Node->getInsts();
  InstList::reverse_iterator RI, E;
  for (RI = Insts.rbegin(), E = Insts.rend(); RI != E; ++RI) {
    if (llvm::isa<InstX8632Ret>(*RI))
      break;
  }
  if (RI == E)
    return;

  // Convert the reverse_iterator position into its corresponding
  // (forward) iterator position.
  InstList::iterator InsertPoint = RI.base();
  --InsertPoint;
  Context.init(Node);
  Context.Next = InsertPoint;

  IceVariable *esp = getPhysicalRegister(Reg_esp);
  if (IsEbpBasedFrame) {
    IceVariable *ebp = getPhysicalRegister(Reg_ebp);
    _mov(esp, ebp);
    _pop(ebp);
  } else {
    // add esp, LocalsSizeBytes
    if (LocalsSizeBytes)
      _add(esp, Ctx->getConstantInt(IceType_i32, LocalsSizeBytes));
  }

  // Add pop instructions for preserved registers.
  llvm::SmallBitVector CalleeSaves =
      getRegisterSet(RegSet_CalleeSave, RegSet_None);
  for (uint32_t i = 0; i < CalleeSaves.size(); ++i) {
    uint32_t j = CalleeSaves.size() - i - 1;
    if (j == Reg_ebp && IsEbpBasedFrame)
      continue;
    if (CalleeSaves[j] && RegsUsed[j]) {
      _pop(getPhysicalRegister(j));
    }
  }
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
  IceVariable *Lo = Var->getLo();
  IceVariable *Hi = Var->getHi();
  if (Lo) {
    assert(Hi);
    return;
  }
  assert(Hi == NULL);
  Lo = Cfg->makeVariable(IceType_i32, Context.getNode(),
                         Var->getName() + "__lo");
  Hi = Cfg->makeVariable(IceType_i32, Context.getNode(),
                         Var->getName() + "__hi");
  Var->setLoHi(Lo, Hi);
  if (Var->getIsArg()) {
    Lo->setIsArg(Cfg);
    Hi->setIsArg(Cfg);
  }
}

IceOperand *IceTargetX8632::loOperand(IceOperand *Operand) {
  assert(Operand->getType() == IceType_i64);
  if (Operand->getType() != IceType_i64)
    return Operand;
  if (IceVariable *Var = llvm::dyn_cast<IceVariable>(Operand)) {
    split64(Var);
    return Var->getLo();
  }
  if (IceConstantInteger *Const = llvm::dyn_cast<IceConstantInteger>(Operand)) {
    uint64_t Mask = (1ul << 32) - 1;
    return Ctx->getConstantInt(IceType_i32, Const->getValue() & Mask);
  }
  if (IceOperandX8632Mem *Mem = llvm::dyn_cast<IceOperandX8632Mem>(Operand)) {
    return IceOperandX8632Mem::create(Cfg, IceType_i32, Mem->getBase(),
                                      Mem->getOffset(), Mem->getIndex(),
                                      Mem->getShift());
  }
  assert(0 && "Unsupported operand type");
  return NULL;
}

IceOperand *IceTargetX8632::hiOperand(IceOperand *Operand) {
  assert(Operand->getType() == IceType_i64);
  if (Operand->getType() != IceType_i64)
    return Operand;
  if (IceVariable *Var = llvm::dyn_cast<IceVariable>(Operand)) {
    split64(Var);
    return Var->getHi();
  }
  if (IceConstantInteger *Const = llvm::dyn_cast<IceConstantInteger>(Operand)) {
    return Ctx->getConstantInt(IceType_i32, Const->getValue() >> 32);
  }
  if (IceOperandX8632Mem *Mem = llvm::dyn_cast<IceOperandX8632Mem>(Operand)) {
    IceConstant *Offset = Mem->getOffset();
    if (Offset == NULL)
      Offset = Ctx->getConstantInt(IceType_i32, 4);
    else if (IceConstantInteger *IntOffset =
                 llvm::dyn_cast<IceConstantInteger>(Offset)) {
      Offset = Ctx->getConstantInt(IceType_i32, 4 + IntOffset->getValue());
    } else if (IceConstantRelocatable *SymOffset =
                   llvm::dyn_cast<IceConstantRelocatable>(Offset)) {
      // TODO: This creates a new entry in the constant pool, instead
      // of reusing the existing entry.
      Offset =
          Ctx->getConstantSym(IceType_i32, SymOffset->getHandle(),
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

void IceTargetX8632::lowerAlloca(const InstAlloca *Inst) {
  IsEbpBasedFrame = true;
  // TODO(sehr,stichnot): align allocated memory, keep stack aligned, minimize
  // the number of adjustments of esp, etc.
  IceVariable *esp = getPhysicalRegister(Reg_esp);
  IceOperand *TotalSize = legalize(Inst->getSrc(0));
  IceVariable *Dest = Inst->getDest();
  _sub(esp, TotalSize);
  _mov(Dest, esp);
}

void IceTargetX8632::lowerArithmetic(const InstArithmetic *Inst) {
  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = legalize(Inst->getSrc(0));
  IceOperand *Src1 = legalize(Inst->getSrc(1));
  if (Dest->getType() == IceType_i64) {
    IceVariable *DestLo = llvm::cast<IceVariable>(loOperand(Dest));
    IceVariable *DestHi = llvm::cast<IceVariable>(hiOperand(Dest));
    IceOperand *Src0Lo = loOperand(Src0);
    IceOperand *Src0Hi = hiOperand(Src0);
    IceOperand *Src1Lo = loOperand(Src1);
    IceOperand *Src1Hi = hiOperand(Src1);
    IceVariable *T_Lo = NULL, *T_Hi = NULL;
    switch (Inst->getOp()) {
    case InstArithmetic::Add:
      _mov(T_Lo, Src0Lo);
      _add(T_Lo, Src1Lo);
      _mov(DestLo, T_Lo);
      _mov(T_Hi, Src0Hi);
      _adc(T_Hi, Src1Hi);
      _mov(DestHi, T_Hi);
      break;
    case InstArithmetic::And:
      _mov(T_Lo, Src0Lo);
      _and(T_Lo, Src1Lo);
      _mov(DestLo, T_Lo);
      _mov(T_Hi, Src0Hi);
      _and(T_Hi, Src1Hi);
      _mov(DestHi, T_Hi);
      break;
    case InstArithmetic::Or:
      _mov(T_Lo, Src0Lo);
      _or(T_Lo, Src1Lo);
      _mov(DestLo, T_Lo);
      _mov(T_Hi, Src0Hi);
      _or(T_Hi, Src1Hi);
      _mov(DestHi, T_Hi);
      break;
    case InstArithmetic::Xor:
      _mov(T_Lo, Src0Lo);
      _xor(T_Lo, Src1Lo);
      _mov(DestLo, T_Lo);
      _mov(T_Hi, Src0Hi);
      _xor(T_Hi, Src1Hi);
      _mov(DestHi, T_Hi);
      break;
    case InstArithmetic::Sub:
      _mov(T_Lo, Src0Lo);
      _sub(T_Lo, Src1Lo);
      _mov(DestLo, T_Lo);
      _mov(T_Hi, Src0Hi);
      _sbb(T_Hi, Src1Hi);
      _mov(DestHi, T_Hi);
      break;
    case InstArithmetic::Mul: {
      IceVariable *T_1 = NULL, *T_2 = NULL, *T_3 = NULL;
      IceVariable *T_4Lo = makeReg(IceType_i32, Reg_eax);
      IceVariable *T_4Hi = makeReg(IceType_i32, Reg_edx);
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
      _mov(T_1, Src0Hi);
      _imul(T_1, Src1Lo);
      _mov(T_2, Src1Hi);
      _imul(T_2, Src0Lo);
      _mov(T_3, Src0Lo, Reg_eax);
      _mul(T_4Lo, T_3, Src1Lo);
      // The mul instruction produces two dest variables, edx:eax.  We
      // create a fake definition of edx to account for this.
      Context.insert(InstFakeDef::create(Cfg, T_4Hi, T_4Lo));
      _mov(DestLo, T_4Lo);
      _add(T_4Hi, T_1);
      _add(T_4Hi, T_2);
      _mov(DestHi, T_4Hi);
    } break;
    case InstArithmetic::Shl: {
      // TODO: Refactor the similarities between Shl, Lshr, and Ashr.
      // gcc does the following:
      // a=b<<c ==>
      //   t1:ecx = c.lo & 0xff
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
      IceVariable *T_1 = NULL, *T_2 = NULL, *T_3 = NULL;
      IceConstant *BitTest = Ctx->getConstantInt(IceType_i32, 0x20);
      IceConstant *Zero = Ctx->getConstantInt(IceType_i32, 0);
      InstX8632Label *Label = InstX8632Label::create(Cfg, this);
      _mov(T_1, Src1Lo, Reg_ecx);
      _mov(T_2, Src0Lo);
      _mov(T_3, Src0Hi);
      _shld(T_3, T_2, T_1);
      _shl(T_2, T_1);
      _test(T_1, BitTest);
      _br(InstX8632Br::Br_e, Label);
      // Because of the intra-block control flow, we need to fake a use
      // of T_3 to prevent its earlier definition from being dead-code
      // eliminated in the presence of its later definition.
      Context.insert(InstFakeUse::create(Cfg, T_3));
      _mov(T_3, T_2);
      _mov(T_2, Zero);
      Context.insert(Label);
      _mov(DestLo, T_2);
      _mov(DestHi, T_3);
    } break;
    case InstArithmetic::Lshr: {
      // a=b>>c (unsigned) ==>
      //   t1:ecx = c.lo & 0xff
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
      IceVariable *T_1 = NULL, *T_2 = NULL, *T_3 = NULL;
      IceConstant *BitTest = Ctx->getConstantInt(IceType_i32, 0x20);
      IceConstant *Zero = Ctx->getConstantInt(IceType_i32, 0);
      InstX8632Label *Label = InstX8632Label::create(Cfg, this);
      _mov(T_1, Src1Lo, Reg_ecx);
      _mov(T_2, Src0Lo);
      _mov(T_3, Src0Hi);
      _shrd(T_2, T_3, T_1);
      _shr(T_3, T_1);
      _test(T_1, BitTest);
      _br(InstX8632Br::Br_e, Label);
      // Because of the intra-block control flow, we need to fake a use
      // of T_3 to prevent its earlier definition from being dead-code
      // eliminated in the presence of its later definition.
      Context.insert(InstFakeUse::create(Cfg, T_2));
      _mov(T_2, T_3);
      _mov(T_3, Zero);
      Context.insert(Label);
      _mov(DestLo, T_2);
      _mov(DestHi, T_3);
    } break;
    case InstArithmetic::Ashr: {
      // a=b>>c (signed) ==>
      //   t1:ecx = c.lo & 0xff
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
      IceVariable *T_1 = NULL, *T_2 = NULL, *T_3 = NULL;
      IceConstant *BitTest = Ctx->getConstantInt(IceType_i32, 0x20);
      IceConstant *SignExtend = Ctx->getConstantInt(IceType_i32, 0x1f);
      InstX8632Label *Label = InstX8632Label::create(Cfg, this);
      _mov(T_1, Src1Lo, Reg_ecx);
      _mov(T_2, Src0Lo);
      _mov(T_3, Src0Hi);
      _shrd(T_2, T_3, T_1);
      _sar(T_3, T_1);
      _test(T_1, BitTest);
      _br(InstX8632Br::Br_e, Label);
      // Because of the intra-block control flow, we need to fake a use
      // of T_3 to prevent its earlier definition from being dead-code
      // eliminated in the presence of its later definition.
      Context.insert(InstFakeUse::create(Cfg, T_2));
      _mov(T_2, T_3);
      _sar(T_3, SignExtend);
      Context.insert(Label);
      _mov(DestLo, T_2);
      _mov(DestHi, T_3);
    } break;
    case InstArithmetic::Udiv: {
      uint32_t MaxSrcs = 2;
      InstCall *Call = makeHelperCall("__udivdi3", Dest, MaxSrcs);
      Call->addArg(Inst->getSrc(0));
      Call->addArg(Inst->getSrc(1));
      lowerCall(Call);
    } break;
    case InstArithmetic::Sdiv: {
      uint32_t MaxSrcs = 2;
      InstCall *Call = makeHelperCall("__divdi3", Dest, MaxSrcs);
      Call->addArg(Inst->getSrc(0));
      Call->addArg(Inst->getSrc(1));
      lowerCall(Call);
    } break;
    case InstArithmetic::Urem: {
      uint32_t MaxSrcs = 2;
      InstCall *Call = makeHelperCall("__umoddi3", Dest, MaxSrcs);
      Call->addArg(Inst->getSrc(0));
      Call->addArg(Inst->getSrc(1));
      lowerCall(Call);
    } break;
    case InstArithmetic::Srem: {
      uint32_t MaxSrcs = 2;
      InstCall *Call = makeHelperCall("__moddi3", Dest, MaxSrcs);
      Call->addArg(Inst->getSrc(0));
      Call->addArg(Inst->getSrc(1));
      lowerCall(Call);
    } break;
    case InstArithmetic::Fadd:
    case InstArithmetic::Fsub:
    case InstArithmetic::Fmul:
    case InstArithmetic::Fdiv:
    case InstArithmetic::Frem:
    case InstArithmetic::OpKind_NUM:
      assert(0);
      break;
    }
  } else { // Dest->getType() != IceType_i64
    IceVariable *T_edx = NULL;
    IceVariable *T = NULL;
    switch (Inst->getOp()) {
    case InstArithmetic::Add:
      _mov(T, Src0);
      _add(T, Src1);
      _mov(Dest, T);
      break;
    case InstArithmetic::And:
      _mov(T, Src0);
      _and(T, Src1);
      _mov(Dest, T);
      break;
    case InstArithmetic::Or:
      _mov(T, Src0);
      _or(T, Src1);
      _mov(Dest, T);
      break;
    case InstArithmetic::Xor:
      _mov(T, Src0);
      _xor(T, Src1);
      _mov(Dest, T);
      break;
    case InstArithmetic::Sub:
      _mov(T, Src0);
      _sub(T, Src1);
      _mov(Dest, T);
      break;
    case InstArithmetic::Mul:
      // TODO: Optimize for llvm::isa<IceConstant>(Src1)
      // TODO: Strength-reduce multiplications by a constant,
      // particularly -1 and powers of 2.  Advanced: use lea to
      // multiply by 3, 5, 9.
      //
      // The 8-bit version of imul only allows the form "imul r/m8"
      // where T must be in eax.
      if (Dest->getType() == IceType_i8)
        _mov(T, Src0, Reg_eax);
      else
        _mov(T, Src0);
      _imul(T, Src1);
      _mov(Dest, T);
      break;
    case InstArithmetic::Shl:
      _mov(T, Src0);
      if (!llvm::isa<IceConstant>(Src1))
        Src1 = legalizeToVar(Src1, false, Reg_ecx);
      _shl(T, Src1);
      _mov(Dest, T);
      break;
    case InstArithmetic::Lshr:
      _mov(T, Src0);
      if (!llvm::isa<IceConstant>(Src1))
        Src1 = legalizeToVar(Src1, false, Reg_ecx);
      _shr(T, Src1);
      _mov(Dest, T);
      break;
    case InstArithmetic::Ashr:
      _mov(T, Src0);
      if (!llvm::isa<IceConstant>(Src1))
        Src1 = legalizeToVar(Src1, false, Reg_ecx);
      _sar(T, Src1);
      _mov(Dest, T);
      break;
    case InstArithmetic::Udiv:
      if (Dest->getType() == IceType_i8) {
        IceVariable *T_ah = NULL;
        IceConstant *Zero = Ctx->getConstantInt(IceType_i8, 0);
        _mov(T, Src0, Reg_eax);
        _mov(T_ah, Zero, Reg_ah);
        _div(T_ah, Src1, T);
        Context.insert(InstFakeUse::create(Cfg, T_ah));
        _mov(Dest, T);
      } else {
        // TODO: fix for 8-bit, see Urem
        IceConstant *Zero = Ctx->getConstantInt(IceType_i32, 0);
        _mov(T, Src0, Reg_eax);
        _mov(T_edx, Zero, Reg_edx);
        _div(T, Src1, T_edx);
        _mov(Dest, T);
      }
      break;
    case InstArithmetic::Sdiv:
      T_edx = makeReg(IceType_i32, Reg_edx);
      _mov(T, Src0, Reg_eax);
      _cdq(T_edx, T);
      _idiv(T, Src1, T_edx);
      _mov(Dest, T);
      break;
    case InstArithmetic::Urem:
      if (Dest->getType() == IceType_i8) {
        IceVariable *T_ah = NULL;
        IceConstant *Zero = Ctx->getConstantInt(IceType_i8, 0);
        _mov(T, Src0, Reg_eax);
        _mov(T_ah, Zero, Reg_ah);
        _div(T_ah, Src1, T);
        _mov(Dest, T_ah);
      } else {
        IceConstant *Zero = Ctx->getConstantInt(IceType_i32, 0);
        _mov(T_edx, Zero, Reg_edx);
        _mov(T, Src0, Reg_eax);
        _div(T_edx, Src1, T);
        _mov(Dest, T_edx);
      }
      break;
    case InstArithmetic::Srem:
      T_edx = makeReg(IceType_i32, Reg_edx);
      _mov(T, Src0, Reg_eax);
      _cdq(T_edx, T);
      _idiv(T_edx, Src1, T);
      _mov(Dest, T_edx);
      break;
    case InstArithmetic::Fadd:
      _mov(T, Src0);
      _addss(T, Src1);
      _mov(Dest, T);
      break;
    case InstArithmetic::Fsub:
      _mov(T, Src0);
      _subss(T, Src1);
      _mov(Dest, T);
      break;
    case InstArithmetic::Fmul:
      _mov(T, Src0);
      _mulss(T, Src1);
      _mov(Dest, T);
      break;
    case InstArithmetic::Fdiv:
      _mov(T, Src0);
      _divss(T, Src1);
      _mov(Dest, T);
      break;
    case InstArithmetic::Frem: {
      uint32_t MaxSrcs = 2;
      IceType Type = Dest->getType();
      InstCall *Call =
          makeHelperCall(Type == IceType_f32 ? "fmodf" : "fmod", Dest, MaxSrcs);
      Call->addArg(Src0);
      Call->addArg(Src1);
      return lowerCall(Call);
    } break;
    case InstArithmetic::OpKind_NUM:
      assert(0);
      break;
    }
  }
}

void IceTargetX8632::lowerAssign(const InstAssign *Inst) {
  IceVariable *Dest = Inst->getDest();
  IceOperand *Src0 = legalize(Inst->getSrc(0));
  assert(Dest->getType() == Src0->getType());
  if (Dest->getType() == IceType_i64) {
    IceOperand *Src0Lo = loOperand(Src0);
    IceOperand *Src0Hi = hiOperand(Src0);
    IceVariable *DestLo = llvm::cast<IceVariable>(loOperand(Dest));
    IceVariable *DestHi = llvm::cast<IceVariable>(hiOperand(Dest));
    IceVariable *T_Lo = NULL, *T_Hi = NULL;
    _mov(T_Lo, Src0Lo);
    _mov(DestLo, T_Lo);
    _mov(T_Hi, Src0Hi);
    _mov(DestHi, T_Hi);
  } else {
    bool AllowOverlap = true;
    // RI is either a physical register or an immediate.
    IceOperand *RI = legalize(Src0, Legal_Reg | Legal_Imm, AllowOverlap);
    _mov(Dest, RI);
  }
}

void IceTargetX8632::lowerBr(const InstBr *Inst) {
  if (Inst->isUnconditional()) {
    _br(Inst->getTargetUnconditional());
  } else {
    IceOperand *Src0 = legalize(Inst->getSrc(0));
    IceConstant *Zero = Ctx->getConstantInt(IceType_i32, 0);
    _cmp(Src0, Zero);
    _br(InstX8632Br::Br_ne, Inst->getTargetTrue(), Inst->getTargetFalse());
  }
}

void IceTargetX8632::lowerCall(const InstCall *Instr) {
  // TODO: what to do about tailcalls?
  // Generate a sequence of push instructions, pushing right to left,
  // keeping track of stack offsets in case a push involves a stack
  // operand and we are using an esp-based frame.
  uint32_t StackOffset = 0;
  // TODO: If for some reason the call instruction gets dead-code
  // eliminated after lowering, we would need to ensure that the
  // pre-call push instructions and the post-call esp adjustment get
  // eliminated as well.
  for (uint32_t NumArgs = Instr->getNumArgs(), i = 0; i < NumArgs; ++i) {
    IceOperand *Arg = legalize(Instr->getArg(NumArgs - i - 1));
    if (Arg->getType() == IceType_i64) {
      _push(hiOperand(Arg));
      _push(loOperand(Arg));
    } else if (Arg->getType() == IceType_f64) {
      // If the Arg turns out to be a memory operand, we need to push
      // 8 bytes, which requires two push instructions.  This ends up
      // being somewhat clumsy in the current IR, so we use a
      // workaround.  Force the operand into a (xmm) register, and
      // then push the register.  An xmm register push is actually not
      // possible in x86, but the Push instruction emitter handles
      // this by decrementing the stack pointer and directly writing
      // the xmm register value.
      IceVariable *T = NULL;
      _mov(T, Arg);
      _push(T);
    } else {
      _push(Arg);
    }
    StackOffset += typeWidthInBytesOnStack(Arg->getType());
  }
  // Generate the call instruction.  Assign its result to a temporary
  // with high register allocation weight.
  IceVariable *Dest = Instr->getDest();
  IceVariable *eax = NULL; // doubles as RegLo as necessary
  IceVariable *edx = NULL;
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
      eax = makeReg(Dest->getType(), Reg_eax);
      break;
    case IceType_i64:
      eax = makeReg(IceType_i32, Reg_eax);
      edx = makeReg(IceType_i32, Reg_edx);
      break;
    case IceType_f32:
    case IceType_f64:
      // Leave eax==edx==NULL, and capture the result with the fstp
      // instruction.
      break;
    }
  }
  IceOperand *CallTarget = legalize(Instr->getCallTarget());
  Inst *NewCall = InstX8632Call::create(Cfg, eax, CallTarget, Instr->isTail());
  Context.insert(NewCall);
  if (edx)
    Context.insert(InstFakeDef::create(Cfg, edx));

  // Add the appropriate offset to esp.
  if (StackOffset) {
    IceVariable *esp = Cfg->getTarget()->getPhysicalRegister(Reg_esp);
    _add(esp, Ctx->getConstantInt(IceType_i32, StackOffset));
  }

  // Insert a register-kill pseudo instruction.
  IceVarList KilledRegs;
  for (uint32_t i = 0; i < ScratchRegs.size(); ++i) {
    if (ScratchRegs[i])
      KilledRegs.push_back(Cfg->getTarget()->getPhysicalRegister(i));
  }
  if (!KilledRegs.empty()) {
    Inst *Kill = InstFakeKill::create(Cfg, KilledRegs, NewCall);
    Context.insert(Kill);
  }

  // Generate a FakeUse to keep the call live if necessary.
  if (Instr->hasSideEffects() && eax) {
    Inst *FakeUse = InstFakeUse::create(Cfg, eax);
    Context.insert(FakeUse);
  }

  // Generate Dest=eax assignment.
  if (Dest && eax) {
    if (edx) {
      split64(Dest);
      IceVariable *DestLo = Dest->getLo();
      IceVariable *DestHi = Dest->getHi();
      DestLo->setPreferredRegister(eax, false);
      DestHi->setPreferredRegister(edx, false);
      _mov(DestLo, eax);
      _mov(DestHi, edx);
    } else {
      Dest->setPreferredRegister(eax, false);
      _mov(Dest, eax);
    }
  }

  // Special treatment for an FP function which returns its result in
  // st(0).
  if (Dest &&
      (Dest->getType() == IceType_f32 || Dest->getType() == IceType_f64)) {
    _fstp(Dest);
    // If Dest ends up being a physical xmm register, the fstp emit
    // code will route st(0) through a temporary stack slot.
  }
}

void IceTargetX8632::lowerCast(const InstCast *Inst) {
  // a = cast(b) ==> t=cast(b); a=t; (link t->b, link a->t, no overlap)
  InstCast::OpKind CastKind = Inst->getCastKind();
  IceVariable *Dest = Inst->getDest();
  // Src0RM is the source operand legalized to physical register or memory, but
  // not immediate, since the relevant x86 native instructions don't allow an
  // immediate operand.  If the operand is an immediate, we could consider
  // computing the strength-reduced result at translation time, but we're
  // unlikely to see something like that in the bitcode that the optimizer
  // wouldn't have already taken care of.
  IceOperand *Src0RM = legalize(Inst->getSrc(0), Legal_Reg | Legal_Mem, true);
  switch (CastKind) {
  default:
    Cfg->setError("Cast type not supported");
    return;
  case InstCast::Sext:
    if (Dest->getType() == IceType_i64) {
      // t1=movsx src; t2=t1; t2=sar t2, 31; dst.lo=t1; dst.hi=t2
      IceVariable *DestLo = llvm::cast<IceVariable>(loOperand(Dest));
      IceVariable *DestHi = llvm::cast<IceVariable>(hiOperand(Dest));
      IceVariable *T_Lo = makeReg(DestLo->getType());
      if (Src0RM->getType() == IceType_i32)
        _mov(T_Lo, Src0RM);
      else
        _movsx(T_Lo, Src0RM);
      _mov(DestLo, T_Lo);
      IceVariable *T_Hi = NULL;
      IceConstant *Shift = Ctx->getConstantInt(IceType_i32, 31);
      _mov(T_Hi, T_Lo);
      _sar(T_Hi, Shift);
      _mov(DestHi, T_Hi);
    } else {
      // TODO: Sign-extend an i1 via "shl reg, 31; sar reg, 31", and
      // also copy to the high operand of a 64-bit variable.
      // t1 = movsx src; dst = t1
      IceVariable *T = makeReg(Dest->getType());
      _movsx(T, Src0RM);
      _mov(Dest, T);
    }
    break;
  case InstCast::Zext:
    if (Dest->getType() == IceType_i64) {
      // t1=movzx src; dst.lo=t1; dst.hi=0
      IceConstant *Zero = Ctx->getConstantInt(IceType_i32, 0);
      IceVariable *DestLo = llvm::cast<IceVariable>(loOperand(Dest));
      IceVariable *DestHi = llvm::cast<IceVariable>(hiOperand(Dest));
      IceVariable *Tmp = makeReg(DestLo->getType());
      if (Src0RM->getType() == IceType_i32)
        _mov(Tmp, Src0RM);
      else
        _movzx(Tmp, Src0RM);
      _mov(DestLo, Tmp);
      _mov(DestHi, Zero);
    } else if (Src0RM->getType() == IceType_i1) {
      // t = Src0RM; t &= 1; Dest = t
      IceOperand *One = Ctx->getConstantInt(IceType_i32, 1);
      IceVariable *T = makeReg(IceType_i32);
      _movzx(T, Src0RM);
      _and(T, One);
      _mov(Dest, T);
    } else {
      // t1 = movzx src; dst = t1
      IceVariable *T = makeReg(Dest->getType());
      _movzx(T, Src0RM);
      _mov(Dest, T);
    }
    break;
  case InstCast::Trunc: {
    if (Src0RM->getType() == IceType_i64)
      Src0RM = loOperand(Src0RM);
    // t1 = trunc Src0RM; Dest = t1
    IceVariable *T = NULL;
    _mov(T, Src0RM);
    _mov(Dest, T);
    break;
  }
  case InstCast::Fptrunc:
  case InstCast::Fpext: {
    // t1 = cvt Src0RM; Dest = t1
    IceVariable *T = makeReg(Dest->getType());
    _cvt(T, Src0RM);
    _mov(Dest, T);
    break;
  }
  case InstCast::Fptosi:
    if (Dest->getType() == IceType_i64) {
      // Use a helper for converting floating-point values to 64-bit
      // integers.  SSE2 appears to have no way to convert from xmm
      // registers to something like the edx:eax register pair, and
      // gcc and clang both want to use x87 instructions complete with
      // temporary manipulation of the status word.  This helper is
      // not needed for x86-64.
      split64(Dest);
      uint32_t MaxSrcs = 1;
      IceType SrcType = Inst->getSrc(0)->getType();
      InstCall *Call = makeHelperCall(
          SrcType == IceType_f32 ? "cvtftosi64" : "cvtdtosi64", Dest, MaxSrcs);
      Call->addArg(Inst->getSrc(0));
      lowerCall(Call);
    } else {
      // t1.i32 = cvt Src0RM; t2.dest_type = t1; Dest = t2.dest_type
      IceVariable *T_1 = makeReg(IceType_i32);
      IceVariable *T_2 = makeReg(Dest->getType());
      _cvt(T_1, Src0RM);
      _mov(T_2, T_1); // T_1 and T_2 may have different integer types
      _mov(Dest, T_2);
      T_2->setPreferredRegister(T_1, true);
    }
    break;
  case InstCast::Fptoui:
    if (Dest->getType() == IceType_i64 || Dest->getType() == IceType_i32) {
      // Use a helper for both x86-32 and x86-64.
      split64(Dest);
      uint32_t MaxSrcs = 1;
      IceType DestType = Dest->getType();
      IceType SrcType = Src0RM->getType();
      IceString DstSubstring = (DestType == IceType_i64 ? "64" : "32");
      IceString SrcSubstring = (SrcType == IceType_f32 ? "f" : "d");
      // Possibilities are cvtftoui32, cvtdtoui32, cvtftoui64, cvtdtoui64
      IceString TargetString = "cvt" + SrcSubstring + "toui" + DstSubstring;
      InstCall *Call = makeHelperCall(TargetString, Dest, MaxSrcs);
      Call->addArg(Inst->getSrc(0));
      lowerCall(Call);
      return;
    } else {
      // t1.i32 = cvt Src0RM; t2.dest_type = t1; Dest = t2.dest_type
      IceVariable *T_1 = makeReg(IceType_i32);
      IceVariable *T_2 = makeReg(Dest->getType());
      _cvt(T_1, Src0RM);
      _mov(T_2, T_1); // T_1 and T_2 may have different integer types
      _mov(Dest, T_2);
      T_2->setPreferredRegister(T_1, true);
    }
    break;
  case InstCast::Sitofp:
    if (Src0RM->getType() == IceType_i64) {
      // Use a helper for x86-32.
      uint32_t MaxSrcs = 1;
      IceType DestType = Dest->getType();
      InstCall *Call = makeHelperCall(
          DestType == IceType_f32 ? "cvtsi64tof" : "cvtsi64tod", Dest, MaxSrcs);
      Call->addArg(Inst->getSrc(0));
      lowerCall(Call);
      return;
    } else {
      // Sign-extend the operand.
      // t1.i32 = movsx Src0RM; t2 = Cvt t1.i32; Dest = t2
      IceVariable *T_1 = makeReg(IceType_i32);
      IceVariable *T_2 = makeReg(Dest->getType());
      if (Src0RM->getType() == IceType_i32)
        _mov(T_1, Src0RM);
      else
        _movsx(T_1, Src0RM);
      _cvt(T_2, T_1);
      _mov(Dest, T_2);
    }
    break;
  case InstCast::Uitofp:
    if (Src0RM->getType() == IceType_i64 || Src0RM->getType() == IceType_i32) {
      // Use a helper for x86-32 and x86-64.  Also use a helper for
      // i32 on x86-32.
      uint32_t MaxSrcs = 1;
      IceType DestType = Dest->getType();
      IceString SrcSubstring = (Src0RM->getType() == IceType_i64 ? "64" : "32");
      IceString DstSubstring = (DestType == IceType_f32 ? "f" : "d");
      // Possibilities are cvtui32tof, cvtui32tod, cvtui64tof, cvtui64tod
      IceString TargetString = "cvtui" + SrcSubstring + "to" + DstSubstring;
      InstCall *Call = makeHelperCall(TargetString, Dest, MaxSrcs);
      Call->addArg(Inst->getSrc(0));
      lowerCall(Call);
      return;
    } else {
      // Zero-extend the operand.
      // t1.i32 = movzx Src0RM; t2 = Cvt t1.i32; Dest = t2
      IceVariable *T_1 = makeReg(IceType_i32);
      IceVariable *T_2 = makeReg(Dest->getType());
      if (Src0RM->getType() == IceType_i32)
        _mov(T_1, Src0RM);
      else
        _movzx(T_1, Src0RM);
      _cvt(T_2, T_1);
      _mov(Dest, T_2);
    }
    break;
  case InstCast::Bitcast:
    if (Dest->getType() == Src0RM->getType()) {
      InstAssign *Assign = InstAssign::create(Cfg, Dest, Src0RM);
      lowerAssign(Assign);
      assert(0 && "Pointer bitcasts aren't lowered correctly.");
      return;
    }
    switch (Dest->getType()) {
    default:
      assert(0 && "Unexpected Bitcast dest type");
    case IceType_i32:
    case IceType_f32: {
      IceType DestType = Dest->getType();
      IceType SrcType = Src0RM->getType();
      assert((DestType == IceType_i32 && SrcType == IceType_f32) ||
             (DestType == IceType_f32 && SrcType == IceType_i32));
      // a.i32 = bitcast b.f32 ==>
      //   t.f32 = b.f32
      //   s.f32 = spill t.f32
      //   a.i32 = s.f32
      IceVariable *T = NULL;
      // TODO: Should be able to force a spill setup by calling legalize() with
      // Legal_Mem and not Legal_Reg or Legal_Imm.
      IceVariable *Spill = Cfg->makeVariable(SrcType, Context.getNode());
      Spill->setWeight(IceRegWeight::Zero);
      Spill->setPreferredRegister(Dest, true);
      _mov(T, Src0RM);
      _mov(Spill, T);
      _mov(Dest, Spill);
    } break;
    case IceType_i64: {
      assert(Src0RM->getType() == IceType_f64);
      // a.i64 = bitcast b.f64 ==>
      //   s.f64 = spill b.f64
      //   t_lo.i32 = lo(s.f64)
      //   a_lo.i32 = t_lo.i32
      //   t_hi.i32 = hi(s.f64)
      //   a_hi.i32 = t_hi.i32
      IceVariable *Spill = Cfg->makeVariable(IceType_f64, Context.getNode());
      Spill->setWeight(IceRegWeight::Zero);
      Spill->setPreferredRegister(llvm::dyn_cast<IceVariable>(Src0RM), true);
      _mov(Spill, Src0RM);

      IceVariable *DestLo = llvm::cast<IceVariable>(loOperand(Dest));
      IceVariable *DestHi = llvm::cast<IceVariable>(hiOperand(Dest));
      IceVariable *T_Lo = makeReg(IceType_i32);
      IceVariable *T_Hi = makeReg(IceType_i32);
      IceVariableSplit *SpillLo =
          IceVariableSplit::create(Cfg, Spill, IceVariableSplit::Low);
      IceVariableSplit *SpillHi =
          IceVariableSplit::create(Cfg, Spill, IceVariableSplit::High);

      _mov(T_Lo, SpillLo);
      _mov(DestLo, T_Lo);
      _mov(T_Hi, SpillHi);
      _mov(DestHi, T_Hi);
    } break;
    case IceType_f64: {
      assert(Src0RM->getType() == IceType_i64);
      // a.f64 = bitcast b.i64 ==>
      //   t_lo.i32 = b_lo.i32
      //   lo(s.f64) = t_lo.i32
      //   FakeUse(s.f64)
      //   t_hi.i32 = b_hi.i32
      //   hi(s.f64) = t_hi.i32
      //   a.f64 = s.f64
      IceVariable *Spill = Cfg->makeVariable(IceType_f64, Context.getNode());
      Spill->setWeight(IceRegWeight::Zero);
      Spill->setPreferredRegister(Dest, true);

      Context.insert(InstFakeDef::create(Cfg, Spill));

      IceVariable *T_Lo = NULL, *T_Hi = NULL;
      IceVariableSplit *SpillLo =
          IceVariableSplit::create(Cfg, Spill, IceVariableSplit::Low);
      IceVariableSplit *SpillHi =
          IceVariableSplit::create(Cfg, Spill, IceVariableSplit::High);
      _mov(T_Lo, loOperand(Src0RM));
      _store(T_Lo, SpillLo);
      _mov(T_Hi, hiOperand(Src0RM));
      _store(T_Hi, SpillHi);
      _mov(Dest, Spill);
    } break;
    }
    break;
  }
}

namespace {

// The following table summarizes the logic for lowering the fcmp instruction.
// There is one table entry for each of the 16 conditions.  A comment in
// lowerFcmp() describes the lowering template.  In the most general case, there
// is a compare followed by two conditional branches, because some fcmp
// conditions don't map to a single x86 conditional branch.  However, in many
// cases it is possible to swap the operands in the comparison and have a single
// conditional branch.  Since it's quite tedious to validate the table by hand,
// good execution tests are helpful.
const struct {
  InstFcmp::FCond Cond;
  uint32_t Default;
  bool SwapOperands;
  InstX8632Br::BrCond C1, C2;
} TableFcmp[] = {
#define X(A, B, C, D, E)                                                       \
  { InstFcmp::A, B, C, InstX8632Br::D, InstX8632Br::E }
    X(False, 0, false, Br_None, Br_None), X(Oeq, 0, false, Br_ne, Br_p),
    X(Ogt, 1, false, Br_a, Br_None),      X(Oge, 1, false, Br_ae, Br_None),
    X(Olt, 1, true, Br_a, Br_None),       X(Ole, 1, true, Br_ae, Br_None),
    X(One, 1, false, Br_ne, Br_None),     X(Ord, 1, false, Br_np, Br_None),
    X(Ueq, 1, false, Br_e, Br_None),      X(Ugt, 1, true, Br_b, Br_None),
    X(Uge, 1, true, Br_be, Br_None),      X(Ult, 1, false, Br_b, Br_None),
    X(Ule, 1, false, Br_be, Br_None),     X(Une, 1, false, Br_ne, Br_p),
    X(Uno, 1, false, Br_p, Br_None),      X(True, 1, false, Br_None, Br_None)
  };
#undef X
const uint32_t TableFcmpSize = sizeof(TableFcmp) / sizeof(*TableFcmp);

} // anonymous namespace

void IceTargetX8632::lowerFcmp(const InstFcmp *Inst) {
  IceOperand *Src0 = Inst->getSrc(0);
  IceOperand *Src1 = Inst->getSrc(1);
  IceVariable *Dest = Inst->getDest();
  // Lowering a = fcmp cond, b, c
  //   ucomiss b, c       /* only if C1 != Br_None */
  //                      /* but swap b,c order if SwapOperands==true */
  //   mov a, <default>
  //   j<C1> label        /* only if C1 != Br_None */
  //   j<C2> label        /* only if C2 != Br_None */
  //   FakeUse(a)         /* only if C1 != Br_None */
  //   mov a, !<default>  /* only if C1 != Br_None */
  //   label:             /* only if C1 != Br_None */
  InstFcmp::FCond Condition = Inst->getCondition();
  uint32_t Index = static_cast<uint32_t>(Condition);
  assert(Index < TableFcmpSize);
  // The table is indexed by InstFcmp::Condition.  Make sure it didn't fall
  // out of order.
  assert(TableFcmp[Index].Cond == Condition);
  if (TableFcmp[Index].SwapOperands) {
    IceOperand *Tmp = Src0;
    Src0 = Src1;
    Src1 = Tmp;
  }
  bool HasC1 = (TableFcmp[Index].C1 != InstX8632Br::Br_None);
  bool HasC2 = (TableFcmp[Index].C2 != InstX8632Br::Br_None);
  if (HasC1) {
    Src0 = legalize(Src0);
    IceOperand *Src1RM = legalize(Src1, Legal_Reg | Legal_Mem);
    IceVariable *T = NULL;
    _mov(T, Src0);
    _ucomiss(T, Src1RM);
  }
  IceConstant *Default =
      Ctx->getConstantInt(IceType_i32, TableFcmp[Index].Default);
  _mov(Dest, Default);
  if (HasC1) {
    InstX8632Label *Label = InstX8632Label::create(Cfg, this);
    _br(TableFcmp[Index].C1, Label);
    if (HasC2) {
      _br(TableFcmp[Index].C2, Label);
    }
    Context.insert(InstFakeUse::create(Cfg, Dest));
    IceConstant *NonDefault =
        Ctx->getConstantInt(IceType_i32, !TableFcmp[Index].Default);
    _mov(Dest, NonDefault);
    Context.insert(Label);
  }
}

namespace {

// The following table summarizes the logic for lowering the icmp instruction
// for i32 and narrower types.  Each icmp condition has a clear mapping to an
// x86 conditional branch instruction.
const struct {
  InstIcmp::ICond Cond;
  InstX8632Br::BrCond Mapping;
} TableIcmp32[] = {
#define X(A, B)                                                                \
  { InstIcmp::A, InstX8632Br::B }
    X(Eq, Br_e),   X(Ne, Br_ne), X(Ugt, Br_a),  X(Uge, Br_ae), X(Ult, Br_b),
    X(Ule, Br_be), X(Sgt, Br_g), X(Sge, Br_ge), X(Slt, Br_l),  X(Sle, Br_le),
  };
#undef X
const uint32_t TableIcmp32Size = sizeof(TableIcmp32) / sizeof(*TableIcmp32);

// The following table summarizes the logic for lowering the icmp instruction
// for the i64 type.  For Eq and Ne, two separate 32-bit comparisons and
// conditional branches are needed.  For the other conditions, three separate
// conditional branches are needed.
const struct {
  InstIcmp::ICond Cond;
  InstX8632Br::BrCond C1, C2, C3;
} TableIcmp64[] = {
#define X(A, B, C, D)                                                          \
  { InstIcmp::A, InstX8632Br::B, InstX8632Br::C, InstX8632Br::D }
    // Eq and Ne are placeholders to ensure TableIcmp64[i].Cond==i
    { InstIcmp::Eq },
    { InstIcmp::Ne },
    X(Ugt, Br_a, Br_b, Br_a),
    X(Uge, Br_a, Br_b, Br_ae),
    X(Ult, Br_b, Br_a, Br_b),
    X(Ule, Br_b, Br_a, Br_be),
    X(Sgt, Br_g, Br_l, Br_a),
    X(Sge, Br_g, Br_l, Br_ae),
    X(Slt, Br_l, Br_g, Br_b),
    X(Sle, Br_l, Br_g, Br_be),
  };
#undef X
const uint32_t TableIcmp64Size = sizeof(TableIcmp64) / sizeof(*TableIcmp64);

InstX8632Br::BrCond getIcmp32Mapping(InstIcmp::ICond Cond) {
  uint32_t Index = static_cast<uint32_t>(Cond);
  assert(Index < TableIcmp32Size);
  assert(TableIcmp32[Index].Cond == Cond);
  return TableIcmp32[Index].Mapping;
}

} // anonymous namespace

void IceTargetX8632::lowerIcmp(const InstIcmp *Inst) {
  IceOperand *Src0 = legalize(Inst->getSrc(0));
  IceOperand *Src1 = legalize(Inst->getSrc(1));
  IceVariable *Dest = Inst->getDest();

  // If Src1 is an immediate, or known to be a physical register, we can
  // allow Src0 to be a memory operand.  Otherwise, Src0 must be copied into
  // a physical register.  (Actually, either Src0 or Src1 can be chosen for
  // the physical register, but unfortunately we have to commit to one or
  // the other before register allocation.)
  bool IsSrc1ImmOrReg = false;
  if (llvm::isa<IceConstant>(Src1))
    IsSrc1ImmOrReg = true;
  else if (IceVariable *Var = llvm::dyn_cast<IceVariable>(Src1)) {
    if (Var->hasReg())
      IsSrc1ImmOrReg = true;
  }

  // Try to fuse a compare immediately followed by a conditional branch.  This
  // is possible when the compare dest and the branch source operands are the
  // same, and are their only uses.  TODO: implement this optimization for i64.
  if (InstBr *NextBr = llvm::dyn_cast_or_null<InstBr>(Context.getNextInst())) {
    if (Src0->getType() != IceType_i64 && !NextBr->isUnconditional() &&
        Dest == NextBr->getSrc(0) && NextBr->isLastUse(Dest)) {
      IceOperand *Src0New =
          legalize(Src0, IsSrc1ImmOrReg ? Legal_All : Legal_Reg, true);
      _cmp(Src0New, Src1);
      _br(getIcmp32Mapping(Inst->getCondition()), NextBr->getTargetTrue(),
          NextBr->getTargetFalse());
      // Skip over the following branch instruction.
      NextBr->setDeleted();
      Context.advanceNext();
      return;
    }
  }

  // a=icmp cond, b, c ==> cmp b,c; a=1; br cond,L1; FakeUse(a); a=0; L1:
  IceConstant *Zero = Ctx->getConstantInt(IceType_i32, 0);
  IceConstant *One = Ctx->getConstantInt(IceType_i32, 1);
  if (Src0->getType() == IceType_i64) {
    InstIcmp::ICond Condition = Inst->getCondition();
    uint32_t Index = static_cast<uint32_t>(Condition);
    assert(Index < TableIcmp64Size);
    // The table is indexed by InstIcmp::Condition.  Make sure it didn't fall
    // out of order.
    assert(TableIcmp64[Index].Cond == Condition);
    IceOperand *Src1LoRI = legalize(loOperand(Src1), Legal_Reg | Legal_Imm);
    IceOperand *Src1HiRI = legalize(hiOperand(Src1), Legal_Reg | Legal_Imm);
    if (Condition == InstIcmp::Eq || Condition == InstIcmp::Ne) {
      InstX8632Label *Label = InstX8632Label::create(Cfg, this);
      _mov(Dest, (Condition == InstIcmp::Eq ? Zero : One));
      _cmp(loOperand(Src0), Src1LoRI);
      _br(InstX8632Br::Br_ne, Label);
      _cmp(hiOperand(Src0), Src1HiRI);
      _br(InstX8632Br::Br_ne, Label);
      Context.insert(InstFakeUse::create(Cfg, Dest));
      _mov(Dest, (Condition == InstIcmp::Eq ? One : Zero));
      Context.insert(Label);
    } else {
      InstX8632Label *LabelFalse = InstX8632Label::create(Cfg, this);
      InstX8632Label *LabelTrue = InstX8632Label::create(Cfg, this);
      _mov(Dest, One);
      _cmp(hiOperand(Src0), Src1HiRI);
      _br(TableIcmp64[Index].C1, LabelTrue);
      _br(TableIcmp64[Index].C2, LabelFalse);
      _cmp(loOperand(Src0), Src1LoRI);
      _br(TableIcmp64[Index].C3, LabelTrue);
      Context.insert(LabelFalse);
      Context.insert(InstFakeUse::create(Cfg, Dest));
      _mov(Dest, Zero);
      Context.insert(LabelTrue);
    }
    return;
  }
  // cmp b, c
  IceOperand *Src0New =
      legalize(Src0, IsSrc1ImmOrReg ? Legal_All : Legal_Reg, true);
  InstX8632Label *Label = InstX8632Label::create(Cfg, this);
  _cmp(Src0New, Src1);
  _mov(Dest, One);
  _br(getIcmp32Mapping(Inst->getCondition()), Label);
  Context.insert(InstFakeUse::create(Cfg, Dest));
  _mov(Dest, Zero);
  Context.insert(Label);
}

namespace {

bool isAdd(const Inst *Inst) {
  if (const InstArithmetic *Arith =
          llvm::dyn_cast_or_null<const InstArithmetic>(Inst)) {
    return (Arith->getOp() == InstArithmetic::Add);
  }
  return false;
}

void computeAddressOpt(IceCfg *Cfg, IceVariable *&Base, IceVariable *&Index,
                       int32_t &Shift, int32_t &Offset) {
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
    const Inst *BaseInst = Base->getDefinition();
    IceOperand *BaseOperand0 = BaseInst ? BaseInst->getSrc(0) : NULL;
    IceVariable *BaseVariable0 =
        llvm::dyn_cast_or_null<IceVariable>(BaseOperand0);
    if (BaseInst && llvm::isa<InstAssign>(BaseInst) && BaseVariable0 &&
        // TODO: ensure BaseVariable0 stays single-BB
        true) {
      Base = BaseVariable0;

      continue;
    }

    // Index is Index=Var ==>
    //   set Index=Var

    // Index==NULL && Base is Base=Var1+Var2 ==>
    //   set Base=Var1, Index=Var2, Shift=0
    IceOperand *BaseOperand1 =
        BaseInst && BaseInst->getSrcSize() >= 2 ? BaseInst->getSrc(1) : NULL;
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
    const Inst *IndexInst = Index ? Index->getDefinition() : NULL;
    if (const InstArithmetic *ArithInst =
            llvm::dyn_cast_or_null<InstArithmetic>(IndexInst)) {
      IceOperand *IndexOperand0 = ArithInst->getSrc(0);
      IceVariable *IndexVariable0 = llvm::dyn_cast<IceVariable>(IndexOperand0);
      IceOperand *IndexOperand1 = ArithInst->getSrc(1);
      IceConstantInteger *IndexConstant1 =
          llvm::dyn_cast<IceConstantInteger>(IndexOperand1);
      if (ArithInst->getOp() == InstArithmetic::Mul && IndexVariable0 &&
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

} // anonymous namespace

void IceTargetX8632::lowerLoad(const InstLoad *Inst) {
  // A Load instruction can be treated the same as an Assign
  // instruction, after the source operand is transformed into an
  // IceOperandX8632Mem operand.  Note that the address mode
  // optimization already creates an IceOperandX8632Mem operand, so it
  // doesn't need another level of transformation.
  IceType Type = Inst->getDest()->getType();
  IceOperand *Src0 = Inst->getSrc(0);
  // Address mode optimization already creates an IceOperandX8632Mem
  // operand, so it doesn't need another level of transformation.
  if (!llvm::isa<IceOperandX8632Mem>(Src0)) {
    IceVariable *Base = llvm::dyn_cast<IceVariable>(Src0);
    IceConstant *Offset = llvm::dyn_cast<IceConstant>(Src0);
    assert(Base || Offset);
    Src0 = IceOperandX8632Mem::create(Cfg, Type, Base, Offset);
  }

  // Fuse this load with a subsequent Arithmetic instruction in the
  // following situations:
  //   a=[mem]; c=b+a ==> c=b+[mem] if last use of a and a not in b
  //   a=[mem]; c=a+b ==> c=b+[mem] if commutative and above is true
  //
  // TODO: Clean up and test thoroughly.
  //
  // TODO: Why limit to Arithmetic instructions?  This could probably be
  // applied to most any instruction type.  Look at all source operands
  // in the following instruction, and if there is one instance of the
  // load instruction's dest variable, and that instruction ends that
  // variable's live range, then make the substitution.  Deal with
  // commutativity optimization in the arithmetic instruction lowering.
  InstArithmetic *NewArith = NULL;
  if (InstArithmetic *Arith =
          llvm::dyn_cast_or_null<InstArithmetic>(Context.getNextInst())) {
    IceVariable *DestLoad = Inst->getDest();
    IceVariable *Src0Arith = llvm::dyn_cast<IceVariable>(Arith->getSrc(0));
    IceVariable *Src1Arith = llvm::dyn_cast<IceVariable>(Arith->getSrc(1));
    if (Src1Arith == DestLoad && Arith->isLastUse(Src1Arith) &&
        DestLoad != Src0Arith) {
      NewArith = InstArithmetic::create(Cfg, Arith->getOp(), Arith->getDest(),
                                        Arith->getSrc(0), Src0);
    } else if (Src0Arith == DestLoad && Arith->isCommutative() &&
               Arith->isLastUse(Src0Arith) && DestLoad != Src1Arith) {
      NewArith = InstArithmetic::create(Cfg, Arith->getOp(), Arith->getDest(),
                                        Arith->getSrc(1), Src0);
    }
    if (NewArith) {
      Arith->setDeleted();
      Context.advanceNext();
      lowerArithmetic(NewArith);
      return;
    }
  }

  InstAssign *Assign = InstAssign::create(Cfg, Inst->getDest(), Src0);
  lowerAssign(Assign);
}

void IceTargetX8632::doAddressOptLoad() {
  Inst *Inst = *Context.getCur();
  IceVariable *Dest = Inst->getDest();
  IceOperand *Addr = Inst->getSrc(0);
  IceVariable *Index = NULL;
  int32_t Shift = 0;
  int32_t Offset = 0; // TODO: make IceConstant
  IceVariable *Base = llvm::dyn_cast<IceVariable>(Addr);
  computeAddressOpt(Cfg, Base, Index, Shift, Offset);
  if (Base && Addr != Base) {
    IceConstant *OffsetOp = Ctx->getConstantInt(IceType_i32, Offset);
    Addr = IceOperandX8632Mem::create(Cfg, Dest->getType(), Base, OffsetOp,
                                      Index, Shift);
    Inst->setDeleted();
    Context.insert(InstLoad::create(Cfg, Dest, Addr));
  }
}

void IceTargetX8632::lowerPhi(const InstPhi *Inst) {
  Cfg->setError("Phi lowering not implemented");
}

void IceTargetX8632::lowerRet(const InstRet *Inst) {
  IceVariable *Reg = NULL;
  if (Inst->getSrcSize()) {
    IceOperand *Src0 = legalize(Inst->getSrc(0));
    if (Src0->getType() == IceType_i64) {
      IceVariable *eax = legalizeToVar(loOperand(Src0), false, Reg_eax);
      IceVariable *edx = legalizeToVar(hiOperand(Src0), false, Reg_edx);
      Reg = eax;
      Context.insert(InstFakeUse::create(Cfg, edx));
    } else if (Src0->getType() == IceType_f32 ||
               Src0->getType() == IceType_f64) {
      _fld(Src0);
    } else {
      _mov(Reg, Src0, Reg_eax);
    }
  }
  _ret(Reg);
  // Add a fake use of esp to make sure esp stays alive for the entire
  // function.  Otherwise post-call esp adjustments get dead-code
  // eliminated.  TODO: Are there more places where the fake use
  // should be inserted?  E.g. "void f(int n){while(1) g(n);}" may not
  // have a ret instruction.
  IceVariable *esp = Cfg->getTarget()->getPhysicalRegister(Reg_esp);
  Context.insert(InstFakeUse::create(Cfg, esp));
}

void IceTargetX8632::lowerSelect(const InstSelect *Inst) {
  // a=d?b:c ==> cmp d,0; a=b; jne L1; FakeUse(a); a=c; L1:
  IceVariable *Dest = Inst->getDest();
  IceOperand *SrcT = Inst->getTrueOperand();
  IceOperand *SrcF = Inst->getFalseOperand();
  IceOperand *Condition = legalize(Inst->getCondition());
  IceConstant *Zero = Ctx->getConstantInt(IceType_i32, 0);
  InstX8632Label *Label = InstX8632Label::create(Cfg, this);

  if (Dest->getType() == IceType_i64) {
    IceVariable *DestLo = llvm::cast<IceVariable>(loOperand(Dest));
    IceVariable *DestHi = llvm::cast<IceVariable>(hiOperand(Dest));
    IceOperand *SrcLoRI =
        legalize(loOperand(SrcT), Legal_Reg | Legal_Imm, true);
    IceOperand *SrcHiRI =
        legalize(hiOperand(SrcT), Legal_Reg | Legal_Imm, true);
    _cmp(Condition, Zero);
    _mov(DestLo, SrcLoRI);
    _mov(DestHi, SrcHiRI);
    _br(InstX8632Br::Br_ne, Label);
    Context.insert(InstFakeUse::create(Cfg, DestLo));
    Context.insert(InstFakeUse::create(Cfg, DestHi));
    IceOperand *SrcFLo = loOperand(SrcF);
    IceOperand *SrcFHi = hiOperand(SrcF);
    SrcLoRI = legalize(SrcFLo, Legal_Reg | Legal_Imm, true);
    SrcHiRI = legalize(SrcFHi, Legal_Reg | Legal_Imm, true);
    _mov(DestLo, SrcLoRI);
    _mov(DestHi, SrcHiRI);
  } else {
    _cmp(Condition, Zero);
    SrcT = legalize(SrcT, Legal_Reg | Legal_Imm, true);
    _mov(Dest, SrcT);
    _br(InstX8632Br::Br_ne, Label);
    Context.insert(InstFakeUse::create(Cfg, Dest));
    SrcF = legalize(SrcF, Legal_Reg | Legal_Imm, true);
    _mov(Dest, SrcF);
  }

  Context.insert(Label);
}

void IceTargetX8632::lowerStore(const InstStore *Inst) {
  IceOperand *Value = Inst->getData();
  IceOperand *Addr = Inst->getAddr();
  IceOperandX8632Mem *NewAddr = llvm::dyn_cast<IceOperandX8632Mem>(Addr);
  // Address mode optimization already creates an IceOperandX8632Mem
  // operand, so it doesn't need another level of transformation.
  if (!NewAddr) {
    // The address will be either a constant (which represents a global
    // variable) or a variable, so either the Base or Offset component
    // of the IceOperandX8632Mem will be set.
    IceVariable *Base = llvm::dyn_cast<IceVariable>(Addr);
    IceConstant *Offset = llvm::dyn_cast<IceConstant>(Addr);
    assert(Base || Offset);
    NewAddr = IceOperandX8632Mem::create(Cfg, Value->getType(), Base, Offset);
  }
  NewAddr = llvm::cast<IceOperandX8632Mem>(legalize(NewAddr));

  if (NewAddr->getType() == IceType_i64) {
    Value = legalize(Value);
    IceOperand *ValueHi =
        legalize(hiOperand(Value), Legal_Reg | Legal_Imm, true);
    IceOperand *ValueLo =
        legalize(loOperand(Value), Legal_Reg | Legal_Imm, true);
    _store(ValueHi, llvm::cast<IceOperandX8632Mem>(hiOperand(NewAddr)));
    _store(ValueLo, llvm::cast<IceOperandX8632Mem>(loOperand(NewAddr)));
  } else {
    Value = legalize(Value, Legal_Reg | Legal_Imm, true);
    _store(Value, NewAddr);
  }
}

void IceTargetX8632::doAddressOptStore() {
  InstStore *Inst = llvm::cast<InstStore>(*Context.getCur());
  IceOperand *Data = Inst->getData();
  IceOperand *Addr = Inst->getAddr();
  IceVariable *Index = NULL;
  int32_t Shift = 0;
  int32_t Offset = 0; // TODO: make IceConstant
  IceVariable *Base = llvm::dyn_cast<IceVariable>(Addr);
  computeAddressOpt(Cfg, Base, Index, Shift, Offset);
  if (Base && Addr != Base) {
    IceConstant *OffsetOp = Ctx->getConstantInt(IceType_i32, Offset);
    Addr = IceOperandX8632Mem::create(Cfg, Data->getType(), Base, OffsetOp,
                                      Index, Shift);
    Inst->setDeleted();
    Context.insert(InstStore::create(Cfg, Data, Addr));
  }
}

void IceTargetX8632::lowerSwitch(const InstSwitch *Inst) {
  // This implements the most naive possible lowering.
  // cmp a,val[0]; jeq label[0]; cmp a,val[1]; jeq label[1]; ... jmp default
  IceOperand *Src0 = Inst->getSrc(0);
  uint32_t NumCases = Inst->getNumCases();
  // OK, we'll be slightly less naive by forcing Src into a physical
  // register if there are 2 or more uses.
  if (NumCases >= 2)
    Src0 = legalizeToVar(Src0, true);
  else
    Src0 = legalize(Src0, Legal_All, true);
  for (uint32_t I = 0; I < NumCases; ++I) {
    IceOperand *Value = Ctx->getConstantInt(IceType_i32, Inst->getValue(I));
    _cmp(Src0, Value);
    _br(InstX8632Br::Br_e, Inst->getLabel(I));
  }

  _br(Inst->getLabelDefault());
}

void IceTargetX8632::lowerUnreachable(const InstUnreachable *Inst) {
  uint32_t MaxSrcs = 0;
  IceVariable *Dest = NULL;
  InstCall *Call = makeHelperCall("ice_unreachable", Dest, MaxSrcs);
  lowerCall(Call);
}

IceOperand *IceTargetX8632::legalize(IceOperand *From, LegalMask Allowed,
                                     bool AllowOverlap, int32_t RegNum) {
  assert(Allowed & Legal_Reg);
  assert(RegNum == IceVariable::NoRegister || Allowed == Legal_Reg);
  if (IceOperandX8632Mem *Mem = llvm::dyn_cast<IceOperandX8632Mem>(From)) {
    IceVariable *Base = Mem->getBase();
    IceVariable *Index = Mem->getIndex();
    IceVariable *RegBase = Base;
    IceVariable *RegIndex = Index;
    if (Base) {
      RegBase = legalizeToVar(Base, true);
    }
    if (Index) {
      RegIndex = legalizeToVar(Index, true);
    }
    if (Base != RegBase || Index != RegIndex) {
      From = IceOperandX8632Mem::create(Cfg, Mem->getType(), RegBase,
                                        Mem->getOffset(), RegIndex,
                                        Mem->getShift());
    }

    if (!(Allowed & Legal_Mem)) {
      IceVariable *Reg = makeReg(From->getType(), RegNum);
      _mov(Reg, From, RegNum);
      From = Reg;
    }
    return From;
  }
  if (llvm::isa<IceConstant>(From)) {
    if (!(Allowed & Legal_Imm)) {
      IceVariable *Reg = makeReg(From->getType(), RegNum);
      _mov(Reg, From);
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
      IceVariable *Reg = makeReg(From->getType(), RegNum);
      if (RegNum == IceVariable::NoRegister) {
        Reg->setPreferredRegister(Var, AllowOverlap);
      }
      _mov(Reg, From);
      From = Reg;
    }
    return From;
  }
  assert(0);
  return From;
}

IceVariable *IceTargetX8632::legalizeToVar(IceOperand *From, bool AllowOverlap,
                                           int32_t RegNum) {
  return llvm::cast<IceVariable>(
      legalize(From, Legal_Reg, AllowOverlap, RegNum));
}

IceVariable *IceTargetX8632::makeReg(IceType Type, int32_t RegNum) {
  IceVariable *Reg = Cfg->makeVariable(Type, Context.getNode());
  if (RegNum == IceVariable::NoRegister)
    Reg->setWeightInfinite();
  else
    Reg->setRegNum(RegNum);
  return Reg;
}

void IceTargetX8632::postLower() {
  if (Ctx->getOptLevel() != IceOpt_m1)
    return;
  llvm::SmallBitVector WhiteList = getRegisterSet(RegSet_All, RegSet_None);
  // Make one pass to black-list pre-colored registers.  TODO: If
  // there was some prior register allocation pass that made register
  // assignments, those registers need to be black-listed here as
  // well.
  for (InstList::iterator I = Context.getCur(), E = Context.getEnd(); I != E;
       ++I) {
    const Inst *Inst = *I;
    if (Inst->isDeleted())
      continue;
    if (llvm::isa<InstFakeKill>(Inst))
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
  for (InstList::iterator I = Context.getCur(), E = Context.getEnd(); I != E;
       ++I) {
    const Inst *Inst = *I;
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

} // end of namespace Ice
