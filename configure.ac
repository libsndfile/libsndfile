# Copyright (C) 1999-2004 Erik de Castro Lopo (erikd AT mega-nerd DOT com).

dnl Require autoconf version
AC_PREREQ(2.53)

AC_INIT(libsndfile,1.0.11,erikd@mega-nerd.com)
AC_CONFIG_SRCDIR([src/sndfile.c])
AC_CANONICAL_TARGET([])
AM_INIT_AUTOMAKE($PACKAGE_NAME,$PACKAGE_VERSION)
AM_CONFIG_HEADER(src/config.h)

#------------------------------------------------------------------------------------
# Rules for library version information:
#
#  1. Start with version information of `0:0:0' for each libtool library.
#  2. Update the version information only immediately before a public release of
#     your software. More frequent updates are unnecessary, and only guarantee
#     that the current interface number gets larger faster.
#  3. If the library source code has changed at all since the last update, then
#     increment revision (`c:r:a' becomes `c:r+1:a').
#  4. If any interfaces have been added, removed, or changed since the last update,
#     increment current, and set revision to 0.
#  5. If any interfaces have been added since the last public release, then increment
#     age.
#  6. If any interfaces have been removed since the last public release, then set age
#     to 0.

SHARED_VERSION_INFO="1:11:0"

AC_PROG_CC
AM_PROG_LIBTOOL

AC_CHECK_PROG(autogen, autogen, yes, no)

AC_PROG_INSTALL
AC_PROG_LN_S

AC_HEADER_STDC

AC_CHECK_HEADERS(endian.h)
AC_CHECK_HEADERS(byteswap.h)
AC_HEADER_SYS_WAIT

AC_CHECK_DECLS(S_IRGRP)
AC_DEFINE_UNQUOTED([HAVE_DECL_S_IRGRP],${HAVE_DECL_S_IRGRP},
		[Set to 1 if S_IRGRP is defined.])

#====================================================================================
# Check for support of the struct hack.

AC_C99_FLEXIBLE_ARRAY

if test x$ac_cv_c99_flexible_array = xyes ; then
	AC_DEFINE([HAVE_FLEXIBLE_ARRAY],1, [Set to 1 if the compile supports the struct hack.])
else
	AC_MSG_WARN([[*** This compiler does not support the 1999 ISO C Standard ***]])
	AC_MSG_WARN([[*** feature known as the flexible array struct member.     ***]])
	AC_DEFINE([HAVE_FLEXIBLE_ARRAY],0)
	fi

#====================================================================================
# Couple of initializations here. Fill in real values later.

SHLIB_VERSION_ARG=""

#====================================================================================
# Finished checking, handle options.

AC_ARG_ENABLE(experimental,
	AC_HELP_STRING([--enable-experimental], [enable experimental code]),
	ac_arg_experimental=yes, ac_arg_experimental=no)

EXPERIMENTAL_CODE=0
if test x$ac_arg_experimental = xyes ; then
	EXPERIMENTAL_CODE=1
	fi
AC_DEFINE_UNQUOTED([ENABLE_EXPERIMENTAL_CODE],${EXPERIMENTAL_CODE}, [Set to 1 to enable experimental code.])

AC_ARG_ENABLE(gcc-werror,
	AC_HELP_STRING([--enable-gcc-werror], [enable -Werror in all Makefiles]),
	ac_arg_gcc_werror=yes, ac_arg_gcc_werror=no)

AC_ARG_ENABLE(gcc-pipe,
	AC_HELP_STRING([--disable-gcc-pipe], [disable gcc -pipe option]),
	ac_arg_gcc_pipe="N", ac_arg_gcc_pipe="Y")

AC_ARG_ENABLE(gcc-opt,
	AC_HELP_STRING([--disable-gcc-opt], [disable gcc optimisations]),
	ac_arg_gcc_opt="N", ac_arg_gcc_opt="Y")

AC_ARG_ENABLE(cpu-clip,
	AC_HELP_STRING([--disable-cpu-clip], [disable tricky cpu specific clipper]),
	ac_arg_cpu_clip="N", ac_arg_cpu_clip="Y")

