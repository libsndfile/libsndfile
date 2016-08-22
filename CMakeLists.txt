#
# Initial CMake and project setup
#

# lowest version supporting C_STANDARD property
cmake_minimum_required (VERSION 3.1.3)

## System-wide includes

include (GNUInstallDirs)

# Options
option (BUILD_SHARED_LIBS "Build shared libraries" OFF)
option (ENABLE_TESTING "Enable testing" OFF)

# Set software information
project(libsndfile VERSION 1.0.28.0)
SET(LIB_VERSION_PATCH pre1)
set(LIB_VERSION ${PROJECT_VERSION}pre1)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
set_directory_properties(PROPERTIES EP_PREFIX ${CMAKE_BINARY_DIR}/library-build)

#
# Setup definitions
#
include(${CMAKE_MODULE_PATH}/check.cmake)

#
#  Find external libs
#

include(${CMAKE_MODULE_PATH}/external_libs.cmake)

#
# Setup configuration
#

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.cmake ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/sndfile.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/sndfile.h)
if (WIN32)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/libsndfile.def.in.cmake ${CMAKE_CURRENT_SOURCE_DIR}/src/${PROJECT_NAME}-${PROJECT_VERSION_MAJOR}.def)
if (BUILD_SHARED_LIBS)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/version-metadata.rc.in.cmake ${CMAKE_CURRENT_SOURCE_DIR}/src/version-metadata.rc)
endif ()
endif ()

#
# libsndfile
#

# Public libsndfile headers
set (include_HEADERS src/sndfile.hh)
set (nodist_include_HEADERS src/sndfile.h)

# Private libsndfile headers
set (noinst_HEADERS
	src/common.h
	src/sfconfig.h
	src/sfendian.h
	src/wavlike.h
	src/sf_unistd.h
	src/ogg.h
	src/chanmap.h)

# Common libsndfile sources
set (COMMON 
	src/common.c
	src/file_io.c
	src/command.c
	src/pcm.c
	src/ulaw.c
	src/alaw.c
	src/float32.c
	src/double64.c
	src/ima_adpcm.c
	src/ms_adpcm.c
	src/gsm610.c
	src/dwvw.c
	src/vox_adpcm.c
	src/interleave.c
	src/strings.c
	src/dither.c
	src/cart.c
	src/broadcast.c
	src/audio_detect.c
 	src/ima_oki_adpcm.c
	src/ima_oki_adpcm.h
	src/alac.c
	src/chunk.c
	src/ogg.c
	src/chanmap.c
	src/id3.c)
if (WIN32)
	list (APPEND COMMON src/windows.c)
if (BUILD_SHARED_LIBS)
	list (APPEND COMMON src/version-metadata.rc)
endif (BUILD_SHARED_LIBS)
endif (WIN32)
	
# File specific libsndfile sources
set (FILESPECIFIC
	src/sndfile.c
	src/aiff.c
	src/au.c
	src/avr.c
	src/caf.c
	src/dwd.c
	src/flac.c
	src/g72x.c
	src/htk.c
	src/ircam.c
	src/macos.c
	src/mat4.c
	src/mat5.c
	src/nist.c
	src/paf.c
	src/pvf.c
	src/raw.c
	src/rx2.c
	src/sd2.c
	src/sds.c
	src/svx.c
	src/txw.c
	src/voc.c
	src/wve.c
	src/w64.c
	src/wavlike.c
	src/wav.c
	src/xi.c
	src/mpc2k.c
	src/rf64.c
	src/ogg_vorbis.c
	src/ogg_speex.c
	src/ogg_pcm.c
	src/ogg_opus.c)
	
# GSM610 sources	
set (libgsm_SOURCES
	src/GSM610/config.h
	src/GSM610/gsm.h
	src/GSM610/gsm610_priv.h
	src/GSM610/add.c
	src/GSM610/code.c
	src/GSM610/decode.c
	src/GSM610/gsm_create.c
	src/GSM610/gsm_decode.c
	src/GSM610/gsm_destroy.c
	src/GSM610/gsm_encode.c
	src/GSM610/gsm_option.c
	src/GSM610/long_term.c
	src/GSM610/lpc.c
	src/GSM610/preprocess.c
	src/GSM610/rpe.c
	src/GSM610/short_term.c
	src/GSM610/table.c)
source_group ("GSM610" FILES ${libgsm_SOURCES})

# G72x sources
set (libg72x_SOURCES
	src/G72x/g72x.h
	src/G72x/g72x_priv.h
	src/G72x/g721.c
	src/G72x/g723_16.c
	src/G72x/g723_24.c
	src/G72x/g723_40.c
	src/G72x/g72x.c)
source_group ("G72x" FILES ${libg72x_SOURCES})

# ALAC sources

set (libalac_SOURCES
	src/ALAC/ALACAudioTypes.h
	src/ALAC/ALACBitUtilities.h
	src/ALAC/EndianPortable.h
	src/ALAC/aglib.h
	src/ALAC/dplib.h
	src/ALAC/matrixlib.h
	src/ALAC/alac_codec.h
	src/ALAC/shift.h
	src/ALAC/ALACBitUtilities.c
	src/ALAC/ag_dec.c
	src/ALAC/ag_enc.c
	src/ALAC/dp_dec.c
	src/ALAC/dp_enc.c
	src/ALAC/matrix_dec.c
	src/ALAC/matrix_enc.c
	src/ALAC/alac_decoder.c
	src/ALAC/alac_encoder.c)
