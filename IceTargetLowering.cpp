/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "IceTargetLowering.h"
#include "IceInstX8632.h"

IceTargetLowering *IceTargetLowering::createLowering(IceTargetArch Target,
                                                     IceCfg *Cfg) {
  // These statements can be #ifdef'd to specialize the code generator
  // to a subset of the available targets.
  if (Target == IceTarget_X8632_old)
    return new IceTargetX8632(Cfg);
  if (Target == IceTarget_X8632)
    return new IceTargetX8632S(Cfg);
#if 0
  if (Target == IceTarget_X8664)
    return new IceTargetX8664(Cfg);
  if (Target == IceTarget_ARM32)
    return new IceTargetARM32(Cfg);
  if (Target == IceTarget_ARM64)
    return new IceTargetARM64(Cfg);
#endif
  assert("Unsupported Target" && 0);
  return NULL;
}

IceInstList IceTargetLowering::lower(const IceInst *Inst, const IceInst *Next,
                                     bool &DeleteNextInst) {
  IceInstList Expansion;
  if (const IceInstAlloca *I = llvm::dyn_cast<const IceInstAlloca>(Inst))
    Expansion = lowerAlloca(I, Next, DeleteNextInst);
  else if (const IceInstArithmetic *I =
               llvm::dyn_cast<const IceInstArithmetic>(Inst))
    Expansion = lowerArithmetic(I, Next, DeleteNextInst);
  else if (const IceInstAssign *I = llvm::dyn_cast<const IceInstAssign>(Inst))
    Expansion = lowerAssign(I, Next, DeleteNextInst);
  else if (const IceInstBr *I = llvm::dyn_cast<const IceInstBr>(Inst))
    Expansion = lowerBr(I, Next, DeleteNextInst);
  else if (const IceInstCall *I = llvm::dyn_cast<const IceInstCall>(Inst))
    Expansion = lowerCall(I, Next, DeleteNextInst);
  else if (const IceInstCast *I = llvm::dyn_cast<const IceInstCast>(Inst))
    Expansion = lowerCast(I, Next, DeleteNextInst);
  else if (const IceInstFcmp *I = llvm::dyn_cast<const IceInstFcmp>(Inst))
    Expansion = lowerFcmp(I, Next, DeleteNextInst);
  else if (const IceInstIcmp *I = llvm::dyn_cast<const IceInstIcmp>(Inst))
    Expansion = lowerIcmp(I, Next, DeleteNextInst);
  else if (const IceInstLoad *I = llvm::dyn_cast<const IceInstLoad>(Inst))
    Expansion = lowerLoad(I, Next, DeleteNextInst);
  else if (const IceInstRet *I = llvm::dyn_cast<const IceInstRet>(Inst))
    Expansion = lowerRet(I, Next, DeleteNextInst);
  else if (const IceInstSelect *I = llvm::dyn_cast<const IceInstSelect>(Inst))
    Expansion = lowerSelect(I, Next, DeleteNextInst);
  else if (const IceInstStore *I = llvm::dyn_cast<const IceInstStore>(Inst))
    Expansion = lowerStore(I, Next, DeleteNextInst);
  else if (const IceInstSwitch *I = llvm::dyn_cast<const IceInstSwitch>(Inst))
    Expansion = lowerSwitch(I, Next, DeleteNextInst);
  else
    assert(0);
  return Expansion;
}
