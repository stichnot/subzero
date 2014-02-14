; RUN: %llvm2ice -verbose inst %s | FileCheck %s

; CHECK: ICE translation error
define void @testBool(i32 %a, i32 %b) #0 {
entry:
  %cmp = icmp eq i32 %a, %b
  tail call void @use(i1 zeroext %cmp) #2
  ret void
}

declare void @use(i1 zeroext) #1

; CHECK-NOT: ICE translation error
