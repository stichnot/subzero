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
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Support/SourceMgr.h"

#include <fstream>
#include <iostream>

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
  LLVM2ICEConverter() : Cfg(NULL), CurrentNode(NULL) {}

  IceCfg *convertFunction(const Function *F) {
    VarMap.clear();
    NodeMap.clear();
    Cfg = new IceCfg;
    Cfg->setName(F->getName());
    Cfg->setReturnType(convertType(F->getReturnType()));
    Cfg->setInternal(F->hasInternalLinkage());

    // The initial definition/use of each arg is the entry node.
    CurrentNode = mapBasicBlockToNode(&F->getEntryBlock());
    for (Function::const_arg_iterator ArgI = F->arg_begin(),
                                      ArgE = F->arg_end();
         ArgI != ArgE; ++ArgI) {
      Cfg->addArg(mapValueToIceVar(ArgI));
    }

    for (Function::const_iterator BBI = F->begin(), BBE = F->end(); BBI != BBE;
         ++BBI) {
      mapBasicBlockToNode(BBI);
    }
    for (Function::const_iterator BBI = F->begin(), BBE = F->end(); BBI != BBE;
         ++BBI) {
      CurrentNode = mapBasicBlockToNode(BBI);
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
    if (IceTy == IceType_void)
      return NULL;
    if (VarMap.find(V) == VarMap.end()) {
      assert(CurrentNode);
      VarMap[V] = Cfg->makeVariable(IceTy, CurrentNode, V->getName());
    }
    return VarMap[V];
  }

  IceVariable *mapValueToIceVar(const Value *V) {
    return mapValueToIceVar(V, convertType(V->getType()));
  }

  IceCfgNode *mapBasicBlockToNode(const BasicBlock *BB) {
    if (NodeMap.find(BB) == NodeMap.end()) {
      NodeMap[BB] = Cfg->makeNode(BB->getName());
    }
    return NodeMap[BB];
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
    case Type::PointerTyID: {
      const PointerType *PTy = cast<PointerType>(Ty);
      return convertType(PTy->getElementType());
      return IceType_i32;
    }
    case Type::FunctionTyID:
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
        return Cfg->getConstantSym(convertType(GV->getType()), GV, 0,
                                   GV->getName());
      } else if (const ConstantInt *CI = dyn_cast<ConstantInt>(Const)) {
        return Cfg->getConstantInt(convertIntegerType(CI->getType()),
                                   CI->getZExtValue());
      } else if (const ConstantFP *CFP = dyn_cast<ConstantFP>(Const)) {
        IceType Type = convertType(CFP->getType());
        if (Type == IceType_f32)
          return Cfg->getConstantFloat(CFP->getValueAPF().convertToFloat());
        else if (Type == IceType_f64)
          return Cfg->getConstantDouble(CFP->getValueAPF().convertToDouble());
        assert(0 && "Unexpected floating point type");
        return NULL;
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
    case Instruction::FCmp:
      return convertFCmpInstruction(cast<FCmpInst>(Inst));
    case Instruction::Select:
      return convertSelectInstruction(cast<SelectInst>(Inst));
    case Instruction::Switch:
      return convertSwitchInstruction(cast<SwitchInst>(Inst));
    case Instruction::Load:
      return convertLoadInstruction(cast<LoadInst>(Inst));
    case Instruction::Store:
      return convertStoreInstruction(cast<StoreInst>(Inst));
    case Instruction::ZExt:
      return convertCastInstruction(cast<ZExtInst>(Inst), IceInstCast::Zext);
    case Instruction::SExt:
      return convertCastInstruction(cast<SExtInst>(Inst), IceInstCast::Sext);
    case Instruction::Trunc:
      return convertCastInstruction(cast<TruncInst>(Inst), IceInstCast::Trunc);
    case Instruction::FPTrunc:
      return convertCastInstruction(cast<FPTruncInst>(Inst),
                                    IceInstCast::Fptrunc);
    case Instruction::FPExt:
      return convertCastInstruction(cast<FPExtInst>(Inst), IceInstCast::Fpext);
    case Instruction::FPToSI:
      return convertCastInstruction(cast<FPToSIInst>(Inst),
                                    IceInstCast::Fptosi);
    case Instruction::FPToUI:
      return convertCastInstruction(cast<FPToUIInst>(Inst),
                                    IceInstCast::Fptoui);
    case Instruction::SIToFP:
      return convertCastInstruction(cast<SIToFPInst>(Inst),
                                    IceInstCast::Sitofp);
    case Instruction::UIToFP:
      return convertCastInstruction(cast<UIToFPInst>(Inst),
                                    IceInstCast::Uitofp);
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
    case Instruction::Alloca:
      return convertAllocaInstruction(cast<AllocaInst>(Inst));
    default:
      report_fatal_error(std::string("Invalid PNaCl instruction: ") +
                         LLVMObjectAsString(Inst));
    }

    llvm_unreachable("convertInstruction");
    return NULL;
  }

  IceInst *convertLoadInstruction(const LoadInst *Inst) {
    IceOperand *Src = convertOperand(Inst, 0);
    // assert(Src->getType() == IceType_i32 && "Expecting loads only from i32");
    IceVariable *Dest = mapValueToIceVar(Inst);
    return IceInstLoad::create(Cfg, Dest, Src);
  }

  IceInst *convertStoreInstruction(const StoreInst *Inst) {
    IceOperand *Addr = convertOperand(Inst, 1);
    // assert(Addr->getType() == IceType_i32 && "Expecting stores only from
    // i32");
    IceOperand *Val = convertOperand(Inst, 0);
    return IceInstStore::create(Cfg, Val, Addr);
  }

  IceInst *convertArithInstruction(const Instruction *Inst,
                                   IceInstArithmetic::OpKind Opcode) {
    const BinaryOperator *BinOp = cast<BinaryOperator>(Inst);
    IceOperand *Src0 = convertOperand(Inst, 0);
    IceOperand *Src1 = convertOperand(Inst, 1);
    IceVariable *Dest = mapValueToIceVar(BinOp);
    return IceInstArithmetic::create(Cfg, Opcode, Dest, Src0, Src1);
  }

  IceInst *convertPHINodeInstruction(const PHINode *Inst) {
    unsigned NumValues = Inst->getNumIncomingValues();
    IceInstPhi *IcePhi =
        IceInstPhi::create(Cfg, NumValues, mapValueToIceVar(Inst));
    for (unsigned N = 0, E = NumValues; N != E; ++N) {
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
      IceCfgNode *NodeThen = mapBasicBlockToNode(BBThen);
      IceCfgNode *NodeElse = mapBasicBlockToNode(BBElse);
      return IceInstBr::create(Cfg, Src, NodeThen, NodeElse);
    } else {
      BasicBlock *BBSucc = Inst->getSuccessor(0);
      return IceInstBr::create(Cfg, mapBasicBlockToNode(BBSucc));
    }
  }

  IceInst *convertIntToPtrInstruction(const IntToPtrInst *Inst) {
    IceOperand *Src = convertOperand(Inst, 0);
    IceVariable *Dest = mapValueToIceVar(Inst, IceType_i32);

    return IceInstAssign::create(Cfg, Dest, Src);
  }

  IceInst *convertRetInstruction(const ReturnInst *Inst) {
    IceOperand *RetOperand = convertOperand(Inst, 0);
    if (RetOperand) {
      return IceInstRet::create(Cfg, RetOperand);
    } else {
      return IceInstRet::create(Cfg);
    }
  }

  IceInst *convertCastInstruction(const Instruction *Inst,
                                  IceInstCast::OpKind CastKind) {
    IceOperand *Src = convertOperand(Inst, 0);
    IceVariable *Dest = mapValueToIceVar(Inst);
    return IceInstCast::create(Cfg, CastKind, Dest, Src);
  }

  IceInst *convertICmpInstruction(const ICmpInst *Inst) {
    IceOperand *Src0 = convertOperand(Inst, 0);
    IceOperand *Src1 = convertOperand(Inst, 1);
    IceVariable *Dest = mapValueToIceVar(Inst);

    IceInstIcmp::ICond Cond;
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

    return IceInstIcmp::create(Cfg, Cond, Dest, Src0, Src1);
  }

  IceInst *convertFCmpInstruction(const FCmpInst *Inst) {
    IceOperand *Src0 = convertOperand(Inst, 0);
    IceOperand *Src1 = convertOperand(Inst, 1);
    IceVariable *Dest = mapValueToIceVar(Inst);

    IceInstFcmp::FCond Cond;
    switch (Inst->getPredicate()) {

    default:
      llvm_unreachable("FCmpInst predicate");

    case CmpInst::FCMP_FALSE:
      Cond = IceInstFcmp::False;
      break;
    case CmpInst::FCMP_OEQ:
      Cond = IceInstFcmp::Oeq;
      break;
    case CmpInst::FCMP_OGT:
      Cond = IceInstFcmp::Ogt;
      break;
    case CmpInst::FCMP_OGE:
      Cond = IceInstFcmp::Oge;
      break;
    case CmpInst::FCMP_OLT:
      Cond = IceInstFcmp::Olt;
      break;
    case CmpInst::FCMP_OLE:
      Cond = IceInstFcmp::Ole;
      break;
    case CmpInst::FCMP_ONE:
      Cond = IceInstFcmp::One;
      break;
    case CmpInst::FCMP_ORD:
      Cond = IceInstFcmp::Ord;
      break;
    case CmpInst::FCMP_UEQ:
      Cond = IceInstFcmp::Ueq;
      break;
    case CmpInst::FCMP_UGT:
      Cond = IceInstFcmp::Ugt;
      break;
    case CmpInst::FCMP_UGE:
      Cond = IceInstFcmp::Uge;
      break;
    case CmpInst::FCMP_ULT:
      Cond = IceInstFcmp::Ult;
      break;
    case CmpInst::FCMP_ULE:
      Cond = IceInstFcmp::Ule;
      break;
    case CmpInst::FCMP_UNE:
      Cond = IceInstFcmp::Une;
      break;
    case CmpInst::FCMP_UNO:
      Cond = IceInstFcmp::Uno;
      break;
    case CmpInst::FCMP_TRUE:
      Cond = IceInstFcmp::True;
      break;
    }

    return IceInstFcmp::create(Cfg, Cond, Dest, Src0, Src1);
  }

  IceInst *convertSelectInstruction(const SelectInst *Inst) {
    IceVariable *Dest = mapValueToIceVar(Inst);
    IceOperand *Cond = convertValue(Inst->getCondition());
    IceOperand *Source1 = convertValue(Inst->getTrueValue());
    IceOperand *Source2 = convertValue(Inst->getFalseValue());
    return IceInstSelect::create(Cfg, Dest, Cond, Source1, Source2);
  }

  IceInst *convertSwitchInstruction(const SwitchInst *Inst) {
    IceOperand *Source = convertValue(Inst->getCondition());
    IceCfgNode *LabelDefault = mapBasicBlockToNode(Inst->getDefaultDest());
    unsigned NumCases = Inst->getNumCases();
    IceInstSwitch *Switch =
        IceInstSwitch::create(Cfg, NumCases, Source, LabelDefault);
    unsigned CurrentCase = 0;
    for (SwitchInst::ConstCaseIt I = Inst->case_begin(), E = Inst->case_end();
         I != E; ++I, ++CurrentCase) {
      uint64_t CaseValue = I.getCaseValue()->getZExtValue();
      IceCfgNode *CaseSuccessor = mapBasicBlockToNode(I.getCaseSuccessor());
      Switch->addBranch(CurrentCase, CaseValue, CaseSuccessor);
    }
    return Switch;
  }

  IceInst *convertCallInstruction(const CallInst *Inst) {
    IceVariable *Dest = mapValueToIceVar(Inst);
    IceOperand *CallTarget = convertValue(Inst->getCalledValue());
    unsigned NumArgs = Inst->getNumArgOperands();
    IceInstCall *NewInst =
        IceInstCall::create(Cfg, NumArgs, Dest, CallTarget, Inst->isTailCall());
    for (unsigned i = 0; i < NumArgs; ++i) {
      NewInst->addArg(convertOperand(Inst, i));
    }
    return NewInst;
  }

  IceInst *convertAllocaInstruction(const AllocaInst *Inst) {
    // PNaCl bitcode only contains allocas of byte-granular objects.
    IceOperand *ByteCount = convertValue(Inst->getArraySize());
    uint32_t Align = Inst->getAlignment();
    IceVariable *Dest = mapValueToIceVar(Inst, IceType_i32);

    return IceInstAlloca::create(Cfg, ByteCount, Align, Dest);
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
  IceCfgNode *CurrentNode;
  std::map<const Value *, IceVariable *> VarMap;
  std::map<const BasicBlock *, IceCfgNode *> NodeMap;
};

