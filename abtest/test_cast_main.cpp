// abtest.py --test=test_cast.cpp --test=test_cast_to_u1.ll --driver=test_cast_main.cpp --prefix=Subzero_ --output=test_cast

#include <stdio.h>
#include <stdint.h>
#include <string.h>

bool castUi64ToUi1(uint64_t a);
bool castSi64ToUi1(int64_t a);
bool castUi32ToUi1(uint32_t a);
bool castSi32ToUi1(int32_t a);
bool castUi16ToUi1(unsigned short a);
bool castSi16ToUi1(short a);
bool castUi8ToUi1(unsigned char a);
bool castSi8ToUi1(signed char a);
bool castUi1ToUi1(bool a);
bool castF64ToUi1(double a);
bool castF32ToUi1(float a);

signed char castUi64ToSi8(uint64_t a);
signed char castSi64ToSi8(int64_t a);
signed char castUi32ToSi8(uint32_t a);
signed char castSi32ToSi8(int32_t a);
signed char castUi16ToSi8(unsigned short a);
signed char castSi16ToSi8(short a);
signed char castUi8ToSi8(unsigned char a);
signed char castSi8ToSi8(signed char a);
signed char castUi1ToSi8(bool a);
signed char castF64ToSi8(double a);
signed char castF32ToSi8(float a);

unsigned char castUi64ToUi8(uint64_t a);
unsigned char castSi64ToUi8(int64_t a);
unsigned char castUi32ToUi8(uint32_t a);
unsigned char castSi32ToUi8(int32_t a);
unsigned char castUi16ToUi8(unsigned short a);
unsigned char castSi16ToUi8(short a);
unsigned char castUi8ToUi8(unsigned char a);
unsigned char castSi8ToUi8(signed char a);
unsigned char castUi1ToUi8(bool a);
unsigned char castF64ToUi8(double a);
unsigned char castF32ToUi8(float a);

short castUi64ToSi16(uint64_t a);
short castSi64ToSi16(int64_t a);
short castUi32ToSi16(uint32_t a);
short castSi32ToSi16(int32_t a);
short castUi16ToSi16(unsigned short a);
short castSi16ToSi16(short a);
short castUi8ToSi16(unsigned char a);
short castSi8ToSi16(signed char a);
short castUi1ToSi16(bool a);
short castF64ToSi16(double a);
short castF32ToSi16(float a);

unsigned short castUi64ToUi16(uint64_t a);
unsigned short castSi64ToUi16(int64_t a);
unsigned short castUi32ToUi16(uint32_t a);
unsigned short castSi32ToUi16(int32_t a);
unsigned short castUi16ToUi16(unsigned short a);
unsigned short castSi16ToUi16(short a);
unsigned short castUi8ToUi16(unsigned char a);
unsigned short castSi8ToUi16(signed char a);
unsigned short castUi1ToUi16(bool a);
unsigned short castF64ToUi16(double a);
unsigned short castF32ToUi16(float a);

int32_t castUi64ToSi32(uint64_t a);
int32_t castSi64ToSi32(int64_t a);
int32_t castUi32ToSi32(uint32_t a);
int32_t castSi32ToSi32(int32_t a);
int32_t castUi16ToSi32(unsigned short a);
int32_t castSi16ToSi32(short a);
int32_t castUi8ToSi32(unsigned char a);
int32_t castSi8ToSi32(signed char a);
int32_t castUi1ToSi32(bool a);
int32_t castF64ToSi32(double a);
int32_t castF32ToSi32(float a);

uint32_t castUi64ToUi32(uint64_t a);
uint32_t castSi64ToUi32(int64_t a);
uint32_t castUi32ToUi32(uint32_t a);
uint32_t castSi32ToUi32(int32_t a);
uint32_t castUi16ToUi32(unsigned short a);
uint32_t castSi16ToUi32(short a);
uint32_t castUi8ToUi32(unsigned char a);
uint32_t castSi8ToUi32(signed char a);
uint32_t castUi1ToUi32(bool a);
uint32_t castF64ToUi32(double a);
uint32_t castF32ToUi32(float a);

int64_t castUi64ToSi64(uint64_t a);
int64_t castSi64ToSi64(int64_t a);
int64_t castUi32ToSi64(uint32_t a);
int64_t castSi32ToSi64(int32_t a);
int64_t castUi16ToSi64(unsigned short a);
int64_t castSi16ToSi64(short a);
int64_t castUi8ToSi64(unsigned char a);
int64_t castSi8ToSi64(signed char a);
int64_t castUi1ToSi64(bool a);
int64_t castF64ToSi64(double a);
int64_t castF32ToSi64(float a);

