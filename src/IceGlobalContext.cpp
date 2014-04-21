//===- subzero/src/IceGlobalContext.cpp - Global context defs ---*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines aspects of the compilation that persist across
// multiple functions.
//
//===----------------------------------------------------------------------===//

#include "IceDefs.h"
#include "IceTypes.h"
#include "IceCfg.h"
#include "IceGlobalContext.h"
#include "IceOperand.h"
#include "IceTargetLowering.h"

// IceTypePool maps constants of type KeyType (e.g. float) to pointers
// to type ValueType (e.g. IceConstantFloat).  KeyType values are
// compared using memcmp() because of potential NaN values in KeyType
// values.  TODO: allow a custom KeyType comparator for a KeyType
// containing e.g. a non-pooled string.
template <typename KeyType, typename ValueType> class IceTypePool {
  IceTypePool(const IceTypePool &) LLVM_DELETED_FUNCTION;
  IceTypePool &operator=(const IceTypePool &) LLVM_DELETED_FUNCTION;

public:
  IceTypePool() {}
  ValueType *getOrAdd(IceGlobalContext *Ctx, IceType Type, KeyType Key) {
    uint32_t Index = KeyToIndex.translate(TupleType(Type, Key));
    if (Index >= Pool.size()) {
      Pool.resize(Index + 1);
      Pool[Index] = ValueType::create(Ctx, Type, Key);
    }
    ValueType *Constant = Pool[Index];
    assert(Constant);
    return Constant;
  }

private:
  typedef std::pair<IceType, KeyType> TupleType;
  struct TupleCompare {
    bool operator()(const TupleType &A, const TupleType &B) {
      if (A.first != B.first)
        return A.first < B.first;
      return memcmp(&A.second, &B.second, sizeof(KeyType)) < 0;
    }
  };
  IceValueTranslation<TupleType, TupleCompare> KeyToIndex;
  std::vector<ValueType *> Pool;
};

// The global constant pool bundles individual pools of each type of
// interest.
class IceConstantPool {
  IceConstantPool(const IceConstantPool &) LLVM_DELETED_FUNCTION;
  IceConstantPool &operator=(const IceConstantPool &) LLVM_DELETED_FUNCTION;

public:
  IceConstantPool() {}
  IceTypePool<float, IceConstantFloat> Floats;
  IceTypePool<double, IceConstantDouble> Doubles;
  IceTypePool<uint64_t, IceConstantInteger> Integers;
  IceTypePool<IceRelocatableTuple, IceConstantRelocatable> Relocatables;
};

IceGlobalContext::IceGlobalContext(llvm::raw_ostream *OsDump,
                                   llvm::raw_ostream *OsEmit,
                                   IceVerboseMask VerboseMask,
                                   IceTargetArch TargetArch,
                                   IceOptLevel OptLevel, IceString TestPrefix)
    : StrDump(OsDump), StrEmit(OsEmit), VerboseMask(VerboseMask),
      ConstantPool(new IceConstantPool()), TargetArch(TargetArch),
      OptLevel(OptLevel), TestPrefix(TestPrefix) {}

// In this context, name mangling means to rewrite a symbol using a
// given prefix.  For a C++ symbol, nest the original symbol inside
// the "prefix" namespace.  For other symbols, just prepend the
// prefix.
IceString IceGlobalContext::mangleName(const IceString &Name) const {
  // An already-nested name like foo::bar() gets pushed down one
  // level, making it equivalent to Prefix::foo::bar().
  //   _ZN3foo3barExyz ==> _ZN6Prefix3foo3barExyz
  // A non-nested but mangled name like bar() gets nested, making it
  // equivalent to Prefix::bar().
  //   _Z3barxyz ==> ZN6Prefix3barExyz
  // An unmangled, extern "C" style name, gets a simple prefix:
  //   bar ==> Prefixbar
  if (getTestPrefix() == "")
    return Name;

  unsigned PrefixLength = getTestPrefix().length();
  char NameBase[1 + Name.length()];
  const size_t BufLen = 30 + Name.length() + PrefixLength;
  char NewName[BufLen];
  uint32_t BaseLength = 0;

  int ItemsParsed = sscanf(Name.c_str(), "_ZN%s", NameBase);
  if (ItemsParsed == 1) {
    // Transform _ZN3foo3barExyz ==> _ZN6Prefix3foo3barExyz
    //   (splice in "6Prefix")          ^^^^^^^
    snprintf(NewName, BufLen, "_ZN%u%s%s", PrefixLength, getTestPrefix().c_str(),
            NameBase);
    // We ignore the snprintf return value (here and below).  If we
    // somehow miscalculated the output buffer length, the output will
    // be truncated, but it will be truncated consistently for all
    // mangleName() calls on the same input string.
    return NewName;
  }

  ItemsParsed = sscanf(Name.c_str(), "_Z%u%s", &BaseLength, NameBase);
  if (ItemsParsed == 2) {
    // Transform _Z3barxyz ==> ZN6Prefix3barExyz
    //                          ^^^^^^^^    ^
    // (splice in "N6Prefix", and insert "E" after "3bar")
    char OrigName[Name.length()];
    char OrigSuffix[Name.length()];
    strncpy(OrigName, NameBase, BaseLength);
    OrigName[BaseLength] = '\0';
    strcpy(OrigSuffix, NameBase + BaseLength);
    snprintf(NewName, BufLen, "_ZN%u%s%u%sE%s", PrefixLength, getTestPrefix().c_str(),
            BaseLength, OrigName, OrigSuffix);
    return NewName;
  }

  // Transform bar ==> Prefixbar
  //                   ^^^^^^
  return getTestPrefix() + Name;
}

IceGlobalContext::~IceGlobalContext() {}

IceConstant *IceGlobalContext::getConstantInt(IceType Type,
                                              uint64_t ConstantInt64) {
  return ConstantPool->Integers.getOrAdd(this, Type, ConstantInt64);
}

IceConstant *IceGlobalContext::getConstantFloat(float ConstantFloat) {
  return ConstantPool->Floats.getOrAdd(this, IceType_f32, ConstantFloat);
}

IceConstant *IceGlobalContext::getConstantDouble(double ConstantDouble) {
  return ConstantPool->Doubles.getOrAdd(this, IceType_f64, ConstantDouble);
}

IceConstant *IceGlobalContext::getConstantSym(IceType Type, const void *Handle,
                                              int64_t Offset,
                                              const IceString &Name,
                                              bool SuppressMangling) {
  return ConstantPool->Relocatables.getOrAdd(
      this, Type, IceRelocatableTuple(Handle, Offset, Name, SuppressMangling));
}

void IceTimer::printElapsedUs(IceGlobalContext *Ctx,
                              const IceString &Tag) const {
  if (Ctx->isVerbose(IceV_Timing)) {
    // Prefixing with '#' allows timing strings to be included
    // without error in textual assembly output.
    Ctx->StrDump << "# " << getElapsedUs() << " usec " << Tag << "\n";
  }
}
