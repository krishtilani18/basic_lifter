; ModuleID = 'lifted_code'
source_filename = "lifted_code"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu-elf"

%struct.State = type { %struct.X86State }
%struct.X86State = type { %struct.ArchState, [32 x %union.VectorReg], %struct.ArithFlags, %union.anon, %struct.Segments, %struct.AddressSpace, %struct.GPR, %struct.X87Stack, %struct.MMX, %struct.FPUStatusFlags, %union.anon, %union.FPU, %struct.SegmentCaches, %struct.K_REG }
%struct.ArchState = type { i32, i32, %union.anon }
%union.VectorReg = type { %union.vec512_t }
%union.vec512_t = type { %struct.uint64v8_t }
%struct.uint64v8_t = type { [8 x i64] }
%struct.ArithFlags = type { i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8 }
%struct.Segments = type { i16, %union.SegmentSelector, i16, %union.SegmentSelector, i16, %union.SegmentSelector, i16, %union.SegmentSelector, i16, %union.SegmentSelector, i16, %union.SegmentSelector }
%union.SegmentSelector = type { i16 }
%struct.AddressSpace = type { i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg }
%struct.Reg = type { %union.anon }
%struct.GPR = type { i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg, i64, %struct.Reg }
%struct.X87Stack = type { [8 x %struct.anon.3] }
%struct.anon.3 = type { [6 x i8], %struct.float80_t }
%struct.float80_t = type { [10 x i8] }
%struct.MMX = type { [8 x %struct.anon.4] }
%struct.anon.4 = type { i64, %union.vec64_t }
%union.vec64_t = type { %struct.uint64v1_t }
%struct.uint64v1_t = type { [1 x i64] }
%struct.FPUStatusFlags = type { i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, [4 x i8] }
%union.anon = type { i64 }
%union.FPU = type { %struct.anon.13 }
%struct.anon.13 = type { %struct.FpuFXSAVE, [96 x i8] }
%struct.FpuFXSAVE = type { %union.SegmentSelector, %union.SegmentSelector, %union.FPUAbridgedTagWord, i8, i16, i32, %union.SegmentSelector, i16, i32, %union.SegmentSelector, i16, %union.FPUControlStatus, %union.FPUControlStatus, [8 x %struct.FPUStackElem], [16 x %union.vec128_t] }
%union.FPUAbridgedTagWord = type { i8 }
%union.FPUControlStatus = type { i32 }
%struct.FPUStackElem = type { %union.anon.11, [6 x i8] }
%union.anon.11 = type { %struct.float80_t }
%union.vec128_t = type { %struct.uint128v1_t }
%struct.uint128v1_t = type { [1 x i128] }
%struct.SegmentCaches = type { %struct.SegmentShadow, %struct.SegmentShadow, %struct.SegmentShadow, %struct.SegmentShadow, %struct.SegmentShadow, %struct.SegmentShadow }
%struct.SegmentShadow = type { %union.anon, i32, i32 }
%struct.K_REG = type { [8 x %struct.anon.18] }
%struct.anon.18 = type { i64, i64 }

@LIFTED.STATE = thread_local(initialexec) global %struct.State zeroinitializer

