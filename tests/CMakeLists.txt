include_directories(${CANAL_SOURCE_DIR})
link_directories(${CANAL_SOURCE_DIR}/lib)

add_executable(OperationsTest OperationsTest.cpp)
target_link_libraries(OperationsTest canal)
add_test(OperationsTest OperationsTest)

add_executable(PointerTest PointerTest.cpp)
target_link_libraries(PointerTest canal)
add_test(PointerTest PointerTest)

add_executable(IntegerIntervalTest IntegerIntervalTest.cpp)
target_link_libraries(IntegerIntervalTest canal)
add_test(IntegerIntervalTest IntegerIntervalTest)

add_executable(ICMPTest ICMPTest.cpp)
target_link_libraries(ICMPTest canal)
add_test(ICMPTest ICMPTest)

add_executable(MergeTest MergeTest.cpp)
target_link_libraries(MergeTest canal)
add_test(MergeTest MergeTest)

add_executable(COWTest COWTest.cpp)
target_link_libraries(COWTest canal)
add_test(COWTest COWTest)

file(GLOB files "*.c")
foreach(file ${files})
    get_filename_component(name ${file} NAME)
    add_test(validate_${name}
        "${CMAKE_SOURCE_DIR}/tests/file_test.sh"
	${CMAKE_BINARY_DIR}/tool/canal
	${file})
endforeach()
