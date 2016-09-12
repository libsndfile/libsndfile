#
# Initial CMake and project setup
#

cmake_minimum_required (VERSION 3.1.3)

# Set software information
project(libsndfile VERSION 1.0.28)

# Variables

set (CPACK_PACKAGE_VERSION_MAJOR ${${PROJECT_VERSION}_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${${PROJECT_VERSION}_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${${PROJECT_VERSION}_PATCH})
set (CPACK_PACKAGE_VERSION "${PROJECT_VERSION}pre1")
set (CMAKE_C_STANDARD 99)
set (CMAKE_C_STANDARD_REQUIRED FALSE)

## System-wide includes

include (GNUInstallDirs)
include (CPack)
include (CTest)
include (FeatureSummary)

# Options
option (BUILD_STATIC_LIBS "Build static libraries" ON)
option (BUILD_SHARED_LIBS "Build shared libraries" ON)
option (DISABLE_EXTERNAL_LIBS "Disable use of FLAC, Ogg and Vorbis" OFF)
option (ENABLE_EXPERIMENTAL "Enable experimental code" OFF)

if ((NOT BUILD_STATIC_LIBS) AND (NOT BUILD_SHARED_LIBS))
message (WARNING "
Both options BUILD_STATIC_LIBS and BUILD_SHARED_LIBS are manually set to OFF. To prevent configuration error
BUILD_STATIC_LIBS is now set to ON.\n")
set (BUILD_STATIC_LIBS ON)
endif ()
option (ENABLE_TESTING "Enable testing" OFF)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

#
# Setup definitions
#
include(${CMAKE_MODULE_PATH}/SndFileChecks.cmake)

add_feature_info(BUILD_STATIC_LIBS BUILD_STATIC_LIBS "build static libraries")
add_feature_info(BUILD_SHARED_LIBS BUILD_SHARED_LIBS "build shared libraries")
add_feature_info(ENABLE_TESTING ENABLE_TESTING "build tests")
add_feature_info(DISABLE_EXTERNAL_LIBS DISABLE_EXTERNAL_LIBS "disable use of FLAC, Ogg and Vorbis")
add_feature_info(ENABLE_EXPERIMENTAL ENABLE_EXPERIMENTAL "enable experimental code")
add_feature_info(BUILD_TESTING BUILD_TESTING "build tests")

set_package_properties(Ogg PROPERTIES TYPE RECOMMENDED
	URL "www.xiph.org/ogg/"
	DESCRIPTION "library for manipulating ogg bitstreams"
	PURPOSE "Required to enable Vorbis, Speex and Opus support")
set_package_properties(Vorbis PROPERTIES TYPE RECOMMENDED
	URL "www.vorbis.com/"
	DESCRIPTION "open source lossy audio codec"	
	PURPOSE "Enables Vorbis support")
set_package_properties(FLAC PROPERTIES TYPE RECOMMENDED
	URL "www.xiph.org/flac/"
	DESCRIPTION "Free Lossless Audio Codec Library"
	PURPOSE "Enables FLAC support")
set_package_properties(Speex PROPERTIES TYPE OPTIONAL
	URL "www.speex.org/"
	DESCRIPTION "an audio codec tuned for speech"
	PURPOSE "Enables experemental Speex support")

feature_summary (WHAT ALL)

#
# Setup configuration
#

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.cmake ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/sndfile.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/sndfile.h)
if (WIN32)
if (BUILD_SHARED_LIBS)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/libsndfile.def.in.cmake ${CMAKE_CURRENT_SOURCE_DIR}/src/${PROJECT_NAME}-${PROJECT_VERSION_MAJOR}.def)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/version-metadata.rc.in.cmake ${CMAKE_CURRENT_SOURCE_DIR}/src/version-metadata.rc)
endif ()
endif ()

set (PC_PREFIX ${CMAKE_INSTALL_PREFIX})
set (PC_EXEC_PREFIX "\$\{prefix\}")
set (PC_LIBDIR "\$\{prefix\}/${CMAKE_INSTALL_LIBDIR}")
set (PC_INCLUDEDIR "\$\{prefix\}/${CMAKE_INSTALL_INCLUDEDIR}")
set (PC_VERSION ${PROJECT_VERSION})
if (EXTERNAL_XIPH_LIBS)
set (PC_PRIVATE_LIBS "-lFLAC  -lvorbisenc")
endif ()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sndfile.pc.cmake.in ${CMAKE_CURRENT_SOURCE_DIR}/sndfile.pc @ONLY)

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

