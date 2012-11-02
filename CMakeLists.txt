CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckIncludeFiles)

project(CANAL)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

SET(CMAKE_BUILD_TYPE "debug")
ENABLE_TESTING()

find_package(Readline)
if (NOT READLINE_FOUND)
	message(FATAL_ERROR "Readline not found")
endif()

find_package(LibElf)
if (NOT LIBELF_FOUND)
	message(FATAL_ERROR "LibElf not found")
endif()

find_package(LLVM)
if (NOT LLVM_FOUND)
	message(FATAL_ERROR "LLVM not found")
endif()

include_directories(${LLVM_INCLUDE_DIR})
link_directories(${LLVM_LIBRARY_DIR})
add_definitions(${LLVM_CFLAGS})
add_definitions(-DLLVM_MAJOR=${LLVM_MAJOR} -DLLVM_MINOR=${LLVM_MINOR})

add_definitions( -DDATADIR="${CMAKE_CURRENT_BINARY_DIR}" )

add_subdirectory("lib/")
add_subdirectory("tool/")
add_subdirectory("tests/")


CHECK_INCLUDE_FILES(gelf.h HAVE_GELF_H)
CHECK_INCLUDE_FILES(libelf.h HAVE_LIBELF_H)
CHECK_LIBRARY_EXISTS(elf elf_begin "" HAVE_LIBELF)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/lib/ConfigCMake.h.in ${CMAKE_CURRENT_SOURCE_DIR}/lib/Config.h)
