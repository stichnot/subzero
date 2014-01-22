// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef _IceRegAlloc_h
#define _IceRegAlloc_h

#include <list>
#include <map>

#include "IceDefs.h"
#include "IceTypes.h"

class IceLiveRangeWrapper {
public:
  const IceLiveRange &Range;
  const IceVariable *Var;
  int Register;
  IceLiveRangeWrapper(const IceLiveRange &Range, const IceVariable *Var,
                      int Register)
      : Range(Range), Var(Var), Register(Register) {}
  void dump(IceOstream &Str) const;
};
IceOstream &operator<<(IceOstream &Str, const IceLiveRangeWrapper &R);

class IceLinearScan {
public:
  IceLinearScan(IceCfg *Cfg) : Cfg(Cfg) {}
  void init(void);
  void reset(void);
  void doScan(const llvm::SmallBitVector &RegMask);
  void dump(IceOstream &Str) const;

private:
  IceCfg *const Cfg;
  llvm::SmallBitVector Registers;
  // RangeCompare is the comparator for sorting an IceLiveRangeWrapper
  // by starting point in a std::set<>.  Ties are broken by variable
  // number so that sorting is stable.
  struct RangeCompare {
    bool operator()(const IceLiveRangeWrapper &L,
                    const IceLiveRangeWrapper &R) const {
      int Lstart = L.Range.getStart();
      int Rstart = R.Range.getStart();
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
