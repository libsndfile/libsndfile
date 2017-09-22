# - Find mpg123
# Find the native mpg123 includes and libraries
#
#  MPG123_INCLUDE_DIRS - where to find mpg123.h, etc.
#  MPG123_LIBRARIES    - List of libraries when using mpg123.
#  Mpg123_FOUND        - True if mpg123 found.

if(MPG123_INCLUDE_DIR)
    # Already in cache, be silent
    set(MPG123_FIND_QUIETLY TRUE)
endif(MPG123_INCLUDE_DIR)

find_package (PkgConfig QUIET)
pkg_check_modules(PC_MPG123 QUIET mpg123)

find_path(
	MPG123_INCLUDE_DIR mpg123.h
	HINTS ${PC_MPG123_INCLUDEDIR} ${PC_MPG123_INCLUDE_DIRS} ${MPG123_ROOT}
	PATH_SUFFIXES include)
# MSVC built mpg123 may be named mpg123_static.
# The provided project files name the library with the lib prefix.
find_library(
	MPG123_LIBRARY NAMES mpg123 mpg123_static libmpg123 libmpg123_static
	HINTS ${PC_MPG123_LIBDIR} ${PC_MPG123_LIBRARY_DIRS} ${MPG123_ROOT}
	PATH_SUFFIXES lib)
# Handle the QUIETLY and REQUIRED arguments and set MPG123_FOUND
# to TRUE if all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
	Mpg123 DEFAULT_MSG MPG123_INCLUDE_DIR MPG123_LIBRARY)

if (MPG123_FOUND)
	set (MPG123_LIBRARIES ${MPG123_LIBRARY})
	set (MPG123_INCLUDE_DIRS ${MPG123_INCLUDE_DIR})
endif (MPG123_FOUND)

mark_as_advanced(MPG123_INCLUDE_DIR MPG123_LIBRARY)
