define void @dummy_icmp(i64 %foo, i64 %bar) {
entry:
  %r1 = icmp eq i64 %foo, %bar
  %r2 = icmp slt i64 %foo, %bar
  ret void
}
