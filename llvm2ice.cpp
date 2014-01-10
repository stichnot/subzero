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

static std::string LLVMTypeAsString(const Type *T) {
  std::string TypeName;
  raw_string_ostream N(TypeName);
  T->print(N);
  return N.str();
}

IceType ConvertIntegerType(const IntegerType *IntTy) {
  switch (IntTy->getBitWidth()) {
  case 32:
    return IceType_i32;
  case 64:
    return IceType_i64;
  default:
    report_fatal_error(std::string("Invalid PNaCl int type: ") +
                       LLVMTypeAsString(IntTy));
    return IceType_void;
  }
}

IceType ConvertType(const Type *Ty) {
  switch (Ty->getTypeID()) {
  case Type::VoidTyID:
    return IceType_void;
  case Type::IntegerTyID:
    return ConvertIntegerType(cast<IntegerType>(Ty));
  default:
    report_fatal_error(std::string("Invalid PNaCl type: ") +
                       LLVMTypeAsString(Ty));
  }

  // We should not get here, as the switch above either returns or raises a
  // fatal error.
  assert(0 && "ConvertType unreachable");
  return IceType_void;
}

IceCfgNode *ConvertBasicBlock(const BasicBlock *BB, IceCfg *Cfg) {
  IceCfgNode *Node = new IceCfgNode(Cfg, Cfg->translateLabel(BB->getName()));
  for (BasicBlock::const_iterator II = BB->begin(), II_e = BB->end();
       II != II_e; ++II) {
    switch (II->getOpcode()) {
    case Instruction::Ret:
      const ReturnInst *Ret = cast<ReturnInst>(II);
      const Value *RetVal = Ret->getReturnValue();
      if (RetVal) {
        IceType IceRetTy = ConvertType(RetVal->getType());
        Node->appendInst(new IceInstRet(
            IceRetTy, Cfg->getVariable(IceRetTy, RetVal->getName())));
      } else {
        Node->appendInst(new IceInstRet(IceType_void));
      }
      break;
    }
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
