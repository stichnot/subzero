; RUN: %llvm2ice --verbose none %s | FileCheck %s

; Function Attrs: nounwind readnone
define i32 @ignore64BitArg(i64 %a, i32 %b, i64 %c) #0 {
entry:
  ret i32 %b
}
; CHECK: ignore64BitArg:
; CHECK:      mov     eax, dword ptr [esp+12]
; CHECK-NEXT: ret

; Function Attrs: nounwind
define i32 @pass64BitArg(i64 %a, i64 %b, i64 %c, i64 %d, i64 %e, i64 %f) #1 {
entry:
  %call = tail call i32 @ignore64BitArgNoInline(i64 %a, i32 123, i64 %b) #4
  %call1 = tail call i32 @ignore64BitArgNoInline(i64 %c, i32 123, i64 %d) #4
  %call2 = tail call i32 @ignore64BitArgNoInline(i64 %e, i32 123, i64 %f) #4
  %add = add nsw i32 %call1, %call
  %add3 = add nsw i32 %add, %call2
  ret i32 %add3
}
; CHECK: pass64BitArg:
; CHECK:      push    123
; CHECK-NEXT: push
; CHECK-NEXT: push
; CHECK-NEXT: call    ignore64BitArgNoInline
; CHECK:      push
; CHECK-NEXT: push
; CHECK-NEXT: push    123
; CHECK-NEXT: push
; CHECK-NEXT: push
; CHECK-NEXT: call    ignore64BitArgNoInline
; CHECK:      push
; CHECK-NEXT: push
; CHECK-NEXT: push    123
; CHECK-NEXT: push
; CHECK-NEXT: push
; CHECK-NEXT: call    ignore64BitArgNoInline

declare i32 @ignore64BitArgNoInline(i64, i32, i64) #2

; Function Attrs: nounwind
define i32 @pass64BitConstArg(i64 %a, i64 %b) #1 {
entry:
  %call = tail call i32 @ignore64BitArgNoInline(i64 %a, i32 123, i64 -2401053092306725256) #4
  ret i32 %call
}
; CHECK: pass64BitConstArg:
; CHECK:      push    3735928559
; CHECK-NEXT: push    305419896
; CHECK-NEXT: push    123
; CHECK-NEXT: push    ecx
; CHECK-NEXT: push    eax
; CHECK-NEXT: call    ignore64BitArgNoInline

; Function Attrs: nounwind readnone
define i64 @return64BitArg(i64 %a) #0 {
entry:
  ret i64 %a
}
; CHECK: return64BitArg:
; CHECK: mov     {{.*}}, dword ptr [esp+4]
; CHECK: mov     {{.*}}, dword ptr [esp+8]
; CHECK: ret

; Function Attrs: nounwind readnone
define i64 @return64BitConst() #0 {
entry:
  ret i64 -2401053092306725256
}
; CHECK: return64BitConst:
; CHECK: mov     eax, 305419896
; CHECK: mov     edx, 3735928559
; CHECK: ret

; Function Attrs: nounwind readnone
define i64 @add64BitSigned(i64 %a, i64 %b) #0 {
entry:
  %add = add nsw i64 %b, %a
  ret i64 %add
}
; CHECK: add64BitSigned:
; CHECK: add
; CHECK: adc
; CHECK: ret

; Function Attrs: nounwind readnone
define i64 @add64BitUnsigned(i64 %a, i64 %b) #0 {
entry:
  %add = add i64 %b, %a
  ret i64 %add
}
; CHECK: add64BitUnsigned:
; CHECK: add
; CHECK: adc
; CHECK: ret

; Function Attrs: nounwind readnone
define i64 @sub64BitSigned(i64 %a, i64 %b) #0 {
entry:
  %sub = sub nsw i64 %a, %b
  ret i64 %sub
}
; CHECK: sub64BitSigned:
; CHECK: sub
; CHECK: sbb
; CHECK: ret

; Function Attrs: nounwind readnone
define i64 @sub64BitUnsigned(i64 %a, i64 %b) #0 {
entry:
  %sub = sub i64 %a, %b
  ret i64 %sub
}
; CHECK: sub64BitUnsigned:
; CHECK: sub
; CHECK: sbb
; CHECK: ret