static cl::list<IceVerbose> VerboseList(
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
        clEnumValN(IceV_Frame, "frame", "Stack frame layout details"),
        clEnumValN(IceV_Timing, "time", "Pass timing details"),
        clEnumValN(IceV_All, "all", "Use all verbose options"),
        clEnumValN(IceV_None, "none", "No verbosity"), clEnumValEnd));
static cl::opt<bool>
DisableTranslation("notranslate", cl::desc("Disable Subzero translation"));
static cl::opt<IceTargetArch> TargetArch(
    "target", cl::desc("Target architecture:"), cl::init(IceTarget_X8632),
    cl::values(clEnumValN(IceTarget_X8632, "x8632", "x86-32"),
               clEnumValN(IceTarget_X8632Fast, "x8632fast", "x86-32 fast"),
               clEnumValN(IceTarget_X8664, "x8664", "x86-64"),
               clEnumValN(IceTarget_ARM32, "arm32", "ARM32"),
               clEnumValN(IceTarget_ARM64, "arm64", "ARM64"), clEnumValEnd));
static cl::opt<std::string> IRFilename(cl::Positional, cl::desc("<IR file>"),
                                       cl::Required);
static cl::opt<std::string> OutputFilename("o",
                                           cl::desc("Override output filename"),
                                           cl::init("-"),
                                           cl::value_desc("filename"));
