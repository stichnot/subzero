//===- subzero/src/IceGlobalContext.h - Global context defs -----*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares aspects of the compilation that persist across
// multiple functions.
//
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEGLOBALCONTEXT_H
#define SUBZERO_SRC_ICEGLOBALCONTEXT_H

#include "llvm/ADT/OwningPtr.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/raw_ostream.h"

#include "IceDefs.h"
#include "IceTypes.h"

class IceGlobalContext {
public:
  IceGlobalContext(llvm::raw_ostream *OsDump, llvm::raw_ostream *OsEmit,
                   IceVerboseMask VerboseMask, IceTargetArch TargetArch,
                   IceString TestPrefix);

  // Returns true if any of the specified options in the verbose mask
  // are set.  If the argument is omitted, it checks if any verbose
  // options at all are set.  IceV_Timing is treated specially, so
  // that running with just IceV_Timing verbosity doesn't trigger an
  // avalanche of extra output.
  bool isVerbose(IceVerboseMask Mask = (IceV_All & ~IceV_Timing)) const {
    return VerboseMask & Mask;
  }
  void setVerbose(IceVerboseMask Mask) { VerboseMask = Mask; }
  void addVerbose(IceVerboseMask Mask) { VerboseMask |= Mask; }
  void subVerbose(IceVerboseMask Mask) { VerboseMask &= ~Mask; }

  IceTargetArch getTargetArch() const { return TargetArch; }

  // When emitting assembly, we allow a string to be prepended to
  // names of translated functions.  This makes it easier to create an
  // execution test against a reference translator like llc, with both
  // translators using the same bitcode as input.
  IceString getTestPrefix() const { return TestPrefix; }
  IceString mangleName(const IceString &Name) const;

  // Allocate data of type T using the global allocator.
  template <typename T> T *allocate() { return Allocator.Allocate<T>(); }

  IceOstream StrDump; // Stream for dumping / diagnostics
  IceOstream StrEmit; // Stream for code emission

private:
  llvm::BumpPtrAllocator Allocator;
  IceVerboseMask VerboseMask;
  // llvm::OwningPtr<class IceConstantPool> ConstantPool;
  IceTargetArch TargetArch;
  IceString TestPrefix;
};

#endif // SUBZERO_SRC_ICEGLOBALCONTEXT_H
