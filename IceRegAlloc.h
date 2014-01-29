// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef _IceRegAlloc_h
#define _IceRegAlloc_h

#include "IceDefs.h"
#include "IceTypes.h"

// Currently this just wraps an IceVariable pointer, so in principle
// we could use containers of IceVariable* instead of
// IceLiveRangeWrapper.  But in the future, we may want to do more
// complex things such as live range splitting, and keeping a wrapper
// should make that simpler.
class IceLiveRangeWrapper {
public:
  IceLiveRangeWrapper(IceVariable *Var) : Var(Var) {}
  const IceLiveRange &range(void) const { return Var->getLiveRange(); }
  bool endsBefore(const IceLiveRangeWrapper &Other) const {
    return range().endsBefore(Other.range());
  }
  bool overlaps(const IceLiveRangeWrapper &Other) const {
    return range().overlaps(Other.range());
  }
  IceVariable *const Var;
  void dump(IceOstream &Str) const;
};
IceOstream &operator<<(IceOstream &Str, const IceLiveRangeWrapper &R);

class IceLinearScan {
public:
  IceLinearScan(IceCfg *Cfg) : Cfg(Cfg) {}
  void init(bool AllowSingleBlockRanges);
  void reset(void);
  void doScan(const llvm::SmallBitVector &RegMask);
  void assign(void) const;
  void dump(IceOstream &Str) const;

private:
  IceCfg *const Cfg;
  // RangeCompare is the comparator for sorting an IceLiveRangeWrapper
  // by starting point in a std::set<>.  Ties are broken by variable
  // number so that sorting is stable.
  struct RangeCompare {
    bool operator()(const IceLiveRangeWrapper &L,
                    const IceLiveRangeWrapper &R) const {
      int Lstart = L.Var->getLiveRange().getStart();
      int Rstart = R.Var->getLiveRange().getStart();
      if (Lstart < Rstart)
        return true;
      if (Lstart > Rstart)
        return false;
      return L.Var->getIndex() < R.Var->getIndex();
    }
  };
  typedef std::set<IceLiveRangeWrapper, RangeCompare> OrderedRanges;
  typedef std::list<IceLiveRangeWrapper> UnorderedRanges;
  OrderedRanges Unhandled;
  UnorderedRanges Active, Inactive, Handled;
};

#endif // _IceRegAlloc_h
