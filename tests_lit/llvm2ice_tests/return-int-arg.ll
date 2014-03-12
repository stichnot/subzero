; RUN: %llvm2ice -verbose inst %s | FileCheck %s
; RUN: %llvm2ice --verbose none %s | FileCheck --check-prefix=ERRORS %s

define i32 @func_single_arg(i32 %a) {
; CHECK: define i32 @func_single_arg
entry:
  ret i32 %a
; CHECK: ret i32 %a
}

define i32 @func_multiple_args(i32 %a, i32 %b, i32 %c) {
; CHECK: func_multiple_args
entry:
  ret i32 %c
; CHECK: ret i32 %c
}

; ERRORS-NOT: ICE translation error
