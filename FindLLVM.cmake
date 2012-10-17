# Find the native LLVM includes and library
#
#  LLVM_INCLUDE_DIR - where to find llvm include files
#  LLVM_LIBRARY_DIR - where to find llvm libs
#  LLVM_CFLAGS      - llvm compiler flags
#  LLVM_LFLAGS      - llvm linker flags
#  LLVM_MODULE_LIBS - list of llvm libs for working with modules.
#  LLVM_FOUND       - True if llvm found.

find_program(LLVM_CONFIG_EXECUTABLE llvm-config DOC "llvm-config executable")

if (LLVM_CONFIG_EXECUTABLE)
  message(STATUS "LLVM llvm-config found at: ${LLVM_CONFIG_EXECUTABLE}")
else (LLVM_CONFIG_EXECUTABLE)
  message(FATAL_ERROR "Could NOT find LLVM executable")
endif (LLVM_CONFIG_EXECUTABLE)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --version
  OUTPUT_VARIABLE LLVM_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

STRING(REGEX REPLACE "^([0-9]+)\\.([0-9]+)" "\\1" LLVM_MAJOR
	     "${LLVM_VERSION}")

STRING(REGEX REPLACE "^([0-9]+)\\.([0-9]+)" "\\2" LLVM_MINOR
	     "${LLVM_VERSION}")

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --includedir
  OUTPUT_VARIABLE LLVM_INCLUDE_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --libdir
  OUTPUT_VARIABLE LLVM_LIBRARY_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --cppflags
  OUTPUT_VARIABLE LLVM_CFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags
  OUTPUT_VARIABLE LLVM_LDFLAGS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs core bitreader asmparser target
  OUTPUT_VARIABLE LLVM_MODULE_LIBS
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (LLVM_CONFIG_EXECUTABLE)
  set(LLVM_FOUND TRUE)
endif (LLVM_CONFIG_EXECUTABLE)
