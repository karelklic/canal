; ModuleID = 'string_example.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@.str = private unnamed_addr constant [11 x i8] c"/etc/hosts\00", align 1
@.str1 = private unnamed_addr constant [2 x i8] c"r\00", align 1
@.str2 = private unnamed_addr constant [12 x i8] c"blocks == 1\00", align 1
@.str3 = private unnamed_addr constant [17 x i8] c"string_example.c\00", align 1
@__PRETTY_FUNCTION__.main = private unnamed_addr constant [23 x i8] c"int main(int, char **)\00", align 1
@stdout = external global %struct._IO_FILE*
@.str4 = private unnamed_addr constant [37 x i8] c"stack and heap file contents differ\0A\00", align 1
@.str5 = private unnamed_addr constant [16 x i8] c"file size: %lu\0A\00", align 1
@.str6 = private unnamed_addr constant [18 x i8] c"heap length: %lu\0A\00", align 1
@.str7 = private unnamed_addr constant [19 x i8] c"stack length: %lu\0A\00", align 1
@.str8 = private unnamed_addr constant [35 x i8] c"stack and heap files are the same\0A\00", align 1
@.str9 = private unnamed_addr constant [8 x i8] c".second\00", align 1
@.str10 = private unnamed_addr constant [19 x i8] c"second path is %s\0A\00", align 1

