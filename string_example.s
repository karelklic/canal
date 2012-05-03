; ModuleID = 'string_example.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@.str = private unnamed_addr constant [11 x i8] c"/etc/hosts\00", align 1
@path1 = common global i8* null, align 8
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
  %path0 = alloca i8*, align 8
  %fp = alloca %struct._IO_FILE*, align 8
  %size = alloca i64, align 8
  %contents_heap = alloca i8*, align 8
  %blocks = alloca i64, align 8
  %4 = alloca i8*
  %path2 = alloca i8*, align 8
  %5 = alloca i32
  store i32 0, i32* %1
  store i32 %argc, i32* %2, align 4
  call void @llvm.dbg.declare(metadata !{i32* %2}, metadata !17), !dbg !18
  store i8** %argv, i8*** %3, align 8
  call void @llvm.dbg.declare(metadata !{i8*** %3}, metadata !19), !dbg !21
  call void @llvm.dbg.declare(metadata !{i8** %path0}, metadata !22), !dbg !24
  store i8* getelementptr inbounds ([11 x i8]* @.str, i32 0, i32 0), i8** %path0, align 8, !dbg !25
  %6 = load i8** %path0, align 8, !dbg !26
  store i8* %6, i8** @path1, align 8, !dbg !26
  call void @llvm.dbg.declare(metadata !{%struct._IO_FILE** %fp}, metadata !27), !dbg !83
  %7 = load i8** @path1, align 8, !dbg !84
  %8 = call %struct._IO_FILE* @fopen(i8* %7, i8* getelementptr inbounds ([2 x i8]* @.str1, i32 0, i32 0)), !dbg !84
  store %struct._IO_FILE* %8, %struct._IO_FILE** %fp, align 8, !dbg !84
  %9 = load %struct._IO_FILE** %fp, align 8, !dbg !85
  %10 = call i32 @fseek(%struct._IO_FILE* %9, i64 0, i32 2), !dbg !85
  call void @llvm.dbg.declare(metadata !{i64* %size}, metadata !86), !dbg !87
  %11 = load %struct._IO_FILE** %fp, align 8, !dbg !88
  %12 = call i64 @ftell(%struct._IO_FILE* %11), !dbg !88
  store i64 %12, i64* %size, align 8, !dbg !88
  %13 = load %struct._IO_FILE** %fp, align 8, !dbg !89
  %14 = call i32 @fseek(%struct._IO_FILE* %13, i64 0, i32 0), !dbg !89
  call void @llvm.dbg.declare(metadata !{i8** %contents_heap}, metadata !90), !dbg !91
  %15 = load i64* %size, align 8, !dbg !92
  %16 = add i64 %15, 1, !dbg !92
  %17 = call noalias i8* @malloc(i64 %16) nounwind, !dbg !92
  store i8* %17, i8** %contents_heap, align 8, !dbg !92
  call void @llvm.dbg.declare(metadata !{i64* %blocks}, metadata !93), !dbg !94
  %18 = load i8** %contents_heap, align 8, !dbg !95
  %19 = load i64* %size, align 8, !dbg !95
  %20 = load %struct._IO_FILE** %fp, align 8, !dbg !95
  %21 = call i64 @fread(i8* %18, i64 %19, i64 1, %struct._IO_FILE* %20), !dbg !95
  store i64 %21, i64* %blocks, align 8, !dbg !95
  %22 = load i64* %size, align 8, !dbg !96
  %23 = load i8** %contents_heap, align 8, !dbg !96
  %24 = getelementptr inbounds i8* %23, i64 %22, !dbg !96
  store i8 0, i8* %24, !dbg !96
  %25 = load i64* %blocks, align 8, !dbg !97
  %26 = icmp eq i64 %25, 1, !dbg !97
  br i1 %26, label %27, label %28, !dbg !97

; <label>:27                                      ; preds = %0
  br label %30, !dbg !97

