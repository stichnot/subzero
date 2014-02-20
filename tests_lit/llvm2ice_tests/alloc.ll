; RUN: %llvm2ice --verbose none %s | FileCheck %s

define void @fixed_400(i32 %n) nounwind {
  %array = alloca i8, i32 400, align 16
  call void @f1(i8* %array) nounwind
  ret void
  ; CHECK:      sub     esp, 400
  ; CHECK-NEXT: mov     eax, esp
  ; CHECK-NEXT: push    eax
  ; CHECK-NEXT: call    f1
}

declare void @f1(i8*)

define void @variable_n(i32 %n) nounwind {
  %array = alloca i8, i32 %n, align 16
  call void @f2(i8* %array) nounwind
  ret void
  ; CHECK:      mov     eax, dword ptr [ebp+8]
  ; CHECK-NEXT: sub     esp, eax
  ; CHECK-NEXT: mov     eax, esp
  ; CHECK-NEXT: push    eax
  ; CHECK-NEXT: call    f2
}

declare void @f2(i8*)