; Function Attrs: nounwind readnone
define i64 @mul64BitSigned(i64 %a, i64 %b) #0 {
entry:
  %mul = mul nsw i64 %b, %a
  ret i64 %mul
}
; CHECK: mul64BitSigned:
; CHECK: imul
; CHECK: imul
; CHECK: mul
; CHECK: add
; CHECK: add
; CHECK: ret

; Function Attrs: nounwind readnone
define i64 @mul64BitUnsigned(i64 %a, i64 %b) #0 {
entry:
  %mul = mul i64 %b, %a
  ret i64 %mul
}
; CHECK: mul64BitUnsigned:
; CHECK: imul
; CHECK: imul
; CHECK: mul
; CHECK: add
; CHECK: add
; CHECK: ret

; Function Attrs: nounwind readnone
define i64 @div64BitSigned(i64 %a, i64 %b) #0 {
entry:
  %div = sdiv i64 %a, %b
  ret i64 %div
}
; CHECK: div64BitSigned:
; CHECK: call    __divdi3
; CHECK: ret

; Function Attrs: nounwind readnone
define i64 @div64BitUnsigned(i64 %a, i64 %b) #0 {
entry:
  %div = udiv i64 %a, %b
  ret i64 %div
}
; CHECK: div64BitUnsigned:
; CHECK: call    __udivdi3
; CHECK: ret

; Function Attrs: nounwind readnone
define i64 @rem64BitSigned(i64 %a, i64 %b) #0 {
entry:
  %rem = srem i64 %a, %b
  ret i64 %rem
}
; CHECK: rem64BitSigned:
; CHECK: call    __moddi3
; CHECK: ret

; Function Attrs: nounwind readnone
define i64 @rem64BitUnsigned(i64 %a, i64 %b) #0 {
entry:
  %rem = urem i64 %a, %b
  ret i64 %rem
}
; CHECK: rem64BitUnsigned:
; CHECK: call    __umoddi3
; CHECK: ret

; Function Attrs: nounwind readnone
define i64 @shl64BitSigned(i64 %a, i64 %b) #0 {
entry:
  %shl = shl i64 %a, %b
  ret i64 %shl
}
; CHECK: shl64BitSigned:
; CHECK: shld
; CHECK: shl e
; CHECK: test {{.*}}, 32
; CHECK: je

; Function Attrs: nounwind readnone
define i64 @shl64BitUnsigned(i64 %a, i64 %b) #0 {
entry:
  %shl = shl i64 %a, %b
  ret i64 %shl
}
; CHECK: shl64BitUnsigned:
; CHECK: shld
; CHECK: shl e
; CHECK: test {{.*}}, 32
; CHECK: je

; Function Attrs: nounwind readnone
define i64 @shr64BitSigned(i64 %a, i64 %b) #0 {
entry:
  %shr = ashr i64 %a, %b
  ret i64 %shr
}
; CHECK: shr64BitSigned:
; CHECK: shrd
; CHECK: sar
; CHECK: test {{.*}}, 32
; CHECK: je
; CHECK: sar {{.*}}, 31

; Function Attrs: nounwind readnone
define i64 @shr64BitUnsigned(i64 %a, i64 %b) #0 {
entry:
  %shr = lshr i64 %a, %b
  ret i64 %shr
}
; CHECK: shr64BitUnsigned:
; CHECK: shrd
; CHECK: shr
; CHECK: test {{.*}}, 32
; CHECK: je

; Function Attrs: nounwind readnone
define i64 @and64BitSigned(i64 %a, i64 %b) #0 {
entry:
  %and = and i64 %b, %a
  ret i64 %and
}
; CHECK: and64BitSigned:
; CHECK: and
; CHECK: and

; Function Attrs: nounwind readnone
define i64 @and64BitUnsigned(i64 %a, i64 %b) #0 {
entry:
  %and = and i64 %b, %a
  ret i64 %and
}
; CHECK: and64BitUnsigned:
; CHECK: and
; CHECK: and

; Function Attrs: nounwind readnone
define i64 @or64BitSigned(i64 %a, i64 %b) #0 {
entry:
  %or = or i64 %b, %a
  ret i64 %or
}
; CHECK: or64BitSigned:
; CHECK: or
; CHECK: or

