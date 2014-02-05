; RUN: %llvm2ice -verbose none %s | FileCheck %s

define i32 @Add(i32 %a, i32 %b) #0 {
; CHECK: define internal i32 Add
entry:
  %add = add nsw i32 %b, %a
; CHECK: add.i32
  tail call void @Use(i32 %add) #2
; CHECK: call Use
  ret i32 %add
}

declare void @Use(i32) #1

define i32 @And(i32 %a, i32 %b) #0 {
; CHECK: define internal i32 And
entry:
  %and = and i32 %b, %a
; CHECK: and.i32
  tail call void @Use(i32 %and) #2
; CHECK: call Use
  ret i32 %and
}

define i32 @Or(i32 %a, i32 %b) #0 {
; CHECK: define internal i32 Or
entry:
  %or = or i32 %b, %a
; CHECK: or.i32
  tail call void @Use(i32 %or) #2
; CHECK: call Use
  ret i32 %or
}

define i32 @Xor(i32 %a, i32 %b) #0 {
; CHECK: define internal i32 Xor
entry:
  %xor = xor i32 %b, %a
; CHECK: xor.i32
  tail call void @Use(i32 %xor) #2
; CHECK: call Use
  ret i32 %xor
}

define i32 @Sub(i32 %a, i32 %b) #0 {
; CHECK: define internal i32 Sub
entry:
  %sub = sub nsw i32 %a, %b
; CHECK: sub.i32
  tail call void @Use(i32 %sub) #2
; CHECK: call Use
  ret i32 %sub
}

define i32 @Mul(i32 %a, i32 %b) #0 {
; CHECK: define internal i32 Mul
entry:
  %mul = mul nsw i32 %b, %a
; CHECK: imul.i32
  tail call void @Use(i32 %mul) #2
; CHECK: call Use
  ret i32 %mul
}

define i32 @Sdiv(i32 %a, i32 %b) #0 {
; CHECK: define internal i32 Sdiv
entry:
  %div = sdiv i32 %a, %b
; CHECK: cdq.i32
; CHECK: idiv.i32
  tail call void @Use(i32 %div) #2
; CHECK: call Use
  ret i32 %div
}

define i32 @Srem(i32 %a, i32 %b) #0 {
; CHECK: define internal i32 Srem
entry:
  %rem = srem i32 %a, %b
; CHECK: cdq.i32
; CHECK: idiv.i32
  tail call void @Use(i32 %rem) #2
; CHECK: call Use
  ret i32 %rem
}

define i32 @Udiv(i32 %a, i32 %b) #0 {
; CHECK: define internal i32 Udiv
entry:
  %div = udiv i32 %a, %b
; CHECK: div.i32
  tail call void @Use(i32 %div) #2
; CHECK: call Use
  ret i32 %div
}

define i32 @Urem(i32 %a, i32 %b) #0 {
; CHECK: define internal i32 Urem
entry:
  %rem = urem i32 %a, %b
; CHECK: div.i32
  tail call void @Use(i32 %rem) #2
; CHECK: call Use
  ret i32 %rem
}

; CHECK-NOT: ICE translation error
