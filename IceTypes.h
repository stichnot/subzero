// -*- Mode: c++ -*-
#ifndef _IceTypes_h
#define _IceTypes_h

#include "IceDefs.h"

enum IceType {
  IceType_void,
  IceType_i1,
  IceType_i8,
  IceType_i16,
  IceType_i32,
  IceType_i64,
  IceType_f32,
  IceType_f64,
};

IceOstream& operator<<(IceOstream &Str, IceType T);

#endif // _IceTypes_h
