// abtest.py --test=test_icmp.cpp --driver=test_icmp_main.cpp --prefix=Subzero_ --output=test_icmp

#include <stdio.h>
#include <stdint.h>

#include "test_icmp.h"
namespace Subzero_ {
#include "test_icmp.h"
}

int main(int argc, char **argv) {
  static unsigned Values[] = {
    0x0,        0x1,        0x7ffffffe, 0x7fffffff,
    0x80000000, 0x80000001, 0xfffffffe, 0xffffffff,
  };
  const static unsigned NumValues = sizeof(Values) / sizeof(*Values);

  typedef bool (*FuncType8)(uint8_t, uint8_t);
  static struct {
    const char *Name;
    FuncType8 FuncSz;
    FuncType8 FuncLlc;
  } Func8[] = { { "icmpEq8Bool", (FuncType8)Subzero_::icmpEq8Bool,
                   (FuncType8)icmpEq8Bool },
                 { "icmpNe8Bool", (FuncType8)Subzero_::icmpNe8Bool,
                   (FuncType8)icmpNe8Bool },
                 { "icmpSgt8Bool", (FuncType8)Subzero_::icmpSgt8Bool,
                   (FuncType8)icmpSgt8Bool },
                 { "icmpUgt8Bool", (FuncType8)Subzero_::icmpUgt8Bool,
                   (FuncType8)icmpUgt8Bool },
                 { "icmpSge8Bool", (FuncType8)Subzero_::icmpSge8Bool,
                   (FuncType8)icmpSge8Bool },
                 { "icmpUge8Bool", (FuncType8)Subzero_::icmpUge8Bool,
                   (FuncType8)icmpUge8Bool },
                 { "icmpSlt8Bool", (FuncType8)Subzero_::icmpSlt8Bool,
                   (FuncType8)icmpSlt8Bool },
                 { "icmpUlt8Bool", (FuncType8)Subzero_::icmpUlt8Bool,
                   (FuncType8)icmpUlt8Bool },
                 { "icmpSle8Bool", (FuncType8)Subzero_::icmpSle8Bool,
                   (FuncType8)icmpSle8Bool },
                 { "icmpUle8Bool", (FuncType8)Subzero_::icmpUle8Bool,
                   (FuncType8)icmpUle8Bool }, };
  const static unsigned NumFunc8 = sizeof(Func8) / sizeof(*Func8);

  typedef bool (*FuncType16)(uint16_t, uint16_t);
  static struct {
    const char *Name;
    FuncType16 FuncSz;
    FuncType16 FuncLlc;
  } Func16[] = { { "icmpEq16Bool", (FuncType16)Subzero_::icmpEq16Bool,
                   (FuncType16)icmpEq16Bool },
                 { "icmpNe16Bool", (FuncType16)Subzero_::icmpNe16Bool,
                   (FuncType16)icmpNe16Bool },
                 { "icmpSgt16Bool", (FuncType16)Subzero_::icmpSgt16Bool,
                   (FuncType16)icmpSgt16Bool },
                 { "icmpUgt16Bool", (FuncType16)Subzero_::icmpUgt16Bool,
                   (FuncType16)icmpUgt16Bool },
                 { "icmpSge16Bool", (FuncType16)Subzero_::icmpSge16Bool,
                   (FuncType16)icmpSge16Bool },
                 { "icmpUge16Bool", (FuncType16)Subzero_::icmpUge16Bool,
                   (FuncType16)icmpUge16Bool },
                 { "icmpSlt16Bool", (FuncType16)Subzero_::icmpSlt16Bool,
                   (FuncType16)icmpSlt16Bool },
                 { "icmpUlt16Bool", (FuncType16)Subzero_::icmpUlt16Bool,
                   (FuncType16)icmpUlt16Bool },
                 { "icmpSle16Bool", (FuncType16)Subzero_::icmpSle16Bool,
                   (FuncType16)icmpSle16Bool },
                 { "icmpUle16Bool", (FuncType16)Subzero_::icmpUle16Bool,
                   (FuncType16)icmpUle16Bool }, };
  const static unsigned NumFunc16 = sizeof(Func16) / sizeof(*Func16);

  typedef bool (*FuncType32)(uint32_t, uint32_t);
  static struct {
    const char *Name;
    FuncType32 FuncSz;
    FuncType32 FuncLlc;
  } Func32[] = { { "icmpEq32Bool", (FuncType32)Subzero_::icmpEq32Bool,
                   (FuncType32)icmpEq32Bool },
                 { "icmpNe32Bool", (FuncType32)Subzero_::icmpNe32Bool,
                   (FuncType32)icmpNe32Bool },
                 { "icmpSgt32Bool", (FuncType32)Subzero_::icmpSgt32Bool,
                   (FuncType32)icmpSgt32Bool },
                 { "icmpUgt32Bool", (FuncType32)Subzero_::icmpUgt32Bool,
                   (FuncType32)icmpUgt32Bool },
                 { "icmpSge32Bool", (FuncType32)Subzero_::icmpSge32Bool,
                   (FuncType32)icmpSge32Bool },
                 { "icmpUge32Bool", (FuncType32)Subzero_::icmpUge32Bool,
                   (FuncType32)icmpUge32Bool },
                 { "icmpSlt32Bool", (FuncType32)Subzero_::icmpSlt32Bool,
                   (FuncType32)icmpSlt32Bool },
                 { "icmpUlt32Bool", (FuncType32)Subzero_::icmpUlt32Bool,
                   (FuncType32)icmpUlt32Bool },
                 { "icmpSle32Bool", (FuncType32)Subzero_::icmpSle32Bool,
                   (FuncType32)icmpSle32Bool },
                 { "icmpUle32Bool", (FuncType32)Subzero_::icmpUle32Bool,
                   (FuncType32)icmpUle32Bool }, };
  const static unsigned NumFunc32 = sizeof(Func32) / sizeof(*Func32);

  typedef bool (*FuncType64)(uint64_t, uint64_t);
  static struct {
    const char *Name;
    FuncType64 FuncSz;
    FuncType64 FuncLlc;
  } Func64[] = { { "icmpEq64Bool", (FuncType64)Subzero_::icmpEq64Bool,
                   (FuncType64)icmpEq64Bool },
                 { "icmpNe64Bool", (FuncType64)Subzero_::icmpNe64Bool,
                   (FuncType64)icmpNe64Bool },
                 { "icmpSgt64Bool", (FuncType64)Subzero_::icmpSgt64Bool,
                   (FuncType64)icmpSgt64Bool },
                 { "icmpUgt64Bool", (FuncType64)Subzero_::icmpUgt64Bool,
                   (FuncType64)icmpUgt64Bool },
                 { "icmpSge64Bool", (FuncType64)Subzero_::icmpSge64Bool,
                   (FuncType64)icmpSge64Bool },
                 { "icmpUge64Bool", (FuncType64)Subzero_::icmpUge64Bool,
                   (FuncType64)icmpUge64Bool },
                 { "icmpSlt64Bool", (FuncType64)Subzero_::icmpSlt64Bool,
                   (FuncType64)icmpSlt64Bool },
                 { "icmpUlt64Bool", (FuncType64)Subzero_::icmpUlt64Bool,
                   (FuncType64)icmpUlt64Bool },
                 { "icmpSle64Bool", (FuncType64)Subzero_::icmpSle64Bool,
                   (FuncType64)icmpSle64Bool },
                 { "icmpUle64Bool", (FuncType64)Subzero_::icmpUle64Bool,
                   (FuncType64)icmpUle64Bool }, };
  const static unsigned NumFunc64 = sizeof(Func64) / sizeof(*Func64);

  bool ResultSz, ResultLlc;

  unsigned TotalTests = 0;
  unsigned Passes = 0;
  unsigned Failures = 0;

  for (unsigned f = 0; f < NumFunc8; ++f) {
    for (unsigned i = 0; i < NumValues; ++i) {
      for (unsigned j = 0; j < NumValues; ++j) {
        ++TotalTests;
        ResultSz = Func8[f].FuncSz(Values[i], Values[j]);
        ResultLlc = Func8[f].FuncLlc(Values[i], Values[j]);
        if (ResultSz == ResultLlc) {
          ++Passes;
        } else {
          ++Failures;
          printf("%s(0x%08x, 0x%08x): sz=%d llc=%d\n", Func8[f].Name,
                 Values[i], Values[j], ResultSz, ResultLlc);
        }
      }
    }
  }

  for (unsigned f = 0; f < NumFunc16; ++f) {
    for (unsigned i = 0; i < NumValues; ++i) {
      for (unsigned j = 0; j < NumValues; ++j) {
        ++TotalTests;
        ResultSz = Func16[f].FuncSz(Values[i], Values[j]);
        ResultLlc = Func16[f].FuncLlc(Values[i], Values[j]);
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
        ++TotalTests;
        ResultSz = Func32[f].FuncSz(Values[i], Values[j]);
        ResultLlc = Func32[f].FuncLlc(Values[i], Values[j]);
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
            ++TotalTests;
            ResultLlc = Func64[f].FuncLlc(Value1, Value2);
            ResultSz = Func64[f].FuncSz(Value1, Value2);
            if (ResultSz == ResultLlc) {
              ++Passes;
            } else {
              ++Failures;
              printf("%s(0x%016llx, 0x%016llx): sz=%d llc=%d\n", Func64[f].Name,
                     Value1, Value2, ResultSz, ResultLlc);
            }
          }
        }
      }
    }
  }
  printf("TotalTests=%u Passes=%u Failures=%u\n", TotalTests, Passes, Failures);
  return Failures;
}
