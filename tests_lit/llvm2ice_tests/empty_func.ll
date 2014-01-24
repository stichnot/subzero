; RUN: %llvm2ice %s | FileCheck %s

; CHECK: converting to ICE

; Function Attrs: nounwind readnone
define void @foo() {
entry:
  ret void
}