; <label>:28                                      ; preds = %0
  call void @__assert_fail(i8* getelementptr inbounds ([12 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([17 x i8]* @.str3, i32 0, i32 0), i32 29, i8* getelementptr inbounds ([23 x i8]* @__PRETTY_FUNCTION__.main, i32 0, i32 0)) noreturn nounwind, !dbg !97
  unreachable, !dbg !97
                                                  ; No predecessors!
  br label %30, !dbg !97

; <label>:30                                      ; preds = %29, %27
  %31 = load i64* %size, align 8, !dbg !98
  %32 = add i64 %31, 1, !dbg !98
  %33 = call i8* @llvm.stacksave(), !dbg !98
  store i8* %33, i8** %4, !dbg !98
  %34 = alloca i8, i64 %32, align 16, !dbg !98
  call void @llvm.dbg.declare(metadata !{i8* %34}, metadata !99), !dbg !103
  %35 = load %struct._IO_FILE** %fp, align 8, !dbg !104
  %36 = call i32 @fseek(%struct._IO_FILE* %35, i64 0, i32 0), !dbg !104
  %37 = load i64* %size, align 8, !dbg !105
  %38 = load %struct._IO_FILE** %fp, align 8, !dbg !105
  %39 = call i64 @fread(i8* %34, i64 %37, i64 1, %struct._IO_FILE* %38), !dbg !105
  store i64 %39, i64* %blocks, align 8, !dbg !105
  %40 = load i64* %size, align 8, !dbg !106
  %41 = getelementptr inbounds i8* %34, i64 %40, !dbg !106
  store i8 0, i8* %41, !dbg !106
  %42 = load i64* %blocks, align 8, !dbg !107
  %43 = icmp eq i64 %42, 1, !dbg !107
  br i1 %43, label %44, label %45, !dbg !107

; <label>:44                                      ; preds = %30
  br label %47, !dbg !107

; <label>:45                                      ; preds = %30
  call void @__assert_fail(i8* getelementptr inbounds ([12 x i8]* @.str2, i32 0, i32 0), i8* getelementptr inbounds ([17 x i8]* @.str3, i32 0, i32 0), i32 35, i8* getelementptr inbounds ([23 x i8]* @__PRETTY_FUNCTION__.main, i32 0, i32 0)) noreturn nounwind, !dbg !107
  unreachable, !dbg !107
                                                  ; No predecessors!
  br label %47, !dbg !107

; <label>:47                                      ; preds = %46, %44
  %48 = load %struct._IO_FILE** %fp, align 8, !dbg !108
  %49 = call i32 @fclose(%struct._IO_FILE* %48), !dbg !108
  %50 = load i8** %contents_heap, align 8, !dbg !109
  %51 = call i32 @strcmp(i8* %50, i8* %34) nounwind readonly, !dbg !109
  %52 = icmp ne i32 %51, 0, !dbg !109
  br i1 %52, label %53, label %66, !dbg !109

; <label>:53                                      ; preds = %47
  %54 = load %struct._IO_FILE** @stdout, align 8, !dbg !110
  %55 = call i32 (%struct._IO_FILE*, i8*, ...)* @fprintf(%struct._IO_FILE* %54, i8* getelementptr inbounds ([37 x i8]* @.str4, i32 0, i32 0)), !dbg !110
  %56 = load %struct._IO_FILE** @stdout, align 8, !dbg !112
  %57 = load i64* %size, align 8, !dbg !112
  %58 = call i32 (%struct._IO_FILE*, i8*, ...)* @fprintf(%struct._IO_FILE* %56, i8* getelementptr inbounds ([16 x i8]* @.str5, i32 0, i32 0), i64 %57), !dbg !112
  %59 = load %struct._IO_FILE** @stdout, align 8, !dbg !113
  %60 = load i8** %contents_heap, align 8, !dbg !114
  %61 = call i64 @strlen(i8* %60) nounwind readonly, !dbg !114
  %62 = call i32 (%struct._IO_FILE*, i8*, ...)* @fprintf(%struct._IO_FILE* %59, i8* getelementptr inbounds ([18 x i8]* @.str6, i32 0, i32 0), i64 %61), !dbg !114
  %63 = load %struct._IO_FILE** @stdout, align 8, !dbg !115
  %64 = call i64 @strlen(i8* %34) nounwind readonly, !dbg !116
  %65 = call i32 (%struct._IO_FILE*, i8*, ...)* @fprintf(%struct._IO_FILE* %63, i8* getelementptr inbounds ([19 x i8]* @.str7, i32 0, i32 0), i64 %64), !dbg !116
  br label %68, !dbg !117

; <label>:66                                      ; preds = %47
  %67 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([35 x i8]* @.str8, i32 0, i32 0)), !dbg !118
  br label %68

; <label>:68                                      ; preds = %66, %53
  %69 = load i8** %contents_heap, align 8, !dbg !119
  call void @free(i8* %69) nounwind, !dbg !119
  call void @llvm.dbg.declare(metadata !{i8** %path2}, metadata !120), !dbg !121
  %70 = load i8** @path1, align 8, !dbg !122
  %71 = call i64 @strlen(i8* %70) nounwind readonly, !dbg !122
  %72 = add i64 %71, 7, !dbg !123
  %73 = add i64 %72, 1, !dbg !123
  %74 = call noalias i8* @malloc(i64 %73) nounwind, !dbg !123
  store i8* %74, i8** %path2, align 8, !dbg !123
  %75 = load i8** %path2, align 8, !dbg !124
  %76 = load i8** @path1, align 8, !dbg !124
  %77 = call i8* @strcpy(i8* %75, i8* %76) nounwind, !dbg !124
  %78 = load i8** %path2, align 8, !dbg !125
  %79 = load i8** @path1, align 8, !dbg !126
  %80 = call i64 @strlen(i8* %79) nounwind readonly, !dbg !126
  %81 = getelementptr inbounds i8* %78, i64 %80, !dbg !126
  %82 = call i8* @strcpy(i8* %81, i8* getelementptr inbounds ([8 x i8]* @.str9, i32 0, i32 0)) nounwind, !dbg !126
  %83 = load i8** %path2, align 8, !dbg !127
  %84 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([19 x i8]* @.str10, i32 0, i32 0), i8* %83), !dbg !127
  %85 = load i8** %path2, align 8, !dbg !128
  %86 = call %struct._IO_FILE* @fopen(i8* %85, i8* getelementptr inbounds ([2 x i8]* @.str1, i32 0, i32 0)), !dbg !128
  store %struct._IO_FILE* %86, %struct._IO_FILE** %fp, align 8, !dbg !128
  %87 = load %struct._IO_FILE** %fp, align 8, !dbg !129
  %88 = icmp ne %struct._IO_FILE* %87, null, !dbg !129
  br i1 %88, label %89, label %92, !dbg !129

