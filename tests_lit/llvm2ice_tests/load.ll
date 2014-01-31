; RUN: %llvm2ice %s | FileCheck %s

define void @load_i64(i32 %addr_arg) {
entry:
  %ptr64 = inttoptr i32 %addr_arg to i64*
  %iv = load i64* %ptr64
  ret void

; CHECK:      %ptr64 = i32 %addr_arg
; CHECK-NEXT:  %iv = load i64* %ptr64, align 1
; CHECK-NEXT:  ret void
}

define void @load_i32(i32 %addr_arg) {
entry:
  %ptr32 = inttoptr i32 %addr_arg to i32*
  %iv = load i32* %ptr32
  ret void

; CHECK:       %ptr32 = i32 %addr_arg
; CHECK-NEXT:  %iv = load i32* %ptr32, align 1
; CHECK-NEXT:  ret void
}

define void @load_i16(i32 %addr_arg) {
entry:
  %ptr16 = inttoptr i32 %addr_arg to i16*
  %iv = load i16* %ptr16
  ret void

; CHECK:       %ptr16 = i32 %addr_arg
; CHECK-NEXT:  %iv = load i16* %ptr16, align 1
; CHECK-NEXT:  ret void
}

define void @load_i8(i32 %addr_arg) {
entry:
  %ptr8 = inttoptr i32 %addr_arg to i8*
  %iv = load i8* %ptr8
  ret void

; CHECK:       %ptr8 = i32 %addr_arg
; CHECK-NEXT:  %iv = load i8* %ptr8, align 1
; CHECK-NEXT:  ret void
}
