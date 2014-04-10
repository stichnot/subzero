typedef long long int64_t;
typedef unsigned long long uint64_t;
typedef int int32_t;
typedef unsigned uint32_t;

#include "test_icmp.h"

bool icmpEq32Bool(int32_t a, int32_t b) { return a == b; }
bool icmpNe32Bool(int32_t a, int32_t b) { return a != b; }
bool icmpSgt32Bool(int32_t a, int32_t b) { return a > b; }
bool icmpUgt32Bool(uint32_t a, uint32_t b) { return a > b; }
bool icmpSge32Bool(int32_t a, int32_t b) { return a >= b; }
bool icmpUge32Bool(uint32_t a, uint32_t b) { return a >= b; }
bool icmpSlt32Bool(int32_t a, int32_t b) { return a < b; }
bool icmpUlt32Bool(uint32_t a, uint32_t b) { return a < b; }
bool icmpSle32Bool(int32_t a, int32_t b) { return a <= b; }
bool icmpUle32Bool(uint32_t a, uint32_t b) { return a <= b; }

bool icmpEq64Bool(int64_t a, int64_t b) { return a == b; }
bool icmpNe64Bool(int64_t a, int64_t b) { return a != b; }
bool icmpSgt64Bool(int64_t a, int64_t b) { return a > b; }
bool icmpUgt64Bool(uint64_t a, uint64_t b) { return a > b; }
bool icmpSge64Bool(int64_t a, int64_t b) { return a >= b; }
bool icmpUge64Bool(uint64_t a, uint64_t b) { return a >= b; }
bool icmpSlt64Bool(int64_t a, int64_t b) { return a < b; }
bool icmpUlt64Bool(uint64_t a, uint64_t b) { return a < b; }
bool icmpSle64Bool(int64_t a, int64_t b) { return a <= b; }
bool icmpUle64Bool(uint64_t a, uint64_t b) { return a <= b; }
