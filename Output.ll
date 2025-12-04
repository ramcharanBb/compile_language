; ModuleID = 'ram-compiler'
source_filename = "ram-compiler"

@str = private unnamed_addr constant [17 x i8] c"x is less than y\00", align 1
@fmt = private unnamed_addr constant [5 x i8] c"%s \0A\00", align 1
@str.1 = private unnamed_addr constant [21 x i8] c"x is not less than y\00", align 1
@str.2 = private unnamed_addr constant [21 x i8] c"Success\0AWith\09Escapes\00", align 1

define void @main() {
  %y = alloca i64, align 8
  store double 2.010000e+01, ptr %y, align 8
  %y2 = load i64, ptr %y, align 4
  %cmptmp = icmp slt i64 10, %y2
  %ifcond = icmp ne i1 %cmptmp, false
  br i1 %ifcond, label %then, label %else

then:                                             ; preds = %0
  %printfCall = call i32 (ptr, ...) @printf(ptr @fmt, ptr @str)
  br label %ifcont

else:                                             ; preds = %0
  %printfCall3 = call i32 (ptr, ...) @printf(ptr @fmt, ptr @str.1)
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  %shl_tmp = shl i64 10, 4
  %cmptmp6 = icmp sgt i64 %shl_tmp, 0
  %ifcond7 = icmp ne i1 %cmptmp6, false
  br i1 %ifcond7, label %then8, label %ifcont10

then8:                                            ; preds = %ifcont
  %printfCall9 = call i32 (ptr, ...) @printf(ptr @fmt, ptr @str.2)
  br label %ifcont10

ifcont10:                                         ; preds = %then8, %ifcont
  ret void
}

declare i32 @printf(ptr, ...)
