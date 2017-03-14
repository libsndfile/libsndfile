# - Find Speex
# Find the native Speex includes and libraries
#
#  SPEEX_INCLUDE_DIRS - where to find speex.h, etc.
#  SPEEX_LIBRARIES    - List of libraries when using Speex.
#  Speex_FOUND        - True if Speex found.

if (SPEEX_INCLUDE_DIR)
	set (SPEEX_FIND_QUIETLY TRUE)
endif ()

find_package (PkgConfig QUIET)
pkg_check_modules(PC_SPEEX QUIET speex)

find_path (SPEEX_INCLUDE_DIR speex/speex.h HINTS ${PC_SPEEX_INCLUDEDIR} ${PC_SPEEX_INCLUDE_DIRS} ${SPEEX_ROOT} PATH_SUFFIXES include)
find_library (SPEEX_LIBRARY NAMES speex HINTS ${PC_SPEEX_LIBDIR} ${PC_SPEEX_LIBRARY_DIRS} ${SPEEX_ROOT} PATH_SUFFIXES lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Speex DEFAULT_MSG SPEEX_INCLUDE_DIR SPEEX_LIBRARY)

mark_as_advanced (SPEEX_INCLUDE_DIR SPEEX_LIBRARY)

set(SPEEX_INCLUDE_DIRS ${SPEEX_INCLUDE_DIR})
set(SPEEX_LIBRARIES ${SPEEX_LIBRARY})

