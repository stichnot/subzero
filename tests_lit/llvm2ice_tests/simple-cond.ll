; RUN: %llvm2ice %s | FileCheck %s

define internal i32 @simple_cond(i32 %a, i32 %n) {
entry:
  %cmp = icmp slt i32 %n, 0
; CHECK:  %cmp = icmp slt i32 %n, 0
  br i1 %cmp, label %if.then, label %if.else
; CHECK-NEXT:  br i1 %cmp, label %if.then, label %if.else

if.then:
  %sub = sub i32 1, %n
  br label %if.end

if.else:
  %gep_array = mul i32 %n, 4
  %gep = add i32 %a, %gep_array
  %gep.asptr = inttoptr i32 %gep to i32*
  %0 = load i32* %gep.asptr, align 1
  br label %if.end

if.end:
  %result.0 = phi i32 [ %sub, %if.then ], [ %0, %if.else ]
; CHECK: %result.0 = phi i32 [ %sub, %if.then ], [ %{{[0-9_]+}}, %if.else ]
  ret i32 %result.0
}
