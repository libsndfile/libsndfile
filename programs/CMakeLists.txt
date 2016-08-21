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
if (COMPILER_IS_GCC)
	target_compile_options(${program} PRIVATE -Wall -Wextra)
endif ()
endforeach ()

# This is the BeOS version of sndfile-play. It needs to be compiled with the C++
# compiler.
if (BEOS OR HAIKU)
	add_executable (sndfile-play-beos sndfile-play-beos.cpp)
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


# TODO : What about the *BSD sndio libs?

