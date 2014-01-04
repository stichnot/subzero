#include "IceTypes.h"

// ======================== Dump routines ======================== //

IceOstream& operator<<(IceOstream &Str, IceType T) {
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
  }
  Str << "???";
  return Str;
}
