
find_package (PkgConfig)
include (FindPackageHandleStandardArgs)

function (find_libogg return_name)
	pkg_check_modules (PC_LIBOGG QUIET libogg)
	set (LIBOGG_DEFINITIONS ${PC_LIBOGG_CFLAGS_OTHER})

	find_path (LIBOGG_INCLUDE_DIR ogg/ogg.h
				HINTS ${PC_LIBOGG_INCLUDEDIR} ${PC_LIBOGG_INCLUDE_DIRS}
				PATH_SUFFIXES libogg)

	find_library (LIBOGG_LIBRARY NAMES ogg libogg
					HINTS ${PC_LIBOGG_LIBDIR} ${PC_LIBOGG_LIBRARY_DIRS})

	find_package_handle_standard_args (LibOgg  DEFAULT_MSG
										LIBOGG_LIBRARY LIBOGG_INCLUDE_DIR)

	mark_as_advanced (LIBOGG_INCLUDE_DIR LIBOGG_LIBRARY)

	set (LIBOGG_LIBRARIES ${LIBOGG_LIBRARY} PARENT_SCOPE)
	set (LIBOGG_INCLUDE_DIRS ${LIBOGG_INCLUDE_DIR} PARENT_SCOPE)
	set (${return_name} ${LIBOGG_FOUND} PARENT_SCOPE)
	endfunction (find_libogg)


function (find_libvorbis return_name)
	pkg_check_modules (PC_LIBVORBIS QUIET libvorbis)
	set (LIBVORBIS_DEFINITIONS ${PC_LIBVORBIS_CFLAGS_OTHER})

	find_path (LIBVORBIS_INCLUDE_DIR vorbis/codec.h
				HINTS ${PC_LIBVORBIS_INCLUDEDIR} ${PC_LIBVORBIS_INCLUDE_DIRS}
				PATH_SUFFIXES libvorbis)

	find_library (LIBVORBIS_LIBRARY NAMES vorbis libvorbis
					HINTS ${PC_LIBVORBIS_LIBDIR} ${PC_LIBVORBIS_LIBRARY_DIRS})

	find_package_handle_standard_args (LibVorbis  DEFAULT_MSG
										LIBVORBIS_LIBRARY LIBVORBIS_INCLUDE_DIR)

	mark_as_advanced (LIBVORBIS_INCLUDE_DIR LIBVORBIS_LIBRARY)

	set (LIBVORBIS_LIBRARIES ${LIBVORBIS_LIBRARY} PARENT_SCOPE)
	set (LIBVORBIS_INCLUDE_DIRS ${LIBVORBIS_INCLUDE_DIR} PARENT_SCOPE)
	set (${return_name} ${LIBVORBIS_FOUND} PARENT_SCOPE)
	endfunction (find_libvorbis)


function (find_libflac return_name)
	pkg_check_modules (PC_LIBFLAC QUIET libFLAC)
	set (LIBFLAC_DEFINITIONS ${PC_LIBFLAC_CFLAGS_OTHER})

	find_path (LIBFLAC_INCLUDE_DIR FLAC/all.h
				HINTS ${PC_LIBFLAC_INCLUDEDIR} ${PC_LIBFLAC_INCLUDE_DIRS}
				PATH_SUFFIXES libFLAC)

	find_library (LIBFLAC_LIBRARY NAMES FLAC libFLAC
					HINTS ${PC_LIBFLAC_LIBDIR} ${PC_LIBFLAC_LIBRARY_DIRS})

	find_package_handle_standard_args (LibFlac  DEFAULT_MSG
										LIBFLAC_LIBRARY LIBFLAC_INCLUDE_DIR)

	mark_as_advanced (LIBFLAC_INCLUDE_DIR LIBFLAC_LIBRARY)

	set (LIBFLAC_LIBRARIES ${LIBFLAC_LIBRARY} PARENT_SCOPE)
	set (LIBFLAC_INCLUDE_DIRS ${LIBFLAC_INCLUDE_DIR} PARENT_SCOPE)
	set (${return_name} ${LIBFLAC_FOUND} PARENT_SCOPE)
	endfunction (find_libflac)


function (find_external_xiph_libs return_name include_dirs external_libs)
	find_libogg (LIBOGG_FOUND)
	find_libvorbis (LIBVORBIS_FOUND)
	find_libflac (LIBFLAC_FOUND)

	set (name 1)
	set (includes "")
	set (libs "")

	if (LIBOGG_FOUND AND LIBVORBIS_FOUND AND LIBFLAC_FOUND)
		set (${name} 1)

		if (NOT (LIBOGG_INCLUDE_DIR STREQUAL "/usr/include"))
			set (${includes} "${includes} ${LIBOGG_INCLUDE_DIR}")
			endif ()
		if (NOT (LIBVORBIS_INCLUDE_DIR STREQUAL "/usr/include"))
			set (${includes} "${includes} ${LIBVORBIS_INCLUDE_DIR}")
			endif ()
		if (NOT (LIBFLAC_INCLUDE_DIR STREQUAL "/usr/include"))
			set (${includes} "${includes} ${LIBFLAC_INCLUDE_DIR}")
			endif ()

		set (libs "FLAC;vorbis;vorbisenc;ogg")
		endif ()

	set (${return_name} ${name} PARENT_SCOPE)
	set (${include_dirs} "${includes}" PARENT_SCOPE)
	set (${external_libs} "${libs}" PARENT_SCOPE)
	endfunction (find_external_xiph_libs)
