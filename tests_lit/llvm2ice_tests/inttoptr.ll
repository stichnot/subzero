; RUN: %llvm2ice -verbose inst %s | FileCheck %s
; RUN: %llvm2ice --verbose none %s | FileCheck --check-prefix=ERRORS %s

define void @dummy_inttoptr(i32 %addr_arg) {
entry:
  %ptr = inttoptr i32 %addr_arg to i32*
  ret void
; CHECK: %ptr = i32 %addr_arg
}

; ERRORS-NOT: ICE translation error
