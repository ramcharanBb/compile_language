; ModuleID = 'ram-compiler'
source_filename = "ram-compiler"

@fmt = private unnamed_addr constant [5 x i8] c"%f \0A\00", align 1
@str = private unnamed_addr constant [17 x i8] c"x is less than y\00", align 1
@fmt.1 = private unnamed_addr constant [5 x i8] c"%s \0A\00", align 1
@str.2 = private unnamed_addr constant [22 x i8] c"x is greater or equal\00", align 1
@str.3 = private unnamed_addr constant [19 x i8] c"This always runs A\00", align 1
@str.4 = private unnamed_addr constant [42 x i8] c"Unreachable print A, but has side effect.\00", align 1
@str.5 = private unnamed_addr constant [42 x i8] c"Unreachable print B, but has side effect.\00", align 1
@str.6 = private unnamed_addr constant [19 x i8] c"This always runs B\00", align 1
@str.7 = private unnamed_addr constant [6 x i8] c"Done!\00", align 1

define void @test_no_dce() {
  %addtmp = fadd double 1.000000e+01, 2.000000e+01
  %printfCall = call i32 (ptr, ...) @printf(ptr @fmt, double %addtmp)
  %cmptmp = fcmp olt double 1.000000e+01, 2.000000e+01
  br i1 %cmptmp, label %then, label %else

then:                                             ; preds = %0
  %printfCall6 = call i32 (ptr, ...) @printf(ptr @fmt.1, ptr @str)
  br label %ifcont

else:                                             ; preds = %0
  %printfCall7 = call i32 (ptr, ...) @printf(ptr @fmt.1, ptr @str.2)
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  %limit = alloca double, align 8
  %multmp = fmul double %addtmp, 2.000000e+00
  store double %multmp, ptr %limit, align 8
  %count = alloca double, align 8
  store double 0.000000e+00, ptr %count, align 8
  br label %whilecond

whilecond:                                        ; preds = %ifcont18, %ifcont
  %count9 = load double, ptr %count, align 8
  %cmptmp10 = fcmp olt double %count9, 5.000000e+00
  br i1 %cmptmp10, label %whilebody, label %afterwhile

whilebody:                                        ; preds = %whilecond
  %count11 = load double, ptr %count, align 8
  %printfCall12 = call i32 (ptr, ...) @printf(ptr @fmt, double %count11)
  %count13 = load double, ptr %count, align 8
  %cmptmp14 = fcmp oeq double %count13, 4.000000e+00
  br i1 %cmptmp14, label %then15, label %ifcont18

then15:                                           ; preds = %whilebody
  %limit16 = load double, ptr %limit, align 8
  %printfCall17 = call i32 (ptr, ...) @printf(ptr @fmt, double %limit16)
  br label %ifcont18

ifcont18:                                         ; preds = %then15, %whilebody
  %count19 = load double, ptr %count, align 8
  %addtmp20 = fadd double %count19, 1.000000e+00
  store double %addtmp20, ptr %count, align 8
  br label %whilecond

afterwhile:                                       ; preds = %whilecond
  br i1 true, label %then21, label %else23

then21:                                           ; preds = %afterwhile
  %printfCall22 = call i32 (ptr, ...) @printf(ptr @fmt.1, ptr @str.3)
  br label %ifcont25

else23:                                           ; preds = %afterwhile
  %printfCall24 = call i32 (ptr, ...) @printf(ptr @fmt.1, ptr @str.4)
  br label %ifcont25

ifcont25:                                         ; preds = %else23, %then21
  br i1 false, label %then26, label %else28

then26:                                           ; preds = %ifcont25
  %printfCall27 = call i32 (ptr, ...) @printf(ptr @fmt.1, ptr @str.5)
  br label %ifcont30

else28:                                           ; preds = %ifcont25
  %printfCall29 = call i32 (ptr, ...) @printf(ptr @fmt.1, ptr @str.6)
  br label %ifcont30

ifcont30:                                         ; preds = %else28, %then26
  %magic_num = alloca double, align 8
  store double 3.600000e+01, ptr %magic_num, align 8
  %magic_num31 = load double, ptr %magic_num, align 8
  %printfCall32 = call i32 (ptr, ...) @printf(ptr @fmt, double %magic_num31)
  ret void
}

declare i32 @printf(ptr, ...)

define void @main() {
  call void @test_no_dce()
  %printfCall = call i32 (ptr, ...) @printf(ptr @fmt.1, ptr @str.7)
  ret void
}
