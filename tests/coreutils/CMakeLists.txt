#Here will be separate executables
file(GLOB files "*.c")
foreach(file ${files})
	GET_FILENAME_COMPONENT(name ${file} NAME)
	add_test(validate_${name} "../file_test.sh" ${CMAKE_SOURCE_DIR}/tool/canal ${file})
endforeach()
