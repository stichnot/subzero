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
  if (const IceInstAlloca *I = llvm::dyn_cast<const IceInstAlloca>(Inst))
    return lowerAlloca(I, Next, DeleteNextInst);
  if (const IceInstArithmetic *I =
          llvm::dyn_cast<const IceInstArithmetic>(Inst))
    return lowerArithmetic(I, Next, DeleteNextInst);
  if (const IceInstAssign *I = llvm::dyn_cast<const IceInstAssign>(Inst))
    return lowerAssign(I, Next, DeleteNextInst);
  if (const IceInstBr *I = llvm::dyn_cast<const IceInstBr>(Inst))
    return lowerBr(I, Next, DeleteNextInst);
  if (const IceInstCall *I = llvm::dyn_cast<const IceInstCall>(Inst))
    return lowerCall(I, Next, DeleteNextInst);
  if (const IceInstCast *I = llvm::dyn_cast<const IceInstCast>(Inst))
    return lowerCast(I, Next, DeleteNextInst);
  if (const IceInstFcmp *I = llvm::dyn_cast<const IceInstFcmp>(Inst))
    return lowerFcmp(I, Next, DeleteNextInst);
  if (const IceInstIcmp *I = llvm::dyn_cast<const IceInstIcmp>(Inst))
    return lowerIcmp(I, Next, DeleteNextInst);
  if (const IceInstLoad *I = llvm::dyn_cast<const IceInstLoad>(Inst))
    return lowerLoad(I, Next, DeleteNextInst);
  if (const IceInstRet *I = llvm::dyn_cast<const IceInstRet>(Inst))
    return lowerRet(I, Next, DeleteNextInst);
  if (const IceInstSelect *I = llvm::dyn_cast<const IceInstSelect>(Inst))
    return lowerSelect(I, Next, DeleteNextInst);
  if (const IceInstStore *I = llvm::dyn_cast<const IceInstStore>(Inst))
    return lowerStore(I, Next, DeleteNextInst);
  if (const IceInstSwitch *I = llvm::dyn_cast<const IceInstSwitch>(Inst))
    return lowerSwitch(I, Next, DeleteNextInst);

  Cfg->setError("Can't lower unsupported instruction type");
  return IceInstList();
}
