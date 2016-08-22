# Public libsndfile headers
set (include_HEADERS sndfile.hh)
set (nodist_include_HEADERS sndfile.h)

# Private libsndfile headers
set (noinst_HEADERS common.h sfconfig.h sfendian.h wavlike.h sf_unistd.h ogg.h chanmap.h)

# Common libsndfile sources
set (COMMON 
	common.c file_io.c command.c pcm.c ulaw.c alaw.c float32.c
	double64.c ima_adpcm.c ms_adpcm.c gsm610.c dwvw.c vox_adpcm.c
	interleave.c strings.c dither.c cart.c broadcast.c audio_detect.c
 	ima_oki_adpcm.c ima_oki_adpcm.h alac.c chunk.c ogg.c chanmap.c
	id3.c)
if (WIN32)
list (APPEND COMMON windows.c)
if (BUILD_SHARED_LIBS)
list (APPEND COMMON version-metadata.rc)
endif ()
endif ()
	
# File specific libsndfile sources
set (FILESPECIFIC sndfile.c aiff.c au.c avr.c caf.c dwd.c flac.c g72x.c htk.c ircam.c
		macos.c mat4.c mat5.c nist.c paf.c pvf.c raw.c rx2.c sd2.c
		sds.c svx.c txw.c voc.c wve.c w64.c wavlike.c wav.c xi.c mpc2k.c rf64.c
		ogg_vorbis.c ogg_speex.c ogg_pcm.c ogg_opus.c)
	
# GSM610 sources	
set (libgsm_SOURCES GSM610/config.h GSM610/gsm.h GSM610/gsm610_priv.h
		GSM610/add.c GSM610/code.c GSM610/decode.c GSM610/gsm_create.c
		GSM610/gsm_decode.c GSM610/gsm_destroy.c GSM610/gsm_encode.c
		GSM610/gsm_option.c GSM610/long_term.c GSM610/lpc.c GSM610/preprocess.c
		GSM610/rpe.c GSM610/short_term.c GSM610/table.c)
source_group ("GSM610" FILES ${libgsm_SOURCES})

# G72x sources
set (libg72x_SOURCES G72x/g72x.h G72x/g72x_priv.h
		G72x/g721.c G72x/g723_16.c G72x/g723_24.c G72x/g723_40.c G72x/g72x.c)
source_group ("G72x" FILES ${libg72x_SOURCES})

# ALAC sources

set (libalac_SOURCES ALAC/ALACAudioTypes.h ALAC/ALACBitUtilities.h
		ALAC/EndianPortable.h ALAC/aglib.h ALAC/dplib.h ALAC/matrixlib.h
		ALAC/alac_codec.h ALAC/shift.h
		ALAC/ALACBitUtilities.c ALAC/ag_dec.c
		ALAC/ag_enc.c ALAC/dp_dec.c ALAC/dp_enc.c ALAC/matrix_dec.c
		ALAC/matrix_enc.c ALAC/alac_decoder.c ALAC/alac_encoder.c)
source_group ("ALAC" FILES ${libalac_SOURCES})
		
# TODO: Generate and add version-metadata.rc

# TODO: Private symbols

# Headers to be installed on target machine
set (libsndfile_PUBLIC_HEADERS ${include_HEADERS} ${nodist_include_HEADERS})
set (libsndfile_SOURCES ${COMMON} ${FILESPECIFIC} ${noinst_HEADERS} ${libgsm_SOURCES} ${libg72x_SOURCES} ${libalac_SOURCES} ${libsndfile_PUBLIC_HEADERS})
if (WIN32)
list (APPEND libsndfile_SOURCES ${PROJECT_NAME}-${PROJECT_VERSION_MAJOR}.def)
endif (WIN32)

add_library(sndfile	${libsndfile_SOURCES})
if (M_LIBRARY)
target_link_libraries (sndfile PUBLIC ${EXTERNAL_XIPH_LIBS} PRIVATE ${M_LIBRARY})
endif ()
set_property(TARGET sndfile PROPERTY C_STANDARD 99)
set_target_properties (sndfile
			PROPERTIES
			SOVERSION ${PROJECT_VERSION_MAJOR}
			VERSION ${PROJECT_VERSION}
			PUBLIC_HEADER "${libsndfile_PUBLIC_HEADERS}")
if (WIN32)
	set_target_properties(sndfile PROPERTIES PREFIX "")
if (BUILD_SHARED_LIBS)
	set_target_properties(sndfile PROPERTIES OUTPUT_NAME "${PROJECT_NAME}-${PROJECT_VERSION_MAJOR}")
else (NOT BUILD_SHARED_LIBS)
	set_target_properties(sndfile PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")
endif ()
endif (WIN32)
target_include_directories (sndfile PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
if (COMPILER_IS_GCC)
	target_compile_options(sndfile PRIVATE -Wall -Wextra)
endif ()

