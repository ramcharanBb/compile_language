; ModuleID = 'ram-compiler'
source_filename = "ram-compiler"

@fmt = private unnamed_addr constant [5 x i8] c"%f \0A\00", align 1
@fmt.1 = private unnamed_addr constant [2 x i8] c"\0A\00", align 1

define double @test_binary() {
  %printfCall = call i32 (ptr, ...) @printf(ptr @fmt, double 3.000000e+00)
  %printfCall1 = call i32 (ptr, ...) @printf(ptr @fmt, double 1.700000e+01)
  %printfCall2 = call i32 (ptr, ...) @printf(ptr @fmt, double 4.000000e+00)
  %printfCall3 = call i32 (ptr, ...) @printf(ptr @fmt.1, i1 true)
  %printfCall4 = call i32 (ptr, ...) @printf(ptr @fmt.1, i1 true)
  %printfCall5 = call i32 (ptr, ...) @printf(ptr @fmt, double 1.400000e+01)
  ret double 0.000000e+00
}

declare i32 @printf(ptr, ...)

define double @main() {
  %calltmp = call double @test_binary()
  ret double 0.000000e+00
}