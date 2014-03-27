define i32 @_Z13castUi64ToUi1y(i64 %a) {
entry:
;  %tobool = icmp ne i64 %a, 0
  %tobool = trunc i64 %a to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @_Z13castSi64ToUi1x(i64 %a) {
entry:
;  %tobool = icmp ne i64 %a, 0
  %tobool = trunc i64 %a to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @_Z13castUi32ToUi1j(i32 %a) {
entry:
;  %tobool = icmp ne i32 %a, 0
  %tobool = trunc i32 %a to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @_Z13castSi32ToUi1i(i32 %a) {
entry:
;  %tobool = icmp ne i32 %a, 0
  %tobool = trunc i32 %a to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @_Z13castUi16ToUi1t(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i16
;  %tobool = icmp ne i16 %a.arg_trunc, 0
  %tobool = trunc i16 %a.arg_trunc to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @_Z13castSi16ToUi1s(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i16
;  %tobool = icmp ne i16 %a.arg_trunc, 0
  %tobool = trunc i16 %a.arg_trunc to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @_Z12castUi8ToUi1h(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i8
;  %tobool = icmp ne i8 %a.arg_trunc, 0
  %tobool = trunc i8 %a.arg_trunc to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @_Z12castSi8ToUi1a(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i8
;  %tobool = icmp ne i8 %a.arg_trunc, 0
  %tobool = trunc i8 %a.arg_trunc to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @_Z12castUi1ToUi1b(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i1
  %a.arg_trunc.ret_ext = zext i1 %a.arg_trunc to i32
  ret i32 %a.arg_trunc.ret_ext
}

define i32 @_Z12castF64ToUi1d(double %a) {
entry:
;  %tobool = fcmp une double %a, 0.000000e+00
  %tobool = fptoui double %a to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @_Z12castF32ToUi1f(float %a) {
entry:
;  %tobool = fcmp une float %a, 0.000000e+00
  %tobool = fptoui float %a to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}
