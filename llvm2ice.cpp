/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "IceCfg.h"
#include "IceCfgNode.h"
#include "IceDefs.h"
#include "IceInst.h"
#include "IceOperand.h"
#include "IceTypes.h"

#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"

using namespace llvm;

// Debugging helper
template <typename T> static std::string LLVMObjectAsString(const T *O) {
  std::string Dump;
  raw_string_ostream Stream(Dump);
  O->print(Stream);
  return Stream.str();
}

// Converter from LLVM to ICE. The entry point is the convertFunction method.
//
// Note: this currently assumes that the given IR was verified to be valid PNaCl
// bitcode:
// https://developers.google.com/native-client/dev/reference/pnacl-bitcode-abi
// If not, all kinds of assertions may fire.
//
class LLVM2ICEConverter {
public:
  LLVM2ICEConverter() : Cfg(NULL) {}

  IceCfg *convertFunction(const Function *F) {
    VariableTranslation.clear();
    LabelTranslation.clear();
    Cfg = new IceCfg;
    Cfg->setName(F->getName());
    Cfg->setReturnType(convertType(F->getReturnType()));

    for (Function::const_arg_iterator ArgI = F->arg_begin(),
                                      ArgE = F->arg_end();
         ArgI != ArgE; ++ArgI) {
      Cfg->addArg(mapValueToIceVar(ArgI));
    }

    for (Function::const_iterator BBI = F->begin(), BBE = F->end(); BBI != BBE;
         ++BBI) {
      convertBasicBlock(BBI);
    }

    Cfg->setEntryNode(mapBasicBlockToNode(&F->getEntryBlock()));
    Cfg->registerEdges();

    return Cfg;
  }

private:
  // LLVM values (instructions, etc.) are mapped directly to ICE variables.
  // mapValueToIceVar has a version that forces an ICE type on the variable,
  // and a version that just uses convertType on V.
  IceVariable *mapValueToIceVar(const Value *V, IceType IceTy) {
    return Cfg->makeVariable(IceTy, VariableTranslation.translate(V),
                             V->getName());
  }

  IceVariable *mapValueToIceVar(const Value *V) {
    return mapValueToIceVar(V, convertType(V->getType()));
  }

  IceCfgNode *mapBasicBlockToNode(const BasicBlock *BB) {
    return Cfg->makeNode(LabelTranslation.translate(BB), BB->getName());
  }

  IceType convertIntegerType(const IntegerType *IntTy) {
    switch (IntTy->getBitWidth()) {
    case 1:
      return IceType_i1;
    case 8:
      return IceType_i8;
    case 16:
      return IceType_i16;
    case 32:
      return IceType_i32;
    case 64:
      return IceType_i64;
    default:
      report_fatal_error(std::string("Invalid PNaCl int type: ") +
                         LLVMObjectAsString(IntTy));
      return IceType_void;
    }
  }

  IceType convertType(const Type *Ty) {
    switch (Ty->getTypeID()) {
    case Type::VoidTyID:
      return IceType_void;
    case Type::IntegerTyID:
      return convertIntegerType(cast<IntegerType>(Ty));
    case Type::FloatTyID:
      return IceType_f32;
    case Type::DoubleTyID:
      return IceType_f64;
    case Type::PointerTyID:
      return IceType_i32;
    default:
      report_fatal_error(std::string("Invalid PNaCl type: ") +
                         LLVMObjectAsString(Ty));
    }

    llvm_unreachable("convertType");
    return IceType_void;
  }

  // Given a LLVM instruction and an operand number, produce the IceOperand this
  // refers to. If there's no such operand, return NULL.
  IceOperand *convertOperand(const Instruction *Inst, unsigned OpNum) {
    if (OpNum >= Inst->getNumOperands()) {
      return NULL;
    }
    const Value *Op = Inst->getOperand(OpNum);
    return convertValue(Op);
  }

