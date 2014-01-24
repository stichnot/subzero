; RUN: %llvm2ice %s | FileCheck %s

define i64 @arithmetic_chain(i64 %foo, i64 %bar) {
entry:
  %r1 = add i64 %foo, %bar
  %r2 = add i64 %foo, %r1
  %r3 = mul i64 %bar, %r1
  %r4 = shl i64 %r3, %r2
  ret i64 %r4

; CHECK:      entry:
; CHECK-NEXT:  %r1 = add i64 %foo, %bar
; CHECK-NEXT:  %r2 = add i64 %foo, %r1
; CHECK-NEXT:  %r3 = mul i64 %bar, %r1
; CHECK-NEXT:  %r4 = shl i64 %r3, %r2
; CHECK-NEXT:  ret i64 %r4
}
