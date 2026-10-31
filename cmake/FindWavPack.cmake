#[=======================================================================[.rst:
FindWavPack
-----------

Finds the WavPack library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``WavPack::WavPack``
  The WavPack library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``WavPack_FOUND``
  True if the system has the WavPack library.
``WavPack_VERSION``
  The version of WavPack that was found on the system.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``WavPack_INCLUDE_DIR``
  The directory containing ``wavpack/wavpack.h``.
``WavPack_LIBRARY``
  The path to the WavPack library.

#]=======================================================================]

if (WavPack_INCLUDE_DIR)
	# Already in cache, be silent
	set (WavPack_FIND_QUIETLY TRUE)
endif ()

if (CMAKE_FIND_PACKAGE_PREFER_CONFIG)
	# WavPack's config-version requires the same minor version, so check
	# the APIs below instead of requesting 5.0 and rejecting newer releases.
	find_package (WavPack CONFIG QUIET)
endif ()

if (TARGET WavPack::WavPack)
	set (_WavPack_LIBRARY WavPack::WavPack)
else ()
	find_package (PkgConfig QUIET)
	pkg_check_modules (PC_WAVPACK QUIET wavpack>=5.0.0)

	find_path (WavPack_INCLUDE_DIR wavpack/wavpack.h
		HINTS
			${PC_WAVPACK_INCLUDEDIR}
			${PC_WAVPACK_INCLUDE_DIRS}
			${WavPack_ROOT}
		)

	# WavPack's own CMake build names the MSVC static library libwavpack
	# and the import library wavpackdll.
	find_library (WavPack_LIBRARY
		NAMES
			wavpack
			libwavpack
			wavpackdll
		HINTS
			${PC_WAVPACK_LIBDIR}
			${PC_WAVPACK_LIBRARY_DIRS}
			${WavPack_ROOT}
		)

	if (PC_WAVPACK_FOUND)
		set (WavPack_VERSION ${PC_WAVPACK_VERSION})
	endif ()
	set (_WavPack_LIBRARY "${WavPack_LIBRARY}")
endif ()

# The public header has no version macro. Check the 5.0 APIs even when
# pkg-config is unavailable, or its version check rejected an older library.
include (CMakePushCheckState)
if (CMAKE_C_COMPILER_LOADED)
	include (CheckCSourceCompiles)
else ()
	include (CheckCXXSourceCompiles)
endif ()
macro (_WavPack_check_api)
	unset (WavPack_HAS_REQUIRED_API CACHE)
	unset (WavPack_HAS_REQUIRED_API)
	if (CMAKE_C_COMPILER_LOADED)
		check_c_source_compiles ("${_WavPack_API_TEST}" WavPack_HAS_REQUIRED_API)
	else ()
		check_cxx_source_compiles ("${_WavPack_API_TEST}" WavPack_HAS_REQUIRED_API)
	endif ()
endmacro ()
set (_WavPack_LINK_LIBRARIES)
set (_WavPack_API_TEST "
#include <wavpack/wavpack.h>
int main (void)
{
	WavpackStreamReader64 reader = { 0 };
	WavpackConfig config = { 0 };
	char error [80];
	WavpackContext *context = WavpackOpenFileInputEx64 (&reader, 0, 0, error, 0, 0);
	config.qmode = QMODE_SIGNED_BYTES;
	return WavpackGetQualifyMode (context)
		+ (int) WavpackGetNumSamples64 (context)
		+ WavpackSetConfiguration64 (context, &config, -1, 0)
		+ WavpackSeekSample64 (context, 0);
}")
set (WavPack_HAS_REQUIRED_API FALSE)
if (_WavPack_LIBRARY AND (WavPack_INCLUDE_DIR OR TARGET WavPack::WavPack))
	cmake_push_check_state (RESET)
	if (NOT TARGET WavPack::WavPack)
		set (CMAKE_REQUIRED_INCLUDES "${WavPack_INCLUDE_DIR}")
	endif ()
	set (CMAKE_REQUIRED_LIBRARIES "${_WavPack_LIBRARY}")
	_WavPack_check_api ()
	if (NOT WavPack_HAS_REQUIRED_API AND UNIX)
		# Static WavPack may need libm and, since 5.8, pthreads. Add these
		# only if the link check demonstrates that they are needed.
		find_library (WavPack_MATH_LIBRARY m)
		if (WavPack_MATH_LIBRARY)
			list (APPEND _WavPack_LINK_LIBRARIES "${WavPack_MATH_LIBRARY}")
			set (CMAKE_REQUIRED_LIBRARIES "${_WavPack_LIBRARY}" ${_WavPack_LINK_LIBRARIES})
			_WavPack_check_api ()
		endif ()
		if (NOT WavPack_HAS_REQUIRED_API)
			find_package (Threads QUIET)
			if (Threads_FOUND)
				list (APPEND _WavPack_LINK_LIBRARIES Threads::Threads)
				set (CMAKE_REQUIRED_LIBRARIES "${_WavPack_LIBRARY}" ${_WavPack_LINK_LIBRARIES})
				_WavPack_check_api ()
			endif ()
		endif ()
	endif ()
	cmake_pop_check_state ()
endif ()

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (WavPack
	REQUIRED_VARS
		_WavPack_LIBRARY
		WavPack_HAS_REQUIRED_API
	VERSION_VAR
		WavPack_VERSION
	)

if (WavPack_FOUND AND NOT TARGET WavPack::WavPack)
	add_library (WavPack::WavPack UNKNOWN IMPORTED)
	set_target_properties (WavPack::WavPack
		PROPERTIES
			IMPORTED_LOCATION "${WavPack_LIBRARY}"
			INTERFACE_INCLUDE_DIRECTORIES "${WavPack_INCLUDE_DIR}"
			INTERFACE_LINK_LIBRARIES "${_WavPack_LINK_LIBRARIES}"
		)
elseif (WavPack_FOUND AND _WavPack_LINK_LIBRARIES)
	set_property (TARGET WavPack::WavPack APPEND PROPERTY INTERFACE_LINK_LIBRARIES "${_WavPack_LINK_LIBRARIES}")
endif ()

mark_as_advanced (WavPack_INCLUDE_DIR WavPack_LIBRARY WavPack_MATH_LIBRARY)
