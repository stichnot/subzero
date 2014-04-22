//===- subzero/src/Operand.cpp - High-level operand implementation -----===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Operand class and its
// target-independent subclasses, primarily for the methods of the
// Variable class.
//
//===----------------------------------------------------------------------===//

#include "IceCfg.h"
#include "IceInst.h"
#include "IceOperand.h"
#include "IceTargetLowering.h" // dumping stack/frame pointer register

namespace Ice {

bool operator<(const RegWeight &A, const RegWeight &B) {
  return A.getWeight() < B.getWeight();
}
bool operator<=(const RegWeight &A, const RegWeight &B) { return !(B < A); }
bool operator==(const RegWeight &A, const RegWeight &B) {
  return !(B < A) && !(A < B);
}

void LiveRange::addSegment(int32_t Start, int32_t End) {
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
bool LiveRange::endsBefore(const LiveRange &Other) const {
  // Neither range should be empty, but let's be graceful.
  if (Range.empty() || Other.Range.empty())
    return true;
  int32_t MyEnd = (*Range.rbegin()).second;
  int32_t OtherStart = (*Other.Range.begin()).first;
  return MyEnd <= OtherStart;
}

// Returns true if there is any overlap between the two live ranges.
bool LiveRange::overlaps(const LiveRange &Other) const {
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
bool LiveRange::containsValue(int32_t Value) const {
  for (RangeType::const_iterator I = Range.begin(), E = Range.end(); I != E;
       ++I) {
    if (I->first <= Value && Value <= I->second)
      return true;
  }
  return false;
}

void Variable::setUse(const Inst *Inst, const CfgNode *Node) {
  if (DefNode == NULL)
    return;
  if (llvm::isa<InstPhi>(Inst) || Node != DefNode)
    DefNode = NULL;
}

void Variable::setDefinition(Inst *Inst, const CfgNode *Node) {
  if (DefNode == NULL)
    return;
  // Can first check preexisting DefInst if we care about multi-def vars.
  DefInst = Inst;
  if (Node != DefNode)
    DefNode = NULL;
}

void Variable::replaceDefinition(Inst *Inst, const CfgNode *Node) {
  DefInst = NULL;
  setDefinition(Inst, Node);
}

void Variable::setIsArg(IceCfg *Cfg) {
  IsArgument = true;
  if (DefNode == NULL)
    return;
  CfgNode *Entry = Cfg->getEntryNode();
  if (DefNode == Entry)
    return;
  DefNode = NULL;
}

IceString Variable::getName() const {
  if (Name != "")
    return Name;
  const static size_t BufLen = 30;
  char buf[BufLen];
  snprintf(buf, BufLen, "__%u", getIndex());
  return buf;
}

Variable Variable::asType(IceType Type) {
  Variable V(Type, DefNode, Number, Name);
  V.RegNum = RegNum;
  V.StackOffset = StackOffset;
  return V;
}

// ======================== dump routines ======================== //

// TODO: This should be handed by the TargetLowering subclass.
void Variable::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->getStrEmit();
  assert(DefNode == NULL || DefNode == Cfg->getCurrentNode());
  if (hasReg()) {
    Str << Cfg->getTarget()->getRegName(RegNum, getType());
    return;
  }
  switch (typeWidthInBytes(getType())) {
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

void Variable::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
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

void ConstantRelocatable::emit(const IceCfg *Cfg, uint32_t Option) const {
  IceOstream &Str = Cfg->getContext()->getStrEmit();
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

void ConstantRelocatable::dump(const IceCfg *Cfg) const {
  IceOstream &Str = Cfg->getContext()->getStrDump();
  Str << "@" << Name;
  if (Offset)
    Str << "+" << Offset;
}

void LiveRange::dump(IceOstream &Str) const {
  Str << "(weight=" << Weight << ") ";
  for (RangeType::const_iterator I = Range.begin(), E = Range.end(); I != E;
       ++I) {
    if (I != Range.begin())
      Str << ", ";
    Str << "[" << (*I).first << ":" << (*I).second << ")";
  }
}

IceOstream &operator<<(IceOstream &Str, const LiveRange &L) {
  L.dump(Str);
  return Str;
}

IceOstream &operator<<(IceOstream &Str, const RegWeight &W) {
  if (W.getWeight() == RegWeight::Inf)
    Str << "Inf";
  else
    Str << W.getWeight();
  return Str;
}

} // end of namespace Ice
