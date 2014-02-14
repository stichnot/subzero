; RUN: %llvm2ice -verbose inst %s | FileCheck %s

@intern_global = global i32 12, align 4
@extern_global = external global i32

define i32 @test_intern_global() #0 {
; CHECK: define internal i32 test_intern_global
entry:
  %0 = load i32* @intern_global, align 4
  ret i32 %0
}

define i32 @test_extern_global() #0 {
; CHECK: define internal i32 test_extern_global
entry:
  %0 = load i32* @extern_global, align 4
  ret i32 %0
}

; CHECK-NOT: ICE translation error
