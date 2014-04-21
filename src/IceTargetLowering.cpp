//===- subzero/src/TargetLowering.cpp - Basic lowering implementation --===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the skeleton of the TargetLowering class,
// specifically invoking the appropriate lowering method for a given
// instruction kind and driving global register allocation.  It also
// implements the non-deleted instruction iteration in
// IceLoweringContext.
//
//===----------------------------------------------------------------------===//

#include "IceCfg.h" // setError()
#include "IceCfgNode.h"
#include "IceOperand.h"
#include "IceRegAlloc.h"
#include "IceTargetLowering.h"
#include "IceTargetLoweringX8632.h"

namespace Ice {

void IceLoweringContext::init(CfgNode *N) {
  Node = N;
  Cur = getNode()->getInsts().begin();
  End = getNode()->getInsts().end();
  skipDeleted(Cur);
  Next = Cur;
  advance(Next);
}

void IceLoweringContext::insert(Inst *Inst) {
  getNode()->getInsts().insert(Next, Inst);
  Inst->updateVars(getNode());
}

void IceLoweringContext::skipDeleted(InstList::iterator &I) {
  while (I != End && (*I)->isDeleted())
    ++I;
}

void IceLoweringContext::advance(InstList::iterator &I) {
  if (I != End) {
    ++I;
    skipDeleted(I);
  }
}

TargetLowering *TargetLowering::createLowering(IceTargetArch Target,
                                               IceCfg *Cfg) {
  // These statements can be #ifdef'd to specialize the code generator
  // to a subset of the available targets.
  if (Target == IceTarget_X8632)
    return TargetX8632::create(Cfg);
#if 0
  if (Target == IceTarget_X8664)
    return IceTargetX8664::create(Cfg);
  if (Target == IceTarget_ARM32)
    return IceTargetARM32::create(Cfg);
  if (Target == IceTarget_ARM64)
    return IceTargetARM64::create(Cfg);
#endif
  Cfg->setError("Unsupported target");
  return NULL;
}

void TargetLowering::doAddressOpt() {
  if (llvm::isa<InstLoad>(*Context.getCur()))
    doAddressOptLoad();
  else if (llvm::isa<InstStore>(*Context.getCur()))
    doAddressOptStore();
  Context.Cur = Context.Next;
  Context.advanceNext();
}

// Lowers a single instruction according to the information in
// Context, by checking the Context.Cur instruction kind and calling
// the appropriate lowering method.  The lowering method should insert
// target instructions at the Cur.Next insertion point, and should not
// delete the Context.Cur instruction or advance Context.Cur.
//
// The lowering method may look ahead in the instruction stream as
// desired, and lower additional instructions in conjunction with the
// current one, for example fusing a compare and branch.  If it does,
// it should advance Context.Cur to point to the next non-deleted
// instruction to process, and it should delete any additional
// instructions it consumes.
void TargetLowering::lower() {
  assert(!Context.atEnd());
  Inst *Inst = *Context.getCur();
  switch (Inst->getKind()) {
  case Inst::Alloca:
    lowerAlloca(llvm::dyn_cast<InstAlloca>(Inst));
    break;
  case Inst::Arithmetic:
    lowerArithmetic(llvm::dyn_cast<InstArithmetic>(Inst));
    break;
  case Inst::Assign:
    lowerAssign(llvm::dyn_cast<InstAssign>(Inst));
    break;
  case Inst::Br:
    lowerBr(llvm::dyn_cast<InstBr>(Inst));
    break;
  case Inst::Call:
    lowerCall(llvm::dyn_cast<InstCall>(Inst));
    break;
  case Inst::Cast:
    lowerCast(llvm::dyn_cast<InstCast>(Inst));
    break;
  case Inst::Fcmp:
    lowerFcmp(llvm::dyn_cast<InstFcmp>(Inst));
    break;
  case Inst::Icmp:
    lowerIcmp(llvm::dyn_cast<InstIcmp>(Inst));
    break;
  case Inst::Load:
    lowerLoad(llvm::dyn_cast<InstLoad>(Inst));
    break;
  case Inst::Phi:
    lowerPhi(llvm::dyn_cast<InstPhi>(Inst));
    break;
  case Inst::Ret:
    lowerRet(llvm::dyn_cast<InstRet>(Inst));
    break;
  case Inst::Select:
    lowerSelect(llvm::dyn_cast<InstSelect>(Inst));
    break;
  case Inst::Store:
    lowerStore(llvm::dyn_cast<InstStore>(Inst));
    break;
  case Inst::Switch:
    lowerSwitch(llvm::dyn_cast<InstSwitch>(Inst));
    break;
  case Inst::Unreachable:
    lowerUnreachable(llvm::dyn_cast<InstUnreachable>(Inst));
    break;
  case Inst::FakeDef:
  case Inst::FakeUse:
  case Inst::FakeKill:
  case Inst::Target:
    // These are all Target instruction types and shouldn't be
    // encountered at this stage.
    Cfg->setError("Can't lower unsupported instruction type");
    break;
  }
  Inst->setDeleted();

  postLower();

  Context.Cur = Context.Next;
  Context.advanceNext();
}

// Drives register allocation, allowing all physical registers (except
// perhaps for the frame pointer) to be allocated.  This set of
// registers could potentially be parameterized if we want to restrict
// registers e.g. for performance testing.
void TargetLowering::regAlloc() {
  LinearScan LinearScan(Cfg);
  RegSetMask RegInclude = RegSet_None;
  RegSetMask RegExclude = RegSet_None;
  RegInclude |= RegSet_CallerSave;
  RegInclude |= RegSet_CalleeSave;
  RegExclude |= RegSet_StackPointer;
  if (hasFramePointer())
    RegExclude |= RegSet_FramePointer;
  llvm::SmallBitVector RegMask = getRegisterSet(RegInclude, RegExclude);
  LinearScan.scan(RegMask);
}

} // end of namespace Ice
