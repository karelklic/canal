include_directories(${CANAL_SOURCE_DIR})
link_directories(${CANAL_SOURCE_DIR}/lib)

add_executable(ArrayExactSizeTest ArrayExactSizeTest.cpp)
target_link_libraries(ArrayExactSizeTest canal)
add_test(ArrayExactSizeTest ArrayExactSizeTest)

add_executable(ArraySingleItemTest ArraySingleItemTest.cpp)
target_link_libraries(ArraySingleItemTest canal)
add_test(ArraySingleItemTest ArraySingleItemTest)

add_executable(ArrayStringPrefixTest ArrayStringPrefixTest.cpp)
target_link_libraries(ArrayStringPrefixTest canal)
add_test(ArrayStringPrefixTest ArrayStringPrefixTest)

add_executable(FieldMinMaxTest FieldMinMaxTest.cpp)
target_link_libraries(FieldMinMaxTest canal)
add_test(FieldMinMaxTest FieldMinMaxTest)

add_executable(FloatIntervalTest FloatIntervalTest.cpp)
target_link_libraries(FloatIntervalTest canal)
add_test(FloatIntervalTest FloatIntervalTest)

add_executable(IntegerBitfieldTest IntegerBitfieldTest.cpp)
target_link_libraries(IntegerBitfieldTest canal)
add_test(IntegerBitfieldTest IntegerBitfieldTest)

add_executable(IntegerSetTest IntegerSetTest.cpp)
target_link_libraries(IntegerSetTest canal)
add_test(IntegerSetTest IntegerSetTest)

add_executable(IntegerIntervalTest IntegerIntervalTest.cpp)
target_link_libraries(IntegerIntervalTest canal)
add_test(IntegerIntervalTest IntegerIntervalTest)

add_executable(PointerTest PointerTest.cpp)
target_link_libraries(PointerTest canal)
add_test(PointerTest PointerTest)

add_executable(ProductMessageTest ProductMessageTest.cpp)
target_link_libraries(ProductMessageTest canal)
add_test(ProductMessageTest ProductMessageTest)

add_executable(ProductVectorTest ProductVectorTest.cpp)
target_link_libraries(ProductVectorTest canal)
add_test(ProductVectorTest ProductVectorTest)

file(GLOB files "*.c")
foreach(file ${files})
    get_filename_component(name ${file} NAME)
    add_test(validate_${name}
        "${CMAKE_SOURCE_DIR}/tests/file_test.sh"
	${CMAKE_BINARY_DIR}/tool/canal
	${file})
endforeach()

file(GLOB files_integration "integration/*.c")
foreach(file ${files_integration})
    get_filename_component(name ${file} NAME)
    add_test(integration_${name}
        "${CMAKE_SOURCE_DIR}/tests/file_test.sh"
	${CMAKE_BINARY_DIR}/tool/canal
	${file})
endforeach()
