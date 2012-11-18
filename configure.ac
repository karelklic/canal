AC_PREREQ([2.60])
AC_INIT([canal],[0])
AM_INIT_AUTOMAKE([-Wall])
AM_MAINTAINER_MODE
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CXX
AC_USE_SYSTEM_EXTENSIONS
AC_CONFIG_HEADERS([lib/Config.h:lib/ConfigAutotools.h.in])

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

CXXFLAGS="$CXXFLAGS -Wall"

# Initialize libtool.
AC_DISABLE_STATIC
LT_INIT
AC_SUBST([LIBTOOL_DEPS])

# LLVM and Clang
AC_PATH_PROG_CRITICAL([LLVM_CONFIG], [llvm-config])
AC_PATH_PROG_CRITICAL([LLVM_LINK], [llvm-link])
AC_PATH_PROG_CRITICAL([CLANG], [clang])
LLVM_VERSION=`$LLVM_CONFIG --version | sed 's/svn.*//g'`
AC_SUBST([LLVM_VERSION])
LLVM_CFLAGS=`$LLVM_CONFIG --cppflags | sed -e 's/-DNDEBUG\>//g' -e 's/-pedantic//g'`
AC_SUBST([LLVM_CFLAGS])
LLVM_LIBS=`$LLVM_CONFIG --libs`
AC_SUBST([LLVM_LIBS])
LLVM_LDFLAGS=`$LLVM_CONFIG --ldflags`
AC_SUBST([LLVM_LDFLAGS])

# See Canal bug #101.
# Quadrigraphs are used here: @<:@ gives you [ and @:>@ gives you ].
CPPFLAGS+=`$LLVM_CONFIG --version | sed 's/\(@<:@0-9@:>@*\).\(@<:@0-9@:>@*\).*/-DLLVM_MAJOR=\1 -DLLVM_MINOR=\2/'`

# Capture compile-time LLVM NDEBUG status.
#
# Including the LLVM headers without the NDEBUG macro defined and then
# linking to LLVM dynamic libraries compiled with NDEBUG leads to
# runtime errors that are extremely difficult to analyze.
$LLVM_CONFIG --cppflags | grep \\-DNDEBUG > /dev/null
LLVM_DEBUG=$?
if test "x$LLVM_DEBUG" = "x0"; then
   AC_DEFINE([LLVM_WITH_NDEBUG], [1], [LLVM has been compiled with NDEBUG])
fi

# readline
AC_CHECK_HEADERS([readline/readline.h], [],
	[AC_MSG_ERROR([Please install readline development headers])])

# elfutils
AC_CHECK_HEADERS([gelf.h libelf.h])
AC_CHECK_LIB([elf], [main])

# pdfLaTeX
AC_PATH_PROG_OPTIONAL([PDFLATEX], [pdflatex],
		      [echo "Documentation will not be build.  If you want to build"]
		      [echo "the documentation, ensure that pdflatex is installed"]
		      [echo "and included in the search path."])

# LaTeX packages
AC_LATEX_PACKAGE_OPTIONAL([LATEX_GALOIS], [galois])
AC_LATEX_PACKAGE_OPTIONAL([LATEX_FLOAT], [float])
AC_LATEX_PACKAGE_OPTIONAL([LATEX_TXFONTS], [txfonts])
AC_LATEX_PACKAGE_OPTIONAL([LATEX_AMSSYMB], [amssymb])
AC_LATEX_PACKAGE_OPTIONAL([LATEX_AMSMATH], [amsmath])
AC_LATEX_PACKAGE_OPTIONAL([LATEX_IMPORT], [import])
AC_LATEX_PACKAGE_OPTIONAL([LATEX_GRAPHICX], [graphicx])
AC_LATEX_PACKAGE_OPTIONAL([LATEX_HYPERREF], [hyperref])
AC_LATEX_PACKAGE_OPTIONAL([LATEX_MAKEIDX], [makeidx])
AC_LATEX_PACKAGE_OPTIONAL([LATEX_A4WIDE], [a4wide])
AC_LATEX_PACKAGE_OPTIONAL([LATEX_INPUTENC], [inputenc])

# Doxygen
AC_PATH_PROG_OPTIONAL([DOXYGEN], [doxygen],
		      [echo "Documentation will not be build.  If you want to"]
		      [echo "build the documentation, ensure that doxygen is installed"]
		      [echo "and included in the search path."])

# Dot (part of graphviz)
AC_PATH_PROG_OPTIONAL([DOT], [dot],
		      [echo "Documentation will not be build.  If you want to"]
		      [echo "build the documentation, ensure that dot is installed"]
		      [echo "and included in the search path."])

AM_CONDITIONAL([BUILD_DOCUMENTATION], test \( "$DOXYGEN" != "no" \) \
				      	-a \( "$DOT" != "no" \) \
					-a \( "$PDFLATEX" != "no" \) \
					-a \( "$LATEX_GALOIS" != "no" \) \
					-a \( "$LATEX_FLOAT" != "no" \) \
					-a \( "$LATEX_TXFONTS" != "no" \) \
					-a \( "$LATEX_AMSSYMB" != "no" \) \
					-a \( "$LATEX_AMSMATH" != "no" \) \
					-a \( "$LATEX_IMPORT" != "no" \) \
					-a \( "$LATEX_GRAPHICX" != "no" \) \
					-a \( "$LATEX_HYPERREF" != "no" \) \
					-a \( "$LATEX_MAKEIDX" != "no" \) \
					-a \( "$LATEX_A4WIDE" != "no" \) \
					-a \( "$LATEX_INPUTENC" != "no" \))

# Initialize the test suite.
AC_CONFIG_TESTDIR(tests)
AM_MISSING_PROG([AUTOM4TE], [autom4te])
# Needed by tests/atlocal.in.
AC_SUBST([O0CFLAGS], [`echo $CFLAGS | sed 's/-O[[0-9]] *//g'`])

AC_CONFIG_FILES([
    canal.spec
    Makefile
    docs/Makefile
    lib/Makefile
    tests/Makefile
    tests/atlocal
    tool/Makefile
    wrappers/config.py
    wrappers/Makefile
])

AC_OUTPUT
