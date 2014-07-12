# cmakelists.txt for libsndfile
cmake_minimum_required (VERSION 2.8.4)

set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4996")

CONFIGURE_FILE( src/sndfile.h.in src/sndfile.h )
add_definitions( "/wd4244 /wd4996" )

project (libsndfile C)

include_directories( include )
file( GLOB libsndfile_srcs 
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

file( GLOB libsndfile_test_srcs
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

add_library( libsndfile
	${libsndfile_srcs} 
)

add_executable( sndfile-info programs/sndfile-info.c )
target_link_libraries( sndfile-info libsndfile )
