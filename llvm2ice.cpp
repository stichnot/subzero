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

#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"

using namespace llvm;

// Debugging helper
template <typename T>
static std::string LLVMObjectAsString(const T *O) {
  std::string Dump;
  raw_string_ostream Stream(Dump);
  O->print(Stream);
  return Stream.str();
}

// The Convert* functions provide conversion from LLVM IR to Ice. The entry
// point is ConvertFunction, which creates a new IceCfg from a given
// llvm::Function.
//
// Note: this currently assumes that the given IR was verified to be valid PNaCl
// bitcode:
// https://developers.google.com/native-client/dev/reference/pnacl-bitcode-abi
// If not, all kinds of assertions may fire.
//
IceType ConvertIntegerType(const IntegerType *IntTy) {
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

IceType ConvertType(const Type *Ty) {
  switch (Ty->getTypeID()) {
  case Type::VoidTyID:
    return IceType_void;
  case Type::IntegerTyID:
    return ConvertIntegerType(cast<IntegerType>(Ty));
  case Type::FloatTyID:
    return IceType_f32;
  case Type::DoubleTyID:
    return IceType_f64;
  default:
    report_fatal_error(std::string("Invalid PNaCl type: ") +
                       LLVMObjectAsString(Ty));
  }

  llvm_unreachable("ConvertType");
  return IceType_void;
}

// Note: this currently assumes a 1x1 mapping between LLVM IR and Ice
// instructions.
IceInst *ConvertInstruction(const Instruction *Inst, IceCfg *Cfg) {
  // TODO: the reliance on getName here is fishy. LLVM Values are uniquely
  // identified by their address, not their name (there might be no name!);
  // think this through and rewrite as needed.
  switch (Inst->getOpcode()) {
  case Instruction::Ret: {
    const ReturnInst *Ret = cast<ReturnInst>(Inst);
    const Value *RetVal = Ret->getReturnValue();
    if (RetVal) {
      IceType IceRetTy = ConvertType(RetVal->getType());
      return new IceInstRet(Cfg, IceRetTy,
                            Cfg->getVariable(IceRetTy, RetVal->getName()));
    } else {
      return new IceInstRet(Cfg, IceType_void);
    }
    break;
  }
  case Instruction::Add: {
    const BinaryOperator *BinOp = cast<BinaryOperator>(Inst);
    IceType IceTy = ConvertType(BinOp->getType());
    Value *Op0 = BinOp->getOperand(0);
    Value *Op1 = BinOp->getOperand(1);
    IceOperand *Src0 = Cfg->getVariable(IceTy, Op0->getName());
    IceOperand *Src1 = Cfg->getVariable(IceTy, Op1->getName());
    IceVariable *Dest = Cfg->getVariable(IceTy, BinOp->getName());
    return new IceInstArithmetic(Cfg, IceInstArithmetic::Add, IceTy, Dest, Src0,
                                 Src1);
    break;
  }
  default:
    report_fatal_error(std::string("Invalid PNaCl instruction: ") +
                       LLVMObjectAsString(Inst));
  }

  llvm_unreachable("ConvertInstruction");
  return NULL;
}

IceCfgNode *ConvertBasicBlock(const BasicBlock *BB, IceCfg *Cfg) {
  IceCfgNode *Node = new IceCfgNode(Cfg, Cfg->translateLabel(BB->getName()));
  for (BasicBlock::const_iterator II = BB->begin(), II_e = BB->end();
       II != II_e; ++II) {
    IceInst *Inst = ConvertInstruction(II, Cfg);
    Node->appendInst(Inst);
  }
  return Node;
}

IceCfg *ConvertFunction(const Function *F) {
  IceCfg *Cfg = new IceCfg;
  Cfg->setName(F->getName());
  Cfg->setReturnType(ConvertType(F->getReturnType()));

  for (Function::const_arg_iterator ArgI = F->arg_begin(), ArgE = F->arg_end();
       ArgI != ArgE; ++ArgI) {
    IceType ArgType = ConvertType(ArgI->getType());
    Cfg->addArg(Cfg->getVariable(ArgType, ArgI->getName()));
  }

  const BasicBlock &EntryBB = F->getEntryBlock();
  IceCfgNode *EntryNode = ConvertBasicBlock(&EntryBB, Cfg);
  Cfg->setEntryNode(EntryNode);

  return Cfg;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    errs() << "Usage: " << argv[0] << " <IR file>\n";
    return 1;
  }

  // Parse the input LLVM IR file into a module.
  SMDiagnostic Err;
  Module *Mod = ParseIRFile(argv[1], Err, getGlobalContext());
  if (!Mod) {
    Err.print(argv[0], errs());
    return 1;
  }

  Mod->dump();

  outs() << "==== converting to ICE ====\n";

  for (Module::const_iterator I = Mod->begin(), E = Mod->end(); I != E; ++I) {
    IceCfg *Cfg = ConvertFunction(I);
    Cfg->dump();
  }

  return 0;
}