AC_ARG_ENABLE(bow-docs,
	AC_HELP_STRING([--enable-bow-docs], [enable black-on-white html docs]),
	ac_arg_bow_docs="Y", ac_arg_bow_docs="N")

#====================================================================================
# Check types and their sizes.

AC_CHECK_SIZEOF(short,2)
AC_CHECK_SIZEOF(int,4)
AC_CHECK_SIZEOF(long,4)
AC_CHECK_SIZEOF(float,4)
AC_CHECK_SIZEOF(double,4)
AC_CHECK_SIZEOF(void*,8)
AC_CHECK_SIZEOF(size_t,4)
AC_CHECK_SIZEOF(int64_t,8)
AC_CHECK_SIZEOF(long long,8)

#====================================================================================
# Find an appropriate type for sf_count_t.
# On systems supporting files larger than 2 Gig, sf_count_t must be a 64 bit value.
# Unfortunately there is more than one way of ensuring this so need to do some
# pretty rigourous testing here.

unset ac_cv_sizeof_off_t

AC_CHECK_SIZEOF(off_t,1)	# Fake default value.

case "$target_os" in
	mingw32*)
		TYPEOF_SF_COUNT_T="__int64"
		SF_COUNT_MAX="0x7FFFFFFFFFFFFFFFLL"
		SIZEOF_SF_COUNT_T=8
		;;
	*)
		if test "x$ac_cv_sizeof_off_t" = "x8" ; then
			# If sizeof (off_t) is 8, no further checking is needed.
			TYPEOF_SF_COUNT_T="off_t"
			SF_COUNT_MAX="0x7FFFFFFFFFFFFFFFLL"
			SIZEOF_SF_COUNT_T=8
		else
			# Check for common 64 bit file offset types.
			AC_CHECK_SIZEOF(loff_t,1)	# Fake default value.
			AC_CHECK_SIZEOF(off64_t,1)	# Fake default value.
		
			TYPEOF_SF_COUNT_T="unknown"
			if test "x$ac_cv_sizeof_loff_t" = "x8" ; then
				TYPEOF_SF_COUNT_T="loff_t"
				SIZEOF_SF_COUNT_T=8
			elif test "x$ac_cv_sizeof_off64_t" = "x8" ; then
				TYPEOF_SF_COUNT_T="off64_t"
				SIZEOF_SF_COUNT_T=8
				fi
		
			# Save the old sizeof (off_t) value  and then unset it to see if it
			# changes when Large File Support is enabled.
		
			pre_largefile_sizeof_off_t=$ac_cv_sizeof_off_t
			unset ac_cv_sizeof_off_t
		
			AC_SYS_EXTRA_LARGEFILE
		
			if test "x$ac_cv_sys_largefile_CFLAGS" = "xno" ; then
				ac_cv_sys_largefile_CFLAGS=""
				fi
			if test "x$ac_cv_sys_largefile_LDFLAGS" = "xno" ; then
				ac_cv_sys_largefile_LDFLAGS=""
				fi
			if test "x$ac_cv_sys_largefile_LIBS" = "xno" ; then
				ac_cv_sys_largefile_LIBS=""
				fi
		
			AC_CHECK_SIZEOF(off_t,1)	# Fake default value.
		
			if test "x$ac_cv_sizeof_off_t" = "x8" ; then
				SF_COUNT_MAX="0x7FFFFFFFFFFFFFFFLL"
			elif test "x$ac_cv_sizeof_off_t" = "x$pre_largefile_sizeof_off_t" ; then
				AC_MSG_WARN([[This machine does not seem to support 64 bit file offsets.]])
				TYPEOF_SF_COUNT_T="off_t"
				SIZEOF_SF_COUNT_T=$ac_cv_sizeof_off_t
			elif test "x$TYPEOF_SF_COUNT_T" = "xunknown" ; then
				echo
				echo "*** The configure process has determined that this system is capable"
				echo "*** of Large File Support but has not been able to find a type which"
				echo "*** is an unambiguous 64 bit file offset."
				echo "*** Please contact the author to help resolve this problem."
				echo
				AC_MSG_ERROR([[Bad file offset type.]])
				fi
			fi
		;;
	esac

