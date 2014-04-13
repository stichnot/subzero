#include <stdint.h>

#include "test_arith.h"

uint64_t testAdd(uint64_t a, uint64_t b) { return a + b; }
uint32_t testAdd(uint32_t a, uint32_t b) { return a + b; }
uint16_t testAdd(uint16_t a, uint16_t b) { return a + b; }
uint8_t testAdd(uint8_t a, uint8_t b) { return a + b; }
bool testAdd(bool a, bool b) { return a + b; }

double testFadd(double a, double b) { return a + b; }
float testFadd(float a, float b) { return a + b; }

uint64_t testSub(uint64_t a, uint64_t b) { return a - b; }
uint32_t testSub(uint32_t a, uint32_t b) { return a - b; }
uint16_t testSub(uint16_t a, uint16_t b) { return a - b; }
uint8_t testSub(uint8_t a, uint8_t b) { return a - b; }
bool testSub(bool a, bool b) { return a - b; }

double testFsub(double a, double b) { return a - b; }
float testFsub(float a, float b) { return a - b; }

uint64_t testMul(uint64_t a, uint64_t b) { return a * b; }
uint32_t testMul(uint32_t a, uint32_t b) { return a * b; }
uint16_t testMul(uint16_t a, uint16_t b) { return a * b; }
uint8_t testMul(uint8_t a, uint8_t b) { return a * b; }
bool testMul(bool a, bool b) { return a * b; }

double testFmul(double a, double b) { return a * b; }
float testFmul(float a, float b) { return a * b; }

uint64_t testUdiv(uint64_t a, uint64_t b) { return a / b; }
uint32_t testUdiv(uint32_t a, uint32_t b) { return a / b; }
uint16_t testUdiv(uint16_t a, uint16_t b) { return a / b; }
uint8_t testUdiv(uint8_t a, uint8_t b) { return a / b; }
bool testUdiv(bool a, bool b) { return a / b; }

int64_t testSdiv(int64_t a, int64_t b) { return a / b; }
int32_t testSdiv(int32_t a, int32_t b) { return a / b; }
int16_t testSdiv(int16_t a, int16_t b) { return a / b; }
int8_t testSdiv(int8_t a, int8_t b) { return a / b; }
bool testSdiv(bool a, bool b) { return a / b; }

double testFdiv(double a, double b) { return a / b; }
float testFdiv(float a, float b) { return a / b; }

uint64_t testUrem(uint64_t a, uint64_t b) { return a % b; }
uint32_t testUrem(uint32_t a, uint32_t b) { return a % b; }
uint16_t testUrem(uint16_t a, uint16_t b) { return a % b; }
uint8_t testUrem(uint8_t a, uint8_t b) { return a % b; }
bool testUrem(bool a, bool b) { return a % b; }

int64_t testSrem(int64_t a, int64_t b) { return a % b; }
int32_t testSrem(int32_t a, int32_t b) { return a % b; }
int16_t testSrem(int16_t a, int16_t b) { return a % b; }
int8_t testSrem(int8_t a, int8_t b) { return a % b; }
bool testSrem(bool a, bool b) { return a % b; }

// double testFrem(double a, double b) { return a % b; }
// float testFrem(float a, float b) { return a % b; }

uint64_t testShl(uint64_t a, uint64_t b) { return a << b; }
uint32_t testShl(uint32_t a, uint32_t b) { return a << b; }
uint16_t testShl(uint16_t a, uint16_t b) { return a << b; }
uint8_t testShl(uint8_t a, uint8_t b) { return a << b; }
bool testShl(bool a, bool b) { return a << b; }

uint64_t testLshr(uint64_t a, uint64_t b) { return a >> b; }
uint32_t testLshr(uint32_t a, uint32_t b) { return a >> b; }
uint16_t testLshr(uint16_t a, uint16_t b) { return a >> b; }
uint8_t testLshr(uint8_t a, uint8_t b) { return a >> b; }
bool testLshr(bool a, bool b) { return a >> b; }

int64_t testAshr(int64_t a, int64_t b) { return a >> b; }
int32_t testAshr(int32_t a, int32_t b) { return a >> b; }
int16_t testAshr(int16_t a, int16_t b) { return a >> b; }
int8_t testAshr(int8_t a, int8_t b) { return a >> b; }
bool testAshr(bool a, bool b) { return a >> b; }

uint64_t testAnd(uint64_t a, uint64_t b) { return a & b; }
uint32_t testAnd(uint32_t a, uint32_t b) { return a & b; }
uint16_t testAnd(uint16_t a, uint16_t b) { return a & b; }
uint8_t testAnd(uint8_t a, uint8_t b) { return a & b; }
bool testAnd(bool a, bool b) { return a & b; }

uint64_t testOr(uint64_t a, uint64_t b) { return a | b; }
uint32_t testOr(uint32_t a, uint32_t b) { return a | b; }
uint16_t testOr(uint16_t a, uint16_t b) { return a | b; }
uint8_t testOr(uint8_t a, uint8_t b) { return a | b; }
bool testOr(bool a, bool b) { return a | b; }

uint64_t testXor(uint64_t a, uint64_t b) { return a ^ b; }
uint32_t testXor(uint32_t a, uint32_t b) { return a ^ b; }
uint16_t testXor(uint16_t a, uint16_t b) { return a ^ b; }
uint8_t testXor(uint8_t a, uint8_t b) { return a ^ b; }
bool testXor(bool a, bool b) { return a ^ b; }
