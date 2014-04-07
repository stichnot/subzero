//===- subzero/src/IceDefs.h - Common Subzero declaraions -------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares various useful types and classes that have
// widespread use across Subzero.  Every Subzero source file is
// expected to include IceDefs.h.
//
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEDEFS_H
#define SUBZERO_SRC_ICEDEFS_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h> // sprintf

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/SmallBitVector.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Timer.h"

class IceCfg;
class IceCfgNode;
class IceConstant;
class IceInst;
class IceInstPhi;
class IceInstTarget;
class IceLiveRange;
class IceLiveness;
class IceOperand;
class IceTargetLowering;
class IceVariable;

// TODO: Switch over to LLVM's ADT container classes.
// http://llvm.org/docs/ProgrammersManual.html#picking-the-right-data-structure-for-a-task
typedef std::string IceString;
typedef std::list<IceInst *> IceInstList;
typedef std::list<IceInstPhi *> IcePhiList;
typedef std::vector<IceOperand *> IceOpList;
typedef std::vector<IceVariable *> IceVarList;
typedef std::vector<IceCfgNode *> IceNodeList;

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
// between a parameterized type and small unsigned integers.
template <typename T> class IceValueTranslation {
public:
  typedef typename std::map<const T, uint32_t> ContainerType;
  IceValueTranslation() {}
  void clear() { Entries.clear(); }
  uint32_t translate(const T &Value) {
    typename ContainerType::const_iterator Iter = Entries.find(Value);
    if (Iter != Entries.end())
      return Iter->second;
    uint32_t Index = Entries.size();
    Entries[Value] = Index;
    return Index;
  }

private:
  ContainerType Entries;
};

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

// The IceOstream class wraps an output stream and an IceCfg pointer,
// so that dump routines have access to the IceCfg object and can
// print labels and variable names.

class IceOstream {
public:
  IceOstream(IceCfg *Cfg)
      : Stream(NULL), Cfg(Cfg), Verbose(IceV_Instructions | IceV_Preds),
        CurrentNode(NULL) {}

  // Returns true if any of the specified options in the verbose mask
  // are set.  If the argument is omitted, it checks if any verbose
  // options at all are set.  IceV_Timing is treated specially, so
  // that running with just IceV_Timing verbosity doesn't trigger an
  // avalanche of extra output.
  bool isVerbose(IceVerboseMask Mask = (IceV_All & ~IceV_Timing)) {
    return Verbose & Mask;
  }
  void setVerbose(IceVerboseMask Mask) { Verbose = Mask; }
  void addVerbose(IceVerboseMask Mask) { Verbose |= Mask; }
  void subVerbose(IceVerboseMask Mask) { Verbose &= ~Mask; }

  void setCurrentNode(const IceCfgNode *Node) { CurrentNode = Node; }
  const IceCfgNode *getCurrentNode() const { return CurrentNode; }

  llvm::raw_ostream *Stream;
  IceCfg *const Cfg;

private:
  IceVerboseMask Verbose;
  // CurrentNode is maintained during dumping/emitting just for
  // validating IceVariable::DefNode.  Normally, a traversal over
  // IceCfgNodes maintains this, but before global operations like
  // register allocation, setCurrentNode(NULL) should be called to
  // avoid spurious validation failures.  TODO: Move this out of
  // IceOstream since streams will be split for dumping/emitting, and
  // this obviously won't be thread-safe.
  const IceCfgNode *CurrentNode;
};

template <typename T>
inline IceOstream &operator<<(IceOstream &Str, const T &Val) {
  if (Str.Stream)
    (*Str.Stream) << Val;
  return Str;
}

// GlobalStr is just for debugging, in situations where the
// IceCfg/IceOstream objects aren't otherwise available.  Not
// thread-safe.
extern IceOstream *GlobalStr;

class IceTimer {
public:
  IceTimer() : Start(llvm::TimeRecord::getCurrentTime(false)) {}
  uint64_t getElapsedNs() const { return getElapsedSec() * 1000 * 1000 * 1000; }
  uint64_t getElapsedUs() const { return getElapsedSec() * 1000 * 1000; }
  uint64_t getElapsedMs() const { return getElapsedSec() * 1000; }
  double getElapsedSec() const {
    llvm::TimeRecord End = llvm::TimeRecord::getCurrentTime(false);
    return End.getWallTime() - Start.getWallTime();
  }
  void printElapsedUs(IceOstream &Str, const IceString &Tag) const {
    if (Str.isVerbose(IceV_Timing)) {
      // Prefixing with '#' allows timing strings to be included
      // without error in textual assembly output.
      Str << "# " << getElapsedUs() << " usec " << Tag << "\n";
    }
  }

private:
  const llvm::TimeRecord Start;
};

#endif // SUBZERO_SRC_ICEDEFS_H
