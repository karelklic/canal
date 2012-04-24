AC_PREREQ([2.60])
AC_INIT([canal], [0])
AM_INIT_AUTOMAKE([-Wall])
AM_MAINTAINER_MODE
AC_CONFIG_MACRO_DIR([m4])
AC_DISABLE_STATIC
AC_PROG_CXX
AC_GNU_SOURCE
AC_PROG_LIBTOOL

# LLVM
AC_PATH_PROG([LLVM_CONFIG], [llvm-config], [no])
[if test "$LLVM_CONFIG" = "no"; then]
    [echo "The llvm-config program was not found in the search path. Please ensure"]
    [echo "that it is installed and its directory is included in the search path."]
    [echo "Then run configure again before attempting to build faf."]
    [exit 1]
[fi]

LLVM_VERSION=`$LLVM_CONFIG --version | sed 's/svn.*//g'`
LLVM_CFLAGS=`$LLVM_CONFIG --cppflags | sed -e 's/-DNDEBUG\>//g' -e 's/-pedantic//g'`
LLVM_LIBS="`$LLVM_CONFIG --libs`"
LLVM_LDFLAGS=`$LLVM_CONFIG --ldflags`
# Quadrigaphs: @<:@ gives you [ and @>:@ gives you ].
CPPFLAGS=`$LLVM_CONFIG --version | sed 's/\(@<:@0-9@>:@*\).\(@<:@0-9@>:@*\).*/-DLLVM_MAJOR=\\1 -DLLVM_MINOR=\\2/'`
AC_SUBST([LLVM_CFLAGS])
AC_SUBST([LLVM_LIBS])
AC_SUBST([LLVM_LDFLAGS])
AC_SUBST([LLVM_VERSION])

# Initialize the test suite.
AC_CONFIG_TESTDIR(tests)
AC_CONFIG_FILES([tests/Makefile tests/atlocal])
AM_MISSING_PROG([AUTOM4TE], [autom4te])
# Needed by tests/atlocal.in.
AC_SUBST([O0CFLAGS], [`echo $CFLAGS   | sed 's/-O[[0-9]] *//'`])

# libedit
PKG_CHECK_MODULES(LIBEDIT, libedit)

AC_CONFIG_FILES([
    canal.spec
    Makefile
    lib/Makefile
])

AC_OUTPUT
