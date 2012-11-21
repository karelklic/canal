# Find libelf from elfutils
#
# It defines the following variables
#  LIBELF_FOUND - If false, do not compile with libelf.
#  LIBELF_INCLUDE_DIRS - Where to find headers.
#  LIBELF_LIBRARIES - The libraries to link against.

find_path(LIBELF_H_INCLUDE_DIR libelf.h)
find_path(GELF_H_INCLUDE_DIR gelf.h)
set(LIBELF_INCLUDE_DIRS ${LIBELF_H_INCLUDE_DIR} ${GELF_H_INCLUDE_DIR})
find_library(LIBELF_LIBRARIES elf)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    LibElf
    DEFAULT_MSG
    LIBELF_LIBRARIES
    LIBELF_INCLUDE_DIRS
    LIBELF_H_INCLUDE_DIR
    GELF_H_INCLUDE_DIR)

mark_as_advanced(LIBELF_INCLUDE_DIRS LIBELF_LIBRARIES)