define ptr @LIFTED.main(ptr noalias %state, i64 %program_counter, ptr noalias %memory) {
  %RAX = getelementptr inbounds %struct.State, ptr %state, i32 0, i32 0, i32 6, i32 1, i32 0, i32 0
  %RSP = getelementptr inbounds %struct.State, ptr %state, i32 0, i32 0, i32 6, i32 13, i32 0, i32 0
  %RBP = getelementptr inbounds %struct.State, ptr %state, i32 0, i32 0, i32 6, i32 15, i32 0, i32 0
  %EDI = getelementptr inbounds %struct.State, ptr %state, i32 0, i32 0, i32 6, i32 11, i32 0, i32 0
  %EDX = getelementptr inbounds %struct.State, ptr %state, i32 0, i32 0, i32 6, i32 7, i32 0, i32 0
  %BRANCH_TAKEN = alloca i8, align 1
  %RETURN_PC = alloca i64, align 8
  %MONITOR = alloca i64, align 8
  %STATE = alloca ptr, align 8
  store ptr %state, ptr %STATE, align 8
  %MEMORY = alloca ptr, align 8
  store ptr %memory, ptr %MEMORY, align 8
  %NEXT_PC = alloca i64, align 8
  store i64 %program_counter, ptr %NEXT_PC, align 8
  %PC = getelementptr inbounds %struct.State, ptr %state, i32 0, i32 0, i32 6, i32 33, i32 0, i32 0
  %CSBASE = alloca i64, align 8
  store i64 0, ptr %CSBASE, align 8
  %SSBASE = alloca i64, align 8
  store i64 0, ptr %SSBASE, align 8
  %ESBASE = alloca i64, align 8
  store i64 0, ptr %ESBASE, align 8
  %DSBASE = alloca i64, align 8
  store i64 0, ptr %DSBASE, align 8
  store i64 %program_counter, ptr %NEXT_PC, align 8
  br label %1

1:                                                ; preds = %0
  %2 = load i64, ptr %NEXT_PC, align 8
  store i64 %2, ptr %PC, align 8
  %3 = add i64 %2, 4
  store i64 %3, ptr %NEXT_PC, align 8
  %4 = load i32, ptr %EDX, align 4
  %5 = zext i32 %4 to i64
  %6 = load i32, ptr %EDI, align 4
  %7 = zext i32 %6 to i64
  %8 = load ptr, ptr %MEMORY, align 8
  store ptr %8, ptr %MEMORY, align 8
  br label %9

9:                                                ; preds = %1
  %10 = load i64, ptr %NEXT_PC, align 8
  store i64 %10, ptr %PC, align 8
  %11 = add i64 %10, 1
  store i64 %11, ptr %NEXT_PC, align 8
  %12 = load i64, ptr %RBP, align 8
  %13 = load ptr, ptr %MEMORY, align 8
  %14 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 6, i32 13
  %15 = load i64, ptr %14, align 8
  %16 = add i64 %15, -8
  %17 = call ptr @__remill_write_memory_64(ptr noundef %13, i64 noundef %16, i64 noundef %12) #3
  store i64 %16, ptr %14, align 8
  store ptr %17, ptr %MEMORY, align 8
  br label %18

18:                                               ; preds = %9
  %19 = load i64, ptr %NEXT_PC, align 8
  store i64 %19, ptr %PC, align 8
  %20 = add i64 %19, 3
  store i64 %20, ptr %NEXT_PC, align 8
  %21 = load i64, ptr %RSP, align 8
  %22 = load ptr, ptr %MEMORY, align 8
  store i64 %21, ptr %RBP, align 8
  store ptr %22, ptr %MEMORY, align 8
  br label %23

23:                                               ; preds = %18
  %24 = load i64, ptr %NEXT_PC, align 8
  store i64 %24, ptr %PC, align 8
  %25 = add i64 %24, 5
  store i64 %25, ptr %NEXT_PC, align 8
  %26 = load ptr, ptr %MEMORY, align 8
  store i64 0, ptr %RAX, align 8
  store ptr %26, ptr %MEMORY, align 8
  br label %27

27:                                               ; preds = %23
  %28 = load i64, ptr %NEXT_PC, align 8
  store i64 %28, ptr %PC, align 8
  %29 = add i64 %28, 1
  store i64 %29, ptr %NEXT_PC, align 8
  %30 = load ptr, ptr %MEMORY, align 8
  %31 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 6, i32 13
  %32 = load i64, ptr %31, align 8
  %33 = add i64 %32, 8
  store i64 %33, ptr %31, align 8
  %34 = call i64 @__remill_read_memory_64(ptr noundef %30, i64 noundef %32) #3
  store i64 %34, ptr %RBP, align 8
  store ptr %30, ptr %MEMORY, align 8
  br label %35

35:                                               ; preds = %27
  %36 = load i64, ptr %NEXT_PC, align 8
  store i64 %36, ptr %PC, align 8
  %37 = add i64 %36, 1
  store i64 %37, ptr %NEXT_PC, align 8
  %38 = load ptr, ptr %MEMORY, align 8
  %39 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 6, i32 13
  %40 = load i64, ptr %39, align 8
  %41 = call i64 @__remill_read_memory_64(ptr noundef %38, i64 noundef %40) #3
  %42 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 6, i32 33
  store i64 %41, ptr %42, align 8
  store i64 %41, ptr %NEXT_PC, align 8
  %43 = load i64, ptr %39, align 8
  %44 = add i64 %43, 8
  store i64 %44, ptr %39, align 8
  store ptr %38, ptr %MEMORY, align 8
  %45 = load i64, ptr %NEXT_PC, align 8
  store i64 %45, ptr %PC, align 8
  %46 = load ptr, ptr %MEMORY, align 8
  %47 = load i64, ptr %PC, align 8
  %48 = tail call ptr @__remill_function_return(ptr %state, i64 %47, ptr %46)
  ret ptr %48
}

