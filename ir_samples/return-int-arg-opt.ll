; ModuleID = '/tmp/tmpAM8vzj/return-int-arg.ll'
target datalayout = "e-i64:64-f80:128-s:64-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readnone
define i32 @func_single_arg(i32 %a) #0 {
entry:
  ret i32 %a
}

; Function Attrs: nounwind readnone
define i32 @func_multiple_args(i32 %a, i32 %b, i32 %c) #0 {
entry:
  ret i32 %c
}

attributes #0 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.5 (trunk 197997)"}
