/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "IceCfg.h"
#include "IceInst.h"
#include "IceOperand.h"
#include "IceRegAlloc.h"

// First-time initialization.  Gather the live ranges of all variables
// and add them to the Unhandled set.
void IceLinearScan::init(bool AllowSingleBlockRanges) {
  assert(Unhandled.empty());
  assert(Handled.empty());
  assert(Inactive.empty());
  assert(Active.empty());
  const IceVarList &Vars = Cfg->getVariables();
  for (IceVarList::const_iterator I = Vars.begin(), E = Vars.end(); I != E;
       ++I) {
    IceVariable *Var = *I;
    if (Var == NULL)
      continue;
#if 0
    if (!AllowSingleBlockRanges && !Var->isMultiblockLife() &&
        !llvm::isa<IceInstPhi>(Var->getDefinition()))
      continue;
#endif
    if (Var->getLiveRange().getStart() < 0) // empty live range
      continue;
    Unhandled.insert(
        IceLiveRangeWrapper(Var->getLiveRange(), Var, Var->getRegNum()));
  }
}

// Re-initialization (for experimentation).  Move live ranges from the
// Handled set back into the Unhandled set, in order to e.g. repeat
// register allocation with a different register set.
void IceLinearScan::reset(void) {
  assert(Unhandled.empty());
  assert(Inactive.empty());
  assert(Active.empty());
  for (UnorderedRanges::const_iterator I = Handled.begin(), E = Handled.end();
       I != E; ++I) {
    Unhandled.insert(*I);
  }
  Handled.clear();
}

