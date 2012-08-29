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

# Quadrigraphs: @<:@ gives you [ and @:>@ gives you ].
CPPFLAGS+=`$LLVM_CONFIG --version | sed 's/\(@<:@0-9@:>@*\).\(@<:@0-9@:>@*\).*/-DLLVM_MAJOR=\1 -DLLVM_MINOR=\2/'`

# readline
AC_CHECK_HEADERS([readline/readline.h], [],
	[AC_MSG_ERROR([Please install readline development headers])])

# pdfLaTeX
AC_PATH_PROG_CRITICAL([PDFLATEX], [pdflatex])

# LaTeX testing macro
AC_DEFUN([_AC_LATEX_TEST],[
rm -rf .tmps_latex 
mkdir .tmps_latex 
cd .tmps_latex 
$2="no"; export $2;
cat > testconf.tex << \EOF
$1
EOF
cat testconf.tex | $PDFLATEX 2>&1 1>/dev/null && $2=yes; export $2;
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

# LaTeX package critical testing macro
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

# LaTeX packages
AC_LATEX_PACKAGE_CRITICAL([LATEX_GALOIS], [galois])
AC_LATEX_PACKAGE_CRITICAL([LATEX_FLOAT], [float])
AC_LATEX_PACKAGE_CRITICAL([LATEX_TXFONTS], [txfonts])
AC_LATEX_PACKAGE_CRITICAL([LATEX_AMSSYMB], [amssymb])
AC_LATEX_PACKAGE_CRITICAL([LATEX_AMSMATH], [amsmath])
AC_LATEX_PACKAGE_CRITICAL([LATEX_IMPORT], [import])
AC_LATEX_PACKAGE_CRITICAL([LATEX_GRAPHICX], [graphicx])
AC_LATEX_PACKAGE_CRITICAL([LATEX_HYPERREF], [hyperref])
AC_LATEX_PACKAGE_CRITICAL([LATEX_MAKEIDX], [makeidx])
AC_LATEX_PACKAGE_CRITICAL([LATEX_A4WIDE], [a4wide])
AC_LATEX_PACKAGE_CRITICAL([LATEX_INPUTENC], [inputenc])

# Doxygen
AC_PATH_PROG_CRITICAL([DOXYGEN], [doxygen])

# dot (part of graphviz)
AC_PATH_PROG_CRITICAL([DOT], [dot])

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
    tests/integration/Makefile
    tests/unit/Makefile
    tool/Makefile
    wrappers/config.py
    wrappers/Makefile
])

AC_OUTPUT
