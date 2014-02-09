; RUN: %llvm2ice %s | FileCheck %s

@i8v = common global i8 0, align 1
@i16v = common global i16 0, align 2
@i32v = common global i32 0, align 4
@i64v = common global i64 0, align 8
@u8v = common global i8 0, align 1
@u16v = common global i16 0, align 2
@u32v = common global i32 0, align 4
@u64v = common global i64 0, align 8
@i1 = common global i32 0, align 4
@i2 = common global i32 0, align 4
@u1 = common global i32 0, align 4
@u2 = common global i32 0, align 4

define void @from_int8() nounwind {
  %1 = load i8* @i8v, align 1
  %2 = sext i8 %1 to i16
  store i16 %2, i16* @i16v, align 2
  %3 = sext i8 %1 to i32
  store i32 %3, i32* @i32v, align 4
  %4 = sext i8 %1 to i64
  store i64 %4, i64* @i64v, align 8
  ret void
  ; CHECK: mov.i8 eax, [
  ; CHECK-NEXT: movsbw ecx, eax
  ; CHECK-NEXT: mov.i16 [
  ; CHECK-NEXT: movsbl ecx, eax
  ; CHECK-NEXT: mov.i32 [
  ; CHECK-NEXT: movsbq eax, eax
  ; CHECK-NEXT: mov.i64 [
}

define void @from_int16() nounwind {
  %1 = load i16* @i16v, align 2
  %2 = trunc i16 %1 to i8
  store i8 %2, i8* @i8v, align 1
  %3 = sext i16 %1 to i32
  store i32 %3, i32* @i32v, align 4
  %4 = sext i16 %1 to i64
  store i64 %4, i64* @i64v, align 8
  ret void
  ; CHECK: mov.i16 eax, [
  ; CHECK-NEXT: mov.i8 ecx, eax
  ; CHECK-NEXT: mov.i8 [
  ; CHECK-NEXT: movswl ecx, eax
  ; CHECK-NEXT: mov.i32 [
  ; CHECK-NEXT: movswq eax, eax
  ; CHECK-NEXT: mov.i64 [
}

define void @from_int32() nounwind {
  %1 = load i32* @i32v, align 4
  %2 = trunc i32 %1 to i8
  store i8 %2, i8* @i8v, align 1
  %3 = trunc i32 %1 to i16
  store i16 %3, i16* @i16v, align 2
  %4 = sext i32 %1 to i64
  store i64 %4, i64* @i64v, align 8
  ret void
  ; CHECK: mov.i32 eax, [
  ; CHECK-NEXT: mov.i8 ecx, eax
  ; CHECK-NEXT: mov.i8 [
  ; CHECK-NEXT: mov.i16 ecx, eax
  ; CHECK-NEXT: mov.i16 [
  ; CHECK-NEXT: movslq eax, eax
  ; CHECK-NEXT: mov.i64 [
}

define void @from_int64() nounwind {
  %1 = load i64* @i64v, align 8
  %2 = trunc i64 %1 to i8
  store i8 %2, i8* @i8v, align 1
  %3 = trunc i64 %1 to i16
  store i16 %3, i16* @i16v, align 2
  %4 = trunc i64 %1 to i32
  store i32 %4, i32* @i32v, align 4
  ret void
  ; CHECK: mov.i64 eax, [
  ; CHECK-NEXT: mov.i8 ecx, eax
  ; CHECK-NEXT: mov.i8 [
  ; CHECK-NEXT: mov.i16 ecx, eax
  ; CHECK-NEXT: mov.i16 [
  ; CHECK-NEXT: mov.i32 [
}

define void @from_uint8() nounwind {
  %1 = load i8* @u8v, align 1
  %2 = zext i8 %1 to i16
  store i16 %2, i16* @i16v, align 2
  %3 = zext i8 %1 to i32
  store i32 %3, i32* @i32v, align 4
  %4 = zext i8 %1 to i64
  store i64 %4, i64* @i64v, align 8
  ret void
  ; CHECK: mov.i8 eax, [
  ; CHECK-NEXT: movzbw ecx, eax
  ; CHECK-NEXT: mov.i16 [
  ; CHECK-NEXT: movzbl ecx, eax
  ; CHECK-NEXT: mov.i32 [
  ; CHECK-NEXT: movzbq eax, eax
  ; CHECK-NEXT: mov.i64 [
}

define void @from_uint16() nounwind {
  %1 = load i16* @u16v, align 2
  %2 = trunc i16 %1 to i8
  store i8 %2, i8* @i8v, align 1
  %3 = zext i16 %1 to i32
  store i32 %3, i32* @i32v, align 4
  %4 = zext i16 %1 to i64
  store i64 %4, i64* @i64v, align 8
  ret void
  ; CHECK: mov.i16 eax, [
  ; CHECK-NEXT: mov.i8 ecx, eax
  ; CHECK-NEXT: mov.i8 [
  ; CHECK-NEXT: movzwl ecx, eax
  ; CHECK-NEXT: mov.i32 [
  ; CHECK-NEXT: movzwq eax, eax
  ; CHECK-NEXT: mov.i64 [
}

define void @from_uint32() nounwind {
  %1 = load i32* @u32v, align 4
  %2 = trunc i32 %1 to i8
  store i8 %2, i8* @i8v, align 1
  %3 = trunc i32 %1 to i16
  store i16 %3, i16* @i16v, align 2
  %4 = zext i32 %1 to i64
  store i64 %4, i64* @i64v, align 8
  ret void
  ; CHECK: mov.i32 eax, [
  ; CHECK-NEXT: mov.i8 ecx, eax
  ; CHECK-NEXT: mov.i8 [
  ; CHECK-NEXT: mov.i16 ecx, eax
  ; CHECK-NEXT: mov.i16 [
  ; CHECK-NEXT: movzlq eax, eax
  ; CHECK-NEXT: mov.i64 [
}

define void @from_uint64() nounwind {
  %1 = load i64* @u64v, align 8
  %2 = trunc i64 %1 to i8
  store i8 %2, i8* @i8v, align 1
  %3 = trunc i64 %1 to i16
  store i16 %3, i16* @i16v, align 2
  %4 = trunc i64 %1 to i32
  store i32 %4, i32* @i32v, align 4
  ret void
  ; CHECK: mov.i64 eax, [
  ; CHECK-NEXT: mov.i8 ecx, eax
  ; CHECK-NEXT: mov.i8 [
  ; CHECK-NEXT: mov.i16 ecx, eax
  ; CHECK-NEXT: mov.i16 [
  ; CHECK-NEXT: mov.i32 [
}
