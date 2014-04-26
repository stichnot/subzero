//===- subzero/src/TargetLoweringX8632.cpp - x86-32 lowering -----------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the TargetLoweringX8632 class, which
// consists almost entirely of the lowering sequence for each
// high-level instruction.  It also implements
// TargetX8632Fast::postLower() which does the simplest possible
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

TargetX8632::TargetX8632(Cfg *Func)
    : TargetLowering(Func), IsEbpBasedFrame(false), FrameSizeLocals(0),
      LocalsSizeBytes(0), NextLabelNumber(0), ComputedLiveRanges(false),
      PhysicalRegisters(VarList(Reg_NUM)) {
  llvm::SmallBitVector IntegerRegisters(Reg_NUM);
  llvm::SmallBitVector IntegerRegistersI8(Reg_NUM);
  llvm::SmallBitVector FloatRegisters(Reg_NUM);
  llvm::SmallBitVector InvalidRegisters(Reg_NUM);
  ScratchRegs.resize(Reg_NUM);
#define X(val, init, name, name16, name8, scratch, preserved, stackptr,        \
          frameptr, isI8, isInt, isFP)                                         \
  IntegerRegisters[val] = isInt;                                               \
  IntegerRegistersI8[val] = isI8;                                              \
  FloatRegisters[val] = isFP;                                                  \
  ScratchRegs[val] = scratch;
  REGX8632_TABLE;
#undef X
  TypeToRegisterSet[IceType_void] = InvalidRegisters;
  TypeToRegisterSet[IceType_i1] = IntegerRegistersI8;
  TypeToRegisterSet[IceType_i8] = IntegerRegistersI8;
  TypeToRegisterSet[IceType_i16] = IntegerRegisters;
  TypeToRegisterSet[IceType_i32] = IntegerRegisters;
  TypeToRegisterSet[IceType_i64] = IntegerRegisters;
  TypeToRegisterSet[IceType_f32] = FloatRegisters;
  TypeToRegisterSet[IceType_f64] = FloatRegisters;
}

void TargetX8632::translateO2() {
  GlobalContext *Context = Func->getContext();
  Ostream &Str = Context->getStrDump();
  Timer T_placePhiLoads;
  Func->placePhiLoads();
  if (Func->hasError())
    return;
  T_placePhiLoads.printElapsedUs(Context, "placePhiLoads()");
  Timer T_placePhiStores;
  Func->placePhiStores();
  if (Func->hasError())
    return;
  T_placePhiStores.printElapsedUs(Context, "placePhiStores()");
  Timer T_deletePhis;
  Func->deletePhis();
  if (Func->hasError())
    return;
  T_deletePhis.printElapsedUs(Context, "deletePhis()");
  Timer T_renumber1;
  Func->renumberInstructions();
  if (Func->hasError())
    return;
  T_renumber1.printElapsedUs(Context, "renumberInstructions()");
  if (Context->isVerbose())
    Str << "================ After Phi lowering ================\n";
  Func->dump();

  Timer T_doAddressOpt;
  Func->doAddressOpt();
  T_doAddressOpt.printElapsedUs(Context, "doAddressOpt()");
  // Liveness may be incorrect after address mode optimization.
  Timer T_renumber2;
  Func->renumberInstructions();
  if (Func->hasError())
    return;
  T_renumber2.printElapsedUs(Context, "renumberInstructions()");
  // TODO: It should be sufficient to use the fastest liveness
  // calculation, i.e. Liveness_LREndLightweight.  However, for
  // some reason that slows down the rest of the translation.
  // Investigate.
  Timer T_liveness1;
  Func->liveness(Liveness_LREndFull);
  if (Func->hasError())
    return;
  T_liveness1.printElapsedUs(Context, "liveness()");
  if (Context->isVerbose())
    Str << "================ After x86 address mode opt ================\n";
  Func->dump();
  Timer T_genCode;
  Func->genCode();
  if (Func->hasError())
    return;
  T_genCode.printElapsedUs(Context, "genCode()");
  Timer T_renumber3;
  Func->renumberInstructions();
  if (Func->hasError())
    return;
  T_renumber3.printElapsedUs(Context, "renumberInstructions()");
  Timer T_liveness2;
  Func->liveness(Liveness_RangesFull);
  if (Func->hasError())
    return;
  T_liveness2.printElapsedUs(Context, "liveness()");
  ComputedLiveRanges = true;
  if (Context->isVerbose())
    Str << "================ After initial x8632 codegen ================\n";
  Func->dump();

  Timer T_regAlloc;
  regAlloc();
  if (Func->hasError())
    return;
  T_regAlloc.printElapsedUs(Context, "regAlloc()");
  if (Context->isVerbose())
    Str << "================ After linear scan regalloc ================\n";
  Func->dump();

  Timer T_genFrame;
  Func->genFrame();
  if (Func->hasError())
    return;
  T_genFrame.printElapsedUs(Context, "genFrame()");
  if (Context->isVerbose())
    Str << "================ After stack frame mapping ================\n";
  Func->dump();
}

void TargetX8632::translateOm1() {
  GlobalContext *Context = Func->getContext();
  Ostream &Str = Context->getStrDump();
  Timer T_placePhiLoads;
  Func->placePhiLoads();
  if (Func->hasError())
    return;
  T_placePhiLoads.printElapsedUs(Context, "placePhiLoads()");
  Timer T_placePhiStores;
  Func->placePhiStores();
  if (Func->hasError())
    return;
  T_placePhiStores.printElapsedUs(Context, "placePhiStores()");
  Timer T_deletePhis;
  Func->deletePhis();
  if (Func->hasError())
    return;
  T_deletePhis.printElapsedUs(Context, "deletePhis()");
  if (Context->isVerbose())
    Str << "================ After Phi lowering ================\n";
  Func->dump();

  Timer T_genCode;
  Func->genCode();
  if (Func->hasError())
    return;
  T_genCode.printElapsedUs(Context, "genCode()");
  if (Context->isVerbose())
    Str << "================ After initial x8632 codegen ================\n";
  Func->dump();

  Timer T_genFrame;
  Func->genFrame();
  if (Func->hasError())
    return;
  T_genFrame.printElapsedUs(Context, "genFrame()");
  if (Context->isVerbose())
    Str << "================ After stack frame mapping ================\n";
  Func->dump();
}

#define X(val, init, name, name16, name8, scratch, preserved, stackptr,        \
          frameptr, isI8, isInt, isFP)                                         \
  name,

IceString TargetX8632::RegNames[] = { REGX8632_TABLE };
#undef X

Variable *TargetX8632::getPhysicalRegister(SizeT RegNum) {
  assert(RegNum < PhysicalRegisters.size());
  Variable *Reg = PhysicalRegisters[RegNum];
  if (Reg == NULL) {
    CfgNode *Node = NULL; // NULL means multi-block lifetime
    Reg = Func->makeVariable(IceType_i32, Node);
    Reg->setRegNum(RegNum);
    PhysicalRegisters[RegNum] = Reg;
  }
  return Reg;
}

IceString TargetX8632::getRegName(SizeT RegNum, Type Ty) const {
  assert(RegNum < Reg_NUM);
  static IceString RegNames8[] = {
#define X(val, init, name, name16, name8, scratch, preserved, stackptr,        \
          frameptr, isI8, isInt, isFP)                                         \
  "" name8,
    REGX8632_TABLE
#undef X
  };
  static IceString RegNames16[] = {
#define X(val, init, name, name16, name8, scratch, preserved, stackptr,        \
          frameptr, isI8, isInt, isFP)                                         \
  "" name16,
    REGX8632_TABLE