# libsndfile static library

if (BUILD_STATIC_LIBS)
set (SNDFILE_STATIC_TARGET sndfile-static)
add_library(${SNDFILE_STATIC_TARGET} STATIC ${libsndfile_SOURCES})
if (M_LIBRARY)
target_link_libraries (${SNDFILE_STATIC_TARGET} PRIVATE ${M_LIBRARY})
endif ()
if (NOT DISABLE_EXTERNAL_LIBS)
	target_link_libraries (${SNDFILE_STATIC_TARGET} PUBLIC ${EXTERNAL_XIPH_LIBS})
	target_include_directories (${SNDFILE_STATIC_TARGET} PRIVATE
		${OGG_INCLUDE_DIRS}
		${VORBIS_INCLUDE_DIRS}
		${FLAC_INCLUDE_DIRS})
		target_compile_definitions (${SNDFILE_STATIC_TARGET} PRIVATE ${FLAC_DEFINITIONS})
	if (ENABLE_EXPERIMENTAL)
		target_link_libraries (${SNDFILE_STATIC_TARGET} PUBLIC ${SPEEX_LIBRARIES})
		target_include_directories (${SNDFILE_STATIC_TARGET} PRIVATE
		${SPEEX_INCLUDE_DIRS})
	endif ()
endif ()
if (WIN32)
	set_target_properties(${SNDFILE_STATIC_TARGET} PROPERTIES PREFIX "lib")