source_group ("ALAC" FILES ${libalac_SOURCES})
		
# TODO: Private symbols (Win32 implemented)

# Headers to be installed on target machine
set (libsndfile_PUBLIC_HEADERS ${include_HEADERS} ${nodist_include_HEADERS})
set (libsndfile_SOURCES ${COMMON} ${FILESPECIFIC} ${noinst_HEADERS} ${libgsm_SOURCES} ${libg72x_SOURCES} ${libalac_SOURCES} ${libsndfile_PUBLIC_HEADERS})
if (WIN32)
list (APPEND libsndfile_SOURCES src/${PROJECT_NAME}-${PROJECT_VERSION_MAJOR}.def)
endif (WIN32)

add_library(sndfile	${libsndfile_SOURCES})
if (M_LIBRARY)
target_link_libraries (sndfile PUBLIC ${EXTERNAL_XIPH_LIBS} PRIVATE ${M_LIBRARY})
endif ()
set_property(TARGET sndfile PROPERTY C_STANDARD 99)
set_target_properties (sndfile
			PROPERTIES
			SOVERSION ${PROJECT_VERSION_MAJOR}
			VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_TWEAK}"
			PUBLIC_HEADER "${libsndfile_PUBLIC_HEADERS}")
if (WIN32)
	set_target_properties(sndfile PROPERTIES PREFIX "lib")
if (BUILD_SHARED_LIBS)
	set_target_properties(sndfile PROPERTIES OUTPUT_NAME "sndfile-${PROJECT_VERSION_MAJOR}")
else (NOT BUILD_SHARED_LIBS)
	set_target_properties(sndfile PROPERTIES OUTPUT_NAME "sndfile")
endif ()
endif (WIN32)
target_include_directories (sndfile PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
if (COMPILER_IS_GCC)
	target_compile_options(sndfile PRIVATE -Wall -Wextra)
endif ()

## Programs

set(PROGRAMS 	sndfile-cmp 
		sndfile-concat
		sndfile-convert
		sndfile-deinterleave
		sndfile-interleave
		sndfile-metadata-get
		sndfile-metadata-set
		sndfile-salvage
		sndfile-play
	)

foreach (program ${PROGRAMS})
	add_executable(${program}
			programs/common.h
			programs/common.c 
			programs/${program}.c
			)
	target_link_libraries(${program} PRIVATE sndfile)
if (COMPILER_IS_GCC)
	target_compile_options(${program} PRIVATE -Wall -Wextra)
endif ()
endforeach ()

# This is the BeOS version of sndfile-play. It needs to be compiled with the C++
# compiler.
if (BEOS OR HAIKU)
	add_executable (sndfile-play-beos programs/sndfile-play-beos.cpp)
	target_link_libraries(sndfile-play-beos PRIVATE sndfile)
if (COMPILER_IS_GCC)
	target_compile_options(sndfile-play-beos PRIVATE -Wall -Wextra)
endif (COMPILER_IS_GCC)
endif (BEOS OR HAIKU)

if (WIN32)
	target_link_libraries(sndfile-play PRIVATE Winmm.lib)
endif ()

if (${ALSA_FOUND})
	target_include_directories(sndfile-play PRIVATE ${ALSA_INCLUDE_DIRS})
	target_link_libraries(sndfile-play PRIVATE ${ALSA_LIBRARIES})
endif ()
if (${SNDIO_FOUND})
	target_include_directories(sndfile-play PRIVATE ${SNDIO_INCLUDE_DIRS})
	target_link_libraries(sndfile-play PRIVATE ${SNDIO_LIBRARIES})
endif ()

## Installation

install (TARGETS sndfile ${PROGRAMS} 
	RUNTIME DESTINATION			${CMAKE_INSTALL_BINDIR}
	ARCHIVE DESTINATION			${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION			${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION	${CMAKE_INSTALL_INCLUDEDIR})

if (Unix)
set (man_MANS
	man/sndfile-info.1
	man/sndfile-play.1
	man/sndfile-convert.1
	man/sndfile-cmp.1
	man/sndfile-metadata-get.1
#	man/sndfile-metadata-set.1
	man/sndfile-concat.1
	man/sndfile-interleave.1
#	man/sndfile-deinterleave.1
	man/sndfile-salvage.1)
install (FILES ${man_MANS} DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 PATTERN "*.1")
endif ()

#TODO: ENABLE_BOW_DOCS option
set (HTML_BGCOLOUR "black")
set (HTML_FGCOLOUR "white")
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/doc/libsndfile.css.in ${CMAKE_CURRENT_SOURCE_DIR}/doc/libsndfile.css)
set (html_DATA 
	doc/index.html
	doc/libsndfile.jpg
	doc/libsndfile.css
	doc/api.html
	doc/command.html
	doc/bugs.html
	doc/sndfile_info.html
	doc/new_file_type.HOWTO
	doc/win32.html
	doc/FAQ.html
	doc/lists.html
	doc/embedded_files.html
	doc/octave.html
	doc/dither.html
	doc/tutorial.html)
install (FILES ${html_DATA} DESTINATION ${CMAKE_INSTALL_DOCDIR}/${PROJECT_NAME})
	
## Testing

if (ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
else ()
    add_subdirectory(tests EXCLUDE_FROM_ALL)
endif ()

