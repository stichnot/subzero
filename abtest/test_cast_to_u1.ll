define i32 @castUi64ToUi1(i64 %a) {
entry:
;  %tobool = icmp ne i64 %a, 0
  %tobool = trunc i64 %a to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @castSi64ToUi1(i64 %a) {
entry:
;  %tobool = icmp ne i64 %a, 0
  %tobool = trunc i64 %a to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @castUi32ToUi1(i32 %a) {
entry:
;  %tobool = icmp ne i32 %a, 0
  %tobool = trunc i32 %a to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @castSi32ToUi1(i32 %a) {
entry:
;  %tobool = icmp ne i32 %a, 0
  %tobool = trunc i32 %a to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @castUi16ToUi1(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i16
;  %tobool = icmp ne i16 %a.arg_trunc, 0
  %tobool = trunc i16 %a.arg_trunc to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @castSi16ToUi1(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i16
;  %tobool = icmp ne i16 %a.arg_trunc, 0
  %tobool = trunc i16 %a.arg_trunc to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @castUi8ToUi1(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i8
;  %tobool = icmp ne i8 %a.arg_trunc, 0
  %tobool = trunc i8 %a.arg_trunc to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @castSi8ToUi1(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i8
;  %tobool = icmp ne i8 %a.arg_trunc, 0
  %tobool = trunc i8 %a.arg_trunc to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @castUi1ToUi1(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i1
  %a.arg_trunc.ret_ext = zext i1 %a.arg_trunc to i32
  ret i32 %a.arg_trunc.ret_ext
}

define i32 @castF64ToUi1(double %a) {
entry:
;  %tobool = fcmp une double %a, 0.000000e+00
  %tobool = fptoui double %a to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}

define i32 @castF32ToUi1(float %a) {
entry:
;  %tobool = fcmp une float %a, 0.000000e+00
  %tobool = fptoui float %a to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}
