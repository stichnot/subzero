; Simple test of non-fused compare/branch.

; RUN: %llvm2ice -O2 --verbose none %s | FileCheck %s
; RUN: %llvm2ice -Om1 --verbose none %s | FileCheck --check-prefix=OPTM1 %s
; RUN: %llvm2ice --verbose none %s | FileCheck --check-prefix=ERRORS %s
; RUN: %llvm2iceinsts %s | %szdiff %s | FileCheck --check-prefix=DUMP %s

define void @testBool(i32 %a, i32 %b) {
entry:
  %cmp = icmp slt i32 %a, %b
  %cmp1 = icmp sgt i32 %a, %b
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  tail call void @use(i1 %cmp)
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  br i1 %cmp1, label %if.then5, label %if.end7

if.then5:                                         ; preds = %if.end
  tail call void @use(i1 %cmp1)
  br label %if.end7

if.end7:                                          ; preds = %if.then5, %if.end
  ret void
}

declare void @use(i1 zeroext)

; CHECK:      .globl testBool
; Two bool computations
; CHECK:      cmp
; CHECK:      cmp
; Test first bool
; CHECK:      cmp
; CHECK:      call
; Test second bool
; CHECK:      cmp
; CHECK:      call
; CHECK:      ret
;
; OPTM1:      .globl testBool
; Two bool computations
; OPTM1:      cmp
; OPTM1:      cmp
; Test first bool
; OPTM1:      cmp
; OPTM1:      call
; Test second bool
; OPTM1:      cmp
; OPTM1:      call
; OPTM1:      ret

; ERRORS-NOT: ICE translation error
; DUMP-NOT: SZ
