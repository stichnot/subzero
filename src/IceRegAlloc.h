//===- subzero/src/IceRegAlloc.h - Linear-scan reg. allocation --*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the data structures used during linear-scan
// register allocation.  This includes IceLiveRangeWrapper which
// encapsulates a variable and its live range, and IceLinearScan which
// holds the various work queues for the linear-scan algorithm.
//
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEREGALLOC_H
#define SUBZERO_SRC_ICEREGALLOC_H

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
  const IceLiveRange &range() const { return Var->getLiveRange(); }
  bool endsBefore(const IceLiveRangeWrapper &Other) const {
    return range().endsBefore(Other.range());
  }
  bool overlaps(const IceLiveRangeWrapper &Other) const {
    return range().overlaps(Other.range());
  }
  IceVariable *const Var;
  void dump(const IceCfg *Cfg) const;

private:
  // IceLiveRangeWrapper(const IceLiveRangeWrapper &) LLVM_DELETED_FUNCTION;
  IceLiveRangeWrapper &
  operator=(const IceLiveRangeWrapper &) LLVM_DELETED_FUNCTION;
};
IceOstream &operator<<(IceOstream &Str, const IceLiveRangeWrapper &R);

class IceLinearScan {
public:
  IceLinearScan(IceCfg *Cfg) : Cfg(Cfg) {}
  void scan(const llvm::SmallBitVector &RegMask);
  void dump(const IceCfg *Cfg) const;

private:
  IceCfg *const Cfg;
  // RangeCompare is the comparator for sorting an IceLiveRangeWrapper
  // by starting point in a std::set<>.  Ties are broken by variable
  // number so that sorting is stable.
  struct RangeCompare {
    bool operator()(const IceLiveRangeWrapper &L,
                    const IceLiveRangeWrapper &R) const {
      int32_t Lstart = L.Var->getLiveRange().getStart();
      int32_t Rstart = R.Var->getLiveRange().getStart();
      if (Lstart == Rstart)
        return L.Var->getIndex() < R.Var->getIndex();
      return Lstart < Rstart;
    }
  };
  typedef std::set<IceLiveRangeWrapper, RangeCompare> OrderedRanges;
  typedef std::list<IceLiveRangeWrapper> UnorderedRanges;
  OrderedRanges Unhandled;
  UnorderedRanges Active, Inactive, Handled;
  IceLinearScan(const IceLinearScan &) LLVM_DELETED_FUNCTION;
  IceLinearScan &operator=(const IceLinearScan &) LLVM_DELETED_FUNCTION;
};

#endif // SUBZERO_SRC_ICEREGALLOC_H
