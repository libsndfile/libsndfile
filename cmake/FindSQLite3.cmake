# - Find SQLITE3
# Find the native SQLITE3 includes and libraries
#
#  SQLITE3_INCLUDE_DIRS - where to find sqlite.h, etc.
#  SQLITE3_LIBRARIES    - List of libraries when using SQLITE3.
#  SQLITE3_FOUND        - True if SQLITE3 found.

if(SQLITE3_INCLUDE_DIR)
    # Already in cache, be silent
    set(SQLITE3_FIND_QUIETLY TRUE)
endif(SQLITE3_INCLUDE_DIR)

find_package (PkgConfig QUIET)
pkg_check_modules(PC_SQLITE3 QUIET sqlite3)

find_path(SQLITE3_INCLUDE_DIR sqlite3.h HINTS ${PC_SQLITE3_INCLUDEDIR} ${PC_SQLITE3_INCLUDE_DIRS} ${SQLITE3_ROOT})

find_library (SQLITE3_LIBRARY NAMES sqlite3 HINTS ${PC_SQLITE3_LIBDIR} ${PC_SQLITE3_LIBRARY_DIRS} ${SQLITE3_ROOT} PATH_SUFFIXES lib)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SQLITE3 DEFAULT_MSG SQLITE3_INCLUDE_DIR SQLITE3_LIBRARY)

if (SQLITE3_FOUND)
	set (SQLITE3_INCLUDE_DIRS ${SQLITE3_INCLUDE_DIR})
	set (SQLITE3_LIBRARIES ${SQLITE3_LIBRARY})
endif (SQLITE3_FOUND)

mark_as_advanced(SQLITE3_INCLUDE_DIR SQLITE3_LIBRARY)