if test $SIZEOF_SF_COUNT_T = 4 ; then
	SF_COUNT_MAX="0x7FFFFFFF"
	fi

AC_DEFINE_UNQUOTED([TYPEOF_SF_COUNT_T],${TYPEOF_SF_COUNT_T}, [Set to long if unknown.])
AC_SUBST(TYPEOF_SF_COUNT_T)

AC_DEFINE_UNQUOTED([SIZEOF_SF_COUNT_T],${SIZEOF_SF_COUNT_T}, [Set to sizeof (long) if unknown.])
AC_SUBST(SIZEOF_SF_COUNT_T)

AC_DEFINE_UNQUOTED([SF_COUNT_MAX],${SF_COUNT_MAX}, [Set to maximum allowed value of sf_count_t type.])
AC_SUBST(SF_COUNT_MAX)

AC_CHECK_TYPES(ssize_t)
AC_CHECK_SIZEOF(ssize_t,4)

#====================================================================================
# Determine endian-ness of target processor.

AC_C_FIND_ENDIAN

AC_DEFINE_UNQUOTED(CPU_IS_BIG_ENDIAN, ${ac_cv_c_big_endian},
	[Target processor is big endian.])
AC_DEFINE_UNQUOTED(CPU_IS_LITTLE_ENDIAN, ${ac_cv_c_little_endian},
	[Target processor is little endian.])

#====================================================================================
# Check for functions.

AC_CHECK_FUNCS(malloc calloc realloc free)
AC_CHECK_FUNCS(open read write lseek pread pwrite)
AC_CHECK_FUNCS(fstat ftruncate fsync fdatasync)
AC_CHECK_FUNCS(snprintf vsnprintf)
AC_CHECK_FUNCS(gmtime gmtime_r)
AC_CHECK_FUNCS(mmap getpagesize)

AC_CHECK_LIB([m],floor)
AC_CHECK_FUNCS(floor ceil fmod)

case "$target_os" in
	darwin* | rhapsody*)
		AC_MSG_WARN([[Not using native lrint() and lrintf() because they are missing on 10.1.]])
		;;
	*)
		AC_C99_FUNC_LRINT
		AC_C99_FUNC_LRINTF

		if test "x$ac_cv_c99_lrint" = "xno" ; then
			if test "x$ac_cv_c99_lrintf" = "xno" ; then
				AC_MSG_WARN([[*** Missing C99 standard functions lrint() and lrintf().]])
				AC_MSG_WARN([[*** This may cause benign compiler warnings on some systems (ie Solaris).]])
				fi
			fi
		;;
	esac

#====================================================================================
# Determine if the processor can do clipping on float to int conversions.

if test x$ac_arg_cpu_clip = "xY" ; then
	AC_C_CLIP_MODE
else
	echo "checking processor clipping capabilities... disabled"
	ac_cv_c_clip_positive=0
	ac_cv_c_clip_negative=0
	fi

AC_DEFINE_UNQUOTED(CPU_CLIPS_POSITIVE, ${ac_cv_c_clip_positive},
	[Target processor clips on positive float to int conversion.])
AC_DEFINE_UNQUOTED(CPU_CLIPS_NEGATIVE, ${ac_cv_c_clip_negative},
	[Target processor clips on negative float to int conversion.])

#====================================================================================
# Target OS specific stuff.

OS_SPECIFIC_CFLAGS=""
OS_SPECIFIC_LINKS=""
os_is_win32=0
os_is_macosx=0

case "$target_os" in
	darwin* | rhapsody*)
		os_is_macosx=1
		OS_SPECIFIC_CFLAGS="-fpascal-strings -I/Developer/Headers/FlatCarbon"
		OS_SPECIFIC_LINKS="-framework CoreAudio"
		;;
	mingw32*)
		os_is_win32=1
		OS_SPECIFIC_LINKS="-lwinmm"
		;;
	esac

AC_DEFINE_UNQUOTED(OS_IS_WIN32, ${os_is_win32}, [Set to 1 if compiling for Win32])
AC_DEFINE_UNQUOTED(OS_IS_MACOSX, ${os_is_macosx}, [Set to 1 if compiling for MacOSX])