; Function Attrs: nounwind readnone
define i64 @or64BitUnsigned(i64 %a, i64 %b) #0 {
entry:
  %or = or i64 %b, %a
  ret i64 %or
}
; CHECK: or64BitUnsigned:
; CHECK: or
; CHECK: or

; Function Attrs: nounwind readnone
define i64 @xor64BitSigned(i64 %a, i64 %b) #0 {
entry:
  %xor = xor i64 %b, %a
  ret i64 %xor
}
; CHECK: xor64BitSigned:
; CHECK: xor
; CHECK: xor

; Function Attrs: nounwind readnone
define i64 @xor64BitUnsigned(i64 %a, i64 %b) #0 {
entry:
  %xor = xor i64 %b, %a
  ret i64 %xor
}
; CHECK: xor64BitUnsigned:
; CHECK: xor
; CHECK: xor

; Function Attrs: nounwind readnone
define i32 @trunc64To32Signed(i64 %a) #0 {
entry:
  %conv = trunc i64 %a to i32
  ret i32 %conv
}
; CHECK: trunc64To32Signed:
; CHECK: mov     eax, dword ptr [esp+4]
; CHECK-NEXT: ret

; Function Attrs: nounwind readnone
define signext i16 @trunc64To16Signed(i64 %a) #0 {
entry:
  %conv = trunc i64 %a to i16
  ret i16 %conv
}
; CHECK: trunc64To16Signed:
; TODO: Should probably be movsx.  There's a problem with sext of args.
; CHECK: mov     eax, dword ptr [esp+4]
; CHECK-NEXT: ret

; Function Attrs: nounwind readnone
define signext i8 @trunc64To8Signed(i64 %a) #0 {
entry:
  %conv = trunc i64 %a to i8
  ret i8 %conv
}
; CHECK: trunc64To8Signed:
; TODO: Should probably be movsx.  There's a problem with sext of args.
; CHECK: mov     eax, dword ptr [esp+4]
; CHECK-NEXT: ret

; Function Attrs: nounwind readnone
define i32 @trunc64To32Unsigned(i64 %a) #0 {
entry:
  %conv = trunc i64 %a to i32
  ret i32 %conv
}
; CHECK: trunc64To32Unsigned:
; CHECK: mov     eax, dword ptr [esp+4]
; CHECK-NEXT: ret

; Function Attrs: nounwind readnone
define zeroext i16 @trunc64To16Unsigned(i64 %a) #0 {
entry:
  %conv = trunc i64 %a to i16
  ret i16 %conv
}
; CHECK: trunc64To16Unsigned:
; TODO: Should probably be movzx.  There's a problem with zext of args.
; CHECK: mov     eax, dword ptr [esp+4]
; CHECK-NEXT: ret

; Function Attrs: nounwind readnone
define zeroext i8 @trunc64To8Unsigned(i64 %a) #0 {
entry:
  %conv = trunc i64 %a to i8
  ret i8 %conv
}
; CHECK: trunc64To8Unsigned:
; TODO: Should probably be movzx.  There's a problem with zext of args.
; CHECK: mov     eax, dword ptr [esp+4]
; CHECK-NEXT: ret

; Function Attrs: nounwind readnone
define zeroext i1 @trunc64To1(i64 %a) #0 {
entry:
;  %tobool = icmp ne i64 %a, 0
  %tobool = trunc i64 %a to i1
  ret i1 %tobool
}
; CHECK: trunc64To1:
; TODO: Should probably include "and 1".  There's a problem with zext of args.
; CHECK: mov     eax, dword ptr [esp+4]
; CHECK-NEXT: ret

; Function Attrs: nounwind readnone
define i64 @sext32To64(i32 %a) #0 {
entry:
  %conv = sext i32 %a to i64
  ret i64 %conv
}
; CHECK: sext32To64:
; CHECK: mov
; CHECK: sar {{.*}}, 31

; Function Attrs: nounwind readnone
define i64 @sext16To64(i16 signext %a) #0 {
entry:
  %conv = sext i16 %a to i64
  ret i64 %conv
}
; CHECK: sext16To64:
; CHECK: movswl
; CHECK: sar {{.*}}, 31

; Function Attrs: nounwind readnone
define i64 @sext8To64(i8 signext %a) #0 {
entry:
  %conv = sext i8 %a to i64
  ret i64 %conv
}
; CHECK: sext8To64:
; CHECK: movsbl
; CHECK: sar {{.*}}, 31