  IceOperand *convertValue(const Value *Op) {
    if (const Constant *Const = dyn_cast<Constant>(Op)) {
      // For now only constant integers are supported.
      // TODO: support all kinds of constants
      if (const GlobalValue *GV = dyn_cast<GlobalValue>(Const)) {
        return Cfg->getConstant(IceType_i32, GV, GV->getName());
      } else if (const ConstantInt *CI = cast<ConstantInt>(Const)) {
        return Cfg->getConstant(IceType_i32,
                                static_cast<uint32_t>(CI->getZExtValue()));
      } else {
        assert(0 && "Unhandled constant type");
        return NULL;
      }
    } else {
      return mapValueToIceVar(Op);
    }
  }

  // Note: this currently assumes a 1x1 mapping between LLVM IR and Ice
  // instructions.
  IceInst *convertInstruction(const Instruction *Inst) {
    switch (Inst->getOpcode()) {
    case Instruction::PHI:
      return convertPHINodeInstruction(cast<PHINode>(Inst));
    case Instruction::Br:
      return convertBrInstruction(cast<BranchInst>(Inst));
    case Instruction::Ret:
      return convertRetInstruction(cast<ReturnInst>(Inst));
    case Instruction::IntToPtr:
      return convertIntToPtrInstruction(cast<IntToPtrInst>(Inst));
    case Instruction::ICmp:
      return convertICmpInstruction(cast<ICmpInst>(Inst));
    case Instruction::Load:
      return convertLoadInstruction(cast<LoadInst>(Inst));
    case Instruction::ZExt:
      return convertZExtInstruction(cast<ZExtInst>(Inst));
    case Instruction::Add:
      return convertArithInstruction(Inst, IceInstArithmetic::Add);
    case Instruction::Sub:
      return convertArithInstruction(Inst, IceInstArithmetic::Sub);
    case Instruction::Mul:
      return convertArithInstruction(Inst, IceInstArithmetic::Mul);
    case Instruction::UDiv:
      return convertArithInstruction(Inst, IceInstArithmetic::Udiv);
    case Instruction::SDiv:
      return convertArithInstruction(Inst, IceInstArithmetic::Sdiv);
    case Instruction::URem:
      return convertArithInstruction(Inst, IceInstArithmetic::Urem);
    case Instruction::SRem:
      return convertArithInstruction(Inst, IceInstArithmetic::Srem);
    case Instruction::Shl:
      return convertArithInstruction(Inst, IceInstArithmetic::Shl);
    case Instruction::LShr:
      return convertArithInstruction(Inst, IceInstArithmetic::Lshr);
    case Instruction::AShr:
      return convertArithInstruction(Inst, IceInstArithmetic::Ashr);
    case Instruction::FAdd:
      return convertArithInstruction(Inst, IceInstArithmetic::Fadd);
    case Instruction::FSub:
      return convertArithInstruction(Inst, IceInstArithmetic::Fsub);
    case Instruction::FMul:
      return convertArithInstruction(Inst, IceInstArithmetic::Fmul);
    case Instruction::FDiv:
      return convertArithInstruction(Inst, IceInstArithmetic::Fdiv);
    case Instruction::FRem:
      return convertArithInstruction(Inst, IceInstArithmetic::Frem);
    case Instruction::And:
      return convertArithInstruction(Inst, IceInstArithmetic::And);
    case Instruction::Or:
      return convertArithInstruction(Inst, IceInstArithmetic::Or);
    case Instruction::Xor:
      return convertArithInstruction(Inst, IceInstArithmetic::Xor);
    case Instruction::Call:
      return convertCallInstruction(cast<CallInst>(Inst));
    default:
      report_fatal_error(std::string("Invalid PNaCl instruction: ") +
                         LLVMObjectAsString(Inst));
    }

    llvm_unreachable("convertInstruction");
    return NULL;
  }

  IceInst *convertLoadInstruction(const LoadInst *Inst) {
    IceOperand *Src = convertOperand(Inst, 0);
    assert(Src->getType() == IceType_i32 && "Expecting loads only from i32");
    IceVariable *Dest = mapValueToIceVar(Inst);
    return new IceInstLoad(Cfg, Dest, Src);
  }

