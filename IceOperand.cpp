/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "IceCfg.h"
#include "IceInst.h"
#include "IceOperand.h"

void IceVariable::setUse(const IceInst *Inst, const IceCfgNode *Node) {
  ++UseCount;
  if (llvm::isa<IceInstPhi>(Inst) ||
      (DefOrUseNode != NULL && DefOrUseNode != Node)) {
    IsMultiblockLife = true;
  }
  DefOrUseNode = Node;
}

void IceVariable::removeUse(void) {
  if (!canAutoDelete())
    return;
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

void IceLiveRange::addSegment(int Start, int End) {
  // TODO: coalesce contiguous ranges, though that may not happen much
  // when most blocks contain phi instructions.  Main exception:
  // extended basic block pairs that are laid out consecutively.
  Range.insert(std::pair<int, int>(Start, End));
}

bool IceLiveRange::endsBefore(const IceLiveRange &Other) const {
  // Neither range should be empty, but let's be graceful.
  if (Range.empty() || Other.Range.empty())
    return true;
  int MyEnd = (*Range.rbegin()).second;
  int OtherStart = (*Other.Range.begin()).first;
  return MyEnd <= OtherStart;
}

bool IceLiveRange::overlaps(const IceLiveRange &Other) const {
  // Do a two-finger walk through the two sorted lists of segments.
  RangeType::const_iterator I1 = Range.begin(), I2 = Other.Range.begin();
  RangeType::const_iterator E1 = Range.end(),   E2 = Other.Range.end();
  while (I1 != E1 && I2 != E2) {
    if (I1->second <= I2->first) {
      ++I1;
      continue;
    }
    if (I2->second <= I1->first) {
      ++I2;
      continue;
    }
    return true;
  }
  return false;
}

void IceLiveRange::unitTests(void) {
  IceCfg Cfg;
  IceOstream &Str = Cfg.Str;
  IceLiveRange R1, R2, R3;
  IceLiveRange Empty;
  R1.addSegment(1, 10);
  R2.addSegment(10, 20);
  R3.addSegment(1, 11);

  Str << "Empty = " << Empty << "\n";
  Str << "R1 = " << R1 << "\n";
  Str << "R2 = " << R2 << "\n";
  Str << "R3 = " << R3 << "\n";
  Str << "R1 endsBefore R2 = " << R1.endsBefore(R2) << "\n";
  Str << "R1 endsBefore R3 = " << R1.endsBefore(R3) << "\n";
  Str << "R2 endsBefore R1 = " << R2.endsBefore(R1) << "\n";
  Str << "R2 endsBefore R3 = " << R2.endsBefore(R3) << "\n";
  Str << "R3 endsBefore R1 = " << R3.endsBefore(R1) << "\n";
  Str << "R3 endsBefore R2 = " << R3.endsBefore(R2) << "\n";

  Str << "R1 overlaps Empty = " << R1.overlaps(Empty) << "\n";
  Str << "Empty overlaps R1 = " << Empty.overlaps(R1) << "\n";
  Str << "R1 overlaps R2 = " << R1.overlaps(R2) << "\n";
  Str << "R2 overlaps R1 = " << R2.overlaps(R1) << "\n";
  Str << "R3 overlaps R2 = " << R3.overlaps(R2) << "\n";
  Str << "R2 overlaps R3 = " << R2.overlaps(R3) << "\n";

  IceLiveRange R4;
  R4.addSegment(1, 10);
  R4.addSegment(20, 30);
  Str << "R4 = " << R4 << "\n";
  Str << "R2 endsBefore R4 = " << R2.endsBefore(R4) << "\n";
  Str << "R4 endsBefore R2 = " << R4.endsBefore(R2) << "\n";
  Str << "R2 overlaps R4 = " << R2.overlaps(R4) << "\n";
  Str << "R4 overlaps R2 = " << R4.overlaps(R2) << "\n";
  Str << "R3 overlaps R4 = " << R3.overlaps(R4) << "\n";
  Str << "R4 overlaps R3 = " << R4.overlaps(R3) << "\n";
}

// ======================== dump routines ======================== //

IceOstream& operator<<(IceOstream &Str, const IceOperand *O) {
  if (O)
    O->dump(Str);
  else
    Str << "<NULL>";
  return Str;
}

void IceVariable::dump(IceOstream &Str) const {
  if (Str.isVerbose(IceV_RegOrigins) || RegNum < 0)
    Str << "%" << Str.Cfg->variableName(VarIndex);
  if (RegNum >= 0) {
    if (Str.isVerbose(IceV_RegOrigins))
      Str << ":";
    Str << Str.Cfg->physicalRegName(RegNum);
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

void IceLiveRange::dump(IceOstream &Str) const {
  Str << "(weight=" << Weight << ") ";
  for (RangeType::const_iterator I = Range.begin(), E = Range.end();
       I != E; ++I) {
    if (I != Range.begin())
      Str << ", ";
    Str << "[" << (*I).first << ":" << (*I).second << ")";
  }
}

IceOstream& operator<<(IceOstream &Str, const IceLiveRange &L) {
  L.dump(Str);
  return Str;
}
