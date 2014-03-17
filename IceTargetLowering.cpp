/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "IceCfg.h" // setError()
#include "IceCfgNode.h"
#include "IceTargetLowering.h"
#include "IceTargetLoweringX8632.h"

IceLoweringContext::IceLoweringContext(IceCfgNode *Node)
    : Node(Node), Insts(Node->getInsts()), Cur(Insts.begin()),
      End(Insts.end()) {
  skipDeleted(Cur);
  Next = Cur;
  advance(Next);
}

void IceLoweringContext::skipDeleted(IceInstList::iterator &I) {
  while (I != End && (*I)->isDeleted())
    ++I;
}

void IceLoweringContext::advance(IceInstList::iterator &I) {
  if (I != End) {
    ++I;
    skipDeleted(I);
  }
}

IceTargetLowering *IceTargetLowering::createLowering(IceTargetArch Target,
                                                     IceCfg *Cfg) {
  // These statements can be #ifdef'd to specialize the code generator
  // to a subset of the available targets.
  if (Target == IceTarget_X8632)
    return IceTargetX8632::create(Cfg);
  if (Target == IceTarget_X8632Fast)
    return IceTargetX8632Fast::create(Cfg);
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

void IceTargetLowering::doAddressOpt(IceLoweringContext &Context) {
  if (llvm::isa<IceInstLoad>(*Context.Cur))
    doAddressOptLoad(Context);
  else if (llvm::isa<IceInstStore>(*Context.Cur))
    doAddressOptStore(Context);
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
void IceTargetLowering::lower(IceLoweringContext &Context) {
  assert(Context.Cur != Context.End);
  IceInstList::iterator Cur = Context.Cur;
  IceInst *Inst = *Cur;
  switch (Inst->getKind()) {
  case IceInst::Alloca:
    lowerAlloca(llvm::dyn_cast<IceInstAlloca>(Inst), Context);
    break;
  case IceInst::Arithmetic:
    lowerArithmetic(llvm::dyn_cast<IceInstArithmetic>(Inst), Context);
    break;
  case IceInst::Assign:
    lowerAssign(llvm::dyn_cast<IceInstAssign>(Inst), Context);
    break;
  case IceInst::Br:
    lowerBr(llvm::dyn_cast<IceInstBr>(Inst), Context);
    break;
  case IceInst::Call:
    lowerCall(llvm::dyn_cast<IceInstCall>(Inst), Context);
    break;
  case IceInst::Cast:
    lowerCast(llvm::dyn_cast<IceInstCast>(Inst), Context);
    break;
  case IceInst::Fcmp:
    lowerFcmp(llvm::dyn_cast<IceInstFcmp>(Inst), Context);
    break;
  case IceInst::Icmp:
    lowerIcmp(llvm::dyn_cast<IceInstIcmp>(Inst), Context);
    break;
  case IceInst::Load:
    lowerLoad(llvm::dyn_cast<IceInstLoad>(Inst), Context);
    break;
  case IceInst::Phi:
    lowerPhi(llvm::dyn_cast<IceInstPhi>(Inst), Context);
    break;
  case IceInst::Ret:
    lowerRet(llvm::dyn_cast<IceInstRet>(Inst), Context);
    break;
  case IceInst::Select:
    lowerSelect(llvm::dyn_cast<IceInstSelect>(Inst), Context);
    break;
  case IceInst::Store:
    lowerStore(llvm::dyn_cast<IceInstStore>(Inst), Context);
    break;
  case IceInst::Switch:
    lowerSwitch(llvm::dyn_cast<IceInstSwitch>(Inst), Context);
    break;
  case IceInst::FakeDef:
  case IceInst::FakeUse:
  case IceInst::FakeKill:
  case IceInst::Target:
    // These are all Target instruction types and shouldn't be
    // encountered at this stage.
    Cfg->setError("Can't lower unsupported instruction type");
    break;
  }
  Inst->setDeleted();

  postLower(Context);

  Context.Cur = Context.Next;
  Context.advanceNext();
}
