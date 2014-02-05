; RUN: %llvm2ice %s | FileCheck %s
; Check that shift left and 

@i1 = common global i32 0, align 4
@i2 = common global i32 0, align 4
@u1 = common global i32 0, align 4
@u2 = common global i32 0, align 4

define void @conv1() nounwind {
  %1 = load i32* @u1, align 4
  %sext = shl i32 %1, 24
  %2 = ashr exact i32 %sext, 24
  store i32 %2, i32* @i1, align 4
  ret void
  ; CHECK: eax = shl.i32 eax, 24
  ; CHECK-NEXT: eax = sar.i32 eax, 24
}

define void @conv2() nounwind {
  %1 = load i32* @u1, align 4
  %sext1 = shl i32 %1, 16
  %2 = ashr exact i32 %sext1, 16
  store i32 %2, i32* @i2, align 4
  ret void
  ; CHECK: eax = shl.i32 eax, 16
  ; CHECK-NEXT: eax = sar.i32 eax, 16
}
