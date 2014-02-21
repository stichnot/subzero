/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "IceCfg.h" // setError()
#include "IceTargetLowering.h"
#include "IceTargetLoweringX8632.h"

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

IceInstList IceTargetLowering::doAddressOpt(const IceInst *Inst) {
  if (const IceInstLoad *I = llvm::dyn_cast<const IceInstLoad>(Inst))
    return doAddressOptLoad(I);
  if (const IceInstStore *I = llvm::dyn_cast<const IceInstStore>(Inst))
    return doAddressOptStore(I);
  return IceInstList();
}

IceInstList IceTargetLowering::lower(const IceInst *Inst, const IceInst *Next,
                                     bool &DeleteNextInst) {
  IceInstList Expansion;
  switch (Inst->getKind()) {
  case IceInst::Alloca:
    Expansion =
        lowerAlloca(llvm::dyn_cast<IceInstAlloca>(Inst), Next, DeleteNextInst);
    break;
  case IceInst::Arithmetic:
    Expansion = lowerArithmetic(llvm::dyn_cast<IceInstArithmetic>(Inst), Next,
                                DeleteNextInst);
    break;
  case IceInst::Assign:
    Expansion =
        lowerAssign(llvm::dyn_cast<IceInstAssign>(Inst), Next, DeleteNextInst);
    break;
  case IceInst::Br:
    Expansion = lowerBr(llvm::dyn_cast<IceInstBr>(Inst), Next, DeleteNextInst);
    break;
  case IceInst::Call:
    Expansion =
        lowerCall(llvm::dyn_cast<IceInstCall>(Inst), Next, DeleteNextInst);
    break;
  case IceInst::Cast:
    Expansion =
        lowerCast(llvm::dyn_cast<IceInstCast>(Inst), Next, DeleteNextInst);
    break;
  case IceInst::Fcmp:
    Expansion =
        lowerFcmp(llvm::dyn_cast<IceInstFcmp>(Inst), Next, DeleteNextInst);
    break;
  case IceInst::Icmp:
    Expansion =
        lowerIcmp(llvm::dyn_cast<IceInstIcmp>(Inst), Next, DeleteNextInst);
    break;
  case IceInst::Load:
    Expansion =
        lowerLoad(llvm::dyn_cast<IceInstLoad>(Inst), Next, DeleteNextInst);
    break;
  case IceInst::Phi:
    Expansion =
        lowerPhi(llvm::dyn_cast<IceInstPhi>(Inst), Next, DeleteNextInst);
    break;
  case IceInst::Ret:
    Expansion =
        lowerRet(llvm::dyn_cast<IceInstRet>(Inst), Next, DeleteNextInst);
    break;
  case IceInst::Select:
    Expansion =
        lowerSelect(llvm::dyn_cast<IceInstSelect>(Inst), Next, DeleteNextInst);
    break;
  case IceInst::Store:
    Expansion =
        lowerStore(llvm::dyn_cast<IceInstStore>(Inst), Next, DeleteNextInst);
    break;
  case IceInst::Switch:
    Expansion =
        lowerSwitch(llvm::dyn_cast<IceInstSwitch>(Inst), Next, DeleteNextInst);
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

  postLower(Expansion);
  return Expansion;
}
