cmake_minimum_required (VERSION 3.1.3)

project(libsndfile VERSION 1.0.29)

#
# Variables
#

set (CPACK_PACKAGE_VERSION_MAJOR ${${PROJECT_VERSION}_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${${PROJECT_VERSION}_MINOR})
set (CPACK_PACKAGE_VERSION_PATCH ${${PROJECT_VERSION}_PATCH})
set (CPACK_PACKAGE_VERSION "${PROJECT_VERSION}pre1")
set (CMAKE_C_STANDARD 99)
set (CMAKE_C_STANDARD_REQUIRED TRUE)

#
# System-wide includes
#

include (GNUInstallDirs)
include (CPack)
include (CTest)
include (FeatureSummary)

#
# Options
#

option (BUILD_STATIC_LIBS "Build static libraries" ON)
option (BUILD_SHARED_LIBS "Build shared libraries" ON)
option (BUILD_PROGRAMS "Build programs" ON)
option (DISABLE_EXTERNAL_LIBS "Disable use of FLAC, Ogg and Vorbis" OFF)
option (ENABLE_EXPERIMENTAL "Enable experimental code" OFF)
option (DISABLE_CPU_CLIP "Disable tricky cpu specific clipper" OFF)

if ((NOT BUILD_STATIC_LIBS) AND (NOT BUILD_SHARED_LIBS))
	message ("
		Both options BUILD_STATIC_LIBS and BUILD_SHARED_LIBS are manually set to OFF. To prevent configuration error
		BUILD_STATIC_LIBS is now set to ON.\n")
	set (BUILD_STATIC_LIBS ON)
endif ()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

#
# Setup definitions
#

include(${CMAKE_MODULE_PATH}/SndFileChecks.cmake)

add_feature_info(BUILD_STATIC_LIBS BUILD_STATIC_LIBS "build static libraries")
add_feature_info(BUILD_SHARED_LIBS BUILD_SHARED_LIBS "build shared libraries")
add_feature_info(DISABLE_EXTERNAL_LIBS DISABLE_EXTERNAL_LIBS "disable use of FLAC, Ogg and Vorbis")
add_feature_info(ENABLE_EXPERIMENTAL ENABLE_EXPERIMENTAL "enable experimental code")
add_feature_info(BUILD_TESTING BUILD_TESTING "build tests")
add_feature_info(DISABLE_CPU_CLIP DISABLE_CPU_CLIP "Disable tricky cpu specific clipper")

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

configure_file (src/config.h.cmake src/config.h)
configure_file (src/sndfile.h.in src/sndfile.h)

set (PC_PREFIX ${CMAKE_INSTALL_PREFIX})
set (PC_EXEC_PREFIX "\$\{prefix\}")
set (PC_LIBDIR "\$\{prefix\}/${CMAKE_INSTALL_LIBDIR}")
set (PC_INCLUDEDIR "\$\{prefix\}/${CMAKE_INSTALL_INCLUDEDIR}")
set (PC_VERSION ${PROJECT_VERSION})
if (EXTERNAL_XIPH_LIBS)
	set (PC_PRIVATE_LIBS "-lFLAC  -lvorbisenc")
endif ()
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/sndfile.pc.cmake.in ${CMAKE_CURRENT_SOURCE_DIR}/sndfile.pc @ONLY)

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

# Headers to be installed on target machine
set (libsndfile_PUBLIC_HEADERS ${include_HEADERS} ${nodist_include_HEADERS})
set (libsndfile_SOURCES ${COMMON} ${FILESPECIFIC} ${noinst_HEADERS} 
	${libgsm_SOURCES} ${libg72x_SOURCES} ${libalac_SOURCES}
	${libsndfile_PUBLIC_HEADERS})

#
# libsndfile static library
#

if (BUILD_STATIC_LIBS)
	set (SNDFILE_STATIC_TARGET sndfile-static)
	add_library (${SNDFILE_STATIC_TARGET} STATIC ${libsndfile_SOURCES})
	target_include_directories(${SNDFILE_STATIC_TARGET} PUBLIC ${CMAKE_BINARY_DIR}/src)
	if (LIBM_REQUIRED)
		target_link_libraries (${SNDFILE_STATIC_TARGET} PUBLIC ${M_LIBRARY})
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
		set_target_properties (${SNDFILE_STATIC_TARGET} PROPERTIES PREFIX "lib")
	endif ()
	# Remove "-static" from output name
	set_target_properties (${SNDFILE_STATIC_TARGET} PROPERTIES OUTPUT_NAME "sndfile")
	target_include_directories (${SNDFILE_STATIC_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
endif ()

if (BUILD_SHARED_LIBS)

	set (SNDFILE_SHARED_TARGET sndfile)

	if (WIN32)
		configure_file(src/version-metadata.rc.in.cmake src/version-metadata.rc)
		configure_file (src/libsndfile.def.in.cmake src/${PROJECT_NAME}-${PROJECT_VERSION_MAJOR}.def)
		list (APPEND libsndfile_SOURCES
			src/version-metadata.rc
			src/${PROJECT_NAME}-${PROJECT_VERSION_MAJOR}.def)
	endif (WIN32)

	add_library(${SNDFILE_SHARED_TARGET} SHARED ${libsndfile_SOURCES})
	target_include_directories(${SNDFILE_SHARED_TARGET} PUBLIC ${CMAKE_BINARY_DIR}/src)

	if (LIBM_REQUIRED)
		target_link_libraries (${SNDFILE_SHARED_TARGET} PRIVATE ${M_LIBRARY})
	endif (LIBM_REQUIRED)

	if (NOT DISABLE_EXTERNAL_LIBS)
		target_link_libraries (${SNDFILE_SHARED_TARGET} PRIVATE ${EXTERNAL_XIPH_LIBS})
		target_include_directories (${SNDFILE_SHARED_TARGET} PRIVATE
			${OGG_INCLUDE_DIRS}
			${VORBIS_INCLUDE_DIRS}
			${FLAC_INCLUDE_DIRS})
			target_compile_definitions (${SNDFILE_SHARED_TARGET} PRIVATE ${FLAC_DEFINITIONS})
		if (ENABLE_EXPERIMENTAL)
			target_link_libraries (${SNDFILE_SHARED_TARGET} PRIVATE ${SPEEX_LIBRARIES})
			target_include_directories (${SNDFILE_SHARED_TARGET} PRIVATE
			${SPEEX_INCLUDE_DIRS})
		endif (ENABLE_EXPERIMENTAL)
	endif (NOT DISABLE_EXTERNAL_LIBS)

	target_include_directories (${SNDFILE_SHARED_TARGET}
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

	set_target_properties (${SNDFILE_SHARED_TARGET} PROPERTIES
		DEFINE_SYMBOL SNDFILE_EXPORTS
		PUBLIC_HEADER "${libsndfile_PUBLIC_HEADERS}"
		SOVERSION ${PROJECT_VERSION_MAJOR}
		VERSION ${PROJECT_VERSION}
		C_VISIBILITY_PRESET hidden)

	if (WIN32)
		set_target_properties(${SNDFILE_SHARED_TARGET} PROPERTIES
			PREFIX "lib"
			OUTPUT_NAME "sndfile-${PROJECT_VERSION_MAJOR}")
	endif (WIN32)

endif (BUILD_SHARED_LIBS)

#
# Programs
#

# Select libsndfile version to link
if (BUILD_SHARED_LIBS)
	set (SNDFILE_TARGET ${SNDFILE_SHARED_TARGET})
else (NOT BUILD_SHARED_LIBS)
	set (SNDFILE_TARGET ${SNDFILE_STATIC_TARGET})
endif (BUILD_SHARED_LIBS)

if (BUILD_PROGRAMS)

# sndfile-info

	set (sndfile_info_SOURCES
		programs/sndfile-info.c
		programs/common.c
		programs/common.h)
	add_executable (sndfile-info ${sndfile_info_SOURCES})
	target_link_libraries(sndfile-info PUBLIC ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(sndfile-info PRIVATE ${M_LIBRARY})
	endif ()

# sndfile-play

	if (NOT BEOS)
		set (sndfile_play_SOURCES
			programs/sndfile-play.c
			programs/common.c
			programs/common.h)
	else ()
		set (sndfile_play_SOURCES
			programs/sndfile-play-beos.cpp)
	endif ()

	add_executable (sndfile-play ${sndfile_play_SOURCES})
	target_link_libraries(sndfile-play PUBLIC ${SNDFILE_TARGET})
	if (WIN32)
		target_link_libraries(sndfile-play PRIVATE Winmm.lib)
	# Maybe ALSA & Sndio are present in BeOS. They are not required
	# so skip them anyway.
	elseif ((NOT BEOS) AND ALSA_FOUND)
		target_include_directories (sndfile-play PRIVATE ${ALSA_INCLUDE_DIRS})
		target_link_libraries(sndfile-play PRIVATE ${ALSA_LIBRARIES})
	elseif ((NOT BEOS) AND Sndio_FOUND)
		target_include_directories (sndfile-play PRIVATE ${Sndio_INCLUDE_DIRS})
		target_link_libraries(sndfile-play PRIVATE ${Sndio_LIBRARIES})
	endif ()

# sndfile-convert

	set (sndfile_convert_SOURCES
		programs/sndfile-convert.c
		programs/common.c
		programs/common.h)
	add_executable (sndfile-convert ${sndfile_convert_SOURCES})
	target_link_libraries(sndfile-convert PUBLIC ${SNDFILE_TARGET})

# sndfile-cmp

	set (sndfile_cmp_SOURCES
		programs/sndfile-cmp.c
		programs/common.c
		programs/common.h)
	add_executable (sndfile-cmp ${sndfile_cmp_SOURCES})
	target_link_libraries(sndfile-cmp PUBLIC ${SNDFILE_TARGET})

# sndfile-metadata-set

	set (sndfile_metadata_set_SOURCES
		programs/sndfile-metadata-set.c
		programs/common.c
		programs/common.h)
	add_executable (sndfile-metadata-set ${sndfile_metadata_set_SOURCES})
	target_link_libraries(sndfile-metadata-set PUBLIC ${SNDFILE_TARGET})

# sndfile-metadata-get

	set (sndfile_metadata_get_SOURCES
		programs/sndfile-metadata-get.c
		programs/common.c
		programs/common.h)
	add_executable (sndfile-metadata-get ${sndfile_metadata_get_SOURCES})
	target_link_libraries(sndfile-metadata-get PUBLIC ${SNDFILE_TARGET})

# sndfile-interleave

	set (sndfile_interleave_SOURCES
		programs/sndfile-interleave.c
		programs/common.c
		programs/common.h)
	add_executable (sndfile-interleave ${sndfile_interleave_SOURCES})
	target_link_libraries(sndfile-interleave PUBLIC ${SNDFILE_TARGET})

# sndfile-deinterleave

	set (sndfile_deinterleave_SOURCES
		programs/sndfile-deinterleave.c
		programs/common.c
		programs/common.h)
	add_executable (sndfile-deinterleave ${sndfile_deinterleave_SOURCES})
	target_link_libraries(sndfile-deinterleave PUBLIC ${SNDFILE_TARGET})

# sndfile-concat

	set (sndfile_concat_SOURCES
		programs/sndfile-concat.c
		programs/common.c
		programs/common.h)
	add_executable (sndfile-concat ${sndfile_concat_SOURCES})
	target_link_libraries(sndfile-concat PUBLIC ${SNDFILE_TARGET})

# sndfile-salvage

	set (sndfile_salvage_SOURCES
		programs/sndfile-salvage.c
		programs/common.c
		programs/common.h)
	add_executable (sndfile-salvage ${sndfile_salvage_SOURCES})
	target_link_libraries(sndfile-salvage PUBLIC ${SNDFILE_TARGET})

	set (sdnfile_PROGRAMS
		sndfile-info
		sndfile-play
		sndfile-convert
		sndfile-cmp
		sndfile-metadata-set
		sndfile-metadata-get
		sndfile-interleave
		sndfile-deinterleave
		sndfile-concat
		sndfile-salvage)

endif (BUILD_PROGRAMS)

#
# Installation
#

install (TARGETS ${SNDFILE_STATIC_TARGET} ${SNDFILE_SHARED_TARGET} ${sdnfile_PROGRAMS}
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
		#man/sndfile-metadata-set.1
		man/sndfile-concat.1
		man/sndfile-interleave.1
		#man/sndfile-deinterleave.1
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
	doc/print.css
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
	doc/tutorial.html)
install (FILES ${html_DATA} DESTINATION ${CMAKE_INSTALL_DOCDIR})

install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/sndfile.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

#
# Testing
#

if (BUILD_TESTING)

	# Create src and tests directory in CMAKE_BINARY_DIR to put generated sources in
	file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/src")
	file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/tests")

	# Custom commands to generate tests sources from autogen templates

	add_custom_command (
		OUTPUT ${CMAKE_BINARY_DIR}/tests/benchmark.c
		COMMAND ${AUTOGEN_EXECUTABLE} --writable -L ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/tests/benchmark.def
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

	add_custom_command (
		OUTPUT ${CMAKE_BINARY_DIR}/tests/floating_point_test.c
		COMMAND ${AUTOGEN_EXECUTABLE} --writable -L ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/tests/floating_point_test.def
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

	add_custom_command (
		OUTPUT ${CMAKE_BINARY_DIR}/tests/header_test.c
		COMMAND ${AUTOGEN_EXECUTABLE} --writable -L ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/tests/header_test.def
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

	add_custom_command (
		OUTPUT ${CMAKE_BINARY_DIR}/tests/pcm_test.c
		COMMAND ${AUTOGEN_EXECUTABLE} --writable -L ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/tests/pcm_test.def
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

	add_custom_command (
		OUTPUT ${CMAKE_BINARY_DIR}/tests/pipe_test.c
		COMMAND ${AUTOGEN_EXECUTABLE} --writable -L ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/tests/pipe_test.def
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

	add_custom_command (
		OUTPUT ${CMAKE_BINARY_DIR}/tests/rdwr_test.c
		COMMAND ${AUTOGEN_EXECUTABLE} --writable -L ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/tests/rdwr_test.def
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

	add_custom_command (
		OUTPUT ${CMAKE_BINARY_DIR}/tests/scale_clip_test.c
		COMMAND ${AUTOGEN_EXECUTABLE} --writable -L ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/tests/scale_clip_test.def
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

	add_custom_command (
		OUTPUT ${CMAKE_BINARY_DIR}/tests/utils.c ${CMAKE_BINARY_DIR}/tests/utils.h
		COMMAND ${AUTOGEN_EXECUTABLE} --writable -L ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/tests/utils.def
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

	add_custom_command (
		OUTPUT ${CMAKE_BINARY_DIR}/tests/write_read_test.c
		COMMAND ${AUTOGEN_EXECUTABLE} --writable -L ${CMAKE_SOURCE_DIR}/tests ${CMAKE_SOURCE_DIR}/tests/write_read_test.def
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests)

	add_custom_command (
		OUTPUT ${CMAKE_BINARY_DIR}/src/test_endswap.c
		COMMAND ${AUTOGEN_EXECUTABLE} --writable -L ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/test_endswap.def
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/src)

	### test_main

	set (test_main_SOURCES
		src/test_main.c
		src/test_main.h
		src/test_conversions.c
		src/test_float.c
		src/test_endswap.c
		src/test_audio_detect.c
		src/test_log_printf.c
		src/test_file_io.c
		src/test_ima_oki_adpcm.c
		src/test_strncpy_crlf.c
		src/test_broadcast_var.c
		src/test_cart_var.c
		src/test_binheader_writef.c)

	add_executable (test_main ${test_main_SOURCES})
	target_link_libraries (test_main ${SNDFILE_STATIC_TARGET})
	add_test (test_main test_main)

	### sfversion_test

	set (sfversion_SOURCES tests/sfversion.c)
	add_executable (sfversion ${sfversion_SOURCES})
	target_link_libraries (sfversion ${SNDFILE_TARGET})
	add_test (sfversion sfversion)
	set_tests_properties (sfversion PROPERTIES
		PASS_REGULAR_EXPRESSION "libsndfile-${CPACK_PACKAGE_VERSION}")

	### error_test

	set (error_test_SOURCES tests/error_test.c tests/utils.c)
	add_executable (error_test ${error_test_SOURCES})
	target_link_libraries (error_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(error_test PRIVATE ${M_LIBRARY})
	endif ()
	add_test (error_test error_test)

	### ulaw_test
	set (ulaw_test_SOURCES tests/utils.c tests/ulaw_test.c)
	add_executable (ulaw_test ${ulaw_test_SOURCES})
	target_link_libraries (ulaw_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(ulaw_test PRIVATE ${M_LIBRARY})
	endif ()
	add_test (ulaw_test ulaw_test)

	### alaw_test
	set (alaw_test_SOURCES tests/utils.c tests/alaw_test.c)
	add_executable (alaw_test ${alaw_test_SOURCES})
	target_link_libraries (alaw_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(alaw_test PRIVATE ${M_LIBRARY})
	endif ()
	add_test (alaw_test alaw_test)

	### dwvw_test

	set (dwvw_test_SOURCES tests/utils.c tests/dwvw_test.c)
	add_executable (dwvw_test ${dwvw_test_SOURCES})
	target_link_libraries (dwvw_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(dwvw_test PRIVATE ${M_LIBRARY})
	endif ()
	add_test (dwvw_test dwvw_test)

	### command_test

	set (command_test_SOURCES tests/command_test.c tests/utils.c)
	add_executable (command_test ${command_test_SOURCES})
	target_link_libraries (command_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(command_test PRIVATE ${M_LIBRARY})
	endif ()
	add_test (command_test command_test all)

	### floating_point_test

	set (floating_point_test_SOURCES tests/utils.c tests/dft_cmp.c tests/floating_point_test.c)
	add_executable (floating_point_test ${floating_point_test_SOURCES})
	target_link_libraries (floating_point_test PRIVATE ${SNDFILE_TARGET})
	target_include_directories(floating_point_test PRIVATE tests)
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(floating_point_test PRIVATE ${M_LIBRARY})
	endif ()
	add_test (floating_point_test floating_point_test)

	### checksum_test

	set (checksum_test_SOURCES tests/checksum_test.c tests/utils.c)
	add_executable (checksum_test ${checksum_test_SOURCES})
	target_link_libraries (checksum_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(checksum_test PRIVATE ${M_LIBRARY})
	endif ()
	add_test (checksum_test checksum_test)

	### scale_clip_test

	set (scale_clip_test_SOURCES tests/scale_clip_test.c tests/utils.c)
	add_executable (scale_clip_test ${scale_clip_test_SOURCES})
	target_link_libraries (scale_clip_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(scale_clip_test PRIVATE ${M_LIBRARY})
	endif ()
	add_test (scale_clip_test scale_clip_test)

	### headerless_test

	set (headerless_test_SOURCES tests/utils.c tests/headerless_test.c)
	add_executable (headerless_test ${headerless_test_SOURCES})
	target_link_libraries (headerless_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(headerless_test PRIVATE ${M_LIBRARY})
	endif ()
	add_test (headerless_test headerless_test)

	### rdwr_test

	set (rdwr_test_SOURCES tests/rdwr_test.c tests/utils.c)
	add_executable (rdwr_test ${rdwr_test_SOURCES})
	target_link_libraries (rdwr_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(rdwr_test PRIVATE ${M_LIBRARY})
	endif ()
	add_test (rdwr_test rdwr_test)

	### locale_test

	set (locale_test_SOURCES tests/locale_test.c tests/utils.c)
	add_executable (locale_test ${locale_test_SOURCES})
	target_link_libraries (locale_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(locale_test PRIVATE ${M_LIBRARY})
	endif ()
	add_test (locale_test locale_test)

	### win32_ordinal_test

	# TODO

	### cpp_test

	set (cpp_test_SOURCES tests/cpp_test.cc tests/utils.c)
	add_executable (cpp_test ${cpp_test_SOURCES})
	target_link_libraries (cpp_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(cpp_test PRIVATE ${M_LIBRARY})
	endif ()
	add_test (cpp_test cpp_test)

	### external_libs_test

	set (external_libs_test_SOURCES tests/external_libs_test.c tests/utils.c)
	add_executable (external_libs_test ${external_libs_test_SOURCES})
	target_link_libraries (external_libs_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(external_libs_test PRIVATE ${M_LIBRARY})
	endif ()
	add_test (external_libs_test external_libs_test)

	### format_check_test

	set (format_check_test_SOURCES tests/format_check_test.c tests/utils.c)
	add_executable (format_check_test ${format_check_test_SOURCES})
	target_link_libraries (format_check_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(format_check_test PRIVATE ${M_LIBRARY})
	endif ()
	add_test (format_check_test format_check_test)

	### channel_test

	set (channel_test_SOURCES tests/channel_test.c tests/utils.c)
	add_executable (channel_test ${channel_test_SOURCES})
	target_link_libraries (channel_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(channel_test PRIVATE ${M_LIBRARY})
	endif ()
	add_test (channel_test channel_test)

	### common test executables

	set (write_read_test_SOURCES tests/utils.c tests/generate.c tests/write_read_test.c)
	add_executable (write_read_test ${write_read_test_SOURCES})
	target_link_libraries (write_read_test PRIVATE ${SNDFILE_TARGET})
	target_include_directories(write_read_test PRIVATE tests)
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(write_read_test PRIVATE ${M_LIBRARY})
	endif ()

	set (lossy_comp_test_SOURCES tests/utils.c tests/lossy_comp_test.c)
	add_executable (lossy_comp_test ${lossy_comp_test_SOURCES})
	target_link_libraries (lossy_comp_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(lossy_comp_test PRIVATE ${M_LIBRARY})
	endif ()

	set (peak_chunk_test_SOURCES tests/peak_chunk_test.c tests/utils.c)
	add_executable (peak_chunk_test ${peak_chunk_test_SOURCES})
	target_link_libraries (peak_chunk_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(peak_chunk_test PRIVATE ${M_LIBRARY})
	endif ()

	set (header_test_SOURCES tests/header_test.c tests/utils.c)
	add_executable (header_test ${header_test_SOURCES})
	target_link_libraries (header_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(header_test PRIVATE ${M_LIBRARY})
	endif ()

	set (misc_test_SOURCES tests/misc_test.c tests/utils.c)
	add_executable (misc_test ${misc_test_SOURCES})
	target_link_libraries (misc_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(misc_test PRIVATE ${M_LIBRARY})
	endif ()

	set (string_test_SOURCES tests/string_test.c tests/utils.c)
	add_executable (string_test ${string_test_SOURCES})
	target_link_libraries (string_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(string_test PRIVATE ${M_LIBRARY})
	endif ()

	set (multi_file_test_SOURCES tests/multi_file_test.c tests/utils.c)
	add_executable (multi_file_test ${multi_file_test_SOURCES})
	target_link_libraries (multi_file_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(multi_file_test PRIVATE ${M_LIBRARY})
	endif ()

	set (aiff_rw_test_SOURCES tests/utils.c tests/aiff_rw_test.c)
	add_executable (aiff_rw_test ${aiff_rw_test_SOURCES})
	target_link_libraries (aiff_rw_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(aiff_rw_test PRIVATE ${M_LIBRARY})
	endif ()

	set (chunk_test_SOURCES tests/chunk_test.c tests/utils.c)
	add_executable (chunk_test ${chunk_test_SOURCES})
	target_link_libraries (chunk_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(chunk_test PRIVATE ${M_LIBRARY})
	endif ()

	set (long_read_write_test_SOURCES tests/long_read_write_test.c tests/utils.c)
	add_executable (long_read_write_test ${long_read_write_test_SOURCES})
	target_link_libraries (long_read_write_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(long_read_write_test PRIVATE ${M_LIBRARY})
	endif ()

	set (raw_test_SOURCES tests/raw_test.c tests/utils.c)
	add_executable (raw_test ${raw_test_SOURCES})
	target_link_libraries (raw_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(raw_test PRIVATE ${M_LIBRARY})
	endif ()

	set (compression_size_test_SOURCES tests/compression_size_test.c tests/utils.c)
	add_executable (compression_size_test ${compression_size_test_SOURCES})
	target_link_libraries (compression_size_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(compression_size_test PRIVATE ${M_LIBRARY})
	endif ()

	set (ogg_test_SOURCES tests/ogg_test.c tests/utils.c)
	add_executable (ogg_test ${ogg_test_SOURCES})
	target_link_libraries (ogg_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(ogg_test PRIVATE ${M_LIBRARY})
	endif ()

	set (stdin_test_SOURCES tests/stdin_test.c tests/utils.c)
	add_executable (stdin_test ${stdin_test_SOURCES})
	target_link_libraries (stdin_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(stdin_test PRIVATE ${M_LIBRARY})
	endif ()

	set (stdout_test_SOURCES tests/stdout_test.c)
	add_executable (stdout_test ${stdout_test_SOURCES})
	target_link_libraries (stdout_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(stdout_test PRIVATE ${M_LIBRARY})
	endif ()

	set (stdio_test_SOURCES tests/stdio_test.c tests/utils.c)
	add_executable (stdio_test ${stdio_test_SOURCES})
	target_link_libraries (stdio_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(stdio_test PRIVATE ${M_LIBRARY})
	endif ()

	set (pipe_test_SOURCES tests/pipe_test.c tests/utils.c tests/utils.h)
	add_executable (pipe_test ${pipe_test_SOURCES})
	target_link_libraries (pipe_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(pipe_test PRIVATE ${M_LIBRARY})
	endif ()

	set (virtual_io_test_SOURCES tests/virtual_io_test.c tests/utils.c)
	add_executable (virtual_io_test ${virtual_io_test_SOURCES})
	target_link_libraries (virtual_io_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(virtual_io_test PRIVATE ${M_LIBRARY})
	endif ()

	### g72x_test

	set (g72x_test_SOURCES src/G72x/g72x_test.c ${libg72x_SOURCES})
	add_executable (g72x_test ${g72x_test_SOURCES})
	target_link_libraries (g72x_test PRIVATE ${SNDFILE_TARGET})
	if (BUILD_SHARED_LIBS AND LIBM_REQUIRED)
		target_link_libraries(g72x_test PRIVATE ${M_LIBRARY})
	endif ()
	add_test (g72x_test g72x_test all)

	### aiff-tests

	add_test (write_read_test_aiff write_read_test aiff)
	add_test (lossy_comp_test_aiff_ulaw lossy_comp_test aiff_ulaw)
	add_test (lossy_comp_test_aiff_alaw lossy_comp_test aiff_alaw)
	add_test (lossy_comp_test_aiff_gsm610 lossy_comp_test aiff_gsm610)
	add_test (peak_chunk_test_aiff peak_chunk_test aiff)
	add_test (header_test_aiff header_test aiff)
	add_test (misc_test_aiff misc_test aiff)
	add_test (string_test_aiff string_test aiff)
	add_test (multi_file_test_aiff multi_file_test aiff)
	add_test (aiff_rw_test aiff_rw_test)

	### au-tests

	add_test (write_read_test_au write_read_test au)
	add_test (lossy_comp_test_au_ulaw lossy_comp_test au_ulaw)
	add_test (lossy_comp_test_au_alaw lossy_comp_test au_alaw)
	add_test (lossy_comp_test_au_g721 lossy_comp_test au_g721)
	add_test (lossy_comp_test_au_g723 lossy_comp_test au_g723)
	add_test (header_test_au header_test au)
	add_test (misc_test_au misc_test au)
	add_test (multi_file_test_au multi_file_test au)

	### caf-tests

	add_test (write_read_test_caf write_read_test caf)
	add_test (lossy_comp_test_caf_ulaw lossy_comp_test caf_ulaw)
	add_test (lossy_comp_test_caf_alaw lossy_comp_test caf_alaw)
	add_test (header_test_caf header_test caf)
	add_test (peak_chunk_test_caf peak_chunk_test caf)
	add_test (misc_test_caf misc_test caf)
	add_test (chunk_test_caf chunk_test caf)
	add_test (string_test_caf string_test caf)
	add_test (long_read_write_test_alac long_read_write_test alac)

	# wav-tests
	add_test (write_read_test_wav write_read_test wav)
	add_test (lossy_comp_test_wav_pcm lossy_comp_test wav_pcm)
	add_test (lossy_comp_test_wav_ima lossy_comp_test wav_ima)
	add_test (lossy_comp_test_wav_msadpcm lossy_comp_test wav_msadpcm)
	add_test (lossy_comp_test_wav_ulaw lossy_comp_test wav_ulaw)
	add_test (lossy_comp_test_wav_alaw lossy_comp_test wav_alaw)
	add_test (lossy_comp_test_wav_gsm610 lossy_comp_test wav_gsm610)
	add_test (lossy_comp_test_wav_g721 lossy_comp_test wav_g721)
	add_test (peak_chunk_test_wav peak_chunk_test wav)
	add_test (header_test_wav header_test wav)
	add_test (misc_test_wav misc_test wav)
	add_test (string_test_wav string_test wav)
	add_test (multi_file_test_wav multi_file_test wav)
	add_test (chunk_test_wav chunk_test wav)

	### w64-tests

	add_test (write_read_test_w64 write_read_test w64)
	add_test (lossy_comp_test_w64_ima lossy_comp_test w64_ima)
	add_test (lossy_comp_test_w64_msadpcm lossy_comp_test w64_msadpcm)
	add_test (lossy_comp_test_w64_ulaw lossy_comp_test w64_ulaw)
	add_test (lossy_comp_test_w64_alaw lossy_comp_test w64_alaw)
	add_test (lossy_comp_test_w64_gsm610 lossy_comp_test w64_gsm610)
	add_test (header_test_w64 header_test w64)
	add_test (misc_test_w64 misc_test w64)

	### rf64-tests

	add_test (write_read_test_rf64 write_read_test rf64)
	add_test (header_test_rf64 header_test rf64)
	add_test (misc_test_rf64 misc_test rf64)
	add_test (string_test_rf64 string_test rf64)
	add_test (peak_chunk_test_rf64 peak_chunk_test rf64)
	add_test (chunk_test_rf64 chunk_test rf64)

	### raw-tests
	add_test (write_read_test_raw write_read_test raw)
	add_test (lossy_comp_test_raw_ulaw lossy_comp_test raw_ulaw)
	add_test (lossy_comp_test_raw_alaw lossy_comp_test raw_alaw)
	add_test (lossy_comp_test_raw_gsm610 lossy_comp_test raw_gsm610)
	add_test (lossy_comp_test_vox_adpcm lossy_comp_test vox_adpcm)
	add_test (raw_test raw_test)

	### paf-tests
	add_test (write_read_test_paf write_read_test paf)
	add_test (header_test_paf header_test paf)
	add_test (misc_test_paf misc_test paf)

	### svx-tests
	add_test (write_read_test_svx write_read_test svx)
	add_test (header_test_svx header_test svx)
	add_test (misc_test_svx misc_test svx)

	### nist-tests
	add_test (write_read_test_nist write_read_test nist)
	add_test (lossy_comp_test_nist_ulaw lossy_comp_test nist_ulaw)
	add_test (lossy_comp_test_nist_alaw lossy_comp_test nist_alaw)
	add_test (header_test_nist header_test nist)
	add_test (misc_test_nist misc_test nist)

	### ircam-tests
	add_test (write_read_test_ircam write_read_test ircam)
	add_test (lossy_comp_test_ircam_ulaw lossy_comp_test ircam_ulaw)
	add_test (lossy_comp_test_ircam_alaw lossy_comp_test ircam_alaw)
	add_test (header_test_ircam header_test ircam)
	add_test (misc_test_ircam misc_test ircam)

	### voc-tests
	add_test (write_read_test_voc write_read_test voc)
	add_test (lossy_comp_test_voc_ulaw lossy_comp_test voc_ulaw)
	add_test (lossy_comp_test_voc_alaw lossy_comp_test voc_alaw)
	add_test (header_test_voc header_test voc)
	add_test (misc_test_voc misc_test voc)

	### mat4-tests
	add_test (write_read_test_mat4 write_read_test mat4)
	add_test (header_test_mat4 header_test mat4)
	add_test (misc_test_mat4 misc_test mat4)

	### mat5-tests
	add_test (write_read_test_mat5 write_read_test mat5)
	add_test (header_test_mat5 header_test mat5)
	add_test (misc_test_mat5 misc_test mat5)

	### pvf-tests
	add_test (write_read_test_pvf write_read_test pvf)
	add_test (header_test_pvf header_test pvf)
	add_test (misc_test_pvf misc_test pvf)

	### xi-tests
	add_test (lossy_comp_test_xi_dpcm lossy_comp_test xi_dpcm)

	### htk-tests
	add_test (write_read_test_htk write_read_test htk)
	add_test (header_test_htk header_test htk)
	add_test (misc_test_htk misc_test htk)

	### avr-tests
	add_test (write_read_test_avr write_read_test avr)
	add_test (header_test_avr header_test avr)
	add_test (misc_test_avr misc_test avr)

	### sds-tests
	add_test (write_read_test_sds write_read_test sds)
	add_test (header_test_sds header_test sds)
	add_test (misc_test_sds misc_test sds)

	# sd2-tests
	add_test (write_read_test_sd2 write_read_test sd2)

	### wve-tests
	add_test (lossy_comp_test_wve lossy_comp_test wve)

	### mpc2k-tests
	add_test (write_read_test_mpc2k write_read_test mpc2k)
	add_test (header_test_mpc2k header_test mpc2k)
	add_test (misc_test_mpc2k misc_test mpc2k)

	### flac-tests
	add_test (write_read_test_flac write_read_test flac)
	add_test (compression_size_test_flac compression_size_test flac)
	add_test (string_test_flac string_test flac)

	### vorbis-tests
	add_test (ogg_test ogg_test)
	add_test (compression_size_test_vorbis compression_size_test vorbis)
	add_test (lossy_comp_test_ogg_vorbis lossy_comp_test ogg_vorbis)
	add_test (string_test_ogg string_test ogg)
	add_test (misc_test_ogg misc_test ogg)

	### io-tests
	add_test (stdio_test stdio_test)
	add_test (pipe_test pipe_test)
	add_test (virtual_io_test virtual_io_test)

endif (BUILD_TESTING)
