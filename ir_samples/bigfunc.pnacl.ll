; ModuleID = '/tmp/bigfunc.ll'
target datalayout = "e-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-p:32:32:32-v128:32:32"
target triple = "le32-unknown-nacl"

@__init_array_start = internal constant [0 x i8] zeroinitializer, align 4
@__fini_array_start = internal constant [0 x i8] zeroinitializer, align 4
@__tls_template_start = internal constant [0 x i8] zeroinitializer, align 8
@__tls_template_alignment = internal constant [4 x i8] c"\01\00\00\00", align 4

define internal i32 @fix_mpy(i32 %a, i32 %b) {
entry:
  %b.arg_trunc = trunc i32 %b to i16
  %a.arg_trunc = trunc i32 %a to i16
  %conv = sext i16 %a.arg_trunc to i32
  %conv1 = sext i16 %b.arg_trunc to i32
  %mul = mul i32 %conv1, %conv
  %shr1 = lshr i32 %mul, 15
  %conv2 = trunc i32 %shr1 to i16
  %conv2.ret_ext = sext i16 %conv2 to i32
  ret i32 %conv2.ret_ext
}

define internal i32 @foo2(i32 %a) {
entry:
  %a.arg_trunc = trunc i32 %a to i16
  %mul = shl i16 %a.arg_trunc, 1
  %conv.i = sext i16 %a.arg_trunc to i32
  %conv1.i = sext i16 %mul to i32
  %mul.i = mul i32 %conv1.i, %conv.i
  %shr1.i = lshr i32 %mul.i, 15
  %conv2.i = trunc i32 %shr1.i to i16
  %mul4 = mul i16 %a.arg_trunc, 3
  %conv1.i4 = sext i16 %mul4 to i32
  %mul.i5 = mul i32 %conv1.i4, %conv.i
  %shr1.i6 = lshr i32 %mul.i5, 15
  %conv2.i7 = trunc i32 %shr1.i6 to i16
  %add = add i16 %conv2.i, %conv2.i7
  %add.ret_ext = sext i16 %add to i32
  ret i32 %add.ret_ext
}

define internal void @foo3(i32 %q, i32 %w, i32 %m) {
entry:
  %cmp20 = icmp sgt i32 %m, 0
  br i1 %cmp20, label %for.cond1.preheader, label %for.end19

for.cond1.preheader:                              ; preds = %entry, %for.inc17
  %storemerge21 = phi i32 [ %inc18, %for.inc17 ], [ 0, %entry ]
  %cmp217 = icmp sgt i32 %storemerge21, 0
  br i1 %cmp217, label %for.cond4.preheader.lr.ph, label %for.inc17

for.cond4.preheader.lr.ph:                        ; preds = %for.cond1.preheader
  %sub = add i32 %storemerge21, -2
  %gep_array = mul i32 %sub, 2
  %gep = add i32 %q, %gep_array
  %gep_array24 = mul i32 %storemerge21, 2
  %gep25 = add i32 %q, %gep_array24
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.cond4.preheader.lr.ph, %for.inc14
  %storemerge118 = phi i32 [ 0, %for.cond4.preheader.lr.ph ], [ %inc15, %for.inc14 ]
  %cmp515 = icmp sgt i32 %storemerge118, 0
  br i1 %cmp515, label %for.body6.lr.ph, label %for.inc14

for.body6.lr.ph:                                  ; preds = %for.cond4.preheader
  %gep_array27 = mul i32 %storemerge118, 2
  %gep28 = add i32 %w, %gep_array27
  br label %for.body6

for.body6:                                        ; preds = %for.body6.lr.ph, %for.body6
  %storemerge216 = phi i32 [ 0, %for.body6.lr.ph ], [ %add12, %for.body6 ]
  %gep28.asptr = inttoptr i32 %gep28 to i16*
  %0 = load i16* %gep28.asptr, align 1
  %conv = sext i16 %0 to i32
  %mul = mul i32 %conv, %m
  %gep.asptr = inttoptr i32 %gep to i16*
  %1 = load i16* %gep.asptr, align 1
  %conv83 = zext i16 %1 to i32
  %add = add i32 %mul, %conv83
  %conv9 = trunc i32 %add to i16
  %gep25.asptr = inttoptr i32 %gep25 to i16*
  store i16 %conv9, i16* %gep25.asptr, align 1
  %gep28.asptr1 = inttoptr i32 %gep28 to i16*
  %2 = load i16* %gep28.asptr1, align 1
  %add12 = add i32 %storemerge216, 1
  %gep_array30 = mul i32 %add12, 2
  %gep31 = add i32 %q, %gep_array30
  %gep31.asptr = inttoptr i32 %gep31 to i16*
  store i16 %2, i16* %gep31.asptr, align 1
  %cmp5 = icmp slt i32 %add12, %storemerge118
  br i1 %cmp5, label %for.body6, label %for.inc14

for.inc14:                                        ; preds = %for.body6, %for.cond4.preheader
  %inc15 = add i32 %storemerge118, 1
  %cmp2 = icmp slt i32 %inc15, %storemerge21
  br i1 %cmp2, label %for.cond4.preheader, label %for.inc17

for.inc17:                                        ; preds = %for.inc14, %for.cond1.preheader
  %inc18 = add i32 %storemerge21, 1
  %cmp = icmp slt i32 %inc18, %m
  br i1 %cmp, label %for.cond1.preheader, label %for.end19

for.end19:                                        ; preds = %for.inc17, %entry
  ret void
}

define internal i32 @fix_fft(i32 %fr, i32 %fi, i32 %Sinewave, i32 %m, i32 %inverse) {
entry:
  %shl = shl i32 1, %m
  %cmp = icmp sgt i32 %shl, 1024
  br i1 %cmp, label %return, label %if.end

if.end:                                           ; preds = %entry
  %sub = add i32 %shl, -1
  %cmp20.i = icmp sgt i32 %m, 0
  br i1 %cmp20.i, label %for.cond1.preheader.i, label %for.cond.preheader

for.cond1.preheader.i:                            ; preds = %if.end, %for.inc17.i
  %storemerge21.i = phi i32 [ %inc18.i, %for.inc17.i ], [ 0, %if.end ]
  %cmp217.i = icmp sgt i32 %storemerge21.i, 0
  br i1 %cmp217.i, label %for.cond4.preheader.lr.ph.i, label %for.inc17.i

for.cond4.preheader.lr.ph.i:                      ; preds = %for.cond1.preheader.i
  %sub.i = add i32 %storemerge21.i, -2
  %gep_array = mul i32 %sub.i, 2
  %gep = add i32 %fr, %gep_array
  %gep_array4519 = mul i32 %storemerge21.i, 2
  %gep4520 = add i32 %fr, %gep_array4519
  br label %for.cond4.preheader.i

for.cond4.preheader.i:                            ; preds = %for.inc14.i, %for.cond4.preheader.lr.ph.i
  %storemerge118.i = phi i32 [ 0, %for.cond4.preheader.lr.ph.i ], [ %inc15.i, %for.inc14.i ]
  %cmp515.i = icmp sgt i32 %storemerge118.i, 0
  br i1 %cmp515.i, label %for.body6.lr.ph.i, label %for.inc14.i

for.body6.lr.ph.i:                                ; preds = %for.cond4.preheader.i
  %gep_array4522 = mul i32 %storemerge118.i, 2
  %gep4523 = add i32 %fi, %gep_array4522
  br label %for.body6.i

for.body6.i:                                      ; preds = %for.body6.i, %for.body6.lr.ph.i
  %storemerge216.i = phi i32 [ 0, %for.body6.lr.ph.i ], [ %add12.i, %for.body6.i ]
  %gep4523.asptr = inttoptr i32 %gep4523 to i16*
  %0 = load i16* %gep4523.asptr, align 1
  %conv.i = sext i16 %0 to i32
  %mul.i = mul i32 %conv.i, %m
  %gep.asptr = inttoptr i32 %gep to i16*
  %1 = load i16* %gep.asptr, align 1
  %conv83.i = zext i16 %1 to i32
  %add.i = add i32 %mul.i, %conv83.i
  %conv9.i = trunc i32 %add.i to i16
  %gep4520.asptr = inttoptr i32 %gep4520 to i16*
  store i16 %conv9.i, i16* %gep4520.asptr, align 1
  %gep4523.asptr1 = inttoptr i32 %gep4523 to i16*
  %2 = load i16* %gep4523.asptr1, align 1
  %add12.i = add i32 %storemerge216.i, 1
  %gep_array4525 = mul i32 %add12.i, 2
  %gep4526 = add i32 %fr, %gep_array4525
  %gep4526.asptr = inttoptr i32 %gep4526 to i16*
  store i16 %2, i16* %gep4526.asptr, align 1
  %cmp5.i = icmp slt i32 %add12.i, %storemerge118.i
  br i1 %cmp5.i, label %for.body6.i, label %for.inc14.i

for.inc14.i:                                      ; preds = %for.body6.i, %for.cond4.preheader.i
  %inc15.i = add i32 %storemerge118.i, 1
  %cmp2.i = icmp slt i32 %inc15.i, %storemerge21.i
  br i1 %cmp2.i, label %for.cond4.preheader.i, label %for.inc17.i

for.inc17.i:                                      ; preds = %for.inc14.i, %for.cond1.preheader.i
  %inc18.i = add i32 %storemerge21.i, 1
  %cmp.i = icmp slt i32 %inc18.i, %m
  br i1 %cmp.i, label %for.cond1.preheader.i, label %for.cond.preheader

for.cond.preheader:                               ; preds = %for.inc17.i, %if.end
  %cmp14511 = icmp slt i32 %shl, 2
  br i1 %cmp14511, label %for.cond1.preheader.i3973.preheader, label %do.body.preheader.lr.ph

do.body.preheader.lr.ph:                          ; preds = %for.cond.preheader
  %3 = icmp sgt i32 %sub, 1
  br label %do.body.preheader

do.body.preheader:                                ; preds = %for.inc, %do.body.preheader.lr.ph
  %storemerge4513 = phi i32 [ 1, %do.body.preheader.lr.ph ], [ %inc, %for.inc ]
  %mr.0.load404844884512 = phi i32 [ 0, %do.body.preheader.lr.ph ], [ %conv5, %for.inc ]
  br label %do.body

do.body:                                          ; preds = %do.body.preheader, %do.body
  %shl524486 = phi i32 [ %shr, %do.body ], [ %shl, %do.body.preheader ]
  %shr = ashr i32 %shl524486, 1
  %add = add i32 %shr, %mr.0.load404844884512
  %cmp2 = icmp sgt i32 %add, %sub
  br i1 %cmp2, label %do.body, label %do.end

do.end:                                           ; preds = %do.body
  %sub3 = add i32 %shr, 65535
  %and = and i32 %sub3, %mr.0.load404844884512
  %add4 = add i32 %and, %shr
  %conv = trunc i32 %add4 to i16
  %mul.i4000 = shl i16 %conv, 1
  %conv.i.i = sext i16 %conv to i32
  %conv1.i.i = sext i16 %mul.i4000 to i32
  %mul.i.i = mul i32 %conv1.i.i, %conv.i.i
  %shr1.i.i = lshr i32 %mul.i.i, 15
  %conv2.i.i = trunc i32 %shr1.i.i to i16
  %mul4.i = mul i16 %conv, 3
  %conv1.i4.i = sext i16 %mul4.i to i32
  %mul.i5.i = mul i32 %conv1.i4.i, %conv.i.i
  %shr1.i6.i = lshr i32 %mul.i5.i, 15
  %conv2.i7.i = trunc i32 %shr1.i6.i to i16
  %add.i4001 = add i16 %conv2.i.i, %conv2.i7.i
  %conv5 = sext i16 %add.i4001 to i32
  %cmp6 = icmp sgt i32 %conv5, %storemerge4513
  br i1 %cmp6, label %if.end9, label %for.inc

if.end9:                                          ; preds = %do.end
  %gep_array4528 = mul i32 %storemerge4513, 2
  %gep4529 = add i32 %fr, %gep_array4528
  %gep4529.asptr = inttoptr i32 %gep4529 to i16*
  %4 = load i16* %gep4529.asptr, align 1
  %gep_array4531 = mul i32 %conv5, 2
  %gep4532 = add i32 %fr, %gep_array4531
  %gep4532.asptr = inttoptr i32 %gep4532 to i16*
  %5 = load i16* %gep4532.asptr, align 1
  %gep4529.asptr2 = inttoptr i32 %gep4529 to i16*
  store i16 %5, i16* %gep4529.asptr2, align 1
  %gep4532.asptr3 = inttoptr i32 %gep4532 to i16*
  store i16 %4, i16* %gep4532.asptr3, align 1
  %gep_array4534 = mul i32 %storemerge4513, 2
  %gep4535 = add i32 %fi, %gep_array4534
  %gep4535.asptr = inttoptr i32 %gep4535 to i16*
  %6 = load i16* %gep4535.asptr, align 1
  %gep_array4537 = mul i32 %conv5, 2
  %gep4538 = add i32 %fi, %gep_array4537
  %gep4538.asptr = inttoptr i32 %gep4538 to i16*
  %7 = load i16* %gep4538.asptr, align 1
  %gep4535.asptr4 = inttoptr i32 %gep4535 to i16*
  store i16 %7, i16* %gep4535.asptr4, align 1
  %gep4538.asptr5 = inttoptr i32 %gep4538 to i16*
  store i16 %6, i16* %gep4538.asptr5, align 1
  br label %for.inc

for.inc:                                          ; preds = %do.end, %if.end9
  %inc = add i32 %storemerge4513, 1
  %cmp1 = icmp slt i32 %storemerge4513, %sub
  br i1 %cmp1, label %do.body.preheader, label %for.end

for.end:                                          ; preds = %for.inc
  %8 = select i1 %3, i32 %shl, i32 2
  %cmp20.i3970 = icmp sgt i32 %8, 0
  br i1 %cmp20.i3970, label %for.cond1.preheader.i3973.preheader, label %while.cond.preheader

for.cond1.preheader.i3973.preheader:              ; preds = %for.cond.preheader, %for.end
  %storemerge.lcssa4517 = phi i32 [ %8, %for.end ], [ 1, %for.cond.preheader ]
  br label %for.cond1.preheader.i3973

for.cond1.preheader.i3973:                        ; preds = %for.cond1.preheader.i3973.preheader, %for.inc17.i3998
  %storemerge21.i3971 = phi i32 [ %inc18.i3996, %for.inc17.i3998 ], [ 0, %for.cond1.preheader.i3973.preheader ]
  %cmp217.i3972 = icmp sgt i32 %storemerge21.i3971, 0
  br i1 %cmp217.i3972, label %for.cond4.preheader.lr.ph.i3977, label %for.inc17.i3998

for.cond4.preheader.lr.ph.i3977:                  ; preds = %for.cond1.preheader.i3973
  %sub.i3974 = add i32 %storemerge21.i3971, -2
  %gep_array4540 = mul i32 %sub.i3974, 2
  %gep4541 = add i32 %fr, %gep_array4540
  %gep_array4543 = mul i32 %storemerge21.i3971, 2
  %gep4544 = add i32 %fr, %gep_array4543
  br label %for.cond4.preheader.i3980

for.cond4.preheader.i3980:                        ; preds = %for.inc14.i3995, %for.cond4.preheader.lr.ph.i3977
  %storemerge118.i3978 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3977 ], [ %inc15.i3993, %for.inc14.i3995 ]
  %cmp515.i3979 = icmp sgt i32 %storemerge118.i3978, 0
  br i1 %cmp515.i3979, label %for.body6.lr.ph.i3982, label %for.inc14.i3995

for.body6.lr.ph.i3982:                            ; preds = %for.cond4.preheader.i3980
  %gep_array4546 = mul i32 %storemerge118.i3978, 2
  %gep4547 = add i32 %fi, %gep_array4546
  br label %for.body6.i3992

for.body6.i3992:                                  ; preds = %for.body6.i3992, %for.body6.lr.ph.i3982
  %storemerge216.i3983 = phi i32 [ 0, %for.body6.lr.ph.i3982 ], [ %add12.i3989, %for.body6.i3992 ]
  %gep4547.asptr = inttoptr i32 %gep4547 to i16*
  %9 = load i16* %gep4547.asptr, align 1
  %conv.i3984 = sext i16 %9 to i32
  %mul.i3985 = mul i32 %conv.i3984, %storemerge.lcssa4517
  %gep4541.asptr = inttoptr i32 %gep4541 to i16*
  %10 = load i16* %gep4541.asptr, align 1
  %conv83.i3986 = zext i16 %10 to i32
  %add.i3987 = add i32 %mul.i3985, %conv83.i3986
  %conv9.i3988 = trunc i32 %add.i3987 to i16
  %gep4544.asptr = inttoptr i32 %gep4544 to i16*
  store i16 %conv9.i3988, i16* %gep4544.asptr, align 1
  %gep4547.asptr6 = inttoptr i32 %gep4547 to i16*
  %11 = load i16* %gep4547.asptr6, align 1
  %add12.i3989 = add i32 %storemerge216.i3983, 1
  %gep_array4549 = mul i32 %add12.i3989, 2
  %gep4550 = add i32 %fr, %gep_array4549
  %gep4550.asptr = inttoptr i32 %gep4550 to i16*
  store i16 %11, i16* %gep4550.asptr, align 1
  %cmp5.i3991 = icmp slt i32 %add12.i3989, %storemerge118.i3978
  br i1 %cmp5.i3991, label %for.body6.i3992, label %for.inc14.i3995

for.inc14.i3995:                                  ; preds = %for.body6.i3992, %for.cond4.preheader.i3980
  %inc15.i3993 = add i32 %storemerge118.i3978, 1
  %cmp2.i3994 = icmp slt i32 %inc15.i3993, %storemerge21.i3971
  br i1 %cmp2.i3994, label %for.cond4.preheader.i3980, label %for.inc17.i3998

for.inc17.i3998:                                  ; preds = %for.inc14.i3995, %for.cond1.preheader.i3973
  %inc18.i3996 = add i32 %storemerge21.i3971, 1
  %cmp.i3997 = icmp slt i32 %inc18.i3996, %storemerge.lcssa4517
  br i1 %cmp.i3997, label %for.cond1.preheader.i3973, label %for.cond1.preheader.i3943

for.cond1.preheader.i3943:                        ; preds = %for.inc17.i3998, %for.inc17.i3968
  %storemerge21.i3941 = phi i32 [ %inc18.i3966, %for.inc17.i3968 ], [ 0, %for.inc17.i3998 ]
  %cmp217.i3942 = icmp sgt i32 %storemerge21.i3941, 0
  br i1 %cmp217.i3942, label %for.cond4.preheader.lr.ph.i3947, label %for.inc17.i3968

for.cond4.preheader.lr.ph.i3947:                  ; preds = %for.cond1.preheader.i3943
  %sub.i3944 = add i32 %storemerge21.i3941, -2
  %gep_array4552 = mul i32 %sub.i3944, 2
  %gep4553 = add i32 %fr, %gep_array4552
  %gep_array4555 = mul i32 %storemerge21.i3941, 2
  %gep4556 = add i32 %fr, %gep_array4555
  br label %for.cond4.preheader.i3950

for.cond4.preheader.i3950:                        ; preds = %for.inc14.i3965, %for.cond4.preheader.lr.ph.i3947
  %storemerge118.i3948 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3947 ], [ %inc15.i3963, %for.inc14.i3965 ]
  %cmp515.i3949 = icmp sgt i32 %storemerge118.i3948, 0
  br i1 %cmp515.i3949, label %for.body6.lr.ph.i3952, label %for.inc14.i3965

for.body6.lr.ph.i3952:                            ; preds = %for.cond4.preheader.i3950
  %gep_array4558 = mul i32 %storemerge118.i3948, 2
  %gep4559 = add i32 %Sinewave, %gep_array4558
  br label %for.body6.i3962

for.body6.i3962:                                  ; preds = %for.body6.i3962, %for.body6.lr.ph.i3952
  %storemerge216.i3953 = phi i32 [ 0, %for.body6.lr.ph.i3952 ], [ %add12.i3959, %for.body6.i3962 ]
  %gep4559.asptr = inttoptr i32 %gep4559 to i16*
  %12 = load i16* %gep4559.asptr, align 1
  %conv.i3954 = sext i16 %12 to i32
  %mul.i3955 = mul i32 %conv.i3954, %storemerge.lcssa4517
  %gep4553.asptr = inttoptr i32 %gep4553 to i16*
  %13 = load i16* %gep4553.asptr, align 1
  %conv83.i3956 = zext i16 %13 to i32
  %add.i3957 = add i32 %mul.i3955, %conv83.i3956
  %conv9.i3958 = trunc i32 %add.i3957 to i16
  %gep4556.asptr = inttoptr i32 %gep4556 to i16*
  store i16 %conv9.i3958, i16* %gep4556.asptr, align 1
  %gep4559.asptr7 = inttoptr i32 %gep4559 to i16*
  %14 = load i16* %gep4559.asptr7, align 1
  %add12.i3959 = add i32 %storemerge216.i3953, 1
  %gep_array4561 = mul i32 %add12.i3959, 2
  %gep4562 = add i32 %fr, %gep_array4561
  %gep4562.asptr = inttoptr i32 %gep4562 to i16*
  store i16 %14, i16* %gep4562.asptr, align 1
  %cmp5.i3961 = icmp slt i32 %add12.i3959, %storemerge118.i3948
  br i1 %cmp5.i3961, label %for.body6.i3962, label %for.inc14.i3965

for.inc14.i3965:                                  ; preds = %for.body6.i3962, %for.cond4.preheader.i3950
  %inc15.i3963 = add i32 %storemerge118.i3948, 1
  %cmp2.i3964 = icmp slt i32 %inc15.i3963, %storemerge21.i3941
  br i1 %cmp2.i3964, label %for.cond4.preheader.i3950, label %for.inc17.i3968

for.inc17.i3968:                                  ; preds = %for.inc14.i3965, %for.cond1.preheader.i3943
  %inc18.i3966 = add i32 %storemerge21.i3941, 1
  %cmp.i3967 = icmp slt i32 %inc18.i3966, %storemerge.lcssa4517
  br i1 %cmp.i3967, label %for.cond1.preheader.i3943, label %for.cond1.preheader.i3913

for.cond1.preheader.i3913:                        ; preds = %for.inc17.i3968, %for.inc17.i3938
  %storemerge21.i3911 = phi i32 [ %inc18.i3936, %for.inc17.i3938 ], [ 0, %for.inc17.i3968 ]
  %cmp217.i3912 = icmp sgt i32 %storemerge21.i3911, 0
  br i1 %cmp217.i3912, label %for.cond4.preheader.lr.ph.i3917, label %for.inc17.i3938

for.cond4.preheader.lr.ph.i3917:                  ; preds = %for.cond1.preheader.i3913
  %sub.i3914 = add i32 %storemerge21.i3911, -2
  %gep_array4564 = mul i32 %sub.i3914, 2
  %gep4565 = add i32 %fr, %gep_array4564
  %gep_array4567 = mul i32 %storemerge21.i3911, 2
  %gep4568 = add i32 %fr, %gep_array4567
  br label %for.cond4.preheader.i3920

for.cond4.preheader.i3920:                        ; preds = %for.inc14.i3935, %for.cond4.preheader.lr.ph.i3917
  %storemerge118.i3918 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3917 ], [ %inc15.i3933, %for.inc14.i3935 ]
  %cmp515.i3919 = icmp sgt i32 %storemerge118.i3918, 0
  br i1 %cmp515.i3919, label %for.body6.lr.ph.i3922, label %for.inc14.i3935

for.body6.lr.ph.i3922:                            ; preds = %for.cond4.preheader.i3920
  %gep_array4570 = mul i32 %storemerge118.i3918, 2
  %gep4571 = add i32 %fi, %gep_array4570
  br label %for.body6.i3932

for.body6.i3932:                                  ; preds = %for.body6.i3932, %for.body6.lr.ph.i3922
  %storemerge216.i3923 = phi i32 [ 0, %for.body6.lr.ph.i3922 ], [ %add12.i3929, %for.body6.i3932 ]
  %gep4571.asptr = inttoptr i32 %gep4571 to i16*
  %15 = load i16* %gep4571.asptr, align 1
  %conv.i3924 = sext i16 %15 to i32
  %mul.i3925 = mul i32 %conv.i3924, %storemerge.lcssa4517
  %gep4565.asptr = inttoptr i32 %gep4565 to i16*
  %16 = load i16* %gep4565.asptr, align 1
  %conv83.i3926 = zext i16 %16 to i32
  %add.i3927 = add i32 %mul.i3925, %conv83.i3926
  %conv9.i3928 = trunc i32 %add.i3927 to i16
  %gep4568.asptr = inttoptr i32 %gep4568 to i16*
  store i16 %conv9.i3928, i16* %gep4568.asptr, align 1
  %gep4571.asptr8 = inttoptr i32 %gep4571 to i16*
  %17 = load i16* %gep4571.asptr8, align 1
  %add12.i3929 = add i32 %storemerge216.i3923, 1
  %gep_array4573 = mul i32 %add12.i3929, 2
  %gep4574 = add i32 %fr, %gep_array4573
  %gep4574.asptr = inttoptr i32 %gep4574 to i16*
  store i16 %17, i16* %gep4574.asptr, align 1
  %cmp5.i3931 = icmp slt i32 %add12.i3929, %storemerge118.i3918
  br i1 %cmp5.i3931, label %for.body6.i3932, label %for.inc14.i3935

for.inc14.i3935:                                  ; preds = %for.body6.i3932, %for.cond4.preheader.i3920
  %inc15.i3933 = add i32 %storemerge118.i3918, 1
  %cmp2.i3934 = icmp slt i32 %inc15.i3933, %storemerge21.i3911
  br i1 %cmp2.i3934, label %for.cond4.preheader.i3920, label %for.inc17.i3938

for.inc17.i3938:                                  ; preds = %for.inc14.i3935, %for.cond1.preheader.i3913
  %inc18.i3936 = add i32 %storemerge21.i3911, 1
  %cmp.i3937 = icmp slt i32 %inc18.i3936, %storemerge.lcssa4517
  br i1 %cmp.i3937, label %for.cond1.preheader.i3913, label %for.cond1.preheader.i3883

for.cond1.preheader.i3883:                        ; preds = %for.inc17.i3938, %for.inc17.i3908
  %storemerge21.i3881 = phi i32 [ %inc18.i3906, %for.inc17.i3908 ], [ 0, %for.inc17.i3938 ]
  %cmp217.i3882 = icmp sgt i32 %storemerge21.i3881, 0
  br i1 %cmp217.i3882, label %for.cond4.preheader.lr.ph.i3887, label %for.inc17.i3908

for.cond4.preheader.lr.ph.i3887:                  ; preds = %for.cond1.preheader.i3883
  %sub.i3884 = add i32 %storemerge21.i3881, -2
  %gep_array4576 = mul i32 %sub.i3884, 2
  %gep4577 = add i32 %fr, %gep_array4576
  %gep_array4579 = mul i32 %storemerge21.i3881, 2
  %gep4580 = add i32 %fr, %gep_array4579
  br label %for.cond4.preheader.i3890

for.cond4.preheader.i3890:                        ; preds = %for.inc14.i3905, %for.cond4.preheader.lr.ph.i3887
  %storemerge118.i3888 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3887 ], [ %inc15.i3903, %for.inc14.i3905 ]
  %cmp515.i3889 = icmp sgt i32 %storemerge118.i3888, 0
  br i1 %cmp515.i3889, label %for.body6.lr.ph.i3892, label %for.inc14.i3905

for.body6.lr.ph.i3892:                            ; preds = %for.cond4.preheader.i3890
  %gep_array4582 = mul i32 %storemerge118.i3888, 2
  %gep4583 = add i32 %Sinewave, %gep_array4582
  br label %for.body6.i3902

for.body6.i3902:                                  ; preds = %for.body6.i3902, %for.body6.lr.ph.i3892
  %storemerge216.i3893 = phi i32 [ 0, %for.body6.lr.ph.i3892 ], [ %add12.i3899, %for.body6.i3902 ]
  %gep4583.asptr = inttoptr i32 %gep4583 to i16*
  %18 = load i16* %gep4583.asptr, align 1
  %conv.i3894 = sext i16 %18 to i32
  %mul.i3895 = mul i32 %conv.i3894, %storemerge.lcssa4517
  %gep4577.asptr = inttoptr i32 %gep4577 to i16*
  %19 = load i16* %gep4577.asptr, align 1
  %conv83.i3896 = zext i16 %19 to i32
  %add.i3897 = add i32 %mul.i3895, %conv83.i3896
  %conv9.i3898 = trunc i32 %add.i3897 to i16
  %gep4580.asptr = inttoptr i32 %gep4580 to i16*
  store i16 %conv9.i3898, i16* %gep4580.asptr, align 1
  %gep4583.asptr9 = inttoptr i32 %gep4583 to i16*
  %20 = load i16* %gep4583.asptr9, align 1
  %add12.i3899 = add i32 %storemerge216.i3893, 1
  %gep_array4585 = mul i32 %add12.i3899, 2
  %gep4586 = add i32 %fr, %gep_array4585
  %gep4586.asptr = inttoptr i32 %gep4586 to i16*
  store i16 %20, i16* %gep4586.asptr, align 1
  %cmp5.i3901 = icmp slt i32 %add12.i3899, %storemerge118.i3888
  br i1 %cmp5.i3901, label %for.body6.i3902, label %for.inc14.i3905

for.inc14.i3905:                                  ; preds = %for.body6.i3902, %for.cond4.preheader.i3890
  %inc15.i3903 = add i32 %storemerge118.i3888, 1
  %cmp2.i3904 = icmp slt i32 %inc15.i3903, %storemerge21.i3881
  br i1 %cmp2.i3904, label %for.cond4.preheader.i3890, label %for.inc17.i3908

for.inc17.i3908:                                  ; preds = %for.inc14.i3905, %for.cond1.preheader.i3883
  %inc18.i3906 = add i32 %storemerge21.i3881, 1
  %cmp.i3907 = icmp slt i32 %inc18.i3906, %storemerge.lcssa4517
  br i1 %cmp.i3907, label %for.cond1.preheader.i3883, label %for.cond1.preheader.i3853

for.cond1.preheader.i3853:                        ; preds = %for.inc17.i3908, %for.inc17.i3878
  %storemerge21.i3851 = phi i32 [ %inc18.i3876, %for.inc17.i3878 ], [ 0, %for.inc17.i3908 ]
  %cmp217.i3852 = icmp sgt i32 %storemerge21.i3851, 0
  br i1 %cmp217.i3852, label %for.cond4.preheader.lr.ph.i3857, label %for.inc17.i3878

for.cond4.preheader.lr.ph.i3857:                  ; preds = %for.cond1.preheader.i3853
  %sub.i3854 = add i32 %storemerge21.i3851, -2
  %gep_array4588 = mul i32 %sub.i3854, 2
  %gep4589 = add i32 %fr, %gep_array4588
  %gep_array4591 = mul i32 %storemerge21.i3851, 2
  %gep4592 = add i32 %fr, %gep_array4591
  br label %for.cond4.preheader.i3860

for.cond4.preheader.i3860:                        ; preds = %for.inc14.i3875, %for.cond4.preheader.lr.ph.i3857
  %storemerge118.i3858 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3857 ], [ %inc15.i3873, %for.inc14.i3875 ]
  %cmp515.i3859 = icmp sgt i32 %storemerge118.i3858, 0
  br i1 %cmp515.i3859, label %for.body6.lr.ph.i3862, label %for.inc14.i3875

for.body6.lr.ph.i3862:                            ; preds = %for.cond4.preheader.i3860
  %gep_array4594 = mul i32 %storemerge118.i3858, 2
  %gep4595 = add i32 %fi, %gep_array4594
  br label %for.body6.i3872

for.body6.i3872:                                  ; preds = %for.body6.i3872, %for.body6.lr.ph.i3862
  %storemerge216.i3863 = phi i32 [ 0, %for.body6.lr.ph.i3862 ], [ %add12.i3869, %for.body6.i3872 ]
  %gep4595.asptr = inttoptr i32 %gep4595 to i16*
  %21 = load i16* %gep4595.asptr, align 1
  %conv.i3864 = sext i16 %21 to i32
  %mul.i3865 = mul i32 %conv.i3864, %storemerge.lcssa4517
  %gep4589.asptr = inttoptr i32 %gep4589 to i16*
  %22 = load i16* %gep4589.asptr, align 1
  %conv83.i3866 = zext i16 %22 to i32
  %add.i3867 = add i32 %mul.i3865, %conv83.i3866
  %conv9.i3868 = trunc i32 %add.i3867 to i16
  %gep4592.asptr = inttoptr i32 %gep4592 to i16*
  store i16 %conv9.i3868, i16* %gep4592.asptr, align 1
  %gep4595.asptr10 = inttoptr i32 %gep4595 to i16*
  %23 = load i16* %gep4595.asptr10, align 1
  %add12.i3869 = add i32 %storemerge216.i3863, 1
  %gep_array4597 = mul i32 %add12.i3869, 2
  %gep4598 = add i32 %fr, %gep_array4597
  %gep4598.asptr = inttoptr i32 %gep4598 to i16*
  store i16 %23, i16* %gep4598.asptr, align 1
  %cmp5.i3871 = icmp slt i32 %add12.i3869, %storemerge118.i3858
  br i1 %cmp5.i3871, label %for.body6.i3872, label %for.inc14.i3875

for.inc14.i3875:                                  ; preds = %for.body6.i3872, %for.cond4.preheader.i3860
  %inc15.i3873 = add i32 %storemerge118.i3858, 1
  %cmp2.i3874 = icmp slt i32 %inc15.i3873, %storemerge21.i3851
  br i1 %cmp2.i3874, label %for.cond4.preheader.i3860, label %for.inc17.i3878

for.inc17.i3878:                                  ; preds = %for.inc14.i3875, %for.cond1.preheader.i3853
  %inc18.i3876 = add i32 %storemerge21.i3851, 1
  %cmp.i3877 = icmp slt i32 %inc18.i3876, %storemerge.lcssa4517
  br i1 %cmp.i3877, label %for.cond1.preheader.i3853, label %for.cond1.preheader.i3823

for.cond1.preheader.i3823:                        ; preds = %for.inc17.i3878, %for.inc17.i3848
  %storemerge21.i3821 = phi i32 [ %inc18.i3846, %for.inc17.i3848 ], [ 0, %for.inc17.i3878 ]
  %cmp217.i3822 = icmp sgt i32 %storemerge21.i3821, 0
  br i1 %cmp217.i3822, label %for.cond4.preheader.lr.ph.i3827, label %for.inc17.i3848

for.cond4.preheader.lr.ph.i3827:                  ; preds = %for.cond1.preheader.i3823
  %sub.i3824 = add i32 %storemerge21.i3821, -2
  %gep_array4600 = mul i32 %sub.i3824, 2
  %gep4601 = add i32 %fr, %gep_array4600
  %gep_array4603 = mul i32 %storemerge21.i3821, 2
  %gep4604 = add i32 %fr, %gep_array4603
  br label %for.cond4.preheader.i3830

for.cond4.preheader.i3830:                        ; preds = %for.inc14.i3845, %for.cond4.preheader.lr.ph.i3827
  %storemerge118.i3828 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3827 ], [ %inc15.i3843, %for.inc14.i3845 ]
  %cmp515.i3829 = icmp sgt i32 %storemerge118.i3828, 0
  br i1 %cmp515.i3829, label %for.body6.lr.ph.i3832, label %for.inc14.i3845

for.body6.lr.ph.i3832:                            ; preds = %for.cond4.preheader.i3830
  %gep_array4606 = mul i32 %storemerge118.i3828, 2
  %gep4607 = add i32 %Sinewave, %gep_array4606
  br label %for.body6.i3842

for.body6.i3842:                                  ; preds = %for.body6.i3842, %for.body6.lr.ph.i3832
  %storemerge216.i3833 = phi i32 [ 0, %for.body6.lr.ph.i3832 ], [ %add12.i3839, %for.body6.i3842 ]
  %gep4607.asptr = inttoptr i32 %gep4607 to i16*
  %24 = load i16* %gep4607.asptr, align 1
  %conv.i3834 = sext i16 %24 to i32
  %mul.i3835 = mul i32 %conv.i3834, %storemerge.lcssa4517
  %gep4601.asptr = inttoptr i32 %gep4601 to i16*
  %25 = load i16* %gep4601.asptr, align 1
  %conv83.i3836 = zext i16 %25 to i32
  %add.i3837 = add i32 %mul.i3835, %conv83.i3836
  %conv9.i3838 = trunc i32 %add.i3837 to i16
  %gep4604.asptr = inttoptr i32 %gep4604 to i16*
  store i16 %conv9.i3838, i16* %gep4604.asptr, align 1
  %gep4607.asptr11 = inttoptr i32 %gep4607 to i16*
  %26 = load i16* %gep4607.asptr11, align 1
  %add12.i3839 = add i32 %storemerge216.i3833, 1
  %gep_array4609 = mul i32 %add12.i3839, 2
  %gep4610 = add i32 %fr, %gep_array4609
  %gep4610.asptr = inttoptr i32 %gep4610 to i16*
  store i16 %26, i16* %gep4610.asptr, align 1
  %cmp5.i3841 = icmp slt i32 %add12.i3839, %storemerge118.i3828
  br i1 %cmp5.i3841, label %for.body6.i3842, label %for.inc14.i3845

for.inc14.i3845:                                  ; preds = %for.body6.i3842, %for.cond4.preheader.i3830
  %inc15.i3843 = add i32 %storemerge118.i3828, 1
  %cmp2.i3844 = icmp slt i32 %inc15.i3843, %storemerge21.i3821
  br i1 %cmp2.i3844, label %for.cond4.preheader.i3830, label %for.inc17.i3848

for.inc17.i3848:                                  ; preds = %for.inc14.i3845, %for.cond1.preheader.i3823
  %inc18.i3846 = add i32 %storemerge21.i3821, 1
  %cmp.i3847 = icmp slt i32 %inc18.i3846, %storemerge.lcssa4517
  br i1 %cmp.i3847, label %for.cond1.preheader.i3823, label %for.cond1.preheader.i3793

for.cond1.preheader.i3793:                        ; preds = %for.inc17.i3848, %for.inc17.i3818
  %storemerge21.i3791 = phi i32 [ %inc18.i3816, %for.inc17.i3818 ], [ 0, %for.inc17.i3848 ]
  %cmp217.i3792 = icmp sgt i32 %storemerge21.i3791, 0
  br i1 %cmp217.i3792, label %for.cond4.preheader.lr.ph.i3797, label %for.inc17.i3818

for.cond4.preheader.lr.ph.i3797:                  ; preds = %for.cond1.preheader.i3793
  %sub.i3794 = add i32 %storemerge21.i3791, -2
  %gep_array4612 = mul i32 %sub.i3794, 2
  %gep4613 = add i32 %fr, %gep_array4612
  %gep_array4615 = mul i32 %storemerge21.i3791, 2
  %gep4616 = add i32 %fr, %gep_array4615
  br label %for.cond4.preheader.i3800

for.cond4.preheader.i3800:                        ; preds = %for.inc14.i3815, %for.cond4.preheader.lr.ph.i3797
  %storemerge118.i3798 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3797 ], [ %inc15.i3813, %for.inc14.i3815 ]
  %cmp515.i3799 = icmp sgt i32 %storemerge118.i3798, 0
  br i1 %cmp515.i3799, label %for.body6.lr.ph.i3802, label %for.inc14.i3815

for.body6.lr.ph.i3802:                            ; preds = %for.cond4.preheader.i3800
  %gep_array4618 = mul i32 %storemerge118.i3798, 2
  %gep4619 = add i32 %fi, %gep_array4618
  br label %for.body6.i3812

for.body6.i3812:                                  ; preds = %for.body6.i3812, %for.body6.lr.ph.i3802
  %storemerge216.i3803 = phi i32 [ 0, %for.body6.lr.ph.i3802 ], [ %add12.i3809, %for.body6.i3812 ]
  %gep4619.asptr = inttoptr i32 %gep4619 to i16*
  %27 = load i16* %gep4619.asptr, align 1
  %conv.i3804 = sext i16 %27 to i32
  %mul.i3805 = mul i32 %conv.i3804, %storemerge.lcssa4517
  %gep4613.asptr = inttoptr i32 %gep4613 to i16*
  %28 = load i16* %gep4613.asptr, align 1
  %conv83.i3806 = zext i16 %28 to i32
  %add.i3807 = add i32 %mul.i3805, %conv83.i3806
  %conv9.i3808 = trunc i32 %add.i3807 to i16
  %gep4616.asptr = inttoptr i32 %gep4616 to i16*
  store i16 %conv9.i3808, i16* %gep4616.asptr, align 1
  %gep4619.asptr12 = inttoptr i32 %gep4619 to i16*
  %29 = load i16* %gep4619.asptr12, align 1
  %add12.i3809 = add i32 %storemerge216.i3803, 1
  %gep_array4621 = mul i32 %add12.i3809, 2
  %gep4622 = add i32 %fr, %gep_array4621
  %gep4622.asptr = inttoptr i32 %gep4622 to i16*
  store i16 %29, i16* %gep4622.asptr, align 1
  %cmp5.i3811 = icmp slt i32 %add12.i3809, %storemerge118.i3798
  br i1 %cmp5.i3811, label %for.body6.i3812, label %for.inc14.i3815

for.inc14.i3815:                                  ; preds = %for.body6.i3812, %for.cond4.preheader.i3800
  %inc15.i3813 = add i32 %storemerge118.i3798, 1
  %cmp2.i3814 = icmp slt i32 %inc15.i3813, %storemerge21.i3791
  br i1 %cmp2.i3814, label %for.cond4.preheader.i3800, label %for.inc17.i3818

for.inc17.i3818:                                  ; preds = %for.inc14.i3815, %for.cond1.preheader.i3793
  %inc18.i3816 = add i32 %storemerge21.i3791, 1
  %cmp.i3817 = icmp slt i32 %inc18.i3816, %storemerge.lcssa4517
  br i1 %cmp.i3817, label %for.cond1.preheader.i3793, label %for.cond1.preheader.i3763

for.cond1.preheader.i3763:                        ; preds = %for.inc17.i3818, %for.inc17.i3788
  %storemerge21.i3761 = phi i32 [ %inc18.i3786, %for.inc17.i3788 ], [ 0, %for.inc17.i3818 ]
  %cmp217.i3762 = icmp sgt i32 %storemerge21.i3761, 0
  br i1 %cmp217.i3762, label %for.cond4.preheader.lr.ph.i3767, label %for.inc17.i3788

for.cond4.preheader.lr.ph.i3767:                  ; preds = %for.cond1.preheader.i3763
  %sub.i3764 = add i32 %storemerge21.i3761, -2
  %gep_array4624 = mul i32 %sub.i3764, 2
  %gep4625 = add i32 %fr, %gep_array4624
  %gep_array4627 = mul i32 %storemerge21.i3761, 2
  %gep4628 = add i32 %fr, %gep_array4627
  br label %for.cond4.preheader.i3770

for.cond4.preheader.i3770:                        ; preds = %for.inc14.i3785, %for.cond4.preheader.lr.ph.i3767
  %storemerge118.i3768 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3767 ], [ %inc15.i3783, %for.inc14.i3785 ]
  %cmp515.i3769 = icmp sgt i32 %storemerge118.i3768, 0
  br i1 %cmp515.i3769, label %for.body6.lr.ph.i3772, label %for.inc14.i3785

for.body6.lr.ph.i3772:                            ; preds = %for.cond4.preheader.i3770
  %gep_array4630 = mul i32 %storemerge118.i3768, 2
  %gep4631 = add i32 %Sinewave, %gep_array4630
  br label %for.body6.i3782

for.body6.i3782:                                  ; preds = %for.body6.i3782, %for.body6.lr.ph.i3772
  %storemerge216.i3773 = phi i32 [ 0, %for.body6.lr.ph.i3772 ], [ %add12.i3779, %for.body6.i3782 ]
  %gep4631.asptr = inttoptr i32 %gep4631 to i16*
  %30 = load i16* %gep4631.asptr, align 1
  %conv.i3774 = sext i16 %30 to i32
  %mul.i3775 = mul i32 %conv.i3774, %storemerge.lcssa4517
  %gep4625.asptr = inttoptr i32 %gep4625 to i16*
  %31 = load i16* %gep4625.asptr, align 1
  %conv83.i3776 = zext i16 %31 to i32
  %add.i3777 = add i32 %mul.i3775, %conv83.i3776
  %conv9.i3778 = trunc i32 %add.i3777 to i16
  %gep4628.asptr = inttoptr i32 %gep4628 to i16*
  store i16 %conv9.i3778, i16* %gep4628.asptr, align 1
  %gep4631.asptr13 = inttoptr i32 %gep4631 to i16*
  %32 = load i16* %gep4631.asptr13, align 1
  %add12.i3779 = add i32 %storemerge216.i3773, 1
  %gep_array4633 = mul i32 %add12.i3779, 2
  %gep4634 = add i32 %fr, %gep_array4633
  %gep4634.asptr = inttoptr i32 %gep4634 to i16*
  store i16 %32, i16* %gep4634.asptr, align 1
  %cmp5.i3781 = icmp slt i32 %add12.i3779, %storemerge118.i3768
  br i1 %cmp5.i3781, label %for.body6.i3782, label %for.inc14.i3785

for.inc14.i3785:                                  ; preds = %for.body6.i3782, %for.cond4.preheader.i3770
  %inc15.i3783 = add i32 %storemerge118.i3768, 1
  %cmp2.i3784 = icmp slt i32 %inc15.i3783, %storemerge21.i3761
  br i1 %cmp2.i3784, label %for.cond4.preheader.i3770, label %for.inc17.i3788

for.inc17.i3788:                                  ; preds = %for.inc14.i3785, %for.cond1.preheader.i3763
  %inc18.i3786 = add i32 %storemerge21.i3761, 1
  %cmp.i3787 = icmp slt i32 %inc18.i3786, %storemerge.lcssa4517
  br i1 %cmp.i3787, label %for.cond1.preheader.i3763, label %for.cond1.preheader.i3733

for.cond1.preheader.i3733:                        ; preds = %for.inc17.i3788, %for.inc17.i3758
  %storemerge21.i3731 = phi i32 [ %inc18.i3756, %for.inc17.i3758 ], [ 0, %for.inc17.i3788 ]
  %cmp217.i3732 = icmp sgt i32 %storemerge21.i3731, 0
  br i1 %cmp217.i3732, label %for.cond4.preheader.lr.ph.i3737, label %for.inc17.i3758

for.cond4.preheader.lr.ph.i3737:                  ; preds = %for.cond1.preheader.i3733
  %sub.i3734 = add i32 %storemerge21.i3731, -2
  %gep_array4636 = mul i32 %sub.i3734, 2
  %gep4637 = add i32 %fr, %gep_array4636
  %gep_array4639 = mul i32 %storemerge21.i3731, 2
  %gep4640 = add i32 %fr, %gep_array4639
  br label %for.cond4.preheader.i3740

for.cond4.preheader.i3740:                        ; preds = %for.inc14.i3755, %for.cond4.preheader.lr.ph.i3737
  %storemerge118.i3738 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3737 ], [ %inc15.i3753, %for.inc14.i3755 ]
  %cmp515.i3739 = icmp sgt i32 %storemerge118.i3738, 0
  br i1 %cmp515.i3739, label %for.body6.lr.ph.i3742, label %for.inc14.i3755

for.body6.lr.ph.i3742:                            ; preds = %for.cond4.preheader.i3740
  %gep_array4642 = mul i32 %storemerge118.i3738, 2
  %gep4643 = add i32 %fi, %gep_array4642
  br label %for.body6.i3752

for.body6.i3752:                                  ; preds = %for.body6.i3752, %for.body6.lr.ph.i3742
  %storemerge216.i3743 = phi i32 [ 0, %for.body6.lr.ph.i3742 ], [ %add12.i3749, %for.body6.i3752 ]
  %gep4643.asptr = inttoptr i32 %gep4643 to i16*
  %33 = load i16* %gep4643.asptr, align 1
  %conv.i3744 = sext i16 %33 to i32
  %mul.i3745 = mul i32 %conv.i3744, %storemerge.lcssa4517
  %gep4637.asptr = inttoptr i32 %gep4637 to i16*
  %34 = load i16* %gep4637.asptr, align 1
  %conv83.i3746 = zext i16 %34 to i32
  %add.i3747 = add i32 %mul.i3745, %conv83.i3746
  %conv9.i3748 = trunc i32 %add.i3747 to i16
  %gep4640.asptr = inttoptr i32 %gep4640 to i16*
  store i16 %conv9.i3748, i16* %gep4640.asptr, align 1
  %gep4643.asptr14 = inttoptr i32 %gep4643 to i16*
  %35 = load i16* %gep4643.asptr14, align 1
  %add12.i3749 = add i32 %storemerge216.i3743, 1
  %gep_array4645 = mul i32 %add12.i3749, 2
  %gep4646 = add i32 %fr, %gep_array4645
  %gep4646.asptr = inttoptr i32 %gep4646 to i16*
  store i16 %35, i16* %gep4646.asptr, align 1
  %cmp5.i3751 = icmp slt i32 %add12.i3749, %storemerge118.i3738
  br i1 %cmp5.i3751, label %for.body6.i3752, label %for.inc14.i3755

for.inc14.i3755:                                  ; preds = %for.body6.i3752, %for.cond4.preheader.i3740
  %inc15.i3753 = add i32 %storemerge118.i3738, 1
  %cmp2.i3754 = icmp slt i32 %inc15.i3753, %storemerge21.i3731
  br i1 %cmp2.i3754, label %for.cond4.preheader.i3740, label %for.inc17.i3758

for.inc17.i3758:                                  ; preds = %for.inc14.i3755, %for.cond1.preheader.i3733
  %inc18.i3756 = add i32 %storemerge21.i3731, 1
  %cmp.i3757 = icmp slt i32 %inc18.i3756, %storemerge.lcssa4517
  br i1 %cmp.i3757, label %for.cond1.preheader.i3733, label %for.cond1.preheader.i3703

for.cond1.preheader.i3703:                        ; preds = %for.inc17.i3758, %for.inc17.i3728
  %storemerge21.i3701 = phi i32 [ %inc18.i3726, %for.inc17.i3728 ], [ 0, %for.inc17.i3758 ]
  %cmp217.i3702 = icmp sgt i32 %storemerge21.i3701, 0
  br i1 %cmp217.i3702, label %for.cond4.preheader.lr.ph.i3707, label %for.inc17.i3728

for.cond4.preheader.lr.ph.i3707:                  ; preds = %for.cond1.preheader.i3703
  %sub.i3704 = add i32 %storemerge21.i3701, -2
  %gep_array4648 = mul i32 %sub.i3704, 2
  %gep4649 = add i32 %fr, %gep_array4648
  %gep_array4651 = mul i32 %storemerge21.i3701, 2
  %gep4652 = add i32 %fr, %gep_array4651
  br label %for.cond4.preheader.i3710

for.cond4.preheader.i3710:                        ; preds = %for.inc14.i3725, %for.cond4.preheader.lr.ph.i3707
  %storemerge118.i3708 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3707 ], [ %inc15.i3723, %for.inc14.i3725 ]
  %cmp515.i3709 = icmp sgt i32 %storemerge118.i3708, 0
  br i1 %cmp515.i3709, label %for.body6.lr.ph.i3712, label %for.inc14.i3725

for.body6.lr.ph.i3712:                            ; preds = %for.cond4.preheader.i3710
  %gep_array4654 = mul i32 %storemerge118.i3708, 2
  %gep4655 = add i32 %Sinewave, %gep_array4654
  br label %for.body6.i3722

for.body6.i3722:                                  ; preds = %for.body6.i3722, %for.body6.lr.ph.i3712
  %storemerge216.i3713 = phi i32 [ 0, %for.body6.lr.ph.i3712 ], [ %add12.i3719, %for.body6.i3722 ]
  %gep4655.asptr = inttoptr i32 %gep4655 to i16*
  %36 = load i16* %gep4655.asptr, align 1
  %conv.i3714 = sext i16 %36 to i32
  %mul.i3715 = mul i32 %conv.i3714, %storemerge.lcssa4517
  %gep4649.asptr = inttoptr i32 %gep4649 to i16*
  %37 = load i16* %gep4649.asptr, align 1
  %conv83.i3716 = zext i16 %37 to i32
  %add.i3717 = add i32 %mul.i3715, %conv83.i3716
  %conv9.i3718 = trunc i32 %add.i3717 to i16
  %gep4652.asptr = inttoptr i32 %gep4652 to i16*
  store i16 %conv9.i3718, i16* %gep4652.asptr, align 1
  %gep4655.asptr15 = inttoptr i32 %gep4655 to i16*
  %38 = load i16* %gep4655.asptr15, align 1
  %add12.i3719 = add i32 %storemerge216.i3713, 1
  %gep_array4657 = mul i32 %add12.i3719, 2
  %gep4658 = add i32 %fr, %gep_array4657
  %gep4658.asptr = inttoptr i32 %gep4658 to i16*
  store i16 %38, i16* %gep4658.asptr, align 1
  %cmp5.i3721 = icmp slt i32 %add12.i3719, %storemerge118.i3708
  br i1 %cmp5.i3721, label %for.body6.i3722, label %for.inc14.i3725

for.inc14.i3725:                                  ; preds = %for.body6.i3722, %for.cond4.preheader.i3710
  %inc15.i3723 = add i32 %storemerge118.i3708, 1
  %cmp2.i3724 = icmp slt i32 %inc15.i3723, %storemerge21.i3701
  br i1 %cmp2.i3724, label %for.cond4.preheader.i3710, label %for.inc17.i3728

for.inc17.i3728:                                  ; preds = %for.inc14.i3725, %for.cond1.preheader.i3703
  %inc18.i3726 = add i32 %storemerge21.i3701, 1
  %cmp.i3727 = icmp slt i32 %inc18.i3726, %storemerge.lcssa4517
  br i1 %cmp.i3727, label %for.cond1.preheader.i3703, label %for.cond1.preheader.i3673

for.cond1.preheader.i3673:                        ; preds = %for.inc17.i3728, %for.inc17.i3698
  %storemerge21.i3671 = phi i32 [ %inc18.i3696, %for.inc17.i3698 ], [ 0, %for.inc17.i3728 ]
  %cmp217.i3672 = icmp sgt i32 %storemerge21.i3671, 0
  br i1 %cmp217.i3672, label %for.cond4.preheader.lr.ph.i3677, label %for.inc17.i3698

for.cond4.preheader.lr.ph.i3677:                  ; preds = %for.cond1.preheader.i3673
  %sub.i3674 = add i32 %storemerge21.i3671, -2
  %gep_array4660 = mul i32 %sub.i3674, 2
  %gep4661 = add i32 %fr, %gep_array4660
  %gep_array4663 = mul i32 %storemerge21.i3671, 2
  %gep4664 = add i32 %fr, %gep_array4663
  br label %for.cond4.preheader.i3680

for.cond4.preheader.i3680:                        ; preds = %for.inc14.i3695, %for.cond4.preheader.lr.ph.i3677
  %storemerge118.i3678 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3677 ], [ %inc15.i3693, %for.inc14.i3695 ]
  %cmp515.i3679 = icmp sgt i32 %storemerge118.i3678, 0
  br i1 %cmp515.i3679, label %for.body6.lr.ph.i3682, label %for.inc14.i3695

for.body6.lr.ph.i3682:                            ; preds = %for.cond4.preheader.i3680
  %gep_array4666 = mul i32 %storemerge118.i3678, 2
  %gep4667 = add i32 %fi, %gep_array4666
  br label %for.body6.i3692

for.body6.i3692:                                  ; preds = %for.body6.i3692, %for.body6.lr.ph.i3682
  %storemerge216.i3683 = phi i32 [ 0, %for.body6.lr.ph.i3682 ], [ %add12.i3689, %for.body6.i3692 ]
  %gep4667.asptr = inttoptr i32 %gep4667 to i16*
  %39 = load i16* %gep4667.asptr, align 1
  %conv.i3684 = sext i16 %39 to i32
  %mul.i3685 = mul i32 %conv.i3684, %storemerge.lcssa4517
  %gep4661.asptr = inttoptr i32 %gep4661 to i16*
  %40 = load i16* %gep4661.asptr, align 1
  %conv83.i3686 = zext i16 %40 to i32
  %add.i3687 = add i32 %mul.i3685, %conv83.i3686
  %conv9.i3688 = trunc i32 %add.i3687 to i16
  %gep4664.asptr = inttoptr i32 %gep4664 to i16*
  store i16 %conv9.i3688, i16* %gep4664.asptr, align 1
  %gep4667.asptr16 = inttoptr i32 %gep4667 to i16*
  %41 = load i16* %gep4667.asptr16, align 1
  %add12.i3689 = add i32 %storemerge216.i3683, 1
  %gep_array4669 = mul i32 %add12.i3689, 2
  %gep4670 = add i32 %fr, %gep_array4669
  %gep4670.asptr = inttoptr i32 %gep4670 to i16*
  store i16 %41, i16* %gep4670.asptr, align 1
  %cmp5.i3691 = icmp slt i32 %add12.i3689, %storemerge118.i3678
  br i1 %cmp5.i3691, label %for.body6.i3692, label %for.inc14.i3695

for.inc14.i3695:                                  ; preds = %for.body6.i3692, %for.cond4.preheader.i3680
  %inc15.i3693 = add i32 %storemerge118.i3678, 1
  %cmp2.i3694 = icmp slt i32 %inc15.i3693, %storemerge21.i3671
  br i1 %cmp2.i3694, label %for.cond4.preheader.i3680, label %for.inc17.i3698

for.inc17.i3698:                                  ; preds = %for.inc14.i3695, %for.cond1.preheader.i3673
  %inc18.i3696 = add i32 %storemerge21.i3671, 1
  %cmp.i3697 = icmp slt i32 %inc18.i3696, %storemerge.lcssa4517
  br i1 %cmp.i3697, label %for.cond1.preheader.i3673, label %for.cond1.preheader.i3643

for.cond1.preheader.i3643:                        ; preds = %for.inc17.i3698, %for.inc17.i3668
  %storemerge21.i3641 = phi i32 [ %inc18.i3666, %for.inc17.i3668 ], [ 0, %for.inc17.i3698 ]
  %cmp217.i3642 = icmp sgt i32 %storemerge21.i3641, 0
  br i1 %cmp217.i3642, label %for.cond4.preheader.lr.ph.i3647, label %for.inc17.i3668

for.cond4.preheader.lr.ph.i3647:                  ; preds = %for.cond1.preheader.i3643
  %sub.i3644 = add i32 %storemerge21.i3641, -2
  %gep_array4672 = mul i32 %sub.i3644, 2
  %gep4673 = add i32 %fr, %gep_array4672
  %gep_array4675 = mul i32 %storemerge21.i3641, 2
  %gep4676 = add i32 %fr, %gep_array4675
  br label %for.cond4.preheader.i3650

for.cond4.preheader.i3650:                        ; preds = %for.inc14.i3665, %for.cond4.preheader.lr.ph.i3647
  %storemerge118.i3648 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3647 ], [ %inc15.i3663, %for.inc14.i3665 ]
  %cmp515.i3649 = icmp sgt i32 %storemerge118.i3648, 0
  br i1 %cmp515.i3649, label %for.body6.lr.ph.i3652, label %for.inc14.i3665

for.body6.lr.ph.i3652:                            ; preds = %for.cond4.preheader.i3650
  %gep_array4678 = mul i32 %storemerge118.i3648, 2
  %gep4679 = add i32 %Sinewave, %gep_array4678
  br label %for.body6.i3662

for.body6.i3662:                                  ; preds = %for.body6.i3662, %for.body6.lr.ph.i3652
  %storemerge216.i3653 = phi i32 [ 0, %for.body6.lr.ph.i3652 ], [ %add12.i3659, %for.body6.i3662 ]
  %gep4679.asptr = inttoptr i32 %gep4679 to i16*
  %42 = load i16* %gep4679.asptr, align 1
  %conv.i3654 = sext i16 %42 to i32
  %mul.i3655 = mul i32 %conv.i3654, %storemerge.lcssa4517
  %gep4673.asptr = inttoptr i32 %gep4673 to i16*
  %43 = load i16* %gep4673.asptr, align 1
  %conv83.i3656 = zext i16 %43 to i32
  %add.i3657 = add i32 %mul.i3655, %conv83.i3656
  %conv9.i3658 = trunc i32 %add.i3657 to i16
  %gep4676.asptr = inttoptr i32 %gep4676 to i16*
  store i16 %conv9.i3658, i16* %gep4676.asptr, align 1
  %gep4679.asptr17 = inttoptr i32 %gep4679 to i16*
  %44 = load i16* %gep4679.asptr17, align 1
  %add12.i3659 = add i32 %storemerge216.i3653, 1
  %gep_array4681 = mul i32 %add12.i3659, 2
  %gep4682 = add i32 %fr, %gep_array4681
  %gep4682.asptr = inttoptr i32 %gep4682 to i16*
  store i16 %44, i16* %gep4682.asptr, align 1
  %cmp5.i3661 = icmp slt i32 %add12.i3659, %storemerge118.i3648
  br i1 %cmp5.i3661, label %for.body6.i3662, label %for.inc14.i3665

for.inc14.i3665:                                  ; preds = %for.body6.i3662, %for.cond4.preheader.i3650
  %inc15.i3663 = add i32 %storemerge118.i3648, 1
  %cmp2.i3664 = icmp slt i32 %inc15.i3663, %storemerge21.i3641
  br i1 %cmp2.i3664, label %for.cond4.preheader.i3650, label %for.inc17.i3668

for.inc17.i3668:                                  ; preds = %for.inc14.i3665, %for.cond1.preheader.i3643
  %inc18.i3666 = add i32 %storemerge21.i3641, 1
  %cmp.i3667 = icmp slt i32 %inc18.i3666, %storemerge.lcssa4517
  br i1 %cmp.i3667, label %for.cond1.preheader.i3643, label %for.cond1.preheader.i3613

for.cond1.preheader.i3613:                        ; preds = %for.inc17.i3668, %for.inc17.i3638
  %storemerge21.i3611 = phi i32 [ %inc18.i3636, %for.inc17.i3638 ], [ 0, %for.inc17.i3668 ]
  %cmp217.i3612 = icmp sgt i32 %storemerge21.i3611, 0
  br i1 %cmp217.i3612, label %for.cond4.preheader.lr.ph.i3617, label %for.inc17.i3638

for.cond4.preheader.lr.ph.i3617:                  ; preds = %for.cond1.preheader.i3613
  %sub.i3614 = add i32 %storemerge21.i3611, -2
  %gep_array4684 = mul i32 %sub.i3614, 2
  %gep4685 = add i32 %fr, %gep_array4684
  %gep_array4687 = mul i32 %storemerge21.i3611, 2
  %gep4688 = add i32 %fr, %gep_array4687
  br label %for.cond4.preheader.i3620

for.cond4.preheader.i3620:                        ; preds = %for.inc14.i3635, %for.cond4.preheader.lr.ph.i3617
  %storemerge118.i3618 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3617 ], [ %inc15.i3633, %for.inc14.i3635 ]
  %cmp515.i3619 = icmp sgt i32 %storemerge118.i3618, 0
  br i1 %cmp515.i3619, label %for.body6.lr.ph.i3622, label %for.inc14.i3635

for.body6.lr.ph.i3622:                            ; preds = %for.cond4.preheader.i3620
  %gep_array4690 = mul i32 %storemerge118.i3618, 2
  %gep4691 = add i32 %fi, %gep_array4690
  br label %for.body6.i3632

for.body6.i3632:                                  ; preds = %for.body6.i3632, %for.body6.lr.ph.i3622
  %storemerge216.i3623 = phi i32 [ 0, %for.body6.lr.ph.i3622 ], [ %add12.i3629, %for.body6.i3632 ]
  %gep4691.asptr = inttoptr i32 %gep4691 to i16*
  %45 = load i16* %gep4691.asptr, align 1
  %conv.i3624 = sext i16 %45 to i32
  %mul.i3625 = mul i32 %conv.i3624, %storemerge.lcssa4517
  %gep4685.asptr = inttoptr i32 %gep4685 to i16*
  %46 = load i16* %gep4685.asptr, align 1
  %conv83.i3626 = zext i16 %46 to i32
  %add.i3627 = add i32 %mul.i3625, %conv83.i3626
  %conv9.i3628 = trunc i32 %add.i3627 to i16
  %gep4688.asptr = inttoptr i32 %gep4688 to i16*
  store i16 %conv9.i3628, i16* %gep4688.asptr, align 1
  %gep4691.asptr18 = inttoptr i32 %gep4691 to i16*
  %47 = load i16* %gep4691.asptr18, align 1
  %add12.i3629 = add i32 %storemerge216.i3623, 1
  %gep_array4693 = mul i32 %add12.i3629, 2
  %gep4694 = add i32 %fr, %gep_array4693
  %gep4694.asptr = inttoptr i32 %gep4694 to i16*
  store i16 %47, i16* %gep4694.asptr, align 1
  %cmp5.i3631 = icmp slt i32 %add12.i3629, %storemerge118.i3618
  br i1 %cmp5.i3631, label %for.body6.i3632, label %for.inc14.i3635

for.inc14.i3635:                                  ; preds = %for.body6.i3632, %for.cond4.preheader.i3620
  %inc15.i3633 = add i32 %storemerge118.i3618, 1
  %cmp2.i3634 = icmp slt i32 %inc15.i3633, %storemerge21.i3611
  br i1 %cmp2.i3634, label %for.cond4.preheader.i3620, label %for.inc17.i3638

for.inc17.i3638:                                  ; preds = %for.inc14.i3635, %for.cond1.preheader.i3613
  %inc18.i3636 = add i32 %storemerge21.i3611, 1
  %cmp.i3637 = icmp slt i32 %inc18.i3636, %storemerge.lcssa4517
  br i1 %cmp.i3637, label %for.cond1.preheader.i3613, label %for.cond1.preheader.i3583

for.cond1.preheader.i3583:                        ; preds = %for.inc17.i3638, %for.inc17.i3608
  %storemerge21.i3581 = phi i32 [ %inc18.i3606, %for.inc17.i3608 ], [ 0, %for.inc17.i3638 ]
  %cmp217.i3582 = icmp sgt i32 %storemerge21.i3581, 0
  br i1 %cmp217.i3582, label %for.cond4.preheader.lr.ph.i3587, label %for.inc17.i3608

for.cond4.preheader.lr.ph.i3587:                  ; preds = %for.cond1.preheader.i3583
  %sub.i3584 = add i32 %storemerge21.i3581, -2
  %gep_array4696 = mul i32 %sub.i3584, 2
  %gep4697 = add i32 %fr, %gep_array4696
  %gep_array4699 = mul i32 %storemerge21.i3581, 2
  %gep4700 = add i32 %fr, %gep_array4699
  br label %for.cond4.preheader.i3590

for.cond4.preheader.i3590:                        ; preds = %for.inc14.i3605, %for.cond4.preheader.lr.ph.i3587
  %storemerge118.i3588 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3587 ], [ %inc15.i3603, %for.inc14.i3605 ]
  %cmp515.i3589 = icmp sgt i32 %storemerge118.i3588, 0
  br i1 %cmp515.i3589, label %for.body6.lr.ph.i3592, label %for.inc14.i3605

for.body6.lr.ph.i3592:                            ; preds = %for.cond4.preheader.i3590
  %gep_array4702 = mul i32 %storemerge118.i3588, 2
  %gep4703 = add i32 %fi, %gep_array4702
  br label %for.body6.i3602

for.body6.i3602:                                  ; preds = %for.body6.i3602, %for.body6.lr.ph.i3592
  %storemerge216.i3593 = phi i32 [ 0, %for.body6.lr.ph.i3592 ], [ %add12.i3599, %for.body6.i3602 ]
  %gep4703.asptr = inttoptr i32 %gep4703 to i16*
  %48 = load i16* %gep4703.asptr, align 1
  %conv.i3594 = sext i16 %48 to i32
  %mul.i3595 = mul i32 %conv.i3594, %storemerge.lcssa4517
  %gep4697.asptr = inttoptr i32 %gep4697 to i16*
  %49 = load i16* %gep4697.asptr, align 1
  %conv83.i3596 = zext i16 %49 to i32
  %add.i3597 = add i32 %mul.i3595, %conv83.i3596
  %conv9.i3598 = trunc i32 %add.i3597 to i16
  %gep4700.asptr = inttoptr i32 %gep4700 to i16*
  store i16 %conv9.i3598, i16* %gep4700.asptr, align 1
  %gep4703.asptr19 = inttoptr i32 %gep4703 to i16*
  %50 = load i16* %gep4703.asptr19, align 1
  %add12.i3599 = add i32 %storemerge216.i3593, 1
  %gep_array4705 = mul i32 %add12.i3599, 2
  %gep4706 = add i32 %fr, %gep_array4705
  %gep4706.asptr = inttoptr i32 %gep4706 to i16*
  store i16 %50, i16* %gep4706.asptr, align 1
  %cmp5.i3601 = icmp slt i32 %add12.i3599, %storemerge118.i3588
  br i1 %cmp5.i3601, label %for.body6.i3602, label %for.inc14.i3605

for.inc14.i3605:                                  ; preds = %for.body6.i3602, %for.cond4.preheader.i3590
  %inc15.i3603 = add i32 %storemerge118.i3588, 1
  %cmp2.i3604 = icmp slt i32 %inc15.i3603, %storemerge21.i3581
  br i1 %cmp2.i3604, label %for.cond4.preheader.i3590, label %for.inc17.i3608

for.inc17.i3608:                                  ; preds = %for.inc14.i3605, %for.cond1.preheader.i3583
  %inc18.i3606 = add i32 %storemerge21.i3581, 1
  %cmp.i3607 = icmp slt i32 %inc18.i3606, %storemerge.lcssa4517
  br i1 %cmp.i3607, label %for.cond1.preheader.i3583, label %for.cond1.preheader.i3553

for.cond1.preheader.i3553:                        ; preds = %for.inc17.i3608, %for.inc17.i3578
  %storemerge21.i3551 = phi i32 [ %inc18.i3576, %for.inc17.i3578 ], [ 0, %for.inc17.i3608 ]
  %cmp217.i3552 = icmp sgt i32 %storemerge21.i3551, 0
  br i1 %cmp217.i3552, label %for.cond4.preheader.lr.ph.i3557, label %for.inc17.i3578

for.cond4.preheader.lr.ph.i3557:                  ; preds = %for.cond1.preheader.i3553
  %sub.i3554 = add i32 %storemerge21.i3551, -2
  %gep_array4708 = mul i32 %sub.i3554, 2
  %gep4709 = add i32 %fr, %gep_array4708
  %gep_array4711 = mul i32 %storemerge21.i3551, 2
  %gep4712 = add i32 %fr, %gep_array4711
  br label %for.cond4.preheader.i3560

for.cond4.preheader.i3560:                        ; preds = %for.inc14.i3575, %for.cond4.preheader.lr.ph.i3557
  %storemerge118.i3558 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3557 ], [ %inc15.i3573, %for.inc14.i3575 ]
  %cmp515.i3559 = icmp sgt i32 %storemerge118.i3558, 0
  br i1 %cmp515.i3559, label %for.body6.lr.ph.i3562, label %for.inc14.i3575

for.body6.lr.ph.i3562:                            ; preds = %for.cond4.preheader.i3560
  %gep_array4714 = mul i32 %storemerge118.i3558, 2
  %gep4715 = add i32 %fi, %gep_array4714
  br label %for.body6.i3572

for.body6.i3572:                                  ; preds = %for.body6.i3572, %for.body6.lr.ph.i3562
  %storemerge216.i3563 = phi i32 [ 0, %for.body6.lr.ph.i3562 ], [ %add12.i3569, %for.body6.i3572 ]
  %gep4715.asptr = inttoptr i32 %gep4715 to i16*
  %51 = load i16* %gep4715.asptr, align 1
  %conv.i3564 = sext i16 %51 to i32
  %mul.i3565 = mul i32 %conv.i3564, %storemerge.lcssa4517
  %gep4709.asptr = inttoptr i32 %gep4709 to i16*
  %52 = load i16* %gep4709.asptr, align 1
  %conv83.i3566 = zext i16 %52 to i32
  %add.i3567 = add i32 %mul.i3565, %conv83.i3566
  %conv9.i3568 = trunc i32 %add.i3567 to i16
  %gep4712.asptr = inttoptr i32 %gep4712 to i16*
  store i16 %conv9.i3568, i16* %gep4712.asptr, align 1
  %gep4715.asptr20 = inttoptr i32 %gep4715 to i16*
  %53 = load i16* %gep4715.asptr20, align 1
  %add12.i3569 = add i32 %storemerge216.i3563, 1
  %gep_array4717 = mul i32 %add12.i3569, 2
  %gep4718 = add i32 %fr, %gep_array4717
  %gep4718.asptr = inttoptr i32 %gep4718 to i16*
  store i16 %53, i16* %gep4718.asptr, align 1
  %cmp5.i3571 = icmp slt i32 %add12.i3569, %storemerge118.i3558
  br i1 %cmp5.i3571, label %for.body6.i3572, label %for.inc14.i3575

for.inc14.i3575:                                  ; preds = %for.body6.i3572, %for.cond4.preheader.i3560
  %inc15.i3573 = add i32 %storemerge118.i3558, 1
  %cmp2.i3574 = icmp slt i32 %inc15.i3573, %storemerge21.i3551
  br i1 %cmp2.i3574, label %for.cond4.preheader.i3560, label %for.inc17.i3578

for.inc17.i3578:                                  ; preds = %for.inc14.i3575, %for.cond1.preheader.i3553
  %inc18.i3576 = add i32 %storemerge21.i3551, 1
  %cmp.i3577 = icmp slt i32 %inc18.i3576, %storemerge.lcssa4517
  br i1 %cmp.i3577, label %for.cond1.preheader.i3553, label %for.cond1.preheader.i3523

for.cond1.preheader.i3523:                        ; preds = %for.inc17.i3578, %for.inc17.i3548
  %storemerge21.i3521 = phi i32 [ %inc18.i3546, %for.inc17.i3548 ], [ 0, %for.inc17.i3578 ]
  %cmp217.i3522 = icmp sgt i32 %storemerge21.i3521, 0
  br i1 %cmp217.i3522, label %for.cond4.preheader.lr.ph.i3527, label %for.inc17.i3548

for.cond4.preheader.lr.ph.i3527:                  ; preds = %for.cond1.preheader.i3523
  %sub.i3524 = add i32 %storemerge21.i3521, -2
  %gep_array4720 = mul i32 %sub.i3524, 2
  %gep4721 = add i32 %fr, %gep_array4720
  %gep_array4723 = mul i32 %storemerge21.i3521, 2
  %gep4724 = add i32 %fr, %gep_array4723
  br label %for.cond4.preheader.i3530

for.cond4.preheader.i3530:                        ; preds = %for.inc14.i3545, %for.cond4.preheader.lr.ph.i3527
  %storemerge118.i3528 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3527 ], [ %inc15.i3543, %for.inc14.i3545 ]
  %cmp515.i3529 = icmp sgt i32 %storemerge118.i3528, 0
  br i1 %cmp515.i3529, label %for.body6.lr.ph.i3532, label %for.inc14.i3545

for.body6.lr.ph.i3532:                            ; preds = %for.cond4.preheader.i3530
  %gep_array4726 = mul i32 %storemerge118.i3528, 2
  %gep4727 = add i32 %fi, %gep_array4726
  br label %for.body6.i3542

for.body6.i3542:                                  ; preds = %for.body6.i3542, %for.body6.lr.ph.i3532
  %storemerge216.i3533 = phi i32 [ 0, %for.body6.lr.ph.i3532 ], [ %add12.i3539, %for.body6.i3542 ]
  %gep4727.asptr = inttoptr i32 %gep4727 to i16*
  %54 = load i16* %gep4727.asptr, align 1
  %conv.i3534 = sext i16 %54 to i32
  %mul.i3535 = mul i32 %conv.i3534, %storemerge.lcssa4517
  %gep4721.asptr = inttoptr i32 %gep4721 to i16*
  %55 = load i16* %gep4721.asptr, align 1
  %conv83.i3536 = zext i16 %55 to i32
  %add.i3537 = add i32 %mul.i3535, %conv83.i3536
  %conv9.i3538 = trunc i32 %add.i3537 to i16
  %gep4724.asptr = inttoptr i32 %gep4724 to i16*
  store i16 %conv9.i3538, i16* %gep4724.asptr, align 1
  %gep4727.asptr21 = inttoptr i32 %gep4727 to i16*
  %56 = load i16* %gep4727.asptr21, align 1
  %add12.i3539 = add i32 %storemerge216.i3533, 1
  %gep_array4729 = mul i32 %add12.i3539, 2
  %gep4730 = add i32 %fr, %gep_array4729
  %gep4730.asptr = inttoptr i32 %gep4730 to i16*
  store i16 %56, i16* %gep4730.asptr, align 1
  %cmp5.i3541 = icmp slt i32 %add12.i3539, %storemerge118.i3528
  br i1 %cmp5.i3541, label %for.body6.i3542, label %for.inc14.i3545

for.inc14.i3545:                                  ; preds = %for.body6.i3542, %for.cond4.preheader.i3530
  %inc15.i3543 = add i32 %storemerge118.i3528, 1
  %cmp2.i3544 = icmp slt i32 %inc15.i3543, %storemerge21.i3521
  br i1 %cmp2.i3544, label %for.cond4.preheader.i3530, label %for.inc17.i3548

for.inc17.i3548:                                  ; preds = %for.inc14.i3545, %for.cond1.preheader.i3523
  %inc18.i3546 = add i32 %storemerge21.i3521, 1
  %cmp.i3547 = icmp slt i32 %inc18.i3546, %storemerge.lcssa4517
  br i1 %cmp.i3547, label %for.cond1.preheader.i3523, label %for.cond1.preheader.i3493

for.cond1.preheader.i3493:                        ; preds = %for.inc17.i3548, %for.inc17.i3518
  %storemerge21.i3491 = phi i32 [ %inc18.i3516, %for.inc17.i3518 ], [ 0, %for.inc17.i3548 ]
  %cmp217.i3492 = icmp sgt i32 %storemerge21.i3491, 0
  br i1 %cmp217.i3492, label %for.cond4.preheader.lr.ph.i3497, label %for.inc17.i3518

for.cond4.preheader.lr.ph.i3497:                  ; preds = %for.cond1.preheader.i3493
  %sub.i3494 = add i32 %storemerge21.i3491, -2
  %gep_array4732 = mul i32 %sub.i3494, 2
  %gep4733 = add i32 %fr, %gep_array4732
  %gep_array4735 = mul i32 %storemerge21.i3491, 2
  %gep4736 = add i32 %fr, %gep_array4735
  br label %for.cond4.preheader.i3500

for.cond4.preheader.i3500:                        ; preds = %for.inc14.i3515, %for.cond4.preheader.lr.ph.i3497
  %storemerge118.i3498 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3497 ], [ %inc15.i3513, %for.inc14.i3515 ]
  %cmp515.i3499 = icmp sgt i32 %storemerge118.i3498, 0
  br i1 %cmp515.i3499, label %for.body6.lr.ph.i3502, label %for.inc14.i3515

for.body6.lr.ph.i3502:                            ; preds = %for.cond4.preheader.i3500
  %gep_array4738 = mul i32 %storemerge118.i3498, 2
  %gep4739 = add i32 %fi, %gep_array4738
  br label %for.body6.i3512

for.body6.i3512:                                  ; preds = %for.body6.i3512, %for.body6.lr.ph.i3502
  %storemerge216.i3503 = phi i32 [ 0, %for.body6.lr.ph.i3502 ], [ %add12.i3509, %for.body6.i3512 ]
  %gep4739.asptr = inttoptr i32 %gep4739 to i16*
  %57 = load i16* %gep4739.asptr, align 1
  %conv.i3504 = sext i16 %57 to i32
  %mul.i3505 = mul i32 %conv.i3504, %storemerge.lcssa4517
  %gep4733.asptr = inttoptr i32 %gep4733 to i16*
  %58 = load i16* %gep4733.asptr, align 1
  %conv83.i3506 = zext i16 %58 to i32
  %add.i3507 = add i32 %mul.i3505, %conv83.i3506
  %conv9.i3508 = trunc i32 %add.i3507 to i16
  %gep4736.asptr = inttoptr i32 %gep4736 to i16*
  store i16 %conv9.i3508, i16* %gep4736.asptr, align 1
  %gep4739.asptr22 = inttoptr i32 %gep4739 to i16*
  %59 = load i16* %gep4739.asptr22, align 1
  %add12.i3509 = add i32 %storemerge216.i3503, 1
  %gep_array4741 = mul i32 %add12.i3509, 2
  %gep4742 = add i32 %fr, %gep_array4741
  %gep4742.asptr = inttoptr i32 %gep4742 to i16*
  store i16 %59, i16* %gep4742.asptr, align 1
  %cmp5.i3511 = icmp slt i32 %add12.i3509, %storemerge118.i3498
  br i1 %cmp5.i3511, label %for.body6.i3512, label %for.inc14.i3515

for.inc14.i3515:                                  ; preds = %for.body6.i3512, %for.cond4.preheader.i3500
  %inc15.i3513 = add i32 %storemerge118.i3498, 1
  %cmp2.i3514 = icmp slt i32 %inc15.i3513, %storemerge21.i3491
  br i1 %cmp2.i3514, label %for.cond4.preheader.i3500, label %for.inc17.i3518

for.inc17.i3518:                                  ; preds = %for.inc14.i3515, %for.cond1.preheader.i3493
  %inc18.i3516 = add i32 %storemerge21.i3491, 1
  %cmp.i3517 = icmp slt i32 %inc18.i3516, %storemerge.lcssa4517
  br i1 %cmp.i3517, label %for.cond1.preheader.i3493, label %for.cond1.preheader.i3463

for.cond1.preheader.i3463:                        ; preds = %for.inc17.i3518, %for.inc17.i3488
  %storemerge21.i3461 = phi i32 [ %inc18.i3486, %for.inc17.i3488 ], [ 0, %for.inc17.i3518 ]
  %cmp217.i3462 = icmp sgt i32 %storemerge21.i3461, 0
  br i1 %cmp217.i3462, label %for.cond4.preheader.lr.ph.i3467, label %for.inc17.i3488

for.cond4.preheader.lr.ph.i3467:                  ; preds = %for.cond1.preheader.i3463
  %sub.i3464 = add i32 %storemerge21.i3461, -2
  %gep_array4744 = mul i32 %sub.i3464, 2
  %gep4745 = add i32 %fr, %gep_array4744
  %gep_array4747 = mul i32 %storemerge21.i3461, 2
  %gep4748 = add i32 %fr, %gep_array4747
  br label %for.cond4.preheader.i3470

for.cond4.preheader.i3470:                        ; preds = %for.inc14.i3485, %for.cond4.preheader.lr.ph.i3467
  %storemerge118.i3468 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3467 ], [ %inc15.i3483, %for.inc14.i3485 ]
  %cmp515.i3469 = icmp sgt i32 %storemerge118.i3468, 0
  br i1 %cmp515.i3469, label %for.body6.lr.ph.i3472, label %for.inc14.i3485

for.body6.lr.ph.i3472:                            ; preds = %for.cond4.preheader.i3470
  %gep_array4750 = mul i32 %storemerge118.i3468, 2
  %gep4751 = add i32 %fi, %gep_array4750
  br label %for.body6.i3482

for.body6.i3482:                                  ; preds = %for.body6.i3482, %for.body6.lr.ph.i3472
  %storemerge216.i3473 = phi i32 [ 0, %for.body6.lr.ph.i3472 ], [ %add12.i3479, %for.body6.i3482 ]
  %gep4751.asptr = inttoptr i32 %gep4751 to i16*
  %60 = load i16* %gep4751.asptr, align 1
  %conv.i3474 = sext i16 %60 to i32
  %mul.i3475 = mul i32 %conv.i3474, %storemerge.lcssa4517
  %gep4745.asptr = inttoptr i32 %gep4745 to i16*
  %61 = load i16* %gep4745.asptr, align 1
  %conv83.i3476 = zext i16 %61 to i32
  %add.i3477 = add i32 %mul.i3475, %conv83.i3476
  %conv9.i3478 = trunc i32 %add.i3477 to i16
  %gep4748.asptr = inttoptr i32 %gep4748 to i16*
  store i16 %conv9.i3478, i16* %gep4748.asptr, align 1
  %gep4751.asptr23 = inttoptr i32 %gep4751 to i16*
  %62 = load i16* %gep4751.asptr23, align 1
  %add12.i3479 = add i32 %storemerge216.i3473, 1
  %gep_array4753 = mul i32 %add12.i3479, 2
  %gep4754 = add i32 %fr, %gep_array4753
  %gep4754.asptr = inttoptr i32 %gep4754 to i16*
  store i16 %62, i16* %gep4754.asptr, align 1
  %cmp5.i3481 = icmp slt i32 %add12.i3479, %storemerge118.i3468
  br i1 %cmp5.i3481, label %for.body6.i3482, label %for.inc14.i3485

for.inc14.i3485:                                  ; preds = %for.body6.i3482, %for.cond4.preheader.i3470
  %inc15.i3483 = add i32 %storemerge118.i3468, 1
  %cmp2.i3484 = icmp slt i32 %inc15.i3483, %storemerge21.i3461
  br i1 %cmp2.i3484, label %for.cond4.preheader.i3470, label %for.inc17.i3488

for.inc17.i3488:                                  ; preds = %for.inc14.i3485, %for.cond1.preheader.i3463
  %inc18.i3486 = add i32 %storemerge21.i3461, 1
  %cmp.i3487 = icmp slt i32 %inc18.i3486, %storemerge.lcssa4517
  br i1 %cmp.i3487, label %for.cond1.preheader.i3463, label %for.cond1.preheader.i3433

for.cond1.preheader.i3433:                        ; preds = %for.inc17.i3488, %for.inc17.i3458
  %storemerge21.i3431 = phi i32 [ %inc18.i3456, %for.inc17.i3458 ], [ 0, %for.inc17.i3488 ]
  %cmp217.i3432 = icmp sgt i32 %storemerge21.i3431, 0
  br i1 %cmp217.i3432, label %for.cond4.preheader.lr.ph.i3437, label %for.inc17.i3458

for.cond4.preheader.lr.ph.i3437:                  ; preds = %for.cond1.preheader.i3433
  %sub.i3434 = add i32 %storemerge21.i3431, -2
  %gep_array4756 = mul i32 %sub.i3434, 2
  %gep4757 = add i32 %fr, %gep_array4756
  %gep_array4759 = mul i32 %storemerge21.i3431, 2
  %gep4760 = add i32 %fr, %gep_array4759
  br label %for.cond4.preheader.i3440

for.cond4.preheader.i3440:                        ; preds = %for.inc14.i3455, %for.cond4.preheader.lr.ph.i3437
  %storemerge118.i3438 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3437 ], [ %inc15.i3453, %for.inc14.i3455 ]
  %cmp515.i3439 = icmp sgt i32 %storemerge118.i3438, 0
  br i1 %cmp515.i3439, label %for.body6.lr.ph.i3442, label %for.inc14.i3455

for.body6.lr.ph.i3442:                            ; preds = %for.cond4.preheader.i3440
  %gep_array4762 = mul i32 %storemerge118.i3438, 2
  %gep4763 = add i32 %fi, %gep_array4762
  br label %for.body6.i3452

for.body6.i3452:                                  ; preds = %for.body6.i3452, %for.body6.lr.ph.i3442
  %storemerge216.i3443 = phi i32 [ 0, %for.body6.lr.ph.i3442 ], [ %add12.i3449, %for.body6.i3452 ]
  %gep4763.asptr = inttoptr i32 %gep4763 to i16*
  %63 = load i16* %gep4763.asptr, align 1
  %conv.i3444 = sext i16 %63 to i32
  %mul.i3445 = mul i32 %conv.i3444, %storemerge.lcssa4517
  %gep4757.asptr = inttoptr i32 %gep4757 to i16*
  %64 = load i16* %gep4757.asptr, align 1
  %conv83.i3446 = zext i16 %64 to i32
  %add.i3447 = add i32 %mul.i3445, %conv83.i3446
  %conv9.i3448 = trunc i32 %add.i3447 to i16
  %gep4760.asptr = inttoptr i32 %gep4760 to i16*
  store i16 %conv9.i3448, i16* %gep4760.asptr, align 1
  %gep4763.asptr24 = inttoptr i32 %gep4763 to i16*
  %65 = load i16* %gep4763.asptr24, align 1
  %add12.i3449 = add i32 %storemerge216.i3443, 1
  %gep_array4765 = mul i32 %add12.i3449, 2
  %gep4766 = add i32 %fr, %gep_array4765
  %gep4766.asptr = inttoptr i32 %gep4766 to i16*
  store i16 %65, i16* %gep4766.asptr, align 1
  %cmp5.i3451 = icmp slt i32 %add12.i3449, %storemerge118.i3438
  br i1 %cmp5.i3451, label %for.body6.i3452, label %for.inc14.i3455

for.inc14.i3455:                                  ; preds = %for.body6.i3452, %for.cond4.preheader.i3440
  %inc15.i3453 = add i32 %storemerge118.i3438, 1
  %cmp2.i3454 = icmp slt i32 %inc15.i3453, %storemerge21.i3431
  br i1 %cmp2.i3454, label %for.cond4.preheader.i3440, label %for.inc17.i3458

for.inc17.i3458:                                  ; preds = %for.inc14.i3455, %for.cond1.preheader.i3433
  %inc18.i3456 = add i32 %storemerge21.i3431, 1
  %cmp.i3457 = icmp slt i32 %inc18.i3456, %storemerge.lcssa4517
  br i1 %cmp.i3457, label %for.cond1.preheader.i3433, label %for.cond1.preheader.i3403

for.cond1.preheader.i3403:                        ; preds = %for.inc17.i3458, %for.inc17.i3428
  %storemerge21.i3401 = phi i32 [ %inc18.i3426, %for.inc17.i3428 ], [ 0, %for.inc17.i3458 ]
  %cmp217.i3402 = icmp sgt i32 %storemerge21.i3401, 0
  br i1 %cmp217.i3402, label %for.cond4.preheader.lr.ph.i3407, label %for.inc17.i3428

for.cond4.preheader.lr.ph.i3407:                  ; preds = %for.cond1.preheader.i3403
  %sub.i3404 = add i32 %storemerge21.i3401, -2
  %gep_array4768 = mul i32 %sub.i3404, 2
  %gep4769 = add i32 %fr, %gep_array4768
  %gep_array4771 = mul i32 %storemerge21.i3401, 2
  %gep4772 = add i32 %fr, %gep_array4771
  br label %for.cond4.preheader.i3410

for.cond4.preheader.i3410:                        ; preds = %for.inc14.i3425, %for.cond4.preheader.lr.ph.i3407
  %storemerge118.i3408 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3407 ], [ %inc15.i3423, %for.inc14.i3425 ]
  %cmp515.i3409 = icmp sgt i32 %storemerge118.i3408, 0
  br i1 %cmp515.i3409, label %for.body6.lr.ph.i3412, label %for.inc14.i3425

for.body6.lr.ph.i3412:                            ; preds = %for.cond4.preheader.i3410
  %gep_array4774 = mul i32 %storemerge118.i3408, 2
  %gep4775 = add i32 %Sinewave, %gep_array4774
  br label %for.body6.i3422

for.body6.i3422:                                  ; preds = %for.body6.i3422, %for.body6.lr.ph.i3412
  %storemerge216.i3413 = phi i32 [ 0, %for.body6.lr.ph.i3412 ], [ %add12.i3419, %for.body6.i3422 ]
  %gep4775.asptr = inttoptr i32 %gep4775 to i16*
  %66 = load i16* %gep4775.asptr, align 1
  %conv.i3414 = sext i16 %66 to i32
  %mul.i3415 = mul i32 %conv.i3414, %storemerge.lcssa4517
  %gep4769.asptr = inttoptr i32 %gep4769 to i16*
  %67 = load i16* %gep4769.asptr, align 1
  %conv83.i3416 = zext i16 %67 to i32
  %add.i3417 = add i32 %mul.i3415, %conv83.i3416
  %conv9.i3418 = trunc i32 %add.i3417 to i16
  %gep4772.asptr = inttoptr i32 %gep4772 to i16*
  store i16 %conv9.i3418, i16* %gep4772.asptr, align 1
  %gep4775.asptr25 = inttoptr i32 %gep4775 to i16*
  %68 = load i16* %gep4775.asptr25, align 1
  %add12.i3419 = add i32 %storemerge216.i3413, 1
  %gep_array4777 = mul i32 %add12.i3419, 2
  %gep4778 = add i32 %fr, %gep_array4777
  %gep4778.asptr = inttoptr i32 %gep4778 to i16*
  store i16 %68, i16* %gep4778.asptr, align 1
  %cmp5.i3421 = icmp slt i32 %add12.i3419, %storemerge118.i3408
  br i1 %cmp5.i3421, label %for.body6.i3422, label %for.inc14.i3425

for.inc14.i3425:                                  ; preds = %for.body6.i3422, %for.cond4.preheader.i3410
  %inc15.i3423 = add i32 %storemerge118.i3408, 1
  %cmp2.i3424 = icmp slt i32 %inc15.i3423, %storemerge21.i3401
  br i1 %cmp2.i3424, label %for.cond4.preheader.i3410, label %for.inc17.i3428

for.inc17.i3428:                                  ; preds = %for.inc14.i3425, %for.cond1.preheader.i3403
  %inc18.i3426 = add i32 %storemerge21.i3401, 1
  %cmp.i3427 = icmp slt i32 %inc18.i3426, %storemerge.lcssa4517
  br i1 %cmp.i3427, label %for.cond1.preheader.i3403, label %for.cond1.preheader.i3373

for.cond1.preheader.i3373:                        ; preds = %for.inc17.i3428, %for.inc17.i3398
  %storemerge21.i3371 = phi i32 [ %inc18.i3396, %for.inc17.i3398 ], [ 0, %for.inc17.i3428 ]
  %cmp217.i3372 = icmp sgt i32 %storemerge21.i3371, 0
  br i1 %cmp217.i3372, label %for.cond4.preheader.lr.ph.i3377, label %for.inc17.i3398

for.cond4.preheader.lr.ph.i3377:                  ; preds = %for.cond1.preheader.i3373
  %sub.i3374 = add i32 %storemerge21.i3371, -2
  %gep_array4780 = mul i32 %sub.i3374, 2
  %gep4781 = add i32 %fr, %gep_array4780
  %gep_array4783 = mul i32 %storemerge21.i3371, 2
  %gep4784 = add i32 %fr, %gep_array4783
  br label %for.cond4.preheader.i3380

for.cond4.preheader.i3380:                        ; preds = %for.inc14.i3395, %for.cond4.preheader.lr.ph.i3377
  %storemerge118.i3378 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3377 ], [ %inc15.i3393, %for.inc14.i3395 ]
  %cmp515.i3379 = icmp sgt i32 %storemerge118.i3378, 0
  br i1 %cmp515.i3379, label %for.body6.lr.ph.i3382, label %for.inc14.i3395

for.body6.lr.ph.i3382:                            ; preds = %for.cond4.preheader.i3380
  %gep_array4786 = mul i32 %storemerge118.i3378, 2
  %gep4787 = add i32 %fi, %gep_array4786
  br label %for.body6.i3392

for.body6.i3392:                                  ; preds = %for.body6.i3392, %for.body6.lr.ph.i3382
  %storemerge216.i3383 = phi i32 [ 0, %for.body6.lr.ph.i3382 ], [ %add12.i3389, %for.body6.i3392 ]
  %gep4787.asptr = inttoptr i32 %gep4787 to i16*
  %69 = load i16* %gep4787.asptr, align 1
  %conv.i3384 = sext i16 %69 to i32
  %mul.i3385 = mul i32 %conv.i3384, %storemerge.lcssa4517
  %gep4781.asptr = inttoptr i32 %gep4781 to i16*
  %70 = load i16* %gep4781.asptr, align 1
  %conv83.i3386 = zext i16 %70 to i32
  %add.i3387 = add i32 %mul.i3385, %conv83.i3386
  %conv9.i3388 = trunc i32 %add.i3387 to i16
  %gep4784.asptr = inttoptr i32 %gep4784 to i16*
  store i16 %conv9.i3388, i16* %gep4784.asptr, align 1
  %gep4787.asptr26 = inttoptr i32 %gep4787 to i16*
  %71 = load i16* %gep4787.asptr26, align 1
  %add12.i3389 = add i32 %storemerge216.i3383, 1
  %gep_array4789 = mul i32 %add12.i3389, 2
  %gep4790 = add i32 %fr, %gep_array4789
  %gep4790.asptr = inttoptr i32 %gep4790 to i16*
  store i16 %71, i16* %gep4790.asptr, align 1
  %cmp5.i3391 = icmp slt i32 %add12.i3389, %storemerge118.i3378
  br i1 %cmp5.i3391, label %for.body6.i3392, label %for.inc14.i3395

for.inc14.i3395:                                  ; preds = %for.body6.i3392, %for.cond4.preheader.i3380
  %inc15.i3393 = add i32 %storemerge118.i3378, 1
  %cmp2.i3394 = icmp slt i32 %inc15.i3393, %storemerge21.i3371
  br i1 %cmp2.i3394, label %for.cond4.preheader.i3380, label %for.inc17.i3398

for.inc17.i3398:                                  ; preds = %for.inc14.i3395, %for.cond1.preheader.i3373
  %inc18.i3396 = add i32 %storemerge21.i3371, 1
  %cmp.i3397 = icmp slt i32 %inc18.i3396, %storemerge.lcssa4517
  br i1 %cmp.i3397, label %for.cond1.preheader.i3373, label %for.cond1.preheader.i3343

for.cond1.preheader.i3343:                        ; preds = %for.inc17.i3398, %for.inc17.i3368
  %storemerge21.i3341 = phi i32 [ %inc18.i3366, %for.inc17.i3368 ], [ 0, %for.inc17.i3398 ]
  %cmp217.i3342 = icmp sgt i32 %storemerge21.i3341, 0
  br i1 %cmp217.i3342, label %for.cond4.preheader.lr.ph.i3347, label %for.inc17.i3368

for.cond4.preheader.lr.ph.i3347:                  ; preds = %for.cond1.preheader.i3343
  %sub.i3344 = add i32 %storemerge21.i3341, -2
  %gep_array4792 = mul i32 %sub.i3344, 2
  %gep4793 = add i32 %fr, %gep_array4792
  %gep_array4795 = mul i32 %storemerge21.i3341, 2
  %gep4796 = add i32 %fr, %gep_array4795
  br label %for.cond4.preheader.i3350

for.cond4.preheader.i3350:                        ; preds = %for.inc14.i3365, %for.cond4.preheader.lr.ph.i3347
  %storemerge118.i3348 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3347 ], [ %inc15.i3363, %for.inc14.i3365 ]
  %cmp515.i3349 = icmp sgt i32 %storemerge118.i3348, 0
  br i1 %cmp515.i3349, label %for.body6.lr.ph.i3352, label %for.inc14.i3365

for.body6.lr.ph.i3352:                            ; preds = %for.cond4.preheader.i3350
  %gep_array4798 = mul i32 %storemerge118.i3348, 2
  %gep4799 = add i32 %Sinewave, %gep_array4798
  br label %for.body6.i3362

for.body6.i3362:                                  ; preds = %for.body6.i3362, %for.body6.lr.ph.i3352
  %storemerge216.i3353 = phi i32 [ 0, %for.body6.lr.ph.i3352 ], [ %add12.i3359, %for.body6.i3362 ]
  %gep4799.asptr = inttoptr i32 %gep4799 to i16*
  %72 = load i16* %gep4799.asptr, align 1
  %conv.i3354 = sext i16 %72 to i32
  %mul.i3355 = mul i32 %conv.i3354, %storemerge.lcssa4517
  %gep4793.asptr = inttoptr i32 %gep4793 to i16*
  %73 = load i16* %gep4793.asptr, align 1
  %conv83.i3356 = zext i16 %73 to i32
  %add.i3357 = add i32 %mul.i3355, %conv83.i3356
  %conv9.i3358 = trunc i32 %add.i3357 to i16
  %gep4796.asptr = inttoptr i32 %gep4796 to i16*
  store i16 %conv9.i3358, i16* %gep4796.asptr, align 1
  %gep4799.asptr27 = inttoptr i32 %gep4799 to i16*
  %74 = load i16* %gep4799.asptr27, align 1
  %add12.i3359 = add i32 %storemerge216.i3353, 1
  %gep_array4801 = mul i32 %add12.i3359, 2
  %gep4802 = add i32 %fr, %gep_array4801
  %gep4802.asptr = inttoptr i32 %gep4802 to i16*
  store i16 %74, i16* %gep4802.asptr, align 1
  %cmp5.i3361 = icmp slt i32 %add12.i3359, %storemerge118.i3348
  br i1 %cmp5.i3361, label %for.body6.i3362, label %for.inc14.i3365

for.inc14.i3365:                                  ; preds = %for.body6.i3362, %for.cond4.preheader.i3350
  %inc15.i3363 = add i32 %storemerge118.i3348, 1
  %cmp2.i3364 = icmp slt i32 %inc15.i3363, %storemerge21.i3341
  br i1 %cmp2.i3364, label %for.cond4.preheader.i3350, label %for.inc17.i3368

for.inc17.i3368:                                  ; preds = %for.inc14.i3365, %for.cond1.preheader.i3343
  %inc18.i3366 = add i32 %storemerge21.i3341, 1
  %cmp.i3367 = icmp slt i32 %inc18.i3366, %storemerge.lcssa4517
  br i1 %cmp.i3367, label %for.cond1.preheader.i3343, label %for.cond1.preheader.i3313

for.cond1.preheader.i3313:                        ; preds = %for.inc17.i3368, %for.inc17.i3338
  %storemerge21.i3311 = phi i32 [ %inc18.i3336, %for.inc17.i3338 ], [ 0, %for.inc17.i3368 ]
  %cmp217.i3312 = icmp sgt i32 %storemerge21.i3311, 0
  br i1 %cmp217.i3312, label %for.cond4.preheader.lr.ph.i3317, label %for.inc17.i3338

for.cond4.preheader.lr.ph.i3317:                  ; preds = %for.cond1.preheader.i3313
  %sub.i3314 = add i32 %storemerge21.i3311, -2
  %gep_array4804 = mul i32 %sub.i3314, 2
  %gep4805 = add i32 %fr, %gep_array4804
  %gep_array4807 = mul i32 %storemerge21.i3311, 2
  %gep4808 = add i32 %fr, %gep_array4807
  br label %for.cond4.preheader.i3320

for.cond4.preheader.i3320:                        ; preds = %for.inc14.i3335, %for.cond4.preheader.lr.ph.i3317
  %storemerge118.i3318 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3317 ], [ %inc15.i3333, %for.inc14.i3335 ]
  %cmp515.i3319 = icmp sgt i32 %storemerge118.i3318, 0
  br i1 %cmp515.i3319, label %for.body6.lr.ph.i3322, label %for.inc14.i3335

for.body6.lr.ph.i3322:                            ; preds = %for.cond4.preheader.i3320
  %gep_array4810 = mul i32 %storemerge118.i3318, 2
  %gep4811 = add i32 %fi, %gep_array4810
  br label %for.body6.i3332

for.body6.i3332:                                  ; preds = %for.body6.i3332, %for.body6.lr.ph.i3322
  %storemerge216.i3323 = phi i32 [ 0, %for.body6.lr.ph.i3322 ], [ %add12.i3329, %for.body6.i3332 ]
  %gep4811.asptr = inttoptr i32 %gep4811 to i16*
  %75 = load i16* %gep4811.asptr, align 1
  %conv.i3324 = sext i16 %75 to i32
  %mul.i3325 = mul i32 %conv.i3324, %storemerge.lcssa4517
  %gep4805.asptr = inttoptr i32 %gep4805 to i16*
  %76 = load i16* %gep4805.asptr, align 1
  %conv83.i3326 = zext i16 %76 to i32
  %add.i3327 = add i32 %mul.i3325, %conv83.i3326
  %conv9.i3328 = trunc i32 %add.i3327 to i16
  %gep4808.asptr = inttoptr i32 %gep4808 to i16*
  store i16 %conv9.i3328, i16* %gep4808.asptr, align 1
  %gep4811.asptr28 = inttoptr i32 %gep4811 to i16*
  %77 = load i16* %gep4811.asptr28, align 1
  %add12.i3329 = add i32 %storemerge216.i3323, 1
  %gep_array4813 = mul i32 %add12.i3329, 2
  %gep4814 = add i32 %fr, %gep_array4813
  %gep4814.asptr = inttoptr i32 %gep4814 to i16*
  store i16 %77, i16* %gep4814.asptr, align 1
  %cmp5.i3331 = icmp slt i32 %add12.i3329, %storemerge118.i3318
  br i1 %cmp5.i3331, label %for.body6.i3332, label %for.inc14.i3335

for.inc14.i3335:                                  ; preds = %for.body6.i3332, %for.cond4.preheader.i3320
  %inc15.i3333 = add i32 %storemerge118.i3318, 1
  %cmp2.i3334 = icmp slt i32 %inc15.i3333, %storemerge21.i3311
  br i1 %cmp2.i3334, label %for.cond4.preheader.i3320, label %for.inc17.i3338

for.inc17.i3338:                                  ; preds = %for.inc14.i3335, %for.cond1.preheader.i3313
  %inc18.i3336 = add i32 %storemerge21.i3311, 1
  %cmp.i3337 = icmp slt i32 %inc18.i3336, %storemerge.lcssa4517
  br i1 %cmp.i3337, label %for.cond1.preheader.i3313, label %for.cond1.preheader.i3283

for.cond1.preheader.i3283:                        ; preds = %for.inc17.i3338, %for.inc17.i3308
  %storemerge21.i3281 = phi i32 [ %inc18.i3306, %for.inc17.i3308 ], [ 0, %for.inc17.i3338 ]
  %cmp217.i3282 = icmp sgt i32 %storemerge21.i3281, 0
  br i1 %cmp217.i3282, label %for.cond4.preheader.lr.ph.i3287, label %for.inc17.i3308

for.cond4.preheader.lr.ph.i3287:                  ; preds = %for.cond1.preheader.i3283
  %sub.i3284 = add i32 %storemerge21.i3281, -2
  %gep_array4816 = mul i32 %sub.i3284, 2
  %gep4817 = add i32 %fr, %gep_array4816
  %gep_array4819 = mul i32 %storemerge21.i3281, 2
  %gep4820 = add i32 %fr, %gep_array4819
  br label %for.cond4.preheader.i3290

for.cond4.preheader.i3290:                        ; preds = %for.inc14.i3305, %for.cond4.preheader.lr.ph.i3287
  %storemerge118.i3288 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3287 ], [ %inc15.i3303, %for.inc14.i3305 ]
  %cmp515.i3289 = icmp sgt i32 %storemerge118.i3288, 0
  br i1 %cmp515.i3289, label %for.body6.lr.ph.i3292, label %for.inc14.i3305

for.body6.lr.ph.i3292:                            ; preds = %for.cond4.preheader.i3290
  %gep_array4822 = mul i32 %storemerge118.i3288, 2
  %gep4823 = add i32 %Sinewave, %gep_array4822
  br label %for.body6.i3302

for.body6.i3302:                                  ; preds = %for.body6.i3302, %for.body6.lr.ph.i3292
  %storemerge216.i3293 = phi i32 [ 0, %for.body6.lr.ph.i3292 ], [ %add12.i3299, %for.body6.i3302 ]
  %gep4823.asptr = inttoptr i32 %gep4823 to i16*
  %78 = load i16* %gep4823.asptr, align 1
  %conv.i3294 = sext i16 %78 to i32
  %mul.i3295 = mul i32 %conv.i3294, %storemerge.lcssa4517
  %gep4817.asptr = inttoptr i32 %gep4817 to i16*
  %79 = load i16* %gep4817.asptr, align 1
  %conv83.i3296 = zext i16 %79 to i32
  %add.i3297 = add i32 %mul.i3295, %conv83.i3296
  %conv9.i3298 = trunc i32 %add.i3297 to i16
  %gep4820.asptr = inttoptr i32 %gep4820 to i16*
  store i16 %conv9.i3298, i16* %gep4820.asptr, align 1
  %gep4823.asptr29 = inttoptr i32 %gep4823 to i16*
  %80 = load i16* %gep4823.asptr29, align 1
  %add12.i3299 = add i32 %storemerge216.i3293, 1
  %gep_array4825 = mul i32 %add12.i3299, 2
  %gep4826 = add i32 %fr, %gep_array4825
  %gep4826.asptr = inttoptr i32 %gep4826 to i16*
  store i16 %80, i16* %gep4826.asptr, align 1
  %cmp5.i3301 = icmp slt i32 %add12.i3299, %storemerge118.i3288
  br i1 %cmp5.i3301, label %for.body6.i3302, label %for.inc14.i3305

for.inc14.i3305:                                  ; preds = %for.body6.i3302, %for.cond4.preheader.i3290
  %inc15.i3303 = add i32 %storemerge118.i3288, 1
  %cmp2.i3304 = icmp slt i32 %inc15.i3303, %storemerge21.i3281
  br i1 %cmp2.i3304, label %for.cond4.preheader.i3290, label %for.inc17.i3308

for.inc17.i3308:                                  ; preds = %for.inc14.i3305, %for.cond1.preheader.i3283
  %inc18.i3306 = add i32 %storemerge21.i3281, 1
  %cmp.i3307 = icmp slt i32 %inc18.i3306, %storemerge.lcssa4517
  br i1 %cmp.i3307, label %for.cond1.preheader.i3283, label %for.cond1.preheader.i3253

for.cond1.preheader.i3253:                        ; preds = %for.inc17.i3308, %for.inc17.i3278
  %storemerge21.i3251 = phi i32 [ %inc18.i3276, %for.inc17.i3278 ], [ 0, %for.inc17.i3308 ]
  %cmp217.i3252 = icmp sgt i32 %storemerge21.i3251, 0
  br i1 %cmp217.i3252, label %for.cond4.preheader.lr.ph.i3257, label %for.inc17.i3278

for.cond4.preheader.lr.ph.i3257:                  ; preds = %for.cond1.preheader.i3253
  %sub.i3254 = add i32 %storemerge21.i3251, -2
  %gep_array4828 = mul i32 %sub.i3254, 2
  %gep4829 = add i32 %fr, %gep_array4828
  %gep_array4831 = mul i32 %storemerge21.i3251, 2
  %gep4832 = add i32 %fr, %gep_array4831
  br label %for.cond4.preheader.i3260

for.cond4.preheader.i3260:                        ; preds = %for.inc14.i3275, %for.cond4.preheader.lr.ph.i3257
  %storemerge118.i3258 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3257 ], [ %inc15.i3273, %for.inc14.i3275 ]
  %cmp515.i3259 = icmp sgt i32 %storemerge118.i3258, 0
  br i1 %cmp515.i3259, label %for.body6.lr.ph.i3262, label %for.inc14.i3275

for.body6.lr.ph.i3262:                            ; preds = %for.cond4.preheader.i3260
  %gep_array4834 = mul i32 %storemerge118.i3258, 2
  %gep4835 = add i32 %fi, %gep_array4834
  br label %for.body6.i3272

for.body6.i3272:                                  ; preds = %for.body6.i3272, %for.body6.lr.ph.i3262
  %storemerge216.i3263 = phi i32 [ 0, %for.body6.lr.ph.i3262 ], [ %add12.i3269, %for.body6.i3272 ]
  %gep4835.asptr = inttoptr i32 %gep4835 to i16*
  %81 = load i16* %gep4835.asptr, align 1
  %conv.i3264 = sext i16 %81 to i32
  %mul.i3265 = mul i32 %conv.i3264, %storemerge.lcssa4517
  %gep4829.asptr = inttoptr i32 %gep4829 to i16*
  %82 = load i16* %gep4829.asptr, align 1
  %conv83.i3266 = zext i16 %82 to i32
  %add.i3267 = add i32 %mul.i3265, %conv83.i3266
  %conv9.i3268 = trunc i32 %add.i3267 to i16
  %gep4832.asptr = inttoptr i32 %gep4832 to i16*
  store i16 %conv9.i3268, i16* %gep4832.asptr, align 1
  %gep4835.asptr30 = inttoptr i32 %gep4835 to i16*
  %83 = load i16* %gep4835.asptr30, align 1
  %add12.i3269 = add i32 %storemerge216.i3263, 1
  %gep_array4837 = mul i32 %add12.i3269, 2
  %gep4838 = add i32 %fr, %gep_array4837
  %gep4838.asptr = inttoptr i32 %gep4838 to i16*
  store i16 %83, i16* %gep4838.asptr, align 1
  %cmp5.i3271 = icmp slt i32 %add12.i3269, %storemerge118.i3258
  br i1 %cmp5.i3271, label %for.body6.i3272, label %for.inc14.i3275

for.inc14.i3275:                                  ; preds = %for.body6.i3272, %for.cond4.preheader.i3260
  %inc15.i3273 = add i32 %storemerge118.i3258, 1
  %cmp2.i3274 = icmp slt i32 %inc15.i3273, %storemerge21.i3251
  br i1 %cmp2.i3274, label %for.cond4.preheader.i3260, label %for.inc17.i3278

for.inc17.i3278:                                  ; preds = %for.inc14.i3275, %for.cond1.preheader.i3253
  %inc18.i3276 = add i32 %storemerge21.i3251, 1
  %cmp.i3277 = icmp slt i32 %inc18.i3276, %storemerge.lcssa4517
  br i1 %cmp.i3277, label %for.cond1.preheader.i3253, label %for.cond1.preheader.i3223

for.cond1.preheader.i3223:                        ; preds = %for.inc17.i3278, %for.inc17.i3248
  %storemerge21.i3221 = phi i32 [ %inc18.i3246, %for.inc17.i3248 ], [ 0, %for.inc17.i3278 ]
  %cmp217.i3222 = icmp sgt i32 %storemerge21.i3221, 0
  br i1 %cmp217.i3222, label %for.cond4.preheader.lr.ph.i3227, label %for.inc17.i3248

for.cond4.preheader.lr.ph.i3227:                  ; preds = %for.cond1.preheader.i3223
  %sub.i3224 = add i32 %storemerge21.i3221, -2
  %gep_array4840 = mul i32 %sub.i3224, 2
  %gep4841 = add i32 %fr, %gep_array4840
  %gep_array4843 = mul i32 %storemerge21.i3221, 2
  %gep4844 = add i32 %fr, %gep_array4843
  br label %for.cond4.preheader.i3230

for.cond4.preheader.i3230:                        ; preds = %for.inc14.i3245, %for.cond4.preheader.lr.ph.i3227
  %storemerge118.i3228 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3227 ], [ %inc15.i3243, %for.inc14.i3245 ]
  %cmp515.i3229 = icmp sgt i32 %storemerge118.i3228, 0
  br i1 %cmp515.i3229, label %for.body6.lr.ph.i3232, label %for.inc14.i3245

for.body6.lr.ph.i3232:                            ; preds = %for.cond4.preheader.i3230
  %gep_array4846 = mul i32 %storemerge118.i3228, 2
  %gep4847 = add i32 %Sinewave, %gep_array4846
  br label %for.body6.i3242

for.body6.i3242:                                  ; preds = %for.body6.i3242, %for.body6.lr.ph.i3232
  %storemerge216.i3233 = phi i32 [ 0, %for.body6.lr.ph.i3232 ], [ %add12.i3239, %for.body6.i3242 ]
  %gep4847.asptr = inttoptr i32 %gep4847 to i16*
  %84 = load i16* %gep4847.asptr, align 1
  %conv.i3234 = sext i16 %84 to i32
  %mul.i3235 = mul i32 %conv.i3234, %storemerge.lcssa4517
  %gep4841.asptr = inttoptr i32 %gep4841 to i16*
  %85 = load i16* %gep4841.asptr, align 1
  %conv83.i3236 = zext i16 %85 to i32
  %add.i3237 = add i32 %mul.i3235, %conv83.i3236
  %conv9.i3238 = trunc i32 %add.i3237 to i16
  %gep4844.asptr = inttoptr i32 %gep4844 to i16*
  store i16 %conv9.i3238, i16* %gep4844.asptr, align 1
  %gep4847.asptr31 = inttoptr i32 %gep4847 to i16*
  %86 = load i16* %gep4847.asptr31, align 1
  %add12.i3239 = add i32 %storemerge216.i3233, 1
  %gep_array4849 = mul i32 %add12.i3239, 2
  %gep4850 = add i32 %fr, %gep_array4849
  %gep4850.asptr = inttoptr i32 %gep4850 to i16*
  store i16 %86, i16* %gep4850.asptr, align 1
  %cmp5.i3241 = icmp slt i32 %add12.i3239, %storemerge118.i3228
  br i1 %cmp5.i3241, label %for.body6.i3242, label %for.inc14.i3245

for.inc14.i3245:                                  ; preds = %for.body6.i3242, %for.cond4.preheader.i3230
  %inc15.i3243 = add i32 %storemerge118.i3228, 1
  %cmp2.i3244 = icmp slt i32 %inc15.i3243, %storemerge21.i3221
  br i1 %cmp2.i3244, label %for.cond4.preheader.i3230, label %for.inc17.i3248

for.inc17.i3248:                                  ; preds = %for.inc14.i3245, %for.cond1.preheader.i3223
  %inc18.i3246 = add i32 %storemerge21.i3221, 1
  %cmp.i3247 = icmp slt i32 %inc18.i3246, %storemerge.lcssa4517
  br i1 %cmp.i3247, label %for.cond1.preheader.i3223, label %for.cond1.preheader.i3193

for.cond1.preheader.i3193:                        ; preds = %for.inc17.i3248, %for.inc17.i3218
  %storemerge21.i3191 = phi i32 [ %inc18.i3216, %for.inc17.i3218 ], [ 0, %for.inc17.i3248 ]
  %cmp217.i3192 = icmp sgt i32 %storemerge21.i3191, 0
  br i1 %cmp217.i3192, label %for.cond4.preheader.lr.ph.i3197, label %for.inc17.i3218

for.cond4.preheader.lr.ph.i3197:                  ; preds = %for.cond1.preheader.i3193
  %sub.i3194 = add i32 %storemerge21.i3191, -2
  %gep_array4852 = mul i32 %sub.i3194, 2
  %gep4853 = add i32 %fr, %gep_array4852
  %gep_array4855 = mul i32 %storemerge21.i3191, 2
  %gep4856 = add i32 %fr, %gep_array4855
  br label %for.cond4.preheader.i3200

for.cond4.preheader.i3200:                        ; preds = %for.inc14.i3215, %for.cond4.preheader.lr.ph.i3197
  %storemerge118.i3198 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3197 ], [ %inc15.i3213, %for.inc14.i3215 ]
  %cmp515.i3199 = icmp sgt i32 %storemerge118.i3198, 0
  br i1 %cmp515.i3199, label %for.body6.lr.ph.i3202, label %for.inc14.i3215

for.body6.lr.ph.i3202:                            ; preds = %for.cond4.preheader.i3200
  %gep_array4858 = mul i32 %storemerge118.i3198, 2
  %gep4859 = add i32 %fi, %gep_array4858
  br label %for.body6.i3212

for.body6.i3212:                                  ; preds = %for.body6.i3212, %for.body6.lr.ph.i3202
  %storemerge216.i3203 = phi i32 [ 0, %for.body6.lr.ph.i3202 ], [ %add12.i3209, %for.body6.i3212 ]
  %gep4859.asptr = inttoptr i32 %gep4859 to i16*
  %87 = load i16* %gep4859.asptr, align 1
  %conv.i3204 = sext i16 %87 to i32
  %mul.i3205 = mul i32 %conv.i3204, %storemerge.lcssa4517
  %gep4853.asptr = inttoptr i32 %gep4853 to i16*
  %88 = load i16* %gep4853.asptr, align 1
  %conv83.i3206 = zext i16 %88 to i32
  %add.i3207 = add i32 %mul.i3205, %conv83.i3206
  %conv9.i3208 = trunc i32 %add.i3207 to i16
  %gep4856.asptr = inttoptr i32 %gep4856 to i16*
  store i16 %conv9.i3208, i16* %gep4856.asptr, align 1
  %gep4859.asptr32 = inttoptr i32 %gep4859 to i16*
  %89 = load i16* %gep4859.asptr32, align 1
  %add12.i3209 = add i32 %storemerge216.i3203, 1
  %gep_array4861 = mul i32 %add12.i3209, 2
  %gep4862 = add i32 %fr, %gep_array4861
  %gep4862.asptr = inttoptr i32 %gep4862 to i16*
  store i16 %89, i16* %gep4862.asptr, align 1
  %cmp5.i3211 = icmp slt i32 %add12.i3209, %storemerge118.i3198
  br i1 %cmp5.i3211, label %for.body6.i3212, label %for.inc14.i3215

for.inc14.i3215:                                  ; preds = %for.body6.i3212, %for.cond4.preheader.i3200
  %inc15.i3213 = add i32 %storemerge118.i3198, 1
  %cmp2.i3214 = icmp slt i32 %inc15.i3213, %storemerge21.i3191
  br i1 %cmp2.i3214, label %for.cond4.preheader.i3200, label %for.inc17.i3218

for.inc17.i3218:                                  ; preds = %for.inc14.i3215, %for.cond1.preheader.i3193
  %inc18.i3216 = add i32 %storemerge21.i3191, 1
  %cmp.i3217 = icmp slt i32 %inc18.i3216, %storemerge.lcssa4517
  br i1 %cmp.i3217, label %for.cond1.preheader.i3193, label %for.cond1.preheader.i3163

for.cond1.preheader.i3163:                        ; preds = %for.inc17.i3218, %for.inc17.i3188
  %storemerge21.i3161 = phi i32 [ %inc18.i3186, %for.inc17.i3188 ], [ 0, %for.inc17.i3218 ]
  %cmp217.i3162 = icmp sgt i32 %storemerge21.i3161, 0
  br i1 %cmp217.i3162, label %for.cond4.preheader.lr.ph.i3167, label %for.inc17.i3188

for.cond4.preheader.lr.ph.i3167:                  ; preds = %for.cond1.preheader.i3163
  %sub.i3164 = add i32 %storemerge21.i3161, -2
  %gep_array4864 = mul i32 %sub.i3164, 2
  %gep4865 = add i32 %fr, %gep_array4864
  %gep_array4867 = mul i32 %storemerge21.i3161, 2
  %gep4868 = add i32 %fr, %gep_array4867
  br label %for.cond4.preheader.i3170

for.cond4.preheader.i3170:                        ; preds = %for.inc14.i3185, %for.cond4.preheader.lr.ph.i3167
  %storemerge118.i3168 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3167 ], [ %inc15.i3183, %for.inc14.i3185 ]
  %cmp515.i3169 = icmp sgt i32 %storemerge118.i3168, 0
  br i1 %cmp515.i3169, label %for.body6.lr.ph.i3172, label %for.inc14.i3185

for.body6.lr.ph.i3172:                            ; preds = %for.cond4.preheader.i3170
  %gep_array4870 = mul i32 %storemerge118.i3168, 2
  %gep4871 = add i32 %Sinewave, %gep_array4870
  br label %for.body6.i3182

for.body6.i3182:                                  ; preds = %for.body6.i3182, %for.body6.lr.ph.i3172
  %storemerge216.i3173 = phi i32 [ 0, %for.body6.lr.ph.i3172 ], [ %add12.i3179, %for.body6.i3182 ]
  %gep4871.asptr = inttoptr i32 %gep4871 to i16*
  %90 = load i16* %gep4871.asptr, align 1
  %conv.i3174 = sext i16 %90 to i32
  %mul.i3175 = mul i32 %conv.i3174, %storemerge.lcssa4517
  %gep4865.asptr = inttoptr i32 %gep4865 to i16*
  %91 = load i16* %gep4865.asptr, align 1
  %conv83.i3176 = zext i16 %91 to i32
  %add.i3177 = add i32 %mul.i3175, %conv83.i3176
  %conv9.i3178 = trunc i32 %add.i3177 to i16
  %gep4868.asptr = inttoptr i32 %gep4868 to i16*
  store i16 %conv9.i3178, i16* %gep4868.asptr, align 1
  %gep4871.asptr33 = inttoptr i32 %gep4871 to i16*
  %92 = load i16* %gep4871.asptr33, align 1
  %add12.i3179 = add i32 %storemerge216.i3173, 1
  %gep_array4873 = mul i32 %add12.i3179, 2
  %gep4874 = add i32 %fr, %gep_array4873
  %gep4874.asptr = inttoptr i32 %gep4874 to i16*
  store i16 %92, i16* %gep4874.asptr, align 1
  %cmp5.i3181 = icmp slt i32 %add12.i3179, %storemerge118.i3168
  br i1 %cmp5.i3181, label %for.body6.i3182, label %for.inc14.i3185

for.inc14.i3185:                                  ; preds = %for.body6.i3182, %for.cond4.preheader.i3170
  %inc15.i3183 = add i32 %storemerge118.i3168, 1
  %cmp2.i3184 = icmp slt i32 %inc15.i3183, %storemerge21.i3161
  br i1 %cmp2.i3184, label %for.cond4.preheader.i3170, label %for.inc17.i3188

for.inc17.i3188:                                  ; preds = %for.inc14.i3185, %for.cond1.preheader.i3163
  %inc18.i3186 = add i32 %storemerge21.i3161, 1
  %cmp.i3187 = icmp slt i32 %inc18.i3186, %storemerge.lcssa4517
  br i1 %cmp.i3187, label %for.cond1.preheader.i3163, label %for.cond1.preheader.i3133

for.cond1.preheader.i3133:                        ; preds = %for.inc17.i3188, %for.inc17.i3158
  %storemerge21.i3131 = phi i32 [ %inc18.i3156, %for.inc17.i3158 ], [ 0, %for.inc17.i3188 ]
  %cmp217.i3132 = icmp sgt i32 %storemerge21.i3131, 0
  br i1 %cmp217.i3132, label %for.cond4.preheader.lr.ph.i3137, label %for.inc17.i3158

for.cond4.preheader.lr.ph.i3137:                  ; preds = %for.cond1.preheader.i3133
  %sub.i3134 = add i32 %storemerge21.i3131, -2
  %gep_array4876 = mul i32 %sub.i3134, 2
  %gep4877 = add i32 %fr, %gep_array4876
  %gep_array4879 = mul i32 %storemerge21.i3131, 2
  %gep4880 = add i32 %fr, %gep_array4879
  br label %for.cond4.preheader.i3140

for.cond4.preheader.i3140:                        ; preds = %for.inc14.i3155, %for.cond4.preheader.lr.ph.i3137
  %storemerge118.i3138 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3137 ], [ %inc15.i3153, %for.inc14.i3155 ]
  %cmp515.i3139 = icmp sgt i32 %storemerge118.i3138, 0
  br i1 %cmp515.i3139, label %for.body6.lr.ph.i3142, label %for.inc14.i3155

for.body6.lr.ph.i3142:                            ; preds = %for.cond4.preheader.i3140
  %gep_array4882 = mul i32 %storemerge118.i3138, 2
  %gep4883 = add i32 %fi, %gep_array4882
  br label %for.body6.i3152

for.body6.i3152:                                  ; preds = %for.body6.i3152, %for.body6.lr.ph.i3142
  %storemerge216.i3143 = phi i32 [ 0, %for.body6.lr.ph.i3142 ], [ %add12.i3149, %for.body6.i3152 ]
  %gep4883.asptr = inttoptr i32 %gep4883 to i16*
  %93 = load i16* %gep4883.asptr, align 1
  %conv.i3144 = sext i16 %93 to i32
  %mul.i3145 = mul i32 %conv.i3144, %storemerge.lcssa4517
  %gep4877.asptr = inttoptr i32 %gep4877 to i16*
  %94 = load i16* %gep4877.asptr, align 1
  %conv83.i3146 = zext i16 %94 to i32
  %add.i3147 = add i32 %mul.i3145, %conv83.i3146
  %conv9.i3148 = trunc i32 %add.i3147 to i16
  %gep4880.asptr = inttoptr i32 %gep4880 to i16*
  store i16 %conv9.i3148, i16* %gep4880.asptr, align 1
  %gep4883.asptr34 = inttoptr i32 %gep4883 to i16*
  %95 = load i16* %gep4883.asptr34, align 1
  %add12.i3149 = add i32 %storemerge216.i3143, 1
  %gep_array4885 = mul i32 %add12.i3149, 2
  %gep4886 = add i32 %fr, %gep_array4885
  %gep4886.asptr = inttoptr i32 %gep4886 to i16*
  store i16 %95, i16* %gep4886.asptr, align 1
  %cmp5.i3151 = icmp slt i32 %add12.i3149, %storemerge118.i3138
  br i1 %cmp5.i3151, label %for.body6.i3152, label %for.inc14.i3155

for.inc14.i3155:                                  ; preds = %for.body6.i3152, %for.cond4.preheader.i3140
  %inc15.i3153 = add i32 %storemerge118.i3138, 1
  %cmp2.i3154 = icmp slt i32 %inc15.i3153, %storemerge21.i3131
  br i1 %cmp2.i3154, label %for.cond4.preheader.i3140, label %for.inc17.i3158

for.inc17.i3158:                                  ; preds = %for.inc14.i3155, %for.cond1.preheader.i3133
  %inc18.i3156 = add i32 %storemerge21.i3131, 1
  %cmp.i3157 = icmp slt i32 %inc18.i3156, %storemerge.lcssa4517
  br i1 %cmp.i3157, label %for.cond1.preheader.i3133, label %for.cond1.preheader.i3103

for.cond1.preheader.i3103:                        ; preds = %for.inc17.i3158, %for.inc17.i3128
  %storemerge21.i3101 = phi i32 [ %inc18.i3126, %for.inc17.i3128 ], [ 0, %for.inc17.i3158 ]
  %cmp217.i3102 = icmp sgt i32 %storemerge21.i3101, 0
  br i1 %cmp217.i3102, label %for.cond4.preheader.lr.ph.i3107, label %for.inc17.i3128

for.cond4.preheader.lr.ph.i3107:                  ; preds = %for.cond1.preheader.i3103
  %sub.i3104 = add i32 %storemerge21.i3101, -2
  %gep_array4888 = mul i32 %sub.i3104, 2
  %gep4889 = add i32 %fr, %gep_array4888
  %gep_array4891 = mul i32 %storemerge21.i3101, 2
  %gep4892 = add i32 %fr, %gep_array4891
  br label %for.cond4.preheader.i3110

for.cond4.preheader.i3110:                        ; preds = %for.inc14.i3125, %for.cond4.preheader.lr.ph.i3107
  %storemerge118.i3108 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3107 ], [ %inc15.i3123, %for.inc14.i3125 ]
  %cmp515.i3109 = icmp sgt i32 %storemerge118.i3108, 0
  br i1 %cmp515.i3109, label %for.body6.lr.ph.i3112, label %for.inc14.i3125

for.body6.lr.ph.i3112:                            ; preds = %for.cond4.preheader.i3110
  %gep_array4894 = mul i32 %storemerge118.i3108, 2
  %gep4895 = add i32 %Sinewave, %gep_array4894
  br label %for.body6.i3122

for.body6.i3122:                                  ; preds = %for.body6.i3122, %for.body6.lr.ph.i3112
  %storemerge216.i3113 = phi i32 [ 0, %for.body6.lr.ph.i3112 ], [ %add12.i3119, %for.body6.i3122 ]
  %gep4895.asptr = inttoptr i32 %gep4895 to i16*
  %96 = load i16* %gep4895.asptr, align 1
  %conv.i3114 = sext i16 %96 to i32
  %mul.i3115 = mul i32 %conv.i3114, %storemerge.lcssa4517
  %gep4889.asptr = inttoptr i32 %gep4889 to i16*
  %97 = load i16* %gep4889.asptr, align 1
  %conv83.i3116 = zext i16 %97 to i32
  %add.i3117 = add i32 %mul.i3115, %conv83.i3116
  %conv9.i3118 = trunc i32 %add.i3117 to i16
  %gep4892.asptr = inttoptr i32 %gep4892 to i16*
  store i16 %conv9.i3118, i16* %gep4892.asptr, align 1
  %gep4895.asptr35 = inttoptr i32 %gep4895 to i16*
  %98 = load i16* %gep4895.asptr35, align 1
  %add12.i3119 = add i32 %storemerge216.i3113, 1
  %gep_array4897 = mul i32 %add12.i3119, 2
  %gep4898 = add i32 %fr, %gep_array4897
  %gep4898.asptr = inttoptr i32 %gep4898 to i16*
  store i16 %98, i16* %gep4898.asptr, align 1
  %cmp5.i3121 = icmp slt i32 %add12.i3119, %storemerge118.i3108
  br i1 %cmp5.i3121, label %for.body6.i3122, label %for.inc14.i3125

for.inc14.i3125:                                  ; preds = %for.body6.i3122, %for.cond4.preheader.i3110
  %inc15.i3123 = add i32 %storemerge118.i3108, 1
  %cmp2.i3124 = icmp slt i32 %inc15.i3123, %storemerge21.i3101
  br i1 %cmp2.i3124, label %for.cond4.preheader.i3110, label %for.inc17.i3128

for.inc17.i3128:                                  ; preds = %for.inc14.i3125, %for.cond1.preheader.i3103
  %inc18.i3126 = add i32 %storemerge21.i3101, 1
  %cmp.i3127 = icmp slt i32 %inc18.i3126, %storemerge.lcssa4517
  br i1 %cmp.i3127, label %for.cond1.preheader.i3103, label %for.cond1.preheader.i3073

for.cond1.preheader.i3073:                        ; preds = %for.inc17.i3128, %for.inc17.i3098
  %storemerge21.i3071 = phi i32 [ %inc18.i3096, %for.inc17.i3098 ], [ 0, %for.inc17.i3128 ]
  %cmp217.i3072 = icmp sgt i32 %storemerge21.i3071, 0
  br i1 %cmp217.i3072, label %for.cond4.preheader.lr.ph.i3077, label %for.inc17.i3098

for.cond4.preheader.lr.ph.i3077:                  ; preds = %for.cond1.preheader.i3073
  %sub.i3074 = add i32 %storemerge21.i3071, -2
  %gep_array4900 = mul i32 %sub.i3074, 2
  %gep4901 = add i32 %fr, %gep_array4900
  %gep_array4903 = mul i32 %storemerge21.i3071, 2
  %gep4904 = add i32 %fr, %gep_array4903
  br label %for.cond4.preheader.i3080

for.cond4.preheader.i3080:                        ; preds = %for.inc14.i3095, %for.cond4.preheader.lr.ph.i3077
  %storemerge118.i3078 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3077 ], [ %inc15.i3093, %for.inc14.i3095 ]
  %cmp515.i3079 = icmp sgt i32 %storemerge118.i3078, 0
  br i1 %cmp515.i3079, label %for.body6.lr.ph.i3082, label %for.inc14.i3095

for.body6.lr.ph.i3082:                            ; preds = %for.cond4.preheader.i3080
  %gep_array4906 = mul i32 %storemerge118.i3078, 2
  %gep4907 = add i32 %Sinewave, %gep_array4906
  br label %for.body6.i3092

for.body6.i3092:                                  ; preds = %for.body6.i3092, %for.body6.lr.ph.i3082
  %storemerge216.i3083 = phi i32 [ 0, %for.body6.lr.ph.i3082 ], [ %add12.i3089, %for.body6.i3092 ]
  %gep4907.asptr = inttoptr i32 %gep4907 to i16*
  %99 = load i16* %gep4907.asptr, align 1
  %conv.i3084 = sext i16 %99 to i32
  %mul.i3085 = mul i32 %conv.i3084, %storemerge.lcssa4517
  %gep4901.asptr = inttoptr i32 %gep4901 to i16*
  %100 = load i16* %gep4901.asptr, align 1
  %conv83.i3086 = zext i16 %100 to i32
  %add.i3087 = add i32 %mul.i3085, %conv83.i3086
  %conv9.i3088 = trunc i32 %add.i3087 to i16
  %gep4904.asptr = inttoptr i32 %gep4904 to i16*
  store i16 %conv9.i3088, i16* %gep4904.asptr, align 1
  %gep4907.asptr36 = inttoptr i32 %gep4907 to i16*
  %101 = load i16* %gep4907.asptr36, align 1
  %add12.i3089 = add i32 %storemerge216.i3083, 1
  %gep_array4909 = mul i32 %add12.i3089, 2
  %gep4910 = add i32 %fr, %gep_array4909
  %gep4910.asptr = inttoptr i32 %gep4910 to i16*
  store i16 %101, i16* %gep4910.asptr, align 1
  %cmp5.i3091 = icmp slt i32 %add12.i3089, %storemerge118.i3078
  br i1 %cmp5.i3091, label %for.body6.i3092, label %for.inc14.i3095

for.inc14.i3095:                                  ; preds = %for.body6.i3092, %for.cond4.preheader.i3080
  %inc15.i3093 = add i32 %storemerge118.i3078, 1
  %cmp2.i3094 = icmp slt i32 %inc15.i3093, %storemerge21.i3071
  br i1 %cmp2.i3094, label %for.cond4.preheader.i3080, label %for.inc17.i3098

for.inc17.i3098:                                  ; preds = %for.inc14.i3095, %for.cond1.preheader.i3073
  %inc18.i3096 = add i32 %storemerge21.i3071, 1
  %cmp.i3097 = icmp slt i32 %inc18.i3096, %storemerge.lcssa4517
  br i1 %cmp.i3097, label %for.cond1.preheader.i3073, label %for.cond1.preheader.i3043

for.cond1.preheader.i3043:                        ; preds = %for.inc17.i3098, %for.inc17.i3068
  %storemerge21.i3041 = phi i32 [ %inc18.i3066, %for.inc17.i3068 ], [ 0, %for.inc17.i3098 ]
  %cmp217.i3042 = icmp sgt i32 %storemerge21.i3041, 0
  br i1 %cmp217.i3042, label %for.cond4.preheader.lr.ph.i3047, label %for.inc17.i3068

for.cond4.preheader.lr.ph.i3047:                  ; preds = %for.cond1.preheader.i3043
  %sub.i3044 = add i32 %storemerge21.i3041, -2
  %gep_array4912 = mul i32 %sub.i3044, 2
  %gep4913 = add i32 %fr, %gep_array4912
  %gep_array4915 = mul i32 %storemerge21.i3041, 2
  %gep4916 = add i32 %fr, %gep_array4915
  br label %for.cond4.preheader.i3050

for.cond4.preheader.i3050:                        ; preds = %for.inc14.i3065, %for.cond4.preheader.lr.ph.i3047
  %storemerge118.i3048 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3047 ], [ %inc15.i3063, %for.inc14.i3065 ]
  %cmp515.i3049 = icmp sgt i32 %storemerge118.i3048, 0
  br i1 %cmp515.i3049, label %for.body6.lr.ph.i3052, label %for.inc14.i3065

for.body6.lr.ph.i3052:                            ; preds = %for.cond4.preheader.i3050
  %gep_array4918 = mul i32 %storemerge118.i3048, 2
  %gep4919 = add i32 %fi, %gep_array4918
  br label %for.body6.i3062

for.body6.i3062:                                  ; preds = %for.body6.i3062, %for.body6.lr.ph.i3052
  %storemerge216.i3053 = phi i32 [ 0, %for.body6.lr.ph.i3052 ], [ %add12.i3059, %for.body6.i3062 ]
  %gep4919.asptr = inttoptr i32 %gep4919 to i16*
  %102 = load i16* %gep4919.asptr, align 1
  %conv.i3054 = sext i16 %102 to i32
  %mul.i3055 = mul i32 %conv.i3054, %storemerge.lcssa4517
  %gep4913.asptr = inttoptr i32 %gep4913 to i16*
  %103 = load i16* %gep4913.asptr, align 1
  %conv83.i3056 = zext i16 %103 to i32
  %add.i3057 = add i32 %mul.i3055, %conv83.i3056
  %conv9.i3058 = trunc i32 %add.i3057 to i16
  %gep4916.asptr = inttoptr i32 %gep4916 to i16*
  store i16 %conv9.i3058, i16* %gep4916.asptr, align 1
  %gep4919.asptr37 = inttoptr i32 %gep4919 to i16*
  %104 = load i16* %gep4919.asptr37, align 1
  %add12.i3059 = add i32 %storemerge216.i3053, 1
  %gep_array4921 = mul i32 %add12.i3059, 2
  %gep4922 = add i32 %fr, %gep_array4921
  %gep4922.asptr = inttoptr i32 %gep4922 to i16*
  store i16 %104, i16* %gep4922.asptr, align 1
  %cmp5.i3061 = icmp slt i32 %add12.i3059, %storemerge118.i3048
  br i1 %cmp5.i3061, label %for.body6.i3062, label %for.inc14.i3065

for.inc14.i3065:                                  ; preds = %for.body6.i3062, %for.cond4.preheader.i3050
  %inc15.i3063 = add i32 %storemerge118.i3048, 1
  %cmp2.i3064 = icmp slt i32 %inc15.i3063, %storemerge21.i3041
  br i1 %cmp2.i3064, label %for.cond4.preheader.i3050, label %for.inc17.i3068

for.inc17.i3068:                                  ; preds = %for.inc14.i3065, %for.cond1.preheader.i3043
  %inc18.i3066 = add i32 %storemerge21.i3041, 1
  %cmp.i3067 = icmp slt i32 %inc18.i3066, %storemerge.lcssa4517
  br i1 %cmp.i3067, label %for.cond1.preheader.i3043, label %for.cond1.preheader.i3013

for.cond1.preheader.i3013:                        ; preds = %for.inc17.i3068, %for.inc17.i3038
  %storemerge21.i3011 = phi i32 [ %inc18.i3036, %for.inc17.i3038 ], [ 0, %for.inc17.i3068 ]
  %cmp217.i3012 = icmp sgt i32 %storemerge21.i3011, 0
  br i1 %cmp217.i3012, label %for.cond4.preheader.lr.ph.i3017, label %for.inc17.i3038

for.cond4.preheader.lr.ph.i3017:                  ; preds = %for.cond1.preheader.i3013
  %sub.i3014 = add i32 %storemerge21.i3011, -2
  %gep_array4924 = mul i32 %sub.i3014, 2
  %gep4925 = add i32 %fr, %gep_array4924
  %gep_array4927 = mul i32 %storemerge21.i3011, 2
  %gep4928 = add i32 %fr, %gep_array4927
  br label %for.cond4.preheader.i3020

for.cond4.preheader.i3020:                        ; preds = %for.inc14.i3035, %for.cond4.preheader.lr.ph.i3017
  %storemerge118.i3018 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i3017 ], [ %inc15.i3033, %for.inc14.i3035 ]
  %cmp515.i3019 = icmp sgt i32 %storemerge118.i3018, 0
  br i1 %cmp515.i3019, label %for.body6.lr.ph.i3022, label %for.inc14.i3035

for.body6.lr.ph.i3022:                            ; preds = %for.cond4.preheader.i3020
  %gep_array4930 = mul i32 %storemerge118.i3018, 2
  %gep4931 = add i32 %Sinewave, %gep_array4930
  br label %for.body6.i3032

for.body6.i3032:                                  ; preds = %for.body6.i3032, %for.body6.lr.ph.i3022
  %storemerge216.i3023 = phi i32 [ 0, %for.body6.lr.ph.i3022 ], [ %add12.i3029, %for.body6.i3032 ]
  %gep4931.asptr = inttoptr i32 %gep4931 to i16*
  %105 = load i16* %gep4931.asptr, align 1
  %conv.i3024 = sext i16 %105 to i32
  %mul.i3025 = mul i32 %conv.i3024, %storemerge.lcssa4517
  %gep4925.asptr = inttoptr i32 %gep4925 to i16*
  %106 = load i16* %gep4925.asptr, align 1
  %conv83.i3026 = zext i16 %106 to i32
  %add.i3027 = add i32 %mul.i3025, %conv83.i3026
  %conv9.i3028 = trunc i32 %add.i3027 to i16
  %gep4928.asptr = inttoptr i32 %gep4928 to i16*
  store i16 %conv9.i3028, i16* %gep4928.asptr, align 1
  %gep4931.asptr38 = inttoptr i32 %gep4931 to i16*
  %107 = load i16* %gep4931.asptr38, align 1
  %add12.i3029 = add i32 %storemerge216.i3023, 1
  %gep_array4933 = mul i32 %add12.i3029, 2
  %gep4934 = add i32 %fr, %gep_array4933
  %gep4934.asptr = inttoptr i32 %gep4934 to i16*
  store i16 %107, i16* %gep4934.asptr, align 1
  %cmp5.i3031 = icmp slt i32 %add12.i3029, %storemerge118.i3018
  br i1 %cmp5.i3031, label %for.body6.i3032, label %for.inc14.i3035

for.inc14.i3035:                                  ; preds = %for.body6.i3032, %for.cond4.preheader.i3020
  %inc15.i3033 = add i32 %storemerge118.i3018, 1
  %cmp2.i3034 = icmp slt i32 %inc15.i3033, %storemerge21.i3011
  br i1 %cmp2.i3034, label %for.cond4.preheader.i3020, label %for.inc17.i3038

for.inc17.i3038:                                  ; preds = %for.inc14.i3035, %for.cond1.preheader.i3013
  %inc18.i3036 = add i32 %storemerge21.i3011, 1
  %cmp.i3037 = icmp slt i32 %inc18.i3036, %storemerge.lcssa4517
  br i1 %cmp.i3037, label %for.cond1.preheader.i3013, label %for.cond1.preheader.i2983

for.cond1.preheader.i2983:                        ; preds = %for.inc17.i3038, %for.inc17.i3008
  %storemerge21.i2981 = phi i32 [ %inc18.i3006, %for.inc17.i3008 ], [ 0, %for.inc17.i3038 ]
  %cmp217.i2982 = icmp sgt i32 %storemerge21.i2981, 0
  br i1 %cmp217.i2982, label %for.cond4.preheader.lr.ph.i2987, label %for.inc17.i3008

for.cond4.preheader.lr.ph.i2987:                  ; preds = %for.cond1.preheader.i2983
  %sub.i2984 = add i32 %storemerge21.i2981, -2
  %gep_array4936 = mul i32 %sub.i2984, 2
  %gep4937 = add i32 %fr, %gep_array4936
  %gep_array4939 = mul i32 %storemerge21.i2981, 2
  %gep4940 = add i32 %fr, %gep_array4939
  br label %for.cond4.preheader.i2990

for.cond4.preheader.i2990:                        ; preds = %for.inc14.i3005, %for.cond4.preheader.lr.ph.i2987
  %storemerge118.i2988 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2987 ], [ %inc15.i3003, %for.inc14.i3005 ]
  %cmp515.i2989 = icmp sgt i32 %storemerge118.i2988, 0
  br i1 %cmp515.i2989, label %for.body6.lr.ph.i2992, label %for.inc14.i3005

for.body6.lr.ph.i2992:                            ; preds = %for.cond4.preheader.i2990
  %gep_array4942 = mul i32 %storemerge118.i2988, 2
  %gep4943 = add i32 %fi, %gep_array4942
  br label %for.body6.i3002

for.body6.i3002:                                  ; preds = %for.body6.i3002, %for.body6.lr.ph.i2992
  %storemerge216.i2993 = phi i32 [ 0, %for.body6.lr.ph.i2992 ], [ %add12.i2999, %for.body6.i3002 ]
  %gep4943.asptr = inttoptr i32 %gep4943 to i16*
  %108 = load i16* %gep4943.asptr, align 1
  %conv.i2994 = sext i16 %108 to i32
  %mul.i2995 = mul i32 %conv.i2994, %storemerge.lcssa4517
  %gep4937.asptr = inttoptr i32 %gep4937 to i16*
  %109 = load i16* %gep4937.asptr, align 1
  %conv83.i2996 = zext i16 %109 to i32
  %add.i2997 = add i32 %mul.i2995, %conv83.i2996
  %conv9.i2998 = trunc i32 %add.i2997 to i16
  %gep4940.asptr = inttoptr i32 %gep4940 to i16*
  store i16 %conv9.i2998, i16* %gep4940.asptr, align 1
  %gep4943.asptr39 = inttoptr i32 %gep4943 to i16*
  %110 = load i16* %gep4943.asptr39, align 1
  %add12.i2999 = add i32 %storemerge216.i2993, 1
  %gep_array4945 = mul i32 %add12.i2999, 2
  %gep4946 = add i32 %fr, %gep_array4945
  %gep4946.asptr = inttoptr i32 %gep4946 to i16*
  store i16 %110, i16* %gep4946.asptr, align 1
  %cmp5.i3001 = icmp slt i32 %add12.i2999, %storemerge118.i2988
  br i1 %cmp5.i3001, label %for.body6.i3002, label %for.inc14.i3005

for.inc14.i3005:                                  ; preds = %for.body6.i3002, %for.cond4.preheader.i2990
  %inc15.i3003 = add i32 %storemerge118.i2988, 1
  %cmp2.i3004 = icmp slt i32 %inc15.i3003, %storemerge21.i2981
  br i1 %cmp2.i3004, label %for.cond4.preheader.i2990, label %for.inc17.i3008

for.inc17.i3008:                                  ; preds = %for.inc14.i3005, %for.cond1.preheader.i2983
  %inc18.i3006 = add i32 %storemerge21.i2981, 1
  %cmp.i3007 = icmp slt i32 %inc18.i3006, %storemerge.lcssa4517
  br i1 %cmp.i3007, label %for.cond1.preheader.i2983, label %for.cond1.preheader.i2953

for.cond1.preheader.i2953:                        ; preds = %for.inc17.i3008, %for.inc17.i2978
  %storemerge21.i2951 = phi i32 [ %inc18.i2976, %for.inc17.i2978 ], [ 0, %for.inc17.i3008 ]
  %cmp217.i2952 = icmp sgt i32 %storemerge21.i2951, 0
  br i1 %cmp217.i2952, label %for.cond4.preheader.lr.ph.i2957, label %for.inc17.i2978

for.cond4.preheader.lr.ph.i2957:                  ; preds = %for.cond1.preheader.i2953
  %sub.i2954 = add i32 %storemerge21.i2951, -2
  %gep_array4948 = mul i32 %sub.i2954, 2
  %gep4949 = add i32 %fr, %gep_array4948
  %gep_array4951 = mul i32 %storemerge21.i2951, 2
  %gep4952 = add i32 %fr, %gep_array4951
  br label %for.cond4.preheader.i2960

for.cond4.preheader.i2960:                        ; preds = %for.inc14.i2975, %for.cond4.preheader.lr.ph.i2957
  %storemerge118.i2958 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2957 ], [ %inc15.i2973, %for.inc14.i2975 ]
  %cmp515.i2959 = icmp sgt i32 %storemerge118.i2958, 0
  br i1 %cmp515.i2959, label %for.body6.lr.ph.i2962, label %for.inc14.i2975

for.body6.lr.ph.i2962:                            ; preds = %for.cond4.preheader.i2960
  %gep_array4954 = mul i32 %storemerge118.i2958, 2
  %gep4955 = add i32 %Sinewave, %gep_array4954
  br label %for.body6.i2972

for.body6.i2972:                                  ; preds = %for.body6.i2972, %for.body6.lr.ph.i2962
  %storemerge216.i2963 = phi i32 [ 0, %for.body6.lr.ph.i2962 ], [ %add12.i2969, %for.body6.i2972 ]
  %gep4955.asptr = inttoptr i32 %gep4955 to i16*
  %111 = load i16* %gep4955.asptr, align 1
  %conv.i2964 = sext i16 %111 to i32
  %mul.i2965 = mul i32 %conv.i2964, %storemerge.lcssa4517
  %gep4949.asptr = inttoptr i32 %gep4949 to i16*
  %112 = load i16* %gep4949.asptr, align 1
  %conv83.i2966 = zext i16 %112 to i32
  %add.i2967 = add i32 %mul.i2965, %conv83.i2966
  %conv9.i2968 = trunc i32 %add.i2967 to i16
  %gep4952.asptr = inttoptr i32 %gep4952 to i16*
  store i16 %conv9.i2968, i16* %gep4952.asptr, align 1
  %gep4955.asptr40 = inttoptr i32 %gep4955 to i16*
  %113 = load i16* %gep4955.asptr40, align 1
  %add12.i2969 = add i32 %storemerge216.i2963, 1
  %gep_array4957 = mul i32 %add12.i2969, 2
  %gep4958 = add i32 %fr, %gep_array4957
  %gep4958.asptr = inttoptr i32 %gep4958 to i16*
  store i16 %113, i16* %gep4958.asptr, align 1
  %cmp5.i2971 = icmp slt i32 %add12.i2969, %storemerge118.i2958
  br i1 %cmp5.i2971, label %for.body6.i2972, label %for.inc14.i2975

for.inc14.i2975:                                  ; preds = %for.body6.i2972, %for.cond4.preheader.i2960
  %inc15.i2973 = add i32 %storemerge118.i2958, 1
  %cmp2.i2974 = icmp slt i32 %inc15.i2973, %storemerge21.i2951
  br i1 %cmp2.i2974, label %for.cond4.preheader.i2960, label %for.inc17.i2978

for.inc17.i2978:                                  ; preds = %for.inc14.i2975, %for.cond1.preheader.i2953
  %inc18.i2976 = add i32 %storemerge21.i2951, 1
  %cmp.i2977 = icmp slt i32 %inc18.i2976, %storemerge.lcssa4517
  br i1 %cmp.i2977, label %for.cond1.preheader.i2953, label %for.cond1.preheader.i2923

for.cond1.preheader.i2923:                        ; preds = %for.inc17.i2978, %for.inc17.i2948
  %storemerge21.i2921 = phi i32 [ %inc18.i2946, %for.inc17.i2948 ], [ 0, %for.inc17.i2978 ]
  %cmp217.i2922 = icmp sgt i32 %storemerge21.i2921, 0
  br i1 %cmp217.i2922, label %for.cond4.preheader.lr.ph.i2927, label %for.inc17.i2948

for.cond4.preheader.lr.ph.i2927:                  ; preds = %for.cond1.preheader.i2923
  %sub.i2924 = add i32 %storemerge21.i2921, -2
  %gep_array4960 = mul i32 %sub.i2924, 2
  %gep4961 = add i32 %fr, %gep_array4960
  %gep_array4963 = mul i32 %storemerge21.i2921, 2
  %gep4964 = add i32 %fr, %gep_array4963
  br label %for.cond4.preheader.i2930

for.cond4.preheader.i2930:                        ; preds = %for.inc14.i2945, %for.cond4.preheader.lr.ph.i2927
  %storemerge118.i2928 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2927 ], [ %inc15.i2943, %for.inc14.i2945 ]
  %cmp515.i2929 = icmp sgt i32 %storemerge118.i2928, 0
  br i1 %cmp515.i2929, label %for.body6.lr.ph.i2932, label %for.inc14.i2945

for.body6.lr.ph.i2932:                            ; preds = %for.cond4.preheader.i2930
  %gep_array4966 = mul i32 %storemerge118.i2928, 2
  %gep4967 = add i32 %fi, %gep_array4966
  br label %for.body6.i2942

for.body6.i2942:                                  ; preds = %for.body6.i2942, %for.body6.lr.ph.i2932
  %storemerge216.i2933 = phi i32 [ 0, %for.body6.lr.ph.i2932 ], [ %add12.i2939, %for.body6.i2942 ]
  %gep4967.asptr = inttoptr i32 %gep4967 to i16*
  %114 = load i16* %gep4967.asptr, align 1
  %conv.i2934 = sext i16 %114 to i32
  %mul.i2935 = mul i32 %conv.i2934, %storemerge.lcssa4517
  %gep4961.asptr = inttoptr i32 %gep4961 to i16*
  %115 = load i16* %gep4961.asptr, align 1
  %conv83.i2936 = zext i16 %115 to i32
  %add.i2937 = add i32 %mul.i2935, %conv83.i2936
  %conv9.i2938 = trunc i32 %add.i2937 to i16
  %gep4964.asptr = inttoptr i32 %gep4964 to i16*
  store i16 %conv9.i2938, i16* %gep4964.asptr, align 1
  %gep4967.asptr41 = inttoptr i32 %gep4967 to i16*
  %116 = load i16* %gep4967.asptr41, align 1
  %add12.i2939 = add i32 %storemerge216.i2933, 1
  %gep_array4969 = mul i32 %add12.i2939, 2
  %gep4970 = add i32 %fr, %gep_array4969
  %gep4970.asptr = inttoptr i32 %gep4970 to i16*
  store i16 %116, i16* %gep4970.asptr, align 1
  %cmp5.i2941 = icmp slt i32 %add12.i2939, %storemerge118.i2928
  br i1 %cmp5.i2941, label %for.body6.i2942, label %for.inc14.i2945

for.inc14.i2945:                                  ; preds = %for.body6.i2942, %for.cond4.preheader.i2930
  %inc15.i2943 = add i32 %storemerge118.i2928, 1
  %cmp2.i2944 = icmp slt i32 %inc15.i2943, %storemerge21.i2921
  br i1 %cmp2.i2944, label %for.cond4.preheader.i2930, label %for.inc17.i2948

for.inc17.i2948:                                  ; preds = %for.inc14.i2945, %for.cond1.preheader.i2923
  %inc18.i2946 = add i32 %storemerge21.i2921, 1
  %cmp.i2947 = icmp slt i32 %inc18.i2946, %storemerge.lcssa4517
  br i1 %cmp.i2947, label %for.cond1.preheader.i2923, label %for.cond1.preheader.i2893

for.cond1.preheader.i2893:                        ; preds = %for.inc17.i2948, %for.inc17.i2918
  %storemerge21.i2891 = phi i32 [ %inc18.i2916, %for.inc17.i2918 ], [ 0, %for.inc17.i2948 ]
  %cmp217.i2892 = icmp sgt i32 %storemerge21.i2891, 0
  br i1 %cmp217.i2892, label %for.cond4.preheader.lr.ph.i2897, label %for.inc17.i2918

for.cond4.preheader.lr.ph.i2897:                  ; preds = %for.cond1.preheader.i2893
  %sub.i2894 = add i32 %storemerge21.i2891, -2
  %gep_array4972 = mul i32 %sub.i2894, 2
  %gep4973 = add i32 %fr, %gep_array4972
  %gep_array4975 = mul i32 %storemerge21.i2891, 2
  %gep4976 = add i32 %fr, %gep_array4975
  br label %for.cond4.preheader.i2900

for.cond4.preheader.i2900:                        ; preds = %for.inc14.i2915, %for.cond4.preheader.lr.ph.i2897
  %storemerge118.i2898 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2897 ], [ %inc15.i2913, %for.inc14.i2915 ]
  %cmp515.i2899 = icmp sgt i32 %storemerge118.i2898, 0
  br i1 %cmp515.i2899, label %for.body6.lr.ph.i2902, label %for.inc14.i2915

for.body6.lr.ph.i2902:                            ; preds = %for.cond4.preheader.i2900
  %gep_array4978 = mul i32 %storemerge118.i2898, 2
  %gep4979 = add i32 %Sinewave, %gep_array4978
  br label %for.body6.i2912

for.body6.i2912:                                  ; preds = %for.body6.i2912, %for.body6.lr.ph.i2902
  %storemerge216.i2903 = phi i32 [ 0, %for.body6.lr.ph.i2902 ], [ %add12.i2909, %for.body6.i2912 ]
  %gep4979.asptr = inttoptr i32 %gep4979 to i16*
  %117 = load i16* %gep4979.asptr, align 1
  %conv.i2904 = sext i16 %117 to i32
  %mul.i2905 = mul i32 %conv.i2904, %storemerge.lcssa4517
  %gep4973.asptr = inttoptr i32 %gep4973 to i16*
  %118 = load i16* %gep4973.asptr, align 1
  %conv83.i2906 = zext i16 %118 to i32
  %add.i2907 = add i32 %mul.i2905, %conv83.i2906
  %conv9.i2908 = trunc i32 %add.i2907 to i16
  %gep4976.asptr = inttoptr i32 %gep4976 to i16*
  store i16 %conv9.i2908, i16* %gep4976.asptr, align 1
  %gep4979.asptr42 = inttoptr i32 %gep4979 to i16*
  %119 = load i16* %gep4979.asptr42, align 1
  %add12.i2909 = add i32 %storemerge216.i2903, 1
  %gep_array4981 = mul i32 %add12.i2909, 2
  %gep4982 = add i32 %fr, %gep_array4981
  %gep4982.asptr = inttoptr i32 %gep4982 to i16*
  store i16 %119, i16* %gep4982.asptr, align 1
  %cmp5.i2911 = icmp slt i32 %add12.i2909, %storemerge118.i2898
  br i1 %cmp5.i2911, label %for.body6.i2912, label %for.inc14.i2915

for.inc14.i2915:                                  ; preds = %for.body6.i2912, %for.cond4.preheader.i2900
  %inc15.i2913 = add i32 %storemerge118.i2898, 1
  %cmp2.i2914 = icmp slt i32 %inc15.i2913, %storemerge21.i2891
  br i1 %cmp2.i2914, label %for.cond4.preheader.i2900, label %for.inc17.i2918

for.inc17.i2918:                                  ; preds = %for.inc14.i2915, %for.cond1.preheader.i2893
  %inc18.i2916 = add i32 %storemerge21.i2891, 1
  %cmp.i2917 = icmp slt i32 %inc18.i2916, %storemerge.lcssa4517
  br i1 %cmp.i2917, label %for.cond1.preheader.i2893, label %for.cond1.preheader.i2863

for.cond1.preheader.i2863:                        ; preds = %for.inc17.i2918, %for.inc17.i2888
  %storemerge21.i2861 = phi i32 [ %inc18.i2886, %for.inc17.i2888 ], [ 0, %for.inc17.i2918 ]
  %cmp217.i2862 = icmp sgt i32 %storemerge21.i2861, 0
  br i1 %cmp217.i2862, label %for.cond4.preheader.lr.ph.i2867, label %for.inc17.i2888

for.cond4.preheader.lr.ph.i2867:                  ; preds = %for.cond1.preheader.i2863
  %sub.i2864 = add i32 %storemerge21.i2861, -2
  %gep_array4984 = mul i32 %sub.i2864, 2
  %gep4985 = add i32 %fr, %gep_array4984
  %gep_array4987 = mul i32 %storemerge21.i2861, 2
  %gep4988 = add i32 %fr, %gep_array4987
  br label %for.cond4.preheader.i2870

for.cond4.preheader.i2870:                        ; preds = %for.inc14.i2885, %for.cond4.preheader.lr.ph.i2867
  %storemerge118.i2868 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2867 ], [ %inc15.i2883, %for.inc14.i2885 ]
  %cmp515.i2869 = icmp sgt i32 %storemerge118.i2868, 0
  br i1 %cmp515.i2869, label %for.body6.lr.ph.i2872, label %for.inc14.i2885

for.body6.lr.ph.i2872:                            ; preds = %for.cond4.preheader.i2870
  %gep_array4990 = mul i32 %storemerge118.i2868, 2
  %gep4991 = add i32 %fi, %gep_array4990
  br label %for.body6.i2882

for.body6.i2882:                                  ; preds = %for.body6.i2882, %for.body6.lr.ph.i2872
  %storemerge216.i2873 = phi i32 [ 0, %for.body6.lr.ph.i2872 ], [ %add12.i2879, %for.body6.i2882 ]
  %gep4991.asptr = inttoptr i32 %gep4991 to i16*
  %120 = load i16* %gep4991.asptr, align 1
  %conv.i2874 = sext i16 %120 to i32
  %mul.i2875 = mul i32 %conv.i2874, %storemerge.lcssa4517
  %gep4985.asptr = inttoptr i32 %gep4985 to i16*
  %121 = load i16* %gep4985.asptr, align 1
  %conv83.i2876 = zext i16 %121 to i32
  %add.i2877 = add i32 %mul.i2875, %conv83.i2876
  %conv9.i2878 = trunc i32 %add.i2877 to i16
  %gep4988.asptr = inttoptr i32 %gep4988 to i16*
  store i16 %conv9.i2878, i16* %gep4988.asptr, align 1
  %gep4991.asptr43 = inttoptr i32 %gep4991 to i16*
  %122 = load i16* %gep4991.asptr43, align 1
  %add12.i2879 = add i32 %storemerge216.i2873, 1
  %gep_array4993 = mul i32 %add12.i2879, 2
  %gep4994 = add i32 %fr, %gep_array4993
  %gep4994.asptr = inttoptr i32 %gep4994 to i16*
  store i16 %122, i16* %gep4994.asptr, align 1
  %cmp5.i2881 = icmp slt i32 %add12.i2879, %storemerge118.i2868
  br i1 %cmp5.i2881, label %for.body6.i2882, label %for.inc14.i2885

for.inc14.i2885:                                  ; preds = %for.body6.i2882, %for.cond4.preheader.i2870
  %inc15.i2883 = add i32 %storemerge118.i2868, 1
  %cmp2.i2884 = icmp slt i32 %inc15.i2883, %storemerge21.i2861
  br i1 %cmp2.i2884, label %for.cond4.preheader.i2870, label %for.inc17.i2888

for.inc17.i2888:                                  ; preds = %for.inc14.i2885, %for.cond1.preheader.i2863
  %inc18.i2886 = add i32 %storemerge21.i2861, 1
  %cmp.i2887 = icmp slt i32 %inc18.i2886, %storemerge.lcssa4517
  br i1 %cmp.i2887, label %for.cond1.preheader.i2863, label %for.cond1.preheader.i2833

for.cond1.preheader.i2833:                        ; preds = %for.inc17.i2888, %for.inc17.i2858
  %storemerge21.i2831 = phi i32 [ %inc18.i2856, %for.inc17.i2858 ], [ 0, %for.inc17.i2888 ]
  %cmp217.i2832 = icmp sgt i32 %storemerge21.i2831, 0
  br i1 %cmp217.i2832, label %for.cond4.preheader.lr.ph.i2837, label %for.inc17.i2858

for.cond4.preheader.lr.ph.i2837:                  ; preds = %for.cond1.preheader.i2833
  %sub.i2834 = add i32 %storemerge21.i2831, -2
  %gep_array4996 = mul i32 %sub.i2834, 2
  %gep4997 = add i32 %fr, %gep_array4996
  %gep_array4999 = mul i32 %storemerge21.i2831, 2
  %gep5000 = add i32 %fr, %gep_array4999
  br label %for.cond4.preheader.i2840

for.cond4.preheader.i2840:                        ; preds = %for.inc14.i2855, %for.cond4.preheader.lr.ph.i2837
  %storemerge118.i2838 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2837 ], [ %inc15.i2853, %for.inc14.i2855 ]
  %cmp515.i2839 = icmp sgt i32 %storemerge118.i2838, 0
  br i1 %cmp515.i2839, label %for.body6.lr.ph.i2842, label %for.inc14.i2855

for.body6.lr.ph.i2842:                            ; preds = %for.cond4.preheader.i2840
  %gep_array5002 = mul i32 %storemerge118.i2838, 2
  %gep5003 = add i32 %Sinewave, %gep_array5002
  br label %for.body6.i2852

for.body6.i2852:                                  ; preds = %for.body6.i2852, %for.body6.lr.ph.i2842
  %storemerge216.i2843 = phi i32 [ 0, %for.body6.lr.ph.i2842 ], [ %add12.i2849, %for.body6.i2852 ]
  %gep5003.asptr = inttoptr i32 %gep5003 to i16*
  %123 = load i16* %gep5003.asptr, align 1
  %conv.i2844 = sext i16 %123 to i32
  %mul.i2845 = mul i32 %conv.i2844, %storemerge.lcssa4517
  %gep4997.asptr = inttoptr i32 %gep4997 to i16*
  %124 = load i16* %gep4997.asptr, align 1
  %conv83.i2846 = zext i16 %124 to i32
  %add.i2847 = add i32 %mul.i2845, %conv83.i2846
  %conv9.i2848 = trunc i32 %add.i2847 to i16
  %gep5000.asptr = inttoptr i32 %gep5000 to i16*
  store i16 %conv9.i2848, i16* %gep5000.asptr, align 1
  %gep5003.asptr44 = inttoptr i32 %gep5003 to i16*
  %125 = load i16* %gep5003.asptr44, align 1
  %add12.i2849 = add i32 %storemerge216.i2843, 1
  %gep_array5005 = mul i32 %add12.i2849, 2
  %gep5006 = add i32 %fr, %gep_array5005
  %gep5006.asptr = inttoptr i32 %gep5006 to i16*
  store i16 %125, i16* %gep5006.asptr, align 1
  %cmp5.i2851 = icmp slt i32 %add12.i2849, %storemerge118.i2838
  br i1 %cmp5.i2851, label %for.body6.i2852, label %for.inc14.i2855

for.inc14.i2855:                                  ; preds = %for.body6.i2852, %for.cond4.preheader.i2840
  %inc15.i2853 = add i32 %storemerge118.i2838, 1
  %cmp2.i2854 = icmp slt i32 %inc15.i2853, %storemerge21.i2831
  br i1 %cmp2.i2854, label %for.cond4.preheader.i2840, label %for.inc17.i2858

for.inc17.i2858:                                  ; preds = %for.inc14.i2855, %for.cond1.preheader.i2833
  %inc18.i2856 = add i32 %storemerge21.i2831, 1
  %cmp.i2857 = icmp slt i32 %inc18.i2856, %storemerge.lcssa4517
  br i1 %cmp.i2857, label %for.cond1.preheader.i2833, label %for.cond1.preheader.i2803

for.cond1.preheader.i2803:                        ; preds = %for.inc17.i2858, %for.inc17.i2828
  %storemerge21.i2801 = phi i32 [ %inc18.i2826, %for.inc17.i2828 ], [ 0, %for.inc17.i2858 ]
  %cmp217.i2802 = icmp sgt i32 %storemerge21.i2801, 0
  br i1 %cmp217.i2802, label %for.cond4.preheader.lr.ph.i2807, label %for.inc17.i2828

for.cond4.preheader.lr.ph.i2807:                  ; preds = %for.cond1.preheader.i2803
  %sub.i2804 = add i32 %storemerge21.i2801, -2
  %gep_array5008 = mul i32 %sub.i2804, 2
  %gep5009 = add i32 %fr, %gep_array5008
  %gep_array5011 = mul i32 %storemerge21.i2801, 2
  %gep5012 = add i32 %fr, %gep_array5011
  br label %for.cond4.preheader.i2810

for.cond4.preheader.i2810:                        ; preds = %for.inc14.i2825, %for.cond4.preheader.lr.ph.i2807
  %storemerge118.i2808 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2807 ], [ %inc15.i2823, %for.inc14.i2825 ]
  %cmp515.i2809 = icmp sgt i32 %storemerge118.i2808, 0
  br i1 %cmp515.i2809, label %for.body6.lr.ph.i2812, label %for.inc14.i2825

for.body6.lr.ph.i2812:                            ; preds = %for.cond4.preheader.i2810
  %gep_array5014 = mul i32 %storemerge118.i2808, 2
  %gep5015 = add i32 %fi, %gep_array5014
  br label %for.body6.i2822

for.body6.i2822:                                  ; preds = %for.body6.i2822, %for.body6.lr.ph.i2812
  %storemerge216.i2813 = phi i32 [ 0, %for.body6.lr.ph.i2812 ], [ %add12.i2819, %for.body6.i2822 ]
  %gep5015.asptr = inttoptr i32 %gep5015 to i16*
  %126 = load i16* %gep5015.asptr, align 1
  %conv.i2814 = sext i16 %126 to i32
  %mul.i2815 = mul i32 %conv.i2814, %storemerge.lcssa4517
  %gep5009.asptr = inttoptr i32 %gep5009 to i16*
  %127 = load i16* %gep5009.asptr, align 1
  %conv83.i2816 = zext i16 %127 to i32
  %add.i2817 = add i32 %mul.i2815, %conv83.i2816
  %conv9.i2818 = trunc i32 %add.i2817 to i16
  %gep5012.asptr = inttoptr i32 %gep5012 to i16*
  store i16 %conv9.i2818, i16* %gep5012.asptr, align 1
  %gep5015.asptr45 = inttoptr i32 %gep5015 to i16*
  %128 = load i16* %gep5015.asptr45, align 1
  %add12.i2819 = add i32 %storemerge216.i2813, 1
  %gep_array5017 = mul i32 %add12.i2819, 2
  %gep5018 = add i32 %fr, %gep_array5017
  %gep5018.asptr = inttoptr i32 %gep5018 to i16*
  store i16 %128, i16* %gep5018.asptr, align 1
  %cmp5.i2821 = icmp slt i32 %add12.i2819, %storemerge118.i2808
  br i1 %cmp5.i2821, label %for.body6.i2822, label %for.inc14.i2825

for.inc14.i2825:                                  ; preds = %for.body6.i2822, %for.cond4.preheader.i2810
  %inc15.i2823 = add i32 %storemerge118.i2808, 1
  %cmp2.i2824 = icmp slt i32 %inc15.i2823, %storemerge21.i2801
  br i1 %cmp2.i2824, label %for.cond4.preheader.i2810, label %for.inc17.i2828

for.inc17.i2828:                                  ; preds = %for.inc14.i2825, %for.cond1.preheader.i2803
  %inc18.i2826 = add i32 %storemerge21.i2801, 1
  %cmp.i2827 = icmp slt i32 %inc18.i2826, %storemerge.lcssa4517
  br i1 %cmp.i2827, label %for.cond1.preheader.i2803, label %for.cond1.preheader.i2773

for.cond1.preheader.i2773:                        ; preds = %for.inc17.i2828, %for.inc17.i2798
  %storemerge21.i2771 = phi i32 [ %inc18.i2796, %for.inc17.i2798 ], [ 0, %for.inc17.i2828 ]
  %cmp217.i2772 = icmp sgt i32 %storemerge21.i2771, 0
  br i1 %cmp217.i2772, label %for.cond4.preheader.lr.ph.i2777, label %for.inc17.i2798

for.cond4.preheader.lr.ph.i2777:                  ; preds = %for.cond1.preheader.i2773
  %sub.i2774 = add i32 %storemerge21.i2771, -2
  %gep_array5020 = mul i32 %sub.i2774, 2
  %gep5021 = add i32 %fr, %gep_array5020
  %gep_array5023 = mul i32 %storemerge21.i2771, 2
  %gep5024 = add i32 %fr, %gep_array5023
  br label %for.cond4.preheader.i2780

for.cond4.preheader.i2780:                        ; preds = %for.inc14.i2795, %for.cond4.preheader.lr.ph.i2777
  %storemerge118.i2778 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2777 ], [ %inc15.i2793, %for.inc14.i2795 ]
  %cmp515.i2779 = icmp sgt i32 %storemerge118.i2778, 0
  br i1 %cmp515.i2779, label %for.body6.lr.ph.i2782, label %for.inc14.i2795

for.body6.lr.ph.i2782:                            ; preds = %for.cond4.preheader.i2780
  %gep_array5026 = mul i32 %storemerge118.i2778, 2
  %gep5027 = add i32 %Sinewave, %gep_array5026
  br label %for.body6.i2792

for.body6.i2792:                                  ; preds = %for.body6.i2792, %for.body6.lr.ph.i2782
  %storemerge216.i2783 = phi i32 [ 0, %for.body6.lr.ph.i2782 ], [ %add12.i2789, %for.body6.i2792 ]
  %gep5027.asptr = inttoptr i32 %gep5027 to i16*
  %129 = load i16* %gep5027.asptr, align 1
  %conv.i2784 = sext i16 %129 to i32
  %mul.i2785 = mul i32 %conv.i2784, %storemerge.lcssa4517
  %gep5021.asptr = inttoptr i32 %gep5021 to i16*
  %130 = load i16* %gep5021.asptr, align 1
  %conv83.i2786 = zext i16 %130 to i32
  %add.i2787 = add i32 %mul.i2785, %conv83.i2786
  %conv9.i2788 = trunc i32 %add.i2787 to i16
  %gep5024.asptr = inttoptr i32 %gep5024 to i16*
  store i16 %conv9.i2788, i16* %gep5024.asptr, align 1
  %gep5027.asptr46 = inttoptr i32 %gep5027 to i16*
  %131 = load i16* %gep5027.asptr46, align 1
  %add12.i2789 = add i32 %storemerge216.i2783, 1
  %gep_array5029 = mul i32 %add12.i2789, 2
  %gep5030 = add i32 %fr, %gep_array5029
  %gep5030.asptr = inttoptr i32 %gep5030 to i16*
  store i16 %131, i16* %gep5030.asptr, align 1
  %cmp5.i2791 = icmp slt i32 %add12.i2789, %storemerge118.i2778
  br i1 %cmp5.i2791, label %for.body6.i2792, label %for.inc14.i2795

for.inc14.i2795:                                  ; preds = %for.body6.i2792, %for.cond4.preheader.i2780
  %inc15.i2793 = add i32 %storemerge118.i2778, 1
  %cmp2.i2794 = icmp slt i32 %inc15.i2793, %storemerge21.i2771
  br i1 %cmp2.i2794, label %for.cond4.preheader.i2780, label %for.inc17.i2798

for.inc17.i2798:                                  ; preds = %for.inc14.i2795, %for.cond1.preheader.i2773
  %inc18.i2796 = add i32 %storemerge21.i2771, 1
  %cmp.i2797 = icmp slt i32 %inc18.i2796, %storemerge.lcssa4517
  br i1 %cmp.i2797, label %for.cond1.preheader.i2773, label %for.cond1.preheader.i2743

for.cond1.preheader.i2743:                        ; preds = %for.inc17.i2798, %for.inc17.i2768
  %storemerge21.i2741 = phi i32 [ %inc18.i2766, %for.inc17.i2768 ], [ 0, %for.inc17.i2798 ]
  %cmp217.i2742 = icmp sgt i32 %storemerge21.i2741, 0
  br i1 %cmp217.i2742, label %for.cond4.preheader.lr.ph.i2747, label %for.inc17.i2768

for.cond4.preheader.lr.ph.i2747:                  ; preds = %for.cond1.preheader.i2743
  %sub.i2744 = add i32 %storemerge21.i2741, -2
  %gep_array5032 = mul i32 %sub.i2744, 2
  %gep5033 = add i32 %fr, %gep_array5032
  %gep_array5035 = mul i32 %storemerge21.i2741, 2
  %gep5036 = add i32 %fr, %gep_array5035
  br label %for.cond4.preheader.i2750

for.cond4.preheader.i2750:                        ; preds = %for.inc14.i2765, %for.cond4.preheader.lr.ph.i2747
  %storemerge118.i2748 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2747 ], [ %inc15.i2763, %for.inc14.i2765 ]
  %cmp515.i2749 = icmp sgt i32 %storemerge118.i2748, 0
  br i1 %cmp515.i2749, label %for.body6.lr.ph.i2752, label %for.inc14.i2765

for.body6.lr.ph.i2752:                            ; preds = %for.cond4.preheader.i2750
  %gep_array5038 = mul i32 %storemerge118.i2748, 2
  %gep5039 = add i32 %Sinewave, %gep_array5038
  br label %for.body6.i2762

for.body6.i2762:                                  ; preds = %for.body6.i2762, %for.body6.lr.ph.i2752
  %storemerge216.i2753 = phi i32 [ 0, %for.body6.lr.ph.i2752 ], [ %add12.i2759, %for.body6.i2762 ]
  %gep5039.asptr = inttoptr i32 %gep5039 to i16*
  %132 = load i16* %gep5039.asptr, align 1
  %conv.i2754 = sext i16 %132 to i32
  %mul.i2755 = mul i32 %conv.i2754, %storemerge.lcssa4517
  %gep5033.asptr = inttoptr i32 %gep5033 to i16*
  %133 = load i16* %gep5033.asptr, align 1
  %conv83.i2756 = zext i16 %133 to i32
  %add.i2757 = add i32 %mul.i2755, %conv83.i2756
  %conv9.i2758 = trunc i32 %add.i2757 to i16
  %gep5036.asptr = inttoptr i32 %gep5036 to i16*
  store i16 %conv9.i2758, i16* %gep5036.asptr, align 1
  %gep5039.asptr47 = inttoptr i32 %gep5039 to i16*
  %134 = load i16* %gep5039.asptr47, align 1
  %add12.i2759 = add i32 %storemerge216.i2753, 1
  %gep_array5041 = mul i32 %add12.i2759, 2
  %gep5042 = add i32 %fr, %gep_array5041
  %gep5042.asptr = inttoptr i32 %gep5042 to i16*
  store i16 %134, i16* %gep5042.asptr, align 1
  %cmp5.i2761 = icmp slt i32 %add12.i2759, %storemerge118.i2748
  br i1 %cmp5.i2761, label %for.body6.i2762, label %for.inc14.i2765

for.inc14.i2765:                                  ; preds = %for.body6.i2762, %for.cond4.preheader.i2750
  %inc15.i2763 = add i32 %storemerge118.i2748, 1
  %cmp2.i2764 = icmp slt i32 %inc15.i2763, %storemerge21.i2741
  br i1 %cmp2.i2764, label %for.cond4.preheader.i2750, label %for.inc17.i2768

for.inc17.i2768:                                  ; preds = %for.inc14.i2765, %for.cond1.preheader.i2743
  %inc18.i2766 = add i32 %storemerge21.i2741, 1
  %cmp.i2767 = icmp slt i32 %inc18.i2766, %storemerge.lcssa4517
  br i1 %cmp.i2767, label %for.cond1.preheader.i2743, label %for.cond1.preheader.i2713

for.cond1.preheader.i2713:                        ; preds = %for.inc17.i2768, %for.inc17.i2738
  %storemerge21.i2711 = phi i32 [ %inc18.i2736, %for.inc17.i2738 ], [ 0, %for.inc17.i2768 ]
  %cmp217.i2712 = icmp sgt i32 %storemerge21.i2711, 0
  br i1 %cmp217.i2712, label %for.cond4.preheader.lr.ph.i2717, label %for.inc17.i2738

for.cond4.preheader.lr.ph.i2717:                  ; preds = %for.cond1.preheader.i2713
  %sub.i2714 = add i32 %storemerge21.i2711, -2
  %gep_array5044 = mul i32 %sub.i2714, 2
  %gep5045 = add i32 %fr, %gep_array5044
  %gep_array5047 = mul i32 %storemerge21.i2711, 2
  %gep5048 = add i32 %fr, %gep_array5047
  br label %for.cond4.preheader.i2720

for.cond4.preheader.i2720:                        ; preds = %for.inc14.i2735, %for.cond4.preheader.lr.ph.i2717
  %storemerge118.i2718 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2717 ], [ %inc15.i2733, %for.inc14.i2735 ]
  %cmp515.i2719 = icmp sgt i32 %storemerge118.i2718, 0
  br i1 %cmp515.i2719, label %for.body6.lr.ph.i2722, label %for.inc14.i2735

for.body6.lr.ph.i2722:                            ; preds = %for.cond4.preheader.i2720
  %gep_array5050 = mul i32 %storemerge118.i2718, 2
  %gep5051 = add i32 %fi, %gep_array5050
  br label %for.body6.i2732

for.body6.i2732:                                  ; preds = %for.body6.i2732, %for.body6.lr.ph.i2722
  %storemerge216.i2723 = phi i32 [ 0, %for.body6.lr.ph.i2722 ], [ %add12.i2729, %for.body6.i2732 ]
  %gep5051.asptr = inttoptr i32 %gep5051 to i16*
  %135 = load i16* %gep5051.asptr, align 1
  %conv.i2724 = sext i16 %135 to i32
  %mul.i2725 = mul i32 %conv.i2724, %storemerge.lcssa4517
  %gep5045.asptr = inttoptr i32 %gep5045 to i16*
  %136 = load i16* %gep5045.asptr, align 1
  %conv83.i2726 = zext i16 %136 to i32
  %add.i2727 = add i32 %mul.i2725, %conv83.i2726
  %conv9.i2728 = trunc i32 %add.i2727 to i16
  %gep5048.asptr = inttoptr i32 %gep5048 to i16*
  store i16 %conv9.i2728, i16* %gep5048.asptr, align 1
  %gep5051.asptr48 = inttoptr i32 %gep5051 to i16*
  %137 = load i16* %gep5051.asptr48, align 1
  %add12.i2729 = add i32 %storemerge216.i2723, 1
  %gep_array5053 = mul i32 %add12.i2729, 2
  %gep5054 = add i32 %fr, %gep_array5053
  %gep5054.asptr = inttoptr i32 %gep5054 to i16*
  store i16 %137, i16* %gep5054.asptr, align 1
  %cmp5.i2731 = icmp slt i32 %add12.i2729, %storemerge118.i2718
  br i1 %cmp5.i2731, label %for.body6.i2732, label %for.inc14.i2735

for.inc14.i2735:                                  ; preds = %for.body6.i2732, %for.cond4.preheader.i2720
  %inc15.i2733 = add i32 %storemerge118.i2718, 1
  %cmp2.i2734 = icmp slt i32 %inc15.i2733, %storemerge21.i2711
  br i1 %cmp2.i2734, label %for.cond4.preheader.i2720, label %for.inc17.i2738

for.inc17.i2738:                                  ; preds = %for.inc14.i2735, %for.cond1.preheader.i2713
  %inc18.i2736 = add i32 %storemerge21.i2711, 1
  %cmp.i2737 = icmp slt i32 %inc18.i2736, %storemerge.lcssa4517
  br i1 %cmp.i2737, label %for.cond1.preheader.i2713, label %for.cond1.preheader.i2683

for.cond1.preheader.i2683:                        ; preds = %for.inc17.i2738, %for.inc17.i2708
  %storemerge21.i2681 = phi i32 [ %inc18.i2706, %for.inc17.i2708 ], [ 0, %for.inc17.i2738 ]
  %cmp217.i2682 = icmp sgt i32 %storemerge21.i2681, 0
  br i1 %cmp217.i2682, label %for.cond4.preheader.lr.ph.i2687, label %for.inc17.i2708

for.cond4.preheader.lr.ph.i2687:                  ; preds = %for.cond1.preheader.i2683
  %sub.i2684 = add i32 %storemerge21.i2681, -2
  %gep_array5056 = mul i32 %sub.i2684, 2
  %gep5057 = add i32 %fr, %gep_array5056
  %gep_array5059 = mul i32 %storemerge21.i2681, 2
  %gep5060 = add i32 %fr, %gep_array5059
  br label %for.cond4.preheader.i2690

for.cond4.preheader.i2690:                        ; preds = %for.inc14.i2705, %for.cond4.preheader.lr.ph.i2687
  %storemerge118.i2688 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2687 ], [ %inc15.i2703, %for.inc14.i2705 ]
  %cmp515.i2689 = icmp sgt i32 %storemerge118.i2688, 0
  br i1 %cmp515.i2689, label %for.body6.lr.ph.i2692, label %for.inc14.i2705

for.body6.lr.ph.i2692:                            ; preds = %for.cond4.preheader.i2690
  %gep_array5062 = mul i32 %storemerge118.i2688, 2
  %gep5063 = add i32 %Sinewave, %gep_array5062
  br label %for.body6.i2702

for.body6.i2702:                                  ; preds = %for.body6.i2702, %for.body6.lr.ph.i2692
  %storemerge216.i2693 = phi i32 [ 0, %for.body6.lr.ph.i2692 ], [ %add12.i2699, %for.body6.i2702 ]
  %gep5063.asptr = inttoptr i32 %gep5063 to i16*
  %138 = load i16* %gep5063.asptr, align 1
  %conv.i2694 = sext i16 %138 to i32
  %mul.i2695 = mul i32 %conv.i2694, %storemerge.lcssa4517
  %gep5057.asptr = inttoptr i32 %gep5057 to i16*
  %139 = load i16* %gep5057.asptr, align 1
  %conv83.i2696 = zext i16 %139 to i32
  %add.i2697 = add i32 %mul.i2695, %conv83.i2696
  %conv9.i2698 = trunc i32 %add.i2697 to i16
  %gep5060.asptr = inttoptr i32 %gep5060 to i16*
  store i16 %conv9.i2698, i16* %gep5060.asptr, align 1
  %gep5063.asptr49 = inttoptr i32 %gep5063 to i16*
  %140 = load i16* %gep5063.asptr49, align 1
  %add12.i2699 = add i32 %storemerge216.i2693, 1
  %gep_array5065 = mul i32 %add12.i2699, 2
  %gep5066 = add i32 %fr, %gep_array5065
  %gep5066.asptr = inttoptr i32 %gep5066 to i16*
  store i16 %140, i16* %gep5066.asptr, align 1
  %cmp5.i2701 = icmp slt i32 %add12.i2699, %storemerge118.i2688
  br i1 %cmp5.i2701, label %for.body6.i2702, label %for.inc14.i2705

for.inc14.i2705:                                  ; preds = %for.body6.i2702, %for.cond4.preheader.i2690
  %inc15.i2703 = add i32 %storemerge118.i2688, 1
  %cmp2.i2704 = icmp slt i32 %inc15.i2703, %storemerge21.i2681
  br i1 %cmp2.i2704, label %for.cond4.preheader.i2690, label %for.inc17.i2708

for.inc17.i2708:                                  ; preds = %for.inc14.i2705, %for.cond1.preheader.i2683
  %inc18.i2706 = add i32 %storemerge21.i2681, 1
  %cmp.i2707 = icmp slt i32 %inc18.i2706, %storemerge.lcssa4517
  br i1 %cmp.i2707, label %for.cond1.preheader.i2683, label %for.cond1.preheader.i2653

for.cond1.preheader.i2653:                        ; preds = %for.inc17.i2708, %for.inc17.i2678
  %storemerge21.i2651 = phi i32 [ %inc18.i2676, %for.inc17.i2678 ], [ 0, %for.inc17.i2708 ]
  %cmp217.i2652 = icmp sgt i32 %storemerge21.i2651, 0
  br i1 %cmp217.i2652, label %for.cond4.preheader.lr.ph.i2657, label %for.inc17.i2678

for.cond4.preheader.lr.ph.i2657:                  ; preds = %for.cond1.preheader.i2653
  %sub.i2654 = add i32 %storemerge21.i2651, -2
  %gep_array5068 = mul i32 %sub.i2654, 2
  %gep5069 = add i32 %fr, %gep_array5068
  %gep_array5071 = mul i32 %storemerge21.i2651, 2
  %gep5072 = add i32 %fr, %gep_array5071
  br label %for.cond4.preheader.i2660

for.cond4.preheader.i2660:                        ; preds = %for.inc14.i2675, %for.cond4.preheader.lr.ph.i2657
  %storemerge118.i2658 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2657 ], [ %inc15.i2673, %for.inc14.i2675 ]
  %cmp515.i2659 = icmp sgt i32 %storemerge118.i2658, 0
  br i1 %cmp515.i2659, label %for.body6.lr.ph.i2662, label %for.inc14.i2675

for.body6.lr.ph.i2662:                            ; preds = %for.cond4.preheader.i2660
  %gep_array5074 = mul i32 %storemerge118.i2658, 2
  %gep5075 = add i32 %fi, %gep_array5074
  br label %for.body6.i2672

for.body6.i2672:                                  ; preds = %for.body6.i2672, %for.body6.lr.ph.i2662
  %storemerge216.i2663 = phi i32 [ 0, %for.body6.lr.ph.i2662 ], [ %add12.i2669, %for.body6.i2672 ]
  %gep5075.asptr = inttoptr i32 %gep5075 to i16*
  %141 = load i16* %gep5075.asptr, align 1
  %conv.i2664 = sext i16 %141 to i32
  %mul.i2665 = mul i32 %conv.i2664, %storemerge.lcssa4517
  %gep5069.asptr = inttoptr i32 %gep5069 to i16*
  %142 = load i16* %gep5069.asptr, align 1
  %conv83.i2666 = zext i16 %142 to i32
  %add.i2667 = add i32 %mul.i2665, %conv83.i2666
  %conv9.i2668 = trunc i32 %add.i2667 to i16
  %gep5072.asptr = inttoptr i32 %gep5072 to i16*
  store i16 %conv9.i2668, i16* %gep5072.asptr, align 1
  %gep5075.asptr50 = inttoptr i32 %gep5075 to i16*
  %143 = load i16* %gep5075.asptr50, align 1
  %add12.i2669 = add i32 %storemerge216.i2663, 1
  %gep_array5077 = mul i32 %add12.i2669, 2
  %gep5078 = add i32 %fr, %gep_array5077
  %gep5078.asptr = inttoptr i32 %gep5078 to i16*
  store i16 %143, i16* %gep5078.asptr, align 1
  %cmp5.i2671 = icmp slt i32 %add12.i2669, %storemerge118.i2658
  br i1 %cmp5.i2671, label %for.body6.i2672, label %for.inc14.i2675

for.inc14.i2675:                                  ; preds = %for.body6.i2672, %for.cond4.preheader.i2660
  %inc15.i2673 = add i32 %storemerge118.i2658, 1
  %cmp2.i2674 = icmp slt i32 %inc15.i2673, %storemerge21.i2651
  br i1 %cmp2.i2674, label %for.cond4.preheader.i2660, label %for.inc17.i2678

for.inc17.i2678:                                  ; preds = %for.inc14.i2675, %for.cond1.preheader.i2653
  %inc18.i2676 = add i32 %storemerge21.i2651, 1
  %cmp.i2677 = icmp slt i32 %inc18.i2676, %storemerge.lcssa4517
  br i1 %cmp.i2677, label %for.cond1.preheader.i2653, label %for.cond1.preheader.i2623

for.cond1.preheader.i2623:                        ; preds = %for.inc17.i2678, %for.inc17.i2648
  %storemerge21.i2621 = phi i32 [ %inc18.i2646, %for.inc17.i2648 ], [ 0, %for.inc17.i2678 ]
  %cmp217.i2622 = icmp sgt i32 %storemerge21.i2621, 0
  br i1 %cmp217.i2622, label %for.cond4.preheader.lr.ph.i2627, label %for.inc17.i2648

for.cond4.preheader.lr.ph.i2627:                  ; preds = %for.cond1.preheader.i2623
  %sub.i2624 = add i32 %storemerge21.i2621, -2
  %gep_array5080 = mul i32 %sub.i2624, 2
  %gep5081 = add i32 %fr, %gep_array5080
  %gep_array5083 = mul i32 %storemerge21.i2621, 2
  %gep5084 = add i32 %fr, %gep_array5083
  br label %for.cond4.preheader.i2630

for.cond4.preheader.i2630:                        ; preds = %for.inc14.i2645, %for.cond4.preheader.lr.ph.i2627
  %storemerge118.i2628 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2627 ], [ %inc15.i2643, %for.inc14.i2645 ]
  %cmp515.i2629 = icmp sgt i32 %storemerge118.i2628, 0
  br i1 %cmp515.i2629, label %for.body6.lr.ph.i2632, label %for.inc14.i2645

for.body6.lr.ph.i2632:                            ; preds = %for.cond4.preheader.i2630
  %gep_array5086 = mul i32 %storemerge118.i2628, 2
  %gep5087 = add i32 %Sinewave, %gep_array5086
  br label %for.body6.i2642

for.body6.i2642:                                  ; preds = %for.body6.i2642, %for.body6.lr.ph.i2632
  %storemerge216.i2633 = phi i32 [ 0, %for.body6.lr.ph.i2632 ], [ %add12.i2639, %for.body6.i2642 ]
  %gep5087.asptr = inttoptr i32 %gep5087 to i16*
  %144 = load i16* %gep5087.asptr, align 1
  %conv.i2634 = sext i16 %144 to i32
  %mul.i2635 = mul i32 %conv.i2634, %storemerge.lcssa4517
  %gep5081.asptr = inttoptr i32 %gep5081 to i16*
  %145 = load i16* %gep5081.asptr, align 1
  %conv83.i2636 = zext i16 %145 to i32
  %add.i2637 = add i32 %mul.i2635, %conv83.i2636
  %conv9.i2638 = trunc i32 %add.i2637 to i16
  %gep5084.asptr = inttoptr i32 %gep5084 to i16*
  store i16 %conv9.i2638, i16* %gep5084.asptr, align 1
  %gep5087.asptr51 = inttoptr i32 %gep5087 to i16*
  %146 = load i16* %gep5087.asptr51, align 1
  %add12.i2639 = add i32 %storemerge216.i2633, 1
  %gep_array5089 = mul i32 %add12.i2639, 2
  %gep5090 = add i32 %fr, %gep_array5089
  %gep5090.asptr = inttoptr i32 %gep5090 to i16*
  store i16 %146, i16* %gep5090.asptr, align 1
  %cmp5.i2641 = icmp slt i32 %add12.i2639, %storemerge118.i2628
  br i1 %cmp5.i2641, label %for.body6.i2642, label %for.inc14.i2645

for.inc14.i2645:                                  ; preds = %for.body6.i2642, %for.cond4.preheader.i2630
  %inc15.i2643 = add i32 %storemerge118.i2628, 1
  %cmp2.i2644 = icmp slt i32 %inc15.i2643, %storemerge21.i2621
  br i1 %cmp2.i2644, label %for.cond4.preheader.i2630, label %for.inc17.i2648

for.inc17.i2648:                                  ; preds = %for.inc14.i2645, %for.cond1.preheader.i2623
  %inc18.i2646 = add i32 %storemerge21.i2621, 1
  %cmp.i2647 = icmp slt i32 %inc18.i2646, %storemerge.lcssa4517
  br i1 %cmp.i2647, label %for.cond1.preheader.i2623, label %for.cond1.preheader.i2593

for.cond1.preheader.i2593:                        ; preds = %for.inc17.i2648, %for.inc17.i2618
  %storemerge21.i2591 = phi i32 [ %inc18.i2616, %for.inc17.i2618 ], [ 0, %for.inc17.i2648 ]
  %cmp217.i2592 = icmp sgt i32 %storemerge21.i2591, 0
  br i1 %cmp217.i2592, label %for.cond4.preheader.lr.ph.i2597, label %for.inc17.i2618

for.cond4.preheader.lr.ph.i2597:                  ; preds = %for.cond1.preheader.i2593
  %sub.i2594 = add i32 %storemerge21.i2591, -2
  %gep_array5092 = mul i32 %sub.i2594, 2
  %gep5093 = add i32 %fr, %gep_array5092
  %gep_array5095 = mul i32 %storemerge21.i2591, 2
  %gep5096 = add i32 %fr, %gep_array5095
  br label %for.cond4.preheader.i2600

for.cond4.preheader.i2600:                        ; preds = %for.inc14.i2615, %for.cond4.preheader.lr.ph.i2597
  %storemerge118.i2598 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2597 ], [ %inc15.i2613, %for.inc14.i2615 ]
  %cmp515.i2599 = icmp sgt i32 %storemerge118.i2598, 0
  br i1 %cmp515.i2599, label %for.body6.lr.ph.i2602, label %for.inc14.i2615

for.body6.lr.ph.i2602:                            ; preds = %for.cond4.preheader.i2600
  %gep_array5098 = mul i32 %storemerge118.i2598, 2
  %gep5099 = add i32 %fi, %gep_array5098
  br label %for.body6.i2612

for.body6.i2612:                                  ; preds = %for.body6.i2612, %for.body6.lr.ph.i2602
  %storemerge216.i2603 = phi i32 [ 0, %for.body6.lr.ph.i2602 ], [ %add12.i2609, %for.body6.i2612 ]
  %gep5099.asptr = inttoptr i32 %gep5099 to i16*
  %147 = load i16* %gep5099.asptr, align 1
  %conv.i2604 = sext i16 %147 to i32
  %mul.i2605 = mul i32 %conv.i2604, %storemerge.lcssa4517
  %gep5093.asptr = inttoptr i32 %gep5093 to i16*
  %148 = load i16* %gep5093.asptr, align 1
  %conv83.i2606 = zext i16 %148 to i32
  %add.i2607 = add i32 %mul.i2605, %conv83.i2606
  %conv9.i2608 = trunc i32 %add.i2607 to i16
  %gep5096.asptr = inttoptr i32 %gep5096 to i16*
  store i16 %conv9.i2608, i16* %gep5096.asptr, align 1
  %gep5099.asptr52 = inttoptr i32 %gep5099 to i16*
  %149 = load i16* %gep5099.asptr52, align 1
  %add12.i2609 = add i32 %storemerge216.i2603, 1
  %gep_array5101 = mul i32 %add12.i2609, 2
  %gep5102 = add i32 %fr, %gep_array5101
  %gep5102.asptr = inttoptr i32 %gep5102 to i16*
  store i16 %149, i16* %gep5102.asptr, align 1
  %cmp5.i2611 = icmp slt i32 %add12.i2609, %storemerge118.i2598
  br i1 %cmp5.i2611, label %for.body6.i2612, label %for.inc14.i2615

for.inc14.i2615:                                  ; preds = %for.body6.i2612, %for.cond4.preheader.i2600
  %inc15.i2613 = add i32 %storemerge118.i2598, 1
  %cmp2.i2614 = icmp slt i32 %inc15.i2613, %storemerge21.i2591
  br i1 %cmp2.i2614, label %for.cond4.preheader.i2600, label %for.inc17.i2618

for.inc17.i2618:                                  ; preds = %for.inc14.i2615, %for.cond1.preheader.i2593
  %inc18.i2616 = add i32 %storemerge21.i2591, 1
  %cmp.i2617 = icmp slt i32 %inc18.i2616, %storemerge.lcssa4517
  br i1 %cmp.i2617, label %for.cond1.preheader.i2593, label %for.cond1.preheader.i2563

for.cond1.preheader.i2563:                        ; preds = %for.inc17.i2618, %for.inc17.i2588
  %storemerge21.i2561 = phi i32 [ %inc18.i2586, %for.inc17.i2588 ], [ 0, %for.inc17.i2618 ]
  %cmp217.i2562 = icmp sgt i32 %storemerge21.i2561, 0
  br i1 %cmp217.i2562, label %for.cond4.preheader.lr.ph.i2567, label %for.inc17.i2588

for.cond4.preheader.lr.ph.i2567:                  ; preds = %for.cond1.preheader.i2563
  %sub.i2564 = add i32 %storemerge21.i2561, -2
  %gep_array5104 = mul i32 %sub.i2564, 2
  %gep5105 = add i32 %fr, %gep_array5104
  %gep_array5107 = mul i32 %storemerge21.i2561, 2
  %gep5108 = add i32 %fr, %gep_array5107
  br label %for.cond4.preheader.i2570

for.cond4.preheader.i2570:                        ; preds = %for.inc14.i2585, %for.cond4.preheader.lr.ph.i2567
  %storemerge118.i2568 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2567 ], [ %inc15.i2583, %for.inc14.i2585 ]
  %cmp515.i2569 = icmp sgt i32 %storemerge118.i2568, 0
  br i1 %cmp515.i2569, label %for.body6.lr.ph.i2572, label %for.inc14.i2585

for.body6.lr.ph.i2572:                            ; preds = %for.cond4.preheader.i2570
  %gep_array5110 = mul i32 %storemerge118.i2568, 2
  %gep5111 = add i32 %Sinewave, %gep_array5110
  br label %for.body6.i2582

for.body6.i2582:                                  ; preds = %for.body6.i2582, %for.body6.lr.ph.i2572
  %storemerge216.i2573 = phi i32 [ 0, %for.body6.lr.ph.i2572 ], [ %add12.i2579, %for.body6.i2582 ]
  %gep5111.asptr = inttoptr i32 %gep5111 to i16*
  %150 = load i16* %gep5111.asptr, align 1
  %conv.i2574 = sext i16 %150 to i32
  %mul.i2575 = mul i32 %conv.i2574, %storemerge.lcssa4517
  %gep5105.asptr = inttoptr i32 %gep5105 to i16*
  %151 = load i16* %gep5105.asptr, align 1
  %conv83.i2576 = zext i16 %151 to i32
  %add.i2577 = add i32 %mul.i2575, %conv83.i2576
  %conv9.i2578 = trunc i32 %add.i2577 to i16
  %gep5108.asptr = inttoptr i32 %gep5108 to i16*
  store i16 %conv9.i2578, i16* %gep5108.asptr, align 1
  %gep5111.asptr53 = inttoptr i32 %gep5111 to i16*
  %152 = load i16* %gep5111.asptr53, align 1
  %add12.i2579 = add i32 %storemerge216.i2573, 1
  %gep_array5113 = mul i32 %add12.i2579, 2
  %gep5114 = add i32 %fr, %gep_array5113
  %gep5114.asptr = inttoptr i32 %gep5114 to i16*
  store i16 %152, i16* %gep5114.asptr, align 1
  %cmp5.i2581 = icmp slt i32 %add12.i2579, %storemerge118.i2568
  br i1 %cmp5.i2581, label %for.body6.i2582, label %for.inc14.i2585

for.inc14.i2585:                                  ; preds = %for.body6.i2582, %for.cond4.preheader.i2570
  %inc15.i2583 = add i32 %storemerge118.i2568, 1
  %cmp2.i2584 = icmp slt i32 %inc15.i2583, %storemerge21.i2561
  br i1 %cmp2.i2584, label %for.cond4.preheader.i2570, label %for.inc17.i2588

for.inc17.i2588:                                  ; preds = %for.inc14.i2585, %for.cond1.preheader.i2563
  %inc18.i2586 = add i32 %storemerge21.i2561, 1
  %cmp.i2587 = icmp slt i32 %inc18.i2586, %storemerge.lcssa4517
  br i1 %cmp.i2587, label %for.cond1.preheader.i2563, label %for.cond1.preheader.i2533

for.cond1.preheader.i2533:                        ; preds = %for.inc17.i2588, %for.inc17.i2558
  %storemerge21.i2531 = phi i32 [ %inc18.i2556, %for.inc17.i2558 ], [ 0, %for.inc17.i2588 ]
  %cmp217.i2532 = icmp sgt i32 %storemerge21.i2531, 0
  br i1 %cmp217.i2532, label %for.cond4.preheader.lr.ph.i2537, label %for.inc17.i2558

for.cond4.preheader.lr.ph.i2537:                  ; preds = %for.cond1.preheader.i2533
  %sub.i2534 = add i32 %storemerge21.i2531, -2
  %gep_array5116 = mul i32 %sub.i2534, 2
  %gep5117 = add i32 %fr, %gep_array5116
  %gep_array5119 = mul i32 %storemerge21.i2531, 2
  %gep5120 = add i32 %fr, %gep_array5119
  br label %for.cond4.preheader.i2540

for.cond4.preheader.i2540:                        ; preds = %for.inc14.i2555, %for.cond4.preheader.lr.ph.i2537
  %storemerge118.i2538 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2537 ], [ %inc15.i2553, %for.inc14.i2555 ]
  %cmp515.i2539 = icmp sgt i32 %storemerge118.i2538, 0
  br i1 %cmp515.i2539, label %for.body6.lr.ph.i2542, label %for.inc14.i2555

for.body6.lr.ph.i2542:                            ; preds = %for.cond4.preheader.i2540
  %gep_array5122 = mul i32 %storemerge118.i2538, 2
  %gep5123 = add i32 %fi, %gep_array5122
  br label %for.body6.i2552

for.body6.i2552:                                  ; preds = %for.body6.i2552, %for.body6.lr.ph.i2542
  %storemerge216.i2543 = phi i32 [ 0, %for.body6.lr.ph.i2542 ], [ %add12.i2549, %for.body6.i2552 ]
  %gep5123.asptr = inttoptr i32 %gep5123 to i16*
  %153 = load i16* %gep5123.asptr, align 1
  %conv.i2544 = sext i16 %153 to i32
  %mul.i2545 = mul i32 %conv.i2544, %storemerge.lcssa4517
  %gep5117.asptr = inttoptr i32 %gep5117 to i16*
  %154 = load i16* %gep5117.asptr, align 1
  %conv83.i2546 = zext i16 %154 to i32
  %add.i2547 = add i32 %mul.i2545, %conv83.i2546
  %conv9.i2548 = trunc i32 %add.i2547 to i16
  %gep5120.asptr = inttoptr i32 %gep5120 to i16*
  store i16 %conv9.i2548, i16* %gep5120.asptr, align 1
  %gep5123.asptr54 = inttoptr i32 %gep5123 to i16*
  %155 = load i16* %gep5123.asptr54, align 1
  %add12.i2549 = add i32 %storemerge216.i2543, 1
  %gep_array5125 = mul i32 %add12.i2549, 2
  %gep5126 = add i32 %fr, %gep_array5125
  %gep5126.asptr = inttoptr i32 %gep5126 to i16*
  store i16 %155, i16* %gep5126.asptr, align 1
  %cmp5.i2551 = icmp slt i32 %add12.i2549, %storemerge118.i2538
  br i1 %cmp5.i2551, label %for.body6.i2552, label %for.inc14.i2555

for.inc14.i2555:                                  ; preds = %for.body6.i2552, %for.cond4.preheader.i2540
  %inc15.i2553 = add i32 %storemerge118.i2538, 1
  %cmp2.i2554 = icmp slt i32 %inc15.i2553, %storemerge21.i2531
  br i1 %cmp2.i2554, label %for.cond4.preheader.i2540, label %for.inc17.i2558

for.inc17.i2558:                                  ; preds = %for.inc14.i2555, %for.cond1.preheader.i2533
  %inc18.i2556 = add i32 %storemerge21.i2531, 1
  %cmp.i2557 = icmp slt i32 %inc18.i2556, %storemerge.lcssa4517
  br i1 %cmp.i2557, label %for.cond1.preheader.i2533, label %for.cond1.preheader.i2503

for.cond1.preheader.i2503:                        ; preds = %for.inc17.i2558, %for.inc17.i2528
  %storemerge21.i2501 = phi i32 [ %inc18.i2526, %for.inc17.i2528 ], [ 0, %for.inc17.i2558 ]
  %cmp217.i2502 = icmp sgt i32 %storemerge21.i2501, 0
  br i1 %cmp217.i2502, label %for.cond4.preheader.lr.ph.i2507, label %for.inc17.i2528

for.cond4.preheader.lr.ph.i2507:                  ; preds = %for.cond1.preheader.i2503
  %sub.i2504 = add i32 %storemerge21.i2501, -2
  %gep_array5128 = mul i32 %sub.i2504, 2
  %gep5129 = add i32 %fr, %gep_array5128
  %gep_array5131 = mul i32 %storemerge21.i2501, 2
  %gep5132 = add i32 %fr, %gep_array5131
  br label %for.cond4.preheader.i2510

for.cond4.preheader.i2510:                        ; preds = %for.inc14.i2525, %for.cond4.preheader.lr.ph.i2507
  %storemerge118.i2508 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2507 ], [ %inc15.i2523, %for.inc14.i2525 ]
  %cmp515.i2509 = icmp sgt i32 %storemerge118.i2508, 0
  br i1 %cmp515.i2509, label %for.body6.lr.ph.i2512, label %for.inc14.i2525

for.body6.lr.ph.i2512:                            ; preds = %for.cond4.preheader.i2510
  %gep_array5134 = mul i32 %storemerge118.i2508, 2
  %gep5135 = add i32 %Sinewave, %gep_array5134
  br label %for.body6.i2522

for.body6.i2522:                                  ; preds = %for.body6.i2522, %for.body6.lr.ph.i2512
  %storemerge216.i2513 = phi i32 [ 0, %for.body6.lr.ph.i2512 ], [ %add12.i2519, %for.body6.i2522 ]
  %gep5135.asptr = inttoptr i32 %gep5135 to i16*
  %156 = load i16* %gep5135.asptr, align 1
  %conv.i2514 = sext i16 %156 to i32
  %mul.i2515 = mul i32 %conv.i2514, %storemerge.lcssa4517
  %gep5129.asptr = inttoptr i32 %gep5129 to i16*
  %157 = load i16* %gep5129.asptr, align 1
  %conv83.i2516 = zext i16 %157 to i32
  %add.i2517 = add i32 %mul.i2515, %conv83.i2516
  %conv9.i2518 = trunc i32 %add.i2517 to i16
  %gep5132.asptr = inttoptr i32 %gep5132 to i16*
  store i16 %conv9.i2518, i16* %gep5132.asptr, align 1
  %gep5135.asptr55 = inttoptr i32 %gep5135 to i16*
  %158 = load i16* %gep5135.asptr55, align 1
  %add12.i2519 = add i32 %storemerge216.i2513, 1
  %gep_array5137 = mul i32 %add12.i2519, 2
  %gep5138 = add i32 %fr, %gep_array5137
  %gep5138.asptr = inttoptr i32 %gep5138 to i16*
  store i16 %158, i16* %gep5138.asptr, align 1
  %cmp5.i2521 = icmp slt i32 %add12.i2519, %storemerge118.i2508
  br i1 %cmp5.i2521, label %for.body6.i2522, label %for.inc14.i2525

for.inc14.i2525:                                  ; preds = %for.body6.i2522, %for.cond4.preheader.i2510
  %inc15.i2523 = add i32 %storemerge118.i2508, 1
  %cmp2.i2524 = icmp slt i32 %inc15.i2523, %storemerge21.i2501
  br i1 %cmp2.i2524, label %for.cond4.preheader.i2510, label %for.inc17.i2528

for.inc17.i2528:                                  ; preds = %for.inc14.i2525, %for.cond1.preheader.i2503
  %inc18.i2526 = add i32 %storemerge21.i2501, 1
  %cmp.i2527 = icmp slt i32 %inc18.i2526, %storemerge.lcssa4517
  br i1 %cmp.i2527, label %for.cond1.preheader.i2503, label %for.cond1.preheader.i2473

for.cond1.preheader.i2473:                        ; preds = %for.inc17.i2528, %for.inc17.i2498
  %storemerge21.i2471 = phi i32 [ %inc18.i2496, %for.inc17.i2498 ], [ 0, %for.inc17.i2528 ]
  %cmp217.i2472 = icmp sgt i32 %storemerge21.i2471, 0
  br i1 %cmp217.i2472, label %for.cond4.preheader.lr.ph.i2477, label %for.inc17.i2498

for.cond4.preheader.lr.ph.i2477:                  ; preds = %for.cond1.preheader.i2473
  %sub.i2474 = add i32 %storemerge21.i2471, -2
  %gep_array5140 = mul i32 %sub.i2474, 2
  %gep5141 = add i32 %fr, %gep_array5140
  %gep_array5143 = mul i32 %storemerge21.i2471, 2
  %gep5144 = add i32 %fr, %gep_array5143
  br label %for.cond4.preheader.i2480

for.cond4.preheader.i2480:                        ; preds = %for.inc14.i2495, %for.cond4.preheader.lr.ph.i2477
  %storemerge118.i2478 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2477 ], [ %inc15.i2493, %for.inc14.i2495 ]
  %cmp515.i2479 = icmp sgt i32 %storemerge118.i2478, 0
  br i1 %cmp515.i2479, label %for.body6.lr.ph.i2482, label %for.inc14.i2495

for.body6.lr.ph.i2482:                            ; preds = %for.cond4.preheader.i2480
  %gep_array5146 = mul i32 %storemerge118.i2478, 2
  %gep5147 = add i32 %fi, %gep_array5146
  br label %for.body6.i2492

for.body6.i2492:                                  ; preds = %for.body6.i2492, %for.body6.lr.ph.i2482
  %storemerge216.i2483 = phi i32 [ 0, %for.body6.lr.ph.i2482 ], [ %add12.i2489, %for.body6.i2492 ]
  %gep5147.asptr = inttoptr i32 %gep5147 to i16*
  %159 = load i16* %gep5147.asptr, align 1
  %conv.i2484 = sext i16 %159 to i32
  %mul.i2485 = mul i32 %conv.i2484, %storemerge.lcssa4517
  %gep5141.asptr = inttoptr i32 %gep5141 to i16*
  %160 = load i16* %gep5141.asptr, align 1
  %conv83.i2486 = zext i16 %160 to i32
  %add.i2487 = add i32 %mul.i2485, %conv83.i2486
  %conv9.i2488 = trunc i32 %add.i2487 to i16
  %gep5144.asptr = inttoptr i32 %gep5144 to i16*
  store i16 %conv9.i2488, i16* %gep5144.asptr, align 1
  %gep5147.asptr56 = inttoptr i32 %gep5147 to i16*
  %161 = load i16* %gep5147.asptr56, align 1
  %add12.i2489 = add i32 %storemerge216.i2483, 1
  %gep_array5149 = mul i32 %add12.i2489, 2
  %gep5150 = add i32 %fr, %gep_array5149
  %gep5150.asptr = inttoptr i32 %gep5150 to i16*
  store i16 %161, i16* %gep5150.asptr, align 1
  %cmp5.i2491 = icmp slt i32 %add12.i2489, %storemerge118.i2478
  br i1 %cmp5.i2491, label %for.body6.i2492, label %for.inc14.i2495

for.inc14.i2495:                                  ; preds = %for.body6.i2492, %for.cond4.preheader.i2480
  %inc15.i2493 = add i32 %storemerge118.i2478, 1
  %cmp2.i2494 = icmp slt i32 %inc15.i2493, %storemerge21.i2471
  br i1 %cmp2.i2494, label %for.cond4.preheader.i2480, label %for.inc17.i2498

for.inc17.i2498:                                  ; preds = %for.inc14.i2495, %for.cond1.preheader.i2473
  %inc18.i2496 = add i32 %storemerge21.i2471, 1
  %cmp.i2497 = icmp slt i32 %inc18.i2496, %storemerge.lcssa4517
  br i1 %cmp.i2497, label %for.cond1.preheader.i2473, label %for.cond1.preheader.i2443

for.cond1.preheader.i2443:                        ; preds = %for.inc17.i2498, %for.inc17.i2468
  %storemerge21.i2441 = phi i32 [ %inc18.i2466, %for.inc17.i2468 ], [ 0, %for.inc17.i2498 ]
  %cmp217.i2442 = icmp sgt i32 %storemerge21.i2441, 0
  br i1 %cmp217.i2442, label %for.cond4.preheader.lr.ph.i2447, label %for.inc17.i2468

for.cond4.preheader.lr.ph.i2447:                  ; preds = %for.cond1.preheader.i2443
  %sub.i2444 = add i32 %storemerge21.i2441, -2
  %gep_array5152 = mul i32 %sub.i2444, 2
  %gep5153 = add i32 %fr, %gep_array5152
  %gep_array5155 = mul i32 %storemerge21.i2441, 2
  %gep5156 = add i32 %fr, %gep_array5155
  br label %for.cond4.preheader.i2450

for.cond4.preheader.i2450:                        ; preds = %for.inc14.i2465, %for.cond4.preheader.lr.ph.i2447
  %storemerge118.i2448 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2447 ], [ %inc15.i2463, %for.inc14.i2465 ]
  %cmp515.i2449 = icmp sgt i32 %storemerge118.i2448, 0
  br i1 %cmp515.i2449, label %for.body6.lr.ph.i2452, label %for.inc14.i2465

for.body6.lr.ph.i2452:                            ; preds = %for.cond4.preheader.i2450
  %gep_array5158 = mul i32 %storemerge118.i2448, 2
  %gep5159 = add i32 %Sinewave, %gep_array5158
  br label %for.body6.i2462

for.body6.i2462:                                  ; preds = %for.body6.i2462, %for.body6.lr.ph.i2452
  %storemerge216.i2453 = phi i32 [ 0, %for.body6.lr.ph.i2452 ], [ %add12.i2459, %for.body6.i2462 ]
  %gep5159.asptr = inttoptr i32 %gep5159 to i16*
  %162 = load i16* %gep5159.asptr, align 1
  %conv.i2454 = sext i16 %162 to i32
  %mul.i2455 = mul i32 %conv.i2454, %storemerge.lcssa4517
  %gep5153.asptr = inttoptr i32 %gep5153 to i16*
  %163 = load i16* %gep5153.asptr, align 1
  %conv83.i2456 = zext i16 %163 to i32
  %add.i2457 = add i32 %mul.i2455, %conv83.i2456
  %conv9.i2458 = trunc i32 %add.i2457 to i16
  %gep5156.asptr = inttoptr i32 %gep5156 to i16*
  store i16 %conv9.i2458, i16* %gep5156.asptr, align 1
  %gep5159.asptr57 = inttoptr i32 %gep5159 to i16*
  %164 = load i16* %gep5159.asptr57, align 1
  %add12.i2459 = add i32 %storemerge216.i2453, 1
  %gep_array5161 = mul i32 %add12.i2459, 2
  %gep5162 = add i32 %fr, %gep_array5161
  %gep5162.asptr = inttoptr i32 %gep5162 to i16*
  store i16 %164, i16* %gep5162.asptr, align 1
  %cmp5.i2461 = icmp slt i32 %add12.i2459, %storemerge118.i2448
  br i1 %cmp5.i2461, label %for.body6.i2462, label %for.inc14.i2465

for.inc14.i2465:                                  ; preds = %for.body6.i2462, %for.cond4.preheader.i2450
  %inc15.i2463 = add i32 %storemerge118.i2448, 1
  %cmp2.i2464 = icmp slt i32 %inc15.i2463, %storemerge21.i2441
  br i1 %cmp2.i2464, label %for.cond4.preheader.i2450, label %for.inc17.i2468

for.inc17.i2468:                                  ; preds = %for.inc14.i2465, %for.cond1.preheader.i2443
  %inc18.i2466 = add i32 %storemerge21.i2441, 1
  %cmp.i2467 = icmp slt i32 %inc18.i2466, %storemerge.lcssa4517
  br i1 %cmp.i2467, label %for.cond1.preheader.i2443, label %for.cond1.preheader.i2413

for.cond1.preheader.i2413:                        ; preds = %for.inc17.i2468, %for.inc17.i2438
  %storemerge21.i2411 = phi i32 [ %inc18.i2436, %for.inc17.i2438 ], [ 0, %for.inc17.i2468 ]
  %cmp217.i2412 = icmp sgt i32 %storemerge21.i2411, 0
  br i1 %cmp217.i2412, label %for.cond4.preheader.lr.ph.i2417, label %for.inc17.i2438

for.cond4.preheader.lr.ph.i2417:                  ; preds = %for.cond1.preheader.i2413
  %sub.i2414 = add i32 %storemerge21.i2411, -2
  %gep_array5164 = mul i32 %sub.i2414, 2
  %gep5165 = add i32 %fr, %gep_array5164
  %gep_array5167 = mul i32 %storemerge21.i2411, 2
  %gep5168 = add i32 %fr, %gep_array5167
  br label %for.cond4.preheader.i2420

for.cond4.preheader.i2420:                        ; preds = %for.inc14.i2435, %for.cond4.preheader.lr.ph.i2417
  %storemerge118.i2418 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2417 ], [ %inc15.i2433, %for.inc14.i2435 ]
  %cmp515.i2419 = icmp sgt i32 %storemerge118.i2418, 0
  br i1 %cmp515.i2419, label %for.body6.lr.ph.i2422, label %for.inc14.i2435

for.body6.lr.ph.i2422:                            ; preds = %for.cond4.preheader.i2420
  %gep_array5170 = mul i32 %storemerge118.i2418, 2
  %gep5171 = add i32 %Sinewave, %gep_array5170
  br label %for.body6.i2432

for.body6.i2432:                                  ; preds = %for.body6.i2432, %for.body6.lr.ph.i2422
  %storemerge216.i2423 = phi i32 [ 0, %for.body6.lr.ph.i2422 ], [ %add12.i2429, %for.body6.i2432 ]
  %gep5171.asptr = inttoptr i32 %gep5171 to i16*
  %165 = load i16* %gep5171.asptr, align 1
  %conv.i2424 = sext i16 %165 to i32
  %mul.i2425 = mul i32 %conv.i2424, %storemerge.lcssa4517
  %gep5165.asptr = inttoptr i32 %gep5165 to i16*
  %166 = load i16* %gep5165.asptr, align 1
  %conv83.i2426 = zext i16 %166 to i32
  %add.i2427 = add i32 %mul.i2425, %conv83.i2426
  %conv9.i2428 = trunc i32 %add.i2427 to i16
  %gep5168.asptr = inttoptr i32 %gep5168 to i16*
  store i16 %conv9.i2428, i16* %gep5168.asptr, align 1
  %gep5171.asptr58 = inttoptr i32 %gep5171 to i16*
  %167 = load i16* %gep5171.asptr58, align 1
  %add12.i2429 = add i32 %storemerge216.i2423, 1
  %gep_array5173 = mul i32 %add12.i2429, 2
  %gep5174 = add i32 %fr, %gep_array5173
  %gep5174.asptr = inttoptr i32 %gep5174 to i16*
  store i16 %167, i16* %gep5174.asptr, align 1
  %cmp5.i2431 = icmp slt i32 %add12.i2429, %storemerge118.i2418
  br i1 %cmp5.i2431, label %for.body6.i2432, label %for.inc14.i2435

for.inc14.i2435:                                  ; preds = %for.body6.i2432, %for.cond4.preheader.i2420
  %inc15.i2433 = add i32 %storemerge118.i2418, 1
  %cmp2.i2434 = icmp slt i32 %inc15.i2433, %storemerge21.i2411
  br i1 %cmp2.i2434, label %for.cond4.preheader.i2420, label %for.inc17.i2438

for.inc17.i2438:                                  ; preds = %for.inc14.i2435, %for.cond1.preheader.i2413
  %inc18.i2436 = add i32 %storemerge21.i2411, 1
  %cmp.i2437 = icmp slt i32 %inc18.i2436, %storemerge.lcssa4517
  br i1 %cmp.i2437, label %for.cond1.preheader.i2413, label %for.cond1.preheader.i2383

for.cond1.preheader.i2383:                        ; preds = %for.inc17.i2438, %for.inc17.i2408
  %storemerge21.i2381 = phi i32 [ %inc18.i2406, %for.inc17.i2408 ], [ 0, %for.inc17.i2438 ]
  %cmp217.i2382 = icmp sgt i32 %storemerge21.i2381, 0
  br i1 %cmp217.i2382, label %for.cond4.preheader.lr.ph.i2387, label %for.inc17.i2408

for.cond4.preheader.lr.ph.i2387:                  ; preds = %for.cond1.preheader.i2383
  %sub.i2384 = add i32 %storemerge21.i2381, -2
  %gep_array5176 = mul i32 %sub.i2384, 2
  %gep5177 = add i32 %fr, %gep_array5176
  %gep_array5179 = mul i32 %storemerge21.i2381, 2
  %gep5180 = add i32 %fr, %gep_array5179
  br label %for.cond4.preheader.i2390

for.cond4.preheader.i2390:                        ; preds = %for.inc14.i2405, %for.cond4.preheader.lr.ph.i2387
  %storemerge118.i2388 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2387 ], [ %inc15.i2403, %for.inc14.i2405 ]
  %cmp515.i2389 = icmp sgt i32 %storemerge118.i2388, 0
  br i1 %cmp515.i2389, label %for.body6.lr.ph.i2392, label %for.inc14.i2405

for.body6.lr.ph.i2392:                            ; preds = %for.cond4.preheader.i2390
  %gep_array5182 = mul i32 %storemerge118.i2388, 2
  %gep5183 = add i32 %fi, %gep_array5182
  br label %for.body6.i2402

for.body6.i2402:                                  ; preds = %for.body6.i2402, %for.body6.lr.ph.i2392
  %storemerge216.i2393 = phi i32 [ 0, %for.body6.lr.ph.i2392 ], [ %add12.i2399, %for.body6.i2402 ]
  %gep5183.asptr = inttoptr i32 %gep5183 to i16*
  %168 = load i16* %gep5183.asptr, align 1
  %conv.i2394 = sext i16 %168 to i32
  %mul.i2395 = mul i32 %conv.i2394, %storemerge.lcssa4517
  %gep5177.asptr = inttoptr i32 %gep5177 to i16*
  %169 = load i16* %gep5177.asptr, align 1
  %conv83.i2396 = zext i16 %169 to i32
  %add.i2397 = add i32 %mul.i2395, %conv83.i2396
  %conv9.i2398 = trunc i32 %add.i2397 to i16
  %gep5180.asptr = inttoptr i32 %gep5180 to i16*
  store i16 %conv9.i2398, i16* %gep5180.asptr, align 1
  %gep5183.asptr59 = inttoptr i32 %gep5183 to i16*
  %170 = load i16* %gep5183.asptr59, align 1
  %add12.i2399 = add i32 %storemerge216.i2393, 1
  %gep_array5185 = mul i32 %add12.i2399, 2
  %gep5186 = add i32 %fr, %gep_array5185
  %gep5186.asptr = inttoptr i32 %gep5186 to i16*
  store i16 %170, i16* %gep5186.asptr, align 1
  %cmp5.i2401 = icmp slt i32 %add12.i2399, %storemerge118.i2388
  br i1 %cmp5.i2401, label %for.body6.i2402, label %for.inc14.i2405

for.inc14.i2405:                                  ; preds = %for.body6.i2402, %for.cond4.preheader.i2390
  %inc15.i2403 = add i32 %storemerge118.i2388, 1
  %cmp2.i2404 = icmp slt i32 %inc15.i2403, %storemerge21.i2381
  br i1 %cmp2.i2404, label %for.cond4.preheader.i2390, label %for.inc17.i2408

for.inc17.i2408:                                  ; preds = %for.inc14.i2405, %for.cond1.preheader.i2383
  %inc18.i2406 = add i32 %storemerge21.i2381, 1
  %cmp.i2407 = icmp slt i32 %inc18.i2406, %storemerge.lcssa4517
  br i1 %cmp.i2407, label %for.cond1.preheader.i2383, label %for.cond1.preheader.i2353

for.cond1.preheader.i2353:                        ; preds = %for.inc17.i2408, %for.inc17.i2378
  %storemerge21.i2351 = phi i32 [ %inc18.i2376, %for.inc17.i2378 ], [ 0, %for.inc17.i2408 ]
  %cmp217.i2352 = icmp sgt i32 %storemerge21.i2351, 0
  br i1 %cmp217.i2352, label %for.cond4.preheader.lr.ph.i2357, label %for.inc17.i2378

for.cond4.preheader.lr.ph.i2357:                  ; preds = %for.cond1.preheader.i2353
  %sub.i2354 = add i32 %storemerge21.i2351, -2
  %gep_array5188 = mul i32 %sub.i2354, 2
  %gep5189 = add i32 %fr, %gep_array5188
  %gep_array5191 = mul i32 %storemerge21.i2351, 2
  %gep5192 = add i32 %fr, %gep_array5191
  br label %for.cond4.preheader.i2360

for.cond4.preheader.i2360:                        ; preds = %for.inc14.i2375, %for.cond4.preheader.lr.ph.i2357
  %storemerge118.i2358 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2357 ], [ %inc15.i2373, %for.inc14.i2375 ]
  %cmp515.i2359 = icmp sgt i32 %storemerge118.i2358, 0
  br i1 %cmp515.i2359, label %for.body6.lr.ph.i2362, label %for.inc14.i2375

for.body6.lr.ph.i2362:                            ; preds = %for.cond4.preheader.i2360
  %gep_array5194 = mul i32 %storemerge118.i2358, 2
  %gep5195 = add i32 %Sinewave, %gep_array5194
  br label %for.body6.i2372

for.body6.i2372:                                  ; preds = %for.body6.i2372, %for.body6.lr.ph.i2362
  %storemerge216.i2363 = phi i32 [ 0, %for.body6.lr.ph.i2362 ], [ %add12.i2369, %for.body6.i2372 ]
  %gep5195.asptr = inttoptr i32 %gep5195 to i16*
  %171 = load i16* %gep5195.asptr, align 1
  %conv.i2364 = sext i16 %171 to i32
  %mul.i2365 = mul i32 %conv.i2364, %storemerge.lcssa4517
  %gep5189.asptr = inttoptr i32 %gep5189 to i16*
  %172 = load i16* %gep5189.asptr, align 1
  %conv83.i2366 = zext i16 %172 to i32
  %add.i2367 = add i32 %mul.i2365, %conv83.i2366
  %conv9.i2368 = trunc i32 %add.i2367 to i16
  %gep5192.asptr = inttoptr i32 %gep5192 to i16*
  store i16 %conv9.i2368, i16* %gep5192.asptr, align 1
  %gep5195.asptr60 = inttoptr i32 %gep5195 to i16*
  %173 = load i16* %gep5195.asptr60, align 1
  %add12.i2369 = add i32 %storemerge216.i2363, 1
  %gep_array5197 = mul i32 %add12.i2369, 2
  %gep5198 = add i32 %fr, %gep_array5197
  %gep5198.asptr = inttoptr i32 %gep5198 to i16*
  store i16 %173, i16* %gep5198.asptr, align 1
  %cmp5.i2371 = icmp slt i32 %add12.i2369, %storemerge118.i2358
  br i1 %cmp5.i2371, label %for.body6.i2372, label %for.inc14.i2375

for.inc14.i2375:                                  ; preds = %for.body6.i2372, %for.cond4.preheader.i2360
  %inc15.i2373 = add i32 %storemerge118.i2358, 1
  %cmp2.i2374 = icmp slt i32 %inc15.i2373, %storemerge21.i2351
  br i1 %cmp2.i2374, label %for.cond4.preheader.i2360, label %for.inc17.i2378

for.inc17.i2378:                                  ; preds = %for.inc14.i2375, %for.cond1.preheader.i2353
  %inc18.i2376 = add i32 %storemerge21.i2351, 1
  %cmp.i2377 = icmp slt i32 %inc18.i2376, %storemerge.lcssa4517
  br i1 %cmp.i2377, label %for.cond1.preheader.i2353, label %for.cond1.preheader.i2323

for.cond1.preheader.i2323:                        ; preds = %for.inc17.i2378, %for.inc17.i2348
  %storemerge21.i2321 = phi i32 [ %inc18.i2346, %for.inc17.i2348 ], [ 0, %for.inc17.i2378 ]
  %cmp217.i2322 = icmp sgt i32 %storemerge21.i2321, 0
  br i1 %cmp217.i2322, label %for.cond4.preheader.lr.ph.i2327, label %for.inc17.i2348

for.cond4.preheader.lr.ph.i2327:                  ; preds = %for.cond1.preheader.i2323
  %sub.i2324 = add i32 %storemerge21.i2321, -2
  %gep_array5200 = mul i32 %sub.i2324, 2
  %gep5201 = add i32 %fr, %gep_array5200
  %gep_array5203 = mul i32 %storemerge21.i2321, 2
  %gep5204 = add i32 %fr, %gep_array5203
  br label %for.cond4.preheader.i2330

for.cond4.preheader.i2330:                        ; preds = %for.inc14.i2345, %for.cond4.preheader.lr.ph.i2327
  %storemerge118.i2328 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2327 ], [ %inc15.i2343, %for.inc14.i2345 ]
  %cmp515.i2329 = icmp sgt i32 %storemerge118.i2328, 0
  br i1 %cmp515.i2329, label %for.body6.lr.ph.i2332, label %for.inc14.i2345

for.body6.lr.ph.i2332:                            ; preds = %for.cond4.preheader.i2330
  %gep_array5206 = mul i32 %storemerge118.i2328, 2
  %gep5207 = add i32 %fi, %gep_array5206
  br label %for.body6.i2342

for.body6.i2342:                                  ; preds = %for.body6.i2342, %for.body6.lr.ph.i2332
  %storemerge216.i2333 = phi i32 [ 0, %for.body6.lr.ph.i2332 ], [ %add12.i2339, %for.body6.i2342 ]
  %gep5207.asptr = inttoptr i32 %gep5207 to i16*
  %174 = load i16* %gep5207.asptr, align 1
  %conv.i2334 = sext i16 %174 to i32
  %mul.i2335 = mul i32 %conv.i2334, %storemerge.lcssa4517
  %gep5201.asptr = inttoptr i32 %gep5201 to i16*
  %175 = load i16* %gep5201.asptr, align 1
  %conv83.i2336 = zext i16 %175 to i32
  %add.i2337 = add i32 %mul.i2335, %conv83.i2336
  %conv9.i2338 = trunc i32 %add.i2337 to i16
  %gep5204.asptr = inttoptr i32 %gep5204 to i16*
  store i16 %conv9.i2338, i16* %gep5204.asptr, align 1
  %gep5207.asptr61 = inttoptr i32 %gep5207 to i16*
  %176 = load i16* %gep5207.asptr61, align 1
  %add12.i2339 = add i32 %storemerge216.i2333, 1
  %gep_array5209 = mul i32 %add12.i2339, 2
  %gep5210 = add i32 %fr, %gep_array5209
  %gep5210.asptr = inttoptr i32 %gep5210 to i16*
  store i16 %176, i16* %gep5210.asptr, align 1
  %cmp5.i2341 = icmp slt i32 %add12.i2339, %storemerge118.i2328
  br i1 %cmp5.i2341, label %for.body6.i2342, label %for.inc14.i2345

for.inc14.i2345:                                  ; preds = %for.body6.i2342, %for.cond4.preheader.i2330
  %inc15.i2343 = add i32 %storemerge118.i2328, 1
  %cmp2.i2344 = icmp slt i32 %inc15.i2343, %storemerge21.i2321
  br i1 %cmp2.i2344, label %for.cond4.preheader.i2330, label %for.inc17.i2348

for.inc17.i2348:                                  ; preds = %for.inc14.i2345, %for.cond1.preheader.i2323
  %inc18.i2346 = add i32 %storemerge21.i2321, 1
  %cmp.i2347 = icmp slt i32 %inc18.i2346, %storemerge.lcssa4517
  br i1 %cmp.i2347, label %for.cond1.preheader.i2323, label %for.cond1.preheader.i2293

for.cond1.preheader.i2293:                        ; preds = %for.inc17.i2348, %for.inc17.i2318
  %storemerge21.i2291 = phi i32 [ %inc18.i2316, %for.inc17.i2318 ], [ 0, %for.inc17.i2348 ]
  %cmp217.i2292 = icmp sgt i32 %storemerge21.i2291, 0
  br i1 %cmp217.i2292, label %for.cond4.preheader.lr.ph.i2297, label %for.inc17.i2318

for.cond4.preheader.lr.ph.i2297:                  ; preds = %for.cond1.preheader.i2293
  %sub.i2294 = add i32 %storemerge21.i2291, -2
  %gep_array5212 = mul i32 %sub.i2294, 2
  %gep5213 = add i32 %fr, %gep_array5212
  %gep_array5215 = mul i32 %storemerge21.i2291, 2
  %gep5216 = add i32 %fr, %gep_array5215
  br label %for.cond4.preheader.i2300

for.cond4.preheader.i2300:                        ; preds = %for.inc14.i2315, %for.cond4.preheader.lr.ph.i2297
  %storemerge118.i2298 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2297 ], [ %inc15.i2313, %for.inc14.i2315 ]
  %cmp515.i2299 = icmp sgt i32 %storemerge118.i2298, 0
  br i1 %cmp515.i2299, label %for.body6.lr.ph.i2302, label %for.inc14.i2315

for.body6.lr.ph.i2302:                            ; preds = %for.cond4.preheader.i2300
  %gep_array5218 = mul i32 %storemerge118.i2298, 2
  %gep5219 = add i32 %Sinewave, %gep_array5218
  br label %for.body6.i2312

for.body6.i2312:                                  ; preds = %for.body6.i2312, %for.body6.lr.ph.i2302
  %storemerge216.i2303 = phi i32 [ 0, %for.body6.lr.ph.i2302 ], [ %add12.i2309, %for.body6.i2312 ]
  %gep5219.asptr = inttoptr i32 %gep5219 to i16*
  %177 = load i16* %gep5219.asptr, align 1
  %conv.i2304 = sext i16 %177 to i32
  %mul.i2305 = mul i32 %conv.i2304, %storemerge.lcssa4517
  %gep5213.asptr = inttoptr i32 %gep5213 to i16*
  %178 = load i16* %gep5213.asptr, align 1
  %conv83.i2306 = zext i16 %178 to i32
  %add.i2307 = add i32 %mul.i2305, %conv83.i2306
  %conv9.i2308 = trunc i32 %add.i2307 to i16
  %gep5216.asptr = inttoptr i32 %gep5216 to i16*
  store i16 %conv9.i2308, i16* %gep5216.asptr, align 1
  %gep5219.asptr62 = inttoptr i32 %gep5219 to i16*
  %179 = load i16* %gep5219.asptr62, align 1
  %add12.i2309 = add i32 %storemerge216.i2303, 1
  %gep_array5221 = mul i32 %add12.i2309, 2
  %gep5222 = add i32 %fr, %gep_array5221
  %gep5222.asptr = inttoptr i32 %gep5222 to i16*
  store i16 %179, i16* %gep5222.asptr, align 1
  %cmp5.i2311 = icmp slt i32 %add12.i2309, %storemerge118.i2298
  br i1 %cmp5.i2311, label %for.body6.i2312, label %for.inc14.i2315

for.inc14.i2315:                                  ; preds = %for.body6.i2312, %for.cond4.preheader.i2300
  %inc15.i2313 = add i32 %storemerge118.i2298, 1
  %cmp2.i2314 = icmp slt i32 %inc15.i2313, %storemerge21.i2291
  br i1 %cmp2.i2314, label %for.cond4.preheader.i2300, label %for.inc17.i2318

for.inc17.i2318:                                  ; preds = %for.inc14.i2315, %for.cond1.preheader.i2293
  %inc18.i2316 = add i32 %storemerge21.i2291, 1
  %cmp.i2317 = icmp slt i32 %inc18.i2316, %storemerge.lcssa4517
  br i1 %cmp.i2317, label %for.cond1.preheader.i2293, label %for.cond1.preheader.i2263

for.cond1.preheader.i2263:                        ; preds = %for.inc17.i2318, %for.inc17.i2288
  %storemerge21.i2261 = phi i32 [ %inc18.i2286, %for.inc17.i2288 ], [ 0, %for.inc17.i2318 ]
  %cmp217.i2262 = icmp sgt i32 %storemerge21.i2261, 0
  br i1 %cmp217.i2262, label %for.cond4.preheader.lr.ph.i2267, label %for.inc17.i2288

for.cond4.preheader.lr.ph.i2267:                  ; preds = %for.cond1.preheader.i2263
  %sub.i2264 = add i32 %storemerge21.i2261, -2
  %gep_array5224 = mul i32 %sub.i2264, 2
  %gep5225 = add i32 %fr, %gep_array5224
  %gep_array5227 = mul i32 %storemerge21.i2261, 2
  %gep5228 = add i32 %fr, %gep_array5227
  br label %for.cond4.preheader.i2270

for.cond4.preheader.i2270:                        ; preds = %for.inc14.i2285, %for.cond4.preheader.lr.ph.i2267
  %storemerge118.i2268 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2267 ], [ %inc15.i2283, %for.inc14.i2285 ]
  %cmp515.i2269 = icmp sgt i32 %storemerge118.i2268, 0
  br i1 %cmp515.i2269, label %for.body6.lr.ph.i2272, label %for.inc14.i2285

for.body6.lr.ph.i2272:                            ; preds = %for.cond4.preheader.i2270
  %gep_array5230 = mul i32 %storemerge118.i2268, 2
  %gep5231 = add i32 %fi, %gep_array5230
  br label %for.body6.i2282

for.body6.i2282:                                  ; preds = %for.body6.i2282, %for.body6.lr.ph.i2272
  %storemerge216.i2273 = phi i32 [ 0, %for.body6.lr.ph.i2272 ], [ %add12.i2279, %for.body6.i2282 ]
  %gep5231.asptr = inttoptr i32 %gep5231 to i16*
  %180 = load i16* %gep5231.asptr, align 1
  %conv.i2274 = sext i16 %180 to i32
  %mul.i2275 = mul i32 %conv.i2274, %storemerge.lcssa4517
  %gep5225.asptr = inttoptr i32 %gep5225 to i16*
  %181 = load i16* %gep5225.asptr, align 1
  %conv83.i2276 = zext i16 %181 to i32
  %add.i2277 = add i32 %mul.i2275, %conv83.i2276
  %conv9.i2278 = trunc i32 %add.i2277 to i16
  %gep5228.asptr = inttoptr i32 %gep5228 to i16*
  store i16 %conv9.i2278, i16* %gep5228.asptr, align 1
  %gep5231.asptr63 = inttoptr i32 %gep5231 to i16*
  %182 = load i16* %gep5231.asptr63, align 1
  %add12.i2279 = add i32 %storemerge216.i2273, 1
  %gep_array5233 = mul i32 %add12.i2279, 2
  %gep5234 = add i32 %fr, %gep_array5233
  %gep5234.asptr = inttoptr i32 %gep5234 to i16*
  store i16 %182, i16* %gep5234.asptr, align 1
  %cmp5.i2281 = icmp slt i32 %add12.i2279, %storemerge118.i2268
  br i1 %cmp5.i2281, label %for.body6.i2282, label %for.inc14.i2285

for.inc14.i2285:                                  ; preds = %for.body6.i2282, %for.cond4.preheader.i2270
  %inc15.i2283 = add i32 %storemerge118.i2268, 1
  %cmp2.i2284 = icmp slt i32 %inc15.i2283, %storemerge21.i2261
  br i1 %cmp2.i2284, label %for.cond4.preheader.i2270, label %for.inc17.i2288

for.inc17.i2288:                                  ; preds = %for.inc14.i2285, %for.cond1.preheader.i2263
  %inc18.i2286 = add i32 %storemerge21.i2261, 1
  %cmp.i2287 = icmp slt i32 %inc18.i2286, %storemerge.lcssa4517
  br i1 %cmp.i2287, label %for.cond1.preheader.i2263, label %for.cond1.preheader.i2233

for.cond1.preheader.i2233:                        ; preds = %for.inc17.i2288, %for.inc17.i2258
  %storemerge21.i2231 = phi i32 [ %inc18.i2256, %for.inc17.i2258 ], [ 0, %for.inc17.i2288 ]
  %cmp217.i2232 = icmp sgt i32 %storemerge21.i2231, 0
  br i1 %cmp217.i2232, label %for.cond4.preheader.lr.ph.i2237, label %for.inc17.i2258

for.cond4.preheader.lr.ph.i2237:                  ; preds = %for.cond1.preheader.i2233
  %sub.i2234 = add i32 %storemerge21.i2231, -2
  %gep_array5236 = mul i32 %sub.i2234, 2
  %gep5237 = add i32 %fr, %gep_array5236
  %gep_array5239 = mul i32 %storemerge21.i2231, 2
  %gep5240 = add i32 %fr, %gep_array5239
  br label %for.cond4.preheader.i2240

for.cond4.preheader.i2240:                        ; preds = %for.inc14.i2255, %for.cond4.preheader.lr.ph.i2237
  %storemerge118.i2238 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2237 ], [ %inc15.i2253, %for.inc14.i2255 ]
  %cmp515.i2239 = icmp sgt i32 %storemerge118.i2238, 0
  br i1 %cmp515.i2239, label %for.body6.lr.ph.i2242, label %for.inc14.i2255

for.body6.lr.ph.i2242:                            ; preds = %for.cond4.preheader.i2240
  %gep_array5242 = mul i32 %storemerge118.i2238, 2
  %gep5243 = add i32 %Sinewave, %gep_array5242
  br label %for.body6.i2252

for.body6.i2252:                                  ; preds = %for.body6.i2252, %for.body6.lr.ph.i2242
  %storemerge216.i2243 = phi i32 [ 0, %for.body6.lr.ph.i2242 ], [ %add12.i2249, %for.body6.i2252 ]
  %gep5243.asptr = inttoptr i32 %gep5243 to i16*
  %183 = load i16* %gep5243.asptr, align 1
  %conv.i2244 = sext i16 %183 to i32
  %mul.i2245 = mul i32 %conv.i2244, %storemerge.lcssa4517
  %gep5237.asptr = inttoptr i32 %gep5237 to i16*
  %184 = load i16* %gep5237.asptr, align 1
  %conv83.i2246 = zext i16 %184 to i32
  %add.i2247 = add i32 %mul.i2245, %conv83.i2246
  %conv9.i2248 = trunc i32 %add.i2247 to i16
  %gep5240.asptr = inttoptr i32 %gep5240 to i16*
  store i16 %conv9.i2248, i16* %gep5240.asptr, align 1
  %gep5243.asptr64 = inttoptr i32 %gep5243 to i16*
  %185 = load i16* %gep5243.asptr64, align 1
  %add12.i2249 = add i32 %storemerge216.i2243, 1
  %gep_array5245 = mul i32 %add12.i2249, 2
  %gep5246 = add i32 %fr, %gep_array5245
  %gep5246.asptr = inttoptr i32 %gep5246 to i16*
  store i16 %185, i16* %gep5246.asptr, align 1
  %cmp5.i2251 = icmp slt i32 %add12.i2249, %storemerge118.i2238
  br i1 %cmp5.i2251, label %for.body6.i2252, label %for.inc14.i2255

for.inc14.i2255:                                  ; preds = %for.body6.i2252, %for.cond4.preheader.i2240
  %inc15.i2253 = add i32 %storemerge118.i2238, 1
  %cmp2.i2254 = icmp slt i32 %inc15.i2253, %storemerge21.i2231
  br i1 %cmp2.i2254, label %for.cond4.preheader.i2240, label %for.inc17.i2258

for.inc17.i2258:                                  ; preds = %for.inc14.i2255, %for.cond1.preheader.i2233
  %inc18.i2256 = add i32 %storemerge21.i2231, 1
  %cmp.i2257 = icmp slt i32 %inc18.i2256, %storemerge.lcssa4517
  br i1 %cmp.i2257, label %for.cond1.preheader.i2233, label %for.cond1.preheader.i2203

for.cond1.preheader.i2203:                        ; preds = %for.inc17.i2258, %for.inc17.i2228
  %storemerge21.i2201 = phi i32 [ %inc18.i2226, %for.inc17.i2228 ], [ 0, %for.inc17.i2258 ]
  %cmp217.i2202 = icmp sgt i32 %storemerge21.i2201, 0
  br i1 %cmp217.i2202, label %for.cond4.preheader.lr.ph.i2207, label %for.inc17.i2228

for.cond4.preheader.lr.ph.i2207:                  ; preds = %for.cond1.preheader.i2203
  %sub.i2204 = add i32 %storemerge21.i2201, -2
  %gep_array5248 = mul i32 %sub.i2204, 2
  %gep5249 = add i32 %fr, %gep_array5248
  %gep_array5251 = mul i32 %storemerge21.i2201, 2
  %gep5252 = add i32 %fr, %gep_array5251
  br label %for.cond4.preheader.i2210

for.cond4.preheader.i2210:                        ; preds = %for.inc14.i2225, %for.cond4.preheader.lr.ph.i2207
  %storemerge118.i2208 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2207 ], [ %inc15.i2223, %for.inc14.i2225 ]
  %cmp515.i2209 = icmp sgt i32 %storemerge118.i2208, 0
  br i1 %cmp515.i2209, label %for.body6.lr.ph.i2212, label %for.inc14.i2225

for.body6.lr.ph.i2212:                            ; preds = %for.cond4.preheader.i2210
  %gep_array5254 = mul i32 %storemerge118.i2208, 2
  %gep5255 = add i32 %fi, %gep_array5254
  br label %for.body6.i2222

for.body6.i2222:                                  ; preds = %for.body6.i2222, %for.body6.lr.ph.i2212
  %storemerge216.i2213 = phi i32 [ 0, %for.body6.lr.ph.i2212 ], [ %add12.i2219, %for.body6.i2222 ]
  %gep5255.asptr = inttoptr i32 %gep5255 to i16*
  %186 = load i16* %gep5255.asptr, align 1
  %conv.i2214 = sext i16 %186 to i32
  %mul.i2215 = mul i32 %conv.i2214, %storemerge.lcssa4517
  %gep5249.asptr = inttoptr i32 %gep5249 to i16*
  %187 = load i16* %gep5249.asptr, align 1
  %conv83.i2216 = zext i16 %187 to i32
  %add.i2217 = add i32 %mul.i2215, %conv83.i2216
  %conv9.i2218 = trunc i32 %add.i2217 to i16
  %gep5252.asptr = inttoptr i32 %gep5252 to i16*
  store i16 %conv9.i2218, i16* %gep5252.asptr, align 1
  %gep5255.asptr65 = inttoptr i32 %gep5255 to i16*
  %188 = load i16* %gep5255.asptr65, align 1
  %add12.i2219 = add i32 %storemerge216.i2213, 1
  %gep_array5257 = mul i32 %add12.i2219, 2
  %gep5258 = add i32 %fr, %gep_array5257
  %gep5258.asptr = inttoptr i32 %gep5258 to i16*
  store i16 %188, i16* %gep5258.asptr, align 1
  %cmp5.i2221 = icmp slt i32 %add12.i2219, %storemerge118.i2208
  br i1 %cmp5.i2221, label %for.body6.i2222, label %for.inc14.i2225

for.inc14.i2225:                                  ; preds = %for.body6.i2222, %for.cond4.preheader.i2210
  %inc15.i2223 = add i32 %storemerge118.i2208, 1
  %cmp2.i2224 = icmp slt i32 %inc15.i2223, %storemerge21.i2201
  br i1 %cmp2.i2224, label %for.cond4.preheader.i2210, label %for.inc17.i2228

for.inc17.i2228:                                  ; preds = %for.inc14.i2225, %for.cond1.preheader.i2203
  %inc18.i2226 = add i32 %storemerge21.i2201, 1
  %cmp.i2227 = icmp slt i32 %inc18.i2226, %storemerge.lcssa4517
  br i1 %cmp.i2227, label %for.cond1.preheader.i2203, label %for.cond1.preheader.i2173

for.cond1.preheader.i2173:                        ; preds = %for.inc17.i2228, %for.inc17.i2198
  %storemerge21.i2171 = phi i32 [ %inc18.i2196, %for.inc17.i2198 ], [ 0, %for.inc17.i2228 ]
  %cmp217.i2172 = icmp sgt i32 %storemerge21.i2171, 0
  br i1 %cmp217.i2172, label %for.cond4.preheader.lr.ph.i2177, label %for.inc17.i2198

for.cond4.preheader.lr.ph.i2177:                  ; preds = %for.cond1.preheader.i2173
  %sub.i2174 = add i32 %storemerge21.i2171, -2
  %gep_array5260 = mul i32 %sub.i2174, 2
  %gep5261 = add i32 %fr, %gep_array5260
  %gep_array5263 = mul i32 %storemerge21.i2171, 2
  %gep5264 = add i32 %fr, %gep_array5263
  br label %for.cond4.preheader.i2180

for.cond4.preheader.i2180:                        ; preds = %for.inc14.i2195, %for.cond4.preheader.lr.ph.i2177
  %storemerge118.i2178 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2177 ], [ %inc15.i2193, %for.inc14.i2195 ]
  %cmp515.i2179 = icmp sgt i32 %storemerge118.i2178, 0
  br i1 %cmp515.i2179, label %for.body6.lr.ph.i2182, label %for.inc14.i2195

for.body6.lr.ph.i2182:                            ; preds = %for.cond4.preheader.i2180
  %gep_array5266 = mul i32 %storemerge118.i2178, 2
  %gep5267 = add i32 %Sinewave, %gep_array5266
  br label %for.body6.i2192

for.body6.i2192:                                  ; preds = %for.body6.i2192, %for.body6.lr.ph.i2182
  %storemerge216.i2183 = phi i32 [ 0, %for.body6.lr.ph.i2182 ], [ %add12.i2189, %for.body6.i2192 ]
  %gep5267.asptr = inttoptr i32 %gep5267 to i16*
  %189 = load i16* %gep5267.asptr, align 1
  %conv.i2184 = sext i16 %189 to i32
  %mul.i2185 = mul i32 %conv.i2184, %storemerge.lcssa4517
  %gep5261.asptr = inttoptr i32 %gep5261 to i16*
  %190 = load i16* %gep5261.asptr, align 1
  %conv83.i2186 = zext i16 %190 to i32
  %add.i2187 = add i32 %mul.i2185, %conv83.i2186
  %conv9.i2188 = trunc i32 %add.i2187 to i16
  %gep5264.asptr = inttoptr i32 %gep5264 to i16*
  store i16 %conv9.i2188, i16* %gep5264.asptr, align 1
  %gep5267.asptr66 = inttoptr i32 %gep5267 to i16*
  %191 = load i16* %gep5267.asptr66, align 1
  %add12.i2189 = add i32 %storemerge216.i2183, 1
  %gep_array5269 = mul i32 %add12.i2189, 2
  %gep5270 = add i32 %fr, %gep_array5269
  %gep5270.asptr = inttoptr i32 %gep5270 to i16*
  store i16 %191, i16* %gep5270.asptr, align 1
  %cmp5.i2191 = icmp slt i32 %add12.i2189, %storemerge118.i2178
  br i1 %cmp5.i2191, label %for.body6.i2192, label %for.inc14.i2195

for.inc14.i2195:                                  ; preds = %for.body6.i2192, %for.cond4.preheader.i2180
  %inc15.i2193 = add i32 %storemerge118.i2178, 1
  %cmp2.i2194 = icmp slt i32 %inc15.i2193, %storemerge21.i2171
  br i1 %cmp2.i2194, label %for.cond4.preheader.i2180, label %for.inc17.i2198

for.inc17.i2198:                                  ; preds = %for.inc14.i2195, %for.cond1.preheader.i2173
  %inc18.i2196 = add i32 %storemerge21.i2171, 1
  %cmp.i2197 = icmp slt i32 %inc18.i2196, %storemerge.lcssa4517
  br i1 %cmp.i2197, label %for.cond1.preheader.i2173, label %for.cond1.preheader.i2143

for.cond1.preheader.i2143:                        ; preds = %for.inc17.i2198, %for.inc17.i2168
  %storemerge21.i2141 = phi i32 [ %inc18.i2166, %for.inc17.i2168 ], [ 0, %for.inc17.i2198 ]
  %cmp217.i2142 = icmp sgt i32 %storemerge21.i2141, 0
  br i1 %cmp217.i2142, label %for.cond4.preheader.lr.ph.i2147, label %for.inc17.i2168

for.cond4.preheader.lr.ph.i2147:                  ; preds = %for.cond1.preheader.i2143
  %sub.i2144 = add i32 %storemerge21.i2141, -2
  %gep_array5272 = mul i32 %sub.i2144, 2
  %gep5273 = add i32 %fr, %gep_array5272
  %gep_array5275 = mul i32 %storemerge21.i2141, 2
  %gep5276 = add i32 %fr, %gep_array5275
  br label %for.cond4.preheader.i2150

for.cond4.preheader.i2150:                        ; preds = %for.inc14.i2165, %for.cond4.preheader.lr.ph.i2147
  %storemerge118.i2148 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2147 ], [ %inc15.i2163, %for.inc14.i2165 ]
  %cmp515.i2149 = icmp sgt i32 %storemerge118.i2148, 0
  br i1 %cmp515.i2149, label %for.body6.lr.ph.i2152, label %for.inc14.i2165

for.body6.lr.ph.i2152:                            ; preds = %for.cond4.preheader.i2150
  %gep_array5278 = mul i32 %storemerge118.i2148, 2
  %gep5279 = add i32 %fi, %gep_array5278
  br label %for.body6.i2162

for.body6.i2162:                                  ; preds = %for.body6.i2162, %for.body6.lr.ph.i2152
  %storemerge216.i2153 = phi i32 [ 0, %for.body6.lr.ph.i2152 ], [ %add12.i2159, %for.body6.i2162 ]
  %gep5279.asptr = inttoptr i32 %gep5279 to i16*
  %192 = load i16* %gep5279.asptr, align 1
  %conv.i2154 = sext i16 %192 to i32
  %mul.i2155 = mul i32 %conv.i2154, %storemerge.lcssa4517
  %gep5273.asptr = inttoptr i32 %gep5273 to i16*
  %193 = load i16* %gep5273.asptr, align 1
  %conv83.i2156 = zext i16 %193 to i32
  %add.i2157 = add i32 %mul.i2155, %conv83.i2156
  %conv9.i2158 = trunc i32 %add.i2157 to i16
  %gep5276.asptr = inttoptr i32 %gep5276 to i16*
  store i16 %conv9.i2158, i16* %gep5276.asptr, align 1
  %gep5279.asptr67 = inttoptr i32 %gep5279 to i16*
  %194 = load i16* %gep5279.asptr67, align 1
  %add12.i2159 = add i32 %storemerge216.i2153, 1
  %gep_array5281 = mul i32 %add12.i2159, 2
  %gep5282 = add i32 %fr, %gep_array5281
  %gep5282.asptr = inttoptr i32 %gep5282 to i16*
  store i16 %194, i16* %gep5282.asptr, align 1
  %cmp5.i2161 = icmp slt i32 %add12.i2159, %storemerge118.i2148
  br i1 %cmp5.i2161, label %for.body6.i2162, label %for.inc14.i2165

for.inc14.i2165:                                  ; preds = %for.body6.i2162, %for.cond4.preheader.i2150
  %inc15.i2163 = add i32 %storemerge118.i2148, 1
  %cmp2.i2164 = icmp slt i32 %inc15.i2163, %storemerge21.i2141
  br i1 %cmp2.i2164, label %for.cond4.preheader.i2150, label %for.inc17.i2168

for.inc17.i2168:                                  ; preds = %for.inc14.i2165, %for.cond1.preheader.i2143
  %inc18.i2166 = add i32 %storemerge21.i2141, 1
  %cmp.i2167 = icmp slt i32 %inc18.i2166, %storemerge.lcssa4517
  br i1 %cmp.i2167, label %for.cond1.preheader.i2143, label %for.cond1.preheader.i2113

for.cond1.preheader.i2113:                        ; preds = %for.inc17.i2168, %for.inc17.i2138
  %storemerge21.i2111 = phi i32 [ %inc18.i2136, %for.inc17.i2138 ], [ 0, %for.inc17.i2168 ]
  %cmp217.i2112 = icmp sgt i32 %storemerge21.i2111, 0
  br i1 %cmp217.i2112, label %for.cond4.preheader.lr.ph.i2117, label %for.inc17.i2138

for.cond4.preheader.lr.ph.i2117:                  ; preds = %for.cond1.preheader.i2113
  %sub.i2114 = add i32 %storemerge21.i2111, -2
  %gep_array5284 = mul i32 %sub.i2114, 2
  %gep5285 = add i32 %fr, %gep_array5284
  %gep_array5287 = mul i32 %storemerge21.i2111, 2
  %gep5288 = add i32 %fr, %gep_array5287
  br label %for.cond4.preheader.i2120

for.cond4.preheader.i2120:                        ; preds = %for.inc14.i2135, %for.cond4.preheader.lr.ph.i2117
  %storemerge118.i2118 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2117 ], [ %inc15.i2133, %for.inc14.i2135 ]
  %cmp515.i2119 = icmp sgt i32 %storemerge118.i2118, 0
  br i1 %cmp515.i2119, label %for.body6.lr.ph.i2122, label %for.inc14.i2135

for.body6.lr.ph.i2122:                            ; preds = %for.cond4.preheader.i2120
  %gep_array5290 = mul i32 %storemerge118.i2118, 2
  %gep5291 = add i32 %Sinewave, %gep_array5290
  br label %for.body6.i2132

for.body6.i2132:                                  ; preds = %for.body6.i2132, %for.body6.lr.ph.i2122
  %storemerge216.i2123 = phi i32 [ 0, %for.body6.lr.ph.i2122 ], [ %add12.i2129, %for.body6.i2132 ]
  %gep5291.asptr = inttoptr i32 %gep5291 to i16*
  %195 = load i16* %gep5291.asptr, align 1
  %conv.i2124 = sext i16 %195 to i32
  %mul.i2125 = mul i32 %conv.i2124, %storemerge.lcssa4517
  %gep5285.asptr = inttoptr i32 %gep5285 to i16*
  %196 = load i16* %gep5285.asptr, align 1
  %conv83.i2126 = zext i16 %196 to i32
  %add.i2127 = add i32 %mul.i2125, %conv83.i2126
  %conv9.i2128 = trunc i32 %add.i2127 to i16
  %gep5288.asptr = inttoptr i32 %gep5288 to i16*
  store i16 %conv9.i2128, i16* %gep5288.asptr, align 1
  %gep5291.asptr68 = inttoptr i32 %gep5291 to i16*
  %197 = load i16* %gep5291.asptr68, align 1
  %add12.i2129 = add i32 %storemerge216.i2123, 1
  %gep_array5293 = mul i32 %add12.i2129, 2
  %gep5294 = add i32 %fr, %gep_array5293
  %gep5294.asptr = inttoptr i32 %gep5294 to i16*
  store i16 %197, i16* %gep5294.asptr, align 1
  %cmp5.i2131 = icmp slt i32 %add12.i2129, %storemerge118.i2118
  br i1 %cmp5.i2131, label %for.body6.i2132, label %for.inc14.i2135

for.inc14.i2135:                                  ; preds = %for.body6.i2132, %for.cond4.preheader.i2120
  %inc15.i2133 = add i32 %storemerge118.i2118, 1
  %cmp2.i2134 = icmp slt i32 %inc15.i2133, %storemerge21.i2111
  br i1 %cmp2.i2134, label %for.cond4.preheader.i2120, label %for.inc17.i2138

for.inc17.i2138:                                  ; preds = %for.inc14.i2135, %for.cond1.preheader.i2113
  %inc18.i2136 = add i32 %storemerge21.i2111, 1
  %cmp.i2137 = icmp slt i32 %inc18.i2136, %storemerge.lcssa4517
  br i1 %cmp.i2137, label %for.cond1.preheader.i2113, label %for.cond1.preheader.i2083

for.cond1.preheader.i2083:                        ; preds = %for.inc17.i2138, %for.inc17.i2108
  %storemerge21.i2081 = phi i32 [ %inc18.i2106, %for.inc17.i2108 ], [ 0, %for.inc17.i2138 ]
  %cmp217.i2082 = icmp sgt i32 %storemerge21.i2081, 0
  br i1 %cmp217.i2082, label %for.cond4.preheader.lr.ph.i2087, label %for.inc17.i2108

for.cond4.preheader.lr.ph.i2087:                  ; preds = %for.cond1.preheader.i2083
  %sub.i2084 = add i32 %storemerge21.i2081, -2
  %gep_array5296 = mul i32 %sub.i2084, 2
  %gep5297 = add i32 %fr, %gep_array5296
  %gep_array5299 = mul i32 %storemerge21.i2081, 2
  %gep5300 = add i32 %fr, %gep_array5299
  br label %for.cond4.preheader.i2090

for.cond4.preheader.i2090:                        ; preds = %for.inc14.i2105, %for.cond4.preheader.lr.ph.i2087
  %storemerge118.i2088 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2087 ], [ %inc15.i2103, %for.inc14.i2105 ]
  %cmp515.i2089 = icmp sgt i32 %storemerge118.i2088, 0
  br i1 %cmp515.i2089, label %for.body6.lr.ph.i2092, label %for.inc14.i2105

for.body6.lr.ph.i2092:                            ; preds = %for.cond4.preheader.i2090
  %gep_array5302 = mul i32 %storemerge118.i2088, 2
  %gep5303 = add i32 %Sinewave, %gep_array5302
  br label %for.body6.i2102

for.body6.i2102:                                  ; preds = %for.body6.i2102, %for.body6.lr.ph.i2092
  %storemerge216.i2093 = phi i32 [ 0, %for.body6.lr.ph.i2092 ], [ %add12.i2099, %for.body6.i2102 ]
  %gep5303.asptr = inttoptr i32 %gep5303 to i16*
  %198 = load i16* %gep5303.asptr, align 1
  %conv.i2094 = sext i16 %198 to i32
  %mul.i2095 = mul i32 %conv.i2094, %storemerge.lcssa4517
  %gep5297.asptr = inttoptr i32 %gep5297 to i16*
  %199 = load i16* %gep5297.asptr, align 1
  %conv83.i2096 = zext i16 %199 to i32
  %add.i2097 = add i32 %mul.i2095, %conv83.i2096
  %conv9.i2098 = trunc i32 %add.i2097 to i16
  %gep5300.asptr = inttoptr i32 %gep5300 to i16*
  store i16 %conv9.i2098, i16* %gep5300.asptr, align 1
  %gep5303.asptr69 = inttoptr i32 %gep5303 to i16*
  %200 = load i16* %gep5303.asptr69, align 1
  %add12.i2099 = add i32 %storemerge216.i2093, 1
  %gep_array5305 = mul i32 %add12.i2099, 2
  %gep5306 = add i32 %fr, %gep_array5305
  %gep5306.asptr = inttoptr i32 %gep5306 to i16*
  store i16 %200, i16* %gep5306.asptr, align 1
  %cmp5.i2101 = icmp slt i32 %add12.i2099, %storemerge118.i2088
  br i1 %cmp5.i2101, label %for.body6.i2102, label %for.inc14.i2105

for.inc14.i2105:                                  ; preds = %for.body6.i2102, %for.cond4.preheader.i2090
  %inc15.i2103 = add i32 %storemerge118.i2088, 1
  %cmp2.i2104 = icmp slt i32 %inc15.i2103, %storemerge21.i2081
  br i1 %cmp2.i2104, label %for.cond4.preheader.i2090, label %for.inc17.i2108

for.inc17.i2108:                                  ; preds = %for.inc14.i2105, %for.cond1.preheader.i2083
  %inc18.i2106 = add i32 %storemerge21.i2081, 1
  %cmp.i2107 = icmp slt i32 %inc18.i2106, %storemerge.lcssa4517
  br i1 %cmp.i2107, label %for.cond1.preheader.i2083, label %for.cond1.preheader.i2053

for.cond1.preheader.i2053:                        ; preds = %for.inc17.i2108, %for.inc17.i2078
  %storemerge21.i2051 = phi i32 [ %inc18.i2076, %for.inc17.i2078 ], [ 0, %for.inc17.i2108 ]
  %cmp217.i2052 = icmp sgt i32 %storemerge21.i2051, 0
  br i1 %cmp217.i2052, label %for.cond4.preheader.lr.ph.i2057, label %for.inc17.i2078

for.cond4.preheader.lr.ph.i2057:                  ; preds = %for.cond1.preheader.i2053
  %sub.i2054 = add i32 %storemerge21.i2051, -2
  %gep_array5308 = mul i32 %sub.i2054, 2
  %gep5309 = add i32 %fr, %gep_array5308
  %gep_array5311 = mul i32 %storemerge21.i2051, 2
  %gep5312 = add i32 %fr, %gep_array5311
  br label %for.cond4.preheader.i2060

for.cond4.preheader.i2060:                        ; preds = %for.inc14.i2075, %for.cond4.preheader.lr.ph.i2057
  %storemerge118.i2058 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2057 ], [ %inc15.i2073, %for.inc14.i2075 ]
  %cmp515.i2059 = icmp sgt i32 %storemerge118.i2058, 0
  br i1 %cmp515.i2059, label %for.body6.lr.ph.i2062, label %for.inc14.i2075

for.body6.lr.ph.i2062:                            ; preds = %for.cond4.preheader.i2060
  %gep_array5314 = mul i32 %storemerge118.i2058, 2
  %gep5315 = add i32 %fi, %gep_array5314
  br label %for.body6.i2072

for.body6.i2072:                                  ; preds = %for.body6.i2072, %for.body6.lr.ph.i2062
  %storemerge216.i2063 = phi i32 [ 0, %for.body6.lr.ph.i2062 ], [ %add12.i2069, %for.body6.i2072 ]
  %gep5315.asptr = inttoptr i32 %gep5315 to i16*
  %201 = load i16* %gep5315.asptr, align 1
  %conv.i2064 = sext i16 %201 to i32
  %mul.i2065 = mul i32 %conv.i2064, %storemerge.lcssa4517
  %gep5309.asptr = inttoptr i32 %gep5309 to i16*
  %202 = load i16* %gep5309.asptr, align 1
  %conv83.i2066 = zext i16 %202 to i32
  %add.i2067 = add i32 %mul.i2065, %conv83.i2066
  %conv9.i2068 = trunc i32 %add.i2067 to i16
  %gep5312.asptr = inttoptr i32 %gep5312 to i16*
  store i16 %conv9.i2068, i16* %gep5312.asptr, align 1
  %gep5315.asptr70 = inttoptr i32 %gep5315 to i16*
  %203 = load i16* %gep5315.asptr70, align 1
  %add12.i2069 = add i32 %storemerge216.i2063, 1
  %gep_array5317 = mul i32 %add12.i2069, 2
  %gep5318 = add i32 %fr, %gep_array5317
  %gep5318.asptr = inttoptr i32 %gep5318 to i16*
  store i16 %203, i16* %gep5318.asptr, align 1
  %cmp5.i2071 = icmp slt i32 %add12.i2069, %storemerge118.i2058
  br i1 %cmp5.i2071, label %for.body6.i2072, label %for.inc14.i2075

for.inc14.i2075:                                  ; preds = %for.body6.i2072, %for.cond4.preheader.i2060
  %inc15.i2073 = add i32 %storemerge118.i2058, 1
  %cmp2.i2074 = icmp slt i32 %inc15.i2073, %storemerge21.i2051
  br i1 %cmp2.i2074, label %for.cond4.preheader.i2060, label %for.inc17.i2078

for.inc17.i2078:                                  ; preds = %for.inc14.i2075, %for.cond1.preheader.i2053
  %inc18.i2076 = add i32 %storemerge21.i2051, 1
  %cmp.i2077 = icmp slt i32 %inc18.i2076, %storemerge.lcssa4517
  br i1 %cmp.i2077, label %for.cond1.preheader.i2053, label %for.cond1.preheader.i2023

for.cond1.preheader.i2023:                        ; preds = %for.inc17.i2078, %for.inc17.i2048
  %storemerge21.i2021 = phi i32 [ %inc18.i2046, %for.inc17.i2048 ], [ 0, %for.inc17.i2078 ]
  %cmp217.i2022 = icmp sgt i32 %storemerge21.i2021, 0
  br i1 %cmp217.i2022, label %for.cond4.preheader.lr.ph.i2027, label %for.inc17.i2048

for.cond4.preheader.lr.ph.i2027:                  ; preds = %for.cond1.preheader.i2023
  %sub.i2024 = add i32 %storemerge21.i2021, -2
  %gep_array5320 = mul i32 %sub.i2024, 2
  %gep5321 = add i32 %fr, %gep_array5320
  %gep_array5323 = mul i32 %storemerge21.i2021, 2
  %gep5324 = add i32 %fr, %gep_array5323
  br label %for.cond4.preheader.i2030

for.cond4.preheader.i2030:                        ; preds = %for.inc14.i2045, %for.cond4.preheader.lr.ph.i2027
  %storemerge118.i2028 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i2027 ], [ %inc15.i2043, %for.inc14.i2045 ]
  %cmp515.i2029 = icmp sgt i32 %storemerge118.i2028, 0
  br i1 %cmp515.i2029, label %for.body6.lr.ph.i2032, label %for.inc14.i2045

for.body6.lr.ph.i2032:                            ; preds = %for.cond4.preheader.i2030
  %gep_array5326 = mul i32 %storemerge118.i2028, 2
  %gep5327 = add i32 %Sinewave, %gep_array5326
  br label %for.body6.i2042

for.body6.i2042:                                  ; preds = %for.body6.i2042, %for.body6.lr.ph.i2032
  %storemerge216.i2033 = phi i32 [ 0, %for.body6.lr.ph.i2032 ], [ %add12.i2039, %for.body6.i2042 ]
  %gep5327.asptr = inttoptr i32 %gep5327 to i16*
  %204 = load i16* %gep5327.asptr, align 1
  %conv.i2034 = sext i16 %204 to i32
  %mul.i2035 = mul i32 %conv.i2034, %storemerge.lcssa4517
  %gep5321.asptr = inttoptr i32 %gep5321 to i16*
  %205 = load i16* %gep5321.asptr, align 1
  %conv83.i2036 = zext i16 %205 to i32
  %add.i2037 = add i32 %mul.i2035, %conv83.i2036
  %conv9.i2038 = trunc i32 %add.i2037 to i16
  %gep5324.asptr = inttoptr i32 %gep5324 to i16*
  store i16 %conv9.i2038, i16* %gep5324.asptr, align 1
  %gep5327.asptr71 = inttoptr i32 %gep5327 to i16*
  %206 = load i16* %gep5327.asptr71, align 1
  %add12.i2039 = add i32 %storemerge216.i2033, 1
  %gep_array5329 = mul i32 %add12.i2039, 2
  %gep5330 = add i32 %fr, %gep_array5329
  %gep5330.asptr = inttoptr i32 %gep5330 to i16*
  store i16 %206, i16* %gep5330.asptr, align 1
  %cmp5.i2041 = icmp slt i32 %add12.i2039, %storemerge118.i2028
  br i1 %cmp5.i2041, label %for.body6.i2042, label %for.inc14.i2045

for.inc14.i2045:                                  ; preds = %for.body6.i2042, %for.cond4.preheader.i2030
  %inc15.i2043 = add i32 %storemerge118.i2028, 1
  %cmp2.i2044 = icmp slt i32 %inc15.i2043, %storemerge21.i2021
  br i1 %cmp2.i2044, label %for.cond4.preheader.i2030, label %for.inc17.i2048

for.inc17.i2048:                                  ; preds = %for.inc14.i2045, %for.cond1.preheader.i2023
  %inc18.i2046 = add i32 %storemerge21.i2021, 1
  %cmp.i2047 = icmp slt i32 %inc18.i2046, %storemerge.lcssa4517
  br i1 %cmp.i2047, label %for.cond1.preheader.i2023, label %for.cond1.preheader.i1993

for.cond1.preheader.i1993:                        ; preds = %for.inc17.i2048, %for.inc17.i2018
  %storemerge21.i1991 = phi i32 [ %inc18.i2016, %for.inc17.i2018 ], [ 0, %for.inc17.i2048 ]
  %cmp217.i1992 = icmp sgt i32 %storemerge21.i1991, 0
  br i1 %cmp217.i1992, label %for.cond4.preheader.lr.ph.i1997, label %for.inc17.i2018

for.cond4.preheader.lr.ph.i1997:                  ; preds = %for.cond1.preheader.i1993
  %sub.i1994 = add i32 %storemerge21.i1991, -2
  %gep_array5332 = mul i32 %sub.i1994, 2
  %gep5333 = add i32 %fr, %gep_array5332
  %gep_array5335 = mul i32 %storemerge21.i1991, 2
  %gep5336 = add i32 %fr, %gep_array5335
  br label %for.cond4.preheader.i2000

for.cond4.preheader.i2000:                        ; preds = %for.inc14.i2015, %for.cond4.preheader.lr.ph.i1997
  %storemerge118.i1998 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1997 ], [ %inc15.i2013, %for.inc14.i2015 ]
  %cmp515.i1999 = icmp sgt i32 %storemerge118.i1998, 0
  br i1 %cmp515.i1999, label %for.body6.lr.ph.i2002, label %for.inc14.i2015

for.body6.lr.ph.i2002:                            ; preds = %for.cond4.preheader.i2000
  %gep_array5338 = mul i32 %storemerge118.i1998, 2
  %gep5339 = add i32 %fi, %gep_array5338
  br label %for.body6.i2012

for.body6.i2012:                                  ; preds = %for.body6.i2012, %for.body6.lr.ph.i2002
  %storemerge216.i2003 = phi i32 [ 0, %for.body6.lr.ph.i2002 ], [ %add12.i2009, %for.body6.i2012 ]
  %gep5339.asptr = inttoptr i32 %gep5339 to i16*
  %207 = load i16* %gep5339.asptr, align 1
  %conv.i2004 = sext i16 %207 to i32
  %mul.i2005 = mul i32 %conv.i2004, %storemerge.lcssa4517
  %gep5333.asptr = inttoptr i32 %gep5333 to i16*
  %208 = load i16* %gep5333.asptr, align 1
  %conv83.i2006 = zext i16 %208 to i32
  %add.i2007 = add i32 %mul.i2005, %conv83.i2006
  %conv9.i2008 = trunc i32 %add.i2007 to i16
  %gep5336.asptr = inttoptr i32 %gep5336 to i16*
  store i16 %conv9.i2008, i16* %gep5336.asptr, align 1
  %gep5339.asptr72 = inttoptr i32 %gep5339 to i16*
  %209 = load i16* %gep5339.asptr72, align 1
  %add12.i2009 = add i32 %storemerge216.i2003, 1
  %gep_array5341 = mul i32 %add12.i2009, 2
  %gep5342 = add i32 %fr, %gep_array5341
  %gep5342.asptr = inttoptr i32 %gep5342 to i16*
  store i16 %209, i16* %gep5342.asptr, align 1
  %cmp5.i2011 = icmp slt i32 %add12.i2009, %storemerge118.i1998
  br i1 %cmp5.i2011, label %for.body6.i2012, label %for.inc14.i2015

for.inc14.i2015:                                  ; preds = %for.body6.i2012, %for.cond4.preheader.i2000
  %inc15.i2013 = add i32 %storemerge118.i1998, 1
  %cmp2.i2014 = icmp slt i32 %inc15.i2013, %storemerge21.i1991
  br i1 %cmp2.i2014, label %for.cond4.preheader.i2000, label %for.inc17.i2018

for.inc17.i2018:                                  ; preds = %for.inc14.i2015, %for.cond1.preheader.i1993
  %inc18.i2016 = add i32 %storemerge21.i1991, 1
  %cmp.i2017 = icmp slt i32 %inc18.i2016, %storemerge.lcssa4517
  br i1 %cmp.i2017, label %for.cond1.preheader.i1993, label %for.cond1.preheader.i1963

for.cond1.preheader.i1963:                        ; preds = %for.inc17.i2018, %for.inc17.i1988
  %storemerge21.i1961 = phi i32 [ %inc18.i1986, %for.inc17.i1988 ], [ 0, %for.inc17.i2018 ]
  %cmp217.i1962 = icmp sgt i32 %storemerge21.i1961, 0
  br i1 %cmp217.i1962, label %for.cond4.preheader.lr.ph.i1967, label %for.inc17.i1988

for.cond4.preheader.lr.ph.i1967:                  ; preds = %for.cond1.preheader.i1963
  %sub.i1964 = add i32 %storemerge21.i1961, -2
  %gep_array5344 = mul i32 %sub.i1964, 2
  %gep5345 = add i32 %fr, %gep_array5344
  %gep_array5347 = mul i32 %storemerge21.i1961, 2
  %gep5348 = add i32 %fr, %gep_array5347
  br label %for.cond4.preheader.i1970

for.cond4.preheader.i1970:                        ; preds = %for.inc14.i1985, %for.cond4.preheader.lr.ph.i1967
  %storemerge118.i1968 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1967 ], [ %inc15.i1983, %for.inc14.i1985 ]
  %cmp515.i1969 = icmp sgt i32 %storemerge118.i1968, 0
  br i1 %cmp515.i1969, label %for.body6.lr.ph.i1972, label %for.inc14.i1985

for.body6.lr.ph.i1972:                            ; preds = %for.cond4.preheader.i1970
  %gep_array5350 = mul i32 %storemerge118.i1968, 2
  %gep5351 = add i32 %Sinewave, %gep_array5350
  br label %for.body6.i1982

for.body6.i1982:                                  ; preds = %for.body6.i1982, %for.body6.lr.ph.i1972
  %storemerge216.i1973 = phi i32 [ 0, %for.body6.lr.ph.i1972 ], [ %add12.i1979, %for.body6.i1982 ]
  %gep5351.asptr = inttoptr i32 %gep5351 to i16*
  %210 = load i16* %gep5351.asptr, align 1
  %conv.i1974 = sext i16 %210 to i32
  %mul.i1975 = mul i32 %conv.i1974, %storemerge.lcssa4517
  %gep5345.asptr = inttoptr i32 %gep5345 to i16*
  %211 = load i16* %gep5345.asptr, align 1
  %conv83.i1976 = zext i16 %211 to i32
  %add.i1977 = add i32 %mul.i1975, %conv83.i1976
  %conv9.i1978 = trunc i32 %add.i1977 to i16
  %gep5348.asptr = inttoptr i32 %gep5348 to i16*
  store i16 %conv9.i1978, i16* %gep5348.asptr, align 1
  %gep5351.asptr73 = inttoptr i32 %gep5351 to i16*
  %212 = load i16* %gep5351.asptr73, align 1
  %add12.i1979 = add i32 %storemerge216.i1973, 1
  %gep_array5353 = mul i32 %add12.i1979, 2
  %gep5354 = add i32 %fr, %gep_array5353
  %gep5354.asptr = inttoptr i32 %gep5354 to i16*
  store i16 %212, i16* %gep5354.asptr, align 1
  %cmp5.i1981 = icmp slt i32 %add12.i1979, %storemerge118.i1968
  br i1 %cmp5.i1981, label %for.body6.i1982, label %for.inc14.i1985

for.inc14.i1985:                                  ; preds = %for.body6.i1982, %for.cond4.preheader.i1970
  %inc15.i1983 = add i32 %storemerge118.i1968, 1
  %cmp2.i1984 = icmp slt i32 %inc15.i1983, %storemerge21.i1961
  br i1 %cmp2.i1984, label %for.cond4.preheader.i1970, label %for.inc17.i1988

for.inc17.i1988:                                  ; preds = %for.inc14.i1985, %for.cond1.preheader.i1963
  %inc18.i1986 = add i32 %storemerge21.i1961, 1
  %cmp.i1987 = icmp slt i32 %inc18.i1986, %storemerge.lcssa4517
  br i1 %cmp.i1987, label %for.cond1.preheader.i1963, label %for.cond1.preheader.i1933

for.cond1.preheader.i1933:                        ; preds = %for.inc17.i1988, %for.inc17.i1958
  %storemerge21.i1931 = phi i32 [ %inc18.i1956, %for.inc17.i1958 ], [ 0, %for.inc17.i1988 ]
  %cmp217.i1932 = icmp sgt i32 %storemerge21.i1931, 0
  br i1 %cmp217.i1932, label %for.cond4.preheader.lr.ph.i1937, label %for.inc17.i1958

for.cond4.preheader.lr.ph.i1937:                  ; preds = %for.cond1.preheader.i1933
  %sub.i1934 = add i32 %storemerge21.i1931, -2
  %gep_array5356 = mul i32 %sub.i1934, 2
  %gep5357 = add i32 %fr, %gep_array5356
  %gep_array5359 = mul i32 %storemerge21.i1931, 2
  %gep5360 = add i32 %fr, %gep_array5359
  br label %for.cond4.preheader.i1940

for.cond4.preheader.i1940:                        ; preds = %for.inc14.i1955, %for.cond4.preheader.lr.ph.i1937
  %storemerge118.i1938 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1937 ], [ %inc15.i1953, %for.inc14.i1955 ]
  %cmp515.i1939 = icmp sgt i32 %storemerge118.i1938, 0
  br i1 %cmp515.i1939, label %for.body6.lr.ph.i1942, label %for.inc14.i1955

for.body6.lr.ph.i1942:                            ; preds = %for.cond4.preheader.i1940
  %gep_array5362 = mul i32 %storemerge118.i1938, 2
  %gep5363 = add i32 %fi, %gep_array5362
  br label %for.body6.i1952

for.body6.i1952:                                  ; preds = %for.body6.i1952, %for.body6.lr.ph.i1942
  %storemerge216.i1943 = phi i32 [ 0, %for.body6.lr.ph.i1942 ], [ %add12.i1949, %for.body6.i1952 ]
  %gep5363.asptr = inttoptr i32 %gep5363 to i16*
  %213 = load i16* %gep5363.asptr, align 1
  %conv.i1944 = sext i16 %213 to i32
  %mul.i1945 = mul i32 %conv.i1944, %storemerge.lcssa4517
  %gep5357.asptr = inttoptr i32 %gep5357 to i16*
  %214 = load i16* %gep5357.asptr, align 1
  %conv83.i1946 = zext i16 %214 to i32
  %add.i1947 = add i32 %mul.i1945, %conv83.i1946
  %conv9.i1948 = trunc i32 %add.i1947 to i16
  %gep5360.asptr = inttoptr i32 %gep5360 to i16*
  store i16 %conv9.i1948, i16* %gep5360.asptr, align 1
  %gep5363.asptr74 = inttoptr i32 %gep5363 to i16*
  %215 = load i16* %gep5363.asptr74, align 1
  %add12.i1949 = add i32 %storemerge216.i1943, 1
  %gep_array5365 = mul i32 %add12.i1949, 2
  %gep5366 = add i32 %fr, %gep_array5365
  %gep5366.asptr = inttoptr i32 %gep5366 to i16*
  store i16 %215, i16* %gep5366.asptr, align 1
  %cmp5.i1951 = icmp slt i32 %add12.i1949, %storemerge118.i1938
  br i1 %cmp5.i1951, label %for.body6.i1952, label %for.inc14.i1955

for.inc14.i1955:                                  ; preds = %for.body6.i1952, %for.cond4.preheader.i1940
  %inc15.i1953 = add i32 %storemerge118.i1938, 1
  %cmp2.i1954 = icmp slt i32 %inc15.i1953, %storemerge21.i1931
  br i1 %cmp2.i1954, label %for.cond4.preheader.i1940, label %for.inc17.i1958

for.inc17.i1958:                                  ; preds = %for.inc14.i1955, %for.cond1.preheader.i1933
  %inc18.i1956 = add i32 %storemerge21.i1931, 1
  %cmp.i1957 = icmp slt i32 %inc18.i1956, %storemerge.lcssa4517
  br i1 %cmp.i1957, label %for.cond1.preheader.i1933, label %for.cond1.preheader.i1903

for.cond1.preheader.i1903:                        ; preds = %for.inc17.i1958, %for.inc17.i1928
  %storemerge21.i1901 = phi i32 [ %inc18.i1926, %for.inc17.i1928 ], [ 0, %for.inc17.i1958 ]
  %cmp217.i1902 = icmp sgt i32 %storemerge21.i1901, 0
  br i1 %cmp217.i1902, label %for.cond4.preheader.lr.ph.i1907, label %for.inc17.i1928

for.cond4.preheader.lr.ph.i1907:                  ; preds = %for.cond1.preheader.i1903
  %sub.i1904 = add i32 %storemerge21.i1901, -2
  %gep_array5368 = mul i32 %sub.i1904, 2
  %gep5369 = add i32 %fr, %gep_array5368
  %gep_array5371 = mul i32 %storemerge21.i1901, 2
  %gep5372 = add i32 %fr, %gep_array5371
  br label %for.cond4.preheader.i1910

for.cond4.preheader.i1910:                        ; preds = %for.inc14.i1925, %for.cond4.preheader.lr.ph.i1907
  %storemerge118.i1908 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1907 ], [ %inc15.i1923, %for.inc14.i1925 ]
  %cmp515.i1909 = icmp sgt i32 %storemerge118.i1908, 0
  br i1 %cmp515.i1909, label %for.body6.lr.ph.i1912, label %for.inc14.i1925

for.body6.lr.ph.i1912:                            ; preds = %for.cond4.preheader.i1910
  %gep_array5374 = mul i32 %storemerge118.i1908, 2
  %gep5375 = add i32 %Sinewave, %gep_array5374
  br label %for.body6.i1922

for.body6.i1922:                                  ; preds = %for.body6.i1922, %for.body6.lr.ph.i1912
  %storemerge216.i1913 = phi i32 [ 0, %for.body6.lr.ph.i1912 ], [ %add12.i1919, %for.body6.i1922 ]
  %gep5375.asptr = inttoptr i32 %gep5375 to i16*
  %216 = load i16* %gep5375.asptr, align 1
  %conv.i1914 = sext i16 %216 to i32
  %mul.i1915 = mul i32 %conv.i1914, %storemerge.lcssa4517
  %gep5369.asptr = inttoptr i32 %gep5369 to i16*
  %217 = load i16* %gep5369.asptr, align 1
  %conv83.i1916 = zext i16 %217 to i32
  %add.i1917 = add i32 %mul.i1915, %conv83.i1916
  %conv9.i1918 = trunc i32 %add.i1917 to i16
  %gep5372.asptr = inttoptr i32 %gep5372 to i16*
  store i16 %conv9.i1918, i16* %gep5372.asptr, align 1
  %gep5375.asptr75 = inttoptr i32 %gep5375 to i16*
  %218 = load i16* %gep5375.asptr75, align 1
  %add12.i1919 = add i32 %storemerge216.i1913, 1
  %gep_array5377 = mul i32 %add12.i1919, 2
  %gep5378 = add i32 %fr, %gep_array5377
  %gep5378.asptr = inttoptr i32 %gep5378 to i16*
  store i16 %218, i16* %gep5378.asptr, align 1
  %cmp5.i1921 = icmp slt i32 %add12.i1919, %storemerge118.i1908
  br i1 %cmp5.i1921, label %for.body6.i1922, label %for.inc14.i1925

for.inc14.i1925:                                  ; preds = %for.body6.i1922, %for.cond4.preheader.i1910
  %inc15.i1923 = add i32 %storemerge118.i1908, 1
  %cmp2.i1924 = icmp slt i32 %inc15.i1923, %storemerge21.i1901
  br i1 %cmp2.i1924, label %for.cond4.preheader.i1910, label %for.inc17.i1928

for.inc17.i1928:                                  ; preds = %for.inc14.i1925, %for.cond1.preheader.i1903
  %inc18.i1926 = add i32 %storemerge21.i1901, 1
  %cmp.i1927 = icmp slt i32 %inc18.i1926, %storemerge.lcssa4517
  br i1 %cmp.i1927, label %for.cond1.preheader.i1903, label %for.cond1.preheader.i1873

for.cond1.preheader.i1873:                        ; preds = %for.inc17.i1928, %for.inc17.i1898
  %storemerge21.i1871 = phi i32 [ %inc18.i1896, %for.inc17.i1898 ], [ 0, %for.inc17.i1928 ]
  %cmp217.i1872 = icmp sgt i32 %storemerge21.i1871, 0
  br i1 %cmp217.i1872, label %for.cond4.preheader.lr.ph.i1877, label %for.inc17.i1898

for.cond4.preheader.lr.ph.i1877:                  ; preds = %for.cond1.preheader.i1873
  %sub.i1874 = add i32 %storemerge21.i1871, -2
  %gep_array5380 = mul i32 %sub.i1874, 2
  %gep5381 = add i32 %fr, %gep_array5380
  %gep_array5383 = mul i32 %storemerge21.i1871, 2
  %gep5384 = add i32 %fr, %gep_array5383
  br label %for.cond4.preheader.i1880

for.cond4.preheader.i1880:                        ; preds = %for.inc14.i1895, %for.cond4.preheader.lr.ph.i1877
  %storemerge118.i1878 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1877 ], [ %inc15.i1893, %for.inc14.i1895 ]
  %cmp515.i1879 = icmp sgt i32 %storemerge118.i1878, 0
  br i1 %cmp515.i1879, label %for.body6.lr.ph.i1882, label %for.inc14.i1895

for.body6.lr.ph.i1882:                            ; preds = %for.cond4.preheader.i1880
  %gep_array5386 = mul i32 %storemerge118.i1878, 2
  %gep5387 = add i32 %fi, %gep_array5386
  br label %for.body6.i1892

for.body6.i1892:                                  ; preds = %for.body6.i1892, %for.body6.lr.ph.i1882
  %storemerge216.i1883 = phi i32 [ 0, %for.body6.lr.ph.i1882 ], [ %add12.i1889, %for.body6.i1892 ]
  %gep5387.asptr = inttoptr i32 %gep5387 to i16*
  %219 = load i16* %gep5387.asptr, align 1
  %conv.i1884 = sext i16 %219 to i32
  %mul.i1885 = mul i32 %conv.i1884, %storemerge.lcssa4517
  %gep5381.asptr = inttoptr i32 %gep5381 to i16*
  %220 = load i16* %gep5381.asptr, align 1
  %conv83.i1886 = zext i16 %220 to i32
  %add.i1887 = add i32 %mul.i1885, %conv83.i1886
  %conv9.i1888 = trunc i32 %add.i1887 to i16
  %gep5384.asptr = inttoptr i32 %gep5384 to i16*
  store i16 %conv9.i1888, i16* %gep5384.asptr, align 1
  %gep5387.asptr76 = inttoptr i32 %gep5387 to i16*
  %221 = load i16* %gep5387.asptr76, align 1
  %add12.i1889 = add i32 %storemerge216.i1883, 1
  %gep_array5389 = mul i32 %add12.i1889, 2
  %gep5390 = add i32 %fr, %gep_array5389
  %gep5390.asptr = inttoptr i32 %gep5390 to i16*
  store i16 %221, i16* %gep5390.asptr, align 1
  %cmp5.i1891 = icmp slt i32 %add12.i1889, %storemerge118.i1878
  br i1 %cmp5.i1891, label %for.body6.i1892, label %for.inc14.i1895

for.inc14.i1895:                                  ; preds = %for.body6.i1892, %for.cond4.preheader.i1880
  %inc15.i1893 = add i32 %storemerge118.i1878, 1
  %cmp2.i1894 = icmp slt i32 %inc15.i1893, %storemerge21.i1871
  br i1 %cmp2.i1894, label %for.cond4.preheader.i1880, label %for.inc17.i1898

for.inc17.i1898:                                  ; preds = %for.inc14.i1895, %for.cond1.preheader.i1873
  %inc18.i1896 = add i32 %storemerge21.i1871, 1
  %cmp.i1897 = icmp slt i32 %inc18.i1896, %storemerge.lcssa4517
  br i1 %cmp.i1897, label %for.cond1.preheader.i1873, label %for.cond1.preheader.i1843

for.cond1.preheader.i1843:                        ; preds = %for.inc17.i1898, %for.inc17.i1868
  %storemerge21.i1841 = phi i32 [ %inc18.i1866, %for.inc17.i1868 ], [ 0, %for.inc17.i1898 ]
  %cmp217.i1842 = icmp sgt i32 %storemerge21.i1841, 0
  br i1 %cmp217.i1842, label %for.cond4.preheader.lr.ph.i1847, label %for.inc17.i1868

for.cond4.preheader.lr.ph.i1847:                  ; preds = %for.cond1.preheader.i1843
  %sub.i1844 = add i32 %storemerge21.i1841, -2
  %gep_array5392 = mul i32 %sub.i1844, 2
  %gep5393 = add i32 %fr, %gep_array5392
  %gep_array5395 = mul i32 %storemerge21.i1841, 2
  %gep5396 = add i32 %fr, %gep_array5395
  br label %for.cond4.preheader.i1850

for.cond4.preheader.i1850:                        ; preds = %for.inc14.i1865, %for.cond4.preheader.lr.ph.i1847
  %storemerge118.i1848 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1847 ], [ %inc15.i1863, %for.inc14.i1865 ]
  %cmp515.i1849 = icmp sgt i32 %storemerge118.i1848, 0
  br i1 %cmp515.i1849, label %for.body6.lr.ph.i1852, label %for.inc14.i1865

for.body6.lr.ph.i1852:                            ; preds = %for.cond4.preheader.i1850
  %gep_array5398 = mul i32 %storemerge118.i1848, 2
  %gep5399 = add i32 %Sinewave, %gep_array5398
  br label %for.body6.i1862

for.body6.i1862:                                  ; preds = %for.body6.i1862, %for.body6.lr.ph.i1852
  %storemerge216.i1853 = phi i32 [ 0, %for.body6.lr.ph.i1852 ], [ %add12.i1859, %for.body6.i1862 ]
  %gep5399.asptr = inttoptr i32 %gep5399 to i16*
  %222 = load i16* %gep5399.asptr, align 1
  %conv.i1854 = sext i16 %222 to i32
  %mul.i1855 = mul i32 %conv.i1854, %storemerge.lcssa4517
  %gep5393.asptr = inttoptr i32 %gep5393 to i16*
  %223 = load i16* %gep5393.asptr, align 1
  %conv83.i1856 = zext i16 %223 to i32
  %add.i1857 = add i32 %mul.i1855, %conv83.i1856
  %conv9.i1858 = trunc i32 %add.i1857 to i16
  %gep5396.asptr = inttoptr i32 %gep5396 to i16*
  store i16 %conv9.i1858, i16* %gep5396.asptr, align 1
  %gep5399.asptr77 = inttoptr i32 %gep5399 to i16*
  %224 = load i16* %gep5399.asptr77, align 1
  %add12.i1859 = add i32 %storemerge216.i1853, 1
  %gep_array5401 = mul i32 %add12.i1859, 2
  %gep5402 = add i32 %fr, %gep_array5401
  %gep5402.asptr = inttoptr i32 %gep5402 to i16*
  store i16 %224, i16* %gep5402.asptr, align 1
  %cmp5.i1861 = icmp slt i32 %add12.i1859, %storemerge118.i1848
  br i1 %cmp5.i1861, label %for.body6.i1862, label %for.inc14.i1865

for.inc14.i1865:                                  ; preds = %for.body6.i1862, %for.cond4.preheader.i1850
  %inc15.i1863 = add i32 %storemerge118.i1848, 1
  %cmp2.i1864 = icmp slt i32 %inc15.i1863, %storemerge21.i1841
  br i1 %cmp2.i1864, label %for.cond4.preheader.i1850, label %for.inc17.i1868

for.inc17.i1868:                                  ; preds = %for.inc14.i1865, %for.cond1.preheader.i1843
  %inc18.i1866 = add i32 %storemerge21.i1841, 1
  %cmp.i1867 = icmp slt i32 %inc18.i1866, %storemerge.lcssa4517
  br i1 %cmp.i1867, label %for.cond1.preheader.i1843, label %for.cond1.preheader.i1813

for.cond1.preheader.i1813:                        ; preds = %for.inc17.i1868, %for.inc17.i1838
  %storemerge21.i1811 = phi i32 [ %inc18.i1836, %for.inc17.i1838 ], [ 0, %for.inc17.i1868 ]
  %cmp217.i1812 = icmp sgt i32 %storemerge21.i1811, 0
  br i1 %cmp217.i1812, label %for.cond4.preheader.lr.ph.i1817, label %for.inc17.i1838

for.cond4.preheader.lr.ph.i1817:                  ; preds = %for.cond1.preheader.i1813
  %sub.i1814 = add i32 %storemerge21.i1811, -2
  %gep_array5404 = mul i32 %sub.i1814, 2
  %gep5405 = add i32 %fr, %gep_array5404
  %gep_array5407 = mul i32 %storemerge21.i1811, 2
  %gep5408 = add i32 %fr, %gep_array5407
  br label %for.cond4.preheader.i1820

for.cond4.preheader.i1820:                        ; preds = %for.inc14.i1835, %for.cond4.preheader.lr.ph.i1817
  %storemerge118.i1818 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1817 ], [ %inc15.i1833, %for.inc14.i1835 ]
  %cmp515.i1819 = icmp sgt i32 %storemerge118.i1818, 0
  br i1 %cmp515.i1819, label %for.body6.lr.ph.i1822, label %for.inc14.i1835

for.body6.lr.ph.i1822:                            ; preds = %for.cond4.preheader.i1820
  %gep_array5410 = mul i32 %storemerge118.i1818, 2
  %gep5411 = add i32 %fi, %gep_array5410
  br label %for.body6.i1832

for.body6.i1832:                                  ; preds = %for.body6.i1832, %for.body6.lr.ph.i1822
  %storemerge216.i1823 = phi i32 [ 0, %for.body6.lr.ph.i1822 ], [ %add12.i1829, %for.body6.i1832 ]
  %gep5411.asptr = inttoptr i32 %gep5411 to i16*
  %225 = load i16* %gep5411.asptr, align 1
  %conv.i1824 = sext i16 %225 to i32
  %mul.i1825 = mul i32 %conv.i1824, %storemerge.lcssa4517
  %gep5405.asptr = inttoptr i32 %gep5405 to i16*
  %226 = load i16* %gep5405.asptr, align 1
  %conv83.i1826 = zext i16 %226 to i32
  %add.i1827 = add i32 %mul.i1825, %conv83.i1826
  %conv9.i1828 = trunc i32 %add.i1827 to i16
  %gep5408.asptr = inttoptr i32 %gep5408 to i16*
  store i16 %conv9.i1828, i16* %gep5408.asptr, align 1
  %gep5411.asptr78 = inttoptr i32 %gep5411 to i16*
  %227 = load i16* %gep5411.asptr78, align 1
  %add12.i1829 = add i32 %storemerge216.i1823, 1
  %gep_array5413 = mul i32 %add12.i1829, 2
  %gep5414 = add i32 %fr, %gep_array5413
  %gep5414.asptr = inttoptr i32 %gep5414 to i16*
  store i16 %227, i16* %gep5414.asptr, align 1
  %cmp5.i1831 = icmp slt i32 %add12.i1829, %storemerge118.i1818
  br i1 %cmp5.i1831, label %for.body6.i1832, label %for.inc14.i1835

for.inc14.i1835:                                  ; preds = %for.body6.i1832, %for.cond4.preheader.i1820
  %inc15.i1833 = add i32 %storemerge118.i1818, 1
  %cmp2.i1834 = icmp slt i32 %inc15.i1833, %storemerge21.i1811
  br i1 %cmp2.i1834, label %for.cond4.preheader.i1820, label %for.inc17.i1838

for.inc17.i1838:                                  ; preds = %for.inc14.i1835, %for.cond1.preheader.i1813
  %inc18.i1836 = add i32 %storemerge21.i1811, 1
  %cmp.i1837 = icmp slt i32 %inc18.i1836, %storemerge.lcssa4517
  br i1 %cmp.i1837, label %for.cond1.preheader.i1813, label %for.cond1.preheader.i1783

for.cond1.preheader.i1783:                        ; preds = %for.inc17.i1838, %for.inc17.i1808
  %storemerge21.i1781 = phi i32 [ %inc18.i1806, %for.inc17.i1808 ], [ 0, %for.inc17.i1838 ]
  %cmp217.i1782 = icmp sgt i32 %storemerge21.i1781, 0
  br i1 %cmp217.i1782, label %for.cond4.preheader.lr.ph.i1787, label %for.inc17.i1808

for.cond4.preheader.lr.ph.i1787:                  ; preds = %for.cond1.preheader.i1783
  %sub.i1784 = add i32 %storemerge21.i1781, -2
  %gep_array5416 = mul i32 %sub.i1784, 2
  %gep5417 = add i32 %fr, %gep_array5416
  %gep_array5419 = mul i32 %storemerge21.i1781, 2
  %gep5420 = add i32 %fr, %gep_array5419
  br label %for.cond4.preheader.i1790

for.cond4.preheader.i1790:                        ; preds = %for.inc14.i1805, %for.cond4.preheader.lr.ph.i1787
  %storemerge118.i1788 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1787 ], [ %inc15.i1803, %for.inc14.i1805 ]
  %cmp515.i1789 = icmp sgt i32 %storemerge118.i1788, 0
  br i1 %cmp515.i1789, label %for.body6.lr.ph.i1792, label %for.inc14.i1805

for.body6.lr.ph.i1792:                            ; preds = %for.cond4.preheader.i1790
  %gep_array5422 = mul i32 %storemerge118.i1788, 2
  %gep5423 = add i32 %Sinewave, %gep_array5422
  br label %for.body6.i1802

for.body6.i1802:                                  ; preds = %for.body6.i1802, %for.body6.lr.ph.i1792
  %storemerge216.i1793 = phi i32 [ 0, %for.body6.lr.ph.i1792 ], [ %add12.i1799, %for.body6.i1802 ]
  %gep5423.asptr = inttoptr i32 %gep5423 to i16*
  %228 = load i16* %gep5423.asptr, align 1
  %conv.i1794 = sext i16 %228 to i32
  %mul.i1795 = mul i32 %conv.i1794, %storemerge.lcssa4517
  %gep5417.asptr = inttoptr i32 %gep5417 to i16*
  %229 = load i16* %gep5417.asptr, align 1
  %conv83.i1796 = zext i16 %229 to i32
  %add.i1797 = add i32 %mul.i1795, %conv83.i1796
  %conv9.i1798 = trunc i32 %add.i1797 to i16
  %gep5420.asptr = inttoptr i32 %gep5420 to i16*
  store i16 %conv9.i1798, i16* %gep5420.asptr, align 1
  %gep5423.asptr79 = inttoptr i32 %gep5423 to i16*
  %230 = load i16* %gep5423.asptr79, align 1
  %add12.i1799 = add i32 %storemerge216.i1793, 1
  %gep_array5425 = mul i32 %add12.i1799, 2
  %gep5426 = add i32 %fr, %gep_array5425
  %gep5426.asptr = inttoptr i32 %gep5426 to i16*
  store i16 %230, i16* %gep5426.asptr, align 1
  %cmp5.i1801 = icmp slt i32 %add12.i1799, %storemerge118.i1788
  br i1 %cmp5.i1801, label %for.body6.i1802, label %for.inc14.i1805

for.inc14.i1805:                                  ; preds = %for.body6.i1802, %for.cond4.preheader.i1790
  %inc15.i1803 = add i32 %storemerge118.i1788, 1
  %cmp2.i1804 = icmp slt i32 %inc15.i1803, %storemerge21.i1781
  br i1 %cmp2.i1804, label %for.cond4.preheader.i1790, label %for.inc17.i1808

for.inc17.i1808:                                  ; preds = %for.inc14.i1805, %for.cond1.preheader.i1783
  %inc18.i1806 = add i32 %storemerge21.i1781, 1
  %cmp.i1807 = icmp slt i32 %inc18.i1806, %storemerge.lcssa4517
  br i1 %cmp.i1807, label %for.cond1.preheader.i1783, label %for.cond1.preheader.i1753

for.cond1.preheader.i1753:                        ; preds = %for.inc17.i1808, %for.inc17.i1778
  %storemerge21.i1751 = phi i32 [ %inc18.i1776, %for.inc17.i1778 ], [ 0, %for.inc17.i1808 ]
  %cmp217.i1752 = icmp sgt i32 %storemerge21.i1751, 0
  br i1 %cmp217.i1752, label %for.cond4.preheader.lr.ph.i1757, label %for.inc17.i1778

for.cond4.preheader.lr.ph.i1757:                  ; preds = %for.cond1.preheader.i1753
  %sub.i1754 = add i32 %storemerge21.i1751, -2
  %gep_array5428 = mul i32 %sub.i1754, 2
  %gep5429 = add i32 %fr, %gep_array5428
  %gep_array5431 = mul i32 %storemerge21.i1751, 2
  %gep5432 = add i32 %fr, %gep_array5431
  br label %for.cond4.preheader.i1760

for.cond4.preheader.i1760:                        ; preds = %for.inc14.i1775, %for.cond4.preheader.lr.ph.i1757
  %storemerge118.i1758 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1757 ], [ %inc15.i1773, %for.inc14.i1775 ]
  %cmp515.i1759 = icmp sgt i32 %storemerge118.i1758, 0
  br i1 %cmp515.i1759, label %for.body6.lr.ph.i1762, label %for.inc14.i1775

for.body6.lr.ph.i1762:                            ; preds = %for.cond4.preheader.i1760
  %gep_array5434 = mul i32 %storemerge118.i1758, 2
  %gep5435 = add i32 %Sinewave, %gep_array5434
  br label %for.body6.i1772

for.body6.i1772:                                  ; preds = %for.body6.i1772, %for.body6.lr.ph.i1762
  %storemerge216.i1763 = phi i32 [ 0, %for.body6.lr.ph.i1762 ], [ %add12.i1769, %for.body6.i1772 ]
  %gep5435.asptr = inttoptr i32 %gep5435 to i16*
  %231 = load i16* %gep5435.asptr, align 1
  %conv.i1764 = sext i16 %231 to i32
  %mul.i1765 = mul i32 %conv.i1764, %storemerge.lcssa4517
  %gep5429.asptr = inttoptr i32 %gep5429 to i16*
  %232 = load i16* %gep5429.asptr, align 1
  %conv83.i1766 = zext i16 %232 to i32
  %add.i1767 = add i32 %mul.i1765, %conv83.i1766
  %conv9.i1768 = trunc i32 %add.i1767 to i16
  %gep5432.asptr = inttoptr i32 %gep5432 to i16*
  store i16 %conv9.i1768, i16* %gep5432.asptr, align 1
  %gep5435.asptr80 = inttoptr i32 %gep5435 to i16*
  %233 = load i16* %gep5435.asptr80, align 1
  %add12.i1769 = add i32 %storemerge216.i1763, 1
  %gep_array5437 = mul i32 %add12.i1769, 2
  %gep5438 = add i32 %fr, %gep_array5437
  %gep5438.asptr = inttoptr i32 %gep5438 to i16*
  store i16 %233, i16* %gep5438.asptr, align 1
  %cmp5.i1771 = icmp slt i32 %add12.i1769, %storemerge118.i1758
  br i1 %cmp5.i1771, label %for.body6.i1772, label %for.inc14.i1775

for.inc14.i1775:                                  ; preds = %for.body6.i1772, %for.cond4.preheader.i1760
  %inc15.i1773 = add i32 %storemerge118.i1758, 1
  %cmp2.i1774 = icmp slt i32 %inc15.i1773, %storemerge21.i1751
  br i1 %cmp2.i1774, label %for.cond4.preheader.i1760, label %for.inc17.i1778

for.inc17.i1778:                                  ; preds = %for.inc14.i1775, %for.cond1.preheader.i1753
  %inc18.i1776 = add i32 %storemerge21.i1751, 1
  %cmp.i1777 = icmp slt i32 %inc18.i1776, %storemerge.lcssa4517
  br i1 %cmp.i1777, label %for.cond1.preheader.i1753, label %for.cond1.preheader.i1723

for.cond1.preheader.i1723:                        ; preds = %for.inc17.i1778, %for.inc17.i1748
  %storemerge21.i1721 = phi i32 [ %inc18.i1746, %for.inc17.i1748 ], [ 0, %for.inc17.i1778 ]
  %cmp217.i1722 = icmp sgt i32 %storemerge21.i1721, 0
  br i1 %cmp217.i1722, label %for.cond4.preheader.lr.ph.i1727, label %for.inc17.i1748

for.cond4.preheader.lr.ph.i1727:                  ; preds = %for.cond1.preheader.i1723
  %sub.i1724 = add i32 %storemerge21.i1721, -2
  %gep_array5440 = mul i32 %sub.i1724, 2
  %gep5441 = add i32 %fr, %gep_array5440
  %gep_array5443 = mul i32 %storemerge21.i1721, 2
  %gep5444 = add i32 %fr, %gep_array5443
  br label %for.cond4.preheader.i1730

for.cond4.preheader.i1730:                        ; preds = %for.inc14.i1745, %for.cond4.preheader.lr.ph.i1727
  %storemerge118.i1728 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1727 ], [ %inc15.i1743, %for.inc14.i1745 ]
  %cmp515.i1729 = icmp sgt i32 %storemerge118.i1728, 0
  br i1 %cmp515.i1729, label %for.body6.lr.ph.i1732, label %for.inc14.i1745

for.body6.lr.ph.i1732:                            ; preds = %for.cond4.preheader.i1730
  %gep_array5446 = mul i32 %storemerge118.i1728, 2
  %gep5447 = add i32 %fi, %gep_array5446
  br label %for.body6.i1742

for.body6.i1742:                                  ; preds = %for.body6.i1742, %for.body6.lr.ph.i1732
  %storemerge216.i1733 = phi i32 [ 0, %for.body6.lr.ph.i1732 ], [ %add12.i1739, %for.body6.i1742 ]
  %gep5447.asptr = inttoptr i32 %gep5447 to i16*
  %234 = load i16* %gep5447.asptr, align 1
  %conv.i1734 = sext i16 %234 to i32
  %mul.i1735 = mul i32 %conv.i1734, %storemerge.lcssa4517
  %gep5441.asptr = inttoptr i32 %gep5441 to i16*
  %235 = load i16* %gep5441.asptr, align 1
  %conv83.i1736 = zext i16 %235 to i32
  %add.i1737 = add i32 %mul.i1735, %conv83.i1736
  %conv9.i1738 = trunc i32 %add.i1737 to i16
  %gep5444.asptr = inttoptr i32 %gep5444 to i16*
  store i16 %conv9.i1738, i16* %gep5444.asptr, align 1
  %gep5447.asptr81 = inttoptr i32 %gep5447 to i16*
  %236 = load i16* %gep5447.asptr81, align 1
  %add12.i1739 = add i32 %storemerge216.i1733, 1
  %gep_array5449 = mul i32 %add12.i1739, 2
  %gep5450 = add i32 %fr, %gep_array5449
  %gep5450.asptr = inttoptr i32 %gep5450 to i16*
  store i16 %236, i16* %gep5450.asptr, align 1
  %cmp5.i1741 = icmp slt i32 %add12.i1739, %storemerge118.i1728
  br i1 %cmp5.i1741, label %for.body6.i1742, label %for.inc14.i1745

for.inc14.i1745:                                  ; preds = %for.body6.i1742, %for.cond4.preheader.i1730
  %inc15.i1743 = add i32 %storemerge118.i1728, 1
  %cmp2.i1744 = icmp slt i32 %inc15.i1743, %storemerge21.i1721
  br i1 %cmp2.i1744, label %for.cond4.preheader.i1730, label %for.inc17.i1748

for.inc17.i1748:                                  ; preds = %for.inc14.i1745, %for.cond1.preheader.i1723
  %inc18.i1746 = add i32 %storemerge21.i1721, 1
  %cmp.i1747 = icmp slt i32 %inc18.i1746, %storemerge.lcssa4517
  br i1 %cmp.i1747, label %for.cond1.preheader.i1723, label %for.cond1.preheader.i1693

for.cond1.preheader.i1693:                        ; preds = %for.inc17.i1748, %for.inc17.i1718
  %storemerge21.i1691 = phi i32 [ %inc18.i1716, %for.inc17.i1718 ], [ 0, %for.inc17.i1748 ]
  %cmp217.i1692 = icmp sgt i32 %storemerge21.i1691, 0
  br i1 %cmp217.i1692, label %for.cond4.preheader.lr.ph.i1697, label %for.inc17.i1718

for.cond4.preheader.lr.ph.i1697:                  ; preds = %for.cond1.preheader.i1693
  %sub.i1694 = add i32 %storemerge21.i1691, -2
  %gep_array5452 = mul i32 %sub.i1694, 2
  %gep5453 = add i32 %fr, %gep_array5452
  %gep_array5455 = mul i32 %storemerge21.i1691, 2
  %gep5456 = add i32 %fr, %gep_array5455
  br label %for.cond4.preheader.i1700

for.cond4.preheader.i1700:                        ; preds = %for.inc14.i1715, %for.cond4.preheader.lr.ph.i1697
  %storemerge118.i1698 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1697 ], [ %inc15.i1713, %for.inc14.i1715 ]
  %cmp515.i1699 = icmp sgt i32 %storemerge118.i1698, 0
  br i1 %cmp515.i1699, label %for.body6.lr.ph.i1702, label %for.inc14.i1715

for.body6.lr.ph.i1702:                            ; preds = %for.cond4.preheader.i1700
  %gep_array5458 = mul i32 %storemerge118.i1698, 2
  %gep5459 = add i32 %Sinewave, %gep_array5458
  br label %for.body6.i1712

for.body6.i1712:                                  ; preds = %for.body6.i1712, %for.body6.lr.ph.i1702
  %storemerge216.i1703 = phi i32 [ 0, %for.body6.lr.ph.i1702 ], [ %add12.i1709, %for.body6.i1712 ]
  %gep5459.asptr = inttoptr i32 %gep5459 to i16*
  %237 = load i16* %gep5459.asptr, align 1
  %conv.i1704 = sext i16 %237 to i32
  %mul.i1705 = mul i32 %conv.i1704, %storemerge.lcssa4517
  %gep5453.asptr = inttoptr i32 %gep5453 to i16*
  %238 = load i16* %gep5453.asptr, align 1
  %conv83.i1706 = zext i16 %238 to i32
  %add.i1707 = add i32 %mul.i1705, %conv83.i1706
  %conv9.i1708 = trunc i32 %add.i1707 to i16
  %gep5456.asptr = inttoptr i32 %gep5456 to i16*
  store i16 %conv9.i1708, i16* %gep5456.asptr, align 1
  %gep5459.asptr82 = inttoptr i32 %gep5459 to i16*
  %239 = load i16* %gep5459.asptr82, align 1
  %add12.i1709 = add i32 %storemerge216.i1703, 1
  %gep_array5461 = mul i32 %add12.i1709, 2
  %gep5462 = add i32 %fr, %gep_array5461
  %gep5462.asptr = inttoptr i32 %gep5462 to i16*
  store i16 %239, i16* %gep5462.asptr, align 1
  %cmp5.i1711 = icmp slt i32 %add12.i1709, %storemerge118.i1698
  br i1 %cmp5.i1711, label %for.body6.i1712, label %for.inc14.i1715

for.inc14.i1715:                                  ; preds = %for.body6.i1712, %for.cond4.preheader.i1700
  %inc15.i1713 = add i32 %storemerge118.i1698, 1
  %cmp2.i1714 = icmp slt i32 %inc15.i1713, %storemerge21.i1691
  br i1 %cmp2.i1714, label %for.cond4.preheader.i1700, label %for.inc17.i1718

for.inc17.i1718:                                  ; preds = %for.inc14.i1715, %for.cond1.preheader.i1693
  %inc18.i1716 = add i32 %storemerge21.i1691, 1
  %cmp.i1717 = icmp slt i32 %inc18.i1716, %storemerge.lcssa4517
  br i1 %cmp.i1717, label %for.cond1.preheader.i1693, label %for.cond1.preheader.i1663

for.cond1.preheader.i1663:                        ; preds = %for.inc17.i1718, %for.inc17.i1688
  %storemerge21.i1661 = phi i32 [ %inc18.i1686, %for.inc17.i1688 ], [ 0, %for.inc17.i1718 ]
  %cmp217.i1662 = icmp sgt i32 %storemerge21.i1661, 0
  br i1 %cmp217.i1662, label %for.cond4.preheader.lr.ph.i1667, label %for.inc17.i1688

for.cond4.preheader.lr.ph.i1667:                  ; preds = %for.cond1.preheader.i1663
  %sub.i1664 = add i32 %storemerge21.i1661, -2
  %gep_array5464 = mul i32 %sub.i1664, 2
  %gep5465 = add i32 %fr, %gep_array5464
  %gep_array5467 = mul i32 %storemerge21.i1661, 2
  %gep5468 = add i32 %fr, %gep_array5467
  br label %for.cond4.preheader.i1670

for.cond4.preheader.i1670:                        ; preds = %for.inc14.i1685, %for.cond4.preheader.lr.ph.i1667
  %storemerge118.i1668 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1667 ], [ %inc15.i1683, %for.inc14.i1685 ]
  %cmp515.i1669 = icmp sgt i32 %storemerge118.i1668, 0
  br i1 %cmp515.i1669, label %for.body6.lr.ph.i1672, label %for.inc14.i1685

for.body6.lr.ph.i1672:                            ; preds = %for.cond4.preheader.i1670
  %gep_array5470 = mul i32 %storemerge118.i1668, 2
  %gep5471 = add i32 %fi, %gep_array5470
  br label %for.body6.i1682

for.body6.i1682:                                  ; preds = %for.body6.i1682, %for.body6.lr.ph.i1672
  %storemerge216.i1673 = phi i32 [ 0, %for.body6.lr.ph.i1672 ], [ %add12.i1679, %for.body6.i1682 ]
  %gep5471.asptr = inttoptr i32 %gep5471 to i16*
  %240 = load i16* %gep5471.asptr, align 1
  %conv.i1674 = sext i16 %240 to i32
  %mul.i1675 = mul i32 %conv.i1674, %storemerge.lcssa4517
  %gep5465.asptr = inttoptr i32 %gep5465 to i16*
  %241 = load i16* %gep5465.asptr, align 1
  %conv83.i1676 = zext i16 %241 to i32
  %add.i1677 = add i32 %mul.i1675, %conv83.i1676
  %conv9.i1678 = trunc i32 %add.i1677 to i16
  %gep5468.asptr = inttoptr i32 %gep5468 to i16*
  store i16 %conv9.i1678, i16* %gep5468.asptr, align 1
  %gep5471.asptr83 = inttoptr i32 %gep5471 to i16*
  %242 = load i16* %gep5471.asptr83, align 1
  %add12.i1679 = add i32 %storemerge216.i1673, 1
  %gep_array5473 = mul i32 %add12.i1679, 2
  %gep5474 = add i32 %fr, %gep_array5473
  %gep5474.asptr = inttoptr i32 %gep5474 to i16*
  store i16 %242, i16* %gep5474.asptr, align 1
  %cmp5.i1681 = icmp slt i32 %add12.i1679, %storemerge118.i1668
  br i1 %cmp5.i1681, label %for.body6.i1682, label %for.inc14.i1685

for.inc14.i1685:                                  ; preds = %for.body6.i1682, %for.cond4.preheader.i1670
  %inc15.i1683 = add i32 %storemerge118.i1668, 1
  %cmp2.i1684 = icmp slt i32 %inc15.i1683, %storemerge21.i1661
  br i1 %cmp2.i1684, label %for.cond4.preheader.i1670, label %for.inc17.i1688

for.inc17.i1688:                                  ; preds = %for.inc14.i1685, %for.cond1.preheader.i1663
  %inc18.i1686 = add i32 %storemerge21.i1661, 1
  %cmp.i1687 = icmp slt i32 %inc18.i1686, %storemerge.lcssa4517
  br i1 %cmp.i1687, label %for.cond1.preheader.i1663, label %for.cond1.preheader.i1633

for.cond1.preheader.i1633:                        ; preds = %for.inc17.i1688, %for.inc17.i1658
  %storemerge21.i1631 = phi i32 [ %inc18.i1656, %for.inc17.i1658 ], [ 0, %for.inc17.i1688 ]
  %cmp217.i1632 = icmp sgt i32 %storemerge21.i1631, 0
  br i1 %cmp217.i1632, label %for.cond4.preheader.lr.ph.i1637, label %for.inc17.i1658

for.cond4.preheader.lr.ph.i1637:                  ; preds = %for.cond1.preheader.i1633
  %sub.i1634 = add i32 %storemerge21.i1631, -2
  %gep_array5476 = mul i32 %sub.i1634, 2
  %gep5477 = add i32 %fr, %gep_array5476
  %gep_array5479 = mul i32 %storemerge21.i1631, 2
  %gep5480 = add i32 %fr, %gep_array5479
  br label %for.cond4.preheader.i1640

for.cond4.preheader.i1640:                        ; preds = %for.inc14.i1655, %for.cond4.preheader.lr.ph.i1637
  %storemerge118.i1638 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1637 ], [ %inc15.i1653, %for.inc14.i1655 ]
  %cmp515.i1639 = icmp sgt i32 %storemerge118.i1638, 0
  br i1 %cmp515.i1639, label %for.body6.lr.ph.i1642, label %for.inc14.i1655

for.body6.lr.ph.i1642:                            ; preds = %for.cond4.preheader.i1640
  %gep_array5482 = mul i32 %storemerge118.i1638, 2
  %gep5483 = add i32 %Sinewave, %gep_array5482
  br label %for.body6.i1652

for.body6.i1652:                                  ; preds = %for.body6.i1652, %for.body6.lr.ph.i1642
  %storemerge216.i1643 = phi i32 [ 0, %for.body6.lr.ph.i1642 ], [ %add12.i1649, %for.body6.i1652 ]
  %gep5483.asptr = inttoptr i32 %gep5483 to i16*
  %243 = load i16* %gep5483.asptr, align 1
  %conv.i1644 = sext i16 %243 to i32
  %mul.i1645 = mul i32 %conv.i1644, %storemerge.lcssa4517
  %gep5477.asptr = inttoptr i32 %gep5477 to i16*
  %244 = load i16* %gep5477.asptr, align 1
  %conv83.i1646 = zext i16 %244 to i32
  %add.i1647 = add i32 %mul.i1645, %conv83.i1646
  %conv9.i1648 = trunc i32 %add.i1647 to i16
  %gep5480.asptr = inttoptr i32 %gep5480 to i16*
  store i16 %conv9.i1648, i16* %gep5480.asptr, align 1
  %gep5483.asptr84 = inttoptr i32 %gep5483 to i16*
  %245 = load i16* %gep5483.asptr84, align 1
  %add12.i1649 = add i32 %storemerge216.i1643, 1
  %gep_array5485 = mul i32 %add12.i1649, 2
  %gep5486 = add i32 %fr, %gep_array5485
  %gep5486.asptr = inttoptr i32 %gep5486 to i16*
  store i16 %245, i16* %gep5486.asptr, align 1
  %cmp5.i1651 = icmp slt i32 %add12.i1649, %storemerge118.i1638
  br i1 %cmp5.i1651, label %for.body6.i1652, label %for.inc14.i1655

for.inc14.i1655:                                  ; preds = %for.body6.i1652, %for.cond4.preheader.i1640
  %inc15.i1653 = add i32 %storemerge118.i1638, 1
  %cmp2.i1654 = icmp slt i32 %inc15.i1653, %storemerge21.i1631
  br i1 %cmp2.i1654, label %for.cond4.preheader.i1640, label %for.inc17.i1658

for.inc17.i1658:                                  ; preds = %for.inc14.i1655, %for.cond1.preheader.i1633
  %inc18.i1656 = add i32 %storemerge21.i1631, 1
  %cmp.i1657 = icmp slt i32 %inc18.i1656, %storemerge.lcssa4517
  br i1 %cmp.i1657, label %for.cond1.preheader.i1633, label %for.cond1.preheader.i1603

for.cond1.preheader.i1603:                        ; preds = %for.inc17.i1658, %for.inc17.i1628
  %storemerge21.i1601 = phi i32 [ %inc18.i1626, %for.inc17.i1628 ], [ 0, %for.inc17.i1658 ]
  %cmp217.i1602 = icmp sgt i32 %storemerge21.i1601, 0
  br i1 %cmp217.i1602, label %for.cond4.preheader.lr.ph.i1607, label %for.inc17.i1628

for.cond4.preheader.lr.ph.i1607:                  ; preds = %for.cond1.preheader.i1603
  %sub.i1604 = add i32 %storemerge21.i1601, -2
  %gep_array5488 = mul i32 %sub.i1604, 2
  %gep5489 = add i32 %fr, %gep_array5488
  %gep_array5491 = mul i32 %storemerge21.i1601, 2
  %gep5492 = add i32 %fr, %gep_array5491
  br label %for.cond4.preheader.i1610

for.cond4.preheader.i1610:                        ; preds = %for.inc14.i1625, %for.cond4.preheader.lr.ph.i1607
  %storemerge118.i1608 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1607 ], [ %inc15.i1623, %for.inc14.i1625 ]
  %cmp515.i1609 = icmp sgt i32 %storemerge118.i1608, 0
  br i1 %cmp515.i1609, label %for.body6.lr.ph.i1612, label %for.inc14.i1625

for.body6.lr.ph.i1612:                            ; preds = %for.cond4.preheader.i1610
  %gep_array5494 = mul i32 %storemerge118.i1608, 2
  %gep5495 = add i32 %fi, %gep_array5494
  br label %for.body6.i1622

for.body6.i1622:                                  ; preds = %for.body6.i1622, %for.body6.lr.ph.i1612
  %storemerge216.i1613 = phi i32 [ 0, %for.body6.lr.ph.i1612 ], [ %add12.i1619, %for.body6.i1622 ]
  %gep5495.asptr = inttoptr i32 %gep5495 to i16*
  %246 = load i16* %gep5495.asptr, align 1
  %conv.i1614 = sext i16 %246 to i32
  %mul.i1615 = mul i32 %conv.i1614, %storemerge.lcssa4517
  %gep5489.asptr = inttoptr i32 %gep5489 to i16*
  %247 = load i16* %gep5489.asptr, align 1
  %conv83.i1616 = zext i16 %247 to i32
  %add.i1617 = add i32 %mul.i1615, %conv83.i1616
  %conv9.i1618 = trunc i32 %add.i1617 to i16
  %gep5492.asptr = inttoptr i32 %gep5492 to i16*
  store i16 %conv9.i1618, i16* %gep5492.asptr, align 1
  %gep5495.asptr85 = inttoptr i32 %gep5495 to i16*
  %248 = load i16* %gep5495.asptr85, align 1
  %add12.i1619 = add i32 %storemerge216.i1613, 1
  %gep_array5497 = mul i32 %add12.i1619, 2
  %gep5498 = add i32 %fr, %gep_array5497
  %gep5498.asptr = inttoptr i32 %gep5498 to i16*
  store i16 %248, i16* %gep5498.asptr, align 1
  %cmp5.i1621 = icmp slt i32 %add12.i1619, %storemerge118.i1608
  br i1 %cmp5.i1621, label %for.body6.i1622, label %for.inc14.i1625

for.inc14.i1625:                                  ; preds = %for.body6.i1622, %for.cond4.preheader.i1610
  %inc15.i1623 = add i32 %storemerge118.i1608, 1
  %cmp2.i1624 = icmp slt i32 %inc15.i1623, %storemerge21.i1601
  br i1 %cmp2.i1624, label %for.cond4.preheader.i1610, label %for.inc17.i1628

for.inc17.i1628:                                  ; preds = %for.inc14.i1625, %for.cond1.preheader.i1603
  %inc18.i1626 = add i32 %storemerge21.i1601, 1
  %cmp.i1627 = icmp slt i32 %inc18.i1626, %storemerge.lcssa4517
  br i1 %cmp.i1627, label %for.cond1.preheader.i1603, label %for.cond1.preheader.i1573

for.cond1.preheader.i1573:                        ; preds = %for.inc17.i1628, %for.inc17.i1598
  %storemerge21.i1571 = phi i32 [ %inc18.i1596, %for.inc17.i1598 ], [ 0, %for.inc17.i1628 ]
  %cmp217.i1572 = icmp sgt i32 %storemerge21.i1571, 0
  br i1 %cmp217.i1572, label %for.cond4.preheader.lr.ph.i1577, label %for.inc17.i1598

for.cond4.preheader.lr.ph.i1577:                  ; preds = %for.cond1.preheader.i1573
  %sub.i1574 = add i32 %storemerge21.i1571, -2
  %gep_array5500 = mul i32 %sub.i1574, 2
  %gep5501 = add i32 %fr, %gep_array5500
  %gep_array5503 = mul i32 %storemerge21.i1571, 2
  %gep5504 = add i32 %fr, %gep_array5503
  br label %for.cond4.preheader.i1580

for.cond4.preheader.i1580:                        ; preds = %for.inc14.i1595, %for.cond4.preheader.lr.ph.i1577
  %storemerge118.i1578 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1577 ], [ %inc15.i1593, %for.inc14.i1595 ]
  %cmp515.i1579 = icmp sgt i32 %storemerge118.i1578, 0
  br i1 %cmp515.i1579, label %for.body6.lr.ph.i1582, label %for.inc14.i1595

for.body6.lr.ph.i1582:                            ; preds = %for.cond4.preheader.i1580
  %gep_array5506 = mul i32 %storemerge118.i1578, 2
  %gep5507 = add i32 %Sinewave, %gep_array5506
  br label %for.body6.i1592

for.body6.i1592:                                  ; preds = %for.body6.i1592, %for.body6.lr.ph.i1582
  %storemerge216.i1583 = phi i32 [ 0, %for.body6.lr.ph.i1582 ], [ %add12.i1589, %for.body6.i1592 ]
  %gep5507.asptr = inttoptr i32 %gep5507 to i16*
  %249 = load i16* %gep5507.asptr, align 1
  %conv.i1584 = sext i16 %249 to i32
  %mul.i1585 = mul i32 %conv.i1584, %storemerge.lcssa4517
  %gep5501.asptr = inttoptr i32 %gep5501 to i16*
  %250 = load i16* %gep5501.asptr, align 1
  %conv83.i1586 = zext i16 %250 to i32
  %add.i1587 = add i32 %mul.i1585, %conv83.i1586
  %conv9.i1588 = trunc i32 %add.i1587 to i16
  %gep5504.asptr = inttoptr i32 %gep5504 to i16*
  store i16 %conv9.i1588, i16* %gep5504.asptr, align 1
  %gep5507.asptr86 = inttoptr i32 %gep5507 to i16*
  %251 = load i16* %gep5507.asptr86, align 1
  %add12.i1589 = add i32 %storemerge216.i1583, 1
  %gep_array5509 = mul i32 %add12.i1589, 2
  %gep5510 = add i32 %fr, %gep_array5509
  %gep5510.asptr = inttoptr i32 %gep5510 to i16*
  store i16 %251, i16* %gep5510.asptr, align 1
  %cmp5.i1591 = icmp slt i32 %add12.i1589, %storemerge118.i1578
  br i1 %cmp5.i1591, label %for.body6.i1592, label %for.inc14.i1595

for.inc14.i1595:                                  ; preds = %for.body6.i1592, %for.cond4.preheader.i1580
  %inc15.i1593 = add i32 %storemerge118.i1578, 1
  %cmp2.i1594 = icmp slt i32 %inc15.i1593, %storemerge21.i1571
  br i1 %cmp2.i1594, label %for.cond4.preheader.i1580, label %for.inc17.i1598

for.inc17.i1598:                                  ; preds = %for.inc14.i1595, %for.cond1.preheader.i1573
  %inc18.i1596 = add i32 %storemerge21.i1571, 1
  %cmp.i1597 = icmp slt i32 %inc18.i1596, %storemerge.lcssa4517
  br i1 %cmp.i1597, label %for.cond1.preheader.i1573, label %for.cond1.preheader.i1543

for.cond1.preheader.i1543:                        ; preds = %for.inc17.i1598, %for.inc17.i1568
  %storemerge21.i1541 = phi i32 [ %inc18.i1566, %for.inc17.i1568 ], [ 0, %for.inc17.i1598 ]
  %cmp217.i1542 = icmp sgt i32 %storemerge21.i1541, 0
  br i1 %cmp217.i1542, label %for.cond4.preheader.lr.ph.i1547, label %for.inc17.i1568

for.cond4.preheader.lr.ph.i1547:                  ; preds = %for.cond1.preheader.i1543
  %sub.i1544 = add i32 %storemerge21.i1541, -2
  %gep_array5512 = mul i32 %sub.i1544, 2
  %gep5513 = add i32 %fr, %gep_array5512
  %gep_array5515 = mul i32 %storemerge21.i1541, 2
  %gep5516 = add i32 %fr, %gep_array5515
  br label %for.cond4.preheader.i1550

for.cond4.preheader.i1550:                        ; preds = %for.inc14.i1565, %for.cond4.preheader.lr.ph.i1547
  %storemerge118.i1548 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1547 ], [ %inc15.i1563, %for.inc14.i1565 ]
  %cmp515.i1549 = icmp sgt i32 %storemerge118.i1548, 0
  br i1 %cmp515.i1549, label %for.body6.lr.ph.i1552, label %for.inc14.i1565

for.body6.lr.ph.i1552:                            ; preds = %for.cond4.preheader.i1550
  %gep_array5518 = mul i32 %storemerge118.i1548, 2
  %gep5519 = add i32 %fi, %gep_array5518
  br label %for.body6.i1562

for.body6.i1562:                                  ; preds = %for.body6.i1562, %for.body6.lr.ph.i1552
  %storemerge216.i1553 = phi i32 [ 0, %for.body6.lr.ph.i1552 ], [ %add12.i1559, %for.body6.i1562 ]
  %gep5519.asptr = inttoptr i32 %gep5519 to i16*
  %252 = load i16* %gep5519.asptr, align 1
  %conv.i1554 = sext i16 %252 to i32
  %mul.i1555 = mul i32 %conv.i1554, %storemerge.lcssa4517
  %gep5513.asptr = inttoptr i32 %gep5513 to i16*
  %253 = load i16* %gep5513.asptr, align 1
  %conv83.i1556 = zext i16 %253 to i32
  %add.i1557 = add i32 %mul.i1555, %conv83.i1556
  %conv9.i1558 = trunc i32 %add.i1557 to i16
  %gep5516.asptr = inttoptr i32 %gep5516 to i16*
  store i16 %conv9.i1558, i16* %gep5516.asptr, align 1
  %gep5519.asptr87 = inttoptr i32 %gep5519 to i16*
  %254 = load i16* %gep5519.asptr87, align 1
  %add12.i1559 = add i32 %storemerge216.i1553, 1
  %gep_array5521 = mul i32 %add12.i1559, 2
  %gep5522 = add i32 %fr, %gep_array5521
  %gep5522.asptr = inttoptr i32 %gep5522 to i16*
  store i16 %254, i16* %gep5522.asptr, align 1
  %cmp5.i1561 = icmp slt i32 %add12.i1559, %storemerge118.i1548
  br i1 %cmp5.i1561, label %for.body6.i1562, label %for.inc14.i1565

for.inc14.i1565:                                  ; preds = %for.body6.i1562, %for.cond4.preheader.i1550
  %inc15.i1563 = add i32 %storemerge118.i1548, 1
  %cmp2.i1564 = icmp slt i32 %inc15.i1563, %storemerge21.i1541
  br i1 %cmp2.i1564, label %for.cond4.preheader.i1550, label %for.inc17.i1568

for.inc17.i1568:                                  ; preds = %for.inc14.i1565, %for.cond1.preheader.i1543
  %inc18.i1566 = add i32 %storemerge21.i1541, 1
  %cmp.i1567 = icmp slt i32 %inc18.i1566, %storemerge.lcssa4517
  br i1 %cmp.i1567, label %for.cond1.preheader.i1543, label %for.cond1.preheader.i1513

for.cond1.preheader.i1513:                        ; preds = %for.inc17.i1568, %for.inc17.i1538
  %storemerge21.i1511 = phi i32 [ %inc18.i1536, %for.inc17.i1538 ], [ 0, %for.inc17.i1568 ]
  %cmp217.i1512 = icmp sgt i32 %storemerge21.i1511, 0
  br i1 %cmp217.i1512, label %for.cond4.preheader.lr.ph.i1517, label %for.inc17.i1538

for.cond4.preheader.lr.ph.i1517:                  ; preds = %for.cond1.preheader.i1513
  %sub.i1514 = add i32 %storemerge21.i1511, -2
  %gep_array5524 = mul i32 %sub.i1514, 2
  %gep5525 = add i32 %fr, %gep_array5524
  %gep_array5527 = mul i32 %storemerge21.i1511, 2
  %gep5528 = add i32 %fr, %gep_array5527
  br label %for.cond4.preheader.i1520

for.cond4.preheader.i1520:                        ; preds = %for.inc14.i1535, %for.cond4.preheader.lr.ph.i1517
  %storemerge118.i1518 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1517 ], [ %inc15.i1533, %for.inc14.i1535 ]
  %cmp515.i1519 = icmp sgt i32 %storemerge118.i1518, 0
  br i1 %cmp515.i1519, label %for.body6.lr.ph.i1522, label %for.inc14.i1535

for.body6.lr.ph.i1522:                            ; preds = %for.cond4.preheader.i1520
  %gep_array5530 = mul i32 %storemerge118.i1518, 2
  %gep5531 = add i32 %Sinewave, %gep_array5530
  br label %for.body6.i1532

for.body6.i1532:                                  ; preds = %for.body6.i1532, %for.body6.lr.ph.i1522
  %storemerge216.i1523 = phi i32 [ 0, %for.body6.lr.ph.i1522 ], [ %add12.i1529, %for.body6.i1532 ]
  %gep5531.asptr = inttoptr i32 %gep5531 to i16*
  %255 = load i16* %gep5531.asptr, align 1
  %conv.i1524 = sext i16 %255 to i32
  %mul.i1525 = mul i32 %conv.i1524, %storemerge.lcssa4517
  %gep5525.asptr = inttoptr i32 %gep5525 to i16*
  %256 = load i16* %gep5525.asptr, align 1
  %conv83.i1526 = zext i16 %256 to i32
  %add.i1527 = add i32 %mul.i1525, %conv83.i1526
  %conv9.i1528 = trunc i32 %add.i1527 to i16
  %gep5528.asptr = inttoptr i32 %gep5528 to i16*
  store i16 %conv9.i1528, i16* %gep5528.asptr, align 1
  %gep5531.asptr88 = inttoptr i32 %gep5531 to i16*
  %257 = load i16* %gep5531.asptr88, align 1
  %add12.i1529 = add i32 %storemerge216.i1523, 1
  %gep_array5533 = mul i32 %add12.i1529, 2
  %gep5534 = add i32 %fr, %gep_array5533
  %gep5534.asptr = inttoptr i32 %gep5534 to i16*
  store i16 %257, i16* %gep5534.asptr, align 1
  %cmp5.i1531 = icmp slt i32 %add12.i1529, %storemerge118.i1518
  br i1 %cmp5.i1531, label %for.body6.i1532, label %for.inc14.i1535

for.inc14.i1535:                                  ; preds = %for.body6.i1532, %for.cond4.preheader.i1520
  %inc15.i1533 = add i32 %storemerge118.i1518, 1
  %cmp2.i1534 = icmp slt i32 %inc15.i1533, %storemerge21.i1511
  br i1 %cmp2.i1534, label %for.cond4.preheader.i1520, label %for.inc17.i1538

for.inc17.i1538:                                  ; preds = %for.inc14.i1535, %for.cond1.preheader.i1513
  %inc18.i1536 = add i32 %storemerge21.i1511, 1
  %cmp.i1537 = icmp slt i32 %inc18.i1536, %storemerge.lcssa4517
  br i1 %cmp.i1537, label %for.cond1.preheader.i1513, label %for.cond1.preheader.i1483

for.cond1.preheader.i1483:                        ; preds = %for.inc17.i1538, %for.inc17.i1508
  %storemerge21.i1481 = phi i32 [ %inc18.i1506, %for.inc17.i1508 ], [ 0, %for.inc17.i1538 ]
  %cmp217.i1482 = icmp sgt i32 %storemerge21.i1481, 0
  br i1 %cmp217.i1482, label %for.cond4.preheader.lr.ph.i1487, label %for.inc17.i1508

for.cond4.preheader.lr.ph.i1487:                  ; preds = %for.cond1.preheader.i1483
  %sub.i1484 = add i32 %storemerge21.i1481, -2
  %gep_array5536 = mul i32 %sub.i1484, 2
  %gep5537 = add i32 %fr, %gep_array5536
  %gep_array5539 = mul i32 %storemerge21.i1481, 2
  %gep5540 = add i32 %fr, %gep_array5539
  br label %for.cond4.preheader.i1490

for.cond4.preheader.i1490:                        ; preds = %for.inc14.i1505, %for.cond4.preheader.lr.ph.i1487
  %storemerge118.i1488 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1487 ], [ %inc15.i1503, %for.inc14.i1505 ]
  %cmp515.i1489 = icmp sgt i32 %storemerge118.i1488, 0
  br i1 %cmp515.i1489, label %for.body6.lr.ph.i1492, label %for.inc14.i1505

for.body6.lr.ph.i1492:                            ; preds = %for.cond4.preheader.i1490
  %gep_array5542 = mul i32 %storemerge118.i1488, 2
  %gep5543 = add i32 %fi, %gep_array5542
  br label %for.body6.i1502

for.body6.i1502:                                  ; preds = %for.body6.i1502, %for.body6.lr.ph.i1492
  %storemerge216.i1493 = phi i32 [ 0, %for.body6.lr.ph.i1492 ], [ %add12.i1499, %for.body6.i1502 ]
  %gep5543.asptr = inttoptr i32 %gep5543 to i16*
  %258 = load i16* %gep5543.asptr, align 1
  %conv.i1494 = sext i16 %258 to i32
  %mul.i1495 = mul i32 %conv.i1494, %storemerge.lcssa4517
  %gep5537.asptr = inttoptr i32 %gep5537 to i16*
  %259 = load i16* %gep5537.asptr, align 1
  %conv83.i1496 = zext i16 %259 to i32
  %add.i1497 = add i32 %mul.i1495, %conv83.i1496
  %conv9.i1498 = trunc i32 %add.i1497 to i16
  %gep5540.asptr = inttoptr i32 %gep5540 to i16*
  store i16 %conv9.i1498, i16* %gep5540.asptr, align 1
  %gep5543.asptr89 = inttoptr i32 %gep5543 to i16*
  %260 = load i16* %gep5543.asptr89, align 1
  %add12.i1499 = add i32 %storemerge216.i1493, 1
  %gep_array5545 = mul i32 %add12.i1499, 2
  %gep5546 = add i32 %fr, %gep_array5545
  %gep5546.asptr = inttoptr i32 %gep5546 to i16*
  store i16 %260, i16* %gep5546.asptr, align 1
  %cmp5.i1501 = icmp slt i32 %add12.i1499, %storemerge118.i1488
  br i1 %cmp5.i1501, label %for.body6.i1502, label %for.inc14.i1505

for.inc14.i1505:                                  ; preds = %for.body6.i1502, %for.cond4.preheader.i1490
  %inc15.i1503 = add i32 %storemerge118.i1488, 1
  %cmp2.i1504 = icmp slt i32 %inc15.i1503, %storemerge21.i1481
  br i1 %cmp2.i1504, label %for.cond4.preheader.i1490, label %for.inc17.i1508

for.inc17.i1508:                                  ; preds = %for.inc14.i1505, %for.cond1.preheader.i1483
  %inc18.i1506 = add i32 %storemerge21.i1481, 1
  %cmp.i1507 = icmp slt i32 %inc18.i1506, %storemerge.lcssa4517
  br i1 %cmp.i1507, label %for.cond1.preheader.i1483, label %for.cond1.preheader.i1453

for.cond1.preheader.i1453:                        ; preds = %for.inc17.i1508, %for.inc17.i1478
  %storemerge21.i1451 = phi i32 [ %inc18.i1476, %for.inc17.i1478 ], [ 0, %for.inc17.i1508 ]
  %cmp217.i1452 = icmp sgt i32 %storemerge21.i1451, 0
  br i1 %cmp217.i1452, label %for.cond4.preheader.lr.ph.i1457, label %for.inc17.i1478

for.cond4.preheader.lr.ph.i1457:                  ; preds = %for.cond1.preheader.i1453
  %sub.i1454 = add i32 %storemerge21.i1451, -2
  %gep_array5548 = mul i32 %sub.i1454, 2
  %gep5549 = add i32 %fr, %gep_array5548
  %gep_array5551 = mul i32 %storemerge21.i1451, 2
  %gep5552 = add i32 %fr, %gep_array5551
  br label %for.cond4.preheader.i1460

for.cond4.preheader.i1460:                        ; preds = %for.inc14.i1475, %for.cond4.preheader.lr.ph.i1457
  %storemerge118.i1458 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1457 ], [ %inc15.i1473, %for.inc14.i1475 ]
  %cmp515.i1459 = icmp sgt i32 %storemerge118.i1458, 0
  br i1 %cmp515.i1459, label %for.body6.lr.ph.i1462, label %for.inc14.i1475

for.body6.lr.ph.i1462:                            ; preds = %for.cond4.preheader.i1460
  %gep_array5554 = mul i32 %storemerge118.i1458, 2
  %gep5555 = add i32 %Sinewave, %gep_array5554
  br label %for.body6.i1472

for.body6.i1472:                                  ; preds = %for.body6.i1472, %for.body6.lr.ph.i1462
  %storemerge216.i1463 = phi i32 [ 0, %for.body6.lr.ph.i1462 ], [ %add12.i1469, %for.body6.i1472 ]
  %gep5555.asptr = inttoptr i32 %gep5555 to i16*
  %261 = load i16* %gep5555.asptr, align 1
  %conv.i1464 = sext i16 %261 to i32
  %mul.i1465 = mul i32 %conv.i1464, %storemerge.lcssa4517
  %gep5549.asptr = inttoptr i32 %gep5549 to i16*
  %262 = load i16* %gep5549.asptr, align 1
  %conv83.i1466 = zext i16 %262 to i32
  %add.i1467 = add i32 %mul.i1465, %conv83.i1466
  %conv9.i1468 = trunc i32 %add.i1467 to i16
  %gep5552.asptr = inttoptr i32 %gep5552 to i16*
  store i16 %conv9.i1468, i16* %gep5552.asptr, align 1
  %gep5555.asptr90 = inttoptr i32 %gep5555 to i16*
  %263 = load i16* %gep5555.asptr90, align 1
  %add12.i1469 = add i32 %storemerge216.i1463, 1
  %gep_array5557 = mul i32 %add12.i1469, 2
  %gep5558 = add i32 %fr, %gep_array5557
  %gep5558.asptr = inttoptr i32 %gep5558 to i16*
  store i16 %263, i16* %gep5558.asptr, align 1
  %cmp5.i1471 = icmp slt i32 %add12.i1469, %storemerge118.i1458
  br i1 %cmp5.i1471, label %for.body6.i1472, label %for.inc14.i1475

for.inc14.i1475:                                  ; preds = %for.body6.i1472, %for.cond4.preheader.i1460
  %inc15.i1473 = add i32 %storemerge118.i1458, 1
  %cmp2.i1474 = icmp slt i32 %inc15.i1473, %storemerge21.i1451
  br i1 %cmp2.i1474, label %for.cond4.preheader.i1460, label %for.inc17.i1478

for.inc17.i1478:                                  ; preds = %for.inc14.i1475, %for.cond1.preheader.i1453
  %inc18.i1476 = add i32 %storemerge21.i1451, 1
  %cmp.i1477 = icmp slt i32 %inc18.i1476, %storemerge.lcssa4517
  br i1 %cmp.i1477, label %for.cond1.preheader.i1453, label %for.cond1.preheader.i1423

for.cond1.preheader.i1423:                        ; preds = %for.inc17.i1478, %for.inc17.i1448
  %storemerge21.i1421 = phi i32 [ %inc18.i1446, %for.inc17.i1448 ], [ 0, %for.inc17.i1478 ]
  %cmp217.i1422 = icmp sgt i32 %storemerge21.i1421, 0
  br i1 %cmp217.i1422, label %for.cond4.preheader.lr.ph.i1427, label %for.inc17.i1448

for.cond4.preheader.lr.ph.i1427:                  ; preds = %for.cond1.preheader.i1423
  %sub.i1424 = add i32 %storemerge21.i1421, -2
  %gep_array5560 = mul i32 %sub.i1424, 2
  %gep5561 = add i32 %fr, %gep_array5560
  %gep_array5563 = mul i32 %storemerge21.i1421, 2
  %gep5564 = add i32 %fr, %gep_array5563
  br label %for.cond4.preheader.i1430

for.cond4.preheader.i1430:                        ; preds = %for.inc14.i1445, %for.cond4.preheader.lr.ph.i1427
  %storemerge118.i1428 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1427 ], [ %inc15.i1443, %for.inc14.i1445 ]
  %cmp515.i1429 = icmp sgt i32 %storemerge118.i1428, 0
  br i1 %cmp515.i1429, label %for.body6.lr.ph.i1432, label %for.inc14.i1445

for.body6.lr.ph.i1432:                            ; preds = %for.cond4.preheader.i1430
  %gep_array5566 = mul i32 %storemerge118.i1428, 2
  %gep5567 = add i32 %Sinewave, %gep_array5566
  br label %for.body6.i1442

for.body6.i1442:                                  ; preds = %for.body6.i1442, %for.body6.lr.ph.i1432
  %storemerge216.i1433 = phi i32 [ 0, %for.body6.lr.ph.i1432 ], [ %add12.i1439, %for.body6.i1442 ]
  %gep5567.asptr = inttoptr i32 %gep5567 to i16*
  %264 = load i16* %gep5567.asptr, align 1
  %conv.i1434 = sext i16 %264 to i32
  %mul.i1435 = mul i32 %conv.i1434, %storemerge.lcssa4517
  %gep5561.asptr = inttoptr i32 %gep5561 to i16*
  %265 = load i16* %gep5561.asptr, align 1
  %conv83.i1436 = zext i16 %265 to i32
  %add.i1437 = add i32 %mul.i1435, %conv83.i1436
  %conv9.i1438 = trunc i32 %add.i1437 to i16
  %gep5564.asptr = inttoptr i32 %gep5564 to i16*
  store i16 %conv9.i1438, i16* %gep5564.asptr, align 1
  %gep5567.asptr91 = inttoptr i32 %gep5567 to i16*
  %266 = load i16* %gep5567.asptr91, align 1
  %add12.i1439 = add i32 %storemerge216.i1433, 1
  %gep_array5569 = mul i32 %add12.i1439, 2
  %gep5570 = add i32 %fr, %gep_array5569
  %gep5570.asptr = inttoptr i32 %gep5570 to i16*
  store i16 %266, i16* %gep5570.asptr, align 1
  %cmp5.i1441 = icmp slt i32 %add12.i1439, %storemerge118.i1428
  br i1 %cmp5.i1441, label %for.body6.i1442, label %for.inc14.i1445

for.inc14.i1445:                                  ; preds = %for.body6.i1442, %for.cond4.preheader.i1430
  %inc15.i1443 = add i32 %storemerge118.i1428, 1
  %cmp2.i1444 = icmp slt i32 %inc15.i1443, %storemerge21.i1421
  br i1 %cmp2.i1444, label %for.cond4.preheader.i1430, label %for.inc17.i1448

for.inc17.i1448:                                  ; preds = %for.inc14.i1445, %for.cond1.preheader.i1423
  %inc18.i1446 = add i32 %storemerge21.i1421, 1
  %cmp.i1447 = icmp slt i32 %inc18.i1446, %storemerge.lcssa4517
  br i1 %cmp.i1447, label %for.cond1.preheader.i1423, label %for.cond1.preheader.i1393

for.cond1.preheader.i1393:                        ; preds = %for.inc17.i1448, %for.inc17.i1418
  %storemerge21.i1391 = phi i32 [ %inc18.i1416, %for.inc17.i1418 ], [ 0, %for.inc17.i1448 ]
  %cmp217.i1392 = icmp sgt i32 %storemerge21.i1391, 0
  br i1 %cmp217.i1392, label %for.cond4.preheader.lr.ph.i1397, label %for.inc17.i1418

for.cond4.preheader.lr.ph.i1397:                  ; preds = %for.cond1.preheader.i1393
  %sub.i1394 = add i32 %storemerge21.i1391, -2
  %gep_array5572 = mul i32 %sub.i1394, 2
  %gep5573 = add i32 %fr, %gep_array5572
  %gep_array5575 = mul i32 %storemerge21.i1391, 2
  %gep5576 = add i32 %fr, %gep_array5575
  br label %for.cond4.preheader.i1400

for.cond4.preheader.i1400:                        ; preds = %for.inc14.i1415, %for.cond4.preheader.lr.ph.i1397
  %storemerge118.i1398 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1397 ], [ %inc15.i1413, %for.inc14.i1415 ]
  %cmp515.i1399 = icmp sgt i32 %storemerge118.i1398, 0
  br i1 %cmp515.i1399, label %for.body6.lr.ph.i1402, label %for.inc14.i1415

for.body6.lr.ph.i1402:                            ; preds = %for.cond4.preheader.i1400
  %gep_array5578 = mul i32 %storemerge118.i1398, 2
  %gep5579 = add i32 %fi, %gep_array5578
  br label %for.body6.i1412

for.body6.i1412:                                  ; preds = %for.body6.i1412, %for.body6.lr.ph.i1402
  %storemerge216.i1403 = phi i32 [ 0, %for.body6.lr.ph.i1402 ], [ %add12.i1409, %for.body6.i1412 ]
  %gep5579.asptr = inttoptr i32 %gep5579 to i16*
  %267 = load i16* %gep5579.asptr, align 1
  %conv.i1404 = sext i16 %267 to i32
  %mul.i1405 = mul i32 %conv.i1404, %storemerge.lcssa4517
  %gep5573.asptr = inttoptr i32 %gep5573 to i16*
  %268 = load i16* %gep5573.asptr, align 1
  %conv83.i1406 = zext i16 %268 to i32
  %add.i1407 = add i32 %mul.i1405, %conv83.i1406
  %conv9.i1408 = trunc i32 %add.i1407 to i16
  %gep5576.asptr = inttoptr i32 %gep5576 to i16*
  store i16 %conv9.i1408, i16* %gep5576.asptr, align 1
  %gep5579.asptr92 = inttoptr i32 %gep5579 to i16*
  %269 = load i16* %gep5579.asptr92, align 1
  %add12.i1409 = add i32 %storemerge216.i1403, 1
  %gep_array5581 = mul i32 %add12.i1409, 2
  %gep5582 = add i32 %fr, %gep_array5581
  %gep5582.asptr = inttoptr i32 %gep5582 to i16*
  store i16 %269, i16* %gep5582.asptr, align 1
  %cmp5.i1411 = icmp slt i32 %add12.i1409, %storemerge118.i1398
  br i1 %cmp5.i1411, label %for.body6.i1412, label %for.inc14.i1415

for.inc14.i1415:                                  ; preds = %for.body6.i1412, %for.cond4.preheader.i1400
  %inc15.i1413 = add i32 %storemerge118.i1398, 1
  %cmp2.i1414 = icmp slt i32 %inc15.i1413, %storemerge21.i1391
  br i1 %cmp2.i1414, label %for.cond4.preheader.i1400, label %for.inc17.i1418

for.inc17.i1418:                                  ; preds = %for.inc14.i1415, %for.cond1.preheader.i1393
  %inc18.i1416 = add i32 %storemerge21.i1391, 1
  %cmp.i1417 = icmp slt i32 %inc18.i1416, %storemerge.lcssa4517
  br i1 %cmp.i1417, label %for.cond1.preheader.i1393, label %for.cond1.preheader.i1363

for.cond1.preheader.i1363:                        ; preds = %for.inc17.i1418, %for.inc17.i1388
  %storemerge21.i1361 = phi i32 [ %inc18.i1386, %for.inc17.i1388 ], [ 0, %for.inc17.i1418 ]
  %cmp217.i1362 = icmp sgt i32 %storemerge21.i1361, 0
  br i1 %cmp217.i1362, label %for.cond4.preheader.lr.ph.i1367, label %for.inc17.i1388

for.cond4.preheader.lr.ph.i1367:                  ; preds = %for.cond1.preheader.i1363
  %sub.i1364 = add i32 %storemerge21.i1361, -2
  %gep_array5584 = mul i32 %sub.i1364, 2
  %gep5585 = add i32 %fr, %gep_array5584
  %gep_array5587 = mul i32 %storemerge21.i1361, 2
  %gep5588 = add i32 %fr, %gep_array5587
  br label %for.cond4.preheader.i1370

for.cond4.preheader.i1370:                        ; preds = %for.inc14.i1385, %for.cond4.preheader.lr.ph.i1367
  %storemerge118.i1368 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1367 ], [ %inc15.i1383, %for.inc14.i1385 ]
  %cmp515.i1369 = icmp sgt i32 %storemerge118.i1368, 0
  br i1 %cmp515.i1369, label %for.body6.lr.ph.i1372, label %for.inc14.i1385

for.body6.lr.ph.i1372:                            ; preds = %for.cond4.preheader.i1370
  %gep_array5590 = mul i32 %storemerge118.i1368, 2
  %gep5591 = add i32 %Sinewave, %gep_array5590
  br label %for.body6.i1382

for.body6.i1382:                                  ; preds = %for.body6.i1382, %for.body6.lr.ph.i1372
  %storemerge216.i1373 = phi i32 [ 0, %for.body6.lr.ph.i1372 ], [ %add12.i1379, %for.body6.i1382 ]
  %gep5591.asptr = inttoptr i32 %gep5591 to i16*
  %270 = load i16* %gep5591.asptr, align 1
  %conv.i1374 = sext i16 %270 to i32
  %mul.i1375 = mul i32 %conv.i1374, %storemerge.lcssa4517
  %gep5585.asptr = inttoptr i32 %gep5585 to i16*
  %271 = load i16* %gep5585.asptr, align 1
  %conv83.i1376 = zext i16 %271 to i32
  %add.i1377 = add i32 %mul.i1375, %conv83.i1376
  %conv9.i1378 = trunc i32 %add.i1377 to i16
  %gep5588.asptr = inttoptr i32 %gep5588 to i16*
  store i16 %conv9.i1378, i16* %gep5588.asptr, align 1
  %gep5591.asptr93 = inttoptr i32 %gep5591 to i16*
  %272 = load i16* %gep5591.asptr93, align 1
  %add12.i1379 = add i32 %storemerge216.i1373, 1
  %gep_array5593 = mul i32 %add12.i1379, 2
  %gep5594 = add i32 %fr, %gep_array5593
  %gep5594.asptr = inttoptr i32 %gep5594 to i16*
  store i16 %272, i16* %gep5594.asptr, align 1
  %cmp5.i1381 = icmp slt i32 %add12.i1379, %storemerge118.i1368
  br i1 %cmp5.i1381, label %for.body6.i1382, label %for.inc14.i1385

for.inc14.i1385:                                  ; preds = %for.body6.i1382, %for.cond4.preheader.i1370
  %inc15.i1383 = add i32 %storemerge118.i1368, 1
  %cmp2.i1384 = icmp slt i32 %inc15.i1383, %storemerge21.i1361
  br i1 %cmp2.i1384, label %for.cond4.preheader.i1370, label %for.inc17.i1388

for.inc17.i1388:                                  ; preds = %for.inc14.i1385, %for.cond1.preheader.i1363
  %inc18.i1386 = add i32 %storemerge21.i1361, 1
  %cmp.i1387 = icmp slt i32 %inc18.i1386, %storemerge.lcssa4517
  br i1 %cmp.i1387, label %for.cond1.preheader.i1363, label %for.cond1.preheader.i1333

for.cond1.preheader.i1333:                        ; preds = %for.inc17.i1388, %for.inc17.i1358
  %storemerge21.i1331 = phi i32 [ %inc18.i1356, %for.inc17.i1358 ], [ 0, %for.inc17.i1388 ]
  %cmp217.i1332 = icmp sgt i32 %storemerge21.i1331, 0
  br i1 %cmp217.i1332, label %for.cond4.preheader.lr.ph.i1337, label %for.inc17.i1358

for.cond4.preheader.lr.ph.i1337:                  ; preds = %for.cond1.preheader.i1333
  %sub.i1334 = add i32 %storemerge21.i1331, -2
  %gep_array5596 = mul i32 %sub.i1334, 2
  %gep5597 = add i32 %fr, %gep_array5596
  %gep_array5599 = mul i32 %storemerge21.i1331, 2
  %gep5600 = add i32 %fr, %gep_array5599
  br label %for.cond4.preheader.i1340

for.cond4.preheader.i1340:                        ; preds = %for.inc14.i1355, %for.cond4.preheader.lr.ph.i1337
  %storemerge118.i1338 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1337 ], [ %inc15.i1353, %for.inc14.i1355 ]
  %cmp515.i1339 = icmp sgt i32 %storemerge118.i1338, 0
  br i1 %cmp515.i1339, label %for.body6.lr.ph.i1342, label %for.inc14.i1355

for.body6.lr.ph.i1342:                            ; preds = %for.cond4.preheader.i1340
  %gep_array5602 = mul i32 %storemerge118.i1338, 2
  %gep5603 = add i32 %fi, %gep_array5602
  br label %for.body6.i1352

for.body6.i1352:                                  ; preds = %for.body6.i1352, %for.body6.lr.ph.i1342
  %storemerge216.i1343 = phi i32 [ 0, %for.body6.lr.ph.i1342 ], [ %add12.i1349, %for.body6.i1352 ]
  %gep5603.asptr = inttoptr i32 %gep5603 to i16*
  %273 = load i16* %gep5603.asptr, align 1
  %conv.i1344 = sext i16 %273 to i32
  %mul.i1345 = mul i32 %conv.i1344, %storemerge.lcssa4517
  %gep5597.asptr = inttoptr i32 %gep5597 to i16*
  %274 = load i16* %gep5597.asptr, align 1
  %conv83.i1346 = zext i16 %274 to i32
  %add.i1347 = add i32 %mul.i1345, %conv83.i1346
  %conv9.i1348 = trunc i32 %add.i1347 to i16
  %gep5600.asptr = inttoptr i32 %gep5600 to i16*
  store i16 %conv9.i1348, i16* %gep5600.asptr, align 1
  %gep5603.asptr94 = inttoptr i32 %gep5603 to i16*
  %275 = load i16* %gep5603.asptr94, align 1
  %add12.i1349 = add i32 %storemerge216.i1343, 1
  %gep_array5605 = mul i32 %add12.i1349, 2
  %gep5606 = add i32 %fr, %gep_array5605
  %gep5606.asptr = inttoptr i32 %gep5606 to i16*
  store i16 %275, i16* %gep5606.asptr, align 1
  %cmp5.i1351 = icmp slt i32 %add12.i1349, %storemerge118.i1338
  br i1 %cmp5.i1351, label %for.body6.i1352, label %for.inc14.i1355

for.inc14.i1355:                                  ; preds = %for.body6.i1352, %for.cond4.preheader.i1340
  %inc15.i1353 = add i32 %storemerge118.i1338, 1
  %cmp2.i1354 = icmp slt i32 %inc15.i1353, %storemerge21.i1331
  br i1 %cmp2.i1354, label %for.cond4.preheader.i1340, label %for.inc17.i1358

for.inc17.i1358:                                  ; preds = %for.inc14.i1355, %for.cond1.preheader.i1333
  %inc18.i1356 = add i32 %storemerge21.i1331, 1
  %cmp.i1357 = icmp slt i32 %inc18.i1356, %storemerge.lcssa4517
  br i1 %cmp.i1357, label %for.cond1.preheader.i1333, label %for.cond1.preheader.i1303

for.cond1.preheader.i1303:                        ; preds = %for.inc17.i1358, %for.inc17.i1328
  %storemerge21.i1301 = phi i32 [ %inc18.i1326, %for.inc17.i1328 ], [ 0, %for.inc17.i1358 ]
  %cmp217.i1302 = icmp sgt i32 %storemerge21.i1301, 0
  br i1 %cmp217.i1302, label %for.cond4.preheader.lr.ph.i1307, label %for.inc17.i1328

for.cond4.preheader.lr.ph.i1307:                  ; preds = %for.cond1.preheader.i1303
  %sub.i1304 = add i32 %storemerge21.i1301, -2
  %gep_array5608 = mul i32 %sub.i1304, 2
  %gep5609 = add i32 %fr, %gep_array5608
  %gep_array5611 = mul i32 %storemerge21.i1301, 2
  %gep5612 = add i32 %fr, %gep_array5611
  br label %for.cond4.preheader.i1310

for.cond4.preheader.i1310:                        ; preds = %for.inc14.i1325, %for.cond4.preheader.lr.ph.i1307
  %storemerge118.i1308 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1307 ], [ %inc15.i1323, %for.inc14.i1325 ]
  %cmp515.i1309 = icmp sgt i32 %storemerge118.i1308, 0
  br i1 %cmp515.i1309, label %for.body6.lr.ph.i1312, label %for.inc14.i1325

for.body6.lr.ph.i1312:                            ; preds = %for.cond4.preheader.i1310
  %gep_array5614 = mul i32 %storemerge118.i1308, 2
  %gep5615 = add i32 %Sinewave, %gep_array5614
  br label %for.body6.i1322

for.body6.i1322:                                  ; preds = %for.body6.i1322, %for.body6.lr.ph.i1312
  %storemerge216.i1313 = phi i32 [ 0, %for.body6.lr.ph.i1312 ], [ %add12.i1319, %for.body6.i1322 ]
  %gep5615.asptr = inttoptr i32 %gep5615 to i16*
  %276 = load i16* %gep5615.asptr, align 1
  %conv.i1314 = sext i16 %276 to i32
  %mul.i1315 = mul i32 %conv.i1314, %storemerge.lcssa4517
  %gep5609.asptr = inttoptr i32 %gep5609 to i16*
  %277 = load i16* %gep5609.asptr, align 1
  %conv83.i1316 = zext i16 %277 to i32
  %add.i1317 = add i32 %mul.i1315, %conv83.i1316
  %conv9.i1318 = trunc i32 %add.i1317 to i16
  %gep5612.asptr = inttoptr i32 %gep5612 to i16*
  store i16 %conv9.i1318, i16* %gep5612.asptr, align 1
  %gep5615.asptr95 = inttoptr i32 %gep5615 to i16*
  %278 = load i16* %gep5615.asptr95, align 1
  %add12.i1319 = add i32 %storemerge216.i1313, 1
  %gep_array5617 = mul i32 %add12.i1319, 2
  %gep5618 = add i32 %fr, %gep_array5617
  %gep5618.asptr = inttoptr i32 %gep5618 to i16*
  store i16 %278, i16* %gep5618.asptr, align 1
  %cmp5.i1321 = icmp slt i32 %add12.i1319, %storemerge118.i1308
  br i1 %cmp5.i1321, label %for.body6.i1322, label %for.inc14.i1325

for.inc14.i1325:                                  ; preds = %for.body6.i1322, %for.cond4.preheader.i1310
  %inc15.i1323 = add i32 %storemerge118.i1308, 1
  %cmp2.i1324 = icmp slt i32 %inc15.i1323, %storemerge21.i1301
  br i1 %cmp2.i1324, label %for.cond4.preheader.i1310, label %for.inc17.i1328

for.inc17.i1328:                                  ; preds = %for.inc14.i1325, %for.cond1.preheader.i1303
  %inc18.i1326 = add i32 %storemerge21.i1301, 1
  %cmp.i1327 = icmp slt i32 %inc18.i1326, %storemerge.lcssa4517
  br i1 %cmp.i1327, label %for.cond1.preheader.i1303, label %for.cond1.preheader.i1273

for.cond1.preheader.i1273:                        ; preds = %for.inc17.i1328, %for.inc17.i1298
  %storemerge21.i1271 = phi i32 [ %inc18.i1296, %for.inc17.i1298 ], [ 0, %for.inc17.i1328 ]
  %cmp217.i1272 = icmp sgt i32 %storemerge21.i1271, 0
  br i1 %cmp217.i1272, label %for.cond4.preheader.lr.ph.i1277, label %for.inc17.i1298

for.cond4.preheader.lr.ph.i1277:                  ; preds = %for.cond1.preheader.i1273
  %sub.i1274 = add i32 %storemerge21.i1271, -2
  %gep_array5620 = mul i32 %sub.i1274, 2
  %gep5621 = add i32 %fr, %gep_array5620
  %gep_array5623 = mul i32 %storemerge21.i1271, 2
  %gep5624 = add i32 %fr, %gep_array5623
  br label %for.cond4.preheader.i1280

for.cond4.preheader.i1280:                        ; preds = %for.inc14.i1295, %for.cond4.preheader.lr.ph.i1277
  %storemerge118.i1278 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1277 ], [ %inc15.i1293, %for.inc14.i1295 ]
  %cmp515.i1279 = icmp sgt i32 %storemerge118.i1278, 0
  br i1 %cmp515.i1279, label %for.body6.lr.ph.i1282, label %for.inc14.i1295

for.body6.lr.ph.i1282:                            ; preds = %for.cond4.preheader.i1280
  %gep_array5626 = mul i32 %storemerge118.i1278, 2
  %gep5627 = add i32 %fi, %gep_array5626
  br label %for.body6.i1292

for.body6.i1292:                                  ; preds = %for.body6.i1292, %for.body6.lr.ph.i1282
  %storemerge216.i1283 = phi i32 [ 0, %for.body6.lr.ph.i1282 ], [ %add12.i1289, %for.body6.i1292 ]
  %gep5627.asptr = inttoptr i32 %gep5627 to i16*
  %279 = load i16* %gep5627.asptr, align 1
  %conv.i1284 = sext i16 %279 to i32
  %mul.i1285 = mul i32 %conv.i1284, %storemerge.lcssa4517
  %gep5621.asptr = inttoptr i32 %gep5621 to i16*
  %280 = load i16* %gep5621.asptr, align 1
  %conv83.i1286 = zext i16 %280 to i32
  %add.i1287 = add i32 %mul.i1285, %conv83.i1286
  %conv9.i1288 = trunc i32 %add.i1287 to i16
  %gep5624.asptr = inttoptr i32 %gep5624 to i16*
  store i16 %conv9.i1288, i16* %gep5624.asptr, align 1
  %gep5627.asptr96 = inttoptr i32 %gep5627 to i16*
  %281 = load i16* %gep5627.asptr96, align 1
  %add12.i1289 = add i32 %storemerge216.i1283, 1
  %gep_array5629 = mul i32 %add12.i1289, 2
  %gep5630 = add i32 %fr, %gep_array5629
  %gep5630.asptr = inttoptr i32 %gep5630 to i16*
  store i16 %281, i16* %gep5630.asptr, align 1
  %cmp5.i1291 = icmp slt i32 %add12.i1289, %storemerge118.i1278
  br i1 %cmp5.i1291, label %for.body6.i1292, label %for.inc14.i1295

for.inc14.i1295:                                  ; preds = %for.body6.i1292, %for.cond4.preheader.i1280
  %inc15.i1293 = add i32 %storemerge118.i1278, 1
  %cmp2.i1294 = icmp slt i32 %inc15.i1293, %storemerge21.i1271
  br i1 %cmp2.i1294, label %for.cond4.preheader.i1280, label %for.inc17.i1298

for.inc17.i1298:                                  ; preds = %for.inc14.i1295, %for.cond1.preheader.i1273
  %inc18.i1296 = add i32 %storemerge21.i1271, 1
  %cmp.i1297 = icmp slt i32 %inc18.i1296, %storemerge.lcssa4517
  br i1 %cmp.i1297, label %for.cond1.preheader.i1273, label %for.cond1.preheader.i1243

for.cond1.preheader.i1243:                        ; preds = %for.inc17.i1298, %for.inc17.i1268
  %storemerge21.i1241 = phi i32 [ %inc18.i1266, %for.inc17.i1268 ], [ 0, %for.inc17.i1298 ]
  %cmp217.i1242 = icmp sgt i32 %storemerge21.i1241, 0
  br i1 %cmp217.i1242, label %for.cond4.preheader.lr.ph.i1247, label %for.inc17.i1268

for.cond4.preheader.lr.ph.i1247:                  ; preds = %for.cond1.preheader.i1243
  %sub.i1244 = add i32 %storemerge21.i1241, -2
  %gep_array5632 = mul i32 %sub.i1244, 2
  %gep5633 = add i32 %fr, %gep_array5632
  %gep_array5635 = mul i32 %storemerge21.i1241, 2
  %gep5636 = add i32 %fr, %gep_array5635
  br label %for.cond4.preheader.i1250

for.cond4.preheader.i1250:                        ; preds = %for.inc14.i1265, %for.cond4.preheader.lr.ph.i1247
  %storemerge118.i1248 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1247 ], [ %inc15.i1263, %for.inc14.i1265 ]
  %cmp515.i1249 = icmp sgt i32 %storemerge118.i1248, 0
  br i1 %cmp515.i1249, label %for.body6.lr.ph.i1252, label %for.inc14.i1265

for.body6.lr.ph.i1252:                            ; preds = %for.cond4.preheader.i1250
  %gep_array5638 = mul i32 %storemerge118.i1248, 2
  %gep5639 = add i32 %Sinewave, %gep_array5638
  br label %for.body6.i1262

for.body6.i1262:                                  ; preds = %for.body6.i1262, %for.body6.lr.ph.i1252
  %storemerge216.i1253 = phi i32 [ 0, %for.body6.lr.ph.i1252 ], [ %add12.i1259, %for.body6.i1262 ]
  %gep5639.asptr = inttoptr i32 %gep5639 to i16*
  %282 = load i16* %gep5639.asptr, align 1
  %conv.i1254 = sext i16 %282 to i32
  %mul.i1255 = mul i32 %conv.i1254, %storemerge.lcssa4517
  %gep5633.asptr = inttoptr i32 %gep5633 to i16*
  %283 = load i16* %gep5633.asptr, align 1
  %conv83.i1256 = zext i16 %283 to i32
  %add.i1257 = add i32 %mul.i1255, %conv83.i1256
  %conv9.i1258 = trunc i32 %add.i1257 to i16
  %gep5636.asptr = inttoptr i32 %gep5636 to i16*
  store i16 %conv9.i1258, i16* %gep5636.asptr, align 1
  %gep5639.asptr97 = inttoptr i32 %gep5639 to i16*
  %284 = load i16* %gep5639.asptr97, align 1
  %add12.i1259 = add i32 %storemerge216.i1253, 1
  %gep_array5641 = mul i32 %add12.i1259, 2
  %gep5642 = add i32 %fr, %gep_array5641
  %gep5642.asptr = inttoptr i32 %gep5642 to i16*
  store i16 %284, i16* %gep5642.asptr, align 1
  %cmp5.i1261 = icmp slt i32 %add12.i1259, %storemerge118.i1248
  br i1 %cmp5.i1261, label %for.body6.i1262, label %for.inc14.i1265

for.inc14.i1265:                                  ; preds = %for.body6.i1262, %for.cond4.preheader.i1250
  %inc15.i1263 = add i32 %storemerge118.i1248, 1
  %cmp2.i1264 = icmp slt i32 %inc15.i1263, %storemerge21.i1241
  br i1 %cmp2.i1264, label %for.cond4.preheader.i1250, label %for.inc17.i1268

for.inc17.i1268:                                  ; preds = %for.inc14.i1265, %for.cond1.preheader.i1243
  %inc18.i1266 = add i32 %storemerge21.i1241, 1
  %cmp.i1267 = icmp slt i32 %inc18.i1266, %storemerge.lcssa4517
  br i1 %cmp.i1267, label %for.cond1.preheader.i1243, label %for.cond1.preheader.i1213

for.cond1.preheader.i1213:                        ; preds = %for.inc17.i1268, %for.inc17.i1238
  %storemerge21.i1211 = phi i32 [ %inc18.i1236, %for.inc17.i1238 ], [ 0, %for.inc17.i1268 ]
  %cmp217.i1212 = icmp sgt i32 %storemerge21.i1211, 0
  br i1 %cmp217.i1212, label %for.cond4.preheader.lr.ph.i1217, label %for.inc17.i1238

for.cond4.preheader.lr.ph.i1217:                  ; preds = %for.cond1.preheader.i1213
  %sub.i1214 = add i32 %storemerge21.i1211, -2
  %gep_array5644 = mul i32 %sub.i1214, 2
  %gep5645 = add i32 %fr, %gep_array5644
  %gep_array5647 = mul i32 %storemerge21.i1211, 2
  %gep5648 = add i32 %fr, %gep_array5647
  br label %for.cond4.preheader.i1220

for.cond4.preheader.i1220:                        ; preds = %for.inc14.i1235, %for.cond4.preheader.lr.ph.i1217
  %storemerge118.i1218 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1217 ], [ %inc15.i1233, %for.inc14.i1235 ]
  %cmp515.i1219 = icmp sgt i32 %storemerge118.i1218, 0
  br i1 %cmp515.i1219, label %for.body6.lr.ph.i1222, label %for.inc14.i1235

for.body6.lr.ph.i1222:                            ; preds = %for.cond4.preheader.i1220
  %gep_array5650 = mul i32 %storemerge118.i1218, 2
  %gep5651 = add i32 %fi, %gep_array5650
  br label %for.body6.i1232

for.body6.i1232:                                  ; preds = %for.body6.i1232, %for.body6.lr.ph.i1222
  %storemerge216.i1223 = phi i32 [ 0, %for.body6.lr.ph.i1222 ], [ %add12.i1229, %for.body6.i1232 ]
  %gep5651.asptr = inttoptr i32 %gep5651 to i16*
  %285 = load i16* %gep5651.asptr, align 1
  %conv.i1224 = sext i16 %285 to i32
  %mul.i1225 = mul i32 %conv.i1224, %storemerge.lcssa4517
  %gep5645.asptr = inttoptr i32 %gep5645 to i16*
  %286 = load i16* %gep5645.asptr, align 1
  %conv83.i1226 = zext i16 %286 to i32
  %add.i1227 = add i32 %mul.i1225, %conv83.i1226
  %conv9.i1228 = trunc i32 %add.i1227 to i16
  %gep5648.asptr = inttoptr i32 %gep5648 to i16*
  store i16 %conv9.i1228, i16* %gep5648.asptr, align 1
  %gep5651.asptr98 = inttoptr i32 %gep5651 to i16*
  %287 = load i16* %gep5651.asptr98, align 1
  %add12.i1229 = add i32 %storemerge216.i1223, 1
  %gep_array5653 = mul i32 %add12.i1229, 2
  %gep5654 = add i32 %fr, %gep_array5653
  %gep5654.asptr = inttoptr i32 %gep5654 to i16*
  store i16 %287, i16* %gep5654.asptr, align 1
  %cmp5.i1231 = icmp slt i32 %add12.i1229, %storemerge118.i1218
  br i1 %cmp5.i1231, label %for.body6.i1232, label %for.inc14.i1235

for.inc14.i1235:                                  ; preds = %for.body6.i1232, %for.cond4.preheader.i1220
  %inc15.i1233 = add i32 %storemerge118.i1218, 1
  %cmp2.i1234 = icmp slt i32 %inc15.i1233, %storemerge21.i1211
  br i1 %cmp2.i1234, label %for.cond4.preheader.i1220, label %for.inc17.i1238

for.inc17.i1238:                                  ; preds = %for.inc14.i1235, %for.cond1.preheader.i1213
  %inc18.i1236 = add i32 %storemerge21.i1211, 1
  %cmp.i1237 = icmp slt i32 %inc18.i1236, %storemerge.lcssa4517
  br i1 %cmp.i1237, label %for.cond1.preheader.i1213, label %for.cond1.preheader.i1183

for.cond1.preheader.i1183:                        ; preds = %for.inc17.i1238, %for.inc17.i1208
  %storemerge21.i1181 = phi i32 [ %inc18.i1206, %for.inc17.i1208 ], [ 0, %for.inc17.i1238 ]
  %cmp217.i1182 = icmp sgt i32 %storemerge21.i1181, 0
  br i1 %cmp217.i1182, label %for.cond4.preheader.lr.ph.i1187, label %for.inc17.i1208

for.cond4.preheader.lr.ph.i1187:                  ; preds = %for.cond1.preheader.i1183
  %sub.i1184 = add i32 %storemerge21.i1181, -2
  %gep_array5656 = mul i32 %sub.i1184, 2
  %gep5657 = add i32 %fr, %gep_array5656
  %gep_array5659 = mul i32 %storemerge21.i1181, 2
  %gep5660 = add i32 %fr, %gep_array5659
  br label %for.cond4.preheader.i1190

for.cond4.preheader.i1190:                        ; preds = %for.inc14.i1205, %for.cond4.preheader.lr.ph.i1187
  %storemerge118.i1188 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1187 ], [ %inc15.i1203, %for.inc14.i1205 ]
  %cmp515.i1189 = icmp sgt i32 %storemerge118.i1188, 0
  br i1 %cmp515.i1189, label %for.body6.lr.ph.i1192, label %for.inc14.i1205

for.body6.lr.ph.i1192:                            ; preds = %for.cond4.preheader.i1190
  %gep_array5662 = mul i32 %storemerge118.i1188, 2
  %gep5663 = add i32 %Sinewave, %gep_array5662
  br label %for.body6.i1202

for.body6.i1202:                                  ; preds = %for.body6.i1202, %for.body6.lr.ph.i1192
  %storemerge216.i1193 = phi i32 [ 0, %for.body6.lr.ph.i1192 ], [ %add12.i1199, %for.body6.i1202 ]
  %gep5663.asptr = inttoptr i32 %gep5663 to i16*
  %288 = load i16* %gep5663.asptr, align 1
  %conv.i1194 = sext i16 %288 to i32
  %mul.i1195 = mul i32 %conv.i1194, %storemerge.lcssa4517
  %gep5657.asptr = inttoptr i32 %gep5657 to i16*
  %289 = load i16* %gep5657.asptr, align 1
  %conv83.i1196 = zext i16 %289 to i32
  %add.i1197 = add i32 %mul.i1195, %conv83.i1196
  %conv9.i1198 = trunc i32 %add.i1197 to i16
  %gep5660.asptr = inttoptr i32 %gep5660 to i16*
  store i16 %conv9.i1198, i16* %gep5660.asptr, align 1
  %gep5663.asptr99 = inttoptr i32 %gep5663 to i16*
  %290 = load i16* %gep5663.asptr99, align 1
  %add12.i1199 = add i32 %storemerge216.i1193, 1
  %gep_array5665 = mul i32 %add12.i1199, 2
  %gep5666 = add i32 %fr, %gep_array5665
  %gep5666.asptr = inttoptr i32 %gep5666 to i16*
  store i16 %290, i16* %gep5666.asptr, align 1
  %cmp5.i1201 = icmp slt i32 %add12.i1199, %storemerge118.i1188
  br i1 %cmp5.i1201, label %for.body6.i1202, label %for.inc14.i1205

for.inc14.i1205:                                  ; preds = %for.body6.i1202, %for.cond4.preheader.i1190
  %inc15.i1203 = add i32 %storemerge118.i1188, 1
  %cmp2.i1204 = icmp slt i32 %inc15.i1203, %storemerge21.i1181
  br i1 %cmp2.i1204, label %for.cond4.preheader.i1190, label %for.inc17.i1208

for.inc17.i1208:                                  ; preds = %for.inc14.i1205, %for.cond1.preheader.i1183
  %inc18.i1206 = add i32 %storemerge21.i1181, 1
  %cmp.i1207 = icmp slt i32 %inc18.i1206, %storemerge.lcssa4517
  br i1 %cmp.i1207, label %for.cond1.preheader.i1183, label %for.cond1.preheader.i1153

for.cond1.preheader.i1153:                        ; preds = %for.inc17.i1208, %for.inc17.i1178
  %storemerge21.i1151 = phi i32 [ %inc18.i1176, %for.inc17.i1178 ], [ 0, %for.inc17.i1208 ]
  %cmp217.i1152 = icmp sgt i32 %storemerge21.i1151, 0
  br i1 %cmp217.i1152, label %for.cond4.preheader.lr.ph.i1157, label %for.inc17.i1178

for.cond4.preheader.lr.ph.i1157:                  ; preds = %for.cond1.preheader.i1153
  %sub.i1154 = add i32 %storemerge21.i1151, -2
  %gep_array5668 = mul i32 %sub.i1154, 2
  %gep5669 = add i32 %fr, %gep_array5668
  %gep_array5671 = mul i32 %storemerge21.i1151, 2
  %gep5672 = add i32 %fr, %gep_array5671
  br label %for.cond4.preheader.i1160

for.cond4.preheader.i1160:                        ; preds = %for.inc14.i1175, %for.cond4.preheader.lr.ph.i1157
  %storemerge118.i1158 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1157 ], [ %inc15.i1173, %for.inc14.i1175 ]
  %cmp515.i1159 = icmp sgt i32 %storemerge118.i1158, 0
  br i1 %cmp515.i1159, label %for.body6.lr.ph.i1162, label %for.inc14.i1175

for.body6.lr.ph.i1162:                            ; preds = %for.cond4.preheader.i1160
  %gep_array5674 = mul i32 %storemerge118.i1158, 2
  %gep5675 = add i32 %fi, %gep_array5674
  br label %for.body6.i1172

for.body6.i1172:                                  ; preds = %for.body6.i1172, %for.body6.lr.ph.i1162
  %storemerge216.i1163 = phi i32 [ 0, %for.body6.lr.ph.i1162 ], [ %add12.i1169, %for.body6.i1172 ]
  %gep5675.asptr = inttoptr i32 %gep5675 to i16*
  %291 = load i16* %gep5675.asptr, align 1
  %conv.i1164 = sext i16 %291 to i32
  %mul.i1165 = mul i32 %conv.i1164, %storemerge.lcssa4517
  %gep5669.asptr = inttoptr i32 %gep5669 to i16*
  %292 = load i16* %gep5669.asptr, align 1
  %conv83.i1166 = zext i16 %292 to i32
  %add.i1167 = add i32 %mul.i1165, %conv83.i1166
  %conv9.i1168 = trunc i32 %add.i1167 to i16
  %gep5672.asptr = inttoptr i32 %gep5672 to i16*
  store i16 %conv9.i1168, i16* %gep5672.asptr, align 1
  %gep5675.asptr100 = inttoptr i32 %gep5675 to i16*
  %293 = load i16* %gep5675.asptr100, align 1
  %add12.i1169 = add i32 %storemerge216.i1163, 1
  %gep_array5677 = mul i32 %add12.i1169, 2
  %gep5678 = add i32 %fr, %gep_array5677
  %gep5678.asptr = inttoptr i32 %gep5678 to i16*
  store i16 %293, i16* %gep5678.asptr, align 1
  %cmp5.i1171 = icmp slt i32 %add12.i1169, %storemerge118.i1158
  br i1 %cmp5.i1171, label %for.body6.i1172, label %for.inc14.i1175

for.inc14.i1175:                                  ; preds = %for.body6.i1172, %for.cond4.preheader.i1160
  %inc15.i1173 = add i32 %storemerge118.i1158, 1
  %cmp2.i1174 = icmp slt i32 %inc15.i1173, %storemerge21.i1151
  br i1 %cmp2.i1174, label %for.cond4.preheader.i1160, label %for.inc17.i1178

for.inc17.i1178:                                  ; preds = %for.inc14.i1175, %for.cond1.preheader.i1153
  %inc18.i1176 = add i32 %storemerge21.i1151, 1
  %cmp.i1177 = icmp slt i32 %inc18.i1176, %storemerge.lcssa4517
  br i1 %cmp.i1177, label %for.cond1.preheader.i1153, label %for.cond1.preheader.i1123

for.cond1.preheader.i1123:                        ; preds = %for.inc17.i1178, %for.inc17.i1148
  %storemerge21.i1121 = phi i32 [ %inc18.i1146, %for.inc17.i1148 ], [ 0, %for.inc17.i1178 ]
  %cmp217.i1122 = icmp sgt i32 %storemerge21.i1121, 0
  br i1 %cmp217.i1122, label %for.cond4.preheader.lr.ph.i1127, label %for.inc17.i1148

for.cond4.preheader.lr.ph.i1127:                  ; preds = %for.cond1.preheader.i1123
  %sub.i1124 = add i32 %storemerge21.i1121, -2
  %gep_array5680 = mul i32 %sub.i1124, 2
  %gep5681 = add i32 %fr, %gep_array5680
  %gep_array5683 = mul i32 %storemerge21.i1121, 2
  %gep5684 = add i32 %fr, %gep_array5683
  br label %for.cond4.preheader.i1130

for.cond4.preheader.i1130:                        ; preds = %for.inc14.i1145, %for.cond4.preheader.lr.ph.i1127
  %storemerge118.i1128 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1127 ], [ %inc15.i1143, %for.inc14.i1145 ]
  %cmp515.i1129 = icmp sgt i32 %storemerge118.i1128, 0
  br i1 %cmp515.i1129, label %for.body6.lr.ph.i1132, label %for.inc14.i1145

for.body6.lr.ph.i1132:                            ; preds = %for.cond4.preheader.i1130
  %gep_array5686 = mul i32 %storemerge118.i1128, 2
  %gep5687 = add i32 %Sinewave, %gep_array5686
  br label %for.body6.i1142

for.body6.i1142:                                  ; preds = %for.body6.i1142, %for.body6.lr.ph.i1132
  %storemerge216.i1133 = phi i32 [ 0, %for.body6.lr.ph.i1132 ], [ %add12.i1139, %for.body6.i1142 ]
  %gep5687.asptr = inttoptr i32 %gep5687 to i16*
  %294 = load i16* %gep5687.asptr, align 1
  %conv.i1134 = sext i16 %294 to i32
  %mul.i1135 = mul i32 %conv.i1134, %storemerge.lcssa4517
  %gep5681.asptr = inttoptr i32 %gep5681 to i16*
  %295 = load i16* %gep5681.asptr, align 1
  %conv83.i1136 = zext i16 %295 to i32
  %add.i1137 = add i32 %mul.i1135, %conv83.i1136
  %conv9.i1138 = trunc i32 %add.i1137 to i16
  %gep5684.asptr = inttoptr i32 %gep5684 to i16*
  store i16 %conv9.i1138, i16* %gep5684.asptr, align 1
  %gep5687.asptr101 = inttoptr i32 %gep5687 to i16*
  %296 = load i16* %gep5687.asptr101, align 1
  %add12.i1139 = add i32 %storemerge216.i1133, 1
  %gep_array5689 = mul i32 %add12.i1139, 2
  %gep5690 = add i32 %fr, %gep_array5689
  %gep5690.asptr = inttoptr i32 %gep5690 to i16*
  store i16 %296, i16* %gep5690.asptr, align 1
  %cmp5.i1141 = icmp slt i32 %add12.i1139, %storemerge118.i1128
  br i1 %cmp5.i1141, label %for.body6.i1142, label %for.inc14.i1145

for.inc14.i1145:                                  ; preds = %for.body6.i1142, %for.cond4.preheader.i1130
  %inc15.i1143 = add i32 %storemerge118.i1128, 1
  %cmp2.i1144 = icmp slt i32 %inc15.i1143, %storemerge21.i1121
  br i1 %cmp2.i1144, label %for.cond4.preheader.i1130, label %for.inc17.i1148

for.inc17.i1148:                                  ; preds = %for.inc14.i1145, %for.cond1.preheader.i1123
  %inc18.i1146 = add i32 %storemerge21.i1121, 1
  %cmp.i1147 = icmp slt i32 %inc18.i1146, %storemerge.lcssa4517
  br i1 %cmp.i1147, label %for.cond1.preheader.i1123, label %while.cond.preheader

while.cond.preheader:                             ; preds = %for.inc17.i1148, %for.end
  %storemerge.lcssa4516 = phi i32 [ %shl, %for.end ], [ %storemerge.lcssa4517, %for.inc17.i1148 ]
  %cmp174504 = icmp sgt i32 %shl, 1
  br i1 %cmp174504, label %while.body.lr.ph, label %return

while.body.lr.ph:                                 ; preds = %while.cond.preheader
  %tobool = icmp eq i32 %inverse, 0
  br label %while.body

while.body:                                       ; preds = %while.body.lr.ph, %for.end136
  %scale.0.load401744834509 = phi i32 [ 0, %while.body.lr.ph ], [ %scale.0.load40174482, %for.end136 ]
  %dec44844507 = phi i32 [ 9, %while.body.lr.ph ], [ %dec, %for.end136 ]
  %shl5244854506 = phi i32 [ 1, %while.body.lr.ph ], [ %shl52, %for.end136 ]
  %storemerge144914505 = phi i32 [ %storemerge.lcssa4516, %while.body.lr.ph ], [ %storemerge1.lcssa, %for.end136 ]
  br i1 %tobool, label %if.end51, label %for.body23

for.cond20:                                       ; preds = %foo3.exit1089
  %cmp21 = icmp slt i32 %inc45, %shl
  br i1 %cmp21, label %for.body23, label %if.end51

for.body23:                                       ; preds = %while.body, %for.cond20
  %storemerge214498 = phi i32 [ %inc45, %for.cond20 ], [ 0, %while.body ]
  %storemerge144904497 = phi i32 [ %storemerge14489, %for.cond20 ], [ %storemerge144914505, %while.body ]
  %cmp20.i1090 = icmp sgt i32 %storemerge144904497, 0
  br i1 %cmp20.i1090, label %for.cond1.preheader.i1093, label %foo3.exit1089

for.cond1.preheader.i1093:                        ; preds = %for.body23, %for.inc17.i1118
  %storemerge21.i1091 = phi i32 [ %inc18.i1116, %for.inc17.i1118 ], [ 0, %for.body23 ]
  %cmp217.i1092 = icmp sgt i32 %storemerge21.i1091, 0
  br i1 %cmp217.i1092, label %for.cond4.preheader.lr.ph.i1097, label %for.inc17.i1118

for.cond4.preheader.lr.ph.i1097:                  ; preds = %for.cond1.preheader.i1093
  %sub.i1094 = add i32 %storemerge21.i1091, -2
  %gep_array5692 = mul i32 %sub.i1094, 2
  %gep5693 = add i32 %fr, %gep_array5692
  %gep_array5695 = mul i32 %storemerge21.i1091, 2
  %gep5696 = add i32 %fr, %gep_array5695
  br label %for.cond4.preheader.i1100

for.cond4.preheader.i1100:                        ; preds = %for.inc14.i1115, %for.cond4.preheader.lr.ph.i1097
  %storemerge118.i1098 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1097 ], [ %inc15.i1113, %for.inc14.i1115 ]
  %cmp515.i1099 = icmp sgt i32 %storemerge118.i1098, 0
  br i1 %cmp515.i1099, label %for.body6.lr.ph.i1102, label %for.inc14.i1115

for.body6.lr.ph.i1102:                            ; preds = %for.cond4.preheader.i1100
  %gep_array5698 = mul i32 %storemerge118.i1098, 2
  %gep5699 = add i32 %fi, %gep_array5698
  br label %for.body6.i1112

for.body6.i1112:                                  ; preds = %for.body6.i1112, %for.body6.lr.ph.i1102
  %storemerge216.i1103 = phi i32 [ 0, %for.body6.lr.ph.i1102 ], [ %add12.i1109, %for.body6.i1112 ]
  %gep5699.asptr = inttoptr i32 %gep5699 to i16*
  %297 = load i16* %gep5699.asptr, align 1
  %conv.i1104 = sext i16 %297 to i32
  %mul.i1105 = mul i32 %conv.i1104, %storemerge144904497
  %gep5693.asptr = inttoptr i32 %gep5693 to i16*
  %298 = load i16* %gep5693.asptr, align 1
  %conv83.i1106 = zext i16 %298 to i32
  %add.i1107 = add i32 %mul.i1105, %conv83.i1106
  %conv9.i1108 = trunc i32 %add.i1107 to i16
  %gep5696.asptr = inttoptr i32 %gep5696 to i16*
  store i16 %conv9.i1108, i16* %gep5696.asptr, align 1
  %gep5699.asptr102 = inttoptr i32 %gep5699 to i16*
  %299 = load i16* %gep5699.asptr102, align 1
  %add12.i1109 = add i32 %storemerge216.i1103, 1
  %gep_array5701 = mul i32 %add12.i1109, 2
  %gep5702 = add i32 %fr, %gep_array5701
  %gep5702.asptr = inttoptr i32 %gep5702 to i16*
  store i16 %299, i16* %gep5702.asptr, align 1
  %cmp5.i1111 = icmp slt i32 %add12.i1109, %storemerge118.i1098
  br i1 %cmp5.i1111, label %for.body6.i1112, label %for.inc14.i1115

for.inc14.i1115:                                  ; preds = %for.body6.i1112, %for.cond4.preheader.i1100
  %inc15.i1113 = add i32 %storemerge118.i1098, 1
  %cmp2.i1114 = icmp slt i32 %inc15.i1113, %storemerge21.i1091
  br i1 %cmp2.i1114, label %for.cond4.preheader.i1100, label %for.inc17.i1118

for.inc17.i1118:                                  ; preds = %for.inc14.i1115, %for.cond1.preheader.i1093
  %inc18.i1116 = add i32 %storemerge21.i1091, 1
  %cmp.i1117 = icmp slt i32 %inc18.i1116, %storemerge144904497
  br i1 %cmp.i1117, label %for.cond1.preheader.i1093, label %for.cond1.preheader.i1063

for.cond1.preheader.i1063:                        ; preds = %for.inc17.i1118, %for.inc17.i1088
  %storemerge21.i1061 = phi i32 [ %inc18.i1086, %for.inc17.i1088 ], [ 0, %for.inc17.i1118 ]
  %cmp217.i1062 = icmp sgt i32 %storemerge21.i1061, 0
  br i1 %cmp217.i1062, label %for.cond4.preheader.lr.ph.i1067, label %for.inc17.i1088

for.cond4.preheader.lr.ph.i1067:                  ; preds = %for.cond1.preheader.i1063
  %sub.i1064 = add i32 %storemerge21.i1061, -2
  %gep_array5704 = mul i32 %sub.i1064, 2
  %gep5705 = add i32 %fr, %gep_array5704
  %gep_array5707 = mul i32 %storemerge21.i1061, 2
  %gep5708 = add i32 %fr, %gep_array5707
  br label %for.cond4.preheader.i1070

for.cond4.preheader.i1070:                        ; preds = %for.inc14.i1085, %for.cond4.preheader.lr.ph.i1067
  %storemerge118.i1068 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1067 ], [ %inc15.i1083, %for.inc14.i1085 ]
  %cmp515.i1069 = icmp sgt i32 %storemerge118.i1068, 0
  br i1 %cmp515.i1069, label %for.body6.lr.ph.i1072, label %for.inc14.i1085

for.body6.lr.ph.i1072:                            ; preds = %for.cond4.preheader.i1070
  %gep_array5710 = mul i32 %storemerge118.i1068, 2
  %gep5711 = add i32 %Sinewave, %gep_array5710
  br label %for.body6.i1082

for.body6.i1082:                                  ; preds = %for.body6.i1082, %for.body6.lr.ph.i1072
  %storemerge216.i1073 = phi i32 [ 0, %for.body6.lr.ph.i1072 ], [ %add12.i1079, %for.body6.i1082 ]
  %gep5711.asptr = inttoptr i32 %gep5711 to i16*
  %300 = load i16* %gep5711.asptr, align 1
  %conv.i1074 = sext i16 %300 to i32
  %mul.i1075 = mul i32 %conv.i1074, %storemerge144904497
  %gep5705.asptr = inttoptr i32 %gep5705 to i16*
  %301 = load i16* %gep5705.asptr, align 1
  %conv83.i1076 = zext i16 %301 to i32
  %add.i1077 = add i32 %mul.i1075, %conv83.i1076
  %conv9.i1078 = trunc i32 %add.i1077 to i16
  %gep5708.asptr = inttoptr i32 %gep5708 to i16*
  store i16 %conv9.i1078, i16* %gep5708.asptr, align 1
  %gep5711.asptr103 = inttoptr i32 %gep5711 to i16*
  %302 = load i16* %gep5711.asptr103, align 1
  %add12.i1079 = add i32 %storemerge216.i1073, 1
  %gep_array5713 = mul i32 %add12.i1079, 2
  %gep5714 = add i32 %fr, %gep_array5713
  %gep5714.asptr = inttoptr i32 %gep5714 to i16*
  store i16 %302, i16* %gep5714.asptr, align 1
  %cmp5.i1081 = icmp slt i32 %add12.i1079, %storemerge118.i1068
  br i1 %cmp5.i1081, label %for.body6.i1082, label %for.inc14.i1085

for.inc14.i1085:                                  ; preds = %for.body6.i1082, %for.cond4.preheader.i1070
  %inc15.i1083 = add i32 %storemerge118.i1068, 1
  %cmp2.i1084 = icmp slt i32 %inc15.i1083, %storemerge21.i1061
  br i1 %cmp2.i1084, label %for.cond4.preheader.i1070, label %for.inc17.i1088

for.inc17.i1088:                                  ; preds = %for.inc14.i1085, %for.cond1.preheader.i1063
  %inc18.i1086 = add i32 %storemerge21.i1061, 1
  %cmp.i1087 = icmp slt i32 %inc18.i1086, %storemerge144904497
  br i1 %cmp.i1087, label %for.cond1.preheader.i1063, label %foo3.exit1089

foo3.exit1089:                                    ; preds = %for.inc17.i1088, %for.body23
  %gep_array5716 = mul i32 %storemerge214498, 2
  %gep5717 = add i32 %fr, %gep_array5716
  %gep5717.asptr = inttoptr i32 %gep5717 to i16*
  %303 = load i16* %gep5717.asptr, align 1
  %conv25 = sext i16 %303 to i32
  %cmp26 = icmp slt i16 %303, 0
  %sub29 = sub i32 0, %conv25
  %sub29.conv25 = select i1 %cmp26, i32 %sub29, i32 %conv25
  %gep_array5719 = mul i32 %storemerge214498, 2
  %gep5720 = add i32 %fi, %gep_array5719
  %gep5720.asptr = inttoptr i32 %gep5720 to i16*
  %304 = load i16* %gep5720.asptr, align 1
  %conv32 = sext i16 %304 to i32
  %cmp33 = icmp slt i16 %304, 0
  %sub36 = sub i32 0, %conv32
  %storemerge14489 = select i1 %cmp33, i32 %sub36, i32 %conv32
  %cmp38 = icmp sgt i32 %sub29.conv25, 16383
  %cmp40 = icmp sgt i32 %storemerge14489, 16383
  %or.cond = or i1 %cmp38, %cmp40
  %inc45 = add i32 %storemerge214498, 1
  br i1 %or.cond, label %if.then48, label %for.cond20

if.then48:                                        ; preds = %foo3.exit1089
  %inc49 = add i32 %scale.0.load401744834509, 1
  br label %if.end51

if.end51:                                         ; preds = %for.cond20, %while.body, %if.then48
  %scale.0.load40174482 = phi i32 [ %inc49, %if.then48 ], [ %scale.0.load401744834509, %while.body ], [ %scale.0.load401744834509, %for.cond20 ]
  %shift.0.load40164480 = phi i32 [ 1, %if.then48 ], [ 1, %while.body ], [ 0, %for.cond20 ]
  %shl52 = shl i32 %shl5244854506, 1
  %cmp544501 = icmp sgt i32 %shl5244854506, 0
  br i1 %cmp544501, label %for.body56.lr.ph, label %for.end136

for.body56.lr.ph:                                 ; preds = %if.end51
  %tobool70 = icmp eq i32 %shift.0.load40164480, 0
  br label %for.body56

for.body56:                                       ; preds = %for.body56.lr.ph, %for.inc134
  %storemerge14502 = phi i32 [ 0, %for.body56.lr.ph ], [ %inc135, %for.inc134 ]
  %shl57 = shl i32 %storemerge14502, %dec44844507
  %add58 = add i32 %shl57, 256
  %gep_array5722 = mul i32 %add58, 2
  %gep5723 = add i32 %Sinewave, %gep_array5722
  %gep5723.asptr = inttoptr i32 %gep5723 to i16*
  %305 = load i16* %gep5723.asptr, align 1
  %gep_array5725 = mul i32 %shl57, 2
  %gep5726 = add i32 %Sinewave, %gep_array5725
  %gep5726.asptr = inttoptr i32 %gep5726 to i16*
  %306 = load i16* %gep5726.asptr, align 1
  %sub62 = sub i16 0, %306
  %sub62. = select i1 %tobool, i16 %sub62, i16 %306
  br i1 %tobool70, label %if.end78, label %if.then71

if.then71:                                        ; preds = %for.body56
  %cmp20.i1030 = icmp sgt i32 %storemerge14502, 0
  br i1 %cmp20.i1030, label %for.cond1.preheader.i1033, label %foo3.exit1029

for.cond1.preheader.i1033:                        ; preds = %if.then71, %for.inc17.i1058
  %storemerge21.i1031 = phi i32 [ %inc18.i1056, %for.inc17.i1058 ], [ 0, %if.then71 ]
  %cmp217.i1032 = icmp sgt i32 %storemerge21.i1031, 0
  br i1 %cmp217.i1032, label %for.cond4.preheader.lr.ph.i1037, label %for.inc17.i1058

for.cond4.preheader.lr.ph.i1037:                  ; preds = %for.cond1.preheader.i1033
  %sub.i1034 = add i32 %storemerge21.i1031, -2
  %gep_array5728 = mul i32 %sub.i1034, 2
  %gep5729 = add i32 %fr, %gep_array5728
  %gep_array5731 = mul i32 %storemerge21.i1031, 2
  %gep5732 = add i32 %fr, %gep_array5731
  br label %for.cond4.preheader.i1040

for.cond4.preheader.i1040:                        ; preds = %for.inc14.i1055, %for.cond4.preheader.lr.ph.i1037
  %storemerge118.i1038 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1037 ], [ %inc15.i1053, %for.inc14.i1055 ]
  %cmp515.i1039 = icmp sgt i32 %storemerge118.i1038, 0
  br i1 %cmp515.i1039, label %for.body6.lr.ph.i1042, label %for.inc14.i1055

for.body6.lr.ph.i1042:                            ; preds = %for.cond4.preheader.i1040
  %gep_array5734 = mul i32 %storemerge118.i1038, 2
  %gep5735 = add i32 %fi, %gep_array5734
  br label %for.body6.i1052

for.body6.i1052:                                  ; preds = %for.body6.i1052, %for.body6.lr.ph.i1042
  %storemerge216.i1043 = phi i32 [ 0, %for.body6.lr.ph.i1042 ], [ %add12.i1049, %for.body6.i1052 ]
  %gep5735.asptr = inttoptr i32 %gep5735 to i16*
  %307 = load i16* %gep5735.asptr, align 1
  %conv.i1044 = sext i16 %307 to i32
  %mul.i1045 = mul i32 %conv.i1044, %storemerge14502
  %gep5729.asptr = inttoptr i32 %gep5729 to i16*
  %308 = load i16* %gep5729.asptr, align 1
  %conv83.i1046 = zext i16 %308 to i32
  %add.i1047 = add i32 %mul.i1045, %conv83.i1046
  %conv9.i1048 = trunc i32 %add.i1047 to i16
  %gep5732.asptr = inttoptr i32 %gep5732 to i16*
  store i16 %conv9.i1048, i16* %gep5732.asptr, align 1
  %gep5735.asptr104 = inttoptr i32 %gep5735 to i16*
  %309 = load i16* %gep5735.asptr104, align 1
  %add12.i1049 = add i32 %storemerge216.i1043, 1
  %gep_array5737 = mul i32 %add12.i1049, 2
  %gep5738 = add i32 %fr, %gep_array5737
  %gep5738.asptr = inttoptr i32 %gep5738 to i16*
  store i16 %309, i16* %gep5738.asptr, align 1
  %cmp5.i1051 = icmp slt i32 %add12.i1049, %storemerge118.i1038
  br i1 %cmp5.i1051, label %for.body6.i1052, label %for.inc14.i1055

for.inc14.i1055:                                  ; preds = %for.body6.i1052, %for.cond4.preheader.i1040
  %inc15.i1053 = add i32 %storemerge118.i1038, 1
  %cmp2.i1054 = icmp slt i32 %inc15.i1053, %storemerge21.i1031
  br i1 %cmp2.i1054, label %for.cond4.preheader.i1040, label %for.inc17.i1058

for.inc17.i1058:                                  ; preds = %for.inc14.i1055, %for.cond1.preheader.i1033
  %inc18.i1056 = add i32 %storemerge21.i1031, 1
  %cmp.i1057 = icmp slt i32 %inc18.i1056, %storemerge14502
  br i1 %cmp.i1057, label %for.cond1.preheader.i1033, label %for.cond1.preheader.i1003

for.cond1.preheader.i1003:                        ; preds = %for.inc17.i1058, %for.inc17.i1028
  %storemerge21.i1001 = phi i32 [ %inc18.i1026, %for.inc17.i1028 ], [ 0, %for.inc17.i1058 ]
  %cmp217.i1002 = icmp sgt i32 %storemerge21.i1001, 0
  br i1 %cmp217.i1002, label %for.cond4.preheader.lr.ph.i1007, label %for.inc17.i1028

for.cond4.preheader.lr.ph.i1007:                  ; preds = %for.cond1.preheader.i1003
  %sub.i1004 = add i32 %storemerge21.i1001, -2
  %gep_array5740 = mul i32 %sub.i1004, 2
  %gep5741 = add i32 %fr, %gep_array5740
  %gep_array5743 = mul i32 %storemerge21.i1001, 2
  %gep5744 = add i32 %fr, %gep_array5743
  br label %for.cond4.preheader.i1010

for.cond4.preheader.i1010:                        ; preds = %for.inc14.i1025, %for.cond4.preheader.lr.ph.i1007
  %storemerge118.i1008 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i1007 ], [ %inc15.i1023, %for.inc14.i1025 ]
  %cmp515.i1009 = icmp sgt i32 %storemerge118.i1008, 0
  br i1 %cmp515.i1009, label %for.body6.lr.ph.i1012, label %for.inc14.i1025

for.body6.lr.ph.i1012:                            ; preds = %for.cond4.preheader.i1010
  %gep_array5746 = mul i32 %storemerge118.i1008, 2
  %gep5747 = add i32 %Sinewave, %gep_array5746
  br label %for.body6.i1022

for.body6.i1022:                                  ; preds = %for.body6.i1022, %for.body6.lr.ph.i1012
  %storemerge216.i1013 = phi i32 [ 0, %for.body6.lr.ph.i1012 ], [ %add12.i1019, %for.body6.i1022 ]
  %gep5747.asptr = inttoptr i32 %gep5747 to i16*
  %310 = load i16* %gep5747.asptr, align 1
  %conv.i1014 = sext i16 %310 to i32
  %mul.i1015 = mul i32 %conv.i1014, %storemerge14502
  %gep5741.asptr = inttoptr i32 %gep5741 to i16*
  %311 = load i16* %gep5741.asptr, align 1
  %conv83.i1016 = zext i16 %311 to i32
  %add.i1017 = add i32 %mul.i1015, %conv83.i1016
  %conv9.i1018 = trunc i32 %add.i1017 to i16
  %gep5744.asptr = inttoptr i32 %gep5744 to i16*
  store i16 %conv9.i1018, i16* %gep5744.asptr, align 1
  %gep5747.asptr105 = inttoptr i32 %gep5747 to i16*
  %312 = load i16* %gep5747.asptr105, align 1
  %add12.i1019 = add i32 %storemerge216.i1013, 1
  %gep_array5749 = mul i32 %add12.i1019, 2
  %gep5750 = add i32 %fr, %gep_array5749
  %gep5750.asptr = inttoptr i32 %gep5750 to i16*
  store i16 %312, i16* %gep5750.asptr, align 1
  %cmp5.i1021 = icmp slt i32 %add12.i1019, %storemerge118.i1008
  br i1 %cmp5.i1021, label %for.body6.i1022, label %for.inc14.i1025

for.inc14.i1025:                                  ; preds = %for.body6.i1022, %for.cond4.preheader.i1010
  %inc15.i1023 = add i32 %storemerge118.i1008, 1
  %cmp2.i1024 = icmp slt i32 %inc15.i1023, %storemerge21.i1001
  br i1 %cmp2.i1024, label %for.cond4.preheader.i1010, label %for.inc17.i1028

for.inc17.i1028:                                  ; preds = %for.inc14.i1025, %for.cond1.preheader.i1003
  %inc18.i1026 = add i32 %storemerge21.i1001, 1
  %cmp.i1027 = icmp slt i32 %inc18.i1026, %storemerge14502
  br i1 %cmp.i1027, label %for.cond1.preheader.i1003, label %foo3.exit1029

foo3.exit1029:                                    ; preds = %for.inc17.i1028, %if.then71
  %conv72 = sext i16 %305 to i32
  %shr7318 = lshr i32 %conv72, 1
  %conv74 = trunc i32 %shr7318 to i16
  %conv75 = sext i16 %sub62. to i32
  %shr7619 = lshr i32 %conv75, 1
  %conv77 = trunc i32 %shr7619 to i16
  br label %if.end78

if.end78:                                         ; preds = %for.body56, %foo3.exit1029
  %wr.0.load40064477 = phi i16 [ %305, %for.body56 ], [ %conv74, %foo3.exit1029 ]
  %wi.0.load40044475 = phi i16 [ %sub62., %for.body56 ], [ %conv77, %foo3.exit1029 ]
  %cmp804499 = icmp slt i32 %storemerge14502, %shl
  br i1 %cmp804499, label %for.body82.lr.ph, label %for.inc134

for.body82.lr.ph:                                 ; preds = %if.end78
  %conv.i995 = sext i16 %wr.0.load40064477 to i32
  %conv.i990 = sext i16 %wi.0.load40044475 to i32
  %cmp20.i953 = icmp sgt i32 %storemerge14502, 0
  br label %for.body82

for.body82:                                       ; preds = %for.body82.lr.ph, %if.end110
  %storemerge34500 = phi i32 [ %storemerge14502, %for.body82.lr.ph ], [ %add132, %if.end110 ]
  %add83 = add i32 %storemerge34500, %shl5244854506
  %gep_array5752 = mul i32 %add83, 2
  %gep5753 = add i32 %fr, %gep_array5752
  %gep5753.asptr = inttoptr i32 %gep5753 to i16*
  %313 = load i16* %gep5753.asptr, align 1
  %conv1.i996 = sext i16 %313 to i32
  %mul.i997 = mul i32 %conv1.i996, %conv.i995
  %shr1.i998 = lshr i32 %mul.i997, 15
  %conv2.i999 = trunc i32 %shr1.i998 to i16
  %gep_array5755 = mul i32 %add83, 2
  %gep5756 = add i32 %fi, %gep_array5755
  %gep5756.asptr = inttoptr i32 %gep5756 to i16*
  %314 = load i16* %gep5756.asptr, align 1
  %conv1.i991 = sext i16 %314 to i32
  %mul.i992 = mul i32 %conv1.i991, %conv.i990
  %shr1.i993 = lshr i32 %mul.i992, 15
  %conv2.i994 = trunc i32 %shr1.i993 to i16
  %sub90 = sub i16 %conv2.i999, %conv2.i994
  %mul.i987 = mul i32 %conv1.i991, %conv.i995
  %shr1.i988 = lshr i32 %mul.i987, 15
  %conv2.i989 = trunc i32 %shr1.i988 to i16
  %mul.i984 = mul i32 %conv1.i996, %conv.i990
  %shr1.i = lshr i32 %mul.i984, 15
  %conv2.i = trunc i32 %shr1.i to i16
  %add98 = add i16 %conv2.i989, %conv2.i
  br i1 %cmp20.i953, label %for.cond1.preheader.i956, label %foo3.exit52

for.cond1.preheader.i956:                         ; preds = %for.body82, %for.inc17.i981
  %storemerge21.i954 = phi i32 [ %inc18.i979, %for.inc17.i981 ], [ 0, %for.body82 ]
  %cmp217.i955 = icmp sgt i32 %storemerge21.i954, 0
  br i1 %cmp217.i955, label %for.cond4.preheader.lr.ph.i960, label %for.inc17.i981

for.cond4.preheader.lr.ph.i960:                   ; preds = %for.cond1.preheader.i956
  %sub.i957 = add i32 %storemerge21.i954, -2
  %gep_array5758 = mul i32 %sub.i957, 2
  %gep5759 = add i32 %fr, %gep_array5758
  %gep_array5761 = mul i32 %storemerge21.i954, 2
  %gep5762 = add i32 %fr, %gep_array5761
  br label %for.cond4.preheader.i963

for.cond4.preheader.i963:                         ; preds = %for.inc14.i978, %for.cond4.preheader.lr.ph.i960
  %storemerge118.i961 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i960 ], [ %inc15.i976, %for.inc14.i978 ]
  %cmp515.i962 = icmp sgt i32 %storemerge118.i961, 0
  br i1 %cmp515.i962, label %for.body6.lr.ph.i965, label %for.inc14.i978

for.body6.lr.ph.i965:                             ; preds = %for.cond4.preheader.i963
  %gep_array5764 = mul i32 %storemerge118.i961, 2
  %gep5765 = add i32 %fi, %gep_array5764
  br label %for.body6.i975

for.body6.i975:                                   ; preds = %for.body6.i975, %for.body6.lr.ph.i965
  %storemerge216.i966 = phi i32 [ 0, %for.body6.lr.ph.i965 ], [ %add12.i972, %for.body6.i975 ]
  %gep5765.asptr = inttoptr i32 %gep5765 to i16*
  %315 = load i16* %gep5765.asptr, align 1
  %conv.i967 = sext i16 %315 to i32
  %mul.i968 = mul i32 %conv.i967, %storemerge14502
  %gep5759.asptr = inttoptr i32 %gep5759 to i16*
  %316 = load i16* %gep5759.asptr, align 1
  %conv83.i969 = zext i16 %316 to i32
  %add.i970 = add i32 %mul.i968, %conv83.i969
  %conv9.i971 = trunc i32 %add.i970 to i16
  %gep5762.asptr = inttoptr i32 %gep5762 to i16*
  store i16 %conv9.i971, i16* %gep5762.asptr, align 1
  %gep5765.asptr106 = inttoptr i32 %gep5765 to i16*
  %317 = load i16* %gep5765.asptr106, align 1
  %add12.i972 = add i32 %storemerge216.i966, 1
  %gep_array5767 = mul i32 %add12.i972, 2
  %gep5768 = add i32 %fr, %gep_array5767
  %gep5768.asptr = inttoptr i32 %gep5768 to i16*
  store i16 %317, i16* %gep5768.asptr, align 1
  %cmp5.i974 = icmp slt i32 %add12.i972, %storemerge118.i961
  br i1 %cmp5.i974, label %for.body6.i975, label %for.inc14.i978

for.inc14.i978:                                   ; preds = %for.body6.i975, %for.cond4.preheader.i963
  %inc15.i976 = add i32 %storemerge118.i961, 1
  %cmp2.i977 = icmp slt i32 %inc15.i976, %storemerge21.i954
  br i1 %cmp2.i977, label %for.cond4.preheader.i963, label %for.inc17.i981

for.inc17.i981:                                   ; preds = %for.inc14.i978, %for.cond1.preheader.i956
  %inc18.i979 = add i32 %storemerge21.i954, 1
  %cmp.i980 = icmp slt i32 %inc18.i979, %storemerge14502
  br i1 %cmp.i980, label %for.cond1.preheader.i956, label %for.cond1.preheader.i926

for.cond1.preheader.i926:                         ; preds = %for.inc17.i981, %for.inc17.i951
  %storemerge21.i924 = phi i32 [ %inc18.i949, %for.inc17.i951 ], [ 0, %for.inc17.i981 ]
  %cmp217.i925 = icmp sgt i32 %storemerge21.i924, 0
  br i1 %cmp217.i925, label %for.cond4.preheader.lr.ph.i930, label %for.inc17.i951

for.cond4.preheader.lr.ph.i930:                   ; preds = %for.cond1.preheader.i926
  %sub.i927 = add i32 %storemerge21.i924, -2
  %gep_array5770 = mul i32 %sub.i927, 2
  %gep5771 = add i32 %fr, %gep_array5770
  %gep_array5773 = mul i32 %storemerge21.i924, 2
  %gep5774 = add i32 %fr, %gep_array5773
  br label %for.cond4.preheader.i933

for.cond4.preheader.i933:                         ; preds = %for.inc14.i948, %for.cond4.preheader.lr.ph.i930
  %storemerge118.i931 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i930 ], [ %inc15.i946, %for.inc14.i948 ]
  %cmp515.i932 = icmp sgt i32 %storemerge118.i931, 0
  br i1 %cmp515.i932, label %for.body6.lr.ph.i935, label %for.inc14.i948

for.body6.lr.ph.i935:                             ; preds = %for.cond4.preheader.i933
  %gep_array5776 = mul i32 %storemerge118.i931, 2
  %gep5777 = add i32 %Sinewave, %gep_array5776
  br label %for.body6.i945

for.body6.i945:                                   ; preds = %for.body6.i945, %for.body6.lr.ph.i935
  %storemerge216.i936 = phi i32 [ 0, %for.body6.lr.ph.i935 ], [ %add12.i942, %for.body6.i945 ]
  %gep5777.asptr = inttoptr i32 %gep5777 to i16*
  %318 = load i16* %gep5777.asptr, align 1
  %conv.i937 = sext i16 %318 to i32
  %mul.i938 = mul i32 %conv.i937, %storemerge14502
  %gep5771.asptr = inttoptr i32 %gep5771 to i16*
  %319 = load i16* %gep5771.asptr, align 1
  %conv83.i939 = zext i16 %319 to i32
  %add.i940 = add i32 %mul.i938, %conv83.i939
  %conv9.i941 = trunc i32 %add.i940 to i16
  %gep5774.asptr = inttoptr i32 %gep5774 to i16*
  store i16 %conv9.i941, i16* %gep5774.asptr, align 1
  %gep5777.asptr107 = inttoptr i32 %gep5777 to i16*
  %320 = load i16* %gep5777.asptr107, align 1
  %add12.i942 = add i32 %storemerge216.i936, 1
  %gep_array5779 = mul i32 %add12.i942, 2
  %gep5780 = add i32 %fr, %gep_array5779
  %gep5780.asptr = inttoptr i32 %gep5780 to i16*
  store i16 %320, i16* %gep5780.asptr, align 1
  %cmp5.i944 = icmp slt i32 %add12.i942, %storemerge118.i931
  br i1 %cmp5.i944, label %for.body6.i945, label %for.inc14.i948

for.inc14.i948:                                   ; preds = %for.body6.i945, %for.cond4.preheader.i933
  %inc15.i946 = add i32 %storemerge118.i931, 1
  %cmp2.i947 = icmp slt i32 %inc15.i946, %storemerge21.i924
  br i1 %cmp2.i947, label %for.cond4.preheader.i933, label %for.inc17.i951

for.inc17.i951:                                   ; preds = %for.inc14.i948, %for.cond1.preheader.i926
  %inc18.i949 = add i32 %storemerge21.i924, 1
  %cmp.i950 = icmp slt i32 %inc18.i949, %storemerge14502
  br i1 %cmp.i950, label %for.cond1.preheader.i926, label %for.cond1.preheader.i896

for.cond1.preheader.i896:                         ; preds = %for.inc17.i951, %for.inc17.i921
  %storemerge21.i894 = phi i32 [ %inc18.i919, %for.inc17.i921 ], [ 0, %for.inc17.i951 ]
  %cmp217.i895 = icmp sgt i32 %storemerge21.i894, 0
  br i1 %cmp217.i895, label %for.cond4.preheader.lr.ph.i900, label %for.inc17.i921

for.cond4.preheader.lr.ph.i900:                   ; preds = %for.cond1.preheader.i896
  %sub.i897 = add i32 %storemerge21.i894, -2
  %gep_array5782 = mul i32 %sub.i897, 2
  %gep5783 = add i32 %fr, %gep_array5782
  %gep_array5785 = mul i32 %storemerge21.i894, 2
  %gep5786 = add i32 %fr, %gep_array5785
  br label %for.cond4.preheader.i903

for.cond4.preheader.i903:                         ; preds = %for.inc14.i918, %for.cond4.preheader.lr.ph.i900
  %storemerge118.i901 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i900 ], [ %inc15.i916, %for.inc14.i918 ]
  %cmp515.i902 = icmp sgt i32 %storemerge118.i901, 0
  br i1 %cmp515.i902, label %for.body6.lr.ph.i905, label %for.inc14.i918

for.body6.lr.ph.i905:                             ; preds = %for.cond4.preheader.i903
  %gep_array5788 = mul i32 %storemerge118.i901, 2
  %gep5789 = add i32 %fi, %gep_array5788
  br label %for.body6.i915

for.body6.i915:                                   ; preds = %for.body6.i915, %for.body6.lr.ph.i905
  %storemerge216.i906 = phi i32 [ 0, %for.body6.lr.ph.i905 ], [ %add12.i912, %for.body6.i915 ]
  %gep5789.asptr = inttoptr i32 %gep5789 to i16*
  %321 = load i16* %gep5789.asptr, align 1
  %conv.i907 = sext i16 %321 to i32
  %mul.i908 = mul i32 %conv.i907, %storemerge14502
  %gep5783.asptr = inttoptr i32 %gep5783 to i16*
  %322 = load i16* %gep5783.asptr, align 1
  %conv83.i909 = zext i16 %322 to i32
  %add.i910 = add i32 %mul.i908, %conv83.i909
  %conv9.i911 = trunc i32 %add.i910 to i16
  %gep5786.asptr = inttoptr i32 %gep5786 to i16*
  store i16 %conv9.i911, i16* %gep5786.asptr, align 1
  %gep5789.asptr108 = inttoptr i32 %gep5789 to i16*
  %323 = load i16* %gep5789.asptr108, align 1
  %add12.i912 = add i32 %storemerge216.i906, 1
  %gep_array5791 = mul i32 %add12.i912, 2
  %gep5792 = add i32 %fr, %gep_array5791
  %gep5792.asptr = inttoptr i32 %gep5792 to i16*
  store i16 %323, i16* %gep5792.asptr, align 1
  %cmp5.i914 = icmp slt i32 %add12.i912, %storemerge118.i901
  br i1 %cmp5.i914, label %for.body6.i915, label %for.inc14.i918

for.inc14.i918:                                   ; preds = %for.body6.i915, %for.cond4.preheader.i903
  %inc15.i916 = add i32 %storemerge118.i901, 1
  %cmp2.i917 = icmp slt i32 %inc15.i916, %storemerge21.i894
  br i1 %cmp2.i917, label %for.cond4.preheader.i903, label %for.inc17.i921

for.inc17.i921:                                   ; preds = %for.inc14.i918, %for.cond1.preheader.i896
  %inc18.i919 = add i32 %storemerge21.i894, 1
  %cmp.i920 = icmp slt i32 %inc18.i919, %storemerge14502
  br i1 %cmp.i920, label %for.cond1.preheader.i896, label %for.cond1.preheader.i866

for.cond1.preheader.i866:                         ; preds = %for.inc17.i921, %for.inc17.i891
  %storemerge21.i864 = phi i32 [ %inc18.i889, %for.inc17.i891 ], [ 0, %for.inc17.i921 ]
  %cmp217.i865 = icmp sgt i32 %storemerge21.i864, 0
  br i1 %cmp217.i865, label %for.cond4.preheader.lr.ph.i870, label %for.inc17.i891

for.cond4.preheader.lr.ph.i870:                   ; preds = %for.cond1.preheader.i866
  %sub.i867 = add i32 %storemerge21.i864, -2
  %gep_array5794 = mul i32 %sub.i867, 2
  %gep5795 = add i32 %fr, %gep_array5794
  %gep_array5797 = mul i32 %storemerge21.i864, 2
  %gep5798 = add i32 %fr, %gep_array5797
  br label %for.cond4.preheader.i873

for.cond4.preheader.i873:                         ; preds = %for.inc14.i888, %for.cond4.preheader.lr.ph.i870
  %storemerge118.i871 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i870 ], [ %inc15.i886, %for.inc14.i888 ]
  %cmp515.i872 = icmp sgt i32 %storemerge118.i871, 0
  br i1 %cmp515.i872, label %for.body6.lr.ph.i875, label %for.inc14.i888

for.body6.lr.ph.i875:                             ; preds = %for.cond4.preheader.i873
  %gep_array5800 = mul i32 %storemerge118.i871, 2
  %gep5801 = add i32 %fi, %gep_array5800
  br label %for.body6.i885

for.body6.i885:                                   ; preds = %for.body6.i885, %for.body6.lr.ph.i875
  %storemerge216.i876 = phi i32 [ 0, %for.body6.lr.ph.i875 ], [ %add12.i882, %for.body6.i885 ]
  %gep5801.asptr = inttoptr i32 %gep5801 to i16*
  %324 = load i16* %gep5801.asptr, align 1
  %conv.i877 = sext i16 %324 to i32
  %mul.i878 = mul i32 %conv.i877, %storemerge14502
  %gep5795.asptr = inttoptr i32 %gep5795 to i16*
  %325 = load i16* %gep5795.asptr, align 1
  %conv83.i879 = zext i16 %325 to i32
  %add.i880 = add i32 %mul.i878, %conv83.i879
  %conv9.i881 = trunc i32 %add.i880 to i16
  %gep5798.asptr = inttoptr i32 %gep5798 to i16*
  store i16 %conv9.i881, i16* %gep5798.asptr, align 1
  %gep5801.asptr109 = inttoptr i32 %gep5801 to i16*
  %326 = load i16* %gep5801.asptr109, align 1
  %add12.i882 = add i32 %storemerge216.i876, 1
  %gep_array5803 = mul i32 %add12.i882, 2
  %gep5804 = add i32 %fr, %gep_array5803
  %gep5804.asptr = inttoptr i32 %gep5804 to i16*
  store i16 %326, i16* %gep5804.asptr, align 1
  %cmp5.i884 = icmp slt i32 %add12.i882, %storemerge118.i871
  br i1 %cmp5.i884, label %for.body6.i885, label %for.inc14.i888

for.inc14.i888:                                   ; preds = %for.body6.i885, %for.cond4.preheader.i873
  %inc15.i886 = add i32 %storemerge118.i871, 1
  %cmp2.i887 = icmp slt i32 %inc15.i886, %storemerge21.i864
  br i1 %cmp2.i887, label %for.cond4.preheader.i873, label %for.inc17.i891

for.inc17.i891:                                   ; preds = %for.inc14.i888, %for.cond1.preheader.i866
  %inc18.i889 = add i32 %storemerge21.i864, 1
  %cmp.i890 = icmp slt i32 %inc18.i889, %storemerge14502
  br i1 %cmp.i890, label %for.cond1.preheader.i866, label %for.cond1.preheader.i836

for.cond1.preheader.i836:                         ; preds = %for.inc17.i891, %for.inc17.i861
  %storemerge21.i834 = phi i32 [ %inc18.i859, %for.inc17.i861 ], [ 0, %for.inc17.i891 ]
  %cmp217.i835 = icmp sgt i32 %storemerge21.i834, 0
  br i1 %cmp217.i835, label %for.cond4.preheader.lr.ph.i840, label %for.inc17.i861

for.cond4.preheader.lr.ph.i840:                   ; preds = %for.cond1.preheader.i836
  %sub.i837 = add i32 %storemerge21.i834, -2
  %gep_array5806 = mul i32 %sub.i837, 2
  %gep5807 = add i32 %fr, %gep_array5806
  %gep_array5809 = mul i32 %storemerge21.i834, 2
  %gep5810 = add i32 %fr, %gep_array5809
  br label %for.cond4.preheader.i843

for.cond4.preheader.i843:                         ; preds = %for.inc14.i858, %for.cond4.preheader.lr.ph.i840
  %storemerge118.i841 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i840 ], [ %inc15.i856, %for.inc14.i858 ]
  %cmp515.i842 = icmp sgt i32 %storemerge118.i841, 0
  br i1 %cmp515.i842, label %for.body6.lr.ph.i845, label %for.inc14.i858

for.body6.lr.ph.i845:                             ; preds = %for.cond4.preheader.i843
  %gep_array5812 = mul i32 %storemerge118.i841, 2
  %gep5813 = add i32 %fi, %gep_array5812
  br label %for.body6.i855

for.body6.i855:                                   ; preds = %for.body6.i855, %for.body6.lr.ph.i845
  %storemerge216.i846 = phi i32 [ 0, %for.body6.lr.ph.i845 ], [ %add12.i852, %for.body6.i855 ]
  %gep5813.asptr = inttoptr i32 %gep5813 to i16*
  %327 = load i16* %gep5813.asptr, align 1
  %conv.i847 = sext i16 %327 to i32
  %mul.i848 = mul i32 %conv.i847, %storemerge14502
  %gep5807.asptr = inttoptr i32 %gep5807 to i16*
  %328 = load i16* %gep5807.asptr, align 1
  %conv83.i849 = zext i16 %328 to i32
  %add.i850 = add i32 %mul.i848, %conv83.i849
  %conv9.i851 = trunc i32 %add.i850 to i16
  %gep5810.asptr = inttoptr i32 %gep5810 to i16*
  store i16 %conv9.i851, i16* %gep5810.asptr, align 1
  %gep5813.asptr110 = inttoptr i32 %gep5813 to i16*
  %329 = load i16* %gep5813.asptr110, align 1
  %add12.i852 = add i32 %storemerge216.i846, 1
  %gep_array5815 = mul i32 %add12.i852, 2
  %gep5816 = add i32 %fr, %gep_array5815
  %gep5816.asptr = inttoptr i32 %gep5816 to i16*
  store i16 %329, i16* %gep5816.asptr, align 1
  %cmp5.i854 = icmp slt i32 %add12.i852, %storemerge118.i841
  br i1 %cmp5.i854, label %for.body6.i855, label %for.inc14.i858

for.inc14.i858:                                   ; preds = %for.body6.i855, %for.cond4.preheader.i843
  %inc15.i856 = add i32 %storemerge118.i841, 1
  %cmp2.i857 = icmp slt i32 %inc15.i856, %storemerge21.i834
  br i1 %cmp2.i857, label %for.cond4.preheader.i843, label %for.inc17.i861

for.inc17.i861:                                   ; preds = %for.inc14.i858, %for.cond1.preheader.i836
  %inc18.i859 = add i32 %storemerge21.i834, 1
  %cmp.i860 = icmp slt i32 %inc18.i859, %storemerge14502
  br i1 %cmp.i860, label %for.cond1.preheader.i836, label %for.cond1.preheader.i806

for.cond1.preheader.i806:                         ; preds = %for.inc17.i861, %for.inc17.i831
  %storemerge21.i804 = phi i32 [ %inc18.i829, %for.inc17.i831 ], [ 0, %for.inc17.i861 ]
  %cmp217.i805 = icmp sgt i32 %storemerge21.i804, 0
  br i1 %cmp217.i805, label %for.cond4.preheader.lr.ph.i810, label %for.inc17.i831

for.cond4.preheader.lr.ph.i810:                   ; preds = %for.cond1.preheader.i806
  %sub.i807 = add i32 %storemerge21.i804, -2
  %gep_array5818 = mul i32 %sub.i807, 2
  %gep5819 = add i32 %fr, %gep_array5818
  %gep_array5821 = mul i32 %storemerge21.i804, 2
  %gep5822 = add i32 %fr, %gep_array5821
  br label %for.cond4.preheader.i813

for.cond4.preheader.i813:                         ; preds = %for.inc14.i828, %for.cond4.preheader.lr.ph.i810
  %storemerge118.i811 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i810 ], [ %inc15.i826, %for.inc14.i828 ]
  %cmp515.i812 = icmp sgt i32 %storemerge118.i811, 0
  br i1 %cmp515.i812, label %for.body6.lr.ph.i815, label %for.inc14.i828

for.body6.lr.ph.i815:                             ; preds = %for.cond4.preheader.i813
  %gep_array5824 = mul i32 %storemerge118.i811, 2
  %gep5825 = add i32 %Sinewave, %gep_array5824
  br label %for.body6.i825

for.body6.i825:                                   ; preds = %for.body6.i825, %for.body6.lr.ph.i815
  %storemerge216.i816 = phi i32 [ 0, %for.body6.lr.ph.i815 ], [ %add12.i822, %for.body6.i825 ]
  %gep5825.asptr = inttoptr i32 %gep5825 to i16*
  %330 = load i16* %gep5825.asptr, align 1
  %conv.i817 = sext i16 %330 to i32
  %mul.i818 = mul i32 %conv.i817, %storemerge14502
  %gep5819.asptr = inttoptr i32 %gep5819 to i16*
  %331 = load i16* %gep5819.asptr, align 1
  %conv83.i819 = zext i16 %331 to i32
  %add.i820 = add i32 %mul.i818, %conv83.i819
  %conv9.i821 = trunc i32 %add.i820 to i16
  %gep5822.asptr = inttoptr i32 %gep5822 to i16*
  store i16 %conv9.i821, i16* %gep5822.asptr, align 1
  %gep5825.asptr111 = inttoptr i32 %gep5825 to i16*
  %332 = load i16* %gep5825.asptr111, align 1
  %add12.i822 = add i32 %storemerge216.i816, 1
  %gep_array5827 = mul i32 %add12.i822, 2
  %gep5828 = add i32 %fr, %gep_array5827
  %gep5828.asptr = inttoptr i32 %gep5828 to i16*
  store i16 %332, i16* %gep5828.asptr, align 1
  %cmp5.i824 = icmp slt i32 %add12.i822, %storemerge118.i811
  br i1 %cmp5.i824, label %for.body6.i825, label %for.inc14.i828

for.inc14.i828:                                   ; preds = %for.body6.i825, %for.cond4.preheader.i813
  %inc15.i826 = add i32 %storemerge118.i811, 1
  %cmp2.i827 = icmp slt i32 %inc15.i826, %storemerge21.i804
  br i1 %cmp2.i827, label %for.cond4.preheader.i813, label %for.inc17.i831

for.inc17.i831:                                   ; preds = %for.inc14.i828, %for.cond1.preheader.i806
  %inc18.i829 = add i32 %storemerge21.i804, 1
  %cmp.i830 = icmp slt i32 %inc18.i829, %storemerge14502
  br i1 %cmp.i830, label %for.cond1.preheader.i806, label %for.cond1.preheader.i776

for.cond1.preheader.i776:                         ; preds = %for.inc17.i831, %for.inc17.i801
  %storemerge21.i774 = phi i32 [ %inc18.i799, %for.inc17.i801 ], [ 0, %for.inc17.i831 ]
  %cmp217.i775 = icmp sgt i32 %storemerge21.i774, 0
  br i1 %cmp217.i775, label %for.cond4.preheader.lr.ph.i780, label %for.inc17.i801

for.cond4.preheader.lr.ph.i780:                   ; preds = %for.cond1.preheader.i776
  %sub.i777 = add i32 %storemerge21.i774, -2
  %gep_array5830 = mul i32 %sub.i777, 2
  %gep5831 = add i32 %fr, %gep_array5830
  %gep_array5833 = mul i32 %storemerge21.i774, 2
  %gep5834 = add i32 %fr, %gep_array5833
  br label %for.cond4.preheader.i783

for.cond4.preheader.i783:                         ; preds = %for.inc14.i798, %for.cond4.preheader.lr.ph.i780
  %storemerge118.i781 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i780 ], [ %inc15.i796, %for.inc14.i798 ]
  %cmp515.i782 = icmp sgt i32 %storemerge118.i781, 0
  br i1 %cmp515.i782, label %for.body6.lr.ph.i785, label %for.inc14.i798

for.body6.lr.ph.i785:                             ; preds = %for.cond4.preheader.i783
  %gep_array5836 = mul i32 %storemerge118.i781, 2
  %gep5837 = add i32 %Sinewave, %gep_array5836
  br label %for.body6.i795

for.body6.i795:                                   ; preds = %for.body6.i795, %for.body6.lr.ph.i785
  %storemerge216.i786 = phi i32 [ 0, %for.body6.lr.ph.i785 ], [ %add12.i792, %for.body6.i795 ]
  %gep5837.asptr = inttoptr i32 %gep5837 to i16*
  %333 = load i16* %gep5837.asptr, align 1
  %conv.i787 = sext i16 %333 to i32
  %mul.i788 = mul i32 %conv.i787, %storemerge14502
  %gep5831.asptr = inttoptr i32 %gep5831 to i16*
  %334 = load i16* %gep5831.asptr, align 1
  %conv83.i789 = zext i16 %334 to i32
  %add.i790 = add i32 %mul.i788, %conv83.i789
  %conv9.i791 = trunc i32 %add.i790 to i16
  %gep5834.asptr = inttoptr i32 %gep5834 to i16*
  store i16 %conv9.i791, i16* %gep5834.asptr, align 1
  %gep5837.asptr112 = inttoptr i32 %gep5837 to i16*
  %335 = load i16* %gep5837.asptr112, align 1
  %add12.i792 = add i32 %storemerge216.i786, 1
  %gep_array5839 = mul i32 %add12.i792, 2
  %gep5840 = add i32 %fr, %gep_array5839
  %gep5840.asptr = inttoptr i32 %gep5840 to i16*
  store i16 %335, i16* %gep5840.asptr, align 1
  %cmp5.i794 = icmp slt i32 %add12.i792, %storemerge118.i781
  br i1 %cmp5.i794, label %for.body6.i795, label %for.inc14.i798

for.inc14.i798:                                   ; preds = %for.body6.i795, %for.cond4.preheader.i783
  %inc15.i796 = add i32 %storemerge118.i781, 1
  %cmp2.i797 = icmp slt i32 %inc15.i796, %storemerge21.i774
  br i1 %cmp2.i797, label %for.cond4.preheader.i783, label %for.inc17.i801

for.inc17.i801:                                   ; preds = %for.inc14.i798, %for.cond1.preheader.i776
  %inc18.i799 = add i32 %storemerge21.i774, 1
  %cmp.i800 = icmp slt i32 %inc18.i799, %storemerge14502
  br i1 %cmp.i800, label %for.cond1.preheader.i776, label %for.cond1.preheader.i746

for.cond1.preheader.i746:                         ; preds = %for.inc17.i801, %for.inc17.i771
  %storemerge21.i744 = phi i32 [ %inc18.i769, %for.inc17.i771 ], [ 0, %for.inc17.i801 ]
  %cmp217.i745 = icmp sgt i32 %storemerge21.i744, 0
  br i1 %cmp217.i745, label %for.cond4.preheader.lr.ph.i750, label %for.inc17.i771

for.cond4.preheader.lr.ph.i750:                   ; preds = %for.cond1.preheader.i746
  %sub.i747 = add i32 %storemerge21.i744, -2
  %gep_array5842 = mul i32 %sub.i747, 2
  %gep5843 = add i32 %fr, %gep_array5842
  %gep_array5845 = mul i32 %storemerge21.i744, 2
  %gep5846 = add i32 %fr, %gep_array5845
  br label %for.cond4.preheader.i753

for.cond4.preheader.i753:                         ; preds = %for.inc14.i768, %for.cond4.preheader.lr.ph.i750
  %storemerge118.i751 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i750 ], [ %inc15.i766, %for.inc14.i768 ]
  %cmp515.i752 = icmp sgt i32 %storemerge118.i751, 0
  br i1 %cmp515.i752, label %for.body6.lr.ph.i755, label %for.inc14.i768

for.body6.lr.ph.i755:                             ; preds = %for.cond4.preheader.i753
  %gep_array5848 = mul i32 %storemerge118.i751, 2
  %gep5849 = add i32 %Sinewave, %gep_array5848
  br label %for.body6.i765

for.body6.i765:                                   ; preds = %for.body6.i765, %for.body6.lr.ph.i755
  %storemerge216.i756 = phi i32 [ 0, %for.body6.lr.ph.i755 ], [ %add12.i762, %for.body6.i765 ]
  %gep5849.asptr = inttoptr i32 %gep5849 to i16*
  %336 = load i16* %gep5849.asptr, align 1
  %conv.i757 = sext i16 %336 to i32
  %mul.i758 = mul i32 %conv.i757, %storemerge14502
  %gep5843.asptr = inttoptr i32 %gep5843 to i16*
  %337 = load i16* %gep5843.asptr, align 1
  %conv83.i759 = zext i16 %337 to i32
  %add.i760 = add i32 %mul.i758, %conv83.i759
  %conv9.i761 = trunc i32 %add.i760 to i16
  %gep5846.asptr = inttoptr i32 %gep5846 to i16*
  store i16 %conv9.i761, i16* %gep5846.asptr, align 1
  %gep5849.asptr113 = inttoptr i32 %gep5849 to i16*
  %338 = load i16* %gep5849.asptr113, align 1
  %add12.i762 = add i32 %storemerge216.i756, 1
  %gep_array5851 = mul i32 %add12.i762, 2
  %gep5852 = add i32 %fr, %gep_array5851
  %gep5852.asptr = inttoptr i32 %gep5852 to i16*
  store i16 %338, i16* %gep5852.asptr, align 1
  %cmp5.i764 = icmp slt i32 %add12.i762, %storemerge118.i751
  br i1 %cmp5.i764, label %for.body6.i765, label %for.inc14.i768

for.inc14.i768:                                   ; preds = %for.body6.i765, %for.cond4.preheader.i753
  %inc15.i766 = add i32 %storemerge118.i751, 1
  %cmp2.i767 = icmp slt i32 %inc15.i766, %storemerge21.i744
  br i1 %cmp2.i767, label %for.cond4.preheader.i753, label %for.inc17.i771

for.inc17.i771:                                   ; preds = %for.inc14.i768, %for.cond1.preheader.i746
  %inc18.i769 = add i32 %storemerge21.i744, 1
  %cmp.i770 = icmp slt i32 %inc18.i769, %storemerge14502
  br i1 %cmp.i770, label %for.cond1.preheader.i746, label %for.cond1.preheader.i716

for.cond1.preheader.i716:                         ; preds = %for.inc17.i771, %for.inc17.i741
  %storemerge21.i714 = phi i32 [ %inc18.i739, %for.inc17.i741 ], [ 0, %for.inc17.i771 ]
  %cmp217.i715 = icmp sgt i32 %storemerge21.i714, 0
  br i1 %cmp217.i715, label %for.cond4.preheader.lr.ph.i720, label %for.inc17.i741

for.cond4.preheader.lr.ph.i720:                   ; preds = %for.cond1.preheader.i716
  %sub.i717 = add i32 %storemerge21.i714, -2
  %gep_array5854 = mul i32 %sub.i717, 2
  %gep5855 = add i32 %fr, %gep_array5854
  %gep_array5857 = mul i32 %storemerge21.i714, 2
  %gep5858 = add i32 %fr, %gep_array5857
  br label %for.cond4.preheader.i723

for.cond4.preheader.i723:                         ; preds = %for.inc14.i738, %for.cond4.preheader.lr.ph.i720
  %storemerge118.i721 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i720 ], [ %inc15.i736, %for.inc14.i738 ]
  %cmp515.i722 = icmp sgt i32 %storemerge118.i721, 0
  br i1 %cmp515.i722, label %for.body6.lr.ph.i725, label %for.inc14.i738

for.body6.lr.ph.i725:                             ; preds = %for.cond4.preheader.i723
  %gep_array5860 = mul i32 %storemerge118.i721, 2
  %gep5861 = add i32 %fi, %gep_array5860
  br label %for.body6.i735

for.body6.i735:                                   ; preds = %for.body6.i735, %for.body6.lr.ph.i725
  %storemerge216.i726 = phi i32 [ 0, %for.body6.lr.ph.i725 ], [ %add12.i732, %for.body6.i735 ]
  %gep5861.asptr = inttoptr i32 %gep5861 to i16*
  %339 = load i16* %gep5861.asptr, align 1
  %conv.i727 = sext i16 %339 to i32
  %mul.i728 = mul i32 %conv.i727, %storemerge14502
  %gep5855.asptr = inttoptr i32 %gep5855 to i16*
  %340 = load i16* %gep5855.asptr, align 1
  %conv83.i729 = zext i16 %340 to i32
  %add.i730 = add i32 %mul.i728, %conv83.i729
  %conv9.i731 = trunc i32 %add.i730 to i16
  %gep5858.asptr = inttoptr i32 %gep5858 to i16*
  store i16 %conv9.i731, i16* %gep5858.asptr, align 1
  %gep5861.asptr114 = inttoptr i32 %gep5861 to i16*
  %341 = load i16* %gep5861.asptr114, align 1
  %add12.i732 = add i32 %storemerge216.i726, 1
  %gep_array5863 = mul i32 %add12.i732, 2
  %gep5864 = add i32 %fr, %gep_array5863
  %gep5864.asptr = inttoptr i32 %gep5864 to i16*
  store i16 %341, i16* %gep5864.asptr, align 1
  %cmp5.i734 = icmp slt i32 %add12.i732, %storemerge118.i721
  br i1 %cmp5.i734, label %for.body6.i735, label %for.inc14.i738

for.inc14.i738:                                   ; preds = %for.body6.i735, %for.cond4.preheader.i723
  %inc15.i736 = add i32 %storemerge118.i721, 1
  %cmp2.i737 = icmp slt i32 %inc15.i736, %storemerge21.i714
  br i1 %cmp2.i737, label %for.cond4.preheader.i723, label %for.inc17.i741

for.inc17.i741:                                   ; preds = %for.inc14.i738, %for.cond1.preheader.i716
  %inc18.i739 = add i32 %storemerge21.i714, 1
  %cmp.i740 = icmp slt i32 %inc18.i739, %storemerge14502
  br i1 %cmp.i740, label %for.cond1.preheader.i716, label %for.cond1.preheader.i686

for.cond1.preheader.i686:                         ; preds = %for.inc17.i741, %for.inc17.i711
  %storemerge21.i684 = phi i32 [ %inc18.i709, %for.inc17.i711 ], [ 0, %for.inc17.i741 ]
  %cmp217.i685 = icmp sgt i32 %storemerge21.i684, 0
  br i1 %cmp217.i685, label %for.cond4.preheader.lr.ph.i690, label %for.inc17.i711

for.cond4.preheader.lr.ph.i690:                   ; preds = %for.cond1.preheader.i686
  %sub.i687 = add i32 %storemerge21.i684, -2
  %gep_array5866 = mul i32 %sub.i687, 2
  %gep5867 = add i32 %fr, %gep_array5866
  %gep_array5869 = mul i32 %storemerge21.i684, 2
  %gep5870 = add i32 %fr, %gep_array5869
  br label %for.cond4.preheader.i693

for.cond4.preheader.i693:                         ; preds = %for.inc14.i708, %for.cond4.preheader.lr.ph.i690
  %storemerge118.i691 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i690 ], [ %inc15.i706, %for.inc14.i708 ]
  %cmp515.i692 = icmp sgt i32 %storemerge118.i691, 0
  br i1 %cmp515.i692, label %for.body6.lr.ph.i695, label %for.inc14.i708

for.body6.lr.ph.i695:                             ; preds = %for.cond4.preheader.i693
  %gep_array5872 = mul i32 %storemerge118.i691, 2
  %gep5873 = add i32 %fi, %gep_array5872
  br label %for.body6.i705

for.body6.i705:                                   ; preds = %for.body6.i705, %for.body6.lr.ph.i695
  %storemerge216.i696 = phi i32 [ 0, %for.body6.lr.ph.i695 ], [ %add12.i702, %for.body6.i705 ]
  %gep5873.asptr = inttoptr i32 %gep5873 to i16*
  %342 = load i16* %gep5873.asptr, align 1
  %conv.i697 = sext i16 %342 to i32
  %mul.i698 = mul i32 %conv.i697, %storemerge14502
  %gep5867.asptr = inttoptr i32 %gep5867 to i16*
  %343 = load i16* %gep5867.asptr, align 1
  %conv83.i699 = zext i16 %343 to i32
  %add.i700 = add i32 %mul.i698, %conv83.i699
  %conv9.i701 = trunc i32 %add.i700 to i16
  %gep5870.asptr = inttoptr i32 %gep5870 to i16*
  store i16 %conv9.i701, i16* %gep5870.asptr, align 1
  %gep5873.asptr115 = inttoptr i32 %gep5873 to i16*
  %344 = load i16* %gep5873.asptr115, align 1
  %add12.i702 = add i32 %storemerge216.i696, 1
  %gep_array5875 = mul i32 %add12.i702, 2
  %gep5876 = add i32 %fr, %gep_array5875
  %gep5876.asptr = inttoptr i32 %gep5876 to i16*
  store i16 %344, i16* %gep5876.asptr, align 1
  %cmp5.i704 = icmp slt i32 %add12.i702, %storemerge118.i691
  br i1 %cmp5.i704, label %for.body6.i705, label %for.inc14.i708

for.inc14.i708:                                   ; preds = %for.body6.i705, %for.cond4.preheader.i693
  %inc15.i706 = add i32 %storemerge118.i691, 1
  %cmp2.i707 = icmp slt i32 %inc15.i706, %storemerge21.i684
  br i1 %cmp2.i707, label %for.cond4.preheader.i693, label %for.inc17.i711

for.inc17.i711:                                   ; preds = %for.inc14.i708, %for.cond1.preheader.i686
  %inc18.i709 = add i32 %storemerge21.i684, 1
  %cmp.i710 = icmp slt i32 %inc18.i709, %storemerge14502
  br i1 %cmp.i710, label %for.cond1.preheader.i686, label %for.cond1.preheader.i656

for.cond1.preheader.i656:                         ; preds = %for.inc17.i711, %for.inc17.i681
  %storemerge21.i654 = phi i32 [ %inc18.i679, %for.inc17.i681 ], [ 0, %for.inc17.i711 ]
  %cmp217.i655 = icmp sgt i32 %storemerge21.i654, 0
  br i1 %cmp217.i655, label %for.cond4.preheader.lr.ph.i660, label %for.inc17.i681

for.cond4.preheader.lr.ph.i660:                   ; preds = %for.cond1.preheader.i656
  %sub.i657 = add i32 %storemerge21.i654, -2
  %gep_array5878 = mul i32 %sub.i657, 2
  %gep5879 = add i32 %fr, %gep_array5878
  %gep_array5881 = mul i32 %storemerge21.i654, 2
  %gep5882 = add i32 %fr, %gep_array5881
  br label %for.cond4.preheader.i663

for.cond4.preheader.i663:                         ; preds = %for.inc14.i678, %for.cond4.preheader.lr.ph.i660
  %storemerge118.i661 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i660 ], [ %inc15.i676, %for.inc14.i678 ]
  %cmp515.i662 = icmp sgt i32 %storemerge118.i661, 0
  br i1 %cmp515.i662, label %for.body6.lr.ph.i665, label %for.inc14.i678

for.body6.lr.ph.i665:                             ; preds = %for.cond4.preheader.i663
  %gep_array5884 = mul i32 %storemerge118.i661, 2
  %gep5885 = add i32 %fi, %gep_array5884
  br label %for.body6.i675

for.body6.i675:                                   ; preds = %for.body6.i675, %for.body6.lr.ph.i665
  %storemerge216.i666 = phi i32 [ 0, %for.body6.lr.ph.i665 ], [ %add12.i672, %for.body6.i675 ]
  %gep5885.asptr = inttoptr i32 %gep5885 to i16*
  %345 = load i16* %gep5885.asptr, align 1
  %conv.i667 = sext i16 %345 to i32
  %mul.i668 = mul i32 %conv.i667, %storemerge14502
  %gep5879.asptr = inttoptr i32 %gep5879 to i16*
  %346 = load i16* %gep5879.asptr, align 1
  %conv83.i669 = zext i16 %346 to i32
  %add.i670 = add i32 %mul.i668, %conv83.i669
  %conv9.i671 = trunc i32 %add.i670 to i16
  %gep5882.asptr = inttoptr i32 %gep5882 to i16*
  store i16 %conv9.i671, i16* %gep5882.asptr, align 1
  %gep5885.asptr116 = inttoptr i32 %gep5885 to i16*
  %347 = load i16* %gep5885.asptr116, align 1
  %add12.i672 = add i32 %storemerge216.i666, 1
  %gep_array5887 = mul i32 %add12.i672, 2
  %gep5888 = add i32 %fr, %gep_array5887
  %gep5888.asptr = inttoptr i32 %gep5888 to i16*
  store i16 %347, i16* %gep5888.asptr, align 1
  %cmp5.i674 = icmp slt i32 %add12.i672, %storemerge118.i661
  br i1 %cmp5.i674, label %for.body6.i675, label %for.inc14.i678

for.inc14.i678:                                   ; preds = %for.body6.i675, %for.cond4.preheader.i663
  %inc15.i676 = add i32 %storemerge118.i661, 1
  %cmp2.i677 = icmp slt i32 %inc15.i676, %storemerge21.i654
  br i1 %cmp2.i677, label %for.cond4.preheader.i663, label %for.inc17.i681

for.inc17.i681:                                   ; preds = %for.inc14.i678, %for.cond1.preheader.i656
  %inc18.i679 = add i32 %storemerge21.i654, 1
  %cmp.i680 = icmp slt i32 %inc18.i679, %storemerge14502
  br i1 %cmp.i680, label %for.cond1.preheader.i656, label %for.cond1.preheader.i626

for.cond1.preheader.i626:                         ; preds = %for.inc17.i681, %for.inc17.i651
  %storemerge21.i624 = phi i32 [ %inc18.i649, %for.inc17.i651 ], [ 0, %for.inc17.i681 ]
  %cmp217.i625 = icmp sgt i32 %storemerge21.i624, 0
  br i1 %cmp217.i625, label %for.cond4.preheader.lr.ph.i630, label %for.inc17.i651

for.cond4.preheader.lr.ph.i630:                   ; preds = %for.cond1.preheader.i626
  %sub.i627 = add i32 %storemerge21.i624, -2
  %gep_array5890 = mul i32 %sub.i627, 2
  %gep5891 = add i32 %fr, %gep_array5890
  %gep_array5893 = mul i32 %storemerge21.i624, 2
  %gep5894 = add i32 %fr, %gep_array5893
  br label %for.cond4.preheader.i633

for.cond4.preheader.i633:                         ; preds = %for.inc14.i648, %for.cond4.preheader.lr.ph.i630
  %storemerge118.i631 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i630 ], [ %inc15.i646, %for.inc14.i648 ]
  %cmp515.i632 = icmp sgt i32 %storemerge118.i631, 0
  br i1 %cmp515.i632, label %for.body6.lr.ph.i635, label %for.inc14.i648

for.body6.lr.ph.i635:                             ; preds = %for.cond4.preheader.i633
  %gep_array5896 = mul i32 %storemerge118.i631, 2
  %gep5897 = add i32 %Sinewave, %gep_array5896
  br label %for.body6.i645

for.body6.i645:                                   ; preds = %for.body6.i645, %for.body6.lr.ph.i635
  %storemerge216.i636 = phi i32 [ 0, %for.body6.lr.ph.i635 ], [ %add12.i642, %for.body6.i645 ]
  %gep5897.asptr = inttoptr i32 %gep5897 to i16*
  %348 = load i16* %gep5897.asptr, align 1
  %conv.i637 = sext i16 %348 to i32
  %mul.i638 = mul i32 %conv.i637, %storemerge14502
  %gep5891.asptr = inttoptr i32 %gep5891 to i16*
  %349 = load i16* %gep5891.asptr, align 1
  %conv83.i639 = zext i16 %349 to i32
  %add.i640 = add i32 %mul.i638, %conv83.i639
  %conv9.i641 = trunc i32 %add.i640 to i16
  %gep5894.asptr = inttoptr i32 %gep5894 to i16*
  store i16 %conv9.i641, i16* %gep5894.asptr, align 1
  %gep5897.asptr117 = inttoptr i32 %gep5897 to i16*
  %350 = load i16* %gep5897.asptr117, align 1
  %add12.i642 = add i32 %storemerge216.i636, 1
  %gep_array5899 = mul i32 %add12.i642, 2
  %gep5900 = add i32 %fr, %gep_array5899
  %gep5900.asptr = inttoptr i32 %gep5900 to i16*
  store i16 %350, i16* %gep5900.asptr, align 1
  %cmp5.i644 = icmp slt i32 %add12.i642, %storemerge118.i631
  br i1 %cmp5.i644, label %for.body6.i645, label %for.inc14.i648

for.inc14.i648:                                   ; preds = %for.body6.i645, %for.cond4.preheader.i633
  %inc15.i646 = add i32 %storemerge118.i631, 1
  %cmp2.i647 = icmp slt i32 %inc15.i646, %storemerge21.i624
  br i1 %cmp2.i647, label %for.cond4.preheader.i633, label %for.inc17.i651

for.inc17.i651:                                   ; preds = %for.inc14.i648, %for.cond1.preheader.i626
  %inc18.i649 = add i32 %storemerge21.i624, 1
  %cmp.i650 = icmp slt i32 %inc18.i649, %storemerge14502
  br i1 %cmp.i650, label %for.cond1.preheader.i626, label %for.cond1.preheader.i596

for.cond1.preheader.i596:                         ; preds = %for.inc17.i651, %for.inc17.i621
  %storemerge21.i594 = phi i32 [ %inc18.i619, %for.inc17.i621 ], [ 0, %for.inc17.i651 ]
  %cmp217.i595 = icmp sgt i32 %storemerge21.i594, 0
  br i1 %cmp217.i595, label %for.cond4.preheader.lr.ph.i600, label %for.inc17.i621

for.cond4.preheader.lr.ph.i600:                   ; preds = %for.cond1.preheader.i596
  %sub.i597 = add i32 %storemerge21.i594, -2
  %gep_array5902 = mul i32 %sub.i597, 2
  %gep5903 = add i32 %fr, %gep_array5902
  %gep_array5905 = mul i32 %storemerge21.i594, 2
  %gep5906 = add i32 %fr, %gep_array5905
  br label %for.cond4.preheader.i603

for.cond4.preheader.i603:                         ; preds = %for.inc14.i618, %for.cond4.preheader.lr.ph.i600
  %storemerge118.i601 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i600 ], [ %inc15.i616, %for.inc14.i618 ]
  %cmp515.i602 = icmp sgt i32 %storemerge118.i601, 0
  br i1 %cmp515.i602, label %for.body6.lr.ph.i605, label %for.inc14.i618

for.body6.lr.ph.i605:                             ; preds = %for.cond4.preheader.i603
  %gep_array5908 = mul i32 %storemerge118.i601, 2
  %gep5909 = add i32 %fi, %gep_array5908
  br label %for.body6.i615

for.body6.i615:                                   ; preds = %for.body6.i615, %for.body6.lr.ph.i605
  %storemerge216.i606 = phi i32 [ 0, %for.body6.lr.ph.i605 ], [ %add12.i612, %for.body6.i615 ]
  %gep5909.asptr = inttoptr i32 %gep5909 to i16*
  %351 = load i16* %gep5909.asptr, align 1
  %conv.i607 = sext i16 %351 to i32
  %mul.i608 = mul i32 %conv.i607, %storemerge14502
  %gep5903.asptr = inttoptr i32 %gep5903 to i16*
  %352 = load i16* %gep5903.asptr, align 1
  %conv83.i609 = zext i16 %352 to i32
  %add.i610 = add i32 %mul.i608, %conv83.i609
  %conv9.i611 = trunc i32 %add.i610 to i16
  %gep5906.asptr = inttoptr i32 %gep5906 to i16*
  store i16 %conv9.i611, i16* %gep5906.asptr, align 1
  %gep5909.asptr118 = inttoptr i32 %gep5909 to i16*
  %353 = load i16* %gep5909.asptr118, align 1
  %add12.i612 = add i32 %storemerge216.i606, 1
  %gep_array5911 = mul i32 %add12.i612, 2
  %gep5912 = add i32 %fr, %gep_array5911
  %gep5912.asptr = inttoptr i32 %gep5912 to i16*
  store i16 %353, i16* %gep5912.asptr, align 1
  %cmp5.i614 = icmp slt i32 %add12.i612, %storemerge118.i601
  br i1 %cmp5.i614, label %for.body6.i615, label %for.inc14.i618

for.inc14.i618:                                   ; preds = %for.body6.i615, %for.cond4.preheader.i603
  %inc15.i616 = add i32 %storemerge118.i601, 1
  %cmp2.i617 = icmp slt i32 %inc15.i616, %storemerge21.i594
  br i1 %cmp2.i617, label %for.cond4.preheader.i603, label %for.inc17.i621

for.inc17.i621:                                   ; preds = %for.inc14.i618, %for.cond1.preheader.i596
  %inc18.i619 = add i32 %storemerge21.i594, 1
  %cmp.i620 = icmp slt i32 %inc18.i619, %storemerge14502
  br i1 %cmp.i620, label %for.cond1.preheader.i596, label %for.cond1.preheader.i566

for.cond1.preheader.i566:                         ; preds = %for.inc17.i621, %for.inc17.i591
  %storemerge21.i564 = phi i32 [ %inc18.i589, %for.inc17.i591 ], [ 0, %for.inc17.i621 ]
  %cmp217.i565 = icmp sgt i32 %storemerge21.i564, 0
  br i1 %cmp217.i565, label %for.cond4.preheader.lr.ph.i570, label %for.inc17.i591

for.cond4.preheader.lr.ph.i570:                   ; preds = %for.cond1.preheader.i566
  %sub.i567 = add i32 %storemerge21.i564, -2
  %gep_array5914 = mul i32 %sub.i567, 2
  %gep5915 = add i32 %fr, %gep_array5914
  %gep_array5917 = mul i32 %storemerge21.i564, 2
  %gep5918 = add i32 %fr, %gep_array5917
  br label %for.cond4.preheader.i573

for.cond4.preheader.i573:                         ; preds = %for.inc14.i588, %for.cond4.preheader.lr.ph.i570
  %storemerge118.i571 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i570 ], [ %inc15.i586, %for.inc14.i588 ]
  %cmp515.i572 = icmp sgt i32 %storemerge118.i571, 0
  br i1 %cmp515.i572, label %for.body6.lr.ph.i575, label %for.inc14.i588

for.body6.lr.ph.i575:                             ; preds = %for.cond4.preheader.i573
  %gep_array5920 = mul i32 %storemerge118.i571, 2
  %gep5921 = add i32 %fi, %gep_array5920
  br label %for.body6.i585

for.body6.i585:                                   ; preds = %for.body6.i585, %for.body6.lr.ph.i575
  %storemerge216.i576 = phi i32 [ 0, %for.body6.lr.ph.i575 ], [ %add12.i582, %for.body6.i585 ]
  %gep5921.asptr = inttoptr i32 %gep5921 to i16*
  %354 = load i16* %gep5921.asptr, align 1
  %conv.i577 = sext i16 %354 to i32
  %mul.i578 = mul i32 %conv.i577, %storemerge14502
  %gep5915.asptr = inttoptr i32 %gep5915 to i16*
  %355 = load i16* %gep5915.asptr, align 1
  %conv83.i579 = zext i16 %355 to i32
  %add.i580 = add i32 %mul.i578, %conv83.i579
  %conv9.i581 = trunc i32 %add.i580 to i16
  %gep5918.asptr = inttoptr i32 %gep5918 to i16*
  store i16 %conv9.i581, i16* %gep5918.asptr, align 1
  %gep5921.asptr119 = inttoptr i32 %gep5921 to i16*
  %356 = load i16* %gep5921.asptr119, align 1
  %add12.i582 = add i32 %storemerge216.i576, 1
  %gep_array5923 = mul i32 %add12.i582, 2
  %gep5924 = add i32 %fr, %gep_array5923
  %gep5924.asptr = inttoptr i32 %gep5924 to i16*
  store i16 %356, i16* %gep5924.asptr, align 1
  %cmp5.i584 = icmp slt i32 %add12.i582, %storemerge118.i571
  br i1 %cmp5.i584, label %for.body6.i585, label %for.inc14.i588

for.inc14.i588:                                   ; preds = %for.body6.i585, %for.cond4.preheader.i573
  %inc15.i586 = add i32 %storemerge118.i571, 1
  %cmp2.i587 = icmp slt i32 %inc15.i586, %storemerge21.i564
  br i1 %cmp2.i587, label %for.cond4.preheader.i573, label %for.inc17.i591

for.inc17.i591:                                   ; preds = %for.inc14.i588, %for.cond1.preheader.i566
  %inc18.i589 = add i32 %storemerge21.i564, 1
  %cmp.i590 = icmp slt i32 %inc18.i589, %storemerge14502
  br i1 %cmp.i590, label %for.cond1.preheader.i566, label %for.cond1.preheader.i536

for.cond1.preheader.i536:                         ; preds = %for.inc17.i591, %for.inc17.i561
  %storemerge21.i534 = phi i32 [ %inc18.i559, %for.inc17.i561 ], [ 0, %for.inc17.i591 ]
  %cmp217.i535 = icmp sgt i32 %storemerge21.i534, 0
  br i1 %cmp217.i535, label %for.cond4.preheader.lr.ph.i540, label %for.inc17.i561

for.cond4.preheader.lr.ph.i540:                   ; preds = %for.cond1.preheader.i536
  %sub.i537 = add i32 %storemerge21.i534, -2
  %gep_array5926 = mul i32 %sub.i537, 2
  %gep5927 = add i32 %fr, %gep_array5926
  %gep_array5929 = mul i32 %storemerge21.i534, 2
  %gep5930 = add i32 %fr, %gep_array5929
  br label %for.cond4.preheader.i543

for.cond4.preheader.i543:                         ; preds = %for.inc14.i558, %for.cond4.preheader.lr.ph.i540
  %storemerge118.i541 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i540 ], [ %inc15.i556, %for.inc14.i558 ]
  %cmp515.i542 = icmp sgt i32 %storemerge118.i541, 0
  br i1 %cmp515.i542, label %for.body6.lr.ph.i545, label %for.inc14.i558

for.body6.lr.ph.i545:                             ; preds = %for.cond4.preheader.i543
  %gep_array5932 = mul i32 %storemerge118.i541, 2
  %gep5933 = add i32 %fi, %gep_array5932
  br label %for.body6.i555

for.body6.i555:                                   ; preds = %for.body6.i555, %for.body6.lr.ph.i545
  %storemerge216.i546 = phi i32 [ 0, %for.body6.lr.ph.i545 ], [ %add12.i552, %for.body6.i555 ]
  %gep5933.asptr = inttoptr i32 %gep5933 to i16*
  %357 = load i16* %gep5933.asptr, align 1
  %conv.i547 = sext i16 %357 to i32
  %mul.i548 = mul i32 %conv.i547, %storemerge14502
  %gep5927.asptr = inttoptr i32 %gep5927 to i16*
  %358 = load i16* %gep5927.asptr, align 1
  %conv83.i549 = zext i16 %358 to i32
  %add.i550 = add i32 %mul.i548, %conv83.i549
  %conv9.i551 = trunc i32 %add.i550 to i16
  %gep5930.asptr = inttoptr i32 %gep5930 to i16*
  store i16 %conv9.i551, i16* %gep5930.asptr, align 1
  %gep5933.asptr120 = inttoptr i32 %gep5933 to i16*
  %359 = load i16* %gep5933.asptr120, align 1
  %add12.i552 = add i32 %storemerge216.i546, 1
  %gep_array5935 = mul i32 %add12.i552, 2
  %gep5936 = add i32 %fr, %gep_array5935
  %gep5936.asptr = inttoptr i32 %gep5936 to i16*
  store i16 %359, i16* %gep5936.asptr, align 1
  %cmp5.i554 = icmp slt i32 %add12.i552, %storemerge118.i541
  br i1 %cmp5.i554, label %for.body6.i555, label %for.inc14.i558

for.inc14.i558:                                   ; preds = %for.body6.i555, %for.cond4.preheader.i543
  %inc15.i556 = add i32 %storemerge118.i541, 1
  %cmp2.i557 = icmp slt i32 %inc15.i556, %storemerge21.i534
  br i1 %cmp2.i557, label %for.cond4.preheader.i543, label %for.inc17.i561

for.inc17.i561:                                   ; preds = %for.inc14.i558, %for.cond1.preheader.i536
  %inc18.i559 = add i32 %storemerge21.i534, 1
  %cmp.i560 = icmp slt i32 %inc18.i559, %storemerge14502
  br i1 %cmp.i560, label %for.cond1.preheader.i536, label %for.cond1.preheader.i506

for.cond1.preheader.i506:                         ; preds = %for.inc17.i561, %for.inc17.i531
  %storemerge21.i504 = phi i32 [ %inc18.i529, %for.inc17.i531 ], [ 0, %for.inc17.i561 ]
  %cmp217.i505 = icmp sgt i32 %storemerge21.i504, 0
  br i1 %cmp217.i505, label %for.cond4.preheader.lr.ph.i510, label %for.inc17.i531

for.cond4.preheader.lr.ph.i510:                   ; preds = %for.cond1.preheader.i506
  %sub.i507 = add i32 %storemerge21.i504, -2
  %gep_array5938 = mul i32 %sub.i507, 2
  %gep5939 = add i32 %fr, %gep_array5938
  %gep_array5941 = mul i32 %storemerge21.i504, 2
  %gep5942 = add i32 %fr, %gep_array5941
  br label %for.cond4.preheader.i513

for.cond4.preheader.i513:                         ; preds = %for.inc14.i528, %for.cond4.preheader.lr.ph.i510
  %storemerge118.i511 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i510 ], [ %inc15.i526, %for.inc14.i528 ]
  %cmp515.i512 = icmp sgt i32 %storemerge118.i511, 0
  br i1 %cmp515.i512, label %for.body6.lr.ph.i515, label %for.inc14.i528

for.body6.lr.ph.i515:                             ; preds = %for.cond4.preheader.i513
  %gep_array5944 = mul i32 %storemerge118.i511, 2
  %gep5945 = add i32 %Sinewave, %gep_array5944
  br label %for.body6.i525

for.body6.i525:                                   ; preds = %for.body6.i525, %for.body6.lr.ph.i515
  %storemerge216.i516 = phi i32 [ 0, %for.body6.lr.ph.i515 ], [ %add12.i522, %for.body6.i525 ]
  %gep5945.asptr = inttoptr i32 %gep5945 to i16*
  %360 = load i16* %gep5945.asptr, align 1
  %conv.i517 = sext i16 %360 to i32
  %mul.i518 = mul i32 %conv.i517, %storemerge14502
  %gep5939.asptr = inttoptr i32 %gep5939 to i16*
  %361 = load i16* %gep5939.asptr, align 1
  %conv83.i519 = zext i16 %361 to i32
  %add.i520 = add i32 %mul.i518, %conv83.i519
  %conv9.i521 = trunc i32 %add.i520 to i16
  %gep5942.asptr = inttoptr i32 %gep5942 to i16*
  store i16 %conv9.i521, i16* %gep5942.asptr, align 1
  %gep5945.asptr121 = inttoptr i32 %gep5945 to i16*
  %362 = load i16* %gep5945.asptr121, align 1
  %add12.i522 = add i32 %storemerge216.i516, 1
  %gep_array5947 = mul i32 %add12.i522, 2
  %gep5948 = add i32 %fr, %gep_array5947
  %gep5948.asptr = inttoptr i32 %gep5948 to i16*
  store i16 %362, i16* %gep5948.asptr, align 1
  %cmp5.i524 = icmp slt i32 %add12.i522, %storemerge118.i511
  br i1 %cmp5.i524, label %for.body6.i525, label %for.inc14.i528

for.inc14.i528:                                   ; preds = %for.body6.i525, %for.cond4.preheader.i513
  %inc15.i526 = add i32 %storemerge118.i511, 1
  %cmp2.i527 = icmp slt i32 %inc15.i526, %storemerge21.i504
  br i1 %cmp2.i527, label %for.cond4.preheader.i513, label %for.inc17.i531

for.inc17.i531:                                   ; preds = %for.inc14.i528, %for.cond1.preheader.i506
  %inc18.i529 = add i32 %storemerge21.i504, 1
  %cmp.i530 = icmp slt i32 %inc18.i529, %storemerge14502
  br i1 %cmp.i530, label %for.cond1.preheader.i506, label %for.cond1.preheader.i476

for.cond1.preheader.i476:                         ; preds = %for.inc17.i531, %for.inc17.i501
  %storemerge21.i474 = phi i32 [ %inc18.i499, %for.inc17.i501 ], [ 0, %for.inc17.i531 ]
  %cmp217.i475 = icmp sgt i32 %storemerge21.i474, 0
  br i1 %cmp217.i475, label %for.cond4.preheader.lr.ph.i480, label %for.inc17.i501

for.cond4.preheader.lr.ph.i480:                   ; preds = %for.cond1.preheader.i476
  %sub.i477 = add i32 %storemerge21.i474, -2
  %gep_array5950 = mul i32 %sub.i477, 2
  %gep5951 = add i32 %fr, %gep_array5950
  %gep_array5953 = mul i32 %storemerge21.i474, 2
  %gep5954 = add i32 %fr, %gep_array5953
  br label %for.cond4.preheader.i483

for.cond4.preheader.i483:                         ; preds = %for.inc14.i498, %for.cond4.preheader.lr.ph.i480
  %storemerge118.i481 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i480 ], [ %inc15.i496, %for.inc14.i498 ]
  %cmp515.i482 = icmp sgt i32 %storemerge118.i481, 0
  br i1 %cmp515.i482, label %for.body6.lr.ph.i485, label %for.inc14.i498

for.body6.lr.ph.i485:                             ; preds = %for.cond4.preheader.i483
  %gep_array5956 = mul i32 %storemerge118.i481, 2
  %gep5957 = add i32 %Sinewave, %gep_array5956
  br label %for.body6.i495

for.body6.i495:                                   ; preds = %for.body6.i495, %for.body6.lr.ph.i485
  %storemerge216.i486 = phi i32 [ 0, %for.body6.lr.ph.i485 ], [ %add12.i492, %for.body6.i495 ]
  %gep5957.asptr = inttoptr i32 %gep5957 to i16*
  %363 = load i16* %gep5957.asptr, align 1
  %conv.i487 = sext i16 %363 to i32
  %mul.i488 = mul i32 %conv.i487, %storemerge14502
  %gep5951.asptr = inttoptr i32 %gep5951 to i16*
  %364 = load i16* %gep5951.asptr, align 1
  %conv83.i489 = zext i16 %364 to i32
  %add.i490 = add i32 %mul.i488, %conv83.i489
  %conv9.i491 = trunc i32 %add.i490 to i16
  %gep5954.asptr = inttoptr i32 %gep5954 to i16*
  store i16 %conv9.i491, i16* %gep5954.asptr, align 1
  %gep5957.asptr122 = inttoptr i32 %gep5957 to i16*
  %365 = load i16* %gep5957.asptr122, align 1
  %add12.i492 = add i32 %storemerge216.i486, 1
  %gep_array5959 = mul i32 %add12.i492, 2
  %gep5960 = add i32 %fr, %gep_array5959
  %gep5960.asptr = inttoptr i32 %gep5960 to i16*
  store i16 %365, i16* %gep5960.asptr, align 1
  %cmp5.i494 = icmp slt i32 %add12.i492, %storemerge118.i481
  br i1 %cmp5.i494, label %for.body6.i495, label %for.inc14.i498

for.inc14.i498:                                   ; preds = %for.body6.i495, %for.cond4.preheader.i483
  %inc15.i496 = add i32 %storemerge118.i481, 1
  %cmp2.i497 = icmp slt i32 %inc15.i496, %storemerge21.i474
  br i1 %cmp2.i497, label %for.cond4.preheader.i483, label %for.inc17.i501

for.inc17.i501:                                   ; preds = %for.inc14.i498, %for.cond1.preheader.i476
  %inc18.i499 = add i32 %storemerge21.i474, 1
  %cmp.i500 = icmp slt i32 %inc18.i499, %storemerge14502
  br i1 %cmp.i500, label %for.cond1.preheader.i476, label %for.cond1.preheader.i446

for.cond1.preheader.i446:                         ; preds = %for.inc17.i501, %for.inc17.i471
  %storemerge21.i444 = phi i32 [ %inc18.i469, %for.inc17.i471 ], [ 0, %for.inc17.i501 ]
  %cmp217.i445 = icmp sgt i32 %storemerge21.i444, 0
  br i1 %cmp217.i445, label %for.cond4.preheader.lr.ph.i450, label %for.inc17.i471

for.cond4.preheader.lr.ph.i450:                   ; preds = %for.cond1.preheader.i446
  %sub.i447 = add i32 %storemerge21.i444, -2
  %gep_array5962 = mul i32 %sub.i447, 2
  %gep5963 = add i32 %fr, %gep_array5962
  %gep_array5965 = mul i32 %storemerge21.i444, 2
  %gep5966 = add i32 %fr, %gep_array5965
  br label %for.cond4.preheader.i453

for.cond4.preheader.i453:                         ; preds = %for.inc14.i468, %for.cond4.preheader.lr.ph.i450
  %storemerge118.i451 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i450 ], [ %inc15.i466, %for.inc14.i468 ]
  %cmp515.i452 = icmp sgt i32 %storemerge118.i451, 0
  br i1 %cmp515.i452, label %for.body6.lr.ph.i455, label %for.inc14.i468

for.body6.lr.ph.i455:                             ; preds = %for.cond4.preheader.i453
  %gep_array5968 = mul i32 %storemerge118.i451, 2
  %gep5969 = add i32 %Sinewave, %gep_array5968
  br label %for.body6.i465

for.body6.i465:                                   ; preds = %for.body6.i465, %for.body6.lr.ph.i455
  %storemerge216.i456 = phi i32 [ 0, %for.body6.lr.ph.i455 ], [ %add12.i462, %for.body6.i465 ]
  %gep5969.asptr = inttoptr i32 %gep5969 to i16*
  %366 = load i16* %gep5969.asptr, align 1
  %conv.i457 = sext i16 %366 to i32
  %mul.i458 = mul i32 %conv.i457, %storemerge14502
  %gep5963.asptr = inttoptr i32 %gep5963 to i16*
  %367 = load i16* %gep5963.asptr, align 1
  %conv83.i459 = zext i16 %367 to i32
  %add.i460 = add i32 %mul.i458, %conv83.i459
  %conv9.i461 = trunc i32 %add.i460 to i16
  %gep5966.asptr = inttoptr i32 %gep5966 to i16*
  store i16 %conv9.i461, i16* %gep5966.asptr, align 1
  %gep5969.asptr123 = inttoptr i32 %gep5969 to i16*
  %368 = load i16* %gep5969.asptr123, align 1
  %add12.i462 = add i32 %storemerge216.i456, 1
  %gep_array5971 = mul i32 %add12.i462, 2
  %gep5972 = add i32 %fr, %gep_array5971
  %gep5972.asptr = inttoptr i32 %gep5972 to i16*
  store i16 %368, i16* %gep5972.asptr, align 1
  %cmp5.i464 = icmp slt i32 %add12.i462, %storemerge118.i451
  br i1 %cmp5.i464, label %for.body6.i465, label %for.inc14.i468

for.inc14.i468:                                   ; preds = %for.body6.i465, %for.cond4.preheader.i453
  %inc15.i466 = add i32 %storemerge118.i451, 1
  %cmp2.i467 = icmp slt i32 %inc15.i466, %storemerge21.i444
  br i1 %cmp2.i467, label %for.cond4.preheader.i453, label %for.inc17.i471

for.inc17.i471:                                   ; preds = %for.inc14.i468, %for.cond1.preheader.i446
  %inc18.i469 = add i32 %storemerge21.i444, 1
  %cmp.i470 = icmp slt i32 %inc18.i469, %storemerge14502
  br i1 %cmp.i470, label %for.cond1.preheader.i446, label %for.cond1.preheader.i416

for.cond1.preheader.i416:                         ; preds = %for.inc17.i471, %for.inc17.i441
  %storemerge21.i414 = phi i32 [ %inc18.i439, %for.inc17.i441 ], [ 0, %for.inc17.i471 ]
  %cmp217.i415 = icmp sgt i32 %storemerge21.i414, 0
  br i1 %cmp217.i415, label %for.cond4.preheader.lr.ph.i420, label %for.inc17.i441

for.cond4.preheader.lr.ph.i420:                   ; preds = %for.cond1.preheader.i416
  %sub.i417 = add i32 %storemerge21.i414, -2
  %gep_array5974 = mul i32 %sub.i417, 2
  %gep5975 = add i32 %fr, %gep_array5974
  %gep_array5977 = mul i32 %storemerge21.i414, 2
  %gep5978 = add i32 %fr, %gep_array5977
  br label %for.cond4.preheader.i423

for.cond4.preheader.i423:                         ; preds = %for.inc14.i438, %for.cond4.preheader.lr.ph.i420
  %storemerge118.i421 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i420 ], [ %inc15.i436, %for.inc14.i438 ]
  %cmp515.i422 = icmp sgt i32 %storemerge118.i421, 0
  br i1 %cmp515.i422, label %for.body6.lr.ph.i425, label %for.inc14.i438

for.body6.lr.ph.i425:                             ; preds = %for.cond4.preheader.i423
  %gep_array5980 = mul i32 %storemerge118.i421, 2
  %gep5981 = add i32 %Sinewave, %gep_array5980
  br label %for.body6.i435

for.body6.i435:                                   ; preds = %for.body6.i435, %for.body6.lr.ph.i425
  %storemerge216.i426 = phi i32 [ 0, %for.body6.lr.ph.i425 ], [ %add12.i432, %for.body6.i435 ]
  %gep5981.asptr = inttoptr i32 %gep5981 to i16*
  %369 = load i16* %gep5981.asptr, align 1
  %conv.i427 = sext i16 %369 to i32
  %mul.i428 = mul i32 %conv.i427, %storemerge14502
  %gep5975.asptr = inttoptr i32 %gep5975 to i16*
  %370 = load i16* %gep5975.asptr, align 1
  %conv83.i429 = zext i16 %370 to i32
  %add.i430 = add i32 %mul.i428, %conv83.i429
  %conv9.i431 = trunc i32 %add.i430 to i16
  %gep5978.asptr = inttoptr i32 %gep5978 to i16*
  store i16 %conv9.i431, i16* %gep5978.asptr, align 1
  %gep5981.asptr124 = inttoptr i32 %gep5981 to i16*
  %371 = load i16* %gep5981.asptr124, align 1
  %add12.i432 = add i32 %storemerge216.i426, 1
  %gep_array5983 = mul i32 %add12.i432, 2
  %gep5984 = add i32 %fr, %gep_array5983
  %gep5984.asptr = inttoptr i32 %gep5984 to i16*
  store i16 %371, i16* %gep5984.asptr, align 1
  %cmp5.i434 = icmp slt i32 %add12.i432, %storemerge118.i421
  br i1 %cmp5.i434, label %for.body6.i435, label %for.inc14.i438

for.inc14.i438:                                   ; preds = %for.body6.i435, %for.cond4.preheader.i423
  %inc15.i436 = add i32 %storemerge118.i421, 1
  %cmp2.i437 = icmp slt i32 %inc15.i436, %storemerge21.i414
  br i1 %cmp2.i437, label %for.cond4.preheader.i423, label %for.inc17.i441

for.inc17.i441:                                   ; preds = %for.inc14.i438, %for.cond1.preheader.i416
  %inc18.i439 = add i32 %storemerge21.i414, 1
  %cmp.i440 = icmp slt i32 %inc18.i439, %storemerge14502
  br i1 %cmp.i440, label %for.cond1.preheader.i416, label %for.cond1.preheader.i386

for.cond1.preheader.i386:                         ; preds = %for.inc17.i441, %for.inc17.i411
  %storemerge21.i384 = phi i32 [ %inc18.i409, %for.inc17.i411 ], [ 0, %for.inc17.i441 ]
  %cmp217.i385 = icmp sgt i32 %storemerge21.i384, 0
  br i1 %cmp217.i385, label %for.cond4.preheader.lr.ph.i390, label %for.inc17.i411

for.cond4.preheader.lr.ph.i390:                   ; preds = %for.cond1.preheader.i386
  %sub.i387 = add i32 %storemerge21.i384, -2
  %gep_array5986 = mul i32 %sub.i387, 2
  %gep5987 = add i32 %fr, %gep_array5986
  %gep_array5989 = mul i32 %storemerge21.i384, 2
  %gep5990 = add i32 %fr, %gep_array5989
  br label %for.cond4.preheader.i393

for.cond4.preheader.i393:                         ; preds = %for.inc14.i408, %for.cond4.preheader.lr.ph.i390
  %storemerge118.i391 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i390 ], [ %inc15.i406, %for.inc14.i408 ]
  %cmp515.i392 = icmp sgt i32 %storemerge118.i391, 0
  br i1 %cmp515.i392, label %for.body6.lr.ph.i395, label %for.inc14.i408

for.body6.lr.ph.i395:                             ; preds = %for.cond4.preheader.i393
  %gep_array5992 = mul i32 %storemerge118.i391, 2
  %gep5993 = add i32 %fi, %gep_array5992
  br label %for.body6.i405

for.body6.i405:                                   ; preds = %for.body6.i405, %for.body6.lr.ph.i395
  %storemerge216.i396 = phi i32 [ 0, %for.body6.lr.ph.i395 ], [ %add12.i402, %for.body6.i405 ]
  %gep5993.asptr = inttoptr i32 %gep5993 to i16*
  %372 = load i16* %gep5993.asptr, align 1
  %conv.i397 = sext i16 %372 to i32
  %mul.i398 = mul i32 %conv.i397, %storemerge14502
  %gep5987.asptr = inttoptr i32 %gep5987 to i16*
  %373 = load i16* %gep5987.asptr, align 1
  %conv83.i399 = zext i16 %373 to i32
  %add.i400 = add i32 %mul.i398, %conv83.i399
  %conv9.i401 = trunc i32 %add.i400 to i16
  %gep5990.asptr = inttoptr i32 %gep5990 to i16*
  store i16 %conv9.i401, i16* %gep5990.asptr, align 1
  %gep5993.asptr125 = inttoptr i32 %gep5993 to i16*
  %374 = load i16* %gep5993.asptr125, align 1
  %add12.i402 = add i32 %storemerge216.i396, 1
  %gep_array5995 = mul i32 %add12.i402, 2
  %gep5996 = add i32 %fr, %gep_array5995
  %gep5996.asptr = inttoptr i32 %gep5996 to i16*
  store i16 %374, i16* %gep5996.asptr, align 1
  %cmp5.i404 = icmp slt i32 %add12.i402, %storemerge118.i391
  br i1 %cmp5.i404, label %for.body6.i405, label %for.inc14.i408

for.inc14.i408:                                   ; preds = %for.body6.i405, %for.cond4.preheader.i393
  %inc15.i406 = add i32 %storemerge118.i391, 1
  %cmp2.i407 = icmp slt i32 %inc15.i406, %storemerge21.i384
  br i1 %cmp2.i407, label %for.cond4.preheader.i393, label %for.inc17.i411

for.inc17.i411:                                   ; preds = %for.inc14.i408, %for.cond1.preheader.i386
  %inc18.i409 = add i32 %storemerge21.i384, 1
  %cmp.i410 = icmp slt i32 %inc18.i409, %storemerge14502
  br i1 %cmp.i410, label %for.cond1.preheader.i386, label %for.cond1.preheader.i356

for.cond1.preheader.i356:                         ; preds = %for.inc17.i411, %for.inc17.i381
  %storemerge21.i354 = phi i32 [ %inc18.i379, %for.inc17.i381 ], [ 0, %for.inc17.i411 ]
  %cmp217.i355 = icmp sgt i32 %storemerge21.i354, 0
  br i1 %cmp217.i355, label %for.cond4.preheader.lr.ph.i360, label %for.inc17.i381

for.cond4.preheader.lr.ph.i360:                   ; preds = %for.cond1.preheader.i356
  %sub.i357 = add i32 %storemerge21.i354, -2
  %gep_array5998 = mul i32 %sub.i357, 2
  %gep5999 = add i32 %fr, %gep_array5998
  %gep_array6001 = mul i32 %storemerge21.i354, 2
  %gep6002 = add i32 %fr, %gep_array6001
  br label %for.cond4.preheader.i363

for.cond4.preheader.i363:                         ; preds = %for.inc14.i378, %for.cond4.preheader.lr.ph.i360
  %storemerge118.i361 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i360 ], [ %inc15.i376, %for.inc14.i378 ]
  %cmp515.i362 = icmp sgt i32 %storemerge118.i361, 0
  br i1 %cmp515.i362, label %for.body6.lr.ph.i365, label %for.inc14.i378

for.body6.lr.ph.i365:                             ; preds = %for.cond4.preheader.i363
  %gep_array6004 = mul i32 %storemerge118.i361, 2
  %gep6005 = add i32 %fi, %gep_array6004
  br label %for.body6.i375

for.body6.i375:                                   ; preds = %for.body6.i375, %for.body6.lr.ph.i365
  %storemerge216.i366 = phi i32 [ 0, %for.body6.lr.ph.i365 ], [ %add12.i372, %for.body6.i375 ]
  %gep6005.asptr = inttoptr i32 %gep6005 to i16*
  %375 = load i16* %gep6005.asptr, align 1
  %conv.i367 = sext i16 %375 to i32
  %mul.i368 = mul i32 %conv.i367, %storemerge14502
  %gep5999.asptr = inttoptr i32 %gep5999 to i16*
  %376 = load i16* %gep5999.asptr, align 1
  %conv83.i369 = zext i16 %376 to i32
  %add.i370 = add i32 %mul.i368, %conv83.i369
  %conv9.i371 = trunc i32 %add.i370 to i16
  %gep6002.asptr = inttoptr i32 %gep6002 to i16*
  store i16 %conv9.i371, i16* %gep6002.asptr, align 1
  %gep6005.asptr126 = inttoptr i32 %gep6005 to i16*
  %377 = load i16* %gep6005.asptr126, align 1
  %add12.i372 = add i32 %storemerge216.i366, 1
  %gep_array6007 = mul i32 %add12.i372, 2
  %gep6008 = add i32 %fr, %gep_array6007
  %gep6008.asptr = inttoptr i32 %gep6008 to i16*
  store i16 %377, i16* %gep6008.asptr, align 1
  %cmp5.i374 = icmp slt i32 %add12.i372, %storemerge118.i361
  br i1 %cmp5.i374, label %for.body6.i375, label %for.inc14.i378

for.inc14.i378:                                   ; preds = %for.body6.i375, %for.cond4.preheader.i363
  %inc15.i376 = add i32 %storemerge118.i361, 1
  %cmp2.i377 = icmp slt i32 %inc15.i376, %storemerge21.i354
  br i1 %cmp2.i377, label %for.cond4.preheader.i363, label %for.inc17.i381

for.inc17.i381:                                   ; preds = %for.inc14.i378, %for.cond1.preheader.i356
  %inc18.i379 = add i32 %storemerge21.i354, 1
  %cmp.i380 = icmp slt i32 %inc18.i379, %storemerge14502
  br i1 %cmp.i380, label %for.cond1.preheader.i356, label %for.cond1.preheader.i326

for.cond1.preheader.i326:                         ; preds = %for.inc17.i381, %for.inc17.i351
  %storemerge21.i324 = phi i32 [ %inc18.i349, %for.inc17.i351 ], [ 0, %for.inc17.i381 ]
  %cmp217.i325 = icmp sgt i32 %storemerge21.i324, 0
  br i1 %cmp217.i325, label %for.cond4.preheader.lr.ph.i330, label %for.inc17.i351

for.cond4.preheader.lr.ph.i330:                   ; preds = %for.cond1.preheader.i326
  %sub.i327 = add i32 %storemerge21.i324, -2
  %gep_array6010 = mul i32 %sub.i327, 2
  %gep6011 = add i32 %fr, %gep_array6010
  %gep_array6013 = mul i32 %storemerge21.i324, 2
  %gep6014 = add i32 %fr, %gep_array6013
  br label %for.cond4.preheader.i333

for.cond4.preheader.i333:                         ; preds = %for.inc14.i348, %for.cond4.preheader.lr.ph.i330
  %storemerge118.i331 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i330 ], [ %inc15.i346, %for.inc14.i348 ]
  %cmp515.i332 = icmp sgt i32 %storemerge118.i331, 0
  br i1 %cmp515.i332, label %for.body6.lr.ph.i335, label %for.inc14.i348

for.body6.lr.ph.i335:                             ; preds = %for.cond4.preheader.i333
  %gep_array6016 = mul i32 %storemerge118.i331, 2
  %gep6017 = add i32 %fi, %gep_array6016
  br label %for.body6.i345

for.body6.i345:                                   ; preds = %for.body6.i345, %for.body6.lr.ph.i335
  %storemerge216.i336 = phi i32 [ 0, %for.body6.lr.ph.i335 ], [ %add12.i342, %for.body6.i345 ]
  %gep6017.asptr = inttoptr i32 %gep6017 to i16*
  %378 = load i16* %gep6017.asptr, align 1
  %conv.i337 = sext i16 %378 to i32
  %mul.i338 = mul i32 %conv.i337, %storemerge14502
  %gep6011.asptr = inttoptr i32 %gep6011 to i16*
  %379 = load i16* %gep6011.asptr, align 1
  %conv83.i339 = zext i16 %379 to i32
  %add.i340 = add i32 %mul.i338, %conv83.i339
  %conv9.i341 = trunc i32 %add.i340 to i16
  %gep6014.asptr = inttoptr i32 %gep6014 to i16*
  store i16 %conv9.i341, i16* %gep6014.asptr, align 1
  %gep6017.asptr127 = inttoptr i32 %gep6017 to i16*
  %380 = load i16* %gep6017.asptr127, align 1
  %add12.i342 = add i32 %storemerge216.i336, 1
  %gep_array6019 = mul i32 %add12.i342, 2
  %gep6020 = add i32 %fr, %gep_array6019
  %gep6020.asptr = inttoptr i32 %gep6020 to i16*
  store i16 %380, i16* %gep6020.asptr, align 1
  %cmp5.i344 = icmp slt i32 %add12.i342, %storemerge118.i331
  br i1 %cmp5.i344, label %for.body6.i345, label %for.inc14.i348

for.inc14.i348:                                   ; preds = %for.body6.i345, %for.cond4.preheader.i333
  %inc15.i346 = add i32 %storemerge118.i331, 1
  %cmp2.i347 = icmp slt i32 %inc15.i346, %storemerge21.i324
  br i1 %cmp2.i347, label %for.cond4.preheader.i333, label %for.inc17.i351

for.inc17.i351:                                   ; preds = %for.inc14.i348, %for.cond1.preheader.i326
  %inc18.i349 = add i32 %storemerge21.i324, 1
  %cmp.i350 = icmp slt i32 %inc18.i349, %storemerge14502
  br i1 %cmp.i350, label %for.cond1.preheader.i326, label %for.cond1.preheader.i296

for.cond1.preheader.i296:                         ; preds = %for.inc17.i351, %for.inc17.i321
  %storemerge21.i294 = phi i32 [ %inc18.i319, %for.inc17.i321 ], [ 0, %for.inc17.i351 ]
  %cmp217.i295 = icmp sgt i32 %storemerge21.i294, 0
  br i1 %cmp217.i295, label %for.cond4.preheader.lr.ph.i300, label %for.inc17.i321

for.cond4.preheader.lr.ph.i300:                   ; preds = %for.cond1.preheader.i296
  %sub.i297 = add i32 %storemerge21.i294, -2
  %gep_array6022 = mul i32 %sub.i297, 2
  %gep6023 = add i32 %fr, %gep_array6022
  %gep_array6025 = mul i32 %storemerge21.i294, 2
  %gep6026 = add i32 %fr, %gep_array6025
  br label %for.cond4.preheader.i303

for.cond4.preheader.i303:                         ; preds = %for.inc14.i318, %for.cond4.preheader.lr.ph.i300
  %storemerge118.i301 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i300 ], [ %inc15.i316, %for.inc14.i318 ]
  %cmp515.i302 = icmp sgt i32 %storemerge118.i301, 0
  br i1 %cmp515.i302, label %for.body6.lr.ph.i305, label %for.inc14.i318

for.body6.lr.ph.i305:                             ; preds = %for.cond4.preheader.i303
  %gep_array6028 = mul i32 %storemerge118.i301, 2
  %gep6029 = add i32 %Sinewave, %gep_array6028
  br label %for.body6.i315

for.body6.i315:                                   ; preds = %for.body6.i315, %for.body6.lr.ph.i305
  %storemerge216.i306 = phi i32 [ 0, %for.body6.lr.ph.i305 ], [ %add12.i312, %for.body6.i315 ]
  %gep6029.asptr = inttoptr i32 %gep6029 to i16*
  %381 = load i16* %gep6029.asptr, align 1
  %conv.i307 = sext i16 %381 to i32
  %mul.i308 = mul i32 %conv.i307, %storemerge14502
  %gep6023.asptr = inttoptr i32 %gep6023 to i16*
  %382 = load i16* %gep6023.asptr, align 1
  %conv83.i309 = zext i16 %382 to i32
  %add.i310 = add i32 %mul.i308, %conv83.i309
  %conv9.i311 = trunc i32 %add.i310 to i16
  %gep6026.asptr = inttoptr i32 %gep6026 to i16*
  store i16 %conv9.i311, i16* %gep6026.asptr, align 1
  %gep6029.asptr128 = inttoptr i32 %gep6029 to i16*
  %383 = load i16* %gep6029.asptr128, align 1
  %add12.i312 = add i32 %storemerge216.i306, 1
  %gep_array6031 = mul i32 %add12.i312, 2
  %gep6032 = add i32 %fr, %gep_array6031
  %gep6032.asptr = inttoptr i32 %gep6032 to i16*
  store i16 %383, i16* %gep6032.asptr, align 1
  %cmp5.i314 = icmp slt i32 %add12.i312, %storemerge118.i301
  br i1 %cmp5.i314, label %for.body6.i315, label %for.inc14.i318

for.inc14.i318:                                   ; preds = %for.body6.i315, %for.cond4.preheader.i303
  %inc15.i316 = add i32 %storemerge118.i301, 1
  %cmp2.i317 = icmp slt i32 %inc15.i316, %storemerge21.i294
  br i1 %cmp2.i317, label %for.cond4.preheader.i303, label %for.inc17.i321

for.inc17.i321:                                   ; preds = %for.inc14.i318, %for.cond1.preheader.i296
  %inc18.i319 = add i32 %storemerge21.i294, 1
  %cmp.i320 = icmp slt i32 %inc18.i319, %storemerge14502
  br i1 %cmp.i320, label %for.cond1.preheader.i296, label %for.cond1.preheader.i266

for.cond1.preheader.i266:                         ; preds = %for.inc17.i321, %for.inc17.i291
  %storemerge21.i264 = phi i32 [ %inc18.i289, %for.inc17.i291 ], [ 0, %for.inc17.i321 ]
  %cmp217.i265 = icmp sgt i32 %storemerge21.i264, 0
  br i1 %cmp217.i265, label %for.cond4.preheader.lr.ph.i270, label %for.inc17.i291

for.cond4.preheader.lr.ph.i270:                   ; preds = %for.cond1.preheader.i266
  %sub.i267 = add i32 %storemerge21.i264, -2
  %gep_array6034 = mul i32 %sub.i267, 2
  %gep6035 = add i32 %fr, %gep_array6034
  %gep_array6037 = mul i32 %storemerge21.i264, 2
  %gep6038 = add i32 %fr, %gep_array6037
  br label %for.cond4.preheader.i273

for.cond4.preheader.i273:                         ; preds = %for.inc14.i288, %for.cond4.preheader.lr.ph.i270
  %storemerge118.i271 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i270 ], [ %inc15.i286, %for.inc14.i288 ]
  %cmp515.i272 = icmp sgt i32 %storemerge118.i271, 0
  br i1 %cmp515.i272, label %for.body6.lr.ph.i275, label %for.inc14.i288

for.body6.lr.ph.i275:                             ; preds = %for.cond4.preheader.i273
  %gep_array6040 = mul i32 %storemerge118.i271, 2
  %gep6041 = add i32 %Sinewave, %gep_array6040
  br label %for.body6.i285

for.body6.i285:                                   ; preds = %for.body6.i285, %for.body6.lr.ph.i275
  %storemerge216.i276 = phi i32 [ 0, %for.body6.lr.ph.i275 ], [ %add12.i282, %for.body6.i285 ]
  %gep6041.asptr = inttoptr i32 %gep6041 to i16*
  %384 = load i16* %gep6041.asptr, align 1
  %conv.i277 = sext i16 %384 to i32
  %mul.i278 = mul i32 %conv.i277, %storemerge14502
  %gep6035.asptr = inttoptr i32 %gep6035 to i16*
  %385 = load i16* %gep6035.asptr, align 1
  %conv83.i279 = zext i16 %385 to i32
  %add.i280 = add i32 %mul.i278, %conv83.i279
  %conv9.i281 = trunc i32 %add.i280 to i16
  %gep6038.asptr = inttoptr i32 %gep6038 to i16*
  store i16 %conv9.i281, i16* %gep6038.asptr, align 1
  %gep6041.asptr129 = inttoptr i32 %gep6041 to i16*
  %386 = load i16* %gep6041.asptr129, align 1
  %add12.i282 = add i32 %storemerge216.i276, 1
  %gep_array6043 = mul i32 %add12.i282, 2
  %gep6044 = add i32 %fr, %gep_array6043
  %gep6044.asptr = inttoptr i32 %gep6044 to i16*
  store i16 %386, i16* %gep6044.asptr, align 1
  %cmp5.i284 = icmp slt i32 %add12.i282, %storemerge118.i271
  br i1 %cmp5.i284, label %for.body6.i285, label %for.inc14.i288

for.inc14.i288:                                   ; preds = %for.body6.i285, %for.cond4.preheader.i273
  %inc15.i286 = add i32 %storemerge118.i271, 1
  %cmp2.i287 = icmp slt i32 %inc15.i286, %storemerge21.i264
  br i1 %cmp2.i287, label %for.cond4.preheader.i273, label %for.inc17.i291

for.inc17.i291:                                   ; preds = %for.inc14.i288, %for.cond1.preheader.i266
  %inc18.i289 = add i32 %storemerge21.i264, 1
  %cmp.i290 = icmp slt i32 %inc18.i289, %storemerge14502
  br i1 %cmp.i290, label %for.cond1.preheader.i266, label %for.cond1.preheader.i236

for.cond1.preheader.i236:                         ; preds = %for.inc17.i291, %for.inc17.i261
  %storemerge21.i234 = phi i32 [ %inc18.i259, %for.inc17.i261 ], [ 0, %for.inc17.i291 ]
  %cmp217.i235 = icmp sgt i32 %storemerge21.i234, 0
  br i1 %cmp217.i235, label %for.cond4.preheader.lr.ph.i240, label %for.inc17.i261

for.cond4.preheader.lr.ph.i240:                   ; preds = %for.cond1.preheader.i236
  %sub.i237 = add i32 %storemerge21.i234, -2
  %gep_array6046 = mul i32 %sub.i237, 2
  %gep6047 = add i32 %fr, %gep_array6046
  %gep_array6049 = mul i32 %storemerge21.i234, 2
  %gep6050 = add i32 %fr, %gep_array6049
  br label %for.cond4.preheader.i243

for.cond4.preheader.i243:                         ; preds = %for.inc14.i258, %for.cond4.preheader.lr.ph.i240
  %storemerge118.i241 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i240 ], [ %inc15.i256, %for.inc14.i258 ]
  %cmp515.i242 = icmp sgt i32 %storemerge118.i241, 0
  br i1 %cmp515.i242, label %for.body6.lr.ph.i245, label %for.inc14.i258

for.body6.lr.ph.i245:                             ; preds = %for.cond4.preheader.i243
  %gep_array6052 = mul i32 %storemerge118.i241, 2
  %gep6053 = add i32 %Sinewave, %gep_array6052
  br label %for.body6.i255

for.body6.i255:                                   ; preds = %for.body6.i255, %for.body6.lr.ph.i245
  %storemerge216.i246 = phi i32 [ 0, %for.body6.lr.ph.i245 ], [ %add12.i252, %for.body6.i255 ]
  %gep6053.asptr = inttoptr i32 %gep6053 to i16*
  %387 = load i16* %gep6053.asptr, align 1
  %conv.i247 = sext i16 %387 to i32
  %mul.i248 = mul i32 %conv.i247, %storemerge14502
  %gep6047.asptr = inttoptr i32 %gep6047 to i16*
  %388 = load i16* %gep6047.asptr, align 1
  %conv83.i249 = zext i16 %388 to i32
  %add.i250 = add i32 %mul.i248, %conv83.i249
  %conv9.i251 = trunc i32 %add.i250 to i16
  %gep6050.asptr = inttoptr i32 %gep6050 to i16*
  store i16 %conv9.i251, i16* %gep6050.asptr, align 1
  %gep6053.asptr130 = inttoptr i32 %gep6053 to i16*
  %389 = load i16* %gep6053.asptr130, align 1
  %add12.i252 = add i32 %storemerge216.i246, 1
  %gep_array6055 = mul i32 %add12.i252, 2
  %gep6056 = add i32 %fr, %gep_array6055
  %gep6056.asptr = inttoptr i32 %gep6056 to i16*
  store i16 %389, i16* %gep6056.asptr, align 1
  %cmp5.i254 = icmp slt i32 %add12.i252, %storemerge118.i241
  br i1 %cmp5.i254, label %for.body6.i255, label %for.inc14.i258

for.inc14.i258:                                   ; preds = %for.body6.i255, %for.cond4.preheader.i243
  %inc15.i256 = add i32 %storemerge118.i241, 1
  %cmp2.i257 = icmp slt i32 %inc15.i256, %storemerge21.i234
  br i1 %cmp2.i257, label %for.cond4.preheader.i243, label %for.inc17.i261

for.inc17.i261:                                   ; preds = %for.inc14.i258, %for.cond1.preheader.i236
  %inc18.i259 = add i32 %storemerge21.i234, 1
  %cmp.i260 = icmp slt i32 %inc18.i259, %storemerge14502
  br i1 %cmp.i260, label %for.cond1.preheader.i236, label %for.cond1.preheader.i206

for.cond1.preheader.i206:                         ; preds = %for.inc17.i261, %for.inc17.i231
  %storemerge21.i204 = phi i32 [ %inc18.i229, %for.inc17.i231 ], [ 0, %for.inc17.i261 ]
  %cmp217.i205 = icmp sgt i32 %storemerge21.i204, 0
  br i1 %cmp217.i205, label %for.cond4.preheader.lr.ph.i210, label %for.inc17.i231

for.cond4.preheader.lr.ph.i210:                   ; preds = %for.cond1.preheader.i206
  %sub.i207 = add i32 %storemerge21.i204, -2
  %gep_array6058 = mul i32 %sub.i207, 2
  %gep6059 = add i32 %fr, %gep_array6058
  %gep_array6061 = mul i32 %storemerge21.i204, 2
  %gep6062 = add i32 %fr, %gep_array6061
  br label %for.cond4.preheader.i213

for.cond4.preheader.i213:                         ; preds = %for.inc14.i228, %for.cond4.preheader.lr.ph.i210
  %storemerge118.i211 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i210 ], [ %inc15.i226, %for.inc14.i228 ]
  %cmp515.i212 = icmp sgt i32 %storemerge118.i211, 0
  br i1 %cmp515.i212, label %for.body6.lr.ph.i215, label %for.inc14.i228

for.body6.lr.ph.i215:                             ; preds = %for.cond4.preheader.i213
  %gep_array6064 = mul i32 %storemerge118.i211, 2
  %gep6065 = add i32 %Sinewave, %gep_array6064
  br label %for.body6.i225

for.body6.i225:                                   ; preds = %for.body6.i225, %for.body6.lr.ph.i215
  %storemerge216.i216 = phi i32 [ 0, %for.body6.lr.ph.i215 ], [ %add12.i222, %for.body6.i225 ]
  %gep6065.asptr = inttoptr i32 %gep6065 to i16*
  %390 = load i16* %gep6065.asptr, align 1
  %conv.i217 = sext i16 %390 to i32
  %mul.i218 = mul i32 %conv.i217, %storemerge14502
  %gep6059.asptr = inttoptr i32 %gep6059 to i16*
  %391 = load i16* %gep6059.asptr, align 1
  %conv83.i219 = zext i16 %391 to i32
  %add.i220 = add i32 %mul.i218, %conv83.i219
  %conv9.i221 = trunc i32 %add.i220 to i16
  %gep6062.asptr = inttoptr i32 %gep6062 to i16*
  store i16 %conv9.i221, i16* %gep6062.asptr, align 1
  %gep6065.asptr131 = inttoptr i32 %gep6065 to i16*
  %392 = load i16* %gep6065.asptr131, align 1
  %add12.i222 = add i32 %storemerge216.i216, 1
  %gep_array6067 = mul i32 %add12.i222, 2
  %gep6068 = add i32 %fr, %gep_array6067
  %gep6068.asptr = inttoptr i32 %gep6068 to i16*
  store i16 %392, i16* %gep6068.asptr, align 1
  %cmp5.i224 = icmp slt i32 %add12.i222, %storemerge118.i211
  br i1 %cmp5.i224, label %for.body6.i225, label %for.inc14.i228

for.inc14.i228:                                   ; preds = %for.body6.i225, %for.cond4.preheader.i213
  %inc15.i226 = add i32 %storemerge118.i211, 1
  %cmp2.i227 = icmp slt i32 %inc15.i226, %storemerge21.i204
  br i1 %cmp2.i227, label %for.cond4.preheader.i213, label %for.inc17.i231

for.inc17.i231:                                   ; preds = %for.inc14.i228, %for.cond1.preheader.i206
  %inc18.i229 = add i32 %storemerge21.i204, 1
  %cmp.i230 = icmp slt i32 %inc18.i229, %storemerge14502
  br i1 %cmp.i230, label %for.cond1.preheader.i206, label %for.cond1.preheader.i176

for.cond1.preheader.i176:                         ; preds = %for.inc17.i231, %for.inc17.i201
  %storemerge21.i174 = phi i32 [ %inc18.i199, %for.inc17.i201 ], [ 0, %for.inc17.i231 ]
  %cmp217.i175 = icmp sgt i32 %storemerge21.i174, 0
  br i1 %cmp217.i175, label %for.cond4.preheader.lr.ph.i180, label %for.inc17.i201

for.cond4.preheader.lr.ph.i180:                   ; preds = %for.cond1.preheader.i176
  %sub.i177 = add i32 %storemerge21.i174, -2
  %gep_array6070 = mul i32 %sub.i177, 2
  %gep6071 = add i32 %fr, %gep_array6070
  %gep_array6073 = mul i32 %storemerge21.i174, 2
  %gep6074 = add i32 %fr, %gep_array6073
  br label %for.cond4.preheader.i183

for.cond4.preheader.i183:                         ; preds = %for.inc14.i198, %for.cond4.preheader.lr.ph.i180
  %storemerge118.i181 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i180 ], [ %inc15.i196, %for.inc14.i198 ]
  %cmp515.i182 = icmp sgt i32 %storemerge118.i181, 0
  br i1 %cmp515.i182, label %for.body6.lr.ph.i185, label %for.inc14.i198

for.body6.lr.ph.i185:                             ; preds = %for.cond4.preheader.i183
  %gep_array6076 = mul i32 %storemerge118.i181, 2
  %gep6077 = add i32 %fi, %gep_array6076
  br label %for.body6.i195

for.body6.i195:                                   ; preds = %for.body6.i195, %for.body6.lr.ph.i185
  %storemerge216.i186 = phi i32 [ 0, %for.body6.lr.ph.i185 ], [ %add12.i192, %for.body6.i195 ]
  %gep6077.asptr = inttoptr i32 %gep6077 to i16*
  %393 = load i16* %gep6077.asptr, align 1
  %conv.i187 = sext i16 %393 to i32
  %mul.i188 = mul i32 %conv.i187, %storemerge14502
  %gep6071.asptr = inttoptr i32 %gep6071 to i16*
  %394 = load i16* %gep6071.asptr, align 1
  %conv83.i189 = zext i16 %394 to i32
  %add.i190 = add i32 %mul.i188, %conv83.i189
  %conv9.i191 = trunc i32 %add.i190 to i16
  %gep6074.asptr = inttoptr i32 %gep6074 to i16*
  store i16 %conv9.i191, i16* %gep6074.asptr, align 1
  %gep6077.asptr132 = inttoptr i32 %gep6077 to i16*
  %395 = load i16* %gep6077.asptr132, align 1
  %add12.i192 = add i32 %storemerge216.i186, 1
  %gep_array6079 = mul i32 %add12.i192, 2
  %gep6080 = add i32 %fr, %gep_array6079
  %gep6080.asptr = inttoptr i32 %gep6080 to i16*
  store i16 %395, i16* %gep6080.asptr, align 1
  %cmp5.i194 = icmp slt i32 %add12.i192, %storemerge118.i181
  br i1 %cmp5.i194, label %for.body6.i195, label %for.inc14.i198

for.inc14.i198:                                   ; preds = %for.body6.i195, %for.cond4.preheader.i183
  %inc15.i196 = add i32 %storemerge118.i181, 1
  %cmp2.i197 = icmp slt i32 %inc15.i196, %storemerge21.i174
  br i1 %cmp2.i197, label %for.cond4.preheader.i183, label %for.inc17.i201

for.inc17.i201:                                   ; preds = %for.inc14.i198, %for.cond1.preheader.i176
  %inc18.i199 = add i32 %storemerge21.i174, 1
  %cmp.i200 = icmp slt i32 %inc18.i199, %storemerge14502
  br i1 %cmp.i200, label %for.cond1.preheader.i176, label %for.cond1.preheader.i146

for.cond1.preheader.i146:                         ; preds = %for.inc17.i201, %for.inc17.i171
  %storemerge21.i144 = phi i32 [ %inc18.i169, %for.inc17.i171 ], [ 0, %for.inc17.i201 ]
  %cmp217.i145 = icmp sgt i32 %storemerge21.i144, 0
  br i1 %cmp217.i145, label %for.cond4.preheader.lr.ph.i150, label %for.inc17.i171

for.cond4.preheader.lr.ph.i150:                   ; preds = %for.cond1.preheader.i146
  %sub.i147 = add i32 %storemerge21.i144, -2
  %gep_array6082 = mul i32 %sub.i147, 2
  %gep6083 = add i32 %fr, %gep_array6082
  %gep_array6085 = mul i32 %storemerge21.i144, 2
  %gep6086 = add i32 %fr, %gep_array6085
  br label %for.cond4.preheader.i153

for.cond4.preheader.i153:                         ; preds = %for.inc14.i168, %for.cond4.preheader.lr.ph.i150
  %storemerge118.i151 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i150 ], [ %inc15.i166, %for.inc14.i168 ]
  %cmp515.i152 = icmp sgt i32 %storemerge118.i151, 0
  br i1 %cmp515.i152, label %for.body6.lr.ph.i155, label %for.inc14.i168

for.body6.lr.ph.i155:                             ; preds = %for.cond4.preheader.i153
  %gep_array6088 = mul i32 %storemerge118.i151, 2
  %gep6089 = add i32 %fi, %gep_array6088
  br label %for.body6.i165

for.body6.i165:                                   ; preds = %for.body6.i165, %for.body6.lr.ph.i155
  %storemerge216.i156 = phi i32 [ 0, %for.body6.lr.ph.i155 ], [ %add12.i162, %for.body6.i165 ]
  %gep6089.asptr = inttoptr i32 %gep6089 to i16*
  %396 = load i16* %gep6089.asptr, align 1
  %conv.i157 = sext i16 %396 to i32
  %mul.i158 = mul i32 %conv.i157, %storemerge14502
  %gep6083.asptr = inttoptr i32 %gep6083 to i16*
  %397 = load i16* %gep6083.asptr, align 1
  %conv83.i159 = zext i16 %397 to i32
  %add.i160 = add i32 %mul.i158, %conv83.i159
  %conv9.i161 = trunc i32 %add.i160 to i16
  %gep6086.asptr = inttoptr i32 %gep6086 to i16*
  store i16 %conv9.i161, i16* %gep6086.asptr, align 1
  %gep6089.asptr133 = inttoptr i32 %gep6089 to i16*
  %398 = load i16* %gep6089.asptr133, align 1
  %add12.i162 = add i32 %storemerge216.i156, 1
  %gep_array6091 = mul i32 %add12.i162, 2
  %gep6092 = add i32 %fr, %gep_array6091
  %gep6092.asptr = inttoptr i32 %gep6092 to i16*
  store i16 %398, i16* %gep6092.asptr, align 1
  %cmp5.i164 = icmp slt i32 %add12.i162, %storemerge118.i151
  br i1 %cmp5.i164, label %for.body6.i165, label %for.inc14.i168

for.inc14.i168:                                   ; preds = %for.body6.i165, %for.cond4.preheader.i153
  %inc15.i166 = add i32 %storemerge118.i151, 1
  %cmp2.i167 = icmp slt i32 %inc15.i166, %storemerge21.i144
  br i1 %cmp2.i167, label %for.cond4.preheader.i153, label %for.inc17.i171

for.inc17.i171:                                   ; preds = %for.inc14.i168, %for.cond1.preheader.i146
  %inc18.i169 = add i32 %storemerge21.i144, 1
  %cmp.i170 = icmp slt i32 %inc18.i169, %storemerge14502
  br i1 %cmp.i170, label %for.cond1.preheader.i146, label %for.cond1.preheader.i116

for.cond1.preheader.i116:                         ; preds = %for.inc17.i171, %for.inc17.i141
  %storemerge21.i114 = phi i32 [ %inc18.i139, %for.inc17.i141 ], [ 0, %for.inc17.i171 ]
  %cmp217.i115 = icmp sgt i32 %storemerge21.i114, 0
  br i1 %cmp217.i115, label %for.cond4.preheader.lr.ph.i120, label %for.inc17.i141

for.cond4.preheader.lr.ph.i120:                   ; preds = %for.cond1.preheader.i116
  %sub.i117 = add i32 %storemerge21.i114, -2
  %gep_array6094 = mul i32 %sub.i117, 2
  %gep6095 = add i32 %fr, %gep_array6094
  %gep_array6097 = mul i32 %storemerge21.i114, 2
  %gep6098 = add i32 %fr, %gep_array6097
  br label %for.cond4.preheader.i123

for.cond4.preheader.i123:                         ; preds = %for.inc14.i138, %for.cond4.preheader.lr.ph.i120
  %storemerge118.i121 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i120 ], [ %inc15.i136, %for.inc14.i138 ]
  %cmp515.i122 = icmp sgt i32 %storemerge118.i121, 0
  br i1 %cmp515.i122, label %for.body6.lr.ph.i125, label %for.inc14.i138

for.body6.lr.ph.i125:                             ; preds = %for.cond4.preheader.i123
  %gep_array6100 = mul i32 %storemerge118.i121, 2
  %gep6101 = add i32 %fi, %gep_array6100
  br label %for.body6.i135

for.body6.i135:                                   ; preds = %for.body6.i135, %for.body6.lr.ph.i125
  %storemerge216.i126 = phi i32 [ 0, %for.body6.lr.ph.i125 ], [ %add12.i132, %for.body6.i135 ]
  %gep6101.asptr = inttoptr i32 %gep6101 to i16*
  %399 = load i16* %gep6101.asptr, align 1
  %conv.i127 = sext i16 %399 to i32
  %mul.i128 = mul i32 %conv.i127, %storemerge14502
  %gep6095.asptr = inttoptr i32 %gep6095 to i16*
  %400 = load i16* %gep6095.asptr, align 1
  %conv83.i129 = zext i16 %400 to i32
  %add.i130 = add i32 %mul.i128, %conv83.i129
  %conv9.i131 = trunc i32 %add.i130 to i16
  %gep6098.asptr = inttoptr i32 %gep6098 to i16*
  store i16 %conv9.i131, i16* %gep6098.asptr, align 1
  %gep6101.asptr134 = inttoptr i32 %gep6101 to i16*
  %401 = load i16* %gep6101.asptr134, align 1
  %add12.i132 = add i32 %storemerge216.i126, 1
  %gep_array6103 = mul i32 %add12.i132, 2
  %gep6104 = add i32 %fr, %gep_array6103
  %gep6104.asptr = inttoptr i32 %gep6104 to i16*
  store i16 %401, i16* %gep6104.asptr, align 1
  %cmp5.i134 = icmp slt i32 %add12.i132, %storemerge118.i121
  br i1 %cmp5.i134, label %for.body6.i135, label %for.inc14.i138

for.inc14.i138:                                   ; preds = %for.body6.i135, %for.cond4.preheader.i123
  %inc15.i136 = add i32 %storemerge118.i121, 1
  %cmp2.i137 = icmp slt i32 %inc15.i136, %storemerge21.i114
  br i1 %cmp2.i137, label %for.cond4.preheader.i123, label %for.inc17.i141

for.inc17.i141:                                   ; preds = %for.inc14.i138, %for.cond1.preheader.i116
  %inc18.i139 = add i32 %storemerge21.i114, 1
  %cmp.i140 = icmp slt i32 %inc18.i139, %storemerge14502
  br i1 %cmp.i140, label %for.cond1.preheader.i116, label %for.cond1.preheader.i86

for.cond1.preheader.i86:                          ; preds = %for.inc17.i141, %for.inc17.i111
  %storemerge21.i84 = phi i32 [ %inc18.i109, %for.inc17.i111 ], [ 0, %for.inc17.i141 ]
  %cmp217.i85 = icmp sgt i32 %storemerge21.i84, 0
  br i1 %cmp217.i85, label %for.cond4.preheader.lr.ph.i90, label %for.inc17.i111

for.cond4.preheader.lr.ph.i90:                    ; preds = %for.cond1.preheader.i86
  %sub.i87 = add i32 %storemerge21.i84, -2
  %gep_array6106 = mul i32 %sub.i87, 2
  %gep6107 = add i32 %fr, %gep_array6106
  %gep_array6109 = mul i32 %storemerge21.i84, 2
  %gep6110 = add i32 %fr, %gep_array6109
  br label %for.cond4.preheader.i93

for.cond4.preheader.i93:                          ; preds = %for.inc14.i108, %for.cond4.preheader.lr.ph.i90
  %storemerge118.i91 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i90 ], [ %inc15.i106, %for.inc14.i108 ]
  %cmp515.i92 = icmp sgt i32 %storemerge118.i91, 0
  br i1 %cmp515.i92, label %for.body6.lr.ph.i95, label %for.inc14.i108

for.body6.lr.ph.i95:                              ; preds = %for.cond4.preheader.i93
  %gep_array6112 = mul i32 %storemerge118.i91, 2
  %gep6113 = add i32 %Sinewave, %gep_array6112
  br label %for.body6.i105

for.body6.i105:                                   ; preds = %for.body6.i105, %for.body6.lr.ph.i95
  %storemerge216.i96 = phi i32 [ 0, %for.body6.lr.ph.i95 ], [ %add12.i102, %for.body6.i105 ]
  %gep6113.asptr = inttoptr i32 %gep6113 to i16*
  %402 = load i16* %gep6113.asptr, align 1
  %conv.i97 = sext i16 %402 to i32
  %mul.i98 = mul i32 %conv.i97, %storemerge14502
  %gep6107.asptr = inttoptr i32 %gep6107 to i16*
  %403 = load i16* %gep6107.asptr, align 1
  %conv83.i99 = zext i16 %403 to i32
  %add.i100 = add i32 %mul.i98, %conv83.i99
  %conv9.i101 = trunc i32 %add.i100 to i16
  %gep6110.asptr = inttoptr i32 %gep6110 to i16*
  store i16 %conv9.i101, i16* %gep6110.asptr, align 1
  %gep6113.asptr135 = inttoptr i32 %gep6113 to i16*
  %404 = load i16* %gep6113.asptr135, align 1
  %add12.i102 = add i32 %storemerge216.i96, 1
  %gep_array6115 = mul i32 %add12.i102, 2
  %gep6116 = add i32 %fr, %gep_array6115
  %gep6116.asptr = inttoptr i32 %gep6116 to i16*
  store i16 %404, i16* %gep6116.asptr, align 1
  %cmp5.i104 = icmp slt i32 %add12.i102, %storemerge118.i91
  br i1 %cmp5.i104, label %for.body6.i105, label %for.inc14.i108

for.inc14.i108:                                   ; preds = %for.body6.i105, %for.cond4.preheader.i93
  %inc15.i106 = add i32 %storemerge118.i91, 1
  %cmp2.i107 = icmp slt i32 %inc15.i106, %storemerge21.i84
  br i1 %cmp2.i107, label %for.cond4.preheader.i93, label %for.inc17.i111

for.inc17.i111:                                   ; preds = %for.inc14.i108, %for.cond1.preheader.i86
  %inc18.i109 = add i32 %storemerge21.i84, 1
  %cmp.i110 = icmp slt i32 %inc18.i109, %storemerge14502
  br i1 %cmp.i110, label %for.cond1.preheader.i86, label %for.cond1.preheader.i56

for.cond1.preheader.i56:                          ; preds = %for.inc17.i111, %for.inc17.i81
  %storemerge21.i54 = phi i32 [ %inc18.i79, %for.inc17.i81 ], [ 0, %for.inc17.i111 ]
  %cmp217.i55 = icmp sgt i32 %storemerge21.i54, 0
  br i1 %cmp217.i55, label %for.cond4.preheader.lr.ph.i60, label %for.inc17.i81

for.cond4.preheader.lr.ph.i60:                    ; preds = %for.cond1.preheader.i56
  %sub.i57 = add i32 %storemerge21.i54, -2
  %gep_array6118 = mul i32 %sub.i57, 2
  %gep6119 = add i32 %fr, %gep_array6118
  %gep_array6121 = mul i32 %storemerge21.i54, 2
  %gep6122 = add i32 %fr, %gep_array6121
  br label %for.cond4.preheader.i63

for.cond4.preheader.i63:                          ; preds = %for.inc14.i78, %for.cond4.preheader.lr.ph.i60
  %storemerge118.i61 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i60 ], [ %inc15.i76, %for.inc14.i78 ]
  %cmp515.i62 = icmp sgt i32 %storemerge118.i61, 0
  br i1 %cmp515.i62, label %for.body6.lr.ph.i65, label %for.inc14.i78

for.body6.lr.ph.i65:                              ; preds = %for.cond4.preheader.i63
  %gep_array6124 = mul i32 %storemerge118.i61, 2
  %gep6125 = add i32 %Sinewave, %gep_array6124
  br label %for.body6.i75

for.body6.i75:                                    ; preds = %for.body6.i75, %for.body6.lr.ph.i65
  %storemerge216.i66 = phi i32 [ 0, %for.body6.lr.ph.i65 ], [ %add12.i72, %for.body6.i75 ]
  %gep6125.asptr = inttoptr i32 %gep6125 to i16*
  %405 = load i16* %gep6125.asptr, align 1
  %conv.i67 = sext i16 %405 to i32
  %mul.i68 = mul i32 %conv.i67, %storemerge14502
  %gep6119.asptr = inttoptr i32 %gep6119 to i16*
  %406 = load i16* %gep6119.asptr, align 1
  %conv83.i69 = zext i16 %406 to i32
  %add.i70 = add i32 %mul.i68, %conv83.i69
  %conv9.i71 = trunc i32 %add.i70 to i16
  %gep6122.asptr = inttoptr i32 %gep6122 to i16*
  store i16 %conv9.i71, i16* %gep6122.asptr, align 1
  %gep6125.asptr136 = inttoptr i32 %gep6125 to i16*
  %407 = load i16* %gep6125.asptr136, align 1
  %add12.i72 = add i32 %storemerge216.i66, 1
  %gep_array6127 = mul i32 %add12.i72, 2
  %gep6128 = add i32 %fr, %gep_array6127
  %gep6128.asptr = inttoptr i32 %gep6128 to i16*
  store i16 %407, i16* %gep6128.asptr, align 1
  %cmp5.i74 = icmp slt i32 %add12.i72, %storemerge118.i61
  br i1 %cmp5.i74, label %for.body6.i75, label %for.inc14.i78

for.inc14.i78:                                    ; preds = %for.body6.i75, %for.cond4.preheader.i63
  %inc15.i76 = add i32 %storemerge118.i61, 1
  %cmp2.i77 = icmp slt i32 %inc15.i76, %storemerge21.i54
  br i1 %cmp2.i77, label %for.cond4.preheader.i63, label %for.inc17.i81

for.inc17.i81:                                    ; preds = %for.inc14.i78, %for.cond1.preheader.i56
  %inc18.i79 = add i32 %storemerge21.i54, 1
  %cmp.i80 = icmp slt i32 %inc18.i79, %storemerge14502
  br i1 %cmp.i80, label %for.cond1.preheader.i56, label %for.cond1.preheader.i26

for.cond1.preheader.i26:                          ; preds = %for.inc17.i81, %for.inc17.i51
  %storemerge21.i24 = phi i32 [ %inc18.i49, %for.inc17.i51 ], [ 0, %for.inc17.i81 ]
  %cmp217.i25 = icmp sgt i32 %storemerge21.i24, 0
  br i1 %cmp217.i25, label %for.cond4.preheader.lr.ph.i30, label %for.inc17.i51

for.cond4.preheader.lr.ph.i30:                    ; preds = %for.cond1.preheader.i26
  %sub.i27 = add i32 %storemerge21.i24, -2
  %gep_array6130 = mul i32 %sub.i27, 2
  %gep6131 = add i32 %fr, %gep_array6130
  %gep_array6133 = mul i32 %storemerge21.i24, 2
  %gep6134 = add i32 %fr, %gep_array6133
  br label %for.cond4.preheader.i33

for.cond4.preheader.i33:                          ; preds = %for.inc14.i48, %for.cond4.preheader.lr.ph.i30
  %storemerge118.i31 = phi i32 [ 0, %for.cond4.preheader.lr.ph.i30 ], [ %inc15.i46, %for.inc14.i48 ]
  %cmp515.i32 = icmp sgt i32 %storemerge118.i31, 0
  br i1 %cmp515.i32, label %for.body6.lr.ph.i35, label %for.inc14.i48

for.body6.lr.ph.i35:                              ; preds = %for.cond4.preheader.i33
  %gep_array6136 = mul i32 %storemerge118.i31, 2
  %gep6137 = add i32 %Sinewave, %gep_array6136
  br label %for.body6.i45

for.body6.i45:                                    ; preds = %for.body6.i45, %for.body6.lr.ph.i35
  %storemerge216.i36 = phi i32 [ 0, %for.body6.lr.ph.i35 ], [ %add12.i42, %for.body6.i45 ]
  %gep6137.asptr = inttoptr i32 %gep6137 to i16*
  %408 = load i16* %gep6137.asptr, align 1
  %conv.i37 = sext i16 %408 to i32
  %mul.i38 = mul i32 %conv.i37, %storemerge14502
  %gep6131.asptr = inttoptr i32 %gep6131 to i16*
  %409 = load i16* %gep6131.asptr, align 1
  %conv83.i39 = zext i16 %409 to i32
  %add.i40 = add i32 %mul.i38, %conv83.i39
  %conv9.i41 = trunc i32 %add.i40 to i16
  %gep6134.asptr = inttoptr i32 %gep6134 to i16*
  store i16 %conv9.i41, i16* %gep6134.asptr, align 1
  %gep6137.asptr137 = inttoptr i32 %gep6137 to i16*
  %410 = load i16* %gep6137.asptr137, align 1
  %add12.i42 = add i32 %storemerge216.i36, 1
  %gep_array6139 = mul i32 %add12.i42, 2
  %gep6140 = add i32 %fr, %gep_array6139
  %gep6140.asptr = inttoptr i32 %gep6140 to i16*
  store i16 %410, i16* %gep6140.asptr, align 1
  %cmp5.i44 = icmp slt i32 %add12.i42, %storemerge118.i31
  br i1 %cmp5.i44, label %for.body6.i45, label %for.inc14.i48

for.inc14.i48:                                    ; preds = %for.body6.i45, %for.cond4.preheader.i33
  %inc15.i46 = add i32 %storemerge118.i31, 1
  %cmp2.i47 = icmp slt i32 %inc15.i46, %storemerge21.i24
  br i1 %cmp2.i47, label %for.cond4.preheader.i33, label %for.inc17.i51

for.inc17.i51:                                    ; preds = %for.inc14.i48, %for.cond1.preheader.i26
  %inc18.i49 = add i32 %storemerge21.i24, 1
  %cmp.i50 = icmp slt i32 %inc18.i49, %storemerge14502
  br i1 %cmp.i50, label %for.cond1.preheader.i26, label %foo3.exit52

foo3.exit52:                                      ; preds = %for.inc17.i51, %for.body82
  %gep_array6142 = mul i32 %storemerge34500, 2
  %gep6143 = add i32 %fr, %gep_array6142
  %gep6143.asptr = inttoptr i32 %gep6143 to i16*
  %411 = load i16* %gep6143.asptr, align 1
  %gep_array6145 = mul i32 %storemerge34500, 2
  %gep6146 = add i32 %fi, %gep_array6145
  %gep6146.asptr = inttoptr i32 %gep6146 to i16*
  %412 = load i16* %gep6146.asptr, align 1
  br i1 %tobool70, label %if.end110, label %if.then103

if.then103:                                       ; preds = %foo3.exit52
  %conv104 = sext i16 %411 to i32
  %shr10516 = lshr i32 %conv104, 1
  %conv106 = trunc i32 %shr10516 to i16
  %conv107 = sext i16 %412 to i32
  %shr10817 = lshr i32 %conv107, 1
  %conv109 = trunc i32 %shr10817 to i16
  br label %if.end110

if.end110:                                        ; preds = %foo3.exit52, %if.then103
  %qr.0.load40144479 = phi i16 [ %411, %foo3.exit52 ], [ %conv106, %if.then103 ]
  %qi.0.load40124478 = phi i16 [ %412, %foo3.exit52 ], [ %conv109, %if.then103 ]
  %sub113 = sub i16 %qr.0.load40144479, %sub90
  %gep5753.asptr138 = inttoptr i32 %gep5753 to i16*
  store i16 %sub113, i16* %gep5753.asptr138, align 1
  %sub118 = sub i16 %qi.0.load40124478, %add98
  %gep5756.asptr139 = inttoptr i32 %gep5756 to i16*
  store i16 %sub118, i16* %gep5756.asptr139, align 1
  %add123 = add i16 %qr.0.load40144479, %sub90
  %gep6143.asptr140 = inttoptr i32 %gep6143 to i16*
  store i16 %add123, i16* %gep6143.asptr140, align 1
  %add128 = add i16 %qi.0.load40124478, %add98
  %gep6146.asptr141 = inttoptr i32 %gep6146 to i16*
  store i16 %add128, i16* %gep6146.asptr141, align 1
  %add132 = add i32 %storemerge34500, %shl52
  %cmp80 = icmp slt i32 %add132, %shl
  br i1 %cmp80, label %for.body82, label %for.inc134

for.inc134:                                       ; preds = %if.end110, %if.end78
  %inc135 = add i32 %storemerge14502, 1
  %cmp54 = icmp slt i32 %inc135, %shl5244854506
  br i1 %cmp54, label %for.body56, label %for.end136

for.end136:                                       ; preds = %for.inc134, %if.end51
  %storemerge1.lcssa = phi i32 [ 0, %if.end51 ], [ %shl5244854506, %for.inc134 ]
  %dec = add i32 %dec44844507, -1
  %cmp17 = icmp slt i32 %shl52, %shl
  br i1 %cmp17, label %while.body, label %return

return:                                           ; preds = %while.cond.preheader, %for.end136, %entry
  %storemerge22 = phi i32 [ -1, %entry ], [ 0, %while.cond.preheader ], [ %scale.0.load40174482, %for.end136 ]
  ret i32 %storemerge22
}

define internal i32 @nacl_tp_tdb_offset(i32) {
entry:
  ret i32 0
}

define internal i32 @nacl_tp_tls_offset(i32 %size) {
entry:
  %result = sub i32 0, %size
  ret i32 %result
}
