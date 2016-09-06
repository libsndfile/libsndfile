# - Find vorbis
# Find the native vorbis includes and libraries
#
#  VORBIS_INCLUDE_DIRS - where to find vorbis.h, etc.
#  VORBIS_LIBRARIES    - List of libraries when using vorbis(file).
#  VORBIS_FOUND        - True if vorbis found.

if(VORBIS_INCLUDE_DIR)
	# Already in cache, be silent
	set(VORBIS_FIND_QUIETLY TRUE)
endif(VORBIS_INCLUDE_DIR)

find_package (Ogg)
if (OGG_FOUND)
	find_package (PkgConfig)
	pkg_check_modules(PC_VORBIS QUIET vorbis)
	pkg_check_modules(PC_VORBISFILE QUIET vorbisfile)
	pkg_check_modules(PC_VORBISENC QUIET vorbisenc)

	find_path(VORBIS_INCLUDE_DIR vorbis/codec.h vorbis/vorbisfile.h vorbis/vorbisfile.h
		HINTS ${PC_VORBIS_INCLUDEDIR} ${PC_VORBIS_INCLUDE_DIRS} ${VORBIS_ROOT}
		PATH_SUFFIXES include)
	# MSVC built vorbis may be named vorbis_static
	# The provided project files name the library with the lib prefix.
	find_library(VORBIS_LIBRARY
	    NAMES vorbis vorbis_static libvorbis libvorbis_static
		HINTS ${PC_VORBIS_LIBDIR} ${PC_VORBIS_LIBRARY_DIRS} ${VORBIS_ROOT}
		PATH_SUFFIXES lib)
	find_library(VORBISFILE_LIBRARY
	    NAMES vorbisfile vorbisfile_static libvorbisfile libvorbisfile_static
		HINTS ${PC_VORBISFILE_LIBDIR} ${PC_VORBISFILE_LIBRARY_DIRS} ${VORBIS_ROOT}
		PATH_SUFFIXES lib)
	find_library(VORBISENC_LIBRARY
	    NAMES vorbisenc vorbisenc_static libvorbisenc libvorbisenc_static
		HINTS ${PC_VORBISENC_LIBDIR} ${PC_VORBISENC_LIBRARY_DIRS} ${VORBIS_ROOT}
		PATH_SUFFIXES lib)
	# Handle the QUIETLY and REQUIRED arguments and set VORBIS_FOUND
	# to TRUE if all listed variables are TRUE.
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(VORBIS DEFAULT_MSG
	    VORBIS_INCLUDE_DIR
	    VORBIS_LIBRARY VORBISFILE_LIBRARY VORBISENC_LIBRARY)
endif(OGG_FOUND)

if (VORBIS_FOUND)
	set (VORBIS_INCLUDE_DIRS VORBIS_INCLUDE_DIR)
	set (VORBIS_LIBRARIES ${VORBISFILE_LIBRARY} ${VORBIS_LIBRARY} ${VORBISENC_LIBRARY}
		${OGG_LIBRARY})
endif(VORBIS_FOUND)

mark_as_advanced(VORBIS_INCLUDE_DIR VORBIS_LIBRARY VORBISFILE_LIBRARY VORBISENC_LIBRARY)