; Function Attrs: nounwind readnone
define i64 @zext32To64(i32 %a) #0 {
entry:
  %conv = zext i32 %a to i64
  ret i64 %conv
}
; CHECK: zext32To64:
; CHECK: mov
; CHECK: mov {{.*}}, 0

; Function Attrs: nounwind readnone
define i64 @zext16To64(i16 zeroext %a) #0 {
entry:
  %conv = zext i16 %a to i64
  ret i64 %conv
}
; CHECK: zext16To64:
; CHECK: movzwl
; CHECK: mov {{.*}}, 0

; Function Attrs: nounwind readnone
define i64 @zext8To64(i8 zeroext %a) #0 {
entry:
  %conv = zext i8 %a to i64
  ret i64 %conv
}
; CHECK: zext8To64:
; CHECK: movzbl
; CHECK: mov {{.*}}, 0

; Function Attrs: nounwind readnone
define i64 @zext1To64(i1 zeroext %a) #0 {
entry:
  %conv = zext i1 %a to i64
  ret i64 %conv
}
; CHECK: zext1To64:
; TODO: probably needs an "and ..., 1"
; CHECK: movzbl
; CHECK: mov {{.*}}, 0

; Function Attrs: nounwind
define void @icmpEq64(i64 %a, i64 %b, i64 %c, i64 %d) #1 {
entry:
  %cmp = icmp eq i64 %a, %b
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  tail call void @func() #4
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %cmp1 = icmp eq i64 %c, %d
  br i1 %cmp1, label %if.then2, label %if.end3

if.then2:                                         ; preds = %if.end
  tail call void @func() #4
  br label %if.end3

if.end3:                                          ; preds = %if.then2, %if.end
  ret void
}
; CHECK: icmpEq64:
; CHECK: jne
; CHECK: jne
; CHECK: call
; CHECK: jne
; CHECK: jne
; CHECK: call

declare void @func() #2

; Function Attrs: nounwind
define void @icmpNe64(i64 %a, i64 %b, i64 %c, i64 %d) #1 {
entry:
  %cmp = icmp ne i64 %a, %b
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  tail call void @func() #4
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %cmp1 = icmp ne i64 %c, %d
  br i1 %cmp1, label %if.then2, label %if.end3

if.then2:                                         ; preds = %if.end
  tail call void @func() #4
  br label %if.end3

if.end3:                                          ; preds = %if.then2, %if.end
  ret void
}
; CHECK: icmpNe64:
; CHECK: je
; CHECK: je
; CHECK: call
; CHECK: je
; CHECK: je
; CHECK: call

; Function Attrs: nounwind
define void @icmpGt64(i64 %a, i64 %b, i64 %c, i64 %d) #1 {
entry:
  %cmp = icmp ugt i64 %a, %b
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  tail call void @func() #4
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %cmp1 = icmp sgt i64 %c, %d
  br i1 %cmp1, label %if.then2, label %if.end3

if.then2:                                         ; preds = %if.end
  tail call void @func() #4
  br label %if.end3

if.end3:                                          ; preds = %if.then2, %if.end
  ret void
}
; CHECK: icmpGt64:
; CHECK: jg
; CHECK: jl
; CHECK: jg
; CHECK: call
; CHECK: ja
; CHECK: jb
; CHECK: jg
; CHECK: call

; Function Attrs: nounwind
define void @icmpGe64(i64 %a, i64 %b, i64 %c, i64 %d) #1 {
entry:
  %cmp = icmp uge i64 %a, %b
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  tail call void @func() #4
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %cmp1 = icmp sge i64 %c, %d
  br i1 %cmp1, label %if.then2, label %if.end3

if.then2:                                         ; preds = %if.end
  tail call void @func() #4
  br label %if.end3

if.end3:                                          ; preds = %if.then2, %if.end
  ret void
}
; CHECK: icmpGe64:
; CHECK: jg
; CHECK: jl
; CHECK: jge
; CHECK: call
; CHECK: ja
; CHECK: jb
; CHECK: jge
; CHECK: call

; Function Attrs: nounwind
define void @icmpLt64(i64 %a, i64 %b, i64 %c, i64 %d) #1 {
entry:
  %cmp = icmp ult i64 %a, %b
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  tail call void @func() #4
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %cmp1 = icmp slt i64 %c, %d
  br i1 %cmp1, label %if.then2, label %if.end3

