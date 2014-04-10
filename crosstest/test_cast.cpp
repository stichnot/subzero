#include <stdint.h>

#if 0
// The bitcode for *ToUi1() needs to be hand-modified to change the
// icmp back to trunc or fptoui.
bool castUi64ToUi1(uint64_t a) { return (bool) a; }
bool castSi64ToUi1(int64_t a) { return (bool) a; }
bool castUi32ToUi1(uint32_t a) { return (bool) a; }
bool castSi32ToUi1(int32_t a) { return (bool) a; }
bool castUi16ToUi1(unsigned short a) { return (bool) a; }
bool castSi16ToUi1(short a) { return (bool) a; }
bool castUi8ToUi1(unsigned char a) { return (bool) a; }
bool castSi8ToUi1(signed char a) { return (bool) a; }
bool castUi1ToUi1(bool a) { return (bool) a; }
bool castF64ToUi1(double a) { return (bool) a; }
bool castF32ToUi1(float a) { return (bool) a; }
#endif

signed char castUi64ToSi8(uint64_t a) { return (signed char)a; }
signed char castSi64ToSi8(int64_t a) { return (signed char)a; }
signed char castUi32ToSi8(uint32_t a) { return (signed char)a; }
signed char castSi32ToSi8(int32_t a) { return (signed char)a; }
signed char castUi16ToSi8(unsigned short a) { return (signed char)a; }
signed char castSi16ToSi8(short a) { return (signed char)a; }
signed char castUi8ToSi8(unsigned char a) { return (signed char)a; }
signed char castSi8ToSi8(signed char a) { return (signed char)a; }
signed char castUi1ToSi8(bool a) { return (signed char)a; }
signed char castF64ToSi8(double a) { return (signed char)a; }
signed char castF32ToSi8(float a) { return (signed char)a; }

unsigned char castUi64ToUi8(uint64_t a) { return (unsigned char)a; }
unsigned char castSi64ToUi8(int64_t a) { return (unsigned char)a; }
unsigned char castUi32ToUi8(uint32_t a) { return (unsigned char)a; }
unsigned char castSi32ToUi8(int32_t a) { return (unsigned char)a; }
unsigned char castUi16ToUi8(unsigned short a) { return (unsigned char)a; }
unsigned char castSi16ToUi8(short a) { return (unsigned char)a; }
unsigned char castUi8ToUi8(unsigned char a) { return (unsigned char)a; }
unsigned char castSi8ToUi8(signed char a) { return (unsigned char)a; }
unsigned char castUi1ToUi8(bool a) { return (unsigned char)a; }
unsigned char castF64ToUi8(double a) { return (unsigned char)a; }
unsigned char castF32ToUi8(float a) { return (unsigned char)a; }

short castUi64ToSi16(uint64_t a) { return (short)a; }
short castSi64ToSi16(int64_t a) { return (short)a; }
short castUi32ToSi16(uint32_t a) { return (short)a; }
short castSi32ToSi16(int32_t a) { return (short)a; }
short castUi16ToSi16(unsigned short a) { return (short)a; }
short castSi16ToSi16(short a) { return (short)a; }
short castUi8ToSi16(unsigned char a) { return (short)a; }
short castSi8ToSi16(signed char a) { return (short)a; }
short castUi1ToSi16(bool a) { return (short)a; }
short castF64ToSi16(double a) { return (short)a; }
short castF32ToSi16(float a) { return (short)a; }

unsigned short castUi64ToUi16(uint64_t a) { return (unsigned short)a; }
unsigned short castSi64ToUi16(int64_t a) { return (unsigned short)a; }
unsigned short castUi32ToUi16(uint32_t a) { return (unsigned short)a; }
unsigned short castSi32ToUi16(int32_t a) { return (unsigned short)a; }
unsigned short castUi16ToUi16(unsigned short a) { return (unsigned short)a; }
unsigned short castSi16ToUi16(short a) { return (unsigned short)a; }
unsigned short castUi8ToUi16(unsigned char a) { return (unsigned short)a; }
unsigned short castSi8ToUi16(signed char a) { return (unsigned short)a; }
unsigned short castUi1ToUi16(bool a) { return (unsigned short)a; }
unsigned short castF64ToUi16(double a) { return (unsigned short)a; }
unsigned short castF32ToUi16(float a) { return (unsigned short)a; }

int32_t castUi64ToSi32(uint64_t a) { return (int32_t)a; }
int32_t castSi64ToSi32(int64_t a) { return (int32_t)a; }
int32_t castUi32ToSi32(uint32_t a) { return (int32_t)a; }
int32_t castSi32ToSi32(int32_t a) { return (int32_t)a; }
int32_t castUi16ToSi32(unsigned short a) { return (int32_t)a; }
int32_t castSi16ToSi32(short a) { return (int32_t)a; }
int32_t castUi8ToSi32(unsigned char a) { return (int32_t)a; }
int32_t castSi8ToSi32(signed char a) { return (int32_t)a; }
int32_t castUi1ToSi32(bool a) { return (int32_t)a; }
int32_t castF64ToSi32(double a) { return (int32_t)a; }
int32_t castF32ToSi32(float a) { return (int32_t)a; }

