/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "IceCfg.h"
#include "IceInst.h"
#include "IceOperand.h"

void IceVariable::setUse(const IceInst *Inst, const IceCfgNode *Node) {
  ++UseCount;
  if (Inst->getKind() == IceInst::Phi ||
      (DefOrUseNode != NULL && DefOrUseNode != Node)) {
    IsMultiblockLife = true;
  }
  DefOrUseNode = Node;
}

void IceVariable::removeUse(void) {
  --UseCount;
  if (UseCount)
    return;
  IceInst *Definition = getDefinition();
  if (Definition == NULL)
    return;
  Definition->removeUse(this);
}

void IceVariable::setDefinition(IceInst *Inst, const IceCfgNode *Node) {
  if (DefInst || IsArgument) {
    DefInst = Inst;
    IsMultiDef = true;
    return;
  }
  if (DefOrUseNode != NULL && DefOrUseNode != Node) {
    IsMultiblockLife = true;
  }
  DefInst = Inst;
  DefOrUseNode = Node;
}

void IceVariable::replaceDefinition(IceInst *Inst, const IceCfgNode *Node) {
  DefInst = NULL;
  setDefinition(Inst, Node);
}

// ======================== Dump routines ======================== //

IceOstream& operator<<(IceOstream &Str, const IceOperand *O) {
  if (O)
    O->dump(Str);
  else
    Str << "<NULL>";
  return Str;
}

void IceVariable::dump(IceOstream &Str) const {
  if (Str.isVerbose() || RegNum < 0)
    Str << "%" << Str.Cfg->variableName(VarIndex);
  if (RegNum >= 0) {
    if (Str.isVerbose())
      Str << ":";
    Str << Str.Cfg->PhysicalRegName(RegNum);
  }
}

void IceOperand::dump(IceOstream &Str) const {
  Str << "IceOperand<?>";
}

void IceConstant::dump(IceOstream &Str) const {
  switch (Type) {
  case IceType_i1:
    Str << Value.I1;
    break;
  case IceType_i8:
    Str << Value.I8;
    break;
  case IceType_i16:
    Str << Value.I16;
    break;
  case IceType_i32:
    Str << Value.I32;
    break;
  case IceType_i64:
    Str << Value.I64;
    break;
  case IceType_f32:
    Str << Value.F32;
    break;
  case IceType_f64:
    Str << Value.F64;
    break;
  case IceType_void:
    break;
  }
}
