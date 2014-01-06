// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef _IceDefs_h
#define _IceDefs_h

#include <assert.h>
#include <stdint.h>

#include <list>
#include <ostream>
#include <string>
#include <vector>

class IceCfg;
class IceCfgNode;
class IceInst;
class IceInstPhi;
class IceOperand;
class IceVariable;
class IceConstant;
class IceRegManager;

// typedefs of containers

// TODO: Switch over to LLVM's ADT container classes.  Also, make use
// of LLVM's isa<>, cast<>, and dyn_cast<> templates.
// http://llvm.org/docs/ProgrammersManual.html#picking-the-right-data-structure-for-a-task
// http://llvm.org/docs/ProgrammersManual.html#isa
typedef std::string IceString;
typedef std::list<IceInst *> IceInstList;
typedef std::vector<IceOperand *> IceOpList;
typedef std::vector<IceVariable *> IceVarList;
typedef std::vector<uint32_t> IceEdgeList;
typedef std::vector<IceCfgNode *> IceNodeList;

// The IceOstream class wraps a std::ostream and an IceCfg pointer, so
// that dump routines have access to the IceCfg object and can print
// labels and variable names.
class IceOstream {
public:
  IceOstream(std::ostream &Stream, IceCfg *Cfg) : Stream(Stream), Cfg(Cfg),
                                                  Verbose(true) {}
  bool isVerbose(void) const { return Verbose; }
  void setVerbose(bool NewValue) { Verbose = NewValue; }
  // TODO: Use LLVM's raw_ostream instead.
  // http://llvm.org/docs/CodingStandards.html#use-raw-ostream
  std::ostream &Stream;
  IceCfg *const Cfg;
private:
  bool Verbose;
};

inline IceOstream& operator<<(IceOstream &Str, const char *S) {
  Str.Stream << S;
  return Str;
}

inline IceOstream& operator<<(IceOstream &Str, const IceString &S) {
  Str.Stream << S;
  return Str;
}

inline IceOstream& operator<<(IceOstream &Str, uint32_t U) {
  Str.Stream << U;
  return Str;
}

#endif // _IceDefs_h
