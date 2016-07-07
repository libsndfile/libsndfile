set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(ALAC)
include_directories(G72x)
include_directories(GSM610)

add_subdirectory(ALAC)
add_subdirectory(G72x)
add_subdirectory(GSM610)

<<<<<<< 0ed77b697139da995110dbba9095a1f48ee4a013
=======
add_definitions(-DPACKAGE_NAME="libsndfile")
add_definitions(-DPACKAGE_VERSION="1.0.28pre1")

add_definitions(-DWITH_OPENCV2)
add_definitions(-DCPU_CLIPS_POSITIVE=0)
add_definitions(-DCPU_CLIPS_NEGATIVE=0)
add_definitions(-DSIZEOF_SF_COUNT_T=8) #need to calculate
add_definitions(-DHAVE_UNISTD_H)

>>>>>>> Reworked CMake build system
set (LIBSNDFILE_SOURCE_FILES
	aiff.c
	alac.c
	alaw.c
	au.c
	audio_detect.c
	avr.c
	broadcast.c
	caf.c
	cart.c
	chanmap.c
	chunk.c
	command.c
	common.c
	dither.c
	double64.c
	dwd.c
	dwvw.c
	file_io.c
	flac.c
	float32.c
	g72x.c
	gsm610.c
	htk.c
	id3.c
	ima_adpcm.c
	ima_oki_adpcm.c
	interleave.c
	ircam.c
	macos.c
	mat4.c
	mat5.c
	mpc2k.c
	ms_adpcm.c
	nist.c
	ogg.c
	ogg_opus.c
	ogg_pcm.c
	ogg_speex.c
	ogg_vorbis.c
	paf.c
	pcm.c
	pvf.c
	raw.c
	rf64.c
	rx2.c
	sd2.c
	sds.c
	sndfile.c
	strings.c
	svx.c
	txw.c
	ulaw.c
	voc.c
	vox_adpcm.c
	w64.c
	wav.c
	wavlike.c
	windows.c
	wve.c
	xi.c
<<<<<<< 0ed77b697139da995110dbba9095a1f48ee4a013
	)

add_library(source OBJECT ${LIBSNDFILE_SOURCE_FILES})
set_property(TARGET source PROPERTY POSITION_INDEPENDENT_CODE TRUE)
=======
)

add_library(source OBJECT ${LIBSNDFILE_SOURCE_FILES})
>>>>>>> Reworked CMake build system
