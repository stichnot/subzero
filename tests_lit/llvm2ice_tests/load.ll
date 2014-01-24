; RUN: %llvm2ice %s | FileCheck %s

define void @dummy_load(i32 %addr_arg) {
entry:
  %ptr = inttoptr i32 %addr_arg to i32*
  %iv = load i32* %ptr
  ret void

; CHECK:      %ptr = i32 %addr_arg
; CHECK-NEXT:  %iv = load i32* %ptr, align 1
; CHECK-NEXT:  ret void
}
