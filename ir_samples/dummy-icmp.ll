define void @dummy_icmp(i64 %foo, i64 %bar) {
entry:
  %cmp_result = icmp eq i64 %foo, %bar
  ret void
}
