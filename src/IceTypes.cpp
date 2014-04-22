//===- subzero/src/IceTypes.cpp - Primitive type properties ---------------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines a few attributes of Subzero primitive types.
//
//===----------------------------------------------------------------------===//

#include "IceDefs.h"
#include "IceTypes.h"

namespace Ice {

namespace {

#define X(tag, size, str)                                                      \
  { size, str }                                                                \
  ,

const struct {
  size_t TypeWidthInBytes;
  IceString DisplayString;
} TypeAttributes[] = { ICETYPE_TABLE };

#undef X

const uint32_t TypeAttributesSize =
    sizeof(TypeAttributes) / sizeof(*TypeAttributes);

} // end anonymous namespace

size_t iceTypeWidthInBytes(IceType Type) {
  size_t Width = 0;
  uint32_t Index = static_cast<uint32_t>(Type);
  if (Index < TypeAttributesSize) {
    Width = TypeAttributes[Index].TypeWidthInBytes;
  }
  assert(Width && "Invalid type for iceTypeWidthInBytes()");
  return Width;
}

// ======================== Dump routines ======================== //

template <> IceOstream &operator<<(IceOstream &Str, const IceType &Type) {
  uint32_t Index = static_cast<uint32_t>(Type);
  if (Index < TypeAttributesSize) {
    Str << TypeAttributes[Index].DisplayString;
  } else {
    Str << "???";
    assert(0 && "Invalid type for printing");
  }

  return Str;
}

} // end of namespace Ice
