; ModuleID = 'ram-compiler'
source_filename = "ram-compiler"

@fmt = private unnamed_addr constant [5 x i8] c"%d \0A\00", align 1
@str = private unnamed_addr constant [17 x i8] c"x is less than y\00", align 1
@fmt.1 = private unnamed_addr constant [5 x i8] c"%s \0A\00", align 1
@str.2 = private unnamed_addr constant [22 x i8] c"x is greater or equal\00", align 1
@str.3 = private unnamed_addr constant [19 x i8] c"This always runs A\00", align 1
@str.4 = private unnamed_addr constant [42 x i8] c"Unreachable print A, but has side effect.\00", align 1
@str.5 = private unnamed_addr constant [42 x i8] c"Unreachable print B, but has side effect.\00", align 1
@str.6 = private unnamed_addr constant [19 x i8] c"This always runs B\00", align 1
@str.7 = private unnamed_addr constant [6 x i8] c"Done!\00", align 1

define void @test_no_dce() {
  %addtmp = add i64 10, 20
  %printfCall = call i32 (ptr, ...) @printf(ptr @fmt, i64 %addtmp)
  %cmptmp = icmp slt i64 10, 20
  %ifcond = icmp ne i1 %cmptmp, false
  br i1 %ifcond, label %then, label %else

then:                                             ; preds = %0
  %printfCall6 = call i32 (ptr, ...) @printf(ptr @fmt.1, ptr @str)
  br label %ifcont

else:                                             ; preds = %0
  %printfCall7 = call i32 (ptr, ...) @printf(ptr @fmt.1, ptr @str.2)
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  %limit = alloca i64, align 8
  %casttmp = uitofp i64 %addtmp to double
  %multmp = fmul double %casttmp, 2.000000e+00
  %fptosi = fptosi double %multmp to i64
  store i64 %fptosi, ptr %limit, align 4
  %count = alloca i64, align 8
  store i64 0, ptr %count, align 4
  br label %whilecond

whilecond:                                        ; preds = %ifcont22, %ifcont
  %count9 = load i64, ptr %count, align 4
  %casttmp10 = uitofp i64 %count9 to double
  %cmptmp11 = fcmp olt double %casttmp10, 5.000000e+00
  %whilecond12 = icmp ne i1 %cmptmp11, false
  br i1 %whilecond12, label %whilebody, label %afterwhile

whilebody:                                        ; preds = %whilecond
  %count13 = load i64, ptr %count, align 4
  %printfCall14 = call i32 (ptr, ...) @printf(ptr @fmt, i64 %count13)
  %count15 = load i64, ptr %count, align 4
  %casttmp16 = uitofp i64 %count15 to double
  %cmptmp17 = fcmp oeq double %casttmp16, 4.000000e+00
  %ifcond18 = icmp ne i1 %cmptmp17, false
  br i1 %ifcond18, label %then19, label %ifcont22

then19:                                           ; preds = %whilebody
  %limit20 = load i64, ptr %limit, align 4
  %printfCall21 = call i32 (ptr, ...) @printf(ptr @fmt, i64 %limit20)
  br label %ifcont22

ifcont22:                                         ; preds = %then19, %whilebody
  %count23 = load i64, ptr %count, align 4
  %casttmp24 = uitofp i64 %count23 to double
  %addtmp25 = fadd double %casttmp24, 1.000000e+00
  %fptosi26 = fptosi double %addtmp25 to i64
  store i64 %fptosi26, ptr %count, align 4
  br label %whilecond

afterwhile:                                       ; preds = %whilecond
  %printfCall28 = call i32 (ptr, ...) @printf(ptr @fmt.1, ptr @str.3)
  br label %ifcont31

else29:                                           ; No predecessors!
  %printfCall30 = call i32 (ptr, ...) @printf(ptr @fmt.1, ptr @str.4)
  br label %ifcont31

ifcont31:                                         ; preds = %else29, %afterwhile
  %printfCall35 = call i32 (ptr, ...) @printf(ptr @fmt.1, ptr @str.6)
  br label %ifcont36

then32:                                           ; No predecessors!
  %printfCall33 = call i32 (ptr, ...) @printf(ptr @fmt.1, ptr @str.5)
  br label %ifcont36

ifcont36:                                         ; preds = %ifcont31, %then32
  %magic_num = alloca i64, align 8
  store i64 36, ptr %magic_num, align 4
  %magic_num37 = load i64, ptr %magic_num, align 4
  %printfCall38 = call i32 (ptr, ...) @printf(ptr @fmt, i64 %magic_num37)
  ret void
}

declare i32 @printf(ptr, ...)

define void @main() {
  call void @test_no_dce()
  %printfCall = call i32 (ptr, ...) @printf(ptr @fmt.1, ptr @str.7)
  ret void
}
