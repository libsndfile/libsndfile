#ifndef _CONFIG_H
#define _CONFIG_H

/* Set to 1 if the compile is GNU GCC. */
#cmakedefine01 COMPILER_IS_GCC

/* Target processor clips on negative float to int conversion. */
#cmakedefine01 CPU_CLIPS_NEGATIVE

/* Target processor clips on positive float to int conversion. */
#cmakedefine01 CPU_CLIPS_POSITIVE

/* Target processor is big endian. */
#cmakedefine01 CPU_IS_BIG_ENDIAN

/* Target processor is little endian. */
#cmakedefine01 CPU_IS_LITTLE_ENDIAN

/* Set to 1 to enable experimental code. */
#cmakedefine01 ENABLE_EXPERIMENTAL_CODE

/* Define to 1 if you have the <alsa/asoundlib.h> header file. */
#cmakedefine01 HAVE_ALSA_ASOUNDLIB_H

/* Define to 1 if you have the <byteswap.h> header file. */
#cmakedefine01 HAVE_BYTESWAP_H

/* Define to 1 if you have the `calloc' function. */
#cmakedefine01 HAVE_CALLOC

/* Define to 1 if you have the `ceil' function. */
#cmakedefine01 HAVE_CEIL

/* Set to 1 if S_IRGRP is defined. */
#cmakedefine01 HAVE_DECL_S_IRGRP

/* Define to 1 if you have the <endian.h> header file. */
#cmakedefine01 HAVE_ENDIAN_H

/* Will be set to 1 if flac, ogg and vorbis are available. */
#cmakedefine01 HAVE_EXTERNAL_XIPH_LIBS

/* Define to 1 if you have the `floor' function. */
#cmakedefine01 HAVE_FLOOR

/* Define to 1 if you have the `fmod' function. */
#cmakedefine01 HAVE_FMOD

/* Define to 1 if you have the `free' function. */
#cmakedefine01 HAVE_FREE

/* Define to 1 if you have the `fstat' function. */
#cmakedefine01 HAVE_FSTAT

/* Define to 1 if you have the `fstat64' function. */
#cmakedefine01 HAVE_FSTAT64

/* Define to 1 if you have the `fsync' function. */
#cmakedefine01 HAVE_FSYNC

/* Define to 1 if you have the `ftruncate' function. */
#cmakedefine01 HAVE_FTRUNCATE

/* Define to 1 if you have the `getpagesize' function. */
#cmakedefine01 HAVE_GETPAGESIZE

/* Define to 1 if you have the `gettimeofday' function. */
#cmakedefine01 HAVE_GETTIMEOFDAY

/* Define to 1 if you have the `gmtime' function. */
#cmakedefine01 HAVE_GMTIME

/* Define to 1 if you have the `gmtime_r' function. */
#cmakedefine01 HAVE_GMTIME_R

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine01 HAVE_INTTYPES_H

/* Define to 1 if you have the `m' library (-lm). */
#cmakedefine01 HAVE_LIBM

/* Define to 1 if you have the <locale.h> header file. */
#cmakedefine01 HAVE_LOCALE_H

/* Define to 1 if you have the `localtime' function. */
#cmakedefine01 HAVE_LOCALTIME

/* Define to 1 if you have the `localtime_r' function. */
#cmakedefine01 HAVE_LOCALTIME_R

/* Define to 1 if you have the `lround' function. */
#cmakedefine01 HAVE_LROUND

/* Define to 1 if you have the `lseek' function. */
#cmakedefine01 HAVE_LSEEK

/* Define to 1 if you have the `lseek64' function. */
#cmakedefine01 HAVE_LSEEK64

/* Define to 1 if you have the `malloc' function. */
#cmakedefine01 HAVE_MALLOC

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine01 HAVE_MEMORY_H

/* Define to 1 if you have the `mmap' function. */
#cmakedefine01 HAVE_MMAP

/* Define to 1 if you have the `open' function. */
#cmakedefine01 HAVE_OPEN

/* Define to 1 if you have the `pipe' function. */
#cmakedefine01 HAVE_PIPE

/* Define to 1 if you have the `read' function. */
#cmakedefine01 HAVE_READ

/* Define to 1 if you have the `realloc' function. */
#cmakedefine01 HAVE_REALLOC

/* Define to 1 if you have the `setlocale' function. */
#cmakedefine01 HAVE_SETLOCALE

/* Set to 1 if <sndio.h> is available. */
#cmakedefine01 HAVE_SNDIO_H

/* Define to 1 if you have the `snprintf' function. */
#cmakedefine01 HAVE_SNPRINTF

/* Set to 1 if you have libsqlite3. */
#cmakedefine01 HAVE_SQLITE3

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine01 HAVE_STDINT_H

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine01 HAVE_STDLIB_H

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine01 HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine01 HAVE_STRING_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine01 HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/time.h> header file. */
#cmakedefine01 HAVE_SYS_TIME_H

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine01 HAVE_SYS_TYPES_H

