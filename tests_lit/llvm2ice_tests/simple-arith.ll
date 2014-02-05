; RUN: %llvm2ice %s | FileCheck %s

define i64 @add_args(i64 %arg1, i64 %arg2) {
; CHECK: define internal i64 add_args
entry:
  %add = add nsw i64 %arg2, %arg1
  ret i64 %add
; CHECK: %add = add i64 %arg2, %arg1
; CHECK-NEXT: ret i64 %add
}

; CHECK-NOT: ICE translation error