#====================================================================================
# Check for ALSA.

ALSA_LIBS=""

AC_CHECK_HEADERS(alsa/asoundlib.h)
if test x$ac_cv_header_alsa_asoundlib_h = xyes ; then
	ALSA_LIBS="-lasound"
	fi

#====================================================================================
# Test for sanity when cross-compiling.

if test x$cross_compiling = xyes ; then
	AC_MSG_WARN([[******************************************************************]])
	AC_MSG_WARN([[*** We are cross-compiling, so have to assume sizeof (short) == 2 ]])
	AC_MSG_WARN([[*** and sizeof (int) == 4. If this is not the case there is no    ]])
	AC_MSG_WARN([[*** chance of this working. Please contact the mantainer.         ]])
	AC_MSG_WARN([[******************************************************************]])
	fi

if test $ac_cv_sizeof_short != 2 ; then
	AC_MSG_WARN([[******************************************************************]])
	AC_MSG_WARN([[*** sizeof (short) != 2.                                          ]])
	AC_MSG_WARN([[******************************************************************]])
	fi

if test $ac_cv_sizeof_int != 4 ; then
	AC_MSG_WARN([[******************************************************************]])
	AC_MSG_WARN([[*** sizeof (int) != 4                                             ]])
	AC_MSG_WARN([[******************************************************************]])
	fi

if test $ac_cv_sizeof_float != 4 ; then
	AC_MSG_WARN([[******************************************************************]])
	AC_MSG_WARN([[*** sizeof (float) != 4.                                          ]])
	AC_MSG_WARN([[******************************************************************]])
	fi

if test $ac_cv_sizeof_double != 8 ; then
	AC_MSG_WARN([[******************************************************************]])
	AC_MSG_WARN([[*** sizeof (double) != 8.                                         ]])
	AC_MSG_WARN([[******************************************************************]])
	fi

if test x"$ac_cv_prog_autogen" = "xno" ; then
	AC_MSG_WARN([[Touching files in directory tests/.]])
	touch tests/*.c tests/*.h
	fi

#====================================================================================
# Settings for the HTML documentation.

htmldocdir=$prefix/share/doc/libsndfile1-dev/html

if test $prefix = "NONE" ; then
	htmldocdir=/usr/local/share/doc/libsndfile1-dev/html
else
	htmldocdir=$prefix/share/doc/libsndfile1-dev/html
	fi

if test x$ac_arg_bow_docs = "xY" ; then
	HTML_BGCOLOUR="white"
	HTML_FGCOLOUR="black"
else
	HTML_BGCOLOUR="black"
	HTML_FGCOLOUR="white"
	fi

#====================================================================================
# Now use the information from the checking stage.

if test x$ac_cv_c_compiler_gnu = xyes ; then
	CFLAGS="$CFLAGS -W -Wall -Wstrict-prototypes -Wmissing-prototypes -Waggregate-return -Wcast-align -Wcast-qual -Wnested-externs -Wshadow -Wbad-function-cast -Wwrite-strings"
	# -Wpointer-arith -Wundef -Wmissing-declarations -Winline -Wconversion"
	if test "$ac_arg_gcc_opt" = "N" ; then
		temp_CFLAGS=`echo $CFLAGS | sed "s/O2/O0/"`
		CFLAGS=$temp_CFLAGS
		AC_MSG_WARN([[*** Compiler optimisations switched off. ***]])
		fi

	# OS specific tweaks.
	case "$target_os" in
		darwin* | rhapsody*)
			# Disable -Wall, -pedantic and -Wshadow for Apple Darwin/Rhapsody.
			# System headers on these systems are broken.
			temp_CFLAGS=`echo $CFLAGS | sed "s/-Wall -pedantic//" | sed "s/-Wshadow//" | sed "s/-Waggregate-return//"`
			CFLAGS=$temp_CFLAGS
			SHLIB_VERSION_ARG="-Wl,-exported_symbols_list -Wl,\$(srcdir)/Symbols.darwin"
			;;
		linux*)
			SHLIB_VERSION_ARG="-Wl,--version-script=\$(srcdir)/Symbols.linux"
			;;
		*)
			;;
		esac
	if test x$ac_arg_gcc_pipe != "xN" ; then
	 	CFLAGS="$CFLAGS -pipe"
	 	fi

	if test x$ac_arg_gcc_werror = "xyes" ; then
		CFLAGS="-Werror $CFLAGS"
		fi

	AC_DEFINE([COMPILER_IS_GCC],1, [Set to 1 if the compile is GNU GCC.])
	GCC_MAJOR_VERSION=`$CC -dumpversion | sed "s/\..*//"`
	AC_DEFINE_UNQUOTED([GCC_MAJOR_VERSION],${GCC_MAJOR_VERSION}, [Major version of GCC or 3 otherwise.])
	fi

