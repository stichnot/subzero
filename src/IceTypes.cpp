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

namespace {

const struct {
  IceType Type;
  size_t TypeWidthInBytes;
  IceString DisplayString;
} TypeAttributes[] = { { IceType_void, 0, "void" },
                       { IceType_i1, 1, "i1" },
                       { IceType_i8, 1, "i8" },
                       { IceType_i16, 2, "i16" },
                       { IceType_i32, 4, "i32" },
                       { IceType_i64, 8, "i64" },
                       { IceType_f32, 4, "float" },
                       { IceType_f64, 8, "double" }, };
const uint32_t TypeAttributesSize =
    sizeof(TypeAttributes) / sizeof(*TypeAttributes);

} // end anonymous namespace

size_t iceTypeWidthInBytes(IceType Type) {
  size_t Width = 0;
  uint32_t Index = static_cast<uint32_t>(Type);
  if (Index < TypeAttributesSize) {
    assert(TypeAttributes[Index].Type == Type);
    Width = TypeAttributes[Index].TypeWidthInBytes;
  }
  assert(Width && "Invalid type for iceTypeWidthInBytes()");
  return Width;
}

// ======================== Dump routines ======================== //

template <> IceOstream &operator<<(IceOstream &Str, const IceType &Type) {
  uint32_t Index = static_cast<uint32_t>(Type);
  if (Index < TypeAttributesSize) {
    assert(TypeAttributes[Index].Type == Type);
    Str << TypeAttributes[Index].DisplayString;
  } else {
    Str << "???";
    assert(0 && "Invalid type for printing");
  }

  return Str;
}
