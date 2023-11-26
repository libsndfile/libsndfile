# - Find WAVPACK
# Find the native WAVPACK includes and libraries
#
#  WAVPACK_INCLUDE_DIRS - where to find Wavpack headers.
#  WAVPACK_LIBRARIES    - List of libraries when using Wavpack.
#  WAVPACK_FOUND        - True if Wavpack found.
#  WAVPACK_DEFINITIONS  - Wavpack compile definitions

if (WAVPACK_INCLUDE_DIR)
    # Already in cache, be silent
    set (WAVPACK_FIND_QUIETLY TRUE)
endif ()

find_package (Ogg QUIET)

find_package (PkgConfig QUIET)
pkg_check_modules(PC_WAVPACK wavpack QUIET)

set(WAVPACK_VERSION ${PC_WAVPACK_VERSION})

find_path (WAVPACK_INCLUDE_DIR wavpack/wavpack.h
	HINTS
		${PC_WAVPACK_INCLUDEDIR}
		${PC_WAVPACK_INCLUDE_DIRS}
		${WAVPACK_ROOT}
	)

# MSVC built libraries can name them *_static, which is good as it
# distinguishes import libraries from static libraries with the same extension.
find_library (WAVPACK_LIBRARY
	NAMES
		wavpack
		libwavpack.lib
	HINTS
		${PC_WAVPACK_LIBDIR}
		${PC_WAVPACK_LIBRARY_DIRS}
		${WAVPACK_ROOT}
	)

# Handle the QUIETLY and REQUIRED arguments and set WAVPACK_FOUND to TRUE if
# all listed variables are TRUE.
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (Wavpack
	REQUIRED_VARS
		WAVPACK_LIBRARY
		WAVPACK_INCLUDE_DIR
	VERSION_VAR
        WAVPACK_VERSION
	)

if (WAVPACK_FOUND)
	set (WAVPACK_INCLUDE_DIRS ${WAVPACK_INCLUDE_DIR})
	set (WAVPACK_LIBRARIES ${WAVPACK_LIBRARY})
    if (NOT TARGET Wavpack::wavpack)
		add_library(Wavpack::wavpack UNKNOWN IMPORTED)
		set_target_properties(Wavpack::wavpack PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${WAVPACK_INCLUDE_DIR}"
			IMPORTED_LOCATION "${WAVPACK_LIBRARY}"
			)
	endif ()
endif ()

mark_as_advanced(WAVPACK_INCLUDE_DIR WAVPACK_LIBRARY)
