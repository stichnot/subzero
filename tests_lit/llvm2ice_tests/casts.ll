; RUN: %llvm2ice -notranslate %s | FileCheck %s

define i64 @simple_zext(i32 %arg) {
entry:
  %c = zext i32 %arg to i64
  ret i64 %c

; CHECK:        entry:
; CHECK-NEXT:       %c = zext i32 %arg to i64
; CHECK-NEXT:       ret i64 %c
}
