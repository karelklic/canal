include_directories(${CANAL_SOURCE_DIR})
link_directories(${CANAL_SOURCE_DIR}/lib)

set(CANAL_UNIT_TESTS
    ArrayExactSizeTest
    ArraySingleItemTest
    ArrayStringPrefixTest
    ArrayStringTrieTest
    FloatIntervalTest
    IntegerBitfieldTest
    IntegerSetTest
    IntegerIntervalTest
    PointerTest
    ProductMessageTest
    ProductVectorTest)

foreach(test ${CANAL_UNIT_TESTS})
    add_executable(${test} "${test}.cpp")
    target_link_libraries(${test} canal)
    add_test(${test} ${test})
endforeach()

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