uint64_t castUi64ToUi64(uint64_t a);
uint64_t castSi64ToUi64(int64_t a);
uint64_t castUi32ToUi64(uint32_t a);
uint64_t castSi32ToUi64(int32_t a);
uint64_t castUi16ToUi64(unsigned short a);
uint64_t castSi16ToUi64(short a);
uint64_t castUi8ToUi64(unsigned char a);
uint64_t castSi8ToUi64(signed char a);
uint64_t castUi1ToUi64(bool a);
uint64_t castF64ToUi64(double a);
uint64_t castF32ToUi64(float a);

float castUi64ToF32(uint64_t a);
float castSi64ToF32(int64_t a);
float castUi32ToF32(uint32_t a);
float castSi32ToF32(int32_t a);
float castUi16ToF32(unsigned short a);
float castSi16ToF32(short a);
float castUi8ToF32(unsigned char a);
float castSi8ToF32(signed char a);
float castUi1ToF32(bool a);
float castF64ToF32(double a);
float castF32ToF32(float a);

double castUi64ToF64(uint64_t a);
double castSi64ToF64(int64_t a);
double castUi32ToF64(uint32_t a);
double castSi32ToF64(int32_t a);
double castUi16ToF64(unsigned short a);
double castSi16ToF64(short a);
double castUi8ToF64(unsigned char a);
double castSi8ToF64(signed char a);
double castUi1ToF64(bool a);
double castF64ToF64(double a);
double castF32ToF64(float a);

uint32_t castbits_F32ToUi32(float a);
float castbits_Ui32ToF32(uint32_t a);
uint64_t castbits_F64ToUi64(double a);
double castbits_Ui64ToF64(uint64_t a);

bool Subzero_castUi64ToUi1(uint64_t a);
bool Subzero_castSi64ToUi1(int64_t a);
bool Subzero_castUi32ToUi1(uint32_t a);
bool Subzero_castSi32ToUi1(int32_t a);
bool Subzero_castUi16ToUi1(unsigned short a);
bool Subzero_castSi16ToUi1(short a);
bool Subzero_castUi8ToUi1(unsigned char a);
bool Subzero_castSi8ToUi1(signed char a);
bool Subzero_castUi1ToUi1(bool a);
bool Subzero_castF64ToUi1(double a);
bool Subzero_castF32ToUi1(float a);

signed char Subzero_castUi64ToSi8(uint64_t a);
signed char Subzero_castSi64ToSi8(int64_t a);
signed char Subzero_castUi32ToSi8(uint32_t a);
signed char Subzero_castSi32ToSi8(int32_t a);
signed char Subzero_castUi16ToSi8(unsigned short a);
signed char Subzero_castSi16ToSi8(short a);
signed char Subzero_castUi8ToSi8(unsigned char a);
signed char Subzero_castSi8ToSi8(signed char a);
signed char Subzero_castUi1ToSi8(bool a);
signed char Subzero_castF64ToSi8(double a);
signed char Subzero_castF32ToSi8(float a);

unsigned char Subzero_castUi64ToUi8(uint64_t a);
unsigned char Subzero_castSi64ToUi8(int64_t a);
unsigned char Subzero_castUi32ToUi8(uint32_t a);
unsigned char Subzero_castSi32ToUi8(int32_t a);
unsigned char Subzero_castUi16ToUi8(unsigned short a);
unsigned char Subzero_castSi16ToUi8(short a);
unsigned char Subzero_castUi8ToUi8(unsigned char a);
unsigned char Subzero_castSi8ToUi8(signed char a);
unsigned char Subzero_castUi1ToUi8(bool a);
unsigned char Subzero_castF64ToUi8(double a);
unsigned char Subzero_castF32ToUi8(float a);

short Subzero_castUi64ToSi16(uint64_t a);
short Subzero_castSi64ToSi16(int64_t a);
short Subzero_castUi32ToSi16(uint32_t a);
short Subzero_castSi32ToSi16(int32_t a);
short Subzero_castUi16ToSi16(unsigned short a);
short Subzero_castSi16ToSi16(short a);
short Subzero_castUi8ToSi16(unsigned char a);
short Subzero_castSi8ToSi16(signed char a);
short Subzero_castUi1ToSi16(bool a);
short Subzero_castF64ToSi16(double a);
short Subzero_castF32ToSi16(float a);

unsigned short Subzero_castUi64ToUi16(uint64_t a);
unsigned short Subzero_castSi64ToUi16(int64_t a);
unsigned short Subzero_castUi32ToUi16(uint32_t a);
unsigned short Subzero_castSi32ToUi16(int32_t a);
unsigned short Subzero_castUi16ToUi16(unsigned short a);
unsigned short Subzero_castSi16ToUi16(short a);
unsigned short Subzero_castUi8ToUi16(unsigned char a);
unsigned short Subzero_castSi8ToUi16(signed char a);
unsigned short Subzero_castUi1ToUi16(bool a);
unsigned short Subzero_castF64ToUi16(double a);
unsigned short Subzero_castF32ToUi16(float a);

