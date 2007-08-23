dnl @synopsis AC_OCTAVE_BUILD
dnl
dnl Check programs and headers required for building octave plugins.
dnl @version 1.0	Aug 23 2007
dnl @author Erik de Castro Lopo <erikd AT mega-nerd DOT com>
dnl
dnl Permission to use, copy, modify, distribute, and sell this file for any
dnl purpose is hereby granted without fee, provided that the above copyright
dnl and this permission notice appear in all copies.  No representations are
dnl made about the suitability of this software for any purpose.  It is
dnl provided "as is" without express or implied warranty.


AC_DEFUN([AC_OCTAVE_BUILD],
[

dnl Default to no.
OCTAVE_BUILD=no

AC_OCTAVE_VERSION
AC_MKOCTFILE_VERSION
AC_OCTAVE_CONFIG_VERSION

prog_concat="$ac_cv_prog_HAVE_OCTAVE$ac_cv_prog_HAVE_OCTAVE_CONFIG$ac_cv_prog_HAVE_MKOCTFILE"

if test "x$prog_concat" = "xyesyesyes" ; then
	if test "x$OCTAVE_VERSION" != "x$MKOCTFILE_VERSION" ; then
		AC_MSG_WARN([** Mismatch between versions of octave and mkoctfile. **])
		AC_MSG_WARN([** Octave libsndfile modules will not be built.       **])
	elif test "x$OCTAVE_VERSION" != "x$OCTAVE_CONFIG_VERSION" ; then
		AC_MSG_WARN([** Mismatch between versions of octave and octave-config. **])
		AC_MSG_WARN([** Octave libsndfile modules will not be built.           **])
	else
		OCTAVE_ODIR=`$OCTAVE_CONFIG --oct-site-dir`
		OCTAVE_MDIR=`$OCTAVE_CONFIG --m-site-dir`

		AC_MSG_RESULT([retrieving compile and link flags from $MKOCTFILE])
		MKOCTFILE_CC=`$MKOCTFILE -p CC`
		MKOCTFILE_CFLAGS=`$MKOCTFILE -p CFLAGS`
		MKOCTFILE_CPPFLAGS=`$MKOCTFILE -p CPPFLAGS`
		MKOCTFILE_CPICFLAG=`$MKOCTFILE -p CPICFLAG`
		MKOCTFILE_LDFLAGS=`$MKOCTFILE -p LDFLAGS`
		MKOCTFILE_LIBS=`$MKOCTFILE -p LIBS`
		MKOCTFILE_CXX=`$MKOCTFILE -p CXX`
		MKOCTFILE_CXXFLAGS=`$MKOCTFILE -p CXXFLAGS`
		MKOCTFILE_CXXPICFLAG=`$MKOCTFILE -p CXXPICFLAG`

		OCTAVE_BUILD=yes
		AC_MSG_RESULT([building octave libsndfile module... $OCTAVE_BUILD])
		fi
	fi

AC_SUBST(OCTAVE_ODIR)
AC_SUBST(OCTAVE_MDIR)

AC_SUBST(MKOCTFILE_CC)
AC_SUBST(MKOCTFILE_CFLAGS)
AC_SUBST(MKOCTFILE_CPPFLAGS)
AC_SUBST(MKOCTFILE_CPICFLAG)
AC_SUBST(MKOCTFILE_CXX)
AC_SUBST(MKOCTFILE_CXXFLAGS)
AC_SUBST(MKOCTFILE_CXXPICFLAG)

])# AC_OCTAVE_BUILD
