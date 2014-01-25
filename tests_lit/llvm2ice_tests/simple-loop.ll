; RUN: %llvm2ice %s | FileCheck %s

define internal i32 @simple_loop(i32 %a, i32 %n) {
entry:
  %cmp4 = icmp sgt i32 %n, 0
  br i1 %cmp4, label %for.body, label %for.end
; CHECK:  br i1 %cmp4, label %for.body, label %for.end

for.body:
; CHECK-NEXT: for.body
  %i.06 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
; CHECK:  %i.06 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
  %sum.05 = phi i32 [ %add, %for.body ], [ 0, %entry ]
; CHECK-NEXT:  %sum.05 = phi i32 [ %add, %for.body ], [ 0, %entry ]
  %gep_array = mul i32 %i.06, 4
  %gep = add i32 %a, %gep_array
  %gep.asptr = inttoptr i32 %gep to i32*
  %0 = load i32* %gep.asptr, align 1
  %add = add i32 %0, %sum.05
  %inc = add i32 %i.06, 1
  %cmp = icmp slt i32 %inc, %n
  br i1 %cmp, label %for.body, label %for.end

for.end:
  %sum.0.lcssa = phi i32 [ 0, %entry ], [ %add, %for.body ]
  ret i32 %sum.0.lcssa
}
