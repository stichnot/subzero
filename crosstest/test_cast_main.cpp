// abtest.py --test=test_cast.cpp --test=test_cast_to_u1.ll --driver=test_cast_main.cpp --prefix=Subzero_ --output=test_cast

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "test_cast.h"

namespace Subzero_ {
#include "test_cast.h"
}

#define XSTR(s) STR(s)
#define STR(s) #s
#define COMPARE(FromCName, FromIceName, FromPrintf, ToCName, ToIceName,        \
                ToPrintf, Input)                                               \
  do {                                                                         \
    ToCName ResultSz, ResultLlc;                                               \
    ResultLlc = cast##FromIceName##To##ToIceName(Input);                       \
    ResultSz = Subzero_::cast##FromIceName##To##ToIceName(Input);              \
    ++TotalTests;                                                              \
    if (!memcmp(&ResultLlc, &ResultSz, sizeof(ToCName))) {                     \
      ++Passes;                                                                \
    } else {                                                                   \
      ++Failures;                                                              \
      printf("cast" XSTR(FromIceName) "To" XSTR(ToIceName) "(" XSTR(           \
                 FromPrintf) "): "                                             \
                             "sz=" ToPrintf " llc=" ToPrintf "\n",             \
             Input, ResultSz, ResultLlc);                                      \
    }                                                                          \
  } while (0)

