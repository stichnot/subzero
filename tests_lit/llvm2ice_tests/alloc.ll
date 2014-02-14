; RUN: %llvm2ice --verbose none %s | FileCheck %s

define void @foo() nounwind {
  %x = alloca i32, align 4
  call void @extern_func1(i32* %x) nounwind
  ret void
  ; CHECK:      sub esp, 4
  ; CHECK-NEXT: mov eax, esp
  ; CHECK-NEXT: push eax
  ; CHECK-NEXT: call extern_func1
}

declare void @extern_func1(i32*)

define void @bar(i32 %n) nounwind {
  %array = alloca [100 x i32], align 16
  call void @extern_func2([100 x i32]* %array) nounwind
  ret void
  ; CHECK:      sub esp, 400
  ; CHECK-NEXT: mov eax, esp
  ; CHECK-NEXT: push eax
  ; CHECK-NEXT: call extern_func2
}

declare void @extern_func2([100 x i32]*)
