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
