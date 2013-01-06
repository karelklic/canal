# Find Clang
#
# It defines the following variables
# CLANG_FOUND        - True if Clang found.
# CLANG_INCLUDE_DIRS - where to find Clang include files
# CLANG_LIBS         - list of clang libs

if (NOT LLVM_INCLUDE_DIRS OR NOT LLVM_LIBRARY_DIRS)
   message(FATAL_ERROR "No LLVM and Clang support requires LLVM")
else (NOT LLVM_INCLUDE_DIRS OR NOT LLVM_LIBRARY_DIRS)

MACRO(FIND_AND_ADD_CLANG_LIB _libname_)
find_library(CLANG_${_libname_}_LIB ${_libname_} ${LLVM_LIB_DIR} ${CLANG_LIB_DIR})
if (CLANG_${_libname_}_LIB)
   set(CLANG_LIBS ${CLANG_LIBS} ${CLANG_${_libname_}_LIB})
endif(CLANG_${_libname_}_LIB)
ENDMACRO(FIND_AND_ADD_CLANG_LIB)

set(CLANG_INCLUDE_DIRS ${CLANG_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})
set(CLANG_INCLUDE_DIRS ${CLANG_INCLUDE_DIRS} ${CLANG_INCLUDE_DIR})

FIND_AND_ADD_CLANG_LIB(clangFrontend)
FIND_AND_ADD_CLANG_LIB(clangDriver)
FIND_AND_ADD_CLANG_LIB(clangCodeGen)
FIND_AND_ADD_CLANG_LIB(clangEdit)
FIND_AND_ADD_CLANG_LIB(clangSema)
FIND_AND_ADD_CLANG_LIB(clangChecker)
FIND_AND_ADD_CLANG_LIB(clangAnalysis)
FIND_AND_ADD_CLANG_LIB(clangRewrite)
FIND_AND_ADD_CLANG_LIB(clangAST)
FIND_AND_ADD_CLANG_LIB(clangParse)
FIND_AND_ADD_CLANG_LIB(clangLex)
FIND_AND_ADD_CLANG_LIB(clangBasic)

# Ubuntu ships clang in a single dynamic shared library.
if (NOT CLANG_LIBS)
  find_library(CLANG_LIBS clang HINTS ${LLVM_LIBRARY_DIRS})
endif (NOT CLANG_LIBS)

MESSAGE(STATUS "Clang libs: " ${CLANG_LIBS})

if (CLANG_LIBS)
  set(CLANG_FOUND TRUE)
endif (CLANG_LIBS)

if (CLANG_FOUND)
  message(STATUS "Found Clang: ${CLANG_INCLUDE_DIRS}")
else (CLANG_FOUND)
  if (CLANG_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find Clang")
  endif (CLANG_FIND_REQUIRED)
endif (CLANG_FOUND)

endif (NOT LLVM_INCLUDE_DIRS OR NOT LLVM_LIBRARY_DIRS)
