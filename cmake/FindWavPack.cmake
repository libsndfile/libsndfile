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

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (WavPack
	REQUIRED_VARS
		WavPack_LIBRARY
		WavPack_INCLUDE_DIR
	VERSION_VAR
		WavPack_VERSION
	)

if (WavPack_FOUND AND NOT TARGET WavPack::WavPack)
	add_library (WavPack::WavPack UNKNOWN IMPORTED)
	set_target_properties (WavPack::WavPack
		PROPERTIES
			IMPORTED_LOCATION "${WavPack_LIBRARY}"
			INTERFACE_INCLUDE_DIRECTORIES "${WavPack_INCLUDE_DIR}"
		)
endif ()

mark_as_advanced (WavPack_INCLUDE_DIR WavPack_LIBRARY)
