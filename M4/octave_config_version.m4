dnl @synopsis AC_OCTAVE_CONFIG_VERSION
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

AC_DEFUN([AC_OCTAVE_CONFIG_VERSION],
[

AC_ARG_WITH(octave-config,
	[  --with-octave-config    choose the octave-config version], [ with_octave_config=$withval ])

test -z "$with_octave_config" && with_octave_config=octave-config

AC_CHECK_PROG(HAVE_OCTAVE_CONFIG,$with_octave_config,yes,no)

if test "x$ac_cv_prog_HAVE_OCTAVE_CONFIG" = "xyes" ; then
	OCTAVE_CONFIG=$with_octave_config
	AC_MSG_CHECKING([for version of $OCTAVE_CONFIG])
	OCTAVE_CONFIG_VERSION=`$OCTAVE_CONFIG --version`
	AC_MSG_RESULT($OCTAVE_CONFIG_VERSION)
	fi

AC_SUBST(OCTAVE_CONFIG)
AC_SUBST(OCTAVE_CONFIG_VERSION)

])# AC_OCTAVE_CONFIG_VERSION

