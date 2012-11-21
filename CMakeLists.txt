CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckIncludeFiles)

project(CANAL)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

SET(CMAKE_BUILD_TYPE "debug")
ENABLE_TESTING()

find_package(Readline REQUIRED)
find_package(LibElf)
find_package(LLVM REQUIRED)

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_CFLAGS})
add_definitions(-DDATADIR="${CMAKE_CURRENT_BINARY_DIR}")

add_subdirectory("lib/")
add_subdirectory("tool/")
add_subdirectory("tests/")

# Prepare variables for the configuration file.
SET(HAVE_GELF_H LIBELF_FOUND)
SET(HAVE_LIBELF_H LIBELF_FOUND)
SET(HAVE_LIBELF LIBELF_FOUND)
SET(HAVE_READLINE_READLINE_H READLINE_FOUND)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/lib/ConfigCMake.h.in
               ${CMAKE_CURRENT_SOURCE_DIR}/lib/Config.h)
