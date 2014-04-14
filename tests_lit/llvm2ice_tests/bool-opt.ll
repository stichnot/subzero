; RUN: %llvm2ice --verbose inst %s | FileCheck %s
; RUN: %llvm2ice --verbose none %s | FileCheck --check-prefix=ERRORS %s
; RUN: %szdiff --llvm2ice=%llvm2ice %s | FileCheck --check-prefix=DUMP %s

define void @testBool(i32 %a, i32 %b) {
entry:
  %cmp = icmp eq i32 %a, %b
  tail call void @use(i1 %cmp)
  ret void
}

declare void @use(i1 zeroext) #1

; CHECK-NOT: ICE translation error
; ERRORS-NOT: ICE translation error
; DUMP-NOT: SZ
