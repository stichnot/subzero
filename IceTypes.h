// -*- Mode: c++ -*-
/* Copyright 2014 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can
 * be found in the LICENSE file.
 */

#ifndef SUBZERO_ICETYPES_H
#define SUBZERO_ICETYPES_H

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
  IceTarget_X8632Fast,
  IceTarget_X8664,
  IceTarget_ARM32,
  IceTarget_ARM64
};

uint32_t iceTypeWidth(IceType T);
IceOstream &operator<<(class IceOstream &Str, IceType T);

#endif // SUBZERO_ICETYPES_H