  IceInst *convertArithInstruction(const Instruction *Inst,
                                   IceInstArithmetic::IceArithmetic Opcode) {
    const BinaryOperator *BinOp = cast<BinaryOperator>(Inst);
    IceOperand *Src0 = convertOperand(Inst, 0);
    IceOperand *Src1 = convertOperand(Inst, 1);
    IceVariable *Dest = mapValueToIceVar(BinOp);
    return new IceInstArithmetic(Cfg, Opcode, Dest, Src0, Src1);
  }

  IceInst *convertPHINodeInstruction(const PHINode *Inst) {
    IceInstPhi *IcePhi = new IceInstPhi(Cfg, mapValueToIceVar(Inst));
    for (unsigned N = 0, E = Inst->getNumIncomingValues(); N != E; ++N) {
      IcePhi->addArgument(convertOperand(Inst, N),
                          mapBasicBlockToNode(Inst->getIncomingBlock(N)));
    }
    return IcePhi;
  }

  IceInst *convertBrInstruction(const BranchInst *Inst) {
    if (Inst->isConditional()) {
      IceOperand *Src = convertOperand(Inst, 0);
      BasicBlock *BBThen = Inst->getSuccessor(0);
      BasicBlock *BBElse = Inst->getSuccessor(1);
      return new IceInstBr(Cfg, Src, mapBasicBlockToNode(BBThen),
                           mapBasicBlockToNode(BBElse));
    } else {
      BasicBlock *BBSucc = Inst->getSuccessor(0);
      return new IceInstBr(Cfg, mapBasicBlockToNode(BBSucc));
    }
  }

  IceInst *convertIntToPtrInstruction(const IntToPtrInst *Inst) {
    IceOperand *Src = convertOperand(Inst, 0);
    IceVariable *Dest = mapValueToIceVar(Inst, IceType_i32);

    return new IceInstAssign(Cfg, Dest, Src);
  }

  IceInst *convertRetInstruction(const ReturnInst *Inst) {
    IceOperand *RetOperand = convertOperand(Inst, 0);
    if (RetOperand) {
      return new IceInstRet(Cfg, RetOperand);
    } else {
      return new IceInstRet(Cfg);
    }
  }

  IceInst *convertZExtInstruction(const ZExtInst *Inst) { return NULL; }

  IceInst *convertICmpInstruction(const ICmpInst *Inst) {
    IceOperand *Src0 = convertOperand(Inst, 0);
    IceOperand *Src1 = convertOperand(Inst, 1);
    IceVariable *Dest = mapValueToIceVar(Inst);

    IceInstIcmp::IceICond Cond;
    switch (Inst->getPredicate()) {
    default:
      llvm_unreachable("ICmpInst predicate");
    case CmpInst::ICMP_EQ:
      Cond = IceInstIcmp::Eq;
      break;
    case CmpInst::ICMP_NE:
      Cond = IceInstIcmp::Ne;
      break;
    case CmpInst::ICMP_UGT:
      Cond = IceInstIcmp::Ugt;
      break;
    case CmpInst::ICMP_UGE:
      Cond = IceInstIcmp::Uge;
      break;
    case CmpInst::ICMP_ULT:
      Cond = IceInstIcmp::Ult;
      break;
    case CmpInst::ICMP_ULE:
      Cond = IceInstIcmp::Ule;
      break;
    case CmpInst::ICMP_SGT:
      Cond = IceInstIcmp::Sgt;
      break;
    case CmpInst::ICMP_SGE:
      Cond = IceInstIcmp::Sge;
      break;
    case CmpInst::ICMP_SLT:
      Cond = IceInstIcmp::Slt;
      break;
    case CmpInst::ICMP_SLE:
      Cond = IceInstIcmp::Sle;
      break;
    }

    return new IceInstIcmp(Cfg, Cond, Dest, Src0, Src1);
  }