if.then2:                                         ; preds = %if.end
  tail call void @func() #4
  br label %if.end3

if.end3:                                          ; preds = %if.then2, %if.end
  ret void
}
; CHECK: icmpLt64:
; CHECK: jl
; CHECK: jg
; CHECK: jl
; CHECK: call
; CHECK: jb
; CHECK: ja
; CHECK: jl
; CHECK: call

; Function Attrs: nounwind
define void @icmpLe64(i64 %a, i64 %b, i64 %c, i64 %d) #1 {
entry:
  %cmp = icmp ule i64 %a, %b
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  tail call void @func() #4
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %cmp1 = icmp sle i64 %c, %d
  br i1 %cmp1, label %if.then2, label %if.end3

if.then2:                                         ; preds = %if.end
  tail call void @func() #4
  br label %if.end3

if.end3:                                          ; preds = %if.then2, %if.end
  ret void
}
; CHECK: icmpLe64:
; CHECK: jl
; CHECK: jg
; CHECK: jle
; CHECK: call
; CHECK: jb
; CHECK: ja
; CHECK: jle
; CHECK: call

; Function Attrs: nounwind readnone
define zeroext i1 @icmpEq64Bool(i64 %a, i64 %b) #0 {
entry:
  %cmp = icmp eq i64 %a, %b
  ret i1 %cmp
}
; CHECK: icmpEq64Bool:
; CHECK: jne
; CHECK: jne

; Function Attrs: nounwind readnone
define zeroext i1 @icmpNe64Bool(i64 %a, i64 %b) #0 {
entry:
  %cmp = icmp ne i64 %a, %b
  ret i1 %cmp
}
; CHECK: icmpNe64Bool:
; CHECK: je
; CHECK: je

; Function Attrs: nounwind readnone
define zeroext i1 @icmpSgt64Bool(i64 %a, i64 %b) #0 {
entry:
  %cmp = icmp sgt i64 %a, %b
  ret i1 %cmp
}
; CHECK: icmpSgt64Bool:
; CHECK: cmp
; CHECK: ja
; CHECK: jb
; CHECK: cmp
; CHECK: jg

; Function Attrs: nounwind readnone
define zeroext i1 @icmpUgt64Bool(i64 %a, i64 %b) #0 {
entry:
  %cmp = icmp ugt i64 %a, %b
  ret i1 %cmp
}
; CHECK: icmpUgt64Bool:
; CHECK: cmp
; CHECK: jg
; CHECK: jl
; CHECK: cmp
; CHECK: jg

; Function Attrs: nounwind readnone
define zeroext i1 @icmpSge64Bool(i64 %a, i64 %b) #0 {
entry:
  %cmp = icmp sge i64 %a, %b
  ret i1 %cmp
}
; CHECK: icmpSge64Bool:
; CHECK: cmp
; CHECK: ja
; CHECK: jb
; CHECK: cmp
; CHECK: jge

; Function Attrs: nounwind readnone
define zeroext i1 @icmpUge64Bool(i64 %a, i64 %b) #0 {
entry:
  %cmp = icmp uge i64 %a, %b
  ret i1 %cmp
}
; CHECK: icmpUge64Bool:
; CHECK: cmp
; CHECK: jg
; CHECK: jl
; CHECK: cmp
; CHECK: jge

; Function Attrs: nounwind readnone
define zeroext i1 @icmpSlt64Bool(i64 %a, i64 %b) #0 {
entry:
  %cmp = icmp slt i64 %a, %b
  ret i1 %cmp
}
; CHECK: icmpSlt64Bool:
; CHECK: cmp
; CHECK: jb
; CHECK: ja
; CHECK: cmp
; CHECK: jl

; Function Attrs: nounwind readnone
define zeroext i1 @icmpUlt64Bool(i64 %a, i64 %b) #0 {
entry:
  %cmp = icmp ult i64 %a, %b
  ret i1 %cmp
}
; CHECK: icmpUlt64Bool:
; CHECK: cmp
; CHECK: jl
; CHECK: jg
; CHECK: cmp
; CHECK: jl

