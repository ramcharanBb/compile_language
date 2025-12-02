; ModuleID = 'ram-compiler'
source_filename = "ram-compiler"

@fmt = private unnamed_addr constant [5 x i8] c"%f \0A\00", align 1
@fmt.1 = private unnamed_addr constant [8 x i8] c"%f %f \0A\00", align 1
@str = private unnamed_addr constant [4 x i8] c"ram\00", align 1

define void @bar(ptr %x, double %y) {
  %printfCall = call i32 (ptr, ...) @printf([5 x i8]* @fmt, double 7.000000e+00)
  %printfCall4 = call i32 (ptr, ...) @printf([5 x i8]* @fmt, double 1.100000e+00)
}

declare i32 @printf(ptr, ...)

define void @__main() {
  %printfCall = call i32 (ptr, ...) @printf([8 x i8]* @fmt.1, double 1.000000e+01, double 8.000000e+00)
  call void @bar([4 x i8]* @str, double 3.200000e+01)
}
