; RUN: %llvm2ice --verbose none %s | FileCheck %s

; Fixed size allocations should always subtract an explicit constant

define void @fixed_4_by_1() nounwind {
  %x = alloca i32, align 4
  call void @f1(i32* %x) nounwind
  ret void
  ; CHECK:      sub     esp, 4
  ; CHECK-NEXT: mov     eax, esp
  ; CHECK-NEXT: push    eax
  ; CHECK-NEXT: call    f1
}

declare void @f1(i32*)

define void @fixed_100_by_4(i32 %n) nounwind {
  %array = alloca i8, i32 400, align 16
  call void @f2(i8* %array) nounwind
  ret void
  ; CHECK:      sub     esp, 400
  ; CHECK-NEXT: mov     eax, esp
  ; CHECK-NEXT: push    eax
  ; CHECK-NEXT: call    f2
}

declare void @f2(i8*)

; Variable size allocations of byte objects should have no multiply or shift

define void @variable_n_by_1(i32 %n) nounwind {
  %array = alloca i8, i32 %n, align 16
  call void @f3(i8* %array) nounwind
  ret void
  ; CHECK:      mov     eax, dword ptr [ebp+8]
  ; CHECK-NEXT: sub     esp, eax
  ; CHECK-NEXT: mov     eax, esp
  ; CHECK-NEXT: push    eax
  ; CHECK-NEXT: call    f3
}

declare void @f3(i8*)

; Variable size allocations of 2^x byte objects should have shift

define void @variable_n_by_2(i32 %n) nounwind {
  %array = alloca i32, i32 %n, align 16
  call void @f4(i32* %array) nounwind
  ret void
  ; CHECK:      mov     eax, dword ptr [ebp+8]
  ; CHECK-NEXT: shl     eax, 2
  ; CHECK-NEXT: sub     esp, eax
  ; CHECK-NEXT: mov     eax, esp
  ; CHECK-NEXT: push    eax
  ; CHECK-NEXT: call    f4
}

declare void @f4(i32*)
