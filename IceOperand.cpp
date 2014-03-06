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

void IceVariable::setIsArg(IceCfg *Cfg) {
  IsArgument = true;
  if (DefOrUseNode == NULL)
    return;
  IceCfgNode *Entry = Cfg->getEntryNode();
  if (DefOrUseNode == Entry)
    return;
  DefOrUseNode = NULL;
}

IceString IceVariable::getName(void) const {
  if (Name != "")
    return Name;
  char buf[30];
  sprintf(buf, "__%u", getIndex());
  return buf;
}

void IceLiveRange::addSegment(int Start, int End) {
#ifdef USE_SET
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
#else
  if (Range.empty()) {
    Range.push_back(RangeElementType(Start, End));
    return;
  }
  // Special case for faking in-arg liveness.
  if (End < Range.front().first) {
    assert(Start < 0);
    Range.push_front(RangeElementType(Start, End));
    return;
  }
  int CurrentEnd = Range.back().second;
  assert(Start >= CurrentEnd);
  // Check for merge opportunity.
  if (Start == CurrentEnd) {
    Range.back().second = End;
    return;
  }
  Range.push_back(RangeElementType(Start, End));
#endif
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

// ======================== dump routines ======================== //

IceOstream &operator<<(IceOstream &Str, const IceOperand *O) {
  if (O)
    O->dump(Str);
  else
    Str << "<NULL>";
  return Str;
}

void IceVariable::emit(IceOstream &Str, uint32_t Option) const {
  assert(DefOrUseNode == NULL || DefOrUseNode == Str.getCurrentNode());
  if (getRegNum() >= 0) {
    Str << Str.Cfg->physicalRegName(RegNum, getType(), Option);
    return;
  }
  Str << "["
      << Str.Cfg->physicalRegName(Str.Cfg->getTarget()->getFrameOrStackReg(),
                                  IceType_i32, Option);
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
  (void)CurrentNode;
  assert(CurrentNode == NULL || DefOrUseNode == NULL ||
         DefOrUseNode == CurrentNode);
  if (Str.isVerbose(IceV_RegOrigins) ||
      (RegNum < 0 && !Str.Cfg->hasComputedFrame()))
    Str << "%" << getName();
  if (RegNum >= 0) {
    if (Str.isVerbose(IceV_RegOrigins))
      Str << ":";
    Str << Str.Cfg->physicalRegName(RegNum, getType());
  } else if (Str.Cfg->hasComputedFrame()) {
    if (Str.isVerbose(IceV_RegOrigins))
      Str << ":";
    Str << "[" << Str.Cfg->physicalRegName(
                      Str.Cfg->getTarget()->getFrameOrStackReg(), IceType_i32);
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

void IceConstantFloat::emit(IceOstream &Str, uint32_t Option) const {
  dump(Str);
}

void IceConstantFloat::dump(IceOstream &Str) const { Str << FloatValue; }

void IceConstantDouble::emit(IceOstream &Str, uint32_t Option) const {
  dump(Str);
}

void IceConstantDouble::dump(IceOstream &Str) const { Str << DoubleValue; }

void IceConstantRelocatable::emit(IceOstream &Str, uint32_t Option) const {
  Str << Str.Cfg->mangleName(Name);
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
