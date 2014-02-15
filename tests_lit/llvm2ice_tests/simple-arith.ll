; RUN: %llvm2ice -verbose inst %s | FileCheck %s


define i64 @add_args_i64(i64 %arg1, i64 %arg2) {
entry:
  %add = add nsw i64 %arg2, %arg1
  ret i64 %add
}

; Checks for verbose instruction output

; CHECK: define internal i64 add_args
; CHECK: %add = add i64 %arg2, %arg1
; CHECK-NEXT: ret i64 %add

define i32 @add_args_i32(i32 %arg1, i32 %arg2) {
entry:
  %add = add nsw i32 %arg2, %arg1
  ret i32 %add
}

; CHECK-NOT: ICE translation error

; Checks for emitted assembly

; CHECK:      .globl add_args_i32
; CHECK:      mov eax, dword ptr [esp+4]
; CHECK-NEXT: mov ecx, dword ptr [esp+8]
; CHECK-NEXT: add ecx, eax
; CHECK-NEXT: mov eax, ecx
; CHECK-NEXT: ret
