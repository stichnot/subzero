// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef _IceDefs_h
#define _IceDefs_h

#include <assert.h>
#include <stdint.h>
#include <stdio.h> // sprintf

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

// See http://llvm.org/docs/ProgrammersManual.html#isa
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Timer.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/SmallBitVector.h"

class IceCfg;
class IceCfgNode;
class IceInst;
class IceInstPhi;
class IceInstTarget;
class IceLiveness;
class IceLiveRange;
class IceOperand;
class IceVariable;
class IceConstant;
class IceTargetLowering;

// typedefs of containers

// TODO: Switch over to LLVM's ADT container classes.
// http://llvm.org/docs/ProgrammersManual.html#picking-the-right-data-structure-for-a-task
typedef std::string IceString;
typedef std::list<IceInst *> IceInstList;
typedef std::list<IceInstPhi *> IcePhiList;
typedef std::vector<IceOperand *> IceOpList;
typedef std::vector<IceVariable *> IceVarList;
typedef std::vector<IceCfgNode *> IceNodeList;

// The IceOstream class wraps an output stream and an IceCfg pointer,
// so that dump routines have access to the IceCfg object and can
// print labels and variable names.

enum IceVerbose {
  IceV_None = 0,
  IceV_Instructions = 1 << 0,
  IceV_Deleted = 1 << 1,
  IceV_InstNumbers = 1 << 2,
  IceV_Preds = 1 << 3,
  IceV_Succs = 1 << 4,
  IceV_Liveness = 1 << 5,
  IceV_RegManager = 1 << 6,
  IceV_RegOrigins = 1 << 7,
  IceV_LinearScan = 1 << 8,
  IceV_Frame = 1 << 9,
  IceV_Timing = 1 << 10,
  IceV_All = ~IceV_None
};
typedef uint32_t IceVerboseMask;

enum IceLivenessMode {
  // Lightweight version of live-range-end calculation.  Marks the
  // last use of variables whose definition and uses are completely
  // within a single block.
  IceLiveness_LREndLightweight,

  // Full version of live-range-end calculation.  Marks the last uses
  // of variables based on dataflow analysis.  Records the set of
  // live-in and live-out variables for each block.  Identifies and
  // deletes dead instructions (primarily stores).
  IceLiveness_LREndFull,

  // In addition to IceLiveness_LREndFull, also calculate the complete
  // live range for each variable in a form suitable for interference
  // calculation and register allocation.
  IceLiveness_RangesFull
};

// This is a convenience templated class that provides a mapping
// between a parameterized type and small unsigned integers.  The
// small integer is meant to be the index for an IceCfgNode or
// IceVariable.  If the ICE is generated directly from the bitcode,
// this won't be necessary, but it is helpful for manual generation or
// generation from a different IR such as translating from LLVM.
template <typename T> class IceValueTranslation {
public:
  typedef typename std::map<const T, uint32_t> ContainerType;
  IceValueTranslation(void) {}
  void clear(void) { Entries.clear(); }
  uint32_t translate(const T &Value) {
    typename ContainerType::const_iterator Iter = Entries.find(Value);
    if (Iter != Entries.end())
      return Iter->second;
    uint32_t Index = Entries.size();
    Index *= 2; // TODO: this adds holes to the index space for testing; remove.
    Entries[Value] = Index;
    return Index;
  }

private:
  ContainerType Entries;
};

class IceOstream {
public:
  IceOstream(IceCfg *Cfg)
      : Stream(NULL), Cfg(Cfg), Verbose(IceV_Instructions | IceV_Preds),
        CurrentNode(NULL) {}
  bool isVerbose(IceVerboseMask Mask = (IceV_All & ~IceV_Timing)) {
    return Verbose & Mask;
  }
  void setVerbose(IceVerboseMask Mask) { Verbose = Mask; }
  void addVerbose(IceVerboseMask Mask) { Verbose |= Mask; }
  void subVerbose(IceVerboseMask Mask) { Verbose &= ~Mask; }
  void setCurrentNode(const IceCfgNode *Node) { CurrentNode = Node; }
  const IceCfgNode *getCurrentNode(void) const { return CurrentNode; }
  llvm::raw_ostream *Stream;
  IceCfg *const Cfg;

private:
  IceVerboseMask Verbose;
  // CurrentNode is maintained during dumping/emitting just for
  // validating IceVariable::DefOrUseNode.
  const IceCfgNode *CurrentNode;
};

inline IceOstream &operator<<(IceOstream &Str, const char *S) {
  if (Str.Stream)
    *(Str.Stream) << S;
  return Str;
}

inline IceOstream &operator<<(IceOstream &Str, const IceString &S) {
  if (Str.Stream)
    *(Str.Stream) << S;
  return Str;
}

inline IceOstream &operator<<(IceOstream &Str, uint32_t U) {
  if (Str.Stream)
    *(Str.Stream) << U;
  return Str;
}

inline IceOstream &operator<<(IceOstream &Str, int32_t I) {
  if (Str.Stream)
    *(Str.Stream) << I;
  return Str;
}

inline IceOstream &operator<<(IceOstream &Str, uint64_t U) {
  if (Str.Stream)
    *(Str.Stream) << U;
  return Str;
}

inline IceOstream &operator<<(IceOstream &Str, int64_t I) {
  if (Str.Stream)
    *(Str.Stream) << I;
  return Str;
}

inline IceOstream &operator<<(IceOstream &Str, double D) {
  if (Str.Stream)
    *(Str.Stream) << D;
  return Str;
}

// GlobalStr is just for debugging, in situations where the
// IceCfg/IceOstream objects aren't otherwise available.  Not
// thread-safe.
extern IceOstream *GlobalStr;

class IceTimer {
public:
  IceTimer(void) : Start(llvm::TimeRecord::getCurrentTime(false)) {}
  uint64_t getElapsedNs(void) const {
    return getElapsedSec() * 1000 * 1000 * 1000;
  }
  uint64_t getElapsedUs(void) const { return getElapsedSec() * 1000 * 1000; }
  uint64_t getElapsedMs(void) const { return getElapsedSec() * 1000; }
  double getElapsedSec(void) const {
    llvm::TimeRecord End = llvm::TimeRecord::getCurrentTime(false);
    return End.getWallTime() - Start.getWallTime();
  }
  void printElapsedUs(IceOstream &Str, const IceString &Tag) const {
    if (Str.isVerbose(IceV_Timing))
      Str << "# " << getElapsedUs() << " usec " << Tag << "\n";
  }

private:
  const llvm::TimeRecord Start;
};

#endif // _IceDefs_h
