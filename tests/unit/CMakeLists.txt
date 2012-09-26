include_directories(${CANAL_SOURCE_DIR})
link_directories(${CANAL_SOURCE_DIR}/lib)

#Here will be separate executables
add_executable(IntegerIntervalTest IntegerIntervalTest.cpp)
target_link_libraries(IntegerIntervalTest canal)
add_test(IntegerIntervalTest IntegerIntervalTest)

add_executable(ICMPTest ICMPTest.cpp)
target_link_libraries(ICMPTest canal)
add_test(ICMPTest ICMPTest)

add_executable(MergeTest MergeTest.cpp)
target_link_libraries(MergeTest canal)
add_test(MergeTest MergeTest)
