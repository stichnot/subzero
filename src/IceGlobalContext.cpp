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

class IceConstantPool {
  // TODO: Try to refactor the getOrAdd*() methods.
public:
  IceConstantPool(IceGlobalContext *Ctx) : Ctx(Ctx) {}
  IceConstantRelocatable *getOrAddRelocatable(IceType Type, const void *Handle,
                                              int64_t Offset,
                                              const IceString &Name) {
    uint32_t Index = NameToIndexReloc.translate(
        KeyTypeReloc(Type, std::pair<IceString, int64_t>(Name, Offset)));
    if (Index >= RelocatablePool.size()) {
      RelocatablePool.resize(Index + 1);
      void *Handle = NULL;
      RelocatablePool[Index] = IceConstantRelocatable::create(
          Ctx, Index, Type, Handle, Offset, Name);
    }
    IceConstantRelocatable *Constant = RelocatablePool[Index];
    assert(Constant);
    return Constant;
  }
  uint32_t getRelocatableSize() const { return RelocatablePool.size(); }
  IceConstantRelocatable *getEntry(uint32_t Index) const {
    assert(Index < RelocatablePool.size());
    return RelocatablePool[Index];
  }
  IceConstantInteger *getOrAddInteger(IceType Type, uint64_t Value) {
    uint32_t Index = NameToIndexInteger.translate(KeyTypeInteger(Type, Value));
    if (Index >= IntegerPool.size()) {
      IntegerPool.resize(Index + 1);
      IntegerPool[Index] = IceConstantInteger::create(Ctx, Type, Value);
    }
    IceConstantInteger *Constant = IntegerPool[Index];
    assert(Constant);
    return Constant;
  }
  IceConstantFloat *getOrAddFloat(float Value) {
    uint32_t Index = NameToIndexFloat.translate(Value);
    if (Index >= FloatPool.size()) {
      FloatPool.resize(Index + 1);
      FloatPool[Index] = IceConstantFloat::create(Ctx, IceType_f32, Value);
    }
    IceConstantFloat *Constant = FloatPool[Index];
    assert(Constant);
    return Constant;
  }
  IceConstantDouble *getOrAddDouble(double Value) {
    uint32_t Index = NameToIndexDouble.translate(Value);
    if (Index >= DoublePool.size()) {
      DoublePool.resize(Index + 1);
      DoublePool[Index] = IceConstantDouble::create(Ctx, IceType_f64, Value);
    }
    IceConstantDouble *Constant = DoublePool[Index];
    assert(Constant);
    return Constant;
  }

private:
  // KeyTypeReloc is a triple of {Type, Name, Offset}.
  typedef std::pair<IceType, std::pair<IceString, int64_t> > KeyTypeReloc;
  typedef std::pair<IceType, int64_t> KeyTypeInteger;
  IceGlobalContext *Ctx;
  // Use IceValueTranslation<> to map (Name,Type) pairs to an index.
  IceValueTranslation<KeyTypeReloc> NameToIndexReloc;
  IceValueTranslation<KeyTypeInteger> NameToIndexInteger;
  IceValueTranslation<float> NameToIndexFloat;
  IceValueTranslation<double> NameToIndexDouble;
  std::vector<IceConstantRelocatable *> RelocatablePool;
  std::vector<IceConstantInteger *> IntegerPool;
  std::vector<IceConstantFloat *> FloatPool;
  std::vector<IceConstantDouble *> DoublePool;
};

IceGlobalContext::IceGlobalContext(llvm::raw_ostream *OsDump,
                                   llvm::raw_ostream *OsEmit,
                                   IceVerboseMask VerboseMask,
                                   IceTargetArch TargetArch,
                                   IceOptLevel OptLevel, IceString TestPrefix)
    : StrDump(OsDump), StrEmit(OsEmit), VerboseMask(VerboseMask),
      ConstantPool(new IceConstantPool(this)), TargetArch(TargetArch),
      OptLevel(OptLevel), TestPrefix(TestPrefix) {}

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

IceGlobalContext::~IceGlobalContext() {}

IceConstant *IceGlobalContext::getConstantInt(IceType Type,
                                              uint64_t ConstantInt64) {
  return ConstantPool->getOrAddInteger(Type, ConstantInt64);
}

IceConstant *IceGlobalContext::getConstantFloat(float ConstantFloat) {
  return ConstantPool->getOrAddFloat(ConstantFloat);
}

IceConstant *IceGlobalContext::getConstantDouble(double ConstantDouble) {
  return ConstantPool->getOrAddDouble(ConstantDouble);
}

IceConstant *IceGlobalContext::getConstantSym(IceType Type, const void *Handle,
                                              int64_t Offset,
                                              const IceString &Name,
                                              bool SuppressMangling) {
  IceConstantRelocatable *Const =
      ConstantPool->getOrAddRelocatable(Type, Handle, Offset, Name);
  Const->setSuppressMangling(SuppressMangling);
  return Const;
}

void IceTimer::printElapsedUs(IceGlobalContext *Ctx,
                              const IceString &Tag) const {
  if (Ctx->isVerbose(IceV_Timing)) {
    // Prefixing with '#' allows timing strings to be included
    // without error in textual assembly output.
    Ctx->StrDump << "# " << getElapsedUs() << " usec " << Tag << "\n";
  }
}
