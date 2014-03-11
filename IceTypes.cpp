/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#include "IceDefs.h"
#include "IceTypes.h"

uint32_t iceTypeWidth(IceType T) {
  switch (T) {
  case IceType_i1:
    return 1;
  case IceType_i8:
    return 1;
  case IceType_i16:
    return 2;
  case IceType_i32:
    return 4;
  case IceType_i64:
    return 8;
  case IceType_f32:
    return 4;
  case IceType_f64:
    return 8;
  case IceType_void:
  case IceType_NUM:
    break;
  }
  assert(0 && "Invalid type for iceTypeWidth()");
  return 0;
}

// ======================== Dump routines ======================== //

IceOstream &operator<<(IceOstream &Str, IceType T) {
  switch (T) {
  case IceType_void:
    Str << "void";
    return Str;
  case IceType_i1:
    Str << "i1";
    return Str;
  case IceType_i8:
    Str << "i8";
    return Str;
  case IceType_i16:
    Str << "i16";
    return Str;
  case IceType_i32:
    Str << "i32";
    return Str;
  case IceType_i64:
    Str << "i64";
    return Str;
  case IceType_f32:
    Str << "float";
    return Str;
  case IceType_f64:
    Str << "double";
    return Str;
  case IceType_NUM:
  default:
    assert(0 && "Invalid type for printing");
    break;
  }
  Str << "???";
  return Str;
}
