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

enum IceType {
  IceType_void,
  IceType_i1,
  IceType_i8,
  IceType_i16,
  IceType_i32,
  IceType_i64,
  IceType_f32,
  IceType_f64,
  IceType_NUM
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

size_t iceTypeWidthInBytes(IceType Type);

template <> IceOstream &operator<<(class IceOstream &Str, const IceType &Type);

#endif // SUBZERO_SRC_ICETYPES_H
