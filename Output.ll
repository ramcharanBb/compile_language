; ModuleID = 'ram-compiler'
source_filename = "ram-compiler"

@fmt = private unnamed_addr constant [5 x i8] c"%d \0A\00", align 1

define i32 @test_vars() {
  %multmp = mul i32 10, 20
  %addtmp = add i32 %multmp, 10
  %subtmp = sub i32 %addtmp, 49
  %printfCall = call i32 (ptr, ...) @printf(ptr @fmt, i32 %subtmp)
  ret i32 %subtmp
}

declare i32 @printf(ptr, ...)

define void @main() {
  %calltmp = call i32 @test_vars()
  ret void
}