; Function Attrs: noduplicate noinline nounwind optnone
declare ptr @__remill_write_memory_64(ptr noundef, i64 noundef, i64 noundef) #0

; Function Attrs: noduplicate noinline nounwind optnone
declare i64 @__remill_read_memory_64(ptr noundef, i64 noundef) #0

; Function Attrs: noduplicate noinline nounwind optnone
declare ptr @__remill_function_return(ptr noundef nonnull align 1, i64 noundef, ptr noundef) #1

define ptr @LIFTED._start(ptr noalias %state, i64 %program_counter, ptr noalias %memory) {
  %RDI = getelementptr inbounds %struct.State, ptr %state, i32 0, i32 0, i32 6, i32 11, i32 0, i32 0
  %ECX = getelementptr inbounds %struct.State, ptr %state, i32 0, i32 0, i32 6, i32 5, i32 0, i32 0
  %RCX = getelementptr inbounds %struct.State, ptr %state, i32 0, i32 0, i32 6, i32 5, i32 0, i32 0
  %R8D = getelementptr inbounds %struct.State, ptr %state, i32 0, i32 0, i32 6, i32 17, i32 0, i32 0
  %R8 = getelementptr inbounds %struct.State, ptr %state, i32 0, i32 0, i32 6, i32 17, i32 0, i32 0
  %RAX = getelementptr inbounds %struct.State, ptr %state, i32 0, i32 0, i32 6, i32 1, i32 0, i32 0
  %RSP = getelementptr inbounds %struct.State, ptr %state, i32 0, i32 0, i32 6, i32 13, i32 0, i32 0
  %RSI = getelementptr inbounds %struct.State, ptr %state, i32 0, i32 0, i32 6, i32 9, i32 0, i32 0
  %RDX = getelementptr inbounds %struct.State, ptr %state, i32 0, i32 0, i32 6, i32 7, i32 0, i32 0
  %R9 = getelementptr inbounds %struct.State, ptr %state, i32 0, i32 0, i32 6, i32 19, i32 0, i32 0
  %EBP = getelementptr inbounds %struct.State, ptr %state, i32 0, i32 0, i32 6, i32 15, i32 0, i32 0
  %RBP = getelementptr inbounds %struct.State, ptr %state, i32 0, i32 0, i32 6, i32 15, i32 0, i32 0
  %EDI = getelementptr inbounds %struct.State, ptr %state, i32 0, i32 0, i32 6, i32 11, i32 0, i32 0
  %EDX = getelementptr inbounds %struct.State, ptr %state, i32 0, i32 0, i32 6, i32 7, i32 0, i32 0
  %BRANCH_TAKEN = alloca i8, align 1
  %RETURN_PC = alloca i64, align 8
  %MONITOR = alloca i64, align 8
  %STATE = alloca ptr, align 8
  store ptr %state, ptr %STATE, align 8
  %MEMORY = alloca ptr, align 8
  store ptr %memory, ptr %MEMORY, align 8
  %NEXT_PC = alloca i64, align 8
  store i64 %program_counter, ptr %NEXT_PC, align 8
  %PC = getelementptr inbounds %struct.State, ptr %state, i32 0, i32 0, i32 6, i32 33, i32 0, i32 0
  %CSBASE = alloca i64, align 8
  store i64 0, ptr %CSBASE, align 8
  %SSBASE = alloca i64, align 8
  store i64 0, ptr %SSBASE, align 8
  %ESBASE = alloca i64, align 8
  store i64 0, ptr %ESBASE, align 8
  %DSBASE = alloca i64, align 8
  store i64 0, ptr %DSBASE, align 8
  store i64 %program_counter, ptr %NEXT_PC, align 8
  br label %1

1:                                                ; preds = %0
  %2 = load i64, ptr %NEXT_PC, align 8
  store i64 %2, ptr %PC, align 8
  %3 = add i64 %2, 4
  store i64 %3, ptr %NEXT_PC, align 8
  %4 = load i32, ptr %EDX, align 4
  %5 = zext i32 %4 to i64
  %6 = load i32, ptr %EDI, align 4
  %7 = zext i32 %6 to i64
  %8 = load ptr, ptr %MEMORY, align 8
  store ptr %8, ptr %MEMORY, align 8
  br label %9

9:                                                ; preds = %1
  %10 = load i64, ptr %NEXT_PC, align 8
  store i64 %10, ptr %PC, align 8
  %11 = add i64 %10, 2
  store i64 %11, ptr %NEXT_PC, align 8
  %12 = load i64, ptr %RBP, align 8
  %13 = load i32, ptr %EBP, align 4
  %14 = zext i32 %13 to i64
  %15 = load ptr, ptr %MEMORY, align 8
  %16 = trunc i64 %12 to i32
  %17 = xor i32 %13, %16
  %18 = zext i32 %17 to i64
  store i64 %18, ptr %RBP, align 8
  %19 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 1
  store i8 0, ptr %19, align 1
  %20 = trunc i32 %17 to i8
  %21 = call i8 @llvm.ctpop.i8(i8 %20)
  %22 = and i8 %21, 1
  %23 = xor i8 %22, 1
  %24 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 3
  store i8 %23, ptr %24, align 1
  %25 = icmp eq i32 %17, 0
  %26 = call zeroext i1 (i1, ...) @__remill_flag_computation_zero(i1 noundef zeroext %25, i32 noundef %16, i32 noundef %13, i32 noundef %17) #3
  %27 = zext i1 %26 to i8
  %28 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 7
  store i8 %27, ptr %28, align 1
  %29 = icmp slt i32 %17, 0
  %30 = call zeroext i1 (i1, ...) @__remill_flag_computation_sign(i1 noundef zeroext %29, i32 noundef %16, i32 noundef %13, i32 noundef %17) #3
  %31 = zext i1 %30 to i8
  %32 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 9
  store i8 %31, ptr %32, align 1
  %33 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 13
  store i8 0, ptr %33, align 1
  %34 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 5
  store i8 0, ptr %34, align 1
  %35 = call zeroext i8 @__remill_undefined_8() #4
  store i8 %35, ptr %34, align 1
  store ptr %15, ptr %MEMORY, align 8
  br label %36

36:                                               ; preds = %9
  %37 = load i64, ptr %NEXT_PC, align 8
  store i64 %37, ptr %PC, align 8
  %38 = add i64 %37, 3
  store i64 %38, ptr %NEXT_PC, align 8
  %39 = load i64, ptr %RDX, align 8
  %40 = load ptr, ptr %MEMORY, align 8
  store i64 %39, ptr %R9, align 8
  store ptr %40, ptr %MEMORY, align 8
  br label %41

41:                                               ; preds = %36
  %42 = load i64, ptr %NEXT_PC, align 8
  store i64 %42, ptr %PC, align 8
  %43 = add i64 %42, 1
  store i64 %43, ptr %NEXT_PC, align 8
  %44 = load ptr, ptr %MEMORY, align 8
  %45 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 6, i32 13
  %46 = load i64, ptr %45, align 8
  %47 = add i64 %46, 8
  store i64 %47, ptr %45, align 8
  %48 = call i64 @__remill_read_memory_64(ptr noundef %44, i64 noundef %46) #3
  store i64 %48, ptr %RSI, align 8
  store ptr %44, ptr %MEMORY, align 8
  br label %49

49:                                               ; preds = %41
  %50 = load i64, ptr %NEXT_PC, align 8
  store i64 %50, ptr %PC, align 8
  %51 = add i64 %50, 3
  store i64 %51, ptr %NEXT_PC, align 8
  %52 = load i64, ptr %RSP, align 8
  %53 = load ptr, ptr %MEMORY, align 8
  store i64 %52, ptr %RDX, align 8
  store ptr %53, ptr %MEMORY, align 8
  br label %54

54:                                               ; preds = %49
  %55 = load i64, ptr %NEXT_PC, align 8
  store i64 %55, ptr %PC, align 8
  %56 = add i64 %55, 4
  store i64 %56, ptr %NEXT_PC, align 8
  %57 = load i64, ptr %RSP, align 8
  %58 = load ptr, ptr %MEMORY, align 8
  %59 = and i64 -16, %57
  store i64 %59, ptr %RSP, align 8
  %60 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 1
  store i8 0, ptr %60, align 1
  %61 = trunc i64 %59 to i8
  %62 = call i8 @llvm.ctpop.i8(i8 %61)
  %63 = and i8 %62, 1
  %64 = xor i8 %63, 1
  %65 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 3
  store i8 %64, ptr %65, align 1
  %66 = icmp eq i64 %59, 0
  %67 = call zeroext i1 (i1, ...) @__remill_flag_computation_zero(i1 noundef zeroext %66, i64 noundef %57, i64 noundef -16, i64 noundef %59) #3
  %68 = zext i1 %67 to i8
  %69 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 7
  store i8 %68, ptr %69, align 1
  %70 = icmp slt i64 %59, 0
  %71 = call zeroext i1 (i1, ...) @__remill_flag_computation_sign(i1 noundef zeroext %70, i64 noundef %57, i64 noundef -16, i64 noundef %59) #3
  %72 = zext i1 %71 to i8
  %73 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 9
  store i8 %72, ptr %73, align 1
  %74 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 13
  store i8 0, ptr %74, align 1
  %75 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 5
  store i8 0, ptr %75, align 1
  store ptr %58, ptr %MEMORY, align 8
  br label %76

76:                                               ; preds = %54
  %77 = load i64, ptr %NEXT_PC, align 8
  store i64 %77, ptr %PC, align 8
  %78 = add i64 %77, 1
  store i64 %78, ptr %NEXT_PC, align 8
  %79 = load i64, ptr %RAX, align 8
  %80 = load ptr, ptr %MEMORY, align 8
  %81 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 6, i32 13
  %82 = load i64, ptr %81, align 8
  %83 = add i64 %82, -8
  %84 = call ptr @__remill_write_memory_64(ptr noundef %80, i64 noundef %83, i64 noundef %79) #3
  store i64 %83, ptr %81, align 8
  store ptr %84, ptr %MEMORY, align 8
  br label %85

85:                                               ; preds = %76
  %86 = load i64, ptr %NEXT_PC, align 8
  store i64 %86, ptr %PC, align 8
  %87 = add i64 %86, 1
  store i64 %87, ptr %NEXT_PC, align 8
  %88 = load i64, ptr %RSP, align 8
  %89 = load ptr, ptr %MEMORY, align 8
  %90 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 6, i32 13
  %91 = load i64, ptr %90, align 8
  %92 = add i64 %91, -8
  %93 = call ptr @__remill_write_memory_64(ptr noundef %89, i64 noundef %92, i64 noundef %88) #3
  store i64 %92, ptr %90, align 8
  store ptr %93, ptr %MEMORY, align 8
  br label %94

94:                                               ; preds = %85
  %95 = load i64, ptr %NEXT_PC, align 8
  store i64 %95, ptr %PC, align 8
  %96 = add i64 %95, 3
  store i64 %96, ptr %NEXT_PC, align 8
  %97 = load i64, ptr %R8, align 8
  %98 = load i32, ptr %R8D, align 4
  %99 = zext i32 %98 to i64
  %100 = load ptr, ptr %MEMORY, align 8
  %101 = trunc i64 %97 to i32
  %102 = xor i32 %98, %101
  %103 = zext i32 %102 to i64
  store i64 %103, ptr %R8, align 8
  %104 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 1
  store i8 0, ptr %104, align 1
  %105 = trunc i32 %102 to i8
  %106 = call i8 @llvm.ctpop.i8(i8 %105)
  %107 = and i8 %106, 1
  %108 = xor i8 %107, 1
  %109 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 3
  store i8 %108, ptr %109, align 1
  %110 = icmp eq i32 %102, 0
  %111 = call zeroext i1 (i1, ...) @__remill_flag_computation_zero(i1 noundef zeroext %110, i32 noundef %101, i32 noundef %98, i32 noundef %102) #3
  %112 = zext i1 %111 to i8
  %113 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 7
  store i8 %112, ptr %113, align 1
  %114 = icmp slt i32 %102, 0
  %115 = call zeroext i1 (i1, ...) @__remill_flag_computation_sign(i1 noundef zeroext %114, i32 noundef %101, i32 noundef %98, i32 noundef %102) #3
  %116 = zext i1 %115 to i8
  %117 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 9
  store i8 %116, ptr %117, align 1
  %118 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 13
  store i8 0, ptr %118, align 1
  %119 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 5
  store i8 0, ptr %119, align 1
  %120 = call zeroext i8 @__remill_undefined_8() #4
  store i8 %120, ptr %119, align 1
  store ptr %100, ptr %MEMORY, align 8
  br label %121

121:                                              ; preds = %94
  %122 = load i64, ptr %NEXT_PC, align 8
  store i64 %122, ptr %PC, align 8
  %123 = add i64 %122, 2
  store i64 %123, ptr %NEXT_PC, align 8
  %124 = load i64, ptr %RCX, align 8
  %125 = load i32, ptr %ECX, align 4
  %126 = zext i32 %125 to i64
  %127 = load ptr, ptr %MEMORY, align 8
  %128 = trunc i64 %124 to i32
  %129 = xor i32 %125, %128
  %130 = zext i32 %129 to i64
  store i64 %130, ptr %RCX, align 8
  %131 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 1
  store i8 0, ptr %131, align 1
  %132 = trunc i32 %129 to i8
  %133 = call i8 @llvm.ctpop.i8(i8 %132)
  %134 = and i8 %133, 1
  %135 = xor i8 %134, 1
  %136 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 3
  store i8 %135, ptr %136, align 1
  %137 = icmp eq i32 %129, 0
  %138 = call zeroext i1 (i1, ...) @__remill_flag_computation_zero(i1 noundef zeroext %137, i32 noundef %128, i32 noundef %125, i32 noundef %129) #3
  %139 = zext i1 %138 to i8
  %140 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 7
  store i8 %139, ptr %140, align 1
  %141 = icmp slt i32 %129, 0
  %142 = call zeroext i1 (i1, ...) @__remill_flag_computation_sign(i1 noundef zeroext %141, i32 noundef %128, i32 noundef %125, i32 noundef %129) #3
  %143 = zext i1 %142 to i8
  %144 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 9
  store i8 %143, ptr %144, align 1
  %145 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 13
  store i8 0, ptr %145, align 1
  %146 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 2, i32 5
  store i8 0, ptr %146, align 1
  %147 = call zeroext i8 @__remill_undefined_8() #4
  store i8 %147, ptr %146, align 1
  store ptr %127, ptr %MEMORY, align 8
  br label %148

148:                                              ; preds = %121
  %149 = load i64, ptr %NEXT_PC, align 8
  store i64 %149, ptr %PC, align 8
  %150 = add i64 %149, 7
  store i64 %150, ptr %NEXT_PC, align 8
  %151 = load i64, ptr %NEXT_PC, align 8
  %152 = add i64 %151, 202
  %153 = load ptr, ptr %MEMORY, align 8
  store i64 %152, ptr %RDI, align 8
  store ptr %153, ptr %MEMORY, align 8
  br label %154

154:                                              ; preds = %148
  %155 = load i64, ptr %NEXT_PC, align 8
  store i64 %155, ptr %PC, align 8
  %156 = add i64 %155, 6
  store i64 %156, ptr %NEXT_PC, align 8
  %157 = load i64, ptr %NEXT_PC, align 8
  %158 = add i64 %157, 12147
  %159 = load i64, ptr %NEXT_PC, align 8
  %160 = load ptr, ptr %MEMORY, align 8
  %161 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 6, i32 13
  %162 = load i64, ptr %161, align 8
  %163 = add i64 %162, -8
  %164 = call i64 @__remill_read_memory_64(ptr noundef %160, i64 noundef %158) #3
  %165 = call ptr @__remill_write_memory_64(ptr noundef %160, i64 noundef %163, i64 noundef %159) #3
  store i64 %163, ptr %161, align 8
  %166 = getelementptr inbounds %struct.X86State, ptr %state, i64 0, i32 6, i32 33
  store i64 %164, ptr %166, align 8
  store i64 %164, ptr %NEXT_PC, align 8
  store i64 %159, ptr %RETURN_PC, align 8
  store ptr %165, ptr %MEMORY, align 8
  %167 = load ptr, ptr %MEMORY, align 8
  %168 = load i64, ptr %PC, align 8
  %169 = call ptr @__remill_function_call(ptr %state, i64 %168, ptr %167)
  br label %170

170:                                              ; preds = %154
  %171 = load i64, ptr %RETURN_PC, align 8
  store i64 %171, ptr %NEXT_PC, align 8
  br label %172

172:                                              ; preds = %170
  %173 = load i64, ptr %NEXT_PC, align 8
  store i64 %173, ptr %PC, align 8
  %174 = add i64 %173, 1
  store i64 %174, ptr %NEXT_PC, align 8
  %175 = load ptr, ptr %MEMORY, align 8
  store ptr %175, ptr %MEMORY, align 8
  %176 = load i64, ptr %NEXT_PC, align 8
  store i64 %176, ptr %PC, align 8
  %177 = load ptr, ptr %MEMORY, align 8
  %178 = load i64, ptr %PC, align 8
  %179 = tail call ptr @__remill_error(ptr %state, i64 %178, ptr %177)
  ret ptr %179
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i8 @llvm.ctpop.i8(i8) #2

; Function Attrs: noduplicate noinline nounwind optnone
declare zeroext i1 @__remill_flag_computation_zero(i1 noundef zeroext, ...) #0

; Function Attrs: noduplicate noinline nounwind optnone
declare zeroext i1 @__remill_flag_computation_sign(i1 noundef zeroext, ...) #0

; Function Attrs: noduplicate noinline nounwind optnone
declare zeroext i8 @__remill_undefined_8() #0

; Function Attrs: noduplicate noinline nounwind optnone
declare ptr @__remill_function_call(ptr noundef nonnull align 1, i64 noundef, ptr noundef) #1

; Function Attrs: noduplicate noinline nounwind optnone
declare ptr @__remill_error(ptr noundef nonnull align 16 dereferenceable(3504), i64 noundef, ptr noundef) #0

define i32 @main() {
  %1 = call ptr @LIFTED._start(ptr @LIFTED.STATE, i64 4160, ptr null)
  %2 = load i32, ptr getelementptr inbounds (%struct.State, ptr @LIFTED.STATE, i32 0, i32 0, i32 6, i32 1, i32 0, i32 0), align 4
  ret i32 %2
}

attributes #0 = { noduplicate noinline nounwind optnone "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "tune-cpu"="generic" }
attributes #1 = { noduplicate noinline nounwind optnone "frame-pointer"="all" "no-builtins" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "tune-cpu"="generic" }
attributes #2 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { nobuiltin nounwind "no-builtins" }
attributes #4 = { alwaysinline nobuiltin nounwind "no-builtins" }