int32_t Subzero_castUi64ToSi32(uint64_t a);
int32_t Subzero_castSi64ToSi32(int64_t a);
int32_t Subzero_castUi32ToSi32(uint32_t a);
int32_t Subzero_castSi32ToSi32(int32_t a);
int32_t Subzero_castUi16ToSi32(unsigned short a);
int32_t Subzero_castSi16ToSi32(short a);
int32_t Subzero_castUi8ToSi32(unsigned char a);
int32_t Subzero_castSi8ToSi32(signed char a);
int32_t Subzero_castUi1ToSi32(bool a);
int32_t Subzero_castF64ToSi32(double a);
int32_t Subzero_castF32ToSi32(float a);

uint32_t Subzero_castUi64ToUi32(uint64_t a);
uint32_t Subzero_castSi64ToUi32(int64_t a);
uint32_t Subzero_castUi32ToUi32(uint32_t a);
uint32_t Subzero_castSi32ToUi32(int32_t a);
uint32_t Subzero_castUi16ToUi32(unsigned short a);
uint32_t Subzero_castSi16ToUi32(short a);
uint32_t Subzero_castUi8ToUi32(unsigned char a);
uint32_t Subzero_castSi8ToUi32(signed char a);
uint32_t Subzero_castUi1ToUi32(bool a);
uint32_t Subzero_castF64ToUi32(double a);
uint32_t Subzero_castF32ToUi32(float a);

int64_t Subzero_castUi64ToSi64(uint64_t a);
int64_t Subzero_castSi64ToSi64(int64_t a);
int64_t Subzero_castUi32ToSi64(uint32_t a);
int64_t Subzero_castSi32ToSi64(int32_t a);
int64_t Subzero_castUi16ToSi64(unsigned short a);
int64_t Subzero_castSi16ToSi64(short a);
int64_t Subzero_castUi8ToSi64(unsigned char a);
int64_t Subzero_castSi8ToSi64(signed char a);
int64_t Subzero_castUi1ToSi64(bool a);
int64_t Subzero_castF64ToSi64(double a);
int64_t Subzero_castF32ToSi64(float a);

uint64_t Subzero_castUi64ToUi64(uint64_t a);
uint64_t Subzero_castSi64ToUi64(int64_t a);
uint64_t Subzero_castUi32ToUi64(uint32_t a);
uint64_t Subzero_castSi32ToUi64(int32_t a);
uint64_t Subzero_castUi16ToUi64(unsigned short a);
uint64_t Subzero_castSi16ToUi64(short a);
uint64_t Subzero_castUi8ToUi64(unsigned char a);
uint64_t Subzero_castSi8ToUi64(signed char a);
uint64_t Subzero_castUi1ToUi64(bool a);
uint64_t Subzero_castF64ToUi64(double a);
uint64_t Subzero_castF32ToUi64(float a);

float Subzero_castUi64ToF32(uint64_t a);
float Subzero_castSi64ToF32(int64_t a);
float Subzero_castUi32ToF32(uint32_t a);
float Subzero_castSi32ToF32(int32_t a);
float Subzero_castUi16ToF32(unsigned short a);
float Subzero_castSi16ToF32(short a);
float Subzero_castUi8ToF32(unsigned char a);
float Subzero_castSi8ToF32(signed char a);
float Subzero_castUi1ToF32(bool a);
float Subzero_castF64ToF32(double a);
float Subzero_castF32ToF32(float a);

double Subzero_castUi64ToF64(uint64_t a);
double Subzero_castSi64ToF64(int64_t a);
double Subzero_castUi32ToF64(uint32_t a);
double Subzero_castSi32ToF64(int32_t a);
double Subzero_castUi16ToF64(unsigned short a);
double Subzero_castSi16ToF64(short a);
double Subzero_castUi8ToF64(unsigned char a);
double Subzero_castSi8ToF64(signed char a);
double Subzero_castUi1ToF64(bool a);
double Subzero_castF64ToF64(double a);
double Subzero_castF32ToF64(float a);

uint32_t Subzero_castbits_F32ToUi32(float a);
float Subzero_castbits_Ui32ToF32(uint32_t a);
uint64_t Subzero_castbits_F64ToUi64(double a);
double Subzero_castbits_Ui64ToF64(uint64_t a);

#define XSTR(s) STR(s)
#define STR(s) #s
#define COMPARE(FromCName, FromIceName, FromPrintf, ToCName, ToIceName,        \
                ToPrintf, Input)                                               \
  do {                                                                         \
    ToCName ResultSz, ResultLlc;                                               \
    ResultLlc = cast##FromIceName##To##ToIceName(Input);                       \
    ResultSz = Subzero_cast##FromIceName##To##ToIceName(Input);                \
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