void IceLinearScan::doScan(const llvm::SmallBitVector &RegMask) {
  if (!RegMask.any())
    return;
  // RegUses[I] is the number of live ranges (variables) that register
  // I is currently assigned to.  It can be greater than 1 as a result
  // of IceVariable::LinkedTo.
  std::vector<int> RegUses(RegMask.size());
  // Unhandled is already set to all ranges in increasing order of
  // start points.
  assert(Active.empty());
  assert(Inactive.empty());
  assert(Handled.empty());
  UnorderedRanges::iterator Next;
  while (!Unhandled.empty()) {
    IceLiveRangeWrapper Cur = *Unhandled.begin();
    if (Cfg->Str.isVerbose(IceV_LinearScan))
      Cfg->Str << "\nConsidering  " << Cur << "\n";
    // TODO: Ignore certain live ranges, or treat their weight as 0,
    // under certain conditions, such as single-block lifetime.
    Unhandled.erase(Unhandled.begin());
    // TODO: If Cur comes from "z=y+..." and y's live range is being
    // expired or inactivated, favor using y's register.

    int LinkedReg = -1;
    // Check for active ranges that have expired.
    for (UnorderedRanges::iterator I = Active.begin(), E = Active.end(); I != E;
         I = Next) {
      Next = I;
      ++Next;
      IceLiveRangeWrapper Item = *I;
      if (Item.Range.endsBefore(Cur.Range)) {
        if (Cfg->Str.isVerbose(IceV_LinearScan))
          Cfg->Str << "Expiring     " << Item << "\n";
        Active.erase(I);
        Handled.push_back(Item);
        assert(Item.Register >= 0);
        --RegUses[Item.Register];
        assert(RegUses[Item.Register] >= 0);
      } else if (!Item.Range.overlaps(Cur.Range)) {
        if (Cfg->Str.isVerbose(IceV_LinearScan))
          Cfg->Str << "Inactivating " << Item << "\n";
        Active.erase(I);
        Inactive.push_back(Item);
        assert(Item.Register >= 0);
        --RegUses[Item.Register];
        assert(RegUses[Item.Register] >= 0);
      } else {
        if (Cur.Var->getLinkedTo() == Item.Var) {
          assert(LinkedReg < 0);
          LinkedReg = Item.Register;
          assert(LinkedReg >= 0);
          if (Cfg->Str.isVerbose(IceV_LinearScan))
            Cfg->Str << "Linking to   " << Item << "\n";
        }
      }
    }
    // Check for inactive ranges that have expired or reactivated.
    for (UnorderedRanges::iterator I = Inactive.begin(), E = Inactive.end();
         I != E; I = Next) {
      Next = I;
      ++Next;
      IceLiveRangeWrapper Item = *I;
      if (Item.Range.endsBefore(Cur.Range)) {
        if (Cfg->Str.isVerbose(IceV_LinearScan))
          Cfg->Str << "Expiring     " << Item << "\n";
        Inactive.erase(I);
        Handled.push_back(Item);
      } else if (Item.Range.overlaps(Cur.Range)) {
        if (Cfg->Str.isVerbose(IceV_LinearScan))
          Cfg->Str << "Reactivating " << Item << "\n";
        Inactive.erase(I);
        Active.push_back(Item);
        assert(Item.Register >= 0);
        ++RegUses[Item.Register];
        assert(RegUses[Item.Register] >= 0);
        if (Cur.Var->getLinkedTo() == Item.Var) {
          assert(LinkedReg < 0);
          LinkedReg = Item.Register;
          assert(LinkedReg >= 0);
          if (Cfg->Str.isVerbose(IceV_LinearScan))
            Cfg->Str << "Linking to   " << Item << "\n";
        }
      }
    }
    // Calculate available registers.
    // TODO: Figure out whether removing any of these registers
    // affects the LinkedReg assignment.
    llvm::SmallBitVector Free = RegMask;
    for (unsigned i = 0; i < RegMask.size(); ++i) {
      if (RegUses[i] > 0)
        Free[i] = false;
    }
    // Remove registers where an Inactive range overlaps with the
    // current range.
    for (UnorderedRanges::iterator I = Inactive.begin(), E = Inactive.end();
         I != E; ++I) {
      IceLiveRangeWrapper Item = *I;
      if (Item.Range.overlaps(Cur.Range)) {
        assert(Free[Item.Register]);
        Free[Item.Register] = false;
      }
    }
    // Remove registers where an Unhandled range overlaps with the
    // current range and is precolored.
    for (OrderedRanges::iterator I = Unhandled.begin(), E = Unhandled.end();
         I != E; ++I) {
      IceLiveRangeWrapper UnhandledItem = *I;
      if (UnhandledItem.Register >= 0 &&
          UnhandledItem.Range.overlaps(Cur.Range)) {
        // assert(Free[UnhandledItem.Register]); // TODO: is this assert valid?
        Free[UnhandledItem.Register] = false;
      }
    }
    if (LinkedReg >= 0) {
      Cur.Register = LinkedReg;
      if (Cfg->Str.isVerbose(IceV_LinearScan))
        Cfg->Str << "Allocating   " << Cur << "\n";
      ++RegUses[Cur.Register];
      assert(RegUses[Cur.Register] >= 0);
      Active.push_back(Cur);
    } else if (Free.any()) {
      if (Cur.Register < 0)
        // TODO: there might be a better policy than lowest-numbered
        // available register.
        Cur.Register = Free.find_first();
      if (Cfg->Str.isVerbose(IceV_LinearScan))
        Cfg->Str << "Allocating   " << Cur << "\n";
      ++RegUses[Cur.Register];
      assert(RegUses[Cur.Register] >= 0);
      Active.push_back(Cur);
    } else {
      std::vector<int> Weights(RegMask.size());
      for (UnorderedRanges::const_iterator I = Active.begin(), E = Active.end();
           I != E; ++I) {
        IceLiveRangeWrapper Item = *I;
        if (Item.Range.overlaps(Cur.Range)) {
          assert(Item.Register >= 0);
          Weights[Item.Register] +=
              Item.Range.getWeight() * Item.Var->getWeight();
        }
      }
      for (UnorderedRanges::const_iterator I = Inactive.begin(),
                                           E = Inactive.end();
           I != E; ++I) {
        IceLiveRangeWrapper Item = *I;
        if (Item.Range.overlaps(Cur.Range)) {
          assert(Item.Register >= 0);
          Weights[Item.Register] +=
              Item.Range.getWeight() * Item.Var->getWeight();
        }
      }
      for (OrderedRanges::const_iterator I = Unhandled.begin(),
                                         E = Unhandled.end();
           I != E; ++I) {
        IceLiveRangeWrapper Item = *I;
        if (Item.Range.overlaps(Cur.Range)) {
          // TODO: handle properly for precolored ranges
          if (Item.Register >= 0 && false) {
            Weights[Item.Register] = 1 << 31; // effectively infinite
          }
        }
      }
      int MinWeightIndex = RegMask.find_first();
      for (unsigned i = MinWeightIndex + 1; i < Weights.size(); ++i) {
        if (RegMask[i] && Weights[i] < Weights[MinWeightIndex])
          MinWeightIndex = i;
      }
      if (Cur.Range.getWeight() <= Weights[MinWeightIndex]) {
        // Cur doesn't have priority over any other live ranges, so
        // don't allocate any register to it, and move it to the
        // Handled state.
        Handled.push_back(Cur);
      } else {
        // Evict all live ranges in Active and Inactive that register
        // number MinWeightIndex is assigned to.
        for (UnorderedRanges::iterator I = Active.begin(), E = Active.end();
             I != E; I = Next) {
          Next = I;
          ++Next;
          IceLiveRangeWrapper Item = *I;
          if (Item.Register == MinWeightIndex) {
            if (Cfg->Str.isVerbose(IceV_LinearScan))
              Cfg->Str << "Evicting     " << Item << "\n";
            --RegUses[Item.Register];
            assert(RegUses[Item.Register] >= 0);
            Item.Register = -1;
            Active.erase(I);
            Handled.push_back(Item);
          }
        }
        for (UnorderedRanges::iterator I = Inactive.begin(), E = Inactive.end();
             I != E; I = Next) {
          Next = I;
          ++Next;
          IceLiveRangeWrapper Item = *I;
          if (Item.Register == MinWeightIndex) {
            if (Cfg->Str.isVerbose(IceV_LinearScan))
              Cfg->Str << "Evicting     " << Item << "\n";
            //--RegUses[Item.Register];
            // assert(RegUses[Item.Register] >= 0);
            Item.Register = -1;
            Inactive.erase(I);
            Handled.push_back(Item);
          }
        }
        // Assign the register to Cur.
        Cur.Register = MinWeightIndex;
        ++RegUses[Cur.Register];
        assert(RegUses[Cur.Register] >= 0);
        Active.push_back(Cur);
        if (Cfg->Str.isVerbose(IceV_LinearScan))
          Cfg->Str << "Allocating   " << Cur << "\n";
      }
    }
    dump(Cfg->Str);
  }
  // Move anything Active or Inactive to Handled for easier handling.
  for (UnorderedRanges::iterator I = Active.begin(), E = Active.end(); I != E;
       I = Next) {
    Next = I;
    ++Next;
    Handled.push_back(*I);
    Active.erase(I);
  }
  for (UnorderedRanges::iterator I = Inactive.begin(), E = Inactive.end();
       I != E; I = Next) {
    Next = I;
    ++Next;
    Handled.push_back(*I);
    Inactive.erase(I);
  }
  dump(Cfg->Str);
}

