# cmakelists.txt for libsndfile
cmake_minimum_required (VERSION 2.8.12)

set (BASEPATH "${CMAKE_SOURCE_DIR}")

project (libsndfile C)

include (CMake/libsndfile.cmake)

if (MSVC)
	set (CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4996")
	add_definitions ("/wd4244 /wd4996")

	set (HAVE_BYTESWAP_H 0)
	set (HAVE_INTTYPES_H 1)
	set (HAVE_STDINT_H 0)
	set (SIZEOF_LONG 4)
	set (COMPILER_IS_GCC 0)
	set (OS_IS_WIN32 1)
	set (USE_WINDOWS_API 1)
else (MSVC)
	# Assume this is Linux or some other POSIX-like system (including OS X).
	set (OS_IS_WIN32 0)
	set (USE_WINDOWS_API 0)
	set (COMPILER_IS_GCC ${COMPILER_IS_GCC})

	set (HAVE_EXTERNAL_LIBS 0)
	set (CPU_CLIPS_NEGATIVE 0)
	set (CPU_CLIPS_POSITIVE 0)
	set (HAVE_GETTIMEOFDAY 1)
	set (_POSIX_SOURCE 1)

	set (SF_COUNT_MAX 0x7fffffffffffffffll)
	set (ENABLE_EXPERIMENTAL_CODE 0)
	set (HAVE_DECL_S_IRGRP 1)
	set (HAVE_FSTAT64 1)
	set (HAVE_FSYNC 1)
	set (OSX_DARWIN_VERSION 0)
endif (MSVC)

#-------------------------------------------------------------------------------
# Project definitions follow.

configure_file (src/sndfile.h.in src/sndfile.h)
configure_file (CMake/config.h.in src/config.h)

include_directories (src)

file (GLOB libsndfile_sources
	src/aiff.c
	src/alaw.c
	src/au.c
	src/audio_detect.c
	src/avr.c
	src/broadcast.c
	src/cart.c
	src/chanmap.c
	src/chunk.c
	src/command.c
	src/common.c
	src/dither.c
	src/double64.c
	src/dwd.c
	src/dwvw.c
	src/file_io.c
	src/flac.c
	src/float32.c
	src/g72x.c
	src/gsm610.c
	src/htk.c
	src/id3.c
	src/ima_adpcm.c
	src/ima_oki_adpcm.c
	src/interleave.c
	src/ircam.c
	src/macbinary3.c
	src/macos.c
	src/mat4.c
	src/mat5.c
	src/mpc2k.c
	src/ms_adpcm.c
	src/new.c
	src/nist.c
	src/ogg.c
	src/ogg_opus.c
	src/ogg_pcm.c
	src/ogg_speex.c
	src/ogg_vorbis.c
	src/paf.c
	src/pcm.c
	src/pvf.c
	src/raw.c
	src/rf64.c
	src/rx2.c
	src/sd2.c
	src/sds.c
	src/sndfile.c
	src/strings.c
	src/svx.c
	src/txw.c
	src/ulaw.c
	src/voc.c
	src/vox_adpcm.c
	src/w64.c
	src/wav.c
	src/wav_w64.c
	src/windows.c
	src/wve.c
	src/xi.c

	src/G72x/g721.c
	src/G72x/g72x.c
	src/G72x/g723_16.c
	src/G72x/g723_24.c
	src/G72x/g723_40.c

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
	src/GSM610/table.c
	)

file (GLOB src_tests_sources
	src/test_audio_detect.c
	src/test_broadcast_var.c
	src/test_cart_var.c
	src/test_conversions.c
	src/test_file_io.c
	src/test_float.c
	src/test_ima_oki_adpcm.c
	src/test_log_printf.c
	src/test_main.c
	src/test_strncpy_crlf.c
	src/GSM610/g72x_test.c
	)

add_library (libsndfile
	${libsndfile_sources}
	)
target_link_libraries (libsndfile m)

add_executable (sndfile-info programs/sndfile-info.c)
target_link_libraries (sndfile-info libsndfile)
