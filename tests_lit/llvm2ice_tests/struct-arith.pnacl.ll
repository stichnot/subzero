; RUN: %llvm2ice -verbose inst %s | FileCheck %s

; This file is lowered from C code that does some simple aritmetic with
; struct members. It's also built with the PNaCl toolchain so this is the
; stable ABI subset of LLVM IR (structs are gone, pointers turned into i32,
; geps gone, etc.)

define internal i32 @compute_important_function(i32 %v1, i32 %v2) {
entry:
  %v1.asptr = inttoptr i32 %v1 to i32*
  %0 = load i32* %v1.asptr, align 1

; CHECK:        entry:
; CHECK-NEXT:       %v1.asptr = i32 %v1
; CHECK-NEXT:       %{{[0-9_]+}} = load i32* {{.*}}, align 1

  %v2.asptr = inttoptr i32 %v2 to i32*
  %1 = load i32* %v2.asptr, align 1
  %gep = add i32 %v2, 12
  %gep.asptr = inttoptr i32 %gep to i32*
  %2 = load i32* %gep.asptr, align 1
  %mul = mul i32 %2, %1
  %gep6 = add i32 %v1, 4
  %gep6.asptr = inttoptr i32 %gep6 to i32*
  %3 = load i32* %gep6.asptr, align 1
  %gep8 = add i32 %v2, 8
  %gep8.asptr = inttoptr i32 %gep8 to i32*
  %4 = load i32* %gep8.asptr, align 1
  %gep10 = add i32 %v2, 4
  %gep10.asptr = inttoptr i32 %gep10 to i32*
  %5 = load i32* %gep10.asptr, align 1
  %mul3 = mul i32 %5, %4
  %gep12 = add i32 %v1, 8
  %gep12.asptr = inttoptr i32 %gep12 to i32*
  %6 = load i32* %gep12.asptr, align 1
  %mul7 = mul i32 %6, %3
  %mul9 = mul i32 %mul7, %6
  %gep14 = add i32 %v1, 12
  %gep14.asptr = inttoptr i32 %gep14 to i32*
  %7 = load i32* %gep14.asptr, align 1
  %mul11 = mul i32 %mul9, %7
  %add4.neg = add i32 %mul, %0
  %add = sub i32 %add4.neg, %3
  %sub = sub i32 %add, %mul3
  %sub12 = sub i32 %sub, %mul11
  ret i32 %sub12

; CHECK:        %sub12 = sub i32 %sub, %mul11
; CHECK-NEXT:       ret i32 %sub12
}

; CHECK-NOT: ICE translation error
