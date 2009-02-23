dnl Make automake/libtool output more friendly to humans
dnl
dnl SHAVE_INIT([shavedir])
dnl
dnl shavedir: the directory where the shave scripts are, it defaults to
dnl           $(top_builddir)
dnl
dnl * SHAVE_INIT should be called late in your configure.(ac|in) file (just
dnl   before AC_CONFIG_FILE/AC_OUTPUT is perfect.  This macro rewrites CC and
dnl   LIBTOOL, you don't want the configure tests to have these variables
dnl   re-defined.
dnl * This macro requires GNU make's -s option.

AC_DEFUN([SHAVE_INIT],
[
  dnl enable/disable shave
  AC_ARG_ENABLE([shave],
    AS_HELP_STRING([--enable-shave],
                   [use shave to make the build pretty [[default=no]]]),,
    [enable_shave=yes])

  if test x"$enable_shave" = xyes; then
    dnl where can we find the shave scripts?
    m4_if([$1],,
      [shavedir='$(top_builddir)'],
      [shavedir='$(top_builddir)'/$1])
    AC_SUBST(shavedir)

    dnl make is now quiet
    AC_SUBST([MAKEFLAGS], [-s])
    AC_SUBST([AM_MAKEFLAGS], ['`test -z $V && echo -s`'])

    dnl we need sed
    AC_CHECK_PROG(SED,sed,sed,false)

    dnl substitute libtool
    SHAVE_SAVED_LIBTOOL=$LIBTOOL
    AC_SUBST(SHAVE_SAVED_LIBTOOL)
    LIBTOOL="\$(SHELL) \$(shavedir)/shave-libtool '\$(SHAVE_SAVED_LIBTOOL)'"
    AC_SUBST(LIBTOOL)

    dnl substitute cc/cxx
    SHAVE_SAVED_CC=$CC
    SHAVE_SAVED_CXX=$CXX
    AC_SUBST(SHAVE_SAVED_CC)
    AC_SUBST(SHAVE_SAVED_CXX)
    CC="\$(SHELL) \$(shavedir)/shave cc '\$(SHAVE_SAVED_CC)'"
    CXX="\$(SHELL) \$(shavedir)/shave cxx '\$(SHAVE_SAVED_CXX)'"
    AC_SUBST(CC)
    AC_SUBST(CXX)

    V=@
  else
    V=1
  fi
  Q='$(V:1=)'
  AC_SUBST(V)
  AC_SUBST(Q)
])

