# - Find vorbisenc
# Find the native vorbisenc includes and libraries
#
#  VORBIS_INCLUDE_DIRS - where to find vorbis.h, etc.
#  VORBIS_LIBRARIES    - List of libraries when using vorbis.
#  VORBIS_FOUND        - True if vorbis found.

if (Vorbis_Vorbis_INCLUDE_DIR)
	# Already in cache, be silent
	set (Vorbis_FIND_QUIETLY TRUE)
endif ()

set (Vorbis_Vorbis_FIND_QUIETLY TRUE)
set (Vorbis_Enc_FIND_QUIETLY TRUE)
set (Vorbis_File_FIND_QUIETLY TRUE)

find_package (Ogg QUIET)

find_package (PkgConfig QUIET)
pkg_check_modules (PC_Vorbis_Vorbis QUIET vorbis)
pkg_check_modules (PC_Vorbis_Enc QUIET vorbisenc)
pkg_check_modules (PC_Vorbis_File QUIET vorbisfile)

set (Vorbis_VERSION ${PC_VORBIS_VERSION})

find_path (Vorbis_Vorbis_INCLUDE_DIR vorbis/codec.h
	HINTS
		${PC_Vorbis_Vorbis_INCLUDEDIR}
		${PC_Vorbis_Vorbis_INCLUDE_DIRS}
		${Vorbis_ROOT}
	)

find_path (Vorbis_Enc_INCLUDE_DIR vorbis/vorbisenc.h
	HINTS
		${PC_Vorbis_Enc_INCLUDEDIR}
		${PC_Vorbis_Enc_INCLUDE_DIRS}
		${Vorbis_ROOT}
	)

find_path (Vorbis_File_INCLUDE_DIR vorbis/vorbisfile.h
	HINTS
		${PC_Vorbis_File_INCLUDEDIR}
		${PC_Vorbis_File_INCLUDE_DIRS}
		${Vorbis_ROOT}
	)

find_library (Vorbis_Vorbis_LIBRARY
	NAMES
		vorbis
		vorbis_static
		libvorbis
		libvorbis_static
	HINTS
		${PC_Vorbis_Vorbis_LIBDIR}
		${PC_Vorbis_Vorbis_LIBRARY_DIRS}
		${Vorbis_ROOT}
	)

find_library (Vorbis_Enc_LIBRARY
	NAMES
		vorbisenc
		vorbisenc_static
		libvorbisenc
		libvorbisenc_static
	HINTS
		${PC_Vorbis_Enc_LIBDIR}
		${PC_Vorbis_Enc_LIBRARY_DIRS}
		${Vorbis_ROOT}
	)

find_library (Vorbis_File_LIBRARY
	NAMES
		vorbisfile
		vorbisfile_static
		libvorbisfile
		libvorbisfile_static
	HINTS
		${PC_Vorbis_File_LIBDIR}
		${PC_Vorbis_File_LIBRARY_DIRS}
		${Vorbis_ROOT}
	)

include (FindPackageHandleStandardArgs)

find_package_handle_standard_args (Vorbis_Vorbis
	REQUIRED_VARS
		Vorbis_Vorbis_LIBRARY
		Vorbis_Vorbis_INCLUDE_DIR
		Ogg_FOUND
	NAME_MISMATCHED
	)

find_package_handle_standard_args (Vorbis_Enc
	REQUIRED_VARS
		Vorbis_Enc_LIBRARY
		Vorbis_Enc_INCLUDE_DIR
		Vorbis_Vorbis_FOUND
	NAME_MISMATCHED
	)

find_package_handle_standard_args (Vorbis_File
	REQUIRED_VARS
		Vorbis_File_LIBRARY
		Vorbis_File_INCLUDE_DIR
		Vorbis_Vorbis_FOUND
	NAME_MISMATCHED
	)

if (Vorbis_Vorbis_FOUND)
	set (Vorbis_Vorbis_INCLUDE_DIRS ${VORBIS_INCLUDE_DIR})
	set (Vorbis_Vorbis_LIBRARIES ${VORBIS_LIBRARY} ${OGG_LIBRARIES})
    if (NOT TARGET Vorbis::vorbis)
		add_library (Vorbis::vorbis UNKNOWN IMPORTED)
		set_target_properties (Vorbis::vorbis PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${Vorbis_Vorbis_INCLUDE_DIR}"
			IMPORTED_LOCATION "${Vorbis_Vorbis_LIBRARY}"
			INTERFACE_LINK_LIBRARIES Ogg::ogg
		)
	endif ()

	if (Vorbis_Enc_FOUND)
		set (Vorbis_Enc_INCLUDE_DIRS ${Vorbis_Enc_INCLUDE_DIR})
		set (Vorbis_Enc_LIBRARIES ${Vorbis_Enc_LIBRARY} ${Vorbis_Enc_LIBRARIES})
		if (NOT TARGET Vorbis::vorbisenc)
			add_library (Vorbis::vorbisenc UNKNOWN IMPORTED)
			set_target_properties (Vorbis::vorbisenc PROPERTIES
				INTERFACE_INCLUDE_DIRECTORIES "${Vorbis_Enc_INCLUDE_DIR}"
				IMPORTED_LOCATION "${Vorbis_Enc_LIBRARY}"
				INTERFACE_LINK_LIBRARIES Vorbis::vorbis
			)
		endif ()
	endif ()

	if (Vorbis_File_FOUND)
		set (Vorbis_File_INCLUDE_DIRS ${Vorbis_File_INCLUDE_DIR})
		set (Vorbis_File_LIBRARIES ${Vorbis_File_LIBRARY} ${Vorbis_File_LIBRARIES})
		if (NOT TARGET Vorbis::vorbisfile)
			add_library (Vorbis::vorbisfile UNKNOWN IMPORTED)
			set_target_properties (Vorbis::vorbisfile PROPERTIES
				INTERFACE_INCLUDE_DIRECTORIES "${Vorbis_File_INCLUDE_DIR}"
				IMPORTED_LOCATION "${Vorbis_File_LIBRARY}"
				INTERFACE_LINK_LIBRARIES Vorbis::vorbis
			)
		endif ()
	endif ()

endif ()

find_package_handle_standard_args (Vorbis
	REQUIRED_VARS
		Vorbis_Vorbis_LIBRARY
		Vorbis_Vorbis_INCLUDE_DIR
		Ogg_FOUND
	HANDLE_COMPONENTS
	VERSION_VAR Vorbis_VERSION)

mark_as_advanced (Vorbis_Vorbis_INCLUDE_DIR Vorbis_Vorbis_LIBRARY)
mark_as_advanced (Vorbis_Enc_INCLUDE_DIR Vorbis_Enc_LIBRARY)
mark_as_advanced (Vorbis_File_INCLUDE_DIR Vorbis_File_LIBRARY)