; Function Attrs: nounwind readnone
define zeroext i1 @icmpSle64Bool(i64 %a, i64 %b) #0 {
entry:
  %cmp = icmp sle i64 %a, %b
  ret i1 %cmp
}
; CHECK: icmpSle64Bool:
; CHECK: cmp
; CHECK: jb
; CHECK: ja
; CHECK: cmp
; CHECK: jle

; Function Attrs: nounwind readnone
define zeroext i1 @icmpUle64Bool(i64 %a, i64 %b) #0 {
entry:
  %cmp = icmp ule i64 %a, %b
  ret i1 %cmp
}
; CHECK: icmpUle64Bool:
; CHECK: cmp
; CHECK: jl
; CHECK: jg
; CHECK: cmp
; CHECK: jle

; Function Attrs: nounwind readonly
define i64 @load64(i64* nocapture readonly %a) #3 {
entry:
  %0 = load i64* %a, align 8, !tbaa !1
  ret i64 %0
}
; CHECK: load64:
; TODO: "qword ptr" is incorrect.
; CHECK: mov e[[REGISTER:[a-z]+]], qword ptr [esp+4]
; CHECK-NEXT: mov {{.*}}, dword ptr [e[[REGISTER]]]
; CHECK-NEXT: mov {{.*}}, dword ptr [e[[REGISTER]]+4]

; Function Attrs: nounwind
define void @store64(i64* nocapture %a, i64 %value) #1 {
entry:
  store i64 %value, i64* %a, align 8, !tbaa !1
  ret void
}
; CHECK: store64:
; TODO: "qword ptr" is incorrect.
; CHECK: mov e[[REGISTER:[a-z]+]], qword ptr [esp+4]
; CHECK: mov dword ptr [e[[REGISTER]]+4],
; CHECK: mov dword ptr [e[[REGISTER]]],

; Function Attrs: nounwind
define void @store64Const(i64* nocapture %a) #1 {
entry:
  store i64 -2401053092306725256, i64* %a, align 8, !tbaa !1
  ret void
}
; CHECK: store64Const:
; TODO: "qword ptr" is incorrect.
; CHECK: mov e[[REGISTER:[a-z]+]], qword ptr [esp+4]
; CHECK: mov dword ptr [e[[REGISTER]]+4], 3735928559
; CHECK: mov dword ptr [e[[REGISTER]]], 305419896

; Function Attrs: nounwind readnone
define i64 @select64VarVar(i64 %a, i64 %b) #0 {
entry:
  %cmp = icmp ult i64 %a, %b
  %cond = select i1 %cmp, i64 %a, i64 %b
  ret i64 %cond
}
; CHECK: select64VarVar:
; CHECK: cmp
; CHECK: jl
; CHECK: jg
; CHECK: cmp
; CHECK: jl
; CHECK: cmp
; CHECK: jne

; Function Attrs: nounwind readnone
define i64 @select64VarConst(i64 %a, i64 %b) #0 {
entry:
  %cmp = icmp ult i64 %a, %b
  %cond = select i1 %cmp, i64 %a, i64 -2401053092306725256
  ret i64 %cond
}
; CHECK: select64VarConst:
; CHECK: cmp
; CHECK: jl
; CHECK: jg
; CHECK: cmp
; CHECK: jl
; CHECK: cmp
; CHECK: jne

; Function Attrs: nounwind readnone
define i64 @select64ConstVar(i64 %a, i64 %b) #0 {
entry:
  %cmp = icmp ult i64 %a, %b
  %cond = select i1 %cmp, i64 -2401053092306725256, i64 %b
  ret i64 %cond
}
; CHECK: select64ConstVar:
; CHECK: cmp
; CHECK: jl
; CHECK: jg
; CHECK: cmp
; CHECK: jl
; CHECK: cmp
; CHECK: jne

attributes #0 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readonly "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.5 (http://llvm.org/git/clang.git 5b153bbd63d067e9f6d93ad6caef18d9d2e64071) (http://llvm.org/git/llvm.git 268ab86f6c863468173276d920c16d1ec6de20b0)"}
!1 = metadata !{metadata !2, metadata !2, i64 0}
!2 = metadata !{metadata !"long long", metadata !3, i64 0}
!3 = metadata !{metadata !"omnipotent char", metadata !4, i64 0}
!4 = metadata !{metadata !"Simple C/C++ TBAA"}