  IceInst *convertCallInstruction(const CallInst *Inst) {
    IceVariable *Dest = mapValueToIceVar(Inst);
    IceOperand *CallTarget = convertValue(Inst->getCalledValue());
    IceInstCall *NewInst =
        new IceInstCall(Cfg, Dest, CallTarget, Inst->isTailCall());
    unsigned NumArgs = Inst->getNumArgOperands();
    for (unsigned i = 0; i < NumArgs; ++i) {
      NewInst->addArg(convertOperand(Inst, i));
    }
    return NewInst;
  }

  IceCfgNode *convertBasicBlock(const BasicBlock *BB) {
    IceCfgNode *Node = mapBasicBlockToNode(BB);
    for (BasicBlock::const_iterator II = BB->begin(), II_e = BB->end();
         II != II_e; ++II) {
      IceInst *Inst = convertInstruction(II);
      Node->appendInst(Inst);
    }
    return Node;
  }

private:
  // Data
  IceCfg *Cfg;
  IceValueTranslation<const Value *> VariableTranslation;
  IceValueTranslation<const BasicBlock *> LabelTranslation;
};

cl::list<IceVerbose> VerboseList(
    "verbose", cl::CommaSeparated,
    cl::desc("Verbose options (can be comma-separated):"),
    cl::values(
        clEnumValN(IceV_Instructions, "inst", "Print basic instructions"),
        clEnumValN(IceV_Deleted, "del", "Include deleted instructions"),
        clEnumValN(IceV_InstNumbers, "instnum", "Print instruction numbers"),
        clEnumValN(IceV_Preds, "pred", "Show predecessors"),
        clEnumValN(IceV_Succs, "succ", "Show successors"),
        clEnumValN(IceV_Liveness, "live", "Liveness information"),
        clEnumValN(IceV_RegManager, "rmgr", "Register manager status"),
        clEnumValN(IceV_RegOrigins, "orig", "Physical register origins"),
        clEnumValN(IceV_LinearScan, "regalloc", "Linear scan details"),
        clEnumValN(IceV_All, "all", "Use all verbose options"),
        clEnumValN(IceV_None, "none", "No verbosity"), clEnumValEnd));
cl::opt<bool> DisableTranslation("notranslate",
                                 cl::desc("Disable Subzero translation"));
cl::opt<IceTargetArch> TargetArch(
    "target", cl::desc("Target architecture:"), cl::init(IceTarget_X8632),
    cl::values(clEnumValN(IceTarget_X8632, "x8632", "x86-32"),
               clEnumValN(IceTarget_X8632_old, "x8632_old",
                          "x86-32 old version"),
               clEnumValN(IceTarget_X8664, "x8664", "x86-64"),
               clEnumValN(IceTarget_ARM32, "arm32", "ARM32"),
               clEnumValN(IceTarget_ARM64, "arm64", "ARM64"), clEnumValEnd));
cl::opt<std::string> IRFilename(cl::Positional, cl::desc("<IR file>"),
                                cl::Required);

int main(int argc, char **argv) {
  cl::ParseCommandLineOptions(argc, argv);

  // Parse the input LLVM IR file into a module.
  SMDiagnostic Err;
  Module *Mod = ParseIRFile(IRFilename, Err, getGlobalContext());
  if (!Mod) {
    Err.print(argv[0], errs());
    return 1;
  }

  IceVerboseMask VerboseMask = IceV_None;
  for (unsigned i = 0; i != VerboseList.size(); ++i)
    VerboseMask |= VerboseList[i];

  for (Module::const_iterator I = Mod->begin(), E = Mod->end(); I != E; ++I) {
    if (I->empty())
      continue;
    LLVM2ICEConverter FunctionConverter;
    IceCfg *Cfg = FunctionConverter.convertFunction(I);
    if (!VerboseList.empty())
      Cfg->Str.setVerbose(VerboseMask);
    Cfg->dump();
    if (!DisableTranslation)
      Cfg->translate(TargetArch);
  }

  return 0;
}
