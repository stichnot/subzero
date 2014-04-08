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
#include "IceTargetLowering.h"

#if 0
class IceConstantPool {
public:
  IceConstantPool(IceGlobalContext *Ctx) : Ctx(Ctx) {}
  IceConstantRelocatable *getOrAddRelocatable(IceType Type, const void *Handle,
                                              int64_t Offset,
                                              const IceString &Name) {
    uint32_t Index = NameToIndex.translate(
        KeyType(Type, std::pair<IceString, int64_t>(Name, Offset)));
    if (Index >= RelocatablePool.size()) {
      RelocatablePool.resize(Index + 1);
      void *Handle = NULL;
      RelocatablePool[Index] = IceConstantRelocatable::create(
          Cfg, Index, Type, Handle, Offset, Name);
    }
    IceConstantRelocatable *Constant = RelocatablePool[Index];
    assert(Constant);
    return Constant;
  }
  uint32_t getSize() const { return RelocatablePool.size(); }
  IceConstantRelocatable *getEntry(uint32_t Index) const {
    assert(Index < RelocatablePool.size());
    return RelocatablePool[Index];
  }

private:
  // KeyType is a triple of {Type, Name, Offset}.
  typedef std::pair<IceType, std::pair<IceString, int64_t> > KeyType;
  IceGlobalContext *Ctx;
  // Use IceValueTranslation<> to map (Name,Type) pairs to an index.
  IceValueTranslation<KeyType> NameToIndex;
  std::vector<IceConstantRelocatable *> RelocatablePool;
};
#endif

IceGlobalContext::IceGlobalContext(llvm::raw_ostream *OsDump,
                                   llvm::raw_ostream *OsEmit,
                                   IceVerboseMask VerboseMask,
                                   IceTargetArch TargetArch,
                                   IceString TestPrefix)
    : StrDump(OsDump), StrEmit(OsEmit), VerboseMask(VerboseMask),
      TargetArch(TargetArch), TestPrefix(TestPrefix) {}

// In this context, name mangling means to rewrite a symbol using a
// given prefix.  For a C++ symbol, we'd like to demangle it, prepend
// the prefix to the original symbol, and remangle it for C++.  For
// other symbols, just prepend the prefix.
IceString IceGlobalContext::mangleName(const IceString &Name) const {
  // TODO: This handles only non-nested C++ symbols, and not ones that
  // begin with "_ZN".  For the latter, we need to rewrite only the
  // last name component.
  if (getTestPrefix() == "")
    return Name;
  IceString Default = getTestPrefix() + Name;
  uint32_t BaseLength = 0;
  char Buffer[1 + Name.length()];
  int ItemsParsed = sscanf(Name.c_str(), "_Z%u%s", &BaseLength, Buffer);
  if (ItemsParsed != 2)
    return Default;
  if (strlen(Buffer) < BaseLength)
    return Default;

  BaseLength += getTestPrefix().length();
  char NewNumber[30 + Name.length() + getTestPrefix().length()];
  sprintf(NewNumber, "_Z%u%s%s", BaseLength, getTestPrefix().c_str(), Buffer);
  return NewNumber;
}

void IceTimer::printElapsedUs(IceGlobalContext *Context,
                              const IceString &Tag) const {
  if (Context->isVerbose(IceV_Timing)) {
    // Prefixing with '#' allows timing strings to be included
    // without error in textual assembly output.
    Context->StrDump << "# " << getElapsedUs() << " usec " << Tag << "\n";
  }
}