void IceLinearScan::assign(void) const {
  for (UnorderedRanges::const_iterator I = Handled.begin(), E = Handled.end();
       I != E; ++I) {
    IceLiveRangeWrapper Item = *I;
    Cfg->Str << "Assigning reg " << Item.Register << " to " << Item.Var << "\n";
    Item.Var->setRegNum(Item.Register);
  }
}

// ======================== Dump routines ======================== //

void IceLiveRangeWrapper::dump(IceOstream &Str) const {
  char buf[30];
  sprintf(buf, "%2d", Register);
  Str << "R=" << buf << "  V=" << Var << "  Range=";
  Range.dump(Str);
}

IceOstream &operator<<(IceOstream &Str, const IceLiveRangeWrapper &R) {
  R.dump(Str);
  return Str;
}

void IceLinearScan::dump(IceOstream &Str) const {
  if (!Cfg->Str.isVerbose(IceV_LinearScan))
    return;
  Str << "**** Current regalloc state:\n";
  Str << "++++++ Handled:\n";
  for (UnorderedRanges::const_iterator I = Handled.begin(), E = Handled.end();
       I != E; ++I) {
    Cfg->Str << *I << "\n";
  }
  Str << "++++++ Unhandled:\n";
  for (OrderedRanges::const_iterator I = Unhandled.begin(), E = Unhandled.end();
       I != E; ++I) {
    Cfg->Str << *I << "\n";
  }
  Str << "++++++ Active:\n";
  for (UnorderedRanges::const_iterator I = Active.begin(), E = Active.end();
       I != E; ++I) {
    Cfg->Str << *I << "\n";
  }
  Str << "++++++ Inactive:\n";
  for (UnorderedRanges::const_iterator I = Inactive.begin(), E = Inactive.end();
       I != E; ++I) {
    Cfg->Str << *I << "\n";
  }
}
