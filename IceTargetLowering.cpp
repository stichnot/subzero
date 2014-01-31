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
  switch (Inst->getKind()) {
  case IceInst::Alloca:
    Expansion = lowerAlloca(Inst, Next, DeleteNextInst);
    break;
  case IceInst::Arithmetic:
    Expansion = lowerArithmetic(Inst, Next, DeleteNextInst);
    break;
  case IceInst::Assign:
    Expansion = lowerAssign(Inst, Next, DeleteNextInst);
    break;
  case IceInst::Br:
    Expansion = lowerBr(Inst, Next, DeleteNextInst);
    break;
  case IceInst::Call:
    Expansion = lowerCall(Inst, Next, DeleteNextInst);
    break;
  case IceInst::Conversion:
    Expansion = lowerConversion(Inst, Next, DeleteNextInst);
    break;
  case IceInst::Fcmp:
    Expansion = lowerFcmp(Inst, Next, DeleteNextInst);
    break;
  case IceInst::Icmp:
    Expansion = lowerIcmp(Inst, Next, DeleteNextInst);
    break;
  case IceInst::Load:
    Expansion = lowerLoad(Inst, Next, DeleteNextInst);
    break;
  case IceInst::Phi:
    // Phi lowering happens elsewhere; it involves more than just a
    // local expansion of instructions; and Phis aren't included in
    // the normal instruction list.
    assert(0);
    break;
  case IceInst::Ret:
    Expansion = lowerRet(Inst, Next, DeleteNextInst);
    break;
  case IceInst::Select:
    Expansion = lowerSelect(Inst, Next, DeleteNextInst);
    break;
  case IceInst::Store:
    Expansion = lowerStore(Inst, Next, DeleteNextInst);
    break;
  case IceInst::Switch:
    Expansion = lowerSwitch(Inst, Next, DeleteNextInst);
    break;
  case IceInst::Target:
    // We're creating target instructions out of high-level
    // instructions, so we should never see a Target instruction
    // here.
    assert(0);
    break;
  }
  return Expansion;
}