uint32_t castUi64ToUi32(uint64_t a) { return (uint32_t)a; }
uint32_t castSi64ToUi32(int64_t a) { return (uint32_t)a; }
uint32_t castUi32ToUi32(uint32_t a) { return (uint32_t)a; }
uint32_t castSi32ToUi32(int32_t a) { return (uint32_t)a; }
uint32_t castUi16ToUi32(unsigned short a) { return (uint32_t)a; }
uint32_t castSi16ToUi32(short a) { return (uint32_t)a; }
uint32_t castUi8ToUi32(unsigned char a) { return (uint32_t)a; }
uint32_t castSi8ToUi32(signed char a) { return (uint32_t)a; }
uint32_t castUi1ToUi32(bool a) { return (uint32_t)a; }
uint32_t castF64ToUi32(double a) { return (uint32_t)a; }
uint32_t castF32ToUi32(float a) { return (uint32_t)a; }

int64_t castUi64ToSi64(uint64_t a) { return (int64_t)a; }
int64_t castSi64ToSi64(int64_t a) { return (int64_t)a; }
int64_t castUi32ToSi64(uint32_t a) { return (int64_t)a; }
int64_t castSi32ToSi64(int32_t a) { return (int64_t)a; }
int64_t castUi16ToSi64(unsigned short a) { return (int64_t)a; }
int64_t castSi16ToSi64(short a) { return (int64_t)a; }
int64_t castUi8ToSi64(unsigned char a) { return (int64_t)a; }
int64_t castSi8ToSi64(signed char a) { return (int64_t)a; }
int64_t castUi1ToSi64(bool a) { return (int64_t)a; }
int64_t castF64ToSi64(double a) { return (int64_t)a; }
int64_t castF32ToSi64(float a) { return (int64_t)a; }

uint64_t castUi64ToUi64(uint64_t a) { return (uint64_t)a; }
uint64_t castSi64ToUi64(int64_t a) { return (uint64_t)a; }
uint64_t castUi32ToUi64(uint32_t a) { return (uint64_t)a; }
uint64_t castSi32ToUi64(int32_t a) { return (uint64_t)a; }
uint64_t castUi16ToUi64(unsigned short a) { return (uint64_t)a; }
uint64_t castSi16ToUi64(short a) { return (uint64_t)a; }
uint64_t castUi8ToUi64(unsigned char a) { return (uint64_t)a; }
uint64_t castSi8ToUi64(signed char a) { return (uint64_t)a; }
uint64_t castUi1ToUi64(bool a) { return (uint64_t)a; }
uint64_t castF64ToUi64(double a) { return (uint64_t)a; }
uint64_t castF32ToUi64(float a) { return (uint64_t)a; }

float castUi64ToF32(uint64_t a) { return (float)a; }
float castSi64ToF32(int64_t a) { return (float)a; }
float castUi32ToF32(uint32_t a) { return (float)a; }
float castSi32ToF32(int32_t a) { return (float)a; }
float castUi16ToF32(unsigned short a) { return (float)a; }
float castSi16ToF32(short a) { return (float)a; }
float castUi8ToF32(unsigned char a) { return (float)a; }
float castSi8ToF32(signed char a) { return (float)a; }
float castUi1ToF32(bool a) { return (float)a; }
float castF64ToF32(double a) { return (float)a; }
float castF32ToF32(float a) { return (float)a; }

double castUi64ToF64(uint64_t a) { return (double)a; }
double castSi64ToF64(int64_t a) { return (double)a; }
double castUi32ToF64(uint32_t a) { return (double)a; }
double castSi32ToF64(int32_t a) { return (double)a; }
double castUi16ToF64(unsigned short a) { return (double)a; }
double castSi16ToF64(short a) { return (double)a; }
double castUi8ToF64(unsigned char a) { return (double)a; }
double castSi8ToF64(signed char a) { return (double)a; }
double castUi1ToF64(bool a) { return (double)a; }
double castF64ToF64(double a) { return (double)a; }
double castF32ToF64(float a) { return (double)a; }

uint32_t castbits_F32ToUi32(float a) { return *(uint32_t *)&a; }
float castbits_Ui32ToF32(uint32_t a) { return *(float *)&a; }
uint64_t castbits_F64ToUi64(double a) { return *(uint64_t *)&a; }
double castbits_Ui64ToF64(uint64_t a) { return *(double *)&a; }
