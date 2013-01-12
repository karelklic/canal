AC_PREREQ([2.60])
AC_INIT([canal],[2])
AM_INIT_AUTOMAKE([-Wall])
AM_MAINTAINER_MODE
AC_PROG_CXX
AC_USE_SYSTEM_EXTENSIONS
AC_CONFIG_HEADERS([lib/Config.h:lib/ConfigAutotools.h.in])
AC_PROG_LN_S
AC_LANG(C++)

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

CXXFLAGS="$CXXFLAGS -Wall"

# Macros
# Program testing macro
AC_DEFUN([AC_PATH_PROG_CRITICAL],[
AC_PATH_PROG($1, $2, [no])
[if test "$$1" = "no"; then]
    [echo "The $2 program was not found in the search path. Please ensure"]
    [echo "that it is installed and its directory is included in the search path."]
    [echo "Then run configure again before attempting to build Canal."]
    [exit 1]
[fi]
AC_SUBST($1)
])

# Program testing macro
AC_DEFUN([AC_PATH_PROG_OPTIONAL],[
AC_PATH_PROG($1, $2, [no])
AM_CONDITIONAL([HAVE_$1], test "$$1" != "no")
[if test "$$1" = "no"; then]
    [echo "The $2 program was not found in the search path."]
    $3
    [echo "Then run configure again before attempting to build Canal."]
[fi]
AC_SUBST($1)
])

# LaTeX testing macro
AC_DEFUN([_AC_LATEX_TEST],[
rm -rf .tmps_latex 
mkdir .tmps_latex 
cd .tmps_latex 
$2="no"; export $2;
cat > testconf.tex << \EOF
$1
EOF
if test "$PDFLATEX" != "no"; then
  cat testconf.tex | $PDFLATEX 2>&1 1>/dev/null && $2=yes; export $2;
fi
cd .. 
rm -rf .tmps_latex 
])

# LaTeX package testing macro
AC_DEFUN([AC_LATEX_PACKAGE],[
AC_CACHE_CHECK([for $2 latex package in class book],[ac_cv_latex_]$2,[
_AC_LATEX_TEST([
\documentclass{book}
\usepackage{$2}
\begin{document}
\end{document}
],[ac_cv_latex_]$2)
])
export $1=$[ac_cv_latex_]$2
AC_SUBST($1)
])

# LaTeX package testing macro
AC_DEFUN([AC_LATEX_PACKAGE_CRITICAL],[
AC_LATEX_PACKAGE($1, $2)
[if test "$$1" = "no"; then]
    [echo "The $2 LaTeX library was not found. Please ensure that it is "]
    [echo "installed and its directory is included in the LaTeX search path."]
    [echo "Then run configure again before attempting to build Canal."]
    [exit 1]
[fi]
AC_SUBST($1)
])

# LaTeX package testing macro
AC_DEFUN([AC_LATEX_PACKAGE_OPTIONAL],[
AC_LATEX_PACKAGE($1, $2)
AM_CONDITIONAL([HAVE_$1], test "$$1" != "no")
[if test "$$1" = "no"; then]
    [echo "The $2 LaTeX library was not found."]
    [echo "Documentation will not be build.  If you want to"]
    [echo "build the documentation, ensure that $2 is installed"]
    [echo "and included in the LaTeX search path."]
    [echo "Then run configure again before attempting to build Canal."]
[fi]
AC_SUBST($1)
])


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
LLVM_LDFLAGS=`$LLVM_CONFIG --ldflags`
AC_SUBST([LLVM_LDFLAGS])
# Quadrigraphs are used here: @<:@ gives you [ and @:>@ gives you ].
LLVM_VERSION_MAJOR=`$LLVM_CONFIG --version | sed 's/\(@<:@0-9@:>@*\).\(@<:@0-9@:>@*\).*/\1/'`
AC_DEFINE_UNQUOTED([LLVM_VERSION_MAJOR], [$LLVM_VERSION_MAJOR], [LLVM major version])
LLVM_VERSION_MINOR=`$LLVM_CONFIG --version | sed 's/\(@<:@0-9@:>@*\).\(@<:@0-9@:>@*\).*/\2/'`
AC_DEFINE_UNQUOTED([LLVM_VERSION_MINOR], [$LLVM_VERSION_MINOR], [LLVM minor version])

LDFLAGS=$LLVM_LDFLAGS

# Prefer LLVM shared library.  If that is not available, use static libraries.
AC_CHECK_LIB([LLVM-$LLVM_VERSION_MAJOR.$LLVM_VERSION_MINOR], [main], [llvm_lib_found=yes], [llvm_lib_found=no])
if test "$clang_lib_found" = no
then
  LLVM_LIBS=`$LLVM_CONFIG --libs`
else
  LLVM_LIBS="-lLLVM-$LLVM_VERSION_MAJOR.$LLVM_VERSION_MINOR"
fi

AC_SUBST([LLVM_LIBS])

# Clang shared library provides just the limited C interface, so it can not be used.
# We look for the static libraries.
AC_CHECK_LIB([clangBasic], [main], [clang_lib_found=yes], [clang_lib_found=no])
CPPFLAGS=$LLVM_CFLAGS
AC_CHECK_HEADERS([clang/Basic/Version.h], [clang_header_found=yes], [clang_header_found=no])

if test "$clang_lib_found" = "yes" -a "$clang_header_found" = "yes"
then
    AC_DEFINE(HAVE_CLANG, [], [Have Clang])
    CLANG_LIBS="-lclangFrontendTool -lclangFrontend -lclangDriver -lclangSerialization -lclangCodeGen -lclangParse -lclangSema  -lclangAnalysis -lclangIndex -lclangRewrite -lclangAST -lclangLex -lclangBasic"
else
    CLANG_LIBS=""
fi

AC_SUBST([CLANG_LIBS])


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
])

AC_OUTPUT
