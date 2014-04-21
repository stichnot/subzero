//===- subzero/src/IceOperand.cpp - High-level operand implementation -----===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the IceOperand class and its
// target-independent subclasses, primarily for the methods of the
// IceVariable class.
//
//===----------------------------------------------------------------------===//

#include "IceCfg.h"
#include "IceInst.h"
#include "IceOperand.h"
#include "IceTargetLowering.h" // dumping stack/frame pointer register

namespace Ice {

bool operator<(const IceRegWeight &A, const IceRegWeight &B) {
  return A.getWeight() < B.getWeight();
}
bool operator<=(const IceRegWeight &A, const IceRegWeight &B) {
  return !(B < A);
}
bool operator==(const IceRegWeight &A, const IceRegWeight &B) {
  return !(B < A) && !(A < B);
}

void IceLiveRange::addSegment(int32_t Start, int32_t End) {
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
  int32_t CurrentEnd = Range.back().second;
  assert(Start >= CurrentEnd);
  // Check for merge opportunity.
  if (Start == CurrentEnd) {
    Range.back().second = End;
    return;
  }
  Range.push_back(RangeElementType(Start, End));
#endif
}

// Returns true if this live range ends before Other's live range
// starts.  This means that the highest instruction number in this
// live range is less than or equal to the lowest instruction number
// of the Other live range.
bool IceLiveRange::endsBefore(const IceLiveRange &Other) const {
  // Neither range should be empty, but let's be graceful.
  if (Range.empty() || Other.Range.empty())
    return true;
  int32_t MyEnd = (*Range.rbegin()).second;
  int32_t OtherStart = (*Other.Range.begin()).first;
  return MyEnd <= OtherStart;
}

// Returns true if there is any overlap between the two live ranges.
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

// Returns true if the live range contains the given instruction
// number.  This is only used for validating the live range
// calculation.
bool IceLiveRange::containsValue(int32_t Value) const {
  for (RangeType::const_iterator I = Range.begin(), E = Range.end(); I != E;
       ++I) {
    if (I->first <= Value && Value <= I->second)
      return true;
  }
  return false;
}

void IceVariable::setUse(const IceInst *Inst, const CfgNode *Node) {
  if (DefNode == NULL)
    return;
  if (llvm::isa<IceInstPhi>(Inst) || Node != DefNode)
    DefNode = NULL;
}

void IceVariable::setDefinition(IceInst *Inst, const CfgNode *Node) {
  if (DefNode == NULL)
    return;
  // Can first check preexisting DefInst if we care about multi-def vars.
  DefInst = Inst;
  if (Node != DefNode)
    DefNode = NULL;
}

void IceVariable::replaceDefinition(IceInst *Inst, const CfgNode *Node) {
  DefInst = NULL;
  setDefinition(Inst, Node);
}

void IceVariable::setIsArg(IceCfg *Cfg) {
  IsArgument = true;
  if (DefNode == NULL)
    return;
  CfgNode *Entry = Cfg->getEntryNode();
  if (DefNode == Entry)
    return;
  DefNode = NULL;
}

IceString IceVariable::getName() const {
  if (Name != "")
    return Name;
  const static size_t BufLen = 30;
  char buf[BufLen];
  snprintf(buf, BufLen, "__%u", getIndex());
  return buf;
}

IceVariable IceVariable::asType(IceType Type) {
  IceVariable V(Type, DefNode, Number, Name);
  V.RegNum = RegNum;
  V.StackOffset = StackOffset;
  return V;
}

// ======================== dump routines ======================== //

// TODO: This should be handed by the IceTargetLowering subclass.
void IceVariable::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  assert(DefNode == NULL || DefNode == Cfg->getCurrentNode());
  if (hasReg()) {
    Str << Cfg->getTarget()->getRegName(RegNum, getType());
    return;
  }
  switch (iceTypeWidthInBytes(getType())) {
  case 1:
    Str << "byte";
    break;
  case 2:
    Str << "word";
    break;
  case 4:
    Str << "dword";
    break;
  case 8:
    Str << "qword";
    break;
  default:
    assert(0);
    break;
  }
  Str << " ptr [" << Cfg->getTarget()->getRegName(
                         Cfg->getTarget()->getFrameOrStackReg(), IceType_i32);
  int32_t Offset = getStackOffset() + Cfg->getTarget()->getStackAdjustment();
  if (Offset) {
    if (Offset > 0)
      Str << "+";
    Str << Offset;
  }
  Str << "]";
}

void IceVariable::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  const CfgNode *CurrentNode = Cfg->getCurrentNode();
  (void)CurrentNode; // used only in assert()
  assert(CurrentNode == NULL || DefNode == NULL || DefNode == CurrentNode);
  if (Cfg->getContext()->isVerbose(IceV_RegOrigins) ||
      (!hasReg() && !Cfg->getTarget()->hasComputedFrame()))
    Str << "%" << getName();
  if (hasReg()) {
    if (Cfg->getContext()->isVerbose(IceV_RegOrigins))
      Str << ":";
    Str << Cfg->getTarget()->getRegName(RegNum, getType());
  } else if (Cfg->getTarget()->hasComputedFrame()) {
    if (Cfg->getContext()->isVerbose(IceV_RegOrigins))
      Str << ":";
    Str << "[" << Cfg->getTarget()->getRegName(
                      Cfg->getTarget()->getFrameOrStackReg(), IceType_i32);
    int32_t Offset = getStackOffset();
    if (Offset) {
      if (Offset > 0)
        Str << "+";
      Str << Offset;
    }
    Str << "]";
  }
}

void IceConstantRelocatable::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->StrEmit;
  if (SuppressMangling)
    Str << Name;
  else
    Str << Cfg->getContext()->mangleName(Name);
  if (Offset) {
    if (Offset > 0)
      Str << "+";
    Str << Offset;
  }
}

void IceConstantRelocatable::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->StrDump;
  Str << "@" << Name;
  if (Offset)
    Str << "+" << Offset;
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

} // end of namespace Ice