define i32 @main(i32 %argc, i8** %argv) nounwind uwtable {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i8**, align 8
  %path1 = alloca i8*, align 8
  %fp = alloca %struct._IO_FILE*, align 8
  %size = alloca i64, align 8
  %contents_heap = alloca i8*, align 8
  %blocks = alloca i64, align 8
  %4 = alloca i8*
  %path2 = alloca i8*, align 8
  %5 = alloca i32
  store i32 0, i32* %1
  store i32 %argc, i32* %2, align 4
  call void @llvm.dbg.declare(metadata !{i32* %2}, metadata !12), !dbg !13
  store i8** %argv, i8*** %3, align 8
  call void @llvm.dbg.declare(metadata !{i8*** %3}, metadata !14), !dbg !18
  call void @llvm.dbg.declare(metadata !{i8** %path1}, metadata !19), !dbg !21
  store i8* getelementptr inbounds ([11 x i8]* @.str, i32 0, i32 0), i8** %path1, align 8, !dbg !22
  call void @llvm.dbg.declare(metadata !{%struct._IO_FILE** %fp}, metadata !23), !dbg !79
  %6 = load i8** %path1, align 8, !dbg !80
  %7 = call %struct._IO_FILE* @fopen(i8* %6, i8* getelementptr inbounds ([2 x i8]* @.str1, i32 0, i32 0)), !dbg !80
  store %struct._IO_FILE* %7, %struct._IO_FILE** %fp, align 8, !dbg !80
  %8 = load %struct._IO_FILE** %fp, align 8, !dbg !81
  %9 = call i32 @fseek(%struct._IO_FILE* %8, i64 0, i32 2), !dbg !81
  call void @llvm.dbg.declare(metadata !{i64* %size}, metadata !82), !dbg !83
  %10 = load %struct._IO_FILE** %fp, align 8, !dbg !84
  %11 = call i64 @ftell(%struct._IO_FILE* %10), !dbg !84
  store i64 %11, i64* %size, align 8, !dbg !84
  %12 = load %struct._IO_FILE** %fp, align 8, !dbg !85
  %13 = call i32 @fseek(%struct._IO_FILE* %12, i64 0, i32 0), !dbg !85
  call void @llvm.dbg.declare(metadata !{i8** %contents_heap}, metadata !86), !dbg !87
  %14 = load i64* %size, align 8, !dbg !88
  %15 = add i64 %14, 1, !dbg !88
  %16 = call noalias i8* @malloc(i64 %15) nounwind, !dbg !88
  store i8* %16, i8** %contents_heap, align 8, !dbg !88
  call void @llvm.dbg.declare(metadata !{i64* %blocks}, metadata !89), !dbg !90
  %17 = load i8** %contents_heap, align 8, !dbg !91
  %18 = load i64* %size, align 8, !dbg !91
  %19 = load %struct._IO_FILE** %fp, align 8, !dbg !91
  %20 = call i64 @fread(i8* %17, i64 %18, i64 1, %struct._IO_FILE* %19), !dbg !91
  store i64 %20, i64* %blocks, align 8, !dbg !91
  %21 = load i64* %size, align 8, !dbg !92
  %22 = load i8** %contents_heap, align 8, !dbg !92
  %23 = getelementptr inbounds i8* %22, i64 %21, !dbg !92
  store i8 0, i8* %23, !dbg !92
  %24 = load i64* %blocks, align 8, !dbg !93
  %25 = icmp eq i64 %24, 1, !dbg !93
  br i1 %25, label %26, label %27, !dbg !93

; <label>:26                                      ; preds = %0
  br label %29, !dbg !93

; <label>:27                                      ; preds = %0
  call void @__assert_fail(i8* getelementptr inbounds ([12 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([17 x i8]* @.str3, i32 0, i32 0), i32 26, i8* getelementptr inbounds ([23 x i8]* @__PRETTY_FUNCTION__.main, i32 0, i32 0)) noreturn nounwind, !dbg !93
  unreachable, !dbg !93
                                                  ; No predecessors!
  br label %29, !dbg !93

; <label>:29                                      ; preds = %28, %26
  %30 = load i64* %size, align 8, !dbg !94
  %31 = add i64 %30, 1, !dbg !94
  %32 = call i8* @llvm.stacksave(), !dbg !94
  store i8* %32, i8** %4, !dbg !94
  %33 = alloca i8, i64 %31, align 16, !dbg !94
  call void @llvm.dbg.declare(metadata !{i8* %33}, metadata !95), !dbg !99
  %34 = load %struct._IO_FILE** %fp, align 8, !dbg !100
  %35 = call i32 @fseek(%struct._IO_FILE* %34, i64 0, i32 0), !dbg !100
  %36 = load i64* %size, align 8, !dbg !101
  %37 = load %struct._IO_FILE** %fp, align 8, !dbg !101
  %38 = call i64 @fread(i8* %33, i64 %36, i64 1, %struct._IO_FILE* %37), !dbg !101
  store i64 %38, i64* %blocks, align 8, !dbg !101
  %39 = load i64* %size, align 8, !dbg !102
  %40 = getelementptr inbounds i8* %33, i64 %39, !dbg !102
  store i8 0, i8* %40, !dbg !102
  %41 = load i64* %blocks, align 8, !dbg !103
  %42 = icmp eq i64 %41, 1, !dbg !103
  br i1 %42, label %43, label %44, !dbg !103

; <label>:43                                      ; preds = %29
  br label %46, !dbg !103

; <label>:44                                      ; preds = %29
  call void @__assert_fail(i8* getelementptr inbounds ([12 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([17 x i8]* @.str3, i32 0, i32 0), i32 32, i8* getelementptr inbounds ([23 x i8]* @__PRETTY_FUNCTION__.main, i32 0, i32 0)) noreturn nounwind, !dbg !103
  unreachable, !dbg !103
                                                  ; No predecessors!
  br label %46, !dbg !103

; <label>:46                                      ; preds = %45, %43
  %47 = load %struct._IO_FILE** %fp, align 8, !dbg !104
  %48 = call i32 @fclose(%struct._IO_FILE* %47), !dbg !104
  %49 = load i8** %contents_heap, align 8, !dbg !105
  %50 = call i32 @strcmp(i8* %49, i8* %33) nounwind readonly, !dbg !105
  %51 = icmp ne i32 %50, 0, !dbg !105
  br i1 %51, label %52, label %65, !dbg !105

; <label>:52                                      ; preds = %46
  %53 = load %struct._IO_FILE** @stdout, align 8, !dbg !106
  %54 = call i32 (%struct._IO_FILE*, i8*, ...)* @fprintf(%struct._IO_FILE* %53, i8* getelementptr inbounds ([37 x i8]* @.str4, i32 0, i32 0)), !dbg !106
  %55 = load %struct._IO_FILE** @stdout, align 8, !dbg !108
  %56 = load i64* %size, align 8, !dbg !108
  %57 = call i32 (%struct._IO_FILE*, i8*, ...)* @fprintf(%struct._IO_FILE* %55, i8* getelementptr inbounds ([16 x i8]* @.str5, i32 0, i32 0), i64 %56), !dbg !108
  %58 = load %struct._IO_FILE** @stdout, align 8, !dbg !109
  %59 = load i8** %contents_heap, align 8, !dbg !110
  %60 = call i64 @strlen(i8* %59) nounwind readonly, !dbg !110
  %61 = call i32 (%struct._IO_FILE*, i8*, ...)* @fprintf(%struct._IO_FILE* %58, i8* getelementptr inbounds ([18 x i8]* @.str6, i32 0, i32 0), i64 %60), !dbg !110
  %62 = load %struct._IO_FILE** @stdout, align 8, !dbg !111
  %63 = call i64 @strlen(i8* %33) nounwind readonly, !dbg !112
  %64 = call i32 (%struct._IO_FILE*, i8*, ...)* @fprintf(%struct._IO_FILE* %62, i8* getelementptr inbounds ([19 x i8]* @.str7, i32 0, i32 0), i64 %63), !dbg !112
  br label %67, !dbg !113

; <label>:65                                      ; preds = %46
  %66 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([35 x i8]* @.str8, i32 0, i32 0)), !dbg !114
  br label %67

; <label>:67                                      ; preds = %65, %52
  %68 = load i8** %contents_heap, align 8, !dbg !115
  call void @free(i8* %68) nounwind, !dbg !115
  call void @llvm.dbg.declare(metadata !{i8** %path2}, metadata !116), !dbg !117
  %69 = load i8** %path1, align 8, !dbg !118
  %70 = call i64 @strlen(i8* %69) nounwind readonly, !dbg !118
  %71 = add i64 %70, 7, !dbg !119
  %72 = add i64 %71, 1, !dbg !119
  %73 = call noalias i8* @malloc(i64 %72) nounwind, !dbg !119
  store i8* %73, i8** %path2, align 8, !dbg !119
  %74 = load i8** %path2, align 8, !dbg !120
  %75 = load i8** %path1, align 8, !dbg !120
  %76 = call i8* @strcpy(i8* %74, i8* %75) nounwind, !dbg !120
  %77 = load i8** %path2, align 8, !dbg !121
  %78 = load i8** %path1, align 8, !dbg !122
  %79 = call i64 @strlen(i8* %78) nounwind readonly, !dbg !122
  %80 = getelementptr inbounds i8* %77, i64 %79, !dbg !122
  %81 = call i8* @strcpy(i8* %80, i8* getelementptr inbounds ([8 x i8]* @.str9, i32 0, i32 0)) nounwind, !dbg !122
  %82 = load i8** %path2, align 8, !dbg !123
  %83 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([19 x i8]* @.str10, i32 0, i32 0), i8* %82), !dbg !123
  %84 = load i8** %path2, align 8, !dbg !124
  call void @free(i8* %84) nounwind, !dbg !124
  store i32 0, i32* %1, !dbg !125
  store i32 1, i32* %5
  %85 = load i8** %4, !dbg !126
  call void @llvm.stackrestore(i8* %85), !dbg !126
  %86 = load i32* %1, !dbg !127
  ret i32 %86, !dbg !127
}

declare void @llvm.dbg.declare(metadata, metadata) nounwind readnone

declare %struct._IO_FILE* @fopen(i8*, i8*)

declare i32 @fseek(%struct._IO_FILE*, i64, i32)

declare i64 @ftell(%struct._IO_FILE*)

declare noalias i8* @malloc(i64) nounwind

declare i64 @fread(i8*, i64, i64, %struct._IO_FILE*)

declare void @__assert_fail(i8*, i8*, i32, i8*) noreturn nounwind

declare i8* @llvm.stacksave() nounwind

declare i32 @fclose(%struct._IO_FILE*)

declare i32 @strcmp(i8*, i8*) nounwind readonly

declare i32 @fprintf(%struct._IO_FILE*, i8*, ...)

declare i64 @strlen(i8*) nounwind readonly

declare i32 @printf(i8*, ...)

declare void @free(i8*) nounwind

declare i8* @strcpy(i8*, i8*) nounwind

declare void @llvm.stackrestore(i8*) nounwind

!llvm.dbg.cu = !{!0}

!0 = metadata !{i32 720913, i32 0, i32 12, metadata !"string_example.c", metadata !"/home/karel/devel/faf/llvm", metadata !"clang version 3.0 (tags/RELEASE_30/final)", i1 true, i1 false, metadata !"", i32 0, metadata !1, metadata !1, metadata !3, metadata !1} ; [ DW_TAG_compile_unit ]
!1 = metadata !{metadata !2}
!2 = metadata !{i32 0}
!3 = metadata !{metadata !4}
!4 = metadata !{metadata !5}
!5 = metadata !{i32 720942, i32 0, metadata !6, metadata !"main", metadata !"main", metadata !"", metadata !6, i32 14, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 256, i1 false, i32 (i32, i8**)* @main, null, null, metadata !10} ; [ DW_TAG_subprogram ]
!6 = metadata !{i32 720937, metadata !"string_example.c", metadata !"/home/karel/devel/faf/llvm", null} ; [ DW_TAG_file_type ]
!7 = metadata !{i32 720917, i32 0, metadata !"", i32 0, i32 0, i64 0, i64 0, i32 0, i32 0, i32 0, metadata !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ]
!8 = metadata !{metadata !9}
!9 = metadata !{i32 720932, null, metadata !"int", null, i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ]
!10 = metadata !{metadata !11}
!11 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!12 = metadata !{i32 721153, metadata !5, metadata !"argc", metadata !6, i32 16777229, metadata !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ]
!13 = metadata !{i32 13, i32 14, metadata !5, null}
!14 = metadata !{i32 721153, metadata !5, metadata !"argv", metadata !6, i32 33554445, metadata !15, i32 0, i32 0} ; [ DW_TAG_arg_variable ]
!15 = metadata !{i32 720911, null, metadata !"", null, i32 0, i64 64, i64 64, i64 0, i32 0, metadata !16} ; [ DW_TAG_pointer_type ]
!16 = metadata !{i32 720911, null, metadata !"", null, i32 0, i64 64, i64 64, i64 0, i32 0, metadata !17} ; [ DW_TAG_pointer_type ]
!17 = metadata !{i32 720932, null, metadata !"char", null, i32 0, i64 8, i64 8, i64 0, i32 0, i32 6} ; [ DW_TAG_base_type ]
!18 = metadata !{i32 13, i32 27, metadata !5, null}
!19 = metadata !{i32 721152, metadata !20, metadata !"path1", metadata !6, i32 15, metadata !16, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!20 = metadata !{i32 720907, metadata !5, i32 14, i32 1, metadata !6, i32 0} ; [ DW_TAG_lexical_block ]
!21 = metadata !{i32 15, i32 9, metadata !20, null}
!22 = metadata !{i32 15, i32 29, metadata !20, null}
!23 = metadata !{i32 721152, metadata !20, metadata !"fp", metadata !6, i32 16, metadata !24, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!24 = metadata !{i32 720911, null, metadata !"", null, i32 0, i64 64, i64 64, i64 0, i32 0, metadata !25} ; [ DW_TAG_pointer_type ]
!25 = metadata !{i32 720918, null, metadata !"FILE", metadata !6, i32 49, i64 0, i64 0, i64 0, i32 0, metadata !26} ; [ DW_TAG_typedef ]
!26 = metadata !{i32 720915, null, metadata !"_IO_FILE", metadata !27, i32 273, i64 1728, i64 64, i32 0, i32 0, i32 0, metadata !28, i32 0, i32 0} ; [ DW_TAG_structure_type ]
!27 = metadata !{i32 720937, metadata !"/usr/include/libio.h", metadata !"/home/karel/devel/faf/llvm", null} ; [ DW_TAG_file_type ]
!28 = metadata !{metadata !29, metadata !30, metadata !31, metadata !32, metadata !33, metadata !34, metadata !35, metadata !36, metadata !37, metadata !38, metadata !39, metadata !40, metadata !41, metadata !49, metadata !50, metadata !51, metadata !52, metadata !55, metadata !57, metadata !59, metadata !63, metadata !65, metadata !67, metadata !68, metadata !69, metadata !70, metadata !71, metadata !74, metadata !75}
!29 = metadata !{i32 720909, metadata !26, metadata !"_flags", metadata !27, i32 274, i64 32, i64 32, i64 0, i32 0, metadata !9} ; [ DW_TAG_member ]
!30 = metadata !{i32 720909, metadata !26, metadata !"_IO_read_ptr", metadata !27, i32 279, i64 64, i64 64, i64 64, i32 0, metadata !16} ; [ DW_TAG_member ]
!31 = metadata !{i32 720909, metadata !26, metadata !"_IO_read_end", metadata !27, i32 280, i64 64, i64 64, i64 128, i32 0, metadata !16} ; [ DW_TAG_member ]
!32 = metadata !{i32 720909, metadata !26, metadata !"_IO_read_base", metadata !27, i32 281, i64 64, i64 64, i64 192, i32 0, metadata !16} ; [ DW_TAG_member ]
!33 = metadata !{i32 720909, metadata !26, metadata !"_IO_write_base", metadata !27, i32 282, i64 64, i64 64, i64 256, i32 0, metadata !16} ; [ DW_TAG_member ]
!34 = metadata !{i32 720909, metadata !26, metadata !"_IO_write_ptr", metadata !27, i32 283, i64 64, i64 64, i64 320, i32 0, metadata !16} ; [ DW_TAG_member ]
!35 = metadata !{i32 720909, metadata !26, metadata !"_IO_write_end", metadata !27, i32 284, i64 64, i64 64, i64 384, i32 0, metadata !16} ; [ DW_TAG_member ]
!36 = metadata !{i32 720909, metadata !26, metadata !"_IO_buf_base", metadata !27, i32 285, i64 64, i64 64, i64 448, i32 0, metadata !16} ; [ DW_TAG_member ]
!37 = metadata !{i32 720909, metadata !26, metadata !"_IO_buf_end", metadata !27, i32 286, i64 64, i64 64, i64 512, i32 0, metadata !16} ; [ DW_TAG_member ]
!38 = metadata !{i32 720909, metadata !26, metadata !"_IO_save_base", metadata !27, i32 288, i64 64, i64 64, i64 576, i32 0, metadata !16} ; [ DW_TAG_member ]
!39 = metadata !{i32 720909, metadata !26, metadata !"_IO_backup_base", metadata !27, i32 289, i64 64, i64 64, i64 640, i32 0, metadata !16} ; [ DW_TAG_member ]
!40 = metadata !{i32 720909, metadata !26, metadata !"_IO_save_end", metadata !27, i32 290, i64 64, i64 64, i64 704, i32 0, metadata !16} ; [ DW_TAG_member ]
!41 = metadata !{i32 720909, metadata !26, metadata !"_markers", metadata !27, i32 292, i64 64, i64 64, i64 768, i32 0, metadata !42} ; [ DW_TAG_member ]
!42 = metadata !{i32 720911, null, metadata !"", null, i32 0, i64 64, i64 64, i64 0, i32 0, metadata !43} ; [ DW_TAG_pointer_type ]
!43 = metadata !{i32 720915, null, metadata !"_IO_marker", metadata !27, i32 188, i64 192, i64 64, i32 0, i32 0, i32 0, metadata !44, i32 0, i32 0} ; [ DW_TAG_structure_type ]
!44 = metadata !{metadata !45, metadata !46, metadata !48}
!45 = metadata !{i32 720909, metadata !43, metadata !"_next", metadata !27, i32 189, i64 64, i64 64, i64 0, i32 0, metadata !42} ; [ DW_TAG_member ]
!46 = metadata !{i32 720909, metadata !43, metadata !"_sbuf", metadata !27, i32 190, i64 64, i64 64, i64 64, i32 0, metadata !47} ; [ DW_TAG_member ]
!47 = metadata !{i32 720911, null, metadata !"", null, i32 0, i64 64, i64 64, i64 0, i32 0, metadata !26} ; [ DW_TAG_pointer_type ]
!48 = metadata !{i32 720909, metadata !43, metadata !"_pos", metadata !27, i32 194, i64 32, i64 32, i64 128, i32 0, metadata !9} ; [ DW_TAG_member ]
!49 = metadata !{i32 720909, metadata !26, metadata !"_chain", metadata !27, i32 294, i64 64, i64 64, i64 832, i32 0, metadata !47} ; [ DW_TAG_member ]
!50 = metadata !{i32 720909, metadata !26, metadata !"_fileno", metadata !27, i32 296, i64 32, i64 32, i64 896, i32 0, metadata !9} ; [ DW_TAG_member ]
!51 = metadata !{i32 720909, metadata !26, metadata !"_flags2", metadata !27, i32 300, i64 32, i64 32, i64 928, i32 0, metadata !9} ; [ DW_TAG_member ]
!52 = metadata !{i32 720909, metadata !26, metadata !"_old_offset", metadata !27, i32 302, i64 64, i64 64, i64 960, i32 0, metadata !53} ; [ DW_TAG_member ]
!53 = metadata !{i32 720918, null, metadata !"__off_t", metadata !27, i32 141, i64 0, i64 0, i64 0, i32 0, metadata !54} ; [ DW_TAG_typedef ]
!54 = metadata !{i32 720932, null, metadata !"long int", null, i32 0, i64 64, i64 64, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ]
!55 = metadata !{i32 720909, metadata !26, metadata !"_cur_column", metadata !27, i32 306, i64 16, i64 16, i64 1024, i32 0, metadata !56} ; [ DW_TAG_member ]
!56 = metadata !{i32 720932, null, metadata !"unsigned short", null, i32 0, i64 16, i64 16, i64 0, i32 0, i32 7} ; [ DW_TAG_base_type ]
!57 = metadata !{i32 720909, metadata !26, metadata !"_vtable_offset", metadata !27, i32 307, i64 8, i64 8, i64 1040, i32 0, metadata !58} ; [ DW_TAG_member ]
!58 = metadata !{i32 720932, null, metadata !"signed char", null, i32 0, i64 8, i64 8, i64 0, i32 0, i32 6} ; [ DW_TAG_base_type ]
!59 = metadata !{i32 720909, metadata !26, metadata !"_shortbuf", metadata !27, i32 308, i64 8, i64 8, i64 1048, i32 0, metadata !60} ; [ DW_TAG_member ]
!60 = metadata !{i32 720897, null, metadata !"", null, i32 0, i64 8, i64 8, i32 0, i32 0, metadata !17, metadata !61, i32 0, i32 0} ; [ DW_TAG_array_type ]
!61 = metadata !{metadata !62}
!62 = metadata !{i32 720929, i64 0, i64 0}        ; [ DW_TAG_subrange_type ]
!63 = metadata !{i32 720909, metadata !26, metadata !"_lock", metadata !27, i32 312, i64 64, i64 64, i64 1088, i32 0, metadata !64} ; [ DW_TAG_member ]
!64 = metadata !{i32 720911, null, metadata !"", null, i32 0, i64 64, i64 64, i64 0, i32 0, null} ; [ DW_TAG_pointer_type ]
!65 = metadata !{i32 720909, metadata !26, metadata !"_offset", metadata !27, i32 321, i64 64, i64 64, i64 1152, i32 0, metadata !66} ; [ DW_TAG_member ]
!66 = metadata !{i32 720918, null, metadata !"__off64_t", metadata !27, i32 142, i64 0, i64 0, i64 0, i32 0, metadata !54} ; [ DW_TAG_typedef ]
!67 = metadata !{i32 720909, metadata !26, metadata !"__pad1", metadata !27, i32 330, i64 64, i64 64, i64 1216, i32 0, metadata !64} ; [ DW_TAG_member ]
!68 = metadata !{i32 720909, metadata !26, metadata !"__pad2", metadata !27, i32 331, i64 64, i64 64, i64 1280, i32 0, metadata !64} ; [ DW_TAG_member ]
!69 = metadata !{i32 720909, metadata !26, metadata !"__pad3", metadata !27, i32 332, i64 64, i64 64, i64 1344, i32 0, metadata !64} ; [ DW_TAG_member ]
!70 = metadata !{i32 720909, metadata !26, metadata !"__pad4", metadata !27, i32 333, i64 64, i64 64, i64 1408, i32 0, metadata !64} ; [ DW_TAG_member ]
!71 = metadata !{i32 720909, metadata !26, metadata !"__pad5", metadata !27, i32 334, i64 64, i64 64, i64 1472, i32 0, metadata !72} ; [ DW_TAG_member ]
!72 = metadata !{i32 720918, null, metadata !"size_t", metadata !27, i32 35, i64 0, i64 0, i64 0, i32 0, metadata !73} ; [ DW_TAG_typedef ]
!73 = metadata !{i32 720932, null, metadata !"long unsigned int", null, i32 0, i64 64, i64 64, i64 0, i32 0, i32 7} ; [ DW_TAG_base_type ]
!74 = metadata !{i32 720909, metadata !26, metadata !"_mode", metadata !27, i32 336, i64 32, i64 32, i64 1536, i32 0, metadata !9} ; [ DW_TAG_member ]
!75 = metadata !{i32 720909, metadata !26, metadata !"_unused2", metadata !27, i32 338, i64 160, i64 8, i64 1568, i32 0, metadata !76} ; [ DW_TAG_member ]
!76 = metadata !{i32 720897, null, metadata !"", null, i32 0, i64 160, i64 8, i32 0, i32 0, metadata !17, metadata !77, i32 0, i32 0} ; [ DW_TAG_array_type ]
!77 = metadata !{metadata !78}
!78 = metadata !{i32 720929, i64 0, i64 19}       ; [ DW_TAG_subrange_type ]
!79 = metadata !{i32 16, i32 9, metadata !20, null}
!80 = metadata !{i32 16, i32 14, metadata !20, null}
!81 = metadata !{i32 19, i32 3, metadata !20, null}
!82 = metadata !{i32 721152, metadata !20, metadata !"size", metadata !6, i32 20, metadata !72, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!83 = metadata !{i32 20, i32 10, metadata !20, null}
!84 = metadata !{i32 20, i32 17, metadata !20, null}
!85 = metadata !{i32 21, i32 3, metadata !20, null}
!86 = metadata !{i32 721152, metadata !20, metadata !"contents_heap", metadata !6, i32 23, metadata !16, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!87 = metadata !{i32 23, i32 9, metadata !20, null}
!88 = metadata !{i32 23, i32 25, metadata !20, null}
!89 = metadata !{i32 721152, metadata !20, metadata !"blocks", metadata !6, i32 24, metadata !72, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!90 = metadata !{i32 24, i32 10, metadata !20, null}
!91 = metadata !{i32 24, i32 19, metadata !20, null}
!92 = metadata !{i32 25, i32 3, metadata !20, null}
!93 = metadata !{i32 26, i32 3, metadata !20, null}
!94 = metadata !{i32 28, i32 32, metadata !20, null}
!95 = metadata !{i32 721152, metadata !20, metadata !"contents_stack", metadata !6, i32 28, metadata !96, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!96 = metadata !{i32 720897, null, metadata !"", null, i32 0, i64 0, i64 8, i32 0, i32 0, metadata !17, metadata !97, i32 0, i32 0} ; [ DW_TAG_array_type ]
!97 = metadata !{metadata !98}
!98 = metadata !{i32 720929, i64 1, i64 0}        ; [ DW_TAG_subrange_type ]
!99 = metadata !{i32 28, i32 8, metadata !20, null}
!100 = metadata !{i32 29, i32 3, metadata !20, null}
!101 = metadata !{i32 30, i32 12, metadata !20, null}
!102 = metadata !{i32 31, i32 3, metadata !20, null}
!103 = metadata !{i32 32, i32 3, metadata !20, null}
!104 = metadata !{i32 34, i32 3, metadata !20, null}
!105 = metadata !{i32 36, i32 7, metadata !20, null}
!106 = metadata !{i32 38, i32 7, metadata !107, null}
!107 = metadata !{i32 720907, metadata !20, i32 37, i32 5, metadata !6, i32 1} ; [ DW_TAG_lexical_block ]
!108 = metadata !{i32 39, i32 7, metadata !107, null}
!109 = metadata !{i32 40, i32 7, metadata !107, null}
!110 = metadata !{i32 40, i32 45, metadata !107, null}
!111 = metadata !{i32 41, i32 7, metadata !107, null}
!112 = metadata !{i32 41, i32 46, metadata !107, null}
!113 = metadata !{i32 42, i32 5, metadata !107, null}
!114 = metadata !{i32 44, i32 5, metadata !20, null}
!115 = metadata !{i32 46, i32 3, metadata !20, null}
!116 = metadata !{i32 721152, metadata !20, metadata !"path2", metadata !6, i32 48, metadata !16, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!117 = metadata !{i32 48, i32 9, metadata !20, null}
!118 = metadata !{i32 48, i32 24, metadata !20, null}
!119 = metadata !{i32 48, i32 40, metadata !20, null}
!120 = metadata !{i32 49, i32 3, metadata !20, null}
!121 = metadata !{i32 50, i32 3, metadata !20, null}
!122 = metadata !{i32 50, i32 18, metadata !20, null}
!123 = metadata !{i32 51, i32 3, metadata !20, null}
!124 = metadata !{i32 52, i32 3, metadata !20, null}
!125 = metadata !{i32 54, i32 3, metadata !20, null}
!126 = metadata !{i32 55, i32 1, metadata !20, null}
!127 = metadata !{i32 55, i32 1, metadata !5, null}
