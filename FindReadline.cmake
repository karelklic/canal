# Find readline
#
# It defines the following variables
#  READLINE_FOUND        - If false, do not compile with readline.
#  READLINE_INCLUDE_DIRS - Where to find headers.
#  READLINE_LIBRARIES    - The libraries to link against.

find_path(READLINE_INCLUDE_DIRS readline/readline.h)
find_library(READLINE_LIBRARIES readline)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    Readline
    DEFAULT_MSG
    READLINE_LIBRARIES
    READLINE_INCLUDE_DIRS)

mark_as_advanced(READLINE_INCLUDE_DIRS READLINE_LIBRARIES)

