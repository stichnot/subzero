//===- subzero/src/IceTypes.h - Primitive ICE types -------------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares a few properties of the primitive types allowed
// in Subzero.  Every Subzero source file is expected to include
// IceTypes.h.
//
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICETYPES_H
#define SUBZERO_SRC_ICETYPES_H

namespace Ice {

#define ICETYPE_TABLE                                                          \
  /* enum value,  size in bytes, printable string */                           \
  X(IceType_void, 0, "void") X(IceType_i1, 1, "i1") X(IceType_i8, 1, "i8")     \
      X(IceType_i16, 2, "i16") X(IceType_i32, 4, "i32")                        \
      X(IceType_i64, 8, "i64") X(IceType_f32, 4, "float")                      \
      X(IceType_f64, 8, "double") X(IceType_NUM, 0, "<invalid>")

#define X(tag, size, str) tag,

enum IceType {
  ICETYPE_TABLE
};
#undef X

enum IceTargetArch {
  IceTarget_X8632,
  IceTarget_X8664,
  IceTarget_ARM32,
  IceTarget_ARM64
};

enum IceOptLevel {
  IceOpt_m1,
  IceOpt_0,
  IceOpt_1,
  IceOpt_2
};

size_t typeWidthInBytes(IceType Type);

template <> IceOstream &operator<<(class IceOstream &Str, const IceType &Type);

} // end of namespace Ice

#endif // SUBZERO_SRC_ICETYPES_H
