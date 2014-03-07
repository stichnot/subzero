// abtest.py --test=test_icmp.cpp --driver=test_icmp_main.cpp --prefix=Subzero_ --output=test_icmp

#include <stdio.h>
#include <stdint.h>

extern "C" {

  bool icmpEq32Bool(int32_t a, int32_t b);
  bool icmpNe32Bool(int32_t a, int32_t b);
  bool icmpSgt32Bool(int32_t a, int32_t b);
  bool icmpUgt32Bool(uint32_t a, uint32_t b);
  bool icmpSge32Bool(int32_t a, int32_t b);
  bool icmpUge32Bool(uint32_t a, uint32_t b);
  bool icmpSlt32Bool(int32_t a, int32_t b);
  bool icmpUlt32Bool(uint32_t a, uint32_t b);
  bool icmpSle32Bool(int32_t a, int32_t b);
  bool icmpUle32Bool(uint32_t a, uint32_t b);

  bool icmpEq64Bool(int64_t a, int64_t b);
  bool icmpNe64Bool(int64_t a, int64_t b);
  bool icmpSgt64Bool(int64_t a, int64_t b);
  bool icmpUgt64Bool(uint64_t a, uint64_t b);
  bool icmpSge64Bool(int64_t a, int64_t b);
  bool icmpUge64Bool(uint64_t a, uint64_t b);
  bool icmpSlt64Bool(int64_t a, int64_t b);
  bool icmpUlt64Bool(uint64_t a, uint64_t b);
  bool icmpSle64Bool(int64_t a, int64_t b);
  bool icmpUle64Bool(uint64_t a, uint64_t b);

  bool Subzero_icmpEq32Bool(int32_t a, int32_t b);
  bool Subzero_icmpNe32Bool(int32_t a, int32_t b);
  bool Subzero_icmpSgt32Bool(int32_t a, int32_t b);
  bool Subzero_icmpUgt32Bool(uint32_t a, uint32_t b);
  bool Subzero_icmpSge32Bool(int32_t a, int32_t b);
  bool Subzero_icmpUge32Bool(uint32_t a, uint32_t b);
  bool Subzero_icmpSlt32Bool(int32_t a, int32_t b);
  bool Subzero_icmpUlt32Bool(uint32_t a, uint32_t b);
  bool Subzero_icmpSle32Bool(int32_t a, int32_t b);
  bool Subzero_icmpUle32Bool(uint32_t a, uint32_t b);

  bool Subzero_icmpEq64Bool(int64_t a, int64_t b);
  bool Subzero_icmpNe64Bool(int64_t a, int64_t b);
  bool Subzero_icmpSgt64Bool(int64_t a, int64_t b);
  bool Subzero_icmpUgt64Bool(uint64_t a, uint64_t b);
  bool Subzero_icmpSge64Bool(int64_t a, int64_t b);
  bool Subzero_icmpUge64Bool(uint64_t a, uint64_t b);
  bool Subzero_icmpSlt64Bool(int64_t a, int64_t b);
  bool Subzero_icmpUlt64Bool(uint64_t a, uint64_t b);
  bool Subzero_icmpSle64Bool(int64_t a, int64_t b);
  bool Subzero_icmpUle64Bool(uint64_t a, uint64_t b);
}

int main(int argc, char **argv) {
  static unsigned Values[] = {
    0x0,
    0x1,
    0x7ffffffe,
    0x7fffffff,
    0x80000000,
    0x80000001,
    0xfffffffe,
    0xffffffff,
  };
  const static unsigned NumValues = sizeof(Values) / sizeof(*Values);
  typedef bool (*FuncType32)(uint32_t, uint32_t);
  static struct {
    const char *Name;
    FuncType32 FuncSz;
    FuncType32 FuncLlc;
  } Func32[] = {
    {"icmpEq32Bool", (FuncType32)Subzero_icmpEq32Bool, (FuncType32)icmpEq32Bool},
    {"icmpNe32Bool", (FuncType32)Subzero_icmpNe32Bool, (FuncType32)icmpNe32Bool},
    {"icmpSgt32Bool", (FuncType32)Subzero_icmpSgt32Bool, (FuncType32)icmpSgt32Bool},
    {"icmpUgt32Bool", (FuncType32)Subzero_icmpUgt32Bool, (FuncType32)icmpUgt32Bool},
    {"icmpSge32Bool", (FuncType32)Subzero_icmpSge32Bool, (FuncType32)icmpSge32Bool},
    {"icmpUge32Bool", (FuncType32)Subzero_icmpUge32Bool, (FuncType32)icmpUge32Bool},
    {"icmpSlt32Bool", (FuncType32)Subzero_icmpSlt32Bool, (FuncType32)icmpSlt32Bool},
    {"icmpUlt32Bool", (FuncType32)Subzero_icmpUlt32Bool, (FuncType32)icmpUlt32Bool},
    {"icmpSle32Bool", (FuncType32)Subzero_icmpSle32Bool, (FuncType32)icmpSle32Bool},
    {"icmpUle32Bool", (FuncType32)Subzero_icmpUle32Bool, (FuncType32)icmpUle32Bool},
  };
  const static unsigned NumFunc32 = sizeof(Func32) / sizeof(*Func32);

  typedef bool (*FuncType64)(uint64_t, uint64_t);
  static struct {
    const char *Name;
    FuncType64 FuncSz;
    FuncType64 FuncLlc;
  } Func64[] = {
    {"icmpEq64Bool", (FuncType64)Subzero_icmpEq64Bool, (FuncType64)icmpEq64Bool},
    {"icmpNe64Bool", (FuncType64)Subzero_icmpNe64Bool, (FuncType64)icmpNe64Bool},
    {"icmpSgt64Bool", (FuncType64)Subzero_icmpSgt64Bool, (FuncType64)icmpSgt64Bool},
    {"icmpUgt64Bool", (FuncType64)Subzero_icmpUgt64Bool, (FuncType64)icmpUgt64Bool},
    {"icmpSge64Bool", (FuncType64)Subzero_icmpSge64Bool, (FuncType64)icmpSge64Bool},
    {"icmpUge64Bool", (FuncType64)Subzero_icmpUge64Bool, (FuncType64)icmpUge64Bool},
    {"icmpSlt64Bool", (FuncType64)Subzero_icmpSlt64Bool, (FuncType64)icmpSlt64Bool},
    {"icmpUlt64Bool", (FuncType64)Subzero_icmpUlt64Bool, (FuncType64)icmpUlt64Bool},
    {"icmpSle64Bool", (FuncType64)Subzero_icmpSle64Bool, (FuncType64)icmpSle64Bool},
    {"icmpUle64Bool", (FuncType64)Subzero_icmpUle64Bool, (FuncType64)icmpUle64Bool},
  };
  const static unsigned NumFunc64 = sizeof(Func64) / sizeof(*Func64);

  bool ResultSz, ResultLlc;

  unsigned TotalTests = 0;
  unsigned Passes = 0;
  unsigned Failures = 0;
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
          printf("%s(0x%08x, 0x%08x): sz=%d llc=%d\n",
                 Func32[f].Name, Values[i], Values[j], ResultSz, ResultLlc);
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
              printf("%s(0x%016llx, 0x%016llx): sz=%d llc=%d\n",
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
