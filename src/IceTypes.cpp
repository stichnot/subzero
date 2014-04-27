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

const struct {
  size_t TypeWidthInBytes;
  size_t TypeAlignInBytes;
  IceString DisplayString;
} TypeAttributes[] = {
#define X(tag, size, align, str)                                               \
  { size, align, str }                                                         \
  ,
    ICETYPE_TABLE
#undef X
  };

#undef X

const size_t TypeAttributesSize =
    sizeof(TypeAttributes) / sizeof(*TypeAttributes);

} // end anonymous namespace

size_t typeWidthInBytes(Type Ty) {
  size_t Width = 0;
  size_t Index = static_cast<size_t>(Ty);
  if (Index < TypeAttributesSize) {
    Width = TypeAttributes[Index].TypeWidthInBytes;
  }
  assert(Width && "Invalid type for iceTypeWidthInBytes()");
  return Width;
}

size_t typeAlignInBytes(Type Ty) {
  size_t Align = 0;
  size_t Index = static_cast<size_t>(Ty);
  if (Index < TypeAttributesSize) {
    Align = TypeAttributes[Index].TypeAlignInBytes;
  }
  assert(Align && "Invalid type for iceTypeAlignInBytes()");
  return Align;
}

// ======================== Dump routines ======================== //

template <> Ostream &operator<<(Ostream &Str, const Type &Ty) {
  size_t Index = static_cast<size_t>(Ty);
  if (Index < TypeAttributesSize) {
    Str << TypeAttributes[Index].DisplayString;
  } else {
    Str << "???";
    assert(0 && "Invalid type for printing");
  }

  return Str;
}

} // end of namespace Ice
