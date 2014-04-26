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

#include "IceTypes.def"

namespace Ice {

enum Type {
#define X(tag, size, str) tag,
  ICETYPE_TABLE
#undef X
};

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

size_t typeWidthInBytes(Type Ty);

template <> Ostream &operator<<(class Ostream &Str, const Type &Ty);

} // end of namespace Ice

#endif // SUBZERO_SRC_ICETYPES_H