CFLAGS="$CFLAGS $OS_SPECIFIC_CFLAGS"

if test x"$CFLAGS" = x ; then
	echo "Error in configure script. CFLAGS has been screwed up."
	exit
	fi

AC_SUBST(htmldocdir)
AC_SUBST(HTML_BGCOLOUR)
AC_SUBST(HTML_FGCOLOUR)

AC_SUBST(SHLIB_VERSION_ARG)
AC_SUBST(SHARED_VERSION_INFO)
AC_SUBST(OS_SPECIFIC_CFLAGS)
AC_SUBST(OS_SPECIFIC_LINKS)
AC_SUBST(ALSA_LIBS)
AC_SUBST(ENABLE_EXPERIMENTAL_CODE)

AC_SUBST(COMPILER_IS_GCC)
AC_SUBST(GCC_MAJOR_VERSION)

dnl The following line causes the libtool distributed with the source 
dnl to be replaced if the build system has a more recent version.
AC_SUBST(LIBTOOL_DEPS)

AC_CONFIG_FILES([ \
			src/sndfile.h src/Makefile src/GSM610/Makefile src/G72x/Makefile \
			man/Makefile examples/Makefile tests/Makefile \
			doc/Makefile doc/libsndfile.css \
			Win32/Makefile Win32/Makefile.mingw \
			MacOS9/Makefile Octave/Makefile \
			Makefile libsndfile.spec sndfile.pc \
			])
AC_OUTPUT

#====================================================================================

AC_MSG_RESULT([
-=-=-=-=-=-=-=-=-=-= Configuration Complete =-=-=-=-=-=-=-=-=-=-

  Configuration summary :

    Version : ..................... ${VERSION}
    Experimental code : ........... ${ac_arg_experimental}
])

if test x$ac_cv_c_compiler_gnu = xyes ; then
	echo -e "  Tools :\n"
	echo "    Compiler is GCC : ............. ${ac_cv_c_compiler_gnu}"
	echo "    GCC major version : ........... ${GCC_MAJOR_VERSION}"
	if test $GCC_MAJOR_VERSION -lt 3 ; then
		echo -e "\n    ** This compiler version allows applications to write"
		echo "    ** to static strings within the library."
		echo "    ** Compile with GCC version 3.X to avoid this problem."
		fi
	fi

if test $libdir = "\${exec_prefix}/lib" ; then
	libdir="$prefix/lib"
	fi

if test $bindir = "\${exec_prefix}/bin" ; then
	bindir="$prefix/bin"
	fi

AC_MSG_RESULT([[
  Installation directories :

    Library directory : ........... $libdir
    Program directory : ........... $bindir
    Pkgconfig directory : ......... $libdir/pkgconfig
    HTML docs directory : ......... $htmldocdir
]])

if test x$prefix != "x/usr" ; then
	echo "Compiling some other packages against libsndfile may require"
	echo -e "the addition of \"$libdir/pkgconfig\" to the"
	echo -e "PKG_CONFIG_PATH environment variable.\n"
	fi

#====================================================================================

ifelse(dnl

 Do not edit or modify anything in this comment block.
 The arch-tag line is a file identity tag for the GNU Arch
 revision control system.

 arch-tag: 6391b316-6cfc-43c2-a18a-8defdc4ee359

)dnl