#undef X
  };
  switch (Ty) {
  case IceType_i1:
  case IceType_i8:
    return RegNames8[RegNum];
  case IceType_i16:
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
void TargetX8632::setArgOffsetAndCopy(Variable *Arg, Variable *FramePtr,
                                      int32_t BasicFrameOffset,
                                      int32_t &InArgsSizeBytes) {
  Variable *Lo = Arg->getLo();
  Variable *Hi = Arg->getHi();
  Type Ty = Arg->getType();
  if (Lo && Hi && Ty == IceType_i64) {
    assert(Lo->getType() != IceType_i64); // don't want infinite recursion
    assert(Hi->getType() != IceType_i64); // don't want infinite recursion
    setArgOffsetAndCopy(Lo, FramePtr, BasicFrameOffset, InArgsSizeBytes);
    setArgOffsetAndCopy(Hi, FramePtr, BasicFrameOffset, InArgsSizeBytes);
    return;
  }
  Arg->setStackOffset(BasicFrameOffset + InArgsSizeBytes);
  if (Arg->hasReg()) {
    assert(Ty != IceType_i64);
    OperandX8632Mem *Mem = OperandX8632Mem::create(
        Func, Ty, FramePtr,
        Ctx->getConstantInt(IceType_i32, Arg->getStackOffset()));
    _mov(Arg, Mem);
  }
  InArgsSizeBytes += typeWidthInBytesOnStack(Ty);
}

void TargetX8632::addProlog(CfgNode *Node) {
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

  // Determine stack frame offsets for each Variable without a
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
  std::vector<int> LocalsSize(Func->getNumNodes());

  // Prepass.  Compute RegsUsed, PreservedRegsSizeBytes, and
  // LocalsSizeBytes.
  RegsUsed = llvm::SmallBitVector(CalleeSaves.size());
  const VarList &Variables = Func->getVariables();
  const VarList &Args = Func->getArgs();
  for (VarList::const_iterator I = Variables.begin(), E = Variables.end();
       I != E; ++I) {
    Variable *Var = *I;
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
    if (Var->getWeight() == RegWeight::Zero && Var->getRegisterOverlap()) {
      if (Variable *Linked = Var->getPreferredRegister()) {
        if (!Linked->hasReg())
          continue;
      }
    }
    int32_t Increment = typeWidthInBytesOnStack(Var->getType());
    if (SimpleCoalescing) {
      if (Var->isMultiblockLife()) {
        GlobalsSize += Increment;
      } else {
        SizeT NodeIndex = Var->getLocalUseNode()->getIndex();
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
  for (SizeT i = 0; i < CalleeSaves.size(); ++i) {
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
    Variable *ebp = getPhysicalRegister(Reg_ebp);
    Variable *esp = getPhysicalRegister(Reg_esp);
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
  // RegManager code may have some permutation logic to leverage),
  // and if they have no home register, home space will need to be
  // allocated on the stack to copy into.
  Variable *FramePtr = getPhysicalRegister(getFrameOrStackReg());
  int32_t BasicFrameOffset = PreservedRegsSizeBytes + RetIpSizeBytes;
  if (!IsEbpBasedFrame)
    BasicFrameOffset += LocalsSizeBytes;
  for (SizeT i = 0; i < Args.size(); ++i) {
    Variable *Arg = Args[i];
    setArgOffsetAndCopy(Arg, FramePtr, BasicFrameOffset, InArgsSizeBytes);
  }

  // Fill in stack offsets for locals.
  int32_t TotalGlobalsSize = GlobalsSize;
  GlobalsSize = 0;
  LocalsSize.assign(LocalsSize.size(), 0);
  int32_t NextStackOffset = 0;
  for (VarList::const_iterator I = Variables.begin(), E = Variables.end();
       I != E; ++I) {
    Variable *Var = *I;
    if (Var->hasReg()) {
      RegsUsed[Var->getRegNum()] = true;
      continue;
    }
    if (Var->getIsArg())
      continue;
    if (ComputedLiveRanges && Var->getLiveRange().isEmpty())
      continue;
    if (Var->getWeight() == RegWeight::Zero && Var->getRegisterOverlap()) {
      if (Variable *Linked = Var->getPreferredRegister()) {
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
        SizeT NodeIndex = Var->getLocalUseNode()->getIndex();
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

  if (Func->getContext()->isVerbose(IceV_Frame)) {
    Func->getContext()->getStrDump() << "LocalsSizeBytes=" << LocalsSizeBytes
                                     << "\n"
                                     << "InArgsSizeBytes=" << InArgsSizeBytes
                                     << "\n"
                                     << "PreservedRegsSizeBytes="
                                     << PreservedRegsSizeBytes << "\n";
  }
}

void TargetX8632::addEpilog(CfgNode *Node) {
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

  Variable *esp = getPhysicalRegister(Reg_esp);
  if (IsEbpBasedFrame) {
    Variable *ebp = getPhysicalRegister(Reg_ebp);
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
  for (SizeT i = 0; i < CalleeSaves.size(); ++i) {
    SizeT j = CalleeSaves.size() - i - 1;
    if (j == Reg_ebp && IsEbpBasedFrame)
      continue;
    if (CalleeSaves[j] && RegsUsed[j]) {
      _pop(getPhysicalRegister(j));
    }
  }
}

void TargetX8632::split64(Variable *Var) {
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
  Variable *Lo = Var->getLo();
  Variable *Hi = Var->getHi();
  if (Lo) {
    assert(Hi);
    return;
  }
  assert(Hi == NULL);
  Lo = Func->makeVariable(IceType_i32, Context.getNode(),
                          Var->getName() + "__lo");
  Hi = Func->makeVariable(IceType_i32, Context.getNode(),
                          Var->getName() + "__hi");
  Var->setLoHi(Lo, Hi);
  if (Var->getIsArg()) {
    Lo->setIsArg(Func);
    Hi->setIsArg(Func);
  }
}

Operand *TargetX8632::loOperand(Operand *Operand) {
  assert(Operand->getType() == IceType_i64);
  if (Operand->getType() != IceType_i64)
    return Operand;
  if (Variable *Var = llvm::dyn_cast<Variable>(Operand)) {
    split64(Var);
    return Var->getLo();
  }
  if (ConstantInteger *Const = llvm::dyn_cast<ConstantInteger>(Operand)) {
    uint64_t Mask = (1ul << 32) - 1;
    return Ctx->getConstantInt(IceType_i32, Const->getValue() & Mask);
  }
  if (OperandX8632Mem *Mem = llvm::dyn_cast<OperandX8632Mem>(Operand)) {
    return OperandX8632Mem::create(Func, IceType_i32, Mem->getBase(),
                                   Mem->getOffset(), Mem->getIndex(),
                                   Mem->getShift());
  }
  assert(0 && "Unsupported operand type");
  return NULL;
}

Operand *TargetX8632::hiOperand(Operand *Operand) {
  assert(Operand->getType() == IceType_i64);
  if (Operand->getType() != IceType_i64)
    return Operand;
  if (Variable *Var = llvm::dyn_cast<Variable>(Operand)) {
    split64(Var);
    return Var->getHi();
  }
  if (ConstantInteger *Const = llvm::dyn_cast<ConstantInteger>(Operand)) {
    return Ctx->getConstantInt(IceType_i32, Const->getValue() >> 32);
  }
  if (OperandX8632Mem *Mem = llvm::dyn_cast<OperandX8632Mem>(Operand)) {
    Constant *Offset = Mem->getOffset();
    if (Offset == NULL)
      Offset = Ctx->getConstantInt(IceType_i32, 4);
    else if (ConstantInteger *IntOffset =
                 llvm::dyn_cast<ConstantInteger>(Offset)) {
      Offset = Ctx->getConstantInt(IceType_i32, 4 + IntOffset->getValue());
    } else if (ConstantRelocatable *SymOffset =
                   llvm::dyn_cast<ConstantRelocatable>(Offset)) {
      // TODO: This creates a new entry in the constant pool, instead
      // of reusing the existing entry.
      Offset = Ctx->getConstantSym(IceType_i32, 4 + SymOffset->getOffset(),
                                   SymOffset->getName());
    }
    return OperandX8632Mem::create(Func, IceType_i32, Mem->getBase(), Offset,
                                   Mem->getIndex(), Mem->getShift());
  }
  assert(0 && "Unsupported operand type");
  return NULL;
}

llvm::SmallBitVector TargetX8632::getRegisterSet(RegSetMask Include,
                                                 RegSetMask Exclude) const {
  llvm::SmallBitVector Registers(Reg_NUM);

#define X(val, init, name, name16, name8, scratch, preserved, stackptr,        \
          frameptr, isI8, isInt, isFP)                                         \
  if (scratch && (Include & RegSet_CallerSave))                                \
    Registers[val] = true;                                                     \
  if (preserved && (Include & RegSet_CalleeSave))                              \
    Registers[val] = true;                                                     \
  if (stackptr && (Include & RegSet_StackPointer))                             \
    Registers[val] = true;                                                     \
  if (frameptr && (Include & RegSet_FramePointer))                             \
    Registers[val] = true;                                                     \
  if (scratch && (Exclude & RegSet_CallerSave))                                \
    Registers[val] = false;                                                    \
  if (preserved && (Exclude & RegSet_CalleeSave))                              \
    Registers[val] = false;                                                    \
  if (stackptr && (Exclude & RegSet_StackPointer))                             \
    Registers[val] = false;                                                    \
  if (frameptr && (Exclude & RegSet_FramePointer))                             \
    Registers[val] = false;

  REGX8632_TABLE

#undef X

  return Registers;
}

void TargetX8632::lowerAlloca(const InstAlloca *Inst) {
  IsEbpBasedFrame = true;
  // TODO(sehr,stichnot): align allocated memory, keep stack aligned, minimize
  // the number of adjustments of esp, etc.
  Variable *esp = getPhysicalRegister(Reg_esp);
  Operand *TotalSize = legalize(Inst->getSizeInBytes());
  Variable *Dest = Inst->getDest();
  _sub(esp, TotalSize);
  _mov(Dest, esp);
}

void TargetX8632::lowerArithmetic(const InstArithmetic *Inst) {
  Variable *Dest = Inst->getDest();
  Operand *Src0 = legalize(Inst->getSrc(0));
  Operand *Src1 = legalize(Inst->getSrc(1));
  if (Dest->getType() == IceType_i64) {
    Variable *DestLo = llvm::cast<Variable>(loOperand(Dest));
    Variable *DestHi = llvm::cast<Variable>(hiOperand(Dest));
    Operand *Src0Lo = loOperand(Src0);
    Operand *Src0Hi = hiOperand(Src0);
    Operand *Src1Lo = loOperand(Src1);
    Operand *Src1Hi = hiOperand(Src1);
    Variable *T_Lo = NULL, *T_Hi = NULL;
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
      Variable *T_1 = NULL, *T_2 = NULL, *T_3 = NULL;
      Variable *T_4Lo = makeReg(IceType_i32, Reg_eax);
      Variable *T_4Hi = makeReg(IceType_i32, Reg_edx);
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
      Context.insert(InstFakeDef::create(Func, T_4Hi, T_4Lo));
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
      Variable *T_1 = NULL, *T_2 = NULL, *T_3 = NULL;
      Constant *BitTest = Ctx->getConstantInt(IceType_i32, 0x20);
      Constant *Zero = Ctx->getConstantInt(IceType_i32, 0);
      InstX8632Label *Label = InstX8632Label::create(Func, this);
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
      Context.insert(InstFakeUse::create(Func, T_3));
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
      Variable *T_1 = NULL, *T_2 = NULL, *T_3 = NULL;
      Constant *BitTest = Ctx->getConstantInt(IceType_i32, 0x20);
      Constant *Zero = Ctx->getConstantInt(IceType_i32, 0);
      InstX8632Label *Label = InstX8632Label::create(Func, this);
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
      Context.insert(InstFakeUse::create(Func, T_2));
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
      Variable *T_1 = NULL, *T_2 = NULL, *T_3 = NULL;
      Constant *BitTest = Ctx->getConstantInt(IceType_i32, 0x20);
      Constant *SignExtend = Ctx->getConstantInt(IceType_i32, 0x1f);
      InstX8632Label *Label = InstX8632Label::create(Func, this);
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
      Context.insert(InstFakeUse::create(Func, T_2));
      _mov(T_2, T_3);
      _sar(T_3, SignExtend);
      Context.insert(Label);
      _mov(DestLo, T_2);
      _mov(DestHi, T_3);
    } break;
    case InstArithmetic::Udiv: {
      const SizeT MaxSrcs = 2;
      InstCall *Call = makeHelperCall("__udivdi3", Dest, MaxSrcs);
      Call->addArg(Inst->getSrc(0));
      Call->addArg(Inst->getSrc(1));
      lowerCall(Call);
    } break;
    case InstArithmetic::Sdiv: {
      const SizeT MaxSrcs = 2;
      InstCall *Call = makeHelperCall("__divdi3", Dest, MaxSrcs);
      Call->addArg(Inst->getSrc(0));
      Call->addArg(Inst->getSrc(1));
      lowerCall(Call);
    } break;
    case InstArithmetic::Urem: {
      const SizeT MaxSrcs = 2;
      InstCall *Call = makeHelperCall("__umoddi3", Dest, MaxSrcs);
      Call->addArg(Inst->getSrc(0));
      Call->addArg(Inst->getSrc(1));
      lowerCall(Call);
    } break;
    case InstArithmetic::Srem: {
      const SizeT MaxSrcs = 2;
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
    Variable *T_edx = NULL;
    Variable *T = NULL;
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
      // TODO: Optimize for llvm::isa<Constant>(Src1)
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
      if (!llvm::isa<Constant>(Src1))
        Src1 = legalizeToVar(Src1, false, Reg_ecx);
      _shl(T, Src1);
      _mov(Dest, T);
      break;
    case InstArithmetic::Lshr:
      _mov(T, Src0);
      if (!llvm::isa<Constant>(Src1))
        Src1 = legalizeToVar(Src1, false, Reg_ecx);
      _shr(T, Src1);
      _mov(Dest, T);
      break;
    case InstArithmetic::Ashr:
      _mov(T, Src0);
      if (!llvm::isa<Constant>(Src1))
        Src1 = legalizeToVar(Src1, false, Reg_ecx);
      _sar(T, Src1);
      _mov(Dest, T);
      break;
    case InstArithmetic::Udiv:
      if (Dest->getType() == IceType_i8) {
        Variable *T_ah = NULL;
        Constant *Zero = Ctx->getConstantInt(IceType_i8, 0);
        _mov(T, Src0, Reg_eax);
        _mov(T_ah, Zero, Reg_ah);
        _div(T_ah, Src1, T);
        Context.insert(InstFakeUse::create(Func, T_ah));
        _mov(Dest, T);
      } else {
        // TODO: fix for 8-bit, see Urem
        Constant *Zero = Ctx->getConstantInt(IceType_i32, 0);
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
        Variable *T_ah = NULL;
        Constant *Zero = Ctx->getConstantInt(IceType_i8, 0);
        _mov(T, Src0, Reg_eax);
        _mov(T_ah, Zero, Reg_ah);
        _div(T_ah, Src1, T);
        _mov(Dest, T_ah);
      } else {
        Constant *Zero = Ctx->getConstantInt(IceType_i32, 0);
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
      const SizeT MaxSrcs = 2;
      Type Ty = Dest->getType();
      InstCall *Call =
          makeHelperCall(Ty == IceType_f32 ? "fmodf" : "fmod", Dest, MaxSrcs);
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

void TargetX8632::lowerAssign(const InstAssign *Inst) {
  Variable *Dest = Inst->getDest();
  Operand *Src0 = legalize(Inst->getSrc(0));
  assert(Dest->getType() == Src0->getType());
  if (Dest->getType() == IceType_i64) {
    Operand *Src0Lo = loOperand(Src0);
    Operand *Src0Hi = hiOperand(Src0);
    Variable *DestLo = llvm::cast<Variable>(loOperand(Dest));
    Variable *DestHi = llvm::cast<Variable>(hiOperand(Dest));
    Variable *T_Lo = NULL, *T_Hi = NULL;
    _mov(T_Lo, Src0Lo);
    _mov(DestLo, T_Lo);
    _mov(T_Hi, Src0Hi);
    _mov(DestHi, T_Hi);
  } else {
    bool AllowOverlap = true;
    // RI is either a physical register or an immediate.
    Operand *RI = legalize(Src0, Legal_Reg | Legal_Imm, AllowOverlap);
    _mov(Dest, RI);
  }
}

void TargetX8632::lowerBr(const InstBr *Inst) {
  if (Inst->isUnconditional()) {
    _br(Inst->getTargetUnconditional());
  } else {
    Operand *Src0 = legalize(Inst->getCondition());
    Constant *Zero = Ctx->getConstantInt(IceType_i32, 0);
    _cmp(Src0, Zero);
    _br(InstX8632Br::Br_ne, Inst->getTargetTrue(), Inst->getTargetFalse());
  }
}

void TargetX8632::lowerCall(const InstCall *Instr) {
  // TODO: what to do about tailcalls?
  // Generate a sequence of push instructions, pushing right to left,
  // keeping track of stack offsets in case a push involves a stack
  // operand and we are using an esp-based frame.
  uint32_t StackOffset = 0;
  // TODO: If for some reason the call instruction gets dead-code
  // eliminated after lowering, we would need to ensure that the
  // pre-call push instructions and the post-call esp adjustment get
  // eliminated as well.
  for (SizeT NumArgs = Instr->getNumArgs(), i = 0; i < NumArgs; ++i) {
    Operand *Arg = legalize(Instr->getArg(NumArgs - i - 1));
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
      Variable *T = NULL;
      _mov(T, Arg);
      _push(T);
    } else {
      _push(Arg);
    }
    StackOffset += typeWidthInBytesOnStack(Arg->getType());
  }
  // Generate the call instruction.  Assign its result to a temporary
  // with high register allocation weight.
  Variable *Dest = Instr->getDest();
  Variable *eax = NULL; // doubles as RegLo as necessary
  Variable *edx = NULL;
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
  Operand *CallTarget = legalize(Instr->getCallTarget());
  Inst *NewCall = InstX8632Call::create(Func, eax, CallTarget);
  Context.insert(NewCall);
  if (edx)
    Context.insert(InstFakeDef::create(Func, edx));

  // Add the appropriate offset to esp.
  if (StackOffset) {
    Variable *esp = Func->getTarget()->getPhysicalRegister(Reg_esp);
    _add(esp, Ctx->getConstantInt(IceType_i32, StackOffset));
  }

  // Insert a register-kill pseudo instruction.
  VarList KilledRegs;
  for (SizeT i = 0; i < ScratchRegs.size(); ++i) {
    if (ScratchRegs[i])
      KilledRegs.push_back(Func->getTarget()->getPhysicalRegister(i));
  }
  if (!KilledRegs.empty()) {
    Inst *Kill = InstFakeKill::create(Func, KilledRegs, NewCall);
    Context.insert(Kill);
  }

  // Generate a FakeUse to keep the call live if necessary.
  if (Instr->hasSideEffects() && eax) {
    Inst *FakeUse = InstFakeUse::create(Func, eax);
    Context.insert(FakeUse);
  }

  // Generate Dest=eax assignment.
  if (Dest && eax) {
    if (edx) {
      split64(Dest);
      Variable *DestLo = Dest->getLo();
      Variable *DestHi = Dest->getHi();
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

void TargetX8632::lowerCast(const InstCast *Inst) {
  // a = cast(b) ==> t=cast(b); a=t; (link t->b, link a->t, no overlap)
  InstCast::OpKind CastKind = Inst->getCastKind();
  Variable *Dest = Inst->getDest();
  // Src0RM is the source operand legalized to physical register or memory, but
  // not immediate, since the relevant x86 native instructions don't allow an
  // immediate operand.  If the operand is an immediate, we could consider
  // computing the strength-reduced result at translation time, but we're
  // unlikely to see something like that in the bitcode that the optimizer
  // wouldn't have already taken care of.
  Operand *Src0RM = legalize(Inst->getSrc(0), Legal_Reg | Legal_Mem, true);
  switch (CastKind) {
  default:
    Func->setError("Cast type not supported");
    return;
  case InstCast::Sext:
    if (Dest->getType() == IceType_i64) {
      // t1=movsx src; t2=t1; t2=sar t2, 31; dst.lo=t1; dst.hi=t2
      Variable *DestLo = llvm::cast<Variable>(loOperand(Dest));
      Variable *DestHi = llvm::cast<Variable>(hiOperand(Dest));
      Variable *T_Lo = makeReg(DestLo->getType());
      if (Src0RM->getType() == IceType_i32)
        _mov(T_Lo, Src0RM);
      else
        _movsx(T_Lo, Src0RM);
      _mov(DestLo, T_Lo);
      Variable *T_Hi = NULL;
      Constant *Shift = Ctx->getConstantInt(IceType_i32, 31);
      _mov(T_Hi, T_Lo);
      _sar(T_Hi, Shift);
      _mov(DestHi, T_Hi);
    } else {
      // TODO: Sign-extend an i1 via "shl reg, 31; sar reg, 31", and
      // also copy to the high operand of a 64-bit variable.
      // t1 = movsx src; dst = t1
      Variable *T = makeReg(Dest->getType());
      _movsx(T, Src0RM);
      _mov(Dest, T);
    }
    break;
  case InstCast::Zext:
    if (Dest->getType() == IceType_i64) {
      // t1=movzx src; dst.lo=t1; dst.hi=0
      Constant *Zero = Ctx->getConstantInt(IceType_i32, 0);
      Variable *DestLo = llvm::cast<Variable>(loOperand(Dest));
      Variable *DestHi = llvm::cast<Variable>(hiOperand(Dest));
      Variable *Tmp = makeReg(DestLo->getType());
      if (Src0RM->getType() == IceType_i32)
        _mov(Tmp, Src0RM);
      else
        _movzx(Tmp, Src0RM);
      _mov(DestLo, Tmp);
      _mov(DestHi, Zero);
    } else if (Src0RM->getType() == IceType_i1) {
      // t = Src0RM; t &= 1; Dest = t
      Operand *One = Ctx->getConstantInt(IceType_i32, 1);
      Variable *T = makeReg(IceType_i32);
      _movzx(T, Src0RM);
      _and(T, One);
      _mov(Dest, T);
    } else {
      // t1 = movzx src; dst = t1
      Variable *T = makeReg(Dest->getType());
      _movzx(T, Src0RM);
      _mov(Dest, T);
    }
    break;
  case InstCast::Trunc: {
    if (Src0RM->getType() == IceType_i64)
      Src0RM = loOperand(Src0RM);
    // t1 = trunc Src0RM; Dest = t1
    Variable *T = NULL;
    _mov(T, Src0RM);
    _mov(Dest, T);
    break;
  }
  case InstCast::Fptrunc:
  case InstCast::Fpext: {
    // t1 = cvt Src0RM; Dest = t1
    Variable *T = makeReg(Dest->getType());
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
      const SizeT MaxSrcs = 1;
      Type SrcType = Inst->getSrc(0)->getType();
      InstCall *Call = makeHelperCall(
          SrcType == IceType_f32 ? "cvtftosi64" : "cvtdtosi64", Dest, MaxSrcs);
      Call->addArg(Inst->getSrc(0));
      lowerCall(Call);
    } else {
      // t1.i32 = cvt Src0RM; t2.dest_type = t1; Dest = t2.dest_type
      Variable *T_1 = makeReg(IceType_i32);
      Variable *T_2 = makeReg(Dest->getType());
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
      const SizeT MaxSrcs = 1;
      Type DestType = Dest->getType();
      Type SrcType = Src0RM->getType();
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
      Variable *T_1 = makeReg(IceType_i32);
      Variable *T_2 = makeReg(Dest->getType());
      _cvt(T_1, Src0RM);
      _mov(T_2, T_1); // T_1 and T_2 may have different integer types
      _mov(Dest, T_2);
      T_2->setPreferredRegister(T_1, true);
    }
    break;
  case InstCast::Sitofp:
    if (Src0RM->getType() == IceType_i64) {
      // Use a helper for x86-32.
      const SizeT MaxSrcs = 1;
      Type DestType = Dest->getType();
      InstCall *Call = makeHelperCall(
          DestType == IceType_f32 ? "cvtsi64tof" : "cvtsi64tod", Dest, MaxSrcs);
      Call->addArg(Inst->getSrc(0));
      lowerCall(Call);
      return;
    } else {
      // Sign-extend the operand.
      // t1.i32 = movsx Src0RM; t2 = Cvt t1.i32; Dest = t2
      Variable *T_1 = makeReg(IceType_i32);
      Variable *T_2 = makeReg(Dest->getType());
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
      const SizeT MaxSrcs = 1;
      Type DestType = Dest->getType();
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
      Variable *T_1 = makeReg(IceType_i32);
      Variable *T_2 = makeReg(Dest->getType());
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
      InstAssign *Assign = InstAssign::create(Func, Dest, Src0RM);
      lowerAssign(Assign);
      assert(0 && "Pointer bitcasts aren't lowered correctly.");
      return;
    }
    switch (Dest->getType()) {
    default:
      assert(0 && "Unexpected Bitcast dest type");
    case IceType_i32:
    case IceType_f32: {
      Type DestType = Dest->getType();
      Type SrcType = Src0RM->getType();
      assert((DestType == IceType_i32 && SrcType == IceType_f32) ||
             (DestType == IceType_f32 && SrcType == IceType_i32));
      // a.i32 = bitcast b.f32 ==>
      //   t.f32 = b.f32
      //   s.f32 = spill t.f32
      //   a.i32 = s.f32
      Variable *T = NULL;
      // TODO: Should be able to force a spill setup by calling legalize() with
      // Legal_Mem and not Legal_Reg or Legal_Imm.
      Variable *Spill = Func->makeVariable(SrcType, Context.getNode());
      Spill->setWeight(RegWeight::Zero);
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
      Variable *Spill = Func->makeVariable(IceType_f64, Context.getNode());
      Spill->setWeight(RegWeight::Zero);
      Spill->setPreferredRegister(llvm::dyn_cast<Variable>(Src0RM), true);
      _mov(Spill, Src0RM);

      Variable *DestLo = llvm::cast<Variable>(loOperand(Dest));
      Variable *DestHi = llvm::cast<Variable>(hiOperand(Dest));
      Variable *T_Lo = makeReg(IceType_i32);
      Variable *T_Hi = makeReg(IceType_i32);
      VariableSplit *SpillLo =
          VariableSplit::create(Func, Spill, VariableSplit::Low);
      VariableSplit *SpillHi =
          VariableSplit::create(Func, Spill, VariableSplit::High);

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
      Variable *Spill = Func->makeVariable(IceType_f64, Context.getNode());
      Spill->setWeight(RegWeight::Zero);
      Spill->setPreferredRegister(Dest, true);

      Context.insert(InstFakeDef::create(Func, Spill));

      Variable *T_Lo = NULL, *T_Hi = NULL;
      VariableSplit *SpillLo =
          VariableSplit::create(Func, Spill, VariableSplit::Low);
      VariableSplit *SpillHi =
          VariableSplit::create(Func, Spill, VariableSplit::High);
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

// TODO: Integrate FCMPX8632_TABLE and ICEINSTFCMP_TABLE.
#define FCMPX8632_TABLE                                                        \
  /* val, dflt, swap, C1, C2 */                                                \
  X(False, 0, false, Br_None, Br_None) /* no line break clang-format */        \
      X(Oeq, 0, false, Br_ne, Br_p)    /* no line break clang-format */           \
      X(Ogt, 1, false, Br_a, Br_None)  /* no line break clang-format */         \
      X(Oge, 1, false, Br_ae, Br_None) /* no line break clang-format */        \
      X(Olt, 1, true, Br_a, Br_None)   /* no line break clang-format */          \
      X(Ole, 1, true, Br_ae, Br_None)  /* no line break clang-format */         \
      X(One, 1, false, Br_ne, Br_None) /* no line break clang-format */        \
      X(Ord, 1, false, Br_np, Br_None) /* no line break clang-format */        \
      X(Ueq, 1, false, Br_e, Br_None)  /* no line break clang-format */         \
      X(Ugt, 1, true, Br_b, Br_None)   /* no line break clang-format */          \
      X(Uge, 1, true, Br_be, Br_None)  /* no line break clang-format */         \
      X(Ult, 1, false, Br_b, Br_None)  /* no line break clang-format */         \
      X(Ule, 1, false, Br_be, Br_None) /* no line break clang-format */        \
      X(Une, 1, false, Br_ne, Br_p)    /* no line break clang-format */           \
      X(Uno, 1, false, Br_p, Br_None)  /* no line break clang-format */         \
      X(True, 1, false, Br_None, Br_None)

const struct {
  InstFcmp::FCond Cond;
  uint32_t Default;
  bool SwapOperands;
  InstX8632Br::BrCond C1, C2;
} TableFcmp[] = {
#define X(val, dflt, swap, C1, C2)                                             \
  { InstFcmp::val, dflt, swap, InstX8632Br::C1, InstX8632Br::C2 }              \
  ,
    FCMPX8632_TABLE
#undef X
  };
const size_t TableFcmpSize = sizeof(TableFcmp) / sizeof(*TableFcmp);

} // anonymous namespace

void TargetX8632::lowerFcmp(const InstFcmp *Inst) {
  Operand *Src0 = Inst->getSrc(0);
  Operand *Src1 = Inst->getSrc(1);
  Variable *Dest = Inst->getDest();
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
  size_t Index = static_cast<size_t>(Condition);
  assert(Index < TableFcmpSize);
  // The table is indexed by InstFcmp::Condition.  Make sure it didn't fall
  // out of order.
  assert(TableFcmp[Index].Cond == Condition);
  if (TableFcmp[Index].SwapOperands) {
    Operand *Tmp = Src0;
    Src0 = Src1;
    Src1 = Tmp;
  }
  bool HasC1 = (TableFcmp[Index].C1 != InstX8632Br::Br_None);
  bool HasC2 = (TableFcmp[Index].C2 != InstX8632Br::Br_None);
  if (HasC1) {
    Src0 = legalize(Src0);
    Operand *Src1RM = legalize(Src1, Legal_Reg | Legal_Mem);
    Variable *T = NULL;
    _mov(T, Src0);
    _ucomiss(T, Src1RM);
  }
  Constant *Default =
      Ctx->getConstantInt(IceType_i32, TableFcmp[Index].Default);
  _mov(Dest, Default);
  if (HasC1) {
    InstX8632Label *Label = InstX8632Label::create(Func, this);
    _br(TableFcmp[Index].C1, Label);
    if (HasC2) {
      _br(TableFcmp[Index].C2, Label);
    }
    Context.insert(InstFakeUse::create(Func, Dest));
    Constant *NonDefault =
        Ctx->getConstantInt(IceType_i32, !TableFcmp[Index].Default);
    _mov(Dest, NonDefault);
    Context.insert(Label);
  }
}

namespace {

// The following table summarizes the logic for lowering the icmp instruction
// for i32 and narrower types.  Each icmp condition has a clear mapping to an
// x86 conditional branch instruction.

// TODO: Integrate ICMPX8632_TABLE and ICEINSTICMP_TABLE.
#define ICMPX8632_TABLE                                                        \
  /* val, C_32, C1_64, C2_64, C3_64 */                                         \
  X(Eq, Br_e, Br_None, Br_None, Br_None)      /* no clang-format line break */      \
      X(Ne, Br_ne, Br_None, Br_None, Br_None) /* no clang-format line break */ \
      X(Ugt, Br_a, Br_a, Br_b, Br_a)          /* no clang-format line break */          \
      X(Uge, Br_ae, Br_a, Br_b, Br_ae)        /* no clang-format line break */        \
      X(Ult, Br_b, Br_b, Br_a, Br_b)          /* no clang-format line break */          \
      X(Ule, Br_be, Br_b, Br_a, Br_be)        /* no clang-format line break */        \
      X(Sgt, Br_g, Br_g, Br_l, Br_a)          /* no clang-format line break */          \
      X(Sge, Br_ge, Br_g, Br_l, Br_ae)        /* no clang-format line break */        \
      X(Slt, Br_l, Br_l, Br_g, Br_b)          /* no clang-format line break */          \
      X(Sle, Br_le, Br_l, Br_g, Br_be)        /* no clang-format line break */

const struct {
  InstIcmp::ICond Cond;
  InstX8632Br::BrCond Mapping;
} TableIcmp32[] = {
#define X(val, C_32, C1_64, C2_64, C3_64)                                      \
  { InstIcmp::val, InstX8632Br::C_32 }                                         \
  ,
    ICMPX8632_TABLE
#undef X
  };
const size_t TableIcmp32Size = sizeof(TableIcmp32) / sizeof(*TableIcmp32);

// The following table summarizes the logic for lowering the icmp instruction
// for the i64 type.  For Eq and Ne, two separate 32-bit comparisons and
// conditional branches are needed.  For the other conditions, three separate
// conditional branches are needed.
const struct {
  InstIcmp::ICond Cond;
  InstX8632Br::BrCond C1, C2, C3;
} TableIcmp64[] = {
#define X(val, C_32, C1_64, C2_64, C3_64)                                       \
  { InstIcmp::val, InstX8632Br::C1_64, InstX8632Br::C2_64, InstX8632Br::C3_64 } \
  ,
    ICMPX8632_TABLE
#undef X
  };
const size_t TableIcmp64Size = sizeof(TableIcmp64) / sizeof(*TableIcmp64);

InstX8632Br::BrCond getIcmp32Mapping(InstIcmp::ICond Cond) {
  size_t Index = static_cast<size_t>(Cond);
  assert(Index < TableIcmp32Size);
  assert(TableIcmp32[Index].Cond == Cond);
  return TableIcmp32[Index].Mapping;
}

} // anonymous namespace

void TargetX8632::lowerIcmp(const InstIcmp *Inst) {
  Operand *Src0 = legalize(Inst->getSrc(0));
  Operand *Src1 = legalize(Inst->getSrc(1));
  Variable *Dest = Inst->getDest();

  // If Src1 is an immediate, or known to be a physical register, we can
  // allow Src0 to be a memory operand.  Otherwise, Src0 must be copied into
  // a physical register.  (Actually, either Src0 or Src1 can be chosen for
  // the physical register, but unfortunately we have to commit to one or
  // the other before register allocation.)
  bool IsSrc1ImmOrReg = false;
  if (llvm::isa<Constant>(Src1))
    IsSrc1ImmOrReg = true;
  else if (Variable *Var = llvm::dyn_cast<Variable>(Src1)) {
    if (Var->hasReg())
      IsSrc1ImmOrReg = true;
  }

  // Try to fuse a compare immediately followed by a conditional branch.  This
  // is possible when the compare dest and the branch source operands are the
  // same, and are their only uses.  TODO: implement this optimization for i64.
  if (InstBr *NextBr = llvm::dyn_cast_or_null<InstBr>(Context.getNextInst())) {
    if (Src0->getType() != IceType_i64 && !NextBr->isUnconditional() &&
        Dest == NextBr->getSrc(0) && NextBr->isLastUse(Dest)) {
      Operand *Src0New =
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
  Constant *Zero = Ctx->getConstantInt(IceType_i32, 0);
  Constant *One = Ctx->getConstantInt(IceType_i32, 1);
  if (Src0->getType() == IceType_i64) {
    InstIcmp::ICond Condition = Inst->getCondition();
    size_t Index = static_cast<size_t>(Condition);
    assert(Index < TableIcmp64Size);
    // The table is indexed by InstIcmp::Condition.  Make sure it didn't fall
    // out of order.
    assert(TableIcmp64[Index].Cond == Condition);
    Operand *Src1LoRI = legalize(loOperand(Src1), Legal_Reg | Legal_Imm);
    Operand *Src1HiRI = legalize(hiOperand(Src1), Legal_Reg | Legal_Imm);
    if (Condition == InstIcmp::Eq || Condition == InstIcmp::Ne) {
      InstX8632Label *Label = InstX8632Label::create(Func, this);
      _mov(Dest, (Condition == InstIcmp::Eq ? Zero : One));
      _cmp(loOperand(Src0), Src1LoRI);
      _br(InstX8632Br::Br_ne, Label);
      _cmp(hiOperand(Src0), Src1HiRI);
      _br(InstX8632Br::Br_ne, Label);
      Context.insert(InstFakeUse::create(Func, Dest));
      _mov(Dest, (Condition == InstIcmp::Eq ? One : Zero));
      Context.insert(Label);
    } else {
      InstX8632Label *LabelFalse = InstX8632Label::create(Func, this);
      InstX8632Label *LabelTrue = InstX8632Label::create(Func, this);
      _mov(Dest, One);
      _cmp(hiOperand(Src0), Src1HiRI);
      _br(TableIcmp64[Index].C1, LabelTrue);
      _br(TableIcmp64[Index].C2, LabelFalse);
      _cmp(loOperand(Src0), Src1LoRI);
      _br(TableIcmp64[Index].C3, LabelTrue);
      Context.insert(LabelFalse);
      Context.insert(InstFakeUse::create(Func, Dest));
      _mov(Dest, Zero);
      Context.insert(LabelTrue);
    }
    return;
  }
  // cmp b, c
  Operand *Src0New =
      legalize(Src0, IsSrc1ImmOrReg ? Legal_All : Legal_Reg, true);
  InstX8632Label *Label = InstX8632Label::create(Func, this);
  _cmp(Src0New, Src1);
  _mov(Dest, One);
  _br(getIcmp32Mapping(Inst->getCondition()), Label);
  Context.insert(InstFakeUse::create(Func, Dest));
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

void computeAddressOpt(Cfg * /*Func*/, Variable *&Base, Variable *&Index,
                       int32_t &Shift, int32_t &Offset) {
  (void)Offset; // TODO: pattern-match for non-zero offsets.
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
    Operand *BaseOperand0 = BaseInst ? BaseInst->getSrc(0) : NULL;
    Variable *BaseVariable0 = llvm::dyn_cast_or_null<Variable>(BaseOperand0);
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
    Operand *BaseOperand1 =
        BaseInst && BaseInst->getSrcSize() >= 2 ? BaseInst->getSrc(1) : NULL;
    Variable *BaseVariable1 = llvm::dyn_cast_or_null<Variable>(BaseOperand1);
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
      Operand *IndexOperand0 = ArithInst->getSrc(0);
      Variable *IndexVariable0 = llvm::dyn_cast<Variable>(IndexOperand0);
      Operand *IndexOperand1 = ArithInst->getSrc(1);
      ConstantInteger *IndexConstant1 =
          llvm::dyn_cast<ConstantInteger>(IndexOperand1);
      if (ArithInst->getOp() == InstArithmetic::Mul && IndexVariable0 &&
          IndexOperand1->getType() == IceType_i32 && IndexConstant1) {
        uint64_t Mult = IndexConstant1->getValue();
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

void TargetX8632::lowerLoad(const InstLoad *Inst) {
  // A Load instruction can be treated the same as an Assign
  // instruction, after the source operand is transformed into an
  // OperandX8632Mem operand.  Note that the address mode
  // optimization already creates an OperandX8632Mem operand, so it
  // doesn't need another level of transformation.
  Type Ty = Inst->getDest()->getType();
  Operand *Src0 = Inst->getSourceAddress();
  // Address mode optimization already creates an OperandX8632Mem
  // operand, so it doesn't need another level of transformation.
  if (!llvm::isa<OperandX8632Mem>(Src0)) {
    Variable *Base = llvm::dyn_cast<Variable>(Src0);
    Constant *Offset = llvm::dyn_cast<Constant>(Src0);
    assert(Base || Offset);
    Src0 = OperandX8632Mem::create(Func, Ty, Base, Offset);
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
    Variable *DestLoad = Inst->getDest();
    Variable *Src0Arith = llvm::dyn_cast<Variable>(Arith->getSrc(0));
    Variable *Src1Arith = llvm::dyn_cast<Variable>(Arith->getSrc(1));
    if (Src1Arith == DestLoad && Arith->isLastUse(Src1Arith) &&
        DestLoad != Src0Arith) {
      NewArith = InstArithmetic::create(Func, Arith->getOp(), Arith->getDest(),
                                        Arith->getSrc(0), Src0);
    } else if (Src0Arith == DestLoad && Arith->isCommutative() &&
               Arith->isLastUse(Src0Arith) && DestLoad != Src1Arith) {
      NewArith = InstArithmetic::create(Func, Arith->getOp(), Arith->getDest(),
                                        Arith->getSrc(1), Src0);
    }
    if (NewArith) {
      Arith->setDeleted();
      Context.advanceNext();
      lowerArithmetic(NewArith);
      return;
    }
  }

  InstAssign *Assign = InstAssign::create(Func, Inst->getDest(), Src0);
  lowerAssign(Assign);
}

void TargetX8632::doAddressOptLoad() {
  Inst *Inst = *Context.getCur();
  Variable *Dest = Inst->getDest();
  Operand *Addr = Inst->getSrc(0);
  Variable *Index = NULL;
  int32_t Shift = 0;
  int32_t Offset = 0; // TODO: make Constant
  Variable *Base = llvm::dyn_cast<Variable>(Addr);
  computeAddressOpt(Func, Base, Index, Shift, Offset);
  if (Base && Addr != Base) {
    Constant *OffsetOp = Ctx->getConstantInt(IceType_i32, Offset);
    Addr = OperandX8632Mem::create(Func, Dest->getType(), Base, OffsetOp, Index,
                                   Shift);
    Inst->setDeleted();
    Context.insert(InstLoad::create(Func, Dest, Addr));
  }
}

void TargetX8632::lowerPhi(const InstPhi * /*Inst*/) {
  Func->setError("Phi lowering not implemented");
}

void TargetX8632::lowerRet(const InstRet *Inst) {
  Variable *Reg = NULL;
  if (Inst->hasRetValue()) {
    Operand *Src0 = legalize(Inst->getRetValue());
    if (Src0->getType() == IceType_i64) {
      Variable *eax = legalizeToVar(loOperand(Src0), false, Reg_eax);
      Variable *edx = legalizeToVar(hiOperand(Src0), false, Reg_edx);
      Reg = eax;
      Context.insert(InstFakeUse::create(Func, edx));
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
  Variable *esp = Func->getTarget()->getPhysicalRegister(Reg_esp);
  Context.insert(InstFakeUse::create(Func, esp));
}

void TargetX8632::lowerSelect(const InstSelect *Inst) {
  // a=d?b:c ==> cmp d,0; a=b; jne L1; FakeUse(a); a=c; L1:
  Variable *Dest = Inst->getDest();
  Operand *SrcT = Inst->getTrueOperand();
  Operand *SrcF = Inst->getFalseOperand();
  Operand *Condition = legalize(Inst->getCondition());
  Constant *Zero = Ctx->getConstantInt(IceType_i32, 0);
  InstX8632Label *Label = InstX8632Label::create(Func, this);

  if (Dest->getType() == IceType_i64) {
    Variable *DestLo = llvm::cast<Variable>(loOperand(Dest));
    Variable *DestHi = llvm::cast<Variable>(hiOperand(Dest));
    Operand *SrcLoRI = legalize(loOperand(SrcT), Legal_Reg | Legal_Imm, true);
    Operand *SrcHiRI = legalize(hiOperand(SrcT), Legal_Reg | Legal_Imm, true);
    _cmp(Condition, Zero);
    _mov(DestLo, SrcLoRI);
    _mov(DestHi, SrcHiRI);
    _br(InstX8632Br::Br_ne, Label);
    Context.insert(InstFakeUse::create(Func, DestLo));
    Context.insert(InstFakeUse::create(Func, DestHi));
    Operand *SrcFLo = loOperand(SrcF);
    Operand *SrcFHi = hiOperand(SrcF);
    SrcLoRI = legalize(SrcFLo, Legal_Reg | Legal_Imm, true);
    SrcHiRI = legalize(SrcFHi, Legal_Reg | Legal_Imm, true);
    _mov(DestLo, SrcLoRI);
    _mov(DestHi, SrcHiRI);
  } else {
    _cmp(Condition, Zero);
    SrcT = legalize(SrcT, Legal_Reg | Legal_Imm, true);
    _mov(Dest, SrcT);
    _br(InstX8632Br::Br_ne, Label);
    Context.insert(InstFakeUse::create(Func, Dest));
    SrcF = legalize(SrcF, Legal_Reg | Legal_Imm, true);
    _mov(Dest, SrcF);
  }

  Context.insert(Label);
}

void TargetX8632::lowerStore(const InstStore *Inst) {
  Operand *Value = Inst->getData();
  Operand *Addr = Inst->getAddr();
  OperandX8632Mem *NewAddr = llvm::dyn_cast<OperandX8632Mem>(Addr);
  // Address mode optimization already creates an OperandX8632Mem
  // operand, so it doesn't need another level of transformation.
  if (!NewAddr) {
    // The address will be either a constant (which represents a global
    // variable) or a variable, so either the Base or Offset component
    // of the OperandX8632Mem will be set.
    Variable *Base = llvm::dyn_cast<Variable>(Addr);
    Constant *Offset = llvm::dyn_cast<Constant>(Addr);
    assert(Base || Offset);
    NewAddr = OperandX8632Mem::create(Func, Value->getType(), Base, Offset);
  }
  NewAddr = llvm::cast<OperandX8632Mem>(legalize(NewAddr));

  if (NewAddr->getType() == IceType_i64) {
    Value = legalize(Value);
    Operand *ValueHi = legalize(hiOperand(Value), Legal_Reg | Legal_Imm, true);
    Operand *ValueLo = legalize(loOperand(Value), Legal_Reg | Legal_Imm, true);
    _store(ValueHi, llvm::cast<OperandX8632Mem>(hiOperand(NewAddr)));
    _store(ValueLo, llvm::cast<OperandX8632Mem>(loOperand(NewAddr)));
  } else {
    Value = legalize(Value, Legal_Reg | Legal_Imm, true);
    _store(Value, NewAddr);
  }
}

void TargetX8632::doAddressOptStore() {
  InstStore *Inst = llvm::cast<InstStore>(*Context.getCur());
  Operand *Data = Inst->getData();
  Operand *Addr = Inst->getAddr();
  Variable *Index = NULL;
  int32_t Shift = 0;
  int32_t Offset = 0; // TODO: make Constant
  Variable *Base = llvm::dyn_cast<Variable>(Addr);
  computeAddressOpt(Func, Base, Index, Shift, Offset);
  if (Base && Addr != Base) {
    Constant *OffsetOp = Ctx->getConstantInt(IceType_i32, Offset);
    Addr = OperandX8632Mem::create(Func, Data->getType(), Base, OffsetOp, Index,
                                   Shift);
    Inst->setDeleted();
    Context.insert(InstStore::create(Func, Data, Addr));
  }
}

void TargetX8632::lowerSwitch(const InstSwitch *Inst) {
  // This implements the most naive possible lowering.
  // cmp a,val[0]; jeq label[0]; cmp a,val[1]; jeq label[1]; ... jmp default
  Operand *Src0 = Inst->getComparison();
  SizeT NumCases = Inst->getNumCases();
  // OK, we'll be slightly less naive by forcing Src into a physical
  // register if there are 2 or more uses.
  if (NumCases >= 2)
    Src0 = legalizeToVar(Src0, true);
  else
    Src0 = legalize(Src0, Legal_All, true);
  for (SizeT I = 0; I < NumCases; ++I) {
    Operand *Value = Ctx->getConstantInt(IceType_i32, Inst->getValue(I));
    _cmp(Src0, Value);
    _br(InstX8632Br::Br_e, Inst->getLabel(I));
  }

  _br(Inst->getLabelDefault());
}

void TargetX8632::lowerUnreachable(const InstUnreachable * /*Inst*/) {
  const SizeT MaxSrcs = 0;
  Variable *Dest = NULL;
  InstCall *Call = makeHelperCall("ice_unreachable", Dest, MaxSrcs);
  lowerCall(Call);
}

Operand *TargetX8632::legalize(Operand *From, LegalMask Allowed,
                               bool AllowOverlap, int32_t RegNum) {
  assert(Allowed & Legal_Reg);
  assert(RegNum == Variable::NoRegister || Allowed == Legal_Reg);
  if (OperandX8632Mem *Mem = llvm::dyn_cast<OperandX8632Mem>(From)) {
    Variable *Base = Mem->getBase();
    Variable *Index = Mem->getIndex();
    Variable *RegBase = Base;
    Variable *RegIndex = Index;
    if (Base) {
      RegBase = legalizeToVar(Base, true);
    }
    if (Index) {
      RegIndex = legalizeToVar(Index, true);
    }
    if (Base != RegBase || Index != RegIndex) {
      From =
          OperandX8632Mem::create(Func, Mem->getType(), RegBase,
                                  Mem->getOffset(), RegIndex, Mem->getShift());
    }

    if (!(Allowed & Legal_Mem)) {
      Variable *Reg = makeReg(From->getType(), RegNum);
      _mov(Reg, From, RegNum);
      From = Reg;
    }
    return From;
  }
  if (llvm::isa<Constant>(From)) {
    if (!(Allowed & Legal_Imm)) {
      Variable *Reg = makeReg(From->getType(), RegNum);
      _mov(Reg, From);
      From = Reg;
    }
    return From;
  }
  if (Variable *Var = llvm::dyn_cast<Variable>(From)) {
    // We need a new physical register for the operand if:
    //   Mem is not allowed and Var->getRegNum() is unknown, or
    //   RegNum is required and Var->getRegNum() doesn't match.
    if ((!(Allowed & Legal_Mem) && !Var->hasReg()) ||
        (RegNum != Variable::NoRegister && RegNum != Var->getRegNum())) {
      Variable *Reg = makeReg(From->getType(), RegNum);
      if (RegNum == Variable::NoRegister) {
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

Variable *TargetX8632::legalizeToVar(Operand *From, bool AllowOverlap,
                                     int32_t RegNum) {
  return llvm::cast<Variable>(legalize(From, Legal_Reg, AllowOverlap, RegNum));
}

Variable *TargetX8632::makeReg(Type Type, int32_t RegNum) {
  Variable *Reg = Func->makeVariable(Type, Context.getNode());
  if (RegNum == Variable::NoRegister)
    Reg->setWeightInfinite();
  else
    Reg->setRegNum(RegNum);
  return Reg;
}

void TargetX8632::postLower() {
  if (Ctx->getOptLevel() != IceOpt_m1)
    return;
  // TODO: Avoid recomputing WhiteList every instruction.
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
    SizeT VarIndex = 0;
    for (SizeT SrcNum = 0; SrcNum < Inst->getSrcSize(); ++SrcNum) {
      Operand *Src = Inst->getSrc(SrcNum);
      SizeT NumVars = Src->getNumVars();
      for (SizeT J = 0; J < NumVars; ++J, ++VarIndex) {
        const Variable *Var = Src->getVar(J);
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
    SizeT VarIndex = 0;
    for (SizeT SrcNum = 0; SrcNum < Inst->getSrcSize(); ++SrcNum) {
      Operand *Src = Inst->getSrc(SrcNum);
      SizeT NumVars = Src->getNumVars();
      for (SizeT J = 0; J < NumVars; ++J, ++VarIndex) {
        Variable *Var = Src->getVar(J);
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