endif ()
# Remove "-static" from output name
set_target_properties(${SNDFILE_STATIC_TARGET} PROPERTIES OUTPUT_NAME "sndfile")
target_include_directories (${SNDFILE_STATIC_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
endif ()

if (BUILD_SHARED_LIBS)

set (SNDFILE_SHARED_TARGET sndfile)

if (WIN32)
list (APPEND libsndfile_SOURCES src/${PROJECT_NAME}-${PROJECT_VERSION_MAJOR}.def)
endif (WIN32)

add_library(${SNDFILE_SHARED_TARGET} SHARED ${libsndfile_SOURCES})
if (M_LIBRARY)
target_link_libraries (${SNDFILE_SHARED_TARGET} PRIVATE ${M_LIBRARY})
endif ()
if (NOT DISABLE_EXTERNAL_LIBS)
	target_link_libraries (${SNDFILE_SHARED_TARGET} PUBLIC ${EXTERNAL_XIPH_LIBS})
	target_include_directories (${SNDFILE_SHARED_TARGET} PRIVATE
		${OGG_INCLUDE_DIRS}
		${VORBIS_INCLUDE_DIRS}
		${FLAC_INCLUDE_DIRS})
		target_compile_definitions (${SNDFILE_SHARED_TARGET} PRIVATE ${FLAC_DEFINITIONS})		
	if (ENABLE_EXPERIMENTAL)
		target_link_libraries (${SNDFILE_SHARED_TARGET} PUBLIC ${SPEEX_LIBRARIES})
		target_include_directories (${SNDFILE_SHARED_TARGET} PRIVATE
		${SPEEX_INCLUDE_DIRS})
	endif ()
endif ()
set_target_properties (${SNDFILE_SHARED_TARGET}
			PROPERTIES
			SOVERSION ${PROJECT_VERSION_MAJOR}
			VERSION ${PROJECT_VERSION}
			PUBLIC_HEADER "${libsndfile_PUBLIC_HEADERS}")
if (WIN32)
	set_target_properties(${SNDFILE_SHARED_TARGET} PROPERTIES PREFIX "lib")
	set_target_properties(${SNDFILE_SHARED_TARGET} PROPERTIES OUTPUT_NAME "sndfile-${PROJECT_VERSION_MAJOR}")
endif (WIN32)
target_include_directories (${SNDFILE_SHARED_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

endif (BUILD_SHARED_LIBS)

## Programs

# Select libsndfile version to link
if (BUILD_SHARED_LIBS)
set (SNDFILE_TARGET ${SNDFILE_SHARED_TARGET})
else (NOT BUILD_SHARED_LIBS)
set (SNDFILE_TARGET ${SNDFILE_STATIC_TARGET})
endif (BUILD_SHARED_LIBS)

### sndfile-info

set (sndfile_info_SOURCES 
	programs/sndfile-info.c
	programs/common.c
	programs/common.h)
add_executable (sndfile-info ${sndfile_info_SOURCES})
target_link_libraries(sndfile-info PUBLIC ${SNDFILE_TARGET} m)

### sndfile-play

set (sndfile_play_SOURCES 
	programs/sndfile-play.c
	programs/common.c
	programs/common.h)
add_executable (sndfile-play ${sndfile_play_SOURCES})
target_link_libraries(sndfile-play PUBLIC
	${SNDFILE_TARGET}
	${ALSA_LIBRARIES}
	${Sndio_LIBRARIES})
target_include_directories (sndfile-play PRIVATE
	${ALSA_INCLUDE_DIRS}
	${Sndio_INCLUDE_DIRS})
if (WIN32)
	target_link_libraries(sndfile-play PRIVATE Winmm.lib)
endif ()

### sndfile-convert

set (sndfile_convert_SOURCES 
	programs/sndfile-convert.c
	programs/common.c
	programs/common.h)
add_executable (sndfile-convert ${sndfile_convert_SOURCES})
target_link_libraries(sndfile-convert PUBLIC ${SNDFILE_TARGET})

### sndfile-cmp

set (sndfile_cmp_SOURCES 
	programs/sndfile-cmp.c
	programs/common.c
	programs/common.h)
add_executable (sndfile-cmp ${sndfile_cmp_SOURCES})
target_link_libraries(sndfile-cmp PUBLIC ${SNDFILE_TARGET})

### sndfile-metadata-set

set (sndfile_metadata_set_SOURCES
	programs/sndfile-metadata-set.c
	programs/common.c
	programs/common.h)
add_executable (sndfile-metadata-set ${sndfile_metadata_set_SOURCES})
target_link_libraries(sndfile-metadata-set PUBLIC ${SNDFILE_TARGET})

### sndfile-metadata-get

set (sndfile_metadata_get_SOURCES
	programs/sndfile-metadata-get.c
	programs/common.c
	programs/common.h)
add_executable (sndfile-metadata-get ${sndfile_metadata_get_SOURCES})
target_link_libraries(sndfile-metadata-get PUBLIC ${SNDFILE_TARGET})

### sndfile-interleave

set (sndfile_interleave_SOURCES
	programs/sndfile-interleave.c
	programs/common.c
	programs/common.h)
add_executable (sndfile-interleave ${sndfile_interleave_SOURCES})
target_link_libraries(sndfile-interleave PUBLIC ${SNDFILE_TARGET})

### sndfile-deinterleave

set (sndfile_deinterleave_SOURCES
	programs/sndfile-deinterleave.c
	programs/common.c
	programs/common.h)
add_executable (sndfile-deinterleave ${sndfile_deinterleave_SOURCES})
target_link_libraries(sndfile-deinterleave PUBLIC ${SNDFILE_TARGET})

### sndfile-concat

set (sndfile_concat_SOURCES
	programs/sndfile-concat.c
	programs/common.c
	programs/common.h)
add_executable (sndfile-concat ${sndfile_concat_SOURCES})
target_link_libraries(sndfile-concat PUBLIC ${SNDFILE_TARGET})

### sndfile-salvage

set (sndfile_salvage_SOURCES
	programs/sndfile-salvage.c
	programs/common.c
	programs/common.h)
add_executable (sndfile-salvage ${sndfile_salvage_SOURCES})
target_link_libraries(sndfile-salvage PUBLIC ${SNDFILE_TARGET})

### sndfile-play-beos

# This is the BeOS version of sndfile-play. It needs to be compiled with the C++
# compiler.
if (BEOS OR HAIKU)
	set (sndfile_play_beos_SOURCES
		programs/sndfile-play-beos.cpp)
	add_executable (sndfile-play-beos ${sndfile_play_beos_SOURCES})
	target_link_libraries(sndfile-play-beos PUBLIC ${SNDFILE_TARGET})
endif (BEOS OR HAIKU)

## Installation

install (TARGETS ${SNDFILE_STATIC_TARGET} ${SNDFILE_SHARED_TARGET} ${PROGRAMS} 
	RUNTIME DESTINATION			${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION			${CMAKE_INSTALL_LIBDIR}
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

install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/sndfile.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
	
## Testing

if (BUILD_TESTING)

endif ()
