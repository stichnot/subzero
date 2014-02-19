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
  ; CHECK: mov ecx, byte ptr [
  ; CHECK-NEXT: movsbw edx, ecx
  ; CHECK-NEXT: mov word ptr [
  ; CHECK-NEXT: movsbl ecx, ecx
  ; CHECK-NEXT: mov dword ptr [
  ; Note: "mov qword ptr" is broken and should be fixed with i64 Store lowering.
  ; CHECK-NEXT: mov qword ptr [
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
  ; CHECK: mov ecx, word ptr [
  ; CHECK-NEXT: mov edx, ecx
  ; CHECK-NEXT: mov byte ptr [
  ; CHECK-NEXT: movswl ecx, ecx
  ; CHECK-NEXT: mov dword ptr [
  ; Note: "mov qword ptr" is broken and should be fixed with i64 Store lowering.
  ; CHECK-NEXT: mov qword ptr [
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
  ; CHECK: mov ecx, dword ptr [
  ; CHECK-NEXT: mov edx, ecx
  ; CHECK-NEXT: mov byte ptr [
  ; CHECK-NEXT: mov word ptr [
  ; Note: "mov qword ptr" is broken and should be fixed with i64 Store lowering.
  ; CHECK-NEXT: mov qword ptr [
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
  ; CHECK: mov eax, dword ptr [
  ; CHECK-NEXT: mov ecx, eax
  ; CHECK-NEXT: mov byte ptr [
  ; CHECK-NEXT: mov ecx, eax
  ; CHECK-NEXT: mov word ptr [
  ; CHECK-NEXT: mov dword ptr [
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
  ; CHECK: mov ecx, byte ptr [
  ; CHECK-NEXT: movzbw edx, ecx
  ; CHECK-NEXT: mov word ptr [
  ; CHECK-NEXT: movzbl ecx, ecx
  ; CHECK-NEXT: mov dword ptr [
  ; Note: "mov qword ptr" is broken and should be fixed with i64 Store lowering.
  ; CHECK-NEXT: mov qword ptr [
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
  ; CHECK: mov ecx, word ptr [
  ; CHECK-NEXT: mov edx, ecx
  ; CHECK-NEXT: mov byte ptr [
  ; CHECK-NEXT: movzwl ecx, ecx
  ; CHECK-NEXT: mov dword ptr [
  ; Note: "mov qword ptr" is broken and should be fixed with i64 Store lowering.
  ; CHECK-NEXT: mov qword ptr [
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
  ; CHECK: mov ecx, dword ptr [
  ; CHECK-NEXT: mov edx, ecx
  ; CHECK-NEXT: mov byte ptr [
  ; CHECK-NEXT: mov word ptr [
  ; Note: "mov qword ptr" is broken and should be fixed with i64 Store lowering.
  ; CHECK-NEXT: mov qword ptr [
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
  ; CHECK: mov eax, dword ptr [
  ; CHECK-NEXT: mov ecx, eax
  ; CHECK-NEXT: mov byte ptr [
  ; CHECK-NEXT: mov ecx, eax
  ; CHECK-NEXT: mov word ptr [
  ; CHECK-NEXT: mov dword ptr [
}
