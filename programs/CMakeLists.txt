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
			${CMAKE_SOURCE_DIR}/programs/common.c 
			${CMAKE_SOURCE_DIR}/programs/${program}.c
			)
	target_link_libraries(${program} PRIVATE sndfile)
endforeach ()

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


# TODO : What about the *BSD sndio libs?

