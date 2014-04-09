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

// TODO: Accesses to all non-const fields of IceGlobalContext need to
// be synchronized, especially the constant pool, the allocator, and
// the output streams.
class IceGlobalContext {
public:
  IceGlobalContext(llvm::raw_ostream *OsDump, llvm::raw_ostream *OsEmit,
                   IceVerboseMask VerboseMask, IceTargetArch TargetArch,
                   IceOptLevel OptLevel, IceString TestPrefix);
  ~IceGlobalContext();

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
  IceOptLevel getOptLevel() const { return OptLevel; }

  // When emitting assembly, we allow a string to be prepended to
  // names of translated functions.  This makes it easier to create an
  // execution test against a reference translator like llc, with both
  // translators using the same bitcode as input.
  IceString getTestPrefix() const { return TestPrefix; }
  IceString mangleName(const IceString &Name) const;

  // Manage IceConstants.
  // getConstant*() functions are not const because they might add
  // something to the constant pool.
  IceConstant *getConstantInt(IceType Type, uint64_t ConstantInt64);
  IceConstant *getConstantFloat(float Value);
  IceConstant *getConstantDouble(double Value);
  // Returns a symbolic constant.  Handle is currently unused but is
  // reserved to hold something LLVM-specific to facilitate linking.
  IceConstant *getConstantSym(IceType Type, const void *Handle, int64_t Offset,
                              const IceString &Name = "",
                              bool SuppressMangling = false);

  // Allocate data of type T using the global allocator.
  template <typename T> T *allocate() { return Allocator.Allocate<T>(); }

  IceOstream StrDump; // Stream for dumping / diagnostics
  IceOstream StrEmit; // Stream for code emission

private:
  llvm::BumpPtrAllocator Allocator;
  IceVerboseMask VerboseMask;
  llvm::OwningPtr<class IceConstantPool> ConstantPool;
  const IceTargetArch TargetArch;
  const IceOptLevel OptLevel;
  const IceString TestPrefix;
  IceGlobalContext(const IceGlobalContext &) LLVM_DELETED_FUNCTION;
  IceGlobalContext &operator=(const IceGlobalContext &) LLVM_DELETED_FUNCTION;
};

#endif // SUBZERO_SRC_ICEGLOBALCONTEXT_H
