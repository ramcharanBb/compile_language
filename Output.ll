; ModuleID = 'ram-compiler'
source_filename = "ram-compiler"

@str = private unnamed_addr constant [17 x i8] c"x is less than y\00", align 1
@fmt = private unnamed_addr constant [5 x i8] c"%s \0A\00", align 1
@str.1 = private unnamed_addr constant [21 x i8] c"x is not less than y\00", align 1
@str.2 = private unnamed_addr constant [21 x i8] c"Success\0AWith\09Escapes\00", align 1

define void @main() {
  %cmptmp = fcmp olt double 1.000000e+01, 2.000000e+01
  br i1 %cmptmp, label %then, label %else

then:                                             ; preds = %0
  %printfCall = call i32 (ptr, ...) @printf(ptr @fmt, ptr @str)
  br label %ifcont

else:                                             ; preds = %0
  %printfCall3 = call i32 (ptr, ...) @printf(ptr @fmt, ptr @str.1)
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  %z = alloca double, align 8
  %addtmp = fadd double 1.000000e+01, 2.000000e+01
  store double %addtmp, ptr %z, align 8
  %z6 = load double, ptr %z, align 8
  %cmptmp7 = fcmp ogt double %z6, 0.000000e+00
  br i1 %cmptmp7, label %then8, label %ifcont10

then8:                                            ; preds = %ifcont
  %printfCall9 = call i32 (ptr, ...) @printf(ptr @fmt, ptr @str.2)
  br label %ifcont10

ifcont10:                                         ; preds = %then8, %ifcont
  ret void
}

declare i32 @printf(ptr, ...)
