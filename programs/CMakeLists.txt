include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/ALAC)
include_directories(${CMAKE_SOURCE_DIR}/src/G72x)
include_directories(${CMAKE_SOURCE_DIR}/src/GSM610)

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
	target_link_libraries(${program} sndfile)
	set_target_properties (${program} 	
				PROPERTIES
				RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
				)
endforeach ()

if (WIN32)
	target_link_libraries(sndfile-play Winmm.lib)
endif ()