; <label>:89                                      ; preds = %68
  %90 = load %struct._IO_FILE** %fp, align 8, !dbg !130
  %91 = call i32 @fclose(%struct._IO_FILE* %90), !dbg !130
  br label %92, !dbg !130

; <label>:92                                      ; preds = %89, %68
  %93 = load i8** %path2, align 8, !dbg !131
  call void @free(i8* %93) nounwind, !dbg !131
  store i32 0, i32* %1, !dbg !132
  store i32 1, i32* %5
  %94 = load i8** %4, !dbg !133
  call void @llvm.stackrestore(i8* %94), !dbg !133
  %95 = load i32* %1, !dbg !134
  ret i32 %95, !dbg !134
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

!0 = metadata !{i32 720913, i32 0, i32 12, metadata !"string_example.c", metadata !"/home/karel/devel/canal", metadata !"clang version 3.0 (tags/RELEASE_30/final)", i1 true, i1 false, metadata !"", i32 0, metadata !1, metadata !1, metadata !3, metadata !12} ; [ DW_TAG_compile_unit ]
!1 = metadata !{metadata !2}
!2 = metadata !{i32 0}
!3 = metadata !{metadata !4}
!4 = metadata !{metadata !5}
!5 = metadata !{i32 720942, i32 0, metadata !6, metadata !"main", metadata !"main", metadata !"", metadata !6, i32 16, metadata !7, i1 false, i1 true, i32 0, i32 0, i32 0, i32 256, i1 false, i32 (i32, i8**)* @main, null, null, metadata !10} ; [ DW_TAG_subprogram ]
!6 = metadata !{i32 720937, metadata !"string_example.c", metadata !"/home/karel/devel/canal", null} ; [ DW_TAG_file_type ]
!7 = metadata !{i32 720917, i32 0, metadata !"", i32 0, i32 0, i64 0, i64 0, i32 0, i32 0, i32 0, metadata !8, i32 0, i32 0} ; [ DW_TAG_subroutine_type ]
!8 = metadata !{metadata !9}
!9 = metadata !{i32 720932, null, metadata !"int", null, i32 0, i64 32, i64 32, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ]
!10 = metadata !{metadata !11}
!11 = metadata !{i32 720932}                      ; [ DW_TAG_base_type ]
!12 = metadata !{metadata !13}
!13 = metadata !{metadata !14}
!14 = metadata !{i32 720948, i32 0, null, metadata !"path1", metadata !"path1", metadata !"", metadata !6, i32 13, metadata !15, i32 0, i32 1, i8** @path1} ; [ DW_TAG_variable ]
!15 = metadata !{i32 720911, null, metadata !"", null, i32 0, i64 64, i64 64, i64 0, i32 0, metadata !16} ; [ DW_TAG_pointer_type ]
!16 = metadata !{i32 720932, null, metadata !"char", null, i32 0, i64 8, i64 8, i64 0, i32 0, i32 6} ; [ DW_TAG_base_type ]
!17 = metadata !{i32 721153, metadata !5, metadata !"argc", metadata !6, i32 16777231, metadata !9, i32 0, i32 0} ; [ DW_TAG_arg_variable ]
!18 = metadata !{i32 15, i32 14, metadata !5, null}
!19 = metadata !{i32 721153, metadata !5, metadata !"argv", metadata !6, i32 33554447, metadata !20, i32 0, i32 0} ; [ DW_TAG_arg_variable ]
!20 = metadata !{i32 720911, null, metadata !"", null, i32 0, i64 64, i64 64, i64 0, i32 0, metadata !15} ; [ DW_TAG_pointer_type ]
!21 = metadata !{i32 15, i32 27, metadata !5, null}
!22 = metadata !{i32 721152, metadata !23, metadata !"path0", metadata !6, i32 17, metadata !15, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!23 = metadata !{i32 720907, metadata !5, i32 16, i32 1, metadata !6, i32 0} ; [ DW_TAG_lexical_block ]
!24 = metadata !{i32 17, i32 9, metadata !23, null}
!25 = metadata !{i32 17, i32 29, metadata !23, null}
!26 = metadata !{i32 18, i32 3, metadata !23, null}
!27 = metadata !{i32 721152, metadata !23, metadata !"fp", metadata !6, i32 19, metadata !28, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!28 = metadata !{i32 720911, null, metadata !"", null, i32 0, i64 64, i64 64, i64 0, i32 0, metadata !29} ; [ DW_TAG_pointer_type ]
!29 = metadata !{i32 720918, null, metadata !"FILE", metadata !6, i32 49, i64 0, i64 0, i64 0, i32 0, metadata !30} ; [ DW_TAG_typedef ]
!30 = metadata !{i32 720915, null, metadata !"_IO_FILE", metadata !31, i32 273, i64 1728, i64 64, i32 0, i32 0, i32 0, metadata !32, i32 0, i32 0} ; [ DW_TAG_structure_type ]
!31 = metadata !{i32 720937, metadata !"/usr/include/libio.h", metadata !"/home/karel/devel/canal", null} ; [ DW_TAG_file_type ]
!32 = metadata !{metadata !33, metadata !34, metadata !35, metadata !36, metadata !37, metadata !38, metadata !39, metadata !40, metadata !41, metadata !42, metadata !43, metadata !44, metadata !45, metadata !53, metadata !54, metadata !55, metadata !56, metadata !59, metadata !61, metadata !63, metadata !67, metadata !69, metadata !71, metadata !72, metadata !73, metadata !74, metadata !75, metadata !78, metadata !79}
!33 = metadata !{i32 720909, metadata !30, metadata !"_flags", metadata !31, i32 274, i64 32, i64 32, i64 0, i32 0, metadata !9} ; [ DW_TAG_member ]
!34 = metadata !{i32 720909, metadata !30, metadata !"_IO_read_ptr", metadata !31, i32 279, i64 64, i64 64, i64 64, i32 0, metadata !15} ; [ DW_TAG_member ]
!35 = metadata !{i32 720909, metadata !30, metadata !"_IO_read_end", metadata !31, i32 280, i64 64, i64 64, i64 128, i32 0, metadata !15} ; [ DW_TAG_member ]
!36 = metadata !{i32 720909, metadata !30, metadata !"_IO_read_base", metadata !31, i32 281, i64 64, i64 64, i64 192, i32 0, metadata !15} ; [ DW_TAG_member ]
!37 = metadata !{i32 720909, metadata !30, metadata !"_IO_write_base", metadata !31, i32 282, i64 64, i64 64, i64 256, i32 0, metadata !15} ; [ DW_TAG_member ]
!38 = metadata !{i32 720909, metadata !30, metadata !"_IO_write_ptr", metadata !31, i32 283, i64 64, i64 64, i64 320, i32 0, metadata !15} ; [ DW_TAG_member ]
!39 = metadata !{i32 720909, metadata !30, metadata !"_IO_write_end", metadata !31, i32 284, i64 64, i64 64, i64 384, i32 0, metadata !15} ; [ DW_TAG_member ]
!40 = metadata !{i32 720909, metadata !30, metadata !"_IO_buf_base", metadata !31, i32 285, i64 64, i64 64, i64 448, i32 0, metadata !15} ; [ DW_TAG_member ]
!41 = metadata !{i32 720909, metadata !30, metadata !"_IO_buf_end", metadata !31, i32 286, i64 64, i64 64, i64 512, i32 0, metadata !15} ; [ DW_TAG_member ]
!42 = metadata !{i32 720909, metadata !30, metadata !"_IO_save_base", metadata !31, i32 288, i64 64, i64 64, i64 576, i32 0, metadata !15} ; [ DW_TAG_member ]
!43 = metadata !{i32 720909, metadata !30, metadata !"_IO_backup_base", metadata !31, i32 289, i64 64, i64 64, i64 640, i32 0, metadata !15} ; [ DW_TAG_member ]
!44 = metadata !{i32 720909, metadata !30, metadata !"_IO_save_end", metadata !31, i32 290, i64 64, i64 64, i64 704, i32 0, metadata !15} ; [ DW_TAG_member ]
!45 = metadata !{i32 720909, metadata !30, metadata !"_markers", metadata !31, i32 292, i64 64, i64 64, i64 768, i32 0, metadata !46} ; [ DW_TAG_member ]
!46 = metadata !{i32 720911, null, metadata !"", null, i32 0, i64 64, i64 64, i64 0, i32 0, metadata !47} ; [ DW_TAG_pointer_type ]
!47 = metadata !{i32 720915, null, metadata !"_IO_marker", metadata !31, i32 188, i64 192, i64 64, i32 0, i32 0, i32 0, metadata !48, i32 0, i32 0} ; [ DW_TAG_structure_type ]
!48 = metadata !{metadata !49, metadata !50, metadata !52}
!49 = metadata !{i32 720909, metadata !47, metadata !"_next", metadata !31, i32 189, i64 64, i64 64, i64 0, i32 0, metadata !46} ; [ DW_TAG_member ]
!50 = metadata !{i32 720909, metadata !47, metadata !"_sbuf", metadata !31, i32 190, i64 64, i64 64, i64 64, i32 0, metadata !51} ; [ DW_TAG_member ]
!51 = metadata !{i32 720911, null, metadata !"", null, i32 0, i64 64, i64 64, i64 0, i32 0, metadata !30} ; [ DW_TAG_pointer_type ]
!52 = metadata !{i32 720909, metadata !47, metadata !"_pos", metadata !31, i32 194, i64 32, i64 32, i64 128, i32 0, metadata !9} ; [ DW_TAG_member ]
!53 = metadata !{i32 720909, metadata !30, metadata !"_chain", metadata !31, i32 294, i64 64, i64 64, i64 832, i32 0, metadata !51} ; [ DW_TAG_member ]
!54 = metadata !{i32 720909, metadata !30, metadata !"_fileno", metadata !31, i32 296, i64 32, i64 32, i64 896, i32 0, metadata !9} ; [ DW_TAG_member ]
!55 = metadata !{i32 720909, metadata !30, metadata !"_flags2", metadata !31, i32 300, i64 32, i64 32, i64 928, i32 0, metadata !9} ; [ DW_TAG_member ]
!56 = metadata !{i32 720909, metadata !30, metadata !"_old_offset", metadata !31, i32 302, i64 64, i64 64, i64 960, i32 0, metadata !57} ; [ DW_TAG_member ]
!57 = metadata !{i32 720918, null, metadata !"__off_t", metadata !31, i32 141, i64 0, i64 0, i64 0, i32 0, metadata !58} ; [ DW_TAG_typedef ]
!58 = metadata !{i32 720932, null, metadata !"long int", null, i32 0, i64 64, i64 64, i64 0, i32 0, i32 5} ; [ DW_TAG_base_type ]
!59 = metadata !{i32 720909, metadata !30, metadata !"_cur_column", metadata !31, i32 306, i64 16, i64 16, i64 1024, i32 0, metadata !60} ; [ DW_TAG_member ]
!60 = metadata !{i32 720932, null, metadata !"unsigned short", null, i32 0, i64 16, i64 16, i64 0, i32 0, i32 7} ; [ DW_TAG_base_type ]
!61 = metadata !{i32 720909, metadata !30, metadata !"_vtable_offset", metadata !31, i32 307, i64 8, i64 8, i64 1040, i32 0, metadata !62} ; [ DW_TAG_member ]
!62 = metadata !{i32 720932, null, metadata !"signed char", null, i32 0, i64 8, i64 8, i64 0, i32 0, i32 6} ; [ DW_TAG_base_type ]
!63 = metadata !{i32 720909, metadata !30, metadata !"_shortbuf", metadata !31, i32 308, i64 8, i64 8, i64 1048, i32 0, metadata !64} ; [ DW_TAG_member ]
!64 = metadata !{i32 720897, null, metadata !"", null, i32 0, i64 8, i64 8, i32 0, i32 0, metadata !16, metadata !65, i32 0, i32 0} ; [ DW_TAG_array_type ]
!65 = metadata !{metadata !66}
!66 = metadata !{i32 720929, i64 0, i64 0}        ; [ DW_TAG_subrange_type ]
!67 = metadata !{i32 720909, metadata !30, metadata !"_lock", metadata !31, i32 312, i64 64, i64 64, i64 1088, i32 0, metadata !68} ; [ DW_TAG_member ]
!68 = metadata !{i32 720911, null, metadata !"", null, i32 0, i64 64, i64 64, i64 0, i32 0, null} ; [ DW_TAG_pointer_type ]
!69 = metadata !{i32 720909, metadata !30, metadata !"_offset", metadata !31, i32 321, i64 64, i64 64, i64 1152, i32 0, metadata !70} ; [ DW_TAG_member ]
!70 = metadata !{i32 720918, null, metadata !"__off64_t", metadata !31, i32 142, i64 0, i64 0, i64 0, i32 0, metadata !58} ; [ DW_TAG_typedef ]
!71 = metadata !{i32 720909, metadata !30, metadata !"__pad1", metadata !31, i32 330, i64 64, i64 64, i64 1216, i32 0, metadata !68} ; [ DW_TAG_member ]
!72 = metadata !{i32 720909, metadata !30, metadata !"__pad2", metadata !31, i32 331, i64 64, i64 64, i64 1280, i32 0, metadata !68} ; [ DW_TAG_member ]
!73 = metadata !{i32 720909, metadata !30, metadata !"__pad3", metadata !31, i32 332, i64 64, i64 64, i64 1344, i32 0, metadata !68} ; [ DW_TAG_member ]
!74 = metadata !{i32 720909, metadata !30, metadata !"__pad4", metadata !31, i32 333, i64 64, i64 64, i64 1408, i32 0, metadata !68} ; [ DW_TAG_member ]
!75 = metadata !{i32 720909, metadata !30, metadata !"__pad5", metadata !31, i32 334, i64 64, i64 64, i64 1472, i32 0, metadata !76} ; [ DW_TAG_member ]
!76 = metadata !{i32 720918, null, metadata !"size_t", metadata !31, i32 35, i64 0, i64 0, i64 0, i32 0, metadata !77} ; [ DW_TAG_typedef ]
!77 = metadata !{i32 720932, null, metadata !"long unsigned int", null, i32 0, i64 64, i64 64, i64 0, i32 0, i32 7} ; [ DW_TAG_base_type ]
!78 = metadata !{i32 720909, metadata !30, metadata !"_mode", metadata !31, i32 336, i64 32, i64 32, i64 1536, i32 0, metadata !9} ; [ DW_TAG_member ]
!79 = metadata !{i32 720909, metadata !30, metadata !"_unused2", metadata !31, i32 338, i64 160, i64 8, i64 1568, i32 0, metadata !80} ; [ DW_TAG_member ]
!80 = metadata !{i32 720897, null, metadata !"", null, i32 0, i64 160, i64 8, i32 0, i32 0, metadata !16, metadata !81, i32 0, i32 0} ; [ DW_TAG_array_type ]
!81 = metadata !{metadata !82}
!82 = metadata !{i32 720929, i64 0, i64 19}       ; [ DW_TAG_subrange_type ]
!83 = metadata !{i32 19, i32 9, metadata !23, null}
!84 = metadata !{i32 19, i32 14, metadata !23, null}
!85 = metadata !{i32 22, i32 3, metadata !23, null}
!86 = metadata !{i32 721152, metadata !23, metadata !"size", metadata !6, i32 23, metadata !76, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!87 = metadata !{i32 23, i32 10, metadata !23, null}
!88 = metadata !{i32 23, i32 17, metadata !23, null}
!89 = metadata !{i32 24, i32 3, metadata !23, null}
!90 = metadata !{i32 721152, metadata !23, metadata !"contents_heap", metadata !6, i32 26, metadata !15, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!91 = metadata !{i32 26, i32 9, metadata !23, null}
!92 = metadata !{i32 26, i32 25, metadata !23, null}
!93 = metadata !{i32 721152, metadata !23, metadata !"blocks", metadata !6, i32 27, metadata !76, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!94 = metadata !{i32 27, i32 10, metadata !23, null}
!95 = metadata !{i32 27, i32 19, metadata !23, null}
!96 = metadata !{i32 28, i32 3, metadata !23, null}
!97 = metadata !{i32 29, i32 3, metadata !23, null}
!98 = metadata !{i32 31, i32 32, metadata !23, null}
!99 = metadata !{i32 721152, metadata !23, metadata !"contents_stack", metadata !6, i32 31, metadata !100, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!100 = metadata !{i32 720897, null, metadata !"", null, i32 0, i64 0, i64 8, i32 0, i32 0, metadata !16, metadata !101, i32 0, i32 0} ; [ DW_TAG_array_type ]
!101 = metadata !{metadata !102}
!102 = metadata !{i32 720929, i64 1, i64 0}       ; [ DW_TAG_subrange_type ]
!103 = metadata !{i32 31, i32 8, metadata !23, null}
!104 = metadata !{i32 32, i32 3, metadata !23, null}
!105 = metadata !{i32 33, i32 12, metadata !23, null}
!106 = metadata !{i32 34, i32 3, metadata !23, null}
!107 = metadata !{i32 35, i32 3, metadata !23, null}
!108 = metadata !{i32 37, i32 3, metadata !23, null}
!109 = metadata !{i32 39, i32 7, metadata !23, null}
!110 = metadata !{i32 41, i32 7, metadata !111, null}
!111 = metadata !{i32 720907, metadata !23, i32 40, i32 5, metadata !6, i32 1} ; [ DW_TAG_lexical_block ]
!112 = metadata !{i32 42, i32 7, metadata !111, null}
!113 = metadata !{i32 43, i32 7, metadata !111, null}
!114 = metadata !{i32 43, i32 45, metadata !111, null}
!115 = metadata !{i32 44, i32 7, metadata !111, null}
!116 = metadata !{i32 44, i32 46, metadata !111, null}
!117 = metadata !{i32 45, i32 5, metadata !111, null}
!118 = metadata !{i32 47, i32 5, metadata !23, null}
!119 = metadata !{i32 49, i32 3, metadata !23, null}
!120 = metadata !{i32 721152, metadata !23, metadata !"path2", metadata !6, i32 51, metadata !15, i32 0, i32 0} ; [ DW_TAG_auto_variable ]
!121 = metadata !{i32 51, i32 9, metadata !23, null}
!122 = metadata !{i32 51, i32 24, metadata !23, null}
!123 = metadata !{i32 51, i32 40, metadata !23, null}
!124 = metadata !{i32 52, i32 3, metadata !23, null}
!125 = metadata !{i32 53, i32 3, metadata !23, null}
!126 = metadata !{i32 53, i32 18, metadata !23, null}
!127 = metadata !{i32 54, i32 3, metadata !23, null}
!128 = metadata !{i32 56, i32 8, metadata !23, null}
!129 = metadata !{i32 57, i32 3, metadata !23, null}
!130 = metadata !{i32 58, i32 5, metadata !23, null}
!131 = metadata !{i32 60, i32 3, metadata !23, null}
!132 = metadata !{i32 62, i32 3, metadata !23, null}
!133 = metadata !{i32 63, i32 1, metadata !23, null}
!134 = metadata !{i32 63, i32 1, metadata !5, null}
