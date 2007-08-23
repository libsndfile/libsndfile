dnl Evaluate an expression in octave
dnl
dnl OCTAVE_EVAL(expr,var) -> var=expr
dnl
dnl Stolen from octave-forge

AC_DEFUN([OCTAVE_EVAL],
[
AC_MSG_CHECKING([for $1 in $OCTAVE])
$2=`echo "disp($1)" | $OCTAVE -qf`
AC_MSG_RESULT($$2)
AC_SUBST($2)
]) # OCTAVE_EVAL

