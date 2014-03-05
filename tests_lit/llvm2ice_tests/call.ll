; RUN: %llvm2ice -verbose inst %s | FileCheck %s
; RUN: %llvm2ice --verbose none %s | FileCheck --check-prefix=ERRORS %s

define i32 @fib(i32 %n) #0 {
; CHECK: define i32 fib
entry:
  %cmp = icmp slt i32 %n, 2
  br i1 %cmp, label %return, label %if.end

if.end:                                           ; preds = %entry
  %sub = add nsw i32 %n, -1
  %call = tail call i32 @fib(i32 %sub)
  %sub1 = add nsw i32 %n, -2
  %call2 = tail call i32 @fib(i32 %sub1)
  %add = add nsw i32 %call2, %call
  ret i32 %add

return:                                           ; preds = %entry
  ret i32 %n
}

define i32 @fact(i32 %n) #0 {
; CHECK: define i32 fact
entry:
  %cmp = icmp slt i32 %n, 2
  br i1 %cmp, label %return, label %if.end

if.end:                                           ; preds = %entry
  %sub = add nsw i32 %n, -1
  %call = tail call i32 @fact(i32 %sub)
  %mul = mul nsw i32 %call, %n
  ret i32 %mul

return:                                           ; preds = %entry
  ret i32 %n
}

define i32 @redirect(i32 %n) #1 {
; CHECK: define i32 redirect
entry:
  %call = tail call i32 @redirect_target(i32 %n) #3
  ret i32 %call
}

declare i32 @redirect_target(i32) #2

define void @call_void(i32 %n) #1 {
; CHECK: define void call_void
entry:
  %cmp2 = icmp sgt i32 %n, 0
  br i1 %cmp2, label %if.then, label %if.end

if.then:                                          ; preds = %entry, %if.then
  %n.tr3 = phi i32 [ %call.i, %if.then ], [ %n, %entry ]
  %sub = add nsw i32 %n.tr3, -1
  %call.i = tail call i32 @redirect_target(i32 %sub) #3
  %cmp = icmp sgt i32 %call.i, 0
  br i1 %cmp, label %if.then, label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

; ERRORS-NOT: ICE translation error
