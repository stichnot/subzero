/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "IceCfg.h"
#include "IceInst.h"
#include "IceOperand.h"
#include "IceTargetLowering.h" // dumping stack/frame pointer register

bool operator<(const IceRegWeight &A, const IceRegWeight &B) {
  return A.getWeight() < B.getWeight();
}
bool operator<=(const IceRegWeight &A, const IceRegWeight &B) {
  return !(B < A);
}

void IceVariable::setUse(const IceInst *Inst, const IceCfgNode *Node) {
  if (DefOrUseNode == NULL)
    return;
  if (llvm::isa<IceInstPhi>(Inst) || Node != DefOrUseNode)
    DefOrUseNode = NULL;
}

void IceVariable::setDefinition(IceInst *Inst, const IceCfgNode *Node) {
  if (DefOrUseNode == NULL)
    return;
  // Can first check preexisting DefInst if we care about multi-def vars.
  DefInst = Inst;
  if (Node != DefOrUseNode)
    DefOrUseNode = NULL;
}

void IceVariable::replaceDefinition(IceInst *Inst, const IceCfgNode *Node) {
  DefInst = NULL;
  setDefinition(Inst, Node);
}

IceString IceVariable::getName(void) const {
  if (Name != "")
    return Name;
  char buf[30];
  sprintf(buf, "__%u", getIndex());
  return buf;
}

void IceLiveRange::addSegment(int Start, int End) {
  RangeElementType Element(Start, End);
  RangeType::iterator Next = Range.lower_bound(Element);
  assert(Next == Range.upper_bound(Element)); // Element not already present

  // Beginning of code that merges contiguous segments.  TODO: change
  // "if(true)" to "if(false)" to see if this extra optimization code
  // gives any performance gain, or is just destabilizing.
  if (true) {
    RangeType::iterator FirstDelete = Next;
    RangeType::iterator Prev = Next;
    bool hasPrev = (Next != Range.begin());
    bool hasNext = (Next != Range.end());
    if (hasPrev)
      --Prev;
    // See if Element and Next should be joined.
    if (hasNext && End == Next->first) {
      Element.second = Next->second;
      ++Next;
    }
    // See if Prev and Element should be joined.
    if (hasPrev && Prev->second == Start) {
      Element.first = Prev->first;
      FirstDelete = Prev;
    }
    Range.erase(FirstDelete, Next);
  }
  // End of code that merges contiguous segments.

  Range.insert(Next, Element);
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
  RangeType::const_iterator E1 = Range.end(), E2 = Other.Range.end();
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

bool IceLiveRange::containsValue(int Value) const {
  for (RangeType::const_iterator I = Range.begin(), E = Range.end(); I != E;
       ++I) {
    if (I->first <= Value && Value <= I->second)
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

IceOstream &operator<<(IceOstream &Str, const IceOperand *O) {
  if (O)
    O->dump(Str);
  else
    Str << "<NULL>";
  return Str;
}

void IceVariable::emit(IceOstream &Str, uint32_t Option) const {
  const IceCfgNode *CurrentNode = Str.getCurrentNode();
  assert(DefOrUseNode == NULL || DefOrUseNode == CurrentNode);
  if (getRegNum() >= 0) {
    Str << Str.Cfg->physicalRegName(RegNum);
    return;
  }
  Str << "["
      << Str.Cfg->physicalRegName(Str.Cfg->getTarget()->getFrameOrStackReg());
  int Offset = getStackOffset() + Str.Cfg->getTarget()->getStackAdjustment();
  if (Offset) {
    if (Offset > 0)
      Str << "+";
    Str << Offset;
  }
  Str << "]";
}

void IceVariable::dump(IceOstream &Str) const {
  const IceCfgNode *CurrentNode = Str.getCurrentNode();
  assert(CurrentNode == NULL || DefOrUseNode == NULL ||
         DefOrUseNode == CurrentNode);
  if (Str.isVerbose(IceV_RegOrigins) ||
      (RegNum < 0 && !Str.Cfg->hasComputedFrame()))
    Str << "%" << getName();
  if (RegNum >= 0) {
    if (Str.isVerbose(IceV_RegOrigins))
      Str << ":";
    Str << Str.Cfg->physicalRegName(RegNum);
  } else if (Str.Cfg->hasComputedFrame()) {
    if (Str.isVerbose(IceV_RegOrigins))
      Str << ":";
    Str << "["
        << Str.Cfg->physicalRegName(Str.Cfg->getTarget()->getFrameOrStackReg());
    int Offset = getStackOffset();
    if (Offset) {
      if (Offset > 0)
        Str << "+";
      Str << Offset;
    }
    Str << "]";
  }
}

void IceOperand::emit(IceOstream &Str, uint32_t Option) const { dump(Str); }

void IceOperand::dump(IceOstream &Str) const { Str << "IceOperand<?>"; }

void IceConstantInteger::emit(IceOstream &Str, uint32_t Option) const {
  dump(Str);
}

void IceConstantInteger::dump(IceOstream &Str) const { Str << IntValue; }

void IceConstantRelocatable::emit(IceOstream &Str, uint32_t Option) const {
  Str << Name;
  if (Offset) {
    if (Offset > 0)
      Str << "+";
    Str << Offset;
  }
}

void IceConstantRelocatable::dump(IceOstream &Str) const {
  Str << Name << "+" << Offset << "(CP=" << CPIndex << ")";
}

void IceLiveRange::dump(IceOstream &Str) const {
  Str << "(weight=" << Weight << ") ";
  for (RangeType::const_iterator I = Range.begin(), E = Range.end(); I != E;
       ++I) {
    if (I != Range.begin())
      Str << ", ";
    Str << "[" << (*I).first << ":" << (*I).second << ")";
  }
}

IceOstream &operator<<(IceOstream &Str, const IceLiveRange &L) {
  L.dump(Str);
  return Str;
}

IceOstream &operator<<(IceOstream &Str, const IceRegWeight &W) {
  if (W.getWeight() == IceRegWeight::Inf)
    Str << "Inf";
  else
    Str << W.getWeight();
  return Str;
}
