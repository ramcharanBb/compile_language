; Simple LLVM IR test file to verify MyPass
define i32 @add(i32 %a, i32 %b) {
entry:
  %sum = add i32 %a, %b
  ret i32 %sum
}

define i32 @multiply(i32 %x, i32 %y) {
entry:
  %product = mul i32 %x, %y
  %incremented = add i32 %product, 1
  ret i32 %incremented
}
