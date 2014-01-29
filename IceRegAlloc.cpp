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
    Unhandled.insert(IceLiveRangeWrapper(Var));
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

    // Check for active ranges that have expired.
    for (UnorderedRanges::iterator I = Active.begin(), E = Active.end(); I != E;
         I = Next) {
      Next = I;
      ++Next;
      IceLiveRangeWrapper Item = *I;
      if (Item.endsBefore(Cur)) {
        if (Cfg->Str.isVerbose(IceV_LinearScan))
          Cfg->Str << "Expiring     " << Item << "\n";
        Active.erase(I);
        Handled.push_back(Item);
        assert(Item.Var->getRegNumTmp() >= 0);
        --RegUses[Item.Var->getRegNumTmp()];
        assert(RegUses[Item.Var->getRegNumTmp()] >= 0);
      } else if (!Item.overlaps(Cur)) {
        if (Cfg->Str.isVerbose(IceV_LinearScan))
          Cfg->Str << "Inactivating " << Item << "\n";
        Active.erase(I);
        Inactive.push_back(Item);
        assert(Item.Var->getRegNumTmp() >= 0);
        --RegUses[Item.Var->getRegNumTmp()];
        assert(RegUses[Item.Var->getRegNumTmp()] >= 0);
      } else {
      }
    }
    // Check for inactive ranges that have expired or reactivated.
    for (UnorderedRanges::iterator I = Inactive.begin(), E = Inactive.end();
         I != E; I = Next) {
      Next = I;
      ++Next;
      IceLiveRangeWrapper Item = *I;
      if (Item.endsBefore(Cur)) {
        if (Cfg->Str.isVerbose(IceV_LinearScan))
          Cfg->Str << "Expiring     " << Item << "\n";
        Inactive.erase(I);
        Handled.push_back(Item);
      } else if (Item.overlaps(Cur)) {
        if (Cfg->Str.isVerbose(IceV_LinearScan))
          Cfg->Str << "Reactivating " << Item << "\n";
        Inactive.erase(I);
        Active.push_back(Item);
        assert(Item.Var->getRegNumTmp() >= 0);
        ++RegUses[Item.Var->getRegNumTmp()];
        assert(RegUses[Item.Var->getRegNumTmp()] >= 0);
      }
    }
    // Calculate available registers.
    // TODO: Figure out whether removing any of these registers
    // affects the preferred register assignment.
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
      if (Item.overlaps(Cur)) {
        assert(Free[Item.Var->getRegNumTmp()]);
        Free[Item.Var->getRegNumTmp()] = false;
      }
    }
    // Remove registers where an Unhandled range overlaps with the
    // current range and is precolored.
    for (OrderedRanges::iterator I = Unhandled.begin(), E = Unhandled.end();
         I != E; ++I) {
      IceLiveRangeWrapper UnhandledItem = *I;
      if (UnhandledItem.Var->getRegNumTmp() >= 0 &&
          UnhandledItem.overlaps(Cur)) {
        // assert(Free[UnhandledItem.Var->getRegNumTmp()]); // TODO: is this
        // assert valid?
        Free[UnhandledItem.Var->getRegNumTmp()] = false;
      }
    }
    IceVariable *Prefer = Cur.Var->getPreferredRegister();
    int PreferReg = Prefer ? Prefer->getRegNumTmp() : -1;
    if (Cfg->Str.isVerbose(IceV_LinearScan)) {
      for (unsigned i = 0; i < RegMask.size(); ++i) {
        if (RegMask[i]) {
          Cfg->Str << Cfg->physicalRegName(i) << "(U=" << RegUses[i]
                   << ",F=" << Free[i] << ") ";
        }
      }
      Cfg->Str << "\n";
    }
    if (Prefer && PreferReg >= 0 &&
        (Cur.Var->getRegisterOverlap() || Free[PreferReg])) {
      Cur.Var->setRegNumTmp(PreferReg);
      if (Cfg->Str.isVerbose(IceV_LinearScan))
        Cfg->Str << "Preferring   " << Cur << "\n";
      ++RegUses[Cur.Var->getRegNumTmp()];
      assert(RegUses[Cur.Var->getRegNumTmp()] >= 0);
      Active.push_back(Cur);
    } else if (Free.any()) {
      if (Cur.Var->getRegNumTmp() < 0) {
        // TODO: there might be a better policy than lowest-numbered
        // available register.
        int Preference = Free.find_first();
        Cur.Var->setRegNumTmp(Preference);
      }
      if (Cfg->Str.isVerbose(IceV_LinearScan))
        Cfg->Str << "Allocating   " << Cur << "\n";
      ++RegUses[Cur.Var->getRegNumTmp()];
      assert(RegUses[Cur.Var->getRegNumTmp()] >= 0);
      Active.push_back(Cur);
    } else {
      std::vector<IceRegWeight> Weights(RegMask.size());
      for (UnorderedRanges::const_iterator I = Active.begin(), E = Active.end();
           I != E; ++I) {
        IceLiveRangeWrapper Item = *I;
        if (Item.overlaps(Cur)) {
          int RegNum = Item.Var->getRegNumTmp();
          assert(RegNum >= 0);
          Weights[RegNum].addWeight(Item.range().getWeight());
        }
      }
      for (UnorderedRanges::const_iterator I = Inactive.begin(),
                                           E = Inactive.end();
           I != E; ++I) {
        IceLiveRangeWrapper Item = *I;
        if (Item.overlaps(Cur)) {
          int RegNum = Item.Var->getRegNumTmp();
          assert(RegNum >= 0);
          Weights[RegNum].addWeight(Item.range().getWeight());
        }
      }
      for (OrderedRanges::const_iterator I = Unhandled.begin(),
                                         E = Unhandled.end();
           I != E; ++I) {
        IceLiveRangeWrapper Item = *I;
        if (Item.overlaps(Cur)) {
          int RegNum = Item.Var->getRegNumTmp();
          // TODO: handle properly for precolored ranges
          if (RegNum >= 0 && false) {
            Weights[RegNum].setWeight(IceRegWeight::Inf);
          }
        }
      }
      int MinWeightIndex = RegMask.find_first();
      for (unsigned i = MinWeightIndex + 1; i < Weights.size(); ++i) {
        if (RegMask[i] &&
            Weights[i] < Weights[MinWeightIndex])
          MinWeightIndex = i;
      }
      if (Cur.range().getWeight() <= Weights[MinWeightIndex]) {
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
          if (Item.Var->getRegNumTmp() == MinWeightIndex) {
            if (Cfg->Str.isVerbose(IceV_LinearScan))
              Cfg->Str << "Evicting     " << Item << "\n";
            --RegUses[Item.Var->getRegNumTmp()];
            assert(RegUses[Item.Var->getRegNumTmp()] >= 0);
            Item.Var->setRegNumTmp(-1);
            Active.erase(I);
            Handled.push_back(Item);
          }
        }
        for (UnorderedRanges::iterator I = Inactive.begin(), E = Inactive.end();
             I != E; I = Next) {
          Next = I;
          ++Next;
          IceLiveRangeWrapper Item = *I;
          if (Item.Var->getRegNumTmp() == MinWeightIndex) {
            if (Cfg->Str.isVerbose(IceV_LinearScan))
              Cfg->Str << "Evicting     " << Item << "\n";
            //--RegUses[Item.Var->getRegNumTmp()];
            // assert(RegUses[Item.Var->getRegNumTmp()] >= 0);
            Item.Var->setRegNumTmp(-1);
            Inactive.erase(I);
            Handled.push_back(Item);
          }
        }
        // Assign the register to Cur.
        Cur.Var->setRegNumTmp(MinWeightIndex);
        ++RegUses[Cur.Var->getRegNumTmp()];
        assert(RegUses[Cur.Var->getRegNumTmp()] >= 0);
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
    int RegNum = Item.Var->getRegNumTmp();
    Cfg->Str << "Assigning " << Cfg->physicalRegName(RegNum) << "(r" << RegNum
             << ") to " << Item.Var << "\n";
    Item.Var->setRegNum(Item.Var->getRegNumTmp());
  }
}

// ======================== Dump routines ======================== //

void IceLiveRangeWrapper::dump(IceOstream &Str) const {
  char buf[30];
  sprintf(buf, "%2d", Var->getRegNumTmp());
  Str << "R=" << buf << "  V=" << Var << "  Range=";
  range().dump(Str);
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
