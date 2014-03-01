; RUN: %llvm2ice --verbose none %s | FileCheck %s
; RUN: %llvm2ice --verbose none %s | FileCheck --check-prefix=ERRORS %s

@__init_array_start = internal constant [0 x i8] zeroinitializer, align 4
@__fini_array_start = internal constant [0 x i8] zeroinitializer, align 4
@__tls_template_start = internal constant [0 x i8] zeroinitializer, align 8
@__tls_template_alignment = internal constant [4 x i8] c"\01\00\00\00", align 4

define internal void @CallIndirect(i32 %f) {
entry:
  %f.asptr = inttoptr i32 %f to void ()*
  call void %f.asptr()
  call void %f.asptr()
  call void %f.asptr()
  call void %f.asptr()
  call void %f.asptr()
  ret void
}
; CHECK: call [[REGISTER:[a-z]+]]
; CHECK: call [[REGISTER]]
; CHECK: call [[REGISTER]]
; CHECK: call [[REGISTER]]
; CHECK: call [[REGISTER]]

define internal i32 @nacl_tp_tdb_offset(i32) {
entry:
  ret i32 0
}

define internal i32 @nacl_tp_tls_offset(i32 %size) {
entry:
  %result = sub i32 0, %size
  ret i32 %result
}

; ERRORS-NOT: ICE translation error
