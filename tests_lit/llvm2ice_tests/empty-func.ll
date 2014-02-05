; RUN: %llvm2ice %s | FileCheck %s

define void @foo() {
; CHECK: define internal void foo()
entry:
  ret void
; CHECK: entry
; CHECK-NEXT: ret void
}

; CHECK-NOT: ICE translation error