/* Define to 1 if you have <sys/wait.h> that is POSIX.1 compatible. */
#cmakedefine01 HAVE_SYS_WAIT_H

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine01 HAVE_UNISTD_H

/* Define to 1 if you have the `vsnprintf' function. */
#cmakedefine01 HAVE_VSNPRINTF

/* Define to 1 if you have the `waitpid' function. */
#cmakedefine01 HAVE_WAITPID

/* Define to 1 if you have the `write' function. */
#cmakedefine01 HAVE_WRITE

/* The host triplet of the compiled binary. */
#cmakedefine01 HOST_TRIPLET

/* Set to 1 if compiling for OpenBSD */
#cmakedefine01 OS_IS_OPENBSD

/* Set to 1 if compiling for Win32 */
#cmakedefine01 OS_IS_WIN32

/* Define to the address where bug reports for this package should be sent. */
#cmakedefine PACKAGE_BUGREPORT "@PACKAGE_BUGREPORT@"

/* Define to the full name of this package. */
#cmakedefine PACKAGE_NAME "@PACKAGE_NAME@"

/* Define to the full name and version of this package. */
#cmakedefine PACKAGE_STRING "@PACKAGE_STRING@"

/* Define to the one symbol short name of this package. */
#cmakedefine PACKAGE_TARNAME "@PACKAGE_TARNAME@"

/* Define to the home page for this package. */
#cmakedefine PACKAGE_URL "@PACKAGE_URL@"

/* Define to the version of this package. */
#cmakedefine PACKAGE_VERSION "@PACKAGE_VERSION@"

/* Set to maximum allowed value of sf_count_t type. */
#cmakedefine SF_COUNT_MAX @SF_COUNT_MAX@

/* The size of `double', as computed by sizeof. */
#cmakedefine SIZEOF_DOUBLE @SIZEOF_DOUBLE@

/* The size of `float', as computed by sizeof. */
#cmakedefine SIZEOF_FLOAT @SIZEOF_FLOAT@

/* The size of `int', as computed by sizeof. */
#cmakedefine SIZEOF_INT @SIZEOF_INT@

/* The size of `int64_t', as computed by sizeof. */
#cmakedefine SIZEOF_INT64_T @SIZEOF_INT64_T@

/* The size of `long', as computed by sizeof. */
#cmakedefine SIZEOF_LONG @SIZEOF_LONG@

/* The size of `long long', as computed by sizeof. */
#cmakedefine SIZEOF_LONG_LONG @SIZEOF_LONG_LONG@

/* The size of `off_t', as computed by sizeof. */
#cmakedefine SIZEOF_OFF_T @SIZEOF_OFF_T@

/* Set to sizeof (long) if unknown. */
#cmakedefine SIZEOF_SF_COUNT_T @SIZEOF_SF_COUNT_T@

/* The size of `short', as computed by sizeof. */
#cmakedefine SIZEOF_SHORT @SIZEOF_SHORT@

/* The size of `size_t', as computed by sizeof. */
#cmakedefine SIZEOF_SIZE_T @SIZEOF_SIZE_T@

/* The size of `void*', as computed by sizeof. */
#cmakedefine SIZEOF_VOIDP @SIZEOF_VOIDP@

/* The size of `wchar_t', as computed by sizeof. */
#cmakedefine SIZEOF_WCHAR_T @SIZEOF_WCHAR_T@

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine01 STDC_HEADERS @STDC_HEADERS@

/* Set to long if unknown. */
#cmakedefine TYPEOF_SF_COUNT_T @TYPEOF_SF_COUNT_T@

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
 #cmakedefine01 _ALL_SOURCE
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
 #cmakedefine01 _GNU_SOURCE
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
 #cmakedefine01 _POSIX_PTHREAD_SEMANTICS
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
 #cmakedefine01 _TANDEM_SOURCE
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
 #cmakedefine01 __EXTENSIONS__
#endif


/* Set to 1 to use the native windows API */
#cmakedefine01 USE_WINDOWS_API

/* Set to 1 if windows DLL is being built. */
#cmakedefine01 WIN32_TARGET_DLL

/* Target processor is big endian. */
#cmakedefine01 WORDS_BIGENDIAN

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
#cmakedefine _FILE_OFFSET_BITS @_FILE_OFFSET_BITS@

/* Define for large files, on AIX-style hosts. */
#cmakedefine _LARGE_FILES

/* Define to 1 if on MINIX. */
#cmakedefine01 _MINIX

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
#cmakedefine _POSIX_1_SOURCE

/* Define to 1 if you need to in order for `stat' and other things to work. */
#cmakedefine01 _POSIX_SOURCE

/* Set to 1 to use C99 printf/snprintf in MinGW. */
#cmakedefine01 __USE_MINGW_ANSI_STDIO

/* Define to `int' if <sys/types.h> does not define. */
#cmakedefine ssize_t

#endif
