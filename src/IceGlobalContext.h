//===- subzero/src/GlobalContext.h - Global context defs -----*- C++ -*-===//
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

namespace Ice {

// TODO: Accesses to all non-const fields of GlobalContext need to
// be synchronized, especially the constant pool, the allocator, and
// the output streams.
class GlobalContext {
public:
  GlobalContext(llvm::raw_ostream *OsDump, llvm::raw_ostream *OsEmit,
                IceVerboseMask VerboseMask, IceTargetArch TargetArch,
                IceOptLevel OptLevel, IceString TestPrefix);
  ~GlobalContext();

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

  Ostream &getStrDump() { return StrDump; }
  Ostream &getStrEmit() { return StrEmit; }

  IceTargetArch getTargetArch() const { return TargetArch; }
  IceOptLevel getOptLevel() const { return OptLevel; }

  // When emitting assembly, we allow a string to be prepended to
  // names of translated functions.  This makes it easier to create an
  // execution test against a reference translator like llc, with both
  // translators using the same bitcode as input.
  IceString getTestPrefix() const { return TestPrefix; }
  IceString mangleName(const IceString &Name) const;

  // Manage Constants.
  // getConstant*() functions are not const because they might add
  // something to the constant pool.
  Constant *getConstantInt(Type Ty, uint64_t ConstantInt64);
  Constant *getConstantFloat(float Value);
  Constant *getConstantDouble(double Value);
  // Returns a symbolic constant.
  Constant *getConstantSym(Type Ty, int64_t Offset, const IceString &Name = "",
                           bool SuppressMangling = false);

  // Allocate data of type T using the global allocator.
  template <typename T> T *allocate() { return Allocator.Allocate<T>(); }

private:
  Ostream StrDump; // Stream for dumping / diagnostics
  Ostream StrEmit; // Stream for code emission

  llvm::BumpPtrAllocator Allocator;
  IceVerboseMask VerboseMask;
  llvm::OwningPtr<class ConstantPool> ConstPool;
  const IceTargetArch TargetArch;
  const IceOptLevel OptLevel;
  const IceString TestPrefix;
  GlobalContext(const GlobalContext &) LLVM_DELETED_FUNCTION;
  GlobalContext &operator=(const GlobalContext &) LLVM_DELETED_FUNCTION;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEGLOBALCONTEXT_H
