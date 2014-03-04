; RUN: %llvm2ice --verbose none %s | FileCheck %s
; RUN: %llvm2ice --verbose none %s | FileCheck --check-prefix=ERRORS %s

@__init_array_start = internal constant [0 x i8] zeroinitializer, align 4
@__fini_array_start = internal constant [0 x i8] zeroinitializer, align 4
@__tls_template_start = internal constant [0 x i8] zeroinitializer, align 8
@__tls_template_alignment = internal constant [4 x i8] c"\01\00\00\00", align 4

define internal i32 @doubleArgs(double %a, i32 %b, double %c) {
entry:
  ret i32 %b
}
; CHECK: doubleArgs:
; CHECK:      mov     eax, dword ptr [esp+12]
; CHECK-NEXT: ret

define internal i32 @floatArgs(float %a, i32 %b, float %c) {
entry:
  ret i32 %b
}
; CHECK: floatArgs:
; CHECK:      mov     eax, dword ptr [esp+8]
; CHECK-NEXT: ret

define internal i32 @passFpArgs(float %a, double %b, float %c, double %d, float %e, double %f) {
entry:
  %call = call i32 @ignoreFpArgsNoInline(float %a, i32 123, double %b)
  %call1 = call i32 @ignoreFpArgsNoInline(float %c, i32 123, double %d)
  %call2 = call i32 @ignoreFpArgsNoInline(float %e, i32 123, double %f)
  %add = add i32 %call1, %call
  %add3 = add i32 %add, %call2
  ret i32 %add3
}
; CHECK: passFpArgs:

declare i32 @ignoreFpArgsNoInline(float, i32, double)

define internal i32 @passFpConstArg(float %a, double %b) {
entry:
  %call = call i32 @ignoreFpArgsNoInline(float %a, i32 123, double 2.340000e+00)
  ret i32 %call
}
; CHECK: passFpConstArg:

define internal float @returnFloatArg(float %a) {
entry:
  ret float %a
}
; CHECK: returnFloatArg:

define internal double @returnDoubleArg(double %a) {
entry:
  ret double %a
}
; CHECK: returnDoubleArg:

define internal float @returnFloatConst() {
entry:
  ret float 0x3FF3AE1480000000
}
; CHECK: returnFloatConst:

define internal double @returnDoubleConst() {
entry:
  ret double 1.230000e+00
}
; CHECK: returnDoubleConst:

define internal float @addFloat(float %a, float %b) {
entry:
  %add = fadd float %a, %b
  ret float %add
}
; CHECK: addFloat:

define internal double @addDouble(double %a, double %b) {
entry:
  %add = fadd double %a, %b
  ret double %add
}
; CHECK: addDouble:

define internal float @subFloat(float %a, float %b) {
entry:
  %sub = fsub float %a, %b
  ret float %sub
}
; CHECK: subFloat:

define internal double @subDouble(double %a, double %b) {
entry:
  %sub = fsub double %a, %b
  ret double %sub
}
; CHECK: subDouble:

define internal float @mulFloat(float %a, float %b) {
entry:
  %mul = fmul float %a, %b
  ret float %mul
}
; CHECK: mulFloat:

define internal double @mulDouble(double %a, double %b) {
entry:
  %mul = fmul double %a, %b
  ret double %mul
}
; CHECK: mulDouble:

define internal float @divFloat(float %a, float %b) {
entry:
  %div = fdiv float %a, %b
  ret float %div
}
; CHECK: divFloat:

define internal double @divDouble(double %a, double %b) {
entry:
  %div = fdiv double %a, %b
  ret double %div
}
; CHECK: divDouble:

define internal float @remFloat(float %a, float %b) {
entry:
  %div = frem float %a, %b
  ret float %div
}
; CHECK: remFloat:

define internal double @remDouble(double %a, double %b) {
entry:
  %div = frem double %a, %b
  ret double %div
}
; CHECK: remDouble:

define internal float @fptrunc(double %a) {
entry:
  %conv = fptrunc double %a to float
  ret float %conv
}
; CHECK: fptrunc:

define internal double @fpext(float %a) {
entry:
  %conv = fpext float %a to double
  ret double %conv
}
; CHECK: fpext:

define internal i64 @doubleToSigned64(double %a) {
entry:
  %conv = fptosi double %a to i64
  ret i64 %conv
}
; CHECK: doubleToSigned64:

define internal i64 @floatToSigned64(float %a) {
entry:
  %conv = fptosi float %a to i64
  ret i64 %conv
}
; CHECK: floatToSigned64:

define internal i64 @doubleToUnsigned64(double %a) {
entry:
  %conv = fptoui double %a to i64
  ret i64 %conv
}
; CHECK: doubleToUnsigned64:

define internal i64 @floatToUnsigned64(float %a) {
entry:
  %conv = fptoui float %a to i64
  ret i64 %conv
}
; CHECK: floatToUnsigned64:

define internal i32 @doubleToSigned32(double %a) {
entry:
  %conv = fptosi double %a to i32
  ret i32 %conv
}
; CHECK: doubleToSigned32:

define internal i32 @floatToSigned32(float %a) {
entry:
  %conv = fptosi float %a to i32
  ret i32 %conv
}
; CHECK: floatToSigned32:

define internal i32 @doubleToUnsigned32(double %a) {
entry:
  %conv = fptoui double %a to i32
  ret i32 %conv
}
; CHECK: doubleToUnsigned32:

define internal i32 @floatToUnsigned32(float %a) {
entry:
  %conv = fptoui float %a to i32
  ret i32 %conv
}
; CHECK: floatToUnsigned32:

define internal i32 @doubleToSigned16(double %a) {
entry:
  %conv = fptosi double %a to i16
  %conv.ret_ext = sext i16 %conv to i32
  ret i32 %conv.ret_ext
}
; CHECK: doubleToSigned16:

define internal i32 @floatToSigned16(float %a) {
entry:
  %conv = fptosi float %a to i16
  %conv.ret_ext = sext i16 %conv to i32
  ret i32 %conv.ret_ext
}
; CHECK: floatToSigned16:

define internal i32 @doubleToUnsigned16(double %a) {
entry:
  %conv = fptoui double %a to i16
  %conv.ret_ext = zext i16 %conv to i32
  ret i32 %conv.ret_ext
}
; CHECK: doubleToUnsigned16:

define internal i32 @floatToUnsigned16(float %a) {
entry:
  %conv = fptoui float %a to i16
  %conv.ret_ext = zext i16 %conv to i32
  ret i32 %conv.ret_ext
}
; CHECK: floatToUnsigned16:

define internal i32 @doubleToSigned8(double %a) {
entry:
  %conv = fptosi double %a to i8
  %conv.ret_ext = sext i8 %conv to i32
  ret i32 %conv.ret_ext
}
; CHECK: doubleToSigned8:

define internal i32 @floatToSigned8(float %a) {
entry:
  %conv = fptosi float %a to i8
  %conv.ret_ext = sext i8 %conv to i32
  ret i32 %conv.ret_ext
}
; CHECK: floatToSigned8:

define internal i32 @doubleToUnsigned8(double %a) {
entry:
  %conv = fptoui double %a to i8
  %conv.ret_ext = zext i8 %conv to i32
  ret i32 %conv.ret_ext
}
; CHECK: doubleToUnsigned8:

define internal i32 @floatToUnsigned8(float %a) {
entry:
  %conv = fptoui float %a to i8
  %conv.ret_ext = zext i8 %conv to i32
  ret i32 %conv.ret_ext
}
; CHECK: floatToUnsigned8:

define internal i32 @doubleToUnsigned1(double %a) {
entry:
  %tobool = fptoui double %a to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}
; CHECK: doubleToUnsigned1:

define internal i32 @floatToUnsigned1(float %a) {
entry:
  %tobool = fptoui float %a to i1
  %tobool.ret_ext = zext i1 %tobool to i32
  ret i32 %tobool.ret_ext
}
; CHECK: floatToUnsigned1:

define internal double @signed64ToDouble(i64 %a) {
entry:
  %conv = sitofp i64 %a to double
  ret double %conv
}
; CHECK: signed64ToDouble:

define internal float @signed64ToFloat(i64 %a) {
entry:
  %conv = sitofp i64 %a to float
  ret float %conv
}
; CHECK: signed64ToFloat:

define internal double @unsigned64ToDouble(i64 %a) {
entry:
  %conv = uitofp i64 %a to double
  ret double %conv
}
; CHECK: unsigned64ToDouble:

define internal float @unsigned64ToFloat(i64 %a) {
entry:
  %conv = uitofp i64 %a to float
  ret float %conv
}
; CHECK: unsigned64ToFloat:

define internal double @signed32ToDouble(i32 %a) {
entry:
  %conv = sitofp i32 %a to double
  ret double %conv
}
; CHECK: signed32ToDouble:

define internal float @signed32ToFloat(i32 %a) {
entry:
  %conv = sitofp i32 %a to float
  ret float %conv
}
; CHECK: signed32ToFloat:

define internal double @unsigned32ToDouble(i32 %a) {
entry:
  %conv = uitofp i32 %a to double
  ret double %conv
}
; CHECK: unsigned32ToDouble:

define internal float @unsigned32ToFloat(i32 %a) {
entry:
  %conv = uitofp i32 %a to float
  ret float %conv
}
; CHECK: unsigned32ToFloat:

define internal double @signed16ToDouble(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i16
  %conv = sitofp i16 %a.arg_trunc to double
  ret double %conv
}
; CHECK: signed16ToDouble:

define internal float @signed16ToFloat(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i16
  %conv = sitofp i16 %a.arg_trunc to float
  ret float %conv
}
; CHECK: signed16ToFloat:

define internal double @unsigned16ToDouble(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i16
  %conv = uitofp i16 %a.arg_trunc to double
  ret double %conv
}
; CHECK: unsigned16ToDouble:

define internal float @unsigned16ToFloat(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i16
  %conv = uitofp i16 %a.arg_trunc to float
  ret float %conv
}
; CHECK: unsigned16ToFloat:

define internal double @signed8ToDouble(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i8
  %conv = sitofp i8 %a.arg_trunc to double
  ret double %conv
}
; CHECK: signed8ToDouble:

define internal float @signed8ToFloat(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i8
  %conv = sitofp i8 %a.arg_trunc to float
  ret float %conv
}
; CHECK: signed8ToFloat:

define internal double @unsigned8ToDouble(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i8
  %conv = uitofp i8 %a.arg_trunc to double
  ret double %conv
}
; CHECK: unsigned8ToDouble:

define internal float @unsigned8ToFloat(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i8
  %conv = uitofp i8 %a.arg_trunc to float
  ret float %conv
}
; CHECK: unsigned8ToFloat:

define internal double @unsigned1ToDouble(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i1
  %conv = uitofp i1 %a.arg_trunc to double
  ret double %conv
}
; CHECK: unsigned1ToDouble:

define internal float @unsigned1ToFloat(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i1
  %conv = uitofp i1 %a.arg_trunc to float
  ret float %conv
}
; CHECK: unsigned1ToFloat:

define internal void @fcmpEq(float %a, float %b, double %c, double %d) {
entry:
  %cmp = fcmp oeq float %a, %b
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  call void @func()
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %cmp1 = fcmp oeq double %c, %d
  br i1 %cmp1, label %if.then2, label %if.end3

if.then2:                                         ; preds = %if.end
  call void @func()
  br label %if.end3

if.end3:                                          ; preds = %if.then2, %if.end
  ret void
}
; CHECK: fcmpEq:

declare void @func()

define internal void @fcmpNe(float %a, float %b, double %c, double %d) {
entry:
  %cmp = fcmp une float %a, %b
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  call void @func()
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %cmp1 = fcmp une double %c, %d
  br i1 %cmp1, label %if.then2, label %if.end3

if.then2:                                         ; preds = %if.end
  call void @func()
  br label %if.end3

if.end3:                                          ; preds = %if.then2, %if.end
  ret void
}
; CHECK: fcmpNe:

define internal void @fcmpGt(float %a, float %b, double %c, double %d) {
entry:
  %cmp = fcmp ogt float %a, %b
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  call void @func()
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %cmp1 = fcmp ogt double %c, %d
  br i1 %cmp1, label %if.then2, label %if.end3

if.then2:                                         ; preds = %if.end
  call void @func()
  br label %if.end3

if.end3:                                          ; preds = %if.then2, %if.end
  ret void
}
; CHECK: fcmpGt:

define internal void @fcmpGe(float %a, float %b, double %c, double %d) {
entry:
  %cmp = fcmp ult float %a, %b
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  call void @func()
  br label %if.end

if.end:                                           ; preds = %entry, %if.then
  %cmp1 = fcmp ult double %c, %d
  br i1 %cmp1, label %if.end3, label %if.then2

if.then2:                                         ; preds = %if.end
  call void @func()
  br label %if.end3

if.end3:                                          ; preds = %if.end, %if.then2
  ret void
}
; CHECK: fcmpGe:

define internal void @fcmpLt(float %a, float %b, double %c, double %d) {
entry:
  %cmp = fcmp olt float %a, %b
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  call void @func()
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %cmp1 = fcmp olt double %c, %d
  br i1 %cmp1, label %if.then2, label %if.end3

if.then2:                                         ; preds = %if.end
  call void @func()
  br label %if.end3

if.end3:                                          ; preds = %if.then2, %if.end
  ret void
}
; CHECK: fcmpLt:

define internal void @fcmpLe(float %a, float %b, double %c, double %d) {
entry:
  %cmp = fcmp ugt float %a, %b
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  call void @func()
  br label %if.end

if.end:                                           ; preds = %entry, %if.then
  %cmp1 = fcmp ugt double %c, %d
  br i1 %cmp1, label %if.end3, label %if.then2

if.then2:                                         ; preds = %if.end
  call void @func()
  br label %if.end3

if.end3:                                          ; preds = %if.end, %if.then2
  ret void
}
; CHECK: fcmpLe:

define internal i32 @fcmpFalseFloat(float %a, float %b) {
entry:
  %cmp = fcmp false float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpFalseFloat:

define internal i32 @fcmpFalseDouble(double %a, double %b) {
entry:
  %cmp = fcmp false double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpFalseDouble:

define internal i32 @fcmpOeqFloat(float %a, float %b) {
entry:
  %cmp = fcmp oeq float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOeqFloat:

define internal i32 @fcmpOeqDouble(double %a, double %b) {
entry:
  %cmp = fcmp oeq double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOeqDouble:

define internal i32 @fcmpOgtFloat(float %a, float %b) {
entry:
  %cmp = fcmp ogt float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOgtFloat:

define internal i32 @fcmpOgtDouble(double %a, double %b) {
entry:
  %cmp = fcmp ogt double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOgtDouble:

define internal i32 @fcmpOgeFloat(float %a, float %b) {
entry:
  %cmp = fcmp oge float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOgeFloat:

define internal i32 @fcmpOgeDouble(double %a, double %b) {
entry:
  %cmp = fcmp oge double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOgeDouble:

define internal i32 @fcmpOltFloat(float %a, float %b) {
entry:
  %cmp = fcmp olt float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOltFloat:

define internal i32 @fcmpOltDouble(double %a, double %b) {
entry:
  %cmp = fcmp olt double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOltDouble:

define internal i32 @fcmpOleFloat(float %a, float %b) {
entry:
  %cmp = fcmp ole float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOleFloat:

define internal i32 @fcmpOleDouble(double %a, double %b) {
entry:
  %cmp = fcmp ole double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOleDouble:

define internal i32 @fcmpOneFloat(float %a, float %b) {
entry:
  %cmp = fcmp one float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOneFloat:

define internal i32 @fcmpOneDouble(double %a, double %b) {
entry:
  %cmp = fcmp one double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOneDouble:

define internal i32 @fcmpOrdFloat(float %a, float %b) {
entry:
  %cmp = fcmp ord float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOrdFloat:

define internal i32 @fcmpOrdDouble(double %a, double %b) {
entry:
  %cmp = fcmp ord double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpOrdDouble:

define internal i32 @fcmpUeqFloat(float %a, float %b) {
entry:
  %cmp = fcmp ueq float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUeqFloat:

define internal i32 @fcmpUeqDouble(double %a, double %b) {
entry:
  %cmp = fcmp ueq double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUeqDouble:

define internal i32 @fcmpUgtFloat(float %a, float %b) {
entry:
  %cmp = fcmp ugt float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUgtFloat:

define internal i32 @fcmpUgtDouble(double %a, double %b) {
entry:
  %cmp = fcmp ugt double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUgtDouble:

define internal i32 @fcmpUgeFloat(float %a, float %b) {
entry:
  %cmp = fcmp uge float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUgeFloat:

define internal i32 @fcmpUgeDouble(double %a, double %b) {
entry:
  %cmp = fcmp uge double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUgeDouble:

define internal i32 @fcmpUltFloat(float %a, float %b) {
entry:
  %cmp = fcmp ult float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUltFloat:

define internal i32 @fcmpUltDouble(double %a, double %b) {
entry:
  %cmp = fcmp ult double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUltDouble:

define internal i32 @fcmpUleFloat(float %a, float %b) {
entry:
  %cmp = fcmp ule float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUleFloat:

define internal i32 @fcmpUleDouble(double %a, double %b) {
entry:
  %cmp = fcmp ule double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUleDouble:

define internal i32 @fcmpUneFloat(float %a, float %b) {
entry:
  %cmp = fcmp une float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUneFloat:

define internal i32 @fcmpUneDouble(double %a, double %b) {
entry:
  %cmp = fcmp une double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUneDouble:

define internal i32 @fcmpUnoFloat(float %a, float %b) {
entry:
  %cmp = fcmp uno float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUnoFloat:

define internal i32 @fcmpUnoDouble(double %a, double %b) {
entry:
  %cmp = fcmp uno double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpUnoDouble:

define internal i32 @fcmpTrueFloat(float %a, float %b) {
entry:
  %cmp = fcmp true float %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpTrueFloat:

define internal i32 @fcmpTrueDouble(double %a, double %b) {
entry:
  %cmp = fcmp true double %a, %b
  %cmp.ret_ext = zext i1 %cmp to i32
  ret i32 %cmp.ret_ext
}
; CHECK: fcmpTrueDouble:

define internal float @loadFloat(i32 %a) {
entry:
  %a.asptr = inttoptr i32 %a to float*
  %0 = load float* %a.asptr, align 4
  ret float %0
}
; CHECK: loadFloat:

define internal double @loadDouble(i32 %a) {
entry:
  %a.asptr = inttoptr i32 %a to double*
  %0 = load double* %a.asptr, align 8
  ret double %0
}
; CHECK: loadDouble:

define internal void @storeFloat(i32 %a, float %value) {
entry:
  %a.asptr = inttoptr i32 %a to float*
  store float %value, float* %a.asptr, align 4
  ret void
}
; CHECK: storeFloat:

define internal void @storeDouble(i32 %a, double %value) {
entry:
  %a.asptr = inttoptr i32 %a to double*
  store double %value, double* %a.asptr, align 8
  ret void
}
; CHECK: storeDouble:

define internal void @storeFloatConst(i32 %a) {
entry:
  %a.asptr = inttoptr i32 %a to float*
  store float 0x3FF3AE1480000000, float* %a.asptr, align 4
  ret void
}
; CHECK: storeFloatConst:

define internal void @storeDoubleConst(i32 %a) {
entry:
  %a.asptr = inttoptr i32 %a to double*
  store double 1.230000e+00, double* %a.asptr, align 8
  ret void
}
; CHECK: storeDoubleConst:

define internal float @selectFloatVarVar(float %a, float %b) {
entry:
  %cmp = fcmp olt float %a, %b
  %cond = select i1 %cmp, float %a, float %b
  ret float %cond
}
; CHECK: selectFloatVarVar:

define internal double @selectDoubleVarVar(double %a, double %b) {
entry:
  %cmp = fcmp olt double %a, %b
  %cond = select i1 %cmp, double %a, double %b
  ret double %cond
}
; CHECK: selectDoubleVarVar:

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
