#include <stdint.h>

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

bool icmpEq16Bool(int16_t a, int16_t b) { return a == b; }
bool icmpNe16Bool(int16_t a, int16_t b) { return a != b; }
bool icmpSgt16Bool(int16_t a, int16_t b) { return a > b; }
bool icmpUgt16Bool(uint16_t a, uint16_t b) { return a > b; }
bool icmpSge16Bool(int16_t a, int16_t b) { return a >= b; }
bool icmpUge16Bool(uint16_t a, uint16_t b) { return a >= b; }
bool icmpSlt16Bool(int16_t a, int16_t b) { return a < b; }
bool icmpUlt16Bool(uint16_t a, uint16_t b) { return a < b; }
bool icmpSle16Bool(int16_t a, int16_t b) { return a <= b; }
bool icmpUle16Bool(uint16_t a, uint16_t b) { return a <= b; }

bool icmpEq8Bool(int8_t a, int8_t b) { return a == b; }
bool icmpNe8Bool(int8_t a, int8_t b) { return a != b; }
bool icmpSgt8Bool(int8_t a, int8_t b) { return a > b; }
bool icmpUgt8Bool(uint8_t a, uint8_t b) { return a > b; }
bool icmpSge8Bool(int8_t a, int8_t b) { return a >= b; }
bool icmpUge8Bool(uint8_t a, uint8_t b) { return a >= b; }
bool icmpSlt8Bool(int8_t a, int8_t b) { return a < b; }
bool icmpUlt8Bool(uint8_t a, uint8_t b) { return a < b; }
bool icmpSle8Bool(int8_t a, int8_t b) { return a <= b; }
bool icmpUle8Bool(uint8_t a, uint8_t b) { return a <= b; }
