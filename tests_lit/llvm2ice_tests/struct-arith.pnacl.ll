; RUN: %llvm2ice --verbose inst %s | FileCheck %s
; RUN: %llvm2ice --verbose none %s | FileCheck --check-prefix=ERRORS %s
; RUN: %szdiff --llvm2ice=%llvm2ice %s | FileCheck --check-prefix=DUMP %s

; This file is lowered from C code that does some simple aritmetic with
; struct members. It's also built with the PNaCl toolchain so this is the
; stable ABI subset of LLVM IR (structs are gone, pointers turned into i32,
; geps gone, etc.)

define internal i32 @compute_important_function(i32 %v1, i32 %v2) {
entry:
  %v1.asptr = inttoptr i32 %v1 to i32*
  %_v0 = load i32* %v1.asptr, align 1

; CHECK:        entry:
; CHECK-NEXT:       %v1.asptr = i32 %v1
; CHECK-NEXT:       %_v0 = load i32* {{.*}}, align 1

  %v2.asptr = inttoptr i32 %v2 to i32*
  %_v1 = load i32* %v2.asptr, align 1
  %gep = add i32 %v2, 12
  %gep.asptr = inttoptr i32 %gep to i32*
  %_v2 = load i32* %gep.asptr, align 1
  %mul = mul i32 %_v2, %_v1
  %gep6 = add i32 %v1, 4
  %gep6.asptr = inttoptr i32 %gep6 to i32*
  %_v3 = load i32* %gep6.asptr, align 1
  %gep8 = add i32 %v2, 8
  %gep8.asptr = inttoptr i32 %gep8 to i32*
  %_v4 = load i32* %gep8.asptr, align 1
  %gep10 = add i32 %v2, 4
  %gep10.asptr = inttoptr i32 %gep10 to i32*
  %_v5 = load i32* %gep10.asptr, align 1
  %mul3 = mul i32 %_v5, %_v4
  %gep12 = add i32 %v1, 8
  %gep12.asptr = inttoptr i32 %gep12 to i32*
  %_v6 = load i32* %gep12.asptr, align 1
  %mul7 = mul i32 %_v6, %_v3
  %mul9 = mul i32 %mul7, %_v6
  %gep14 = add i32 %v1, 12
  %gep14.asptr = inttoptr i32 %gep14 to i32*
  %_v7 = load i32* %gep14.asptr, align 1
  %mul11 = mul i32 %mul9, %_v7
  %add4.neg = add i32 %mul, %_v0
  %add = sub i32 %add4.neg, %_v3
  %sub = sub i32 %add, %mul3
  %sub12 = sub i32 %sub, %mul11
  ret i32 %sub12

; CHECK:        %sub12 = sub i32 %sub, %mul11
; CHECK-NEXT:       ret i32 %sub12
}

; ERRORS-NOT: ICE translation error
; DUMP-NOT: SZ
