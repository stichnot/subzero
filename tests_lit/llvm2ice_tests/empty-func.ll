; RUN: %llvm2ice -verbose inst %s | FileCheck %s
; RUN: %llvm2ice --verbose none %s | FileCheck --check-prefix=ERRORS %s

define void @foo() {
; CHECK: define void foo()
entry:
  ret void
; CHECK: entry
; CHECK-NEXT: ret void
}

; ERRORS-NOT: ICE translation error