static cl::opt<std::string>
TestPrefix("prefix", cl::desc("Prepend a prefix to symbol names for testing"),
           cl::init(""), cl::value_desc("prefix"));
static cl::opt<bool>
DisableInternal("external",
                cl::desc("Disable 'internal' linkage type for testing"));

static cl::opt<bool> SubzeroTimingEnabled(
    "timing", cl::desc("Enable breakdown timing of Subzero translation"));

int main(int argc, char **argv) {
  cl::ParseCommandLineOptions(argc, argv);

  // Parse the input LLVM IR file into a module.
  SMDiagnostic Err;
  Module *Mod;

  {
    IceTimer T;
    Mod = ParseIRFile(IRFilename, Err, getGlobalContext());

    if (SubzeroTimingEnabled) {
      std::cerr << "[Subzero timing] IR Parsing: " << T.getElapsedSec()
                << " sec\n";
    }
  }

  if (!Mod) {
    Err.print(argv[0], errs());
    return 1;
  }

  IceVerboseMask VerboseMask = IceV_None;
  for (unsigned i = 0; i != VerboseList.size(); ++i)
    VerboseMask |= VerboseList[i];

  std::ofstream Ofs;
  if (OutputFilename != "-") {
    Ofs.open(OutputFilename.c_str(), std::ofstream::out);
  }
  raw_os_ostream *Os =
      new raw_os_ostream(OutputFilename == "-" ? std::cout : Ofs);
  Os->SetUnbuffered();

  for (Module::const_iterator I = Mod->begin(), E = Mod->end(); I != E; ++I) {
    if (I->empty())
      continue;
    LLVM2ICEConverter FunctionConverter;

    IceTimer TConvert;
    IceCfg *Cfg = FunctionConverter.convertFunction(I);
    if (DisableInternal)
      Cfg->setInternal(false);

    if (SubzeroTimingEnabled) {
      std::cerr << "[Subzero timing] Convert function " << Cfg->getName()
                << ": " << TConvert.getElapsedSec() << " sec\n";
    }

    Cfg->setTestPrefix(TestPrefix);
    Cfg->Str.Stream = Os;
    Cfg->Str.setVerbose(VerboseMask);
    if (!DisableTranslation) {
      IceTimer TTranslate;
      Cfg->translate(TargetArch);
      if (SubzeroTimingEnabled) {
        std::cerr << "[Subzero timing] Translate function " << Cfg->getName()
                  << ": " << TTranslate.getElapsedSec() << " sec\n";
      }
      if (Cfg->hasError()) {
        errs() << "ICE translation error: " << Cfg->getError() << "\n";
      }
      uint32_t AsmFormat = 0;

      IceTimer TEmit;
      Cfg->emit(AsmFormat);
      if (SubzeroTimingEnabled) {
        std::cerr << "[Subzero timing] Emit function " << Cfg->getName() << ": "
                  << TEmit.getElapsedSec() << " sec\n";
      }
    }
  }

  return 0;
}
