// crosstest.py --test=test_arith.cpp --driver=test_arith_main.cpp
// --prefix=Subzero_ --output=test_arith

#include <stdio.h>
#include <stdint.h>

#include "test_arith.h"
namespace Subzero_ {
#include "test_arith.h"
}

int main(int argc, char **argv) {
  static unsigned Values[] = { 0x0,        0x1,        0x7ffffffe, 0x7fffffff,
                               0x80000000, 0x80000001, 0xfffffffe, 0xffffffff,
                               0x7e,       0x7f,       0x80,       0x81,
                               0xfe,       0xff,       0x100,      0x101,
                               0x7ffe,     0x7fff,     0x8000,     0x8001,
                               0xfffe,     0xffff,     0x10000,    0x10001, };
  const static unsigned NumValues = sizeof(Values) / sizeof(*Values);

  typedef uint8_t (*FuncType8)(uint8_t, uint8_t);
  typedef int8_t (*FuncTypeS8)(int8_t, int8_t);
  static struct {
    const char *Name;
    FuncType8 FuncLlc;
    FuncType8 FuncSz;
    bool DisallowIllegal; // for divide related tests
  } Func8[] = {
      { "testAdd8", (FuncType8)testAdd, (FuncType8)Subzero_::testAdd },
      { "testSub8", (FuncType8)testSub, (FuncType8)Subzero_::testSub },
      { "testMul8", (FuncType8)testMul, (FuncType8)Subzero_::testMul },
      { "testUdiv8", (FuncType8)testUdiv, (FuncType8)Subzero_::testUdiv, true },
      { "testSdiv8",
        (FuncType8)(FuncTypeS8)testSdiv,
        (FuncType8)(FuncTypeS8)Subzero_::testSdiv,
        true },
      { "testUrem8", (FuncType8)testUrem, (FuncType8)Subzero_::testUrem, true },
      { "testSrem8",
        (FuncType8)(FuncTypeS8)testSrem,
        (FuncType8)(FuncTypeS8)Subzero_::testSrem,
        true },
      { "testShl8", (FuncType8)testShl, (FuncType8)Subzero_::testShl },
      { "testLshr8", (FuncType8)testLshr, (FuncType8)Subzero_::testLshr },
      { "testAshr8", (FuncType8)(FuncTypeS8)testAshr,
        (FuncType8)(FuncTypeS8)Subzero_::testAshr },
      { "testAnd8", (FuncType8)testAnd, (FuncType8)Subzero_::testAnd },
      { "testOr8", (FuncType8)testOr, (FuncType8)Subzero_::testOr },
      { "testXor8", (FuncType8)testXor, (FuncType8)Subzero_::testXor },
    };
  const static unsigned NumFunc8 = sizeof(Func8) / sizeof(*Func8);

  typedef uint16_t (*FuncType16)(uint16_t, uint16_t);
  typedef int16_t (*FuncTypeS16)(int16_t, int16_t);
  static struct {
    const char *Name;
    FuncType16 FuncLlc;
    FuncType16 FuncSz;
    bool DisallowIllegal; // on the second arg, for divide related tests
  } Func16[] =
        { { "testAdd16", (FuncType16)testAdd, (FuncType16)Subzero_::testAdd },
          { "testSub16", (FuncType16)testSub, (FuncType16)Subzero_::testSub },
          { "testMul16", (FuncType16)testMul, (FuncType16)Subzero_::testMul },
          { "testUdiv16",                   (FuncType16)testUdiv,
            (FuncType16)Subzero_::testUdiv, true },
          { "testSdiv16",
            (FuncType16)(FuncTypeS16)testSdiv,
            (FuncType16)(FuncTypeS16)Subzero_::testSdiv,
            true },
          { "testUrem16",                   (FuncType16)testUrem,
            (FuncType16)Subzero_::testUrem, true },
          { "testSrem16",
            (FuncType16)(FuncTypeS16)testSrem,
            (FuncType16)(FuncTypeS16)Subzero_::testSrem,
            true },
          { "testShl16", (FuncType16)testShl, (FuncType16)Subzero_::testShl },
          { "testLshr16", (FuncType16)testLshr,
            (FuncType16)Subzero_::testLshr },
          { "testAshr16", (FuncType16)(FuncTypeS16)testAshr,
            (FuncType16)(FuncTypeS16)Subzero_::testAshr },
          { "testAnd16", (FuncType16)testAnd, (FuncType16)Subzero_::testAnd },
          { "testOr16", (FuncType16)testOr, (FuncType16)Subzero_::testOr },
          { "testXor16", (FuncType16)testXor,
            (FuncType16)Subzero_::testXor }, };
  const static unsigned NumFunc16 = sizeof(Func16) / sizeof(*Func16);

  typedef uint32_t (*FuncType32)(uint32_t, uint32_t);
  typedef int32_t (*FuncTypeS32)(int32_t, int32_t);
  static struct {
    const char *Name;
    FuncType32 FuncLlc;
    FuncType32 FuncSz;
    bool DisallowIllegal; // on the second arg, for divide related tests
  } Func32[] =
        { { "testAdd32", (FuncType32)testAdd, (FuncType32)Subzero_::testAdd },
          { "testSub32", (FuncType32)testSub, (FuncType32)Subzero_::testSub },
          { "testMul32", (FuncType32)testMul, (FuncType32)Subzero_::testMul },
          { "testUdiv32",                   (FuncType32)testUdiv,
            (FuncType32)Subzero_::testUdiv, true },
          { "testSdiv32",
            (FuncType32)(FuncTypeS32)testSdiv,
            (FuncType32)(FuncTypeS32)Subzero_::testSdiv,
            true },
          { "testUrem32",                   (FuncType32)testUrem,
            (FuncType32)Subzero_::testUrem, true },
          { "testSrem32",
            (FuncType32)(FuncTypeS32)testSrem,
            (FuncType32)(FuncTypeS32)Subzero_::testSrem,
            true },
          { "testShl32", (FuncType32)testShl, (FuncType32)Subzero_::testShl },
          { "testLshr32", (FuncType32)testLshr,
            (FuncType32)Subzero_::testLshr },
          { "testAshr32", (FuncType32)(FuncTypeS32)testAshr,
            (FuncType32)(FuncTypeS32)Subzero_::testAshr },
          { "testAnd32", (FuncType32)testAnd, (FuncType32)Subzero_::testAnd },
          { "testOr32", (FuncType32)testOr, (FuncType32)Subzero_::testOr },
          { "testXor32", (FuncType32)testXor,
            (FuncType32)Subzero_::testXor }, };
  const static unsigned NumFunc32 = sizeof(Func32) / sizeof(*Func32);

  typedef uint64_t (*FuncType64)(uint64_t, uint64_t);
  typedef int64_t (*FuncTypeS64)(int64_t, int64_t);
  static struct {
    const char *Name;
    FuncType64 FuncLlc;
    FuncType64 FuncSz;
    bool DisallowIllegal; // on the second arg, for divide related tests
  } Func64[] =
        { { "testAdd64", (FuncType64)testAdd, (FuncType64)Subzero_::testAdd },
          { "testSub64", (FuncType64)testSub, (FuncType64)Subzero_::testSub },
          { "testMul64", (FuncType64)testMul, (FuncType64)Subzero_::testMul },
          { "testUdiv64",                   (FuncType64)testUdiv,
            (FuncType64)Subzero_::testUdiv, true },
          { "testSdiv64",
            (FuncType64)(FuncTypeS64)testSdiv,
            (FuncType64)(FuncTypeS64)Subzero_::testSdiv,
            true },
          { "testUrem64",                   (FuncType64)testUrem,
            (FuncType64)Subzero_::testUrem, true },
          { "testSrem64",
            (FuncType64)(FuncTypeS64)testSrem,
            (FuncType64)(FuncTypeS64)Subzero_::testSrem,
            true },
          { "testShl64", (FuncType64)testShl, (FuncType64)Subzero_::testShl },
          { "testLshr64", (FuncType64)testLshr,
            (FuncType64)Subzero_::testLshr },
          { "testAshr64", (FuncType64)(FuncTypeS64)testAshr,
            (FuncType64)(FuncTypeS64)Subzero_::testAshr },
          { "testAnd64", (FuncType64)testAnd, (FuncType64)Subzero_::testAnd },
          { "testOr64", (FuncType64)testOr, (FuncType64)Subzero_::testOr },
          { "testXor64", (FuncType64)testXor,
            (FuncType64)Subzero_::testXor }, };
  const static unsigned NumFunc64 = sizeof(Func64) / sizeof(*Func64);

  unsigned TotalTests = 0;
  unsigned Passes = 0;
  unsigned Failures = 0;

  for (unsigned f = 0; f < NumFunc8; ++f) {
    for (unsigned i = 0; i < NumValues; ++i) {
      for (unsigned j = 0; j < NumValues; ++j) {
        if (Func8[f].DisallowIllegal && ((uint8_t)Values[j] == 0))
          continue;
        ++TotalTests;
        uint8_t ResultSz = Func8[f].FuncSz(Values[i], Values[j]);
        uint8_t ResultLlc = Func8[f].FuncLlc(Values[i], Values[j]);
        if (ResultSz == ResultLlc) {
          ++Passes;
        } else {
          ++Failures;
          printf("%s(0x%08x, 0x%08x): sz=%d llc=%d\n", Func8[f].Name, Values[i],
                 Values[j], ResultSz, ResultLlc);
        }
      }
    }
  }

  for (unsigned f = 0; f < NumFunc16; ++f) {
    for (unsigned i = 0; i < NumValues; ++i) {
      for (unsigned j = 0; j < NumValues; ++j) {
        if (Func16[f].DisallowIllegal && ((uint16_t)Values[j] == 0))
          continue;
        ++TotalTests;
        uint16_t ResultSz = Func16[f].FuncSz(Values[i], Values[j]);
        uint16_t ResultLlc = Func16[f].FuncLlc(Values[i], Values[j]);
        if (ResultSz == ResultLlc) {
          ++Passes;
        } else {
          ++Failures;
          printf("%s(0x%08x, 0x%08x): sz=%d llc=%d\n", Func16[f].Name,
                 Values[i], Values[j], ResultSz, ResultLlc);
        }
      }
    }
  }

  for (unsigned f = 0; f < NumFunc32; ++f) {
    for (unsigned i = 0; i < NumValues; ++i) {
      for (unsigned j = 0; j < NumValues; ++j) {
        if (Func32[f].DisallowIllegal && Values[j] == 0)
          continue;
        if (Func32[f].DisallowIllegal && Values[i] == 0x80000000 &&
            Values[j] == 0xffffffff)
          continue;
        ++TotalTests;
        uint32_t ResultSz = Func32[f].FuncSz(Values[i], Values[j]);
        uint32_t ResultLlc = Func32[f].FuncLlc(Values[i], Values[j]);
        if (ResultSz == ResultLlc) {
          ++Passes;
        } else {
          ++Failures;
          printf("%s(0x%08x, 0x%08x): sz=%d llc=%d\n", Func32[f].Name,
                 Values[i], Values[j], ResultSz, ResultLlc);
        }
      }
    }
  }

  for (unsigned f = 0; f < NumFunc64; ++f) {
    for (unsigned iLo = 0; iLo < NumValues; ++iLo) {
      for (unsigned iHi = 0; iHi < NumValues; ++iHi) {
        for (unsigned jLo = 0; jLo < NumValues; ++jLo) {
          for (unsigned jHi = 0; jHi < NumValues; ++jHi) {
            uint64_t Value1 = (((uint64_t)Values[iHi]) << 32) + Values[iLo];
            uint64_t Value2 = (((uint64_t)Values[jHi]) << 32) + Values[jLo];

            if (Func64[f].DisallowIllegal && Value2 == 0)
              continue;
            ++TotalTests;
            uint64_t ResultSz = Func64[f].FuncSz(Value1, Value2);
            uint64_t ResultLlc = Func64[f].FuncLlc(Value1, Value2);
            if (ResultSz == ResultLlc) {
              ++Passes;
            } else {
              ++Failures;
              printf("%s(0x%016llx, 0x%016llx): sz=%llx llc=%llx\n",
                     Func64[f].Name, Value1, Value2, ResultSz, ResultLlc);
            }
          }
        }
      }
    }
  }

  printf("TotalTests=%u Passes=%u Failures=%u\n", TotalTests, Passes, Failures);
  return Failures;
}
