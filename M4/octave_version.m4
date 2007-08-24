dnl @synopsis AC_OCTAVE_VERSION
dnl
dnl Find the version of Octave.
dnl @version 1.0	Aug 23 2007
dnl @author Erik de Castro Lopo <erikd AT mega-nerd DOT com>
dnl
dnl Permission to use, copy, modify, distribute, and sell this file for any 
dnl purpose is hereby granted without fee, provided that the above copyright 
dnl and this permission notice appear in all copies.  No representations are
dnl made about the suitability of this software for any purpose.  It is 
dnl provided "as is" without express or implied warranty.
dnl

AC_DEFUN([AC_OCTAVE_VERSION],
[

AC_ARG_WITH(octave,
	[  --with-octave           choose the octave version], [ with_octave=$withval ])

test -z "$with_octave" && with_octave=octave

AC_CHECK_PROG(HAVE_OCTAVE,$with_octave,yes,no)

if test "x$ac_cv_prog_HAVE_OCTAVE" = "xyes" ; then
	OCTAVE=$with_octave
	OCTAVE_EVAL(OCTAVE_VERSION,OCTAVE_VERSION)
	fi

AC_SUBST(OCTAVE)
AC_SUBST(OCTAVE_VERSION)

])# AC_OCTAVE_VERSION

