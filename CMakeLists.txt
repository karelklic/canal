cmake_minimum_required(VERSION 2.8)
project(CANAL)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ".")

find_package(Readline)
if ( NOT READLINE_FOUND )
	message(FATAL_ERROR "Readline not found")
endif()

find_package(LibElf)
if ( NOT LIBELF_FOUND )
	message(FATAL_ERROR "LibElf not found")
endif()

find_package(LLVM)

if( NOT LLVM_FOUND )
  message(FATAL_ERROR "LLVM package can't be found. Set CMAKE_PREFIX_PATH variable to LLVM's installation prefix.")
endif()
include_directories( ${LLVM_INCLUDE_DIRS} )
link_directories( ${LLVM_LIBRARY_DIRS} )
add_definitions( ${LLVM_DEFINITIONS} )

add_definitions( -DDATADIR="${CMAKE_CURRENT_BINARY_DIR}" )

SET(CMAKE_BUILD_TYPE "debug")
ENABLE_TESTING()

# Let's suppose we want to build a JIT compiler with support for
# binary code (no interpreter):
llvm_map_components_to_libraries(REQ_LLVM_LIBRARIES bitreader asmparser target)


add_subdirectory("lib/")
add_subdirectory("tool/")
add_subdirectory("tests/")
