AC_PREREQ([2.60])
AC_INIT([canal],[0])
AM_INIT_AUTOMAKE([-Wall])
AM_MAINTAINER_MODE
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CXX
AC_USE_SYSTEM_EXTENSIONS
AC_CONFIG_HEADERS([lib/Config.h])

# Initialize libtool.
AC_DISABLE_STATIC
LT_INIT
AC_SUBST([LIBTOOL_DEPS])

# LLVM
AC_PATH_PROG([LLVM_CONFIG], [llvm-config], [no])
[if test "$LLVM_CONFIG" = "no"; then]
    [echo "The llvm-config program was not found in the search path. Please ensure"]
    [echo "that it is installed and its directory is included in the search path."]
    [echo "Then run configure again before attempting to build Canal."]
    [exit 1]
[fi]

AC_PATH_PROG([LLVM_LINK], [llvm-link], [no])
[if test "$LLVM_LINK" = "no"; then]
    [echo "The llvm-link program was not found in the search path. Please ensure"]
    [echo "that it is installed and its directory is included in the search path."]
    [echo "Then run configure again before attempting to build Canal."]
    [exit 1]
[fi]
AC_SUBST([LLVM_LINK])

LLVM_VERSION=`$LLVM_CONFIG --version | sed 's/svn.*//g'`
AC_SUBST([LLVM_VERSION])

LLVM_CFLAGS=`$LLVM_CONFIG --cppflags | sed -e 's/-DNDEBUG\>//g' -e 's/-pedantic//g'`
AC_SUBST([LLVM_CFLAGS])

LLVM_LIBS=`$LLVM_CONFIG --libs`
AC_SUBST([LLVM_LIBS])

LLVM_LDFLAGS=`$LLVM_CONFIG --ldflags`
AC_SUBST([LLVM_LDFLAGS])

# Quadrigraphs: @<:@ gives you [ and @:>@ gives you ].
CPPFLAGS+=`$LLVM_CONFIG --version | sed 's/\(@<:@0-9@:>@*\).\(@<:@0-9@:>@*\).*/-DLLVM_MAJOR=\1 -DLLVM_MINOR=\2/'`

# CLANG
AC_PATH_PROG([CLANG], [clang], [no])
[if test "$CLANG" = "no"; then]
    [echo "The clang program was not found in the search path. Please ensure"]
    [echo "that it is installed and its directory is included in the search path."]
    [echo "Then run configure again before attempting to build Canal."]
    [exit 1]
[fi]
AC_SUBST([CLANG])

# readline
AC_CHECK_HEADERS([readline/readline.h], [],
	[AC_MSG_ERROR([Please install readline development headers])])

# pdflatex
AC_PATH_PROG([PDFLATEX], [pdflatex], [no])
[if test "$PDFLATEX" = "no"; then]
    [echo "The pdflatex program was not found in the search path. Please ensure"]
    [echo "that it is installed and its directory is included in the search path."]
    [echo "Then run configure again before attempting to build Canal."]
    [exit 1]
[fi]

# doxygen
AC_PATH_PROG([DOXYGEN], [doxygen], [no])
[if test "$DOXYGEN" = "no"; then]
    [echo "The doxygen program was not found in the search path. Please ensure"]
    [echo "that it is installed and its directory is included in the search path."]
    [echo "Then run configure again before attempting to build Canal."]
    [exit 1]
[fi]

# dot (part of graphviz
AC_PATH_PROG([DOT], [dot], [no])
[if test "DOT" = "no"; then]
    [echo "The dot program was not found in the search path. Please ensure"]
    [echo "that it is installed and its directory is included in the search path."]
    [echo "Then run configure again before attempting to build Canal."]
    [exit 1]
[fi]

# Initialize the test suite.
AC_CONFIG_TESTDIR(tests)
AC_CONFIG_FILES([tests/Makefile tests/atlocal])
AM_MISSING_PROG([AUTOM4TE], [autom4te])
# Needed by tests/atlocal.in.
AC_SUBST([O0CFLAGS], [`echo $CFLAGS | sed 's/-O[[0-9]] *//g'`])

AC_CONFIG_FILES([
    canal.spec
    Makefile
    docs/Makefile
    lib/Makefile
    tool/Makefile
    wrappers/config.py
    wrappers/Makefile
])

AC_OUTPUT