int main(int argc, char **argv) {
  unsigned TotalTests = 0;
  unsigned Passes = 0;
  unsigned Failures = 0;

  static const bool ValsUi1[] = { false, true };
  static const unsigned NumValsUi1 = sizeof(ValsUi1) / sizeof(*ValsUi1);
  static const unsigned char ValsUi8[] = { 0,    1,    0x7e, 0x7f,
                                           0x80, 0x81, 0xfe, 0xff };
  static const unsigned NumValsUi8 = sizeof(ValsUi8) / sizeof(*ValsUi8);

  static const signed char ValsSi8[] = { 0,    1,    0x7e, 0x7f,
                                         0x80, 0x81, 0xfe, 0xff };
  static const unsigned NumValsSi8 = sizeof(ValsSi8) / sizeof(*ValsSi8);

  static const unsigned short ValsUi16[] = { 0,      1,      0x7e,   0x7f,
                                             0x80,   0x81,   0xfe,   0xff,
                                             0x7ffe, 0x7fff, 0x8000, 0x8001,
                                             0xfffe, 0xffff };
  static const unsigned NumValsUi16 = sizeof(ValsUi16) / sizeof(*ValsUi16);

  static const short ValsSi16[] = { 0,      1,      0x7e,   0x7f,   0x80,
                                    0x81,   0xfe,   0xff,   0x7ffe, 0x7fff,
                                    0x8000, 0x8001, 0xfffe, 0xffff };
  static const unsigned NumValsSi16 = sizeof(ValsSi16) / sizeof(*ValsSi16);

  static const unsigned ValsUi32[] = {
    0,          1,          0x7e,       0x7f,       0x80,
    0x81,       0xfe,       0xff,       0x7ffe,     0x7fff,
    0x8000,     0x8001,     0xfffe,     0xffff,     0x7ffffffe,
    0x7fffffff, 0x80000000, 0x80000001, 0xfffffffe, 0xffffffff
  };
  static const unsigned NumValsUi32 = sizeof(ValsUi32) / sizeof(*ValsUi32);

  static const unsigned ValsSi32[] = {
    0,          1,          0x7e,       0x7f,       0x80,
    0x81,       0xfe,       0xff,       0x7ffe,     0x7fff,
    0x8000,     0x8001,     0xfffe,     0xffff,     0x7ffffffe,
    0x7fffffff, 0x80000000, 0x80000001, 0xfffffffe, 0xffffffff
  };
  static const unsigned NumValsSi32 = sizeof(ValsSi32) / sizeof(*ValsSi32);

  static const uint64_t ValsUi64[] = {
    0,                     1,                     0x7e,
    0x7f,                  0x80,                  0x81,
    0xfe,                  0xff,                  0x7ffe,
    0x7fff,                0x8000,                0x8001,
    0xfffe,                0xffff,                0x7ffffffe,
    0x7fffffff,            0x80000000,            0x80000001,
    0xfffffffe,            0xffffffff,            0x100000000ull,
    0x100000001ull,        0x7ffffffffffffffeull, 0x7fffffffffffffffull,
    0x8000000000000000ull, 0x8000000000000001ull, 0xfffffffffffffffeull,
    0xffffffffffffffffull
  };
  static const unsigned NumValsUi64 = sizeof(ValsUi64) / sizeof(*ValsUi64);

  static const int64_t ValsSi64[] = {
    0,                    1,                    0x7e,
    0x7f,                 0x80,                 0x81,
    0xfe,                 0xff,                 0x7ffe,
    0x7fff,               0x8000,               0x8001,
    0xfffe,               0xffff,               0x7ffffffe,
    0x7fffffff,           0x80000000,           0x80000001,
    0xfffffffe,           0xffffffff,           0x100000000ll,
    0x100000001ll,        0x7ffffffffffffffell, 0x7fffffffffffffffll,
    0x8000000000000000ll, 0x8000000000000001ll, 0xfffffffffffffffell,
    0xffffffffffffffffll
  };
  static const unsigned NumValsSi64 = sizeof(ValsSi64) / sizeof(*ValsSi64);

  static const float ValsF32[] = {
    0,                    1,                    0x7e,
    0x7f,                 0x80,                 0x81,
    0xfe,                 0xff,                 0x7ffe,
    0x7fff,               0x8000,               0x8001,
    0xfffe,               0xffff,               0x7ffffffe,
    0x7fffffff,           0x80000000,           0x80000001,
    0xfffffffe,           0xffffffff,           0x100000000ll,
    0x100000001ll,        0x7ffffffffffffffell, 0x7fffffffffffffffll,
    0x8000000000000000ll, 0x8000000000000001ll, 0xfffffffffffffffell,
    0xffffffffffffffffll
  };
  static const unsigned NumValsF32 = sizeof(ValsF32) / sizeof(*ValsF32);

  static const double ValsF64[] = {
    0,                    1,                    0x7e,
    0x7f,                 0x80,                 0x81,
    0xfe,                 0xff,                 0x7ffe,
    0x7fff,               0x8000,               0x8001,
    0xfffe,               0xffff,               0x7ffffffe,
    0x7fffffff,           0x80000000,           0x80000001,
    0xfffffffe,           0xffffffff,           0x100000000ll,
    0x100000001ll,        0x7ffffffffffffffell, 0x7fffffffffffffffll,
    0x8000000000000000ll, 0x8000000000000001ll, 0xfffffffffffffffell,
    0xffffffffffffffffll
  };
  static const unsigned NumValsF64 = sizeof(ValsF64) / sizeof(*ValsF64);

  for (unsigned i = 0; i < NumValsUi1; ++i) {
    {
      bool Val = ValsUi1[i];
      COMPARE(bool, Ui1, "%u", bool, Ui1, "%u", Val);
      COMPARE(bool, Ui1, "%u", unsigned char, Ui8, "%u", Val);
      COMPARE(bool, Ui1, "%u", signed char, Si8, "%d", Val);
      COMPARE(bool, Ui1, "%u", unsigned short, Ui16, "%u", Val);
      COMPARE(bool, Ui1, "%u", short, Si16, "%d", Val);
      COMPARE(bool, Ui1, "%u", uint32_t, Ui32, "%u", Val);
      COMPARE(bool, Ui1, "%u", int32_t, Si32, "%d", Val);
      COMPARE(bool, Ui1, "%u", uint64_t, Ui64, "%llu", Val);
      COMPARE(bool, Ui1, "%u", int64_t, Si64, "%lld", Val);
      COMPARE(bool, Ui1, "%u", float, F32, "%f", Val);
      COMPARE(bool, Ui1, "%u", double, F64, "%f", Val);
    }
  }
  for (unsigned i = 0; i < NumValsUi8; ++i) {
    {
      unsigned char Val = ValsUi8[i];
      COMPARE(unsigned char, Ui8, "%u", bool, Ui1, "%u", Val);
      COMPARE(unsigned char, Ui8, "%u", unsigned char, Ui8, "%u", Val);
      COMPARE(unsigned char, Ui8, "%u", signed char, Si8, "%d", Val);
      COMPARE(unsigned char, Ui8, "%u", unsigned short, Ui16, "%u", Val);
      COMPARE(unsigned char, Ui8, "%u", short, Si16, "%d", Val);
      COMPARE(unsigned char, Ui8, "%u", uint32_t, Ui32, "%u", Val);
      COMPARE(unsigned char, Ui8, "%u", int32_t, Si32, "%d", Val);
      COMPARE(unsigned char, Ui8, "%u", uint64_t, Ui64, "%llu", Val);
      COMPARE(unsigned char, Ui8, "%u", int64_t, Si64, "%lld", Val);
      COMPARE(unsigned char, Ui8, "%u", float, F32, "%f", Val);
      COMPARE(unsigned char, Ui8, "%u", double, F64, "%f", Val);
    }
  }
  for (unsigned i = 0; i < NumValsSi8; ++i) {
    {
      signed char Val = ValsSi8[i];
      COMPARE(signed char, Si8, "%d", bool, Ui1, "%u", Val);
      COMPARE(signed char, Si8, "%d", unsigned char, Ui8, "%u", Val);
      COMPARE(signed char, Si8, "%d", signed char, Si8, "%d", Val);
      COMPARE(signed char, Si8, "%d", unsigned short, Ui16, "%u", Val);
      COMPARE(signed char, Si8, "%d", short, Si16, "%d", Val);
      COMPARE(signed char, Si8, "%d", uint32_t, Ui32, "%u", Val);
      COMPARE(signed char, Si8, "%d", int32_t, Si32, "%d", Val);
      COMPARE(signed char, Si8, "%d", uint64_t, Ui64, "%llu", Val);
      COMPARE(signed char, Si8, "%d", int64_t, Si64, "%lld", Val);
      COMPARE(signed char, Si8, "%d", float, F32, "%f", Val);
      COMPARE(signed char, Si8, "%d", double, F64, "%f", Val);
    }
  }
  for (unsigned i = 0; i < NumValsUi16; ++i) {
    {
      unsigned short Val = ValsUi16[i];
      COMPARE(unsigned short, Ui16, "%u", bool, Ui1, "%u", Val);
      COMPARE(unsigned short, Ui16, "%u", unsigned char, Ui8, "%u", Val);
      COMPARE(unsigned short, Ui16, "%u", signed char, Si8, "%d", Val);
      COMPARE(unsigned short, Ui16, "%u", unsigned short, Ui16, "%u", Val);
      COMPARE(unsigned short, Ui16, "%u", short, Si16, "%d", Val);
      COMPARE(unsigned short, Ui16, "%u", uint32_t, Ui32, "%u", Val);
      COMPARE(unsigned short, Ui16, "%u", int32_t, Si32, "%d", Val);
      COMPARE(unsigned short, Ui16, "%u", uint64_t, Ui64, "%llu", Val);
      COMPARE(unsigned short, Ui16, "%u", int64_t, Si64, "%lld", Val);
      COMPARE(unsigned short, Ui16, "%u", float, F32, "%f", Val);
      COMPARE(unsigned short, Ui16, "%u", double, F64, "%f", Val);
    }
  }
  for (unsigned i = 0; i < NumValsSi16; ++i) {
    {
      signed char Val = ValsSi16[i];
      COMPARE(short, Si16, "%d", bool, Ui1, "%u", Val);
      COMPARE(short, Si16, "%d", unsigned char, Ui8, "%u", Val);
      COMPARE(short, Si16, "%d", signed char, Si8, "%d", Val);
      COMPARE(short, Si16, "%d", unsigned short, Ui16, "%u", Val);
      COMPARE(short, Si16, "%d", short, Si16, "%d", Val);
      COMPARE(short, Si16, "%d", uint32_t, Ui32, "%u", Val);
      COMPARE(short, Si16, "%d", int32_t, Si32, "%d", Val);
      COMPARE(short, Si16, "%d", uint64_t, Ui64, "%llu", Val);
      COMPARE(short, Si16, "%d", int64_t, Si64, "%lld", Val);
      COMPARE(short, Si16, "%d", float, F32, "%f", Val);
      COMPARE(short, Si16, "%d", double, F64, "%f", Val);
    }
  }
  for (unsigned i = 0; i < NumValsUi32; ++i) {
    {
      unsigned Val = ValsUi32[i];
      COMPARE(unsigned, Ui32, "%u", bool, Ui1, "%u", Val);
      COMPARE(unsigned, Ui32, "%u", unsigned char, Ui8, "%u", Val);
      COMPARE(unsigned, Ui32, "%u", signed char, Si8, "%d", Val);
      COMPARE(unsigned, Ui32, "%u", unsigned short, Ui16, "%u", Val);
      COMPARE(unsigned, Ui32, "%u", short, Si16, "%d", Val);
      COMPARE(unsigned, Ui32, "%u", uint32_t, Ui32, "%u", Val);
      COMPARE(unsigned, Ui32, "%u", int32_t, Si32, "%d", Val);
      COMPARE(unsigned, Ui32, "%u", uint64_t, Ui64, "%llu", Val);
      COMPARE(unsigned, Ui32, "%u", int64_t, Si64, "%lld", Val);
      COMPARE(unsigned, Ui32, "%u", float, F32, "%f", Val);
      COMPARE(unsigned, Ui32, "%u", double, F64, "%f", Val);
      COMPARE(unsigned, bits_Ui32, "%u", float, F32, "%f", Val);
    }
  }
  for (unsigned i = 0; i < NumValsSi32; ++i) {
    {
      int Val = ValsSi32[i];
      COMPARE(int, Si32, "%d", bool, Ui1, "%u", Val);
      COMPARE(int, Si32, "%d", unsigned char, Ui8, "%u", Val);
      COMPARE(int, Si32, "%d", signed char, Si8, "%d", Val);
      COMPARE(int, Si32, "%d", unsigned short, Ui16, "%u", Val);
      COMPARE(int, Si32, "%d", short, Si16, "%d", Val);
      COMPARE(int, Si32, "%d", uint32_t, Ui32, "%u", Val);
      COMPARE(int, Si32, "%d", int32_t, Si32, "%d", Val);
      COMPARE(int, Si32, "%d", uint64_t, Ui64, "%llu", Val);
      COMPARE(int, Si32, "%d", int64_t, Si64, "%lld", Val);
      COMPARE(int, Si32, "%d", float, F32, "%f", Val);
      COMPARE(int, Si32, "%d", double, F64, "%f", Val);
    }
  }
  for (unsigned i = 0; i < NumValsUi64; ++i) {
    {
      uint64_t Val = ValsUi64[i];
      COMPARE(uint64_t, Ui64, "%llu", bool, Ui1, "%u", Val);
      COMPARE(uint64_t, Ui64, "%llu", unsigned char, Ui8, "%u", Val);
      COMPARE(uint64_t, Ui64, "%llu", signed char, Si8, "%d", Val);
      COMPARE(uint64_t, Ui64, "%llu", unsigned short, Ui16, "%u", Val);
      COMPARE(uint64_t, Ui64, "%llu", short, Si16, "%d", Val);
      COMPARE(uint64_t, Ui64, "%llu", uint32_t, Ui32, "%u", Val);
      COMPARE(uint64_t, Ui64, "%llu", int32_t, Si32, "%d", Val);
      COMPARE(uint64_t, Ui64, "%llu", uint64_t, Ui64, "%llu", Val);
      COMPARE(uint64_t, Ui64, "%llu", int64_t, Si64, "%lld", Val);
      COMPARE(uint64_t, Ui64, "%llu", float, F32, "%f", Val);
      COMPARE(uint64_t, Ui64, "%llu", double, F64, "%f", Val);
      COMPARE(uint64_t, bits_Ui64, "%llu", float, F64, "%f", Val);
    }
  }
  for (unsigned i = 0; i < NumValsSi64; ++i) {
    {
      uint64_t Val = ValsSi64[i];
      COMPARE(int64_t, Si64, "%lld", bool, Ui1, "%u", Val);
      COMPARE(int64_t, Si64, "%lld", unsigned char, Ui8, "%u", Val);
      COMPARE(int64_t, Si64, "%lld", signed char, Si8, "%d", Val);
      COMPARE(int64_t, Si64, "%lld", unsigned short, Ui16, "%u", Val);
      COMPARE(int64_t, Si64, "%lld", short, Si16, "%d", Val);
      COMPARE(int64_t, Si64, "%lld", uint32_t, Ui32, "%u", Val);
      COMPARE(int64_t, Si64, "%lld", int32_t, Si32, "%d", Val);
      COMPARE(int64_t, Si64, "%lld", uint64_t, Ui64, "%llu", Val);
      COMPARE(int64_t, Si64, "%lld", int64_t, Si64, "%lld", Val);
      COMPARE(int64_t, Si64, "%lld", float, F32, "%f", Val);
      COMPARE(int64_t, Si64, "%lld", double, F64, "%f", Val);
    }
  }
  for (unsigned i = 0; i < NumValsF32; ++i) {
    for (unsigned j = 0; j < 2; ++j) {
      float Val = ValsF32[i];
      if (j > 0)
        Val = -Val;
      COMPARE(float, F32, "%f", bool, Ui1, "%u", Val);
      COMPARE(float, F32, "%f", unsigned char, Ui8, "%u", Val);
      COMPARE(float, F32, "%f", signed char, Si8, "%d", Val);
      COMPARE(float, F32, "%f", unsigned short, Ui16, "%u", Val);
      COMPARE(float, F32, "%f", short, Si16, "%d", Val);
      COMPARE(float, F32, "%f", uint32_t, Ui32, "%u", Val);
      COMPARE(float, F32, "%f", int32_t, Si32, "%d", Val);
      COMPARE(float, F32, "%f", uint64_t, Ui64, "%llu", Val);
      COMPARE(float, F32, "%f", int64_t, Si64, "%lld", Val);
      COMPARE(float, F32, "%f", float, F32, "%f", Val);
      COMPARE(float, F32, "%f", double, F64, "%f", Val);
      COMPARE(float, bits_F32, "%f", uint32_t, Ui32, "%u", Val);
    }
  }
  for (unsigned i = 0; i < NumValsF64; ++i) {
    for (unsigned j = 0; j < 2; ++j) {
      double Val = ValsF64[i];
      if (j > 0)
        Val = -Val;
      COMPARE(double, F64, "%f", bool, Ui1, "%u", Val);
      COMPARE(double, F64, "%f", unsigned char, Ui8, "%u", Val);
      COMPARE(double, F64, "%f", signed char, Si8, "%d", Val);
      COMPARE(double, F64, "%f", unsigned short, Ui16, "%u", Val);
      COMPARE(double, F64, "%f", short, Si16, "%d", Val);
      COMPARE(double, F64, "%f", uint32_t, Ui32, "%u", Val);
      COMPARE(double, F64, "%f", int32_t, Si32, "%d", Val);
      COMPARE(double, F64, "%f", uint64_t, Ui64, "%llu", Val);
      COMPARE(double, F64, "%f", int64_t, Si64, "%lld", Val);
      COMPARE(double, F64, "%f", float, F32, "%f", Val);
      COMPARE(double, F64, "%f", double, F64, "%f", Val);
      COMPARE(double, bits_F64, "%f", uint64_t, Ui64, "%llu", Val);
    }
  }

  printf("TotalTests=%u Passes=%u Failures=%u\n", TotalTests, Passes, Failures);
  return Failures;
}

////////////////////////////////////////////////////////////////

// The following are helper definitions that should be part of the
// Subzero runtime.

extern "C" uint32_t cvtdtoui32(double a) { return (uint32_t)a; }

extern "C" uint32_t cvtftoui32(float a) { return (uint32_t)a; }

extern "C" int64_t cvtdtosi64(double a) { return (int64_t)a; }

extern "C" int64_t cvtftosi64(float a) { return (int64_t)a; }

extern "C" uint64_t cvtdtoui64(double a) { return (uint64_t)a; }

extern "C" uint64_t cvtftoui64(float a) { return (uint64_t)a; }

extern "C" float cvtui64tof(uint64_t a) { return (float)a; }

extern "C" double cvtui64tod(uint64_t a) { return (double)a; }

extern "C" float cvtsi64tof(int64_t a) { return (float)a; }

extern "C" float cvtui32tof(uint32_t a) { return (float)a; }

extern "C" double cvtui32tod(uint32_t a) { return (double)a; }

extern "C" double cvtsi64tod(int64_t a) { return (double)a; }
