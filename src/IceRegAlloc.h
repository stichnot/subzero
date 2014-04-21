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
// register allocation.  This includes LiveRangeWrapper which
// encapsulates a variable and its live range, and IceLinearScan which
// holds the various work queues for the linear-scan algorithm.
//
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEREGALLOC_H
#define SUBZERO_SRC_ICEREGALLOC_H

#include "IceDefs.h"
#include "IceTypes.h"

namespace Ice {

// Currently this just wraps an IceVariable pointer, so in principle
// we could use containers of IceVariable* instead of
// LiveRangeWrapper.  But in the future, we may want to do more
// complex things such as live range splitting, and keeping a wrapper
// should make that simpler.
class LiveRangeWrapper {
public:
  LiveRangeWrapper(IceVariable *Var) : Var(Var) {}
  const LiveRange &range() const { return Var->getLiveRange(); }
  bool endsBefore(const LiveRangeWrapper &Other) const {
    return range().endsBefore(Other.range());
  }
  bool overlaps(const LiveRangeWrapper &Other) const {
    return range().overlaps(Other.range());
  }
  IceVariable *const Var;
  void dump(const IceCfg *Cfg) const;

private:
  // LiveRangeWrapper(const LiveRangeWrapper &) LLVM_DELETED_FUNCTION;
  LiveRangeWrapper &operator=(const LiveRangeWrapper &) LLVM_DELETED_FUNCTION;
};

class IceLinearScan {
public:
  IceLinearScan(IceCfg *Cfg) : Cfg(Cfg) {}
  void scan(const llvm::SmallBitVector &RegMask);
  void dump(IceCfg *Cfg) const;

private:
  IceCfg *const Cfg;
  // RangeCompare is the comparator for sorting an LiveRangeWrapper
  // by starting point in a std::set<>.  Ties are broken by variable
  // number so that sorting is stable.
  struct RangeCompare {
    bool operator()(const LiveRangeWrapper &L,
                    const LiveRangeWrapper &R) const {
      int32_t Lstart = L.Var->getLiveRange().getStart();
      int32_t Rstart = R.Var->getLiveRange().getStart();
      if (Lstart == Rstart)
        return L.Var->getIndex() < R.Var->getIndex();
      return Lstart < Rstart;
    }
  };
  typedef std::set<LiveRangeWrapper, RangeCompare> OrderedRanges;
  typedef std::list<LiveRangeWrapper> UnorderedRanges;
  OrderedRanges Unhandled;
  UnorderedRanges Active, Inactive, Handled;
  IceLinearScan(const IceLinearScan &) LLVM_DELETED_FUNCTION;
  IceLinearScan &operator=(const IceLinearScan &) LLVM_DELETED_FUNCTION;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEREGALLOC_H
