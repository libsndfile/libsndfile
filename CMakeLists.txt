#
# Initial CMake and project setup
#

# lowest version supporting portable endianness check
cmake_minimum_required (VERSION 3.0.0)

# Options
option (ENABLE_TESTING "Enable testing" OFF)

# Set software information
project(libsndfile VERSION 1.0.28)
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
add_definitions(-DHAVE_EXTERNAL_XIPH_LIBS=0)

#
# Setup configuration
#

configure_file(${CMAKE_SOURCE_DIR}/src/config.h.cmake ${CMAKE_SOURCE_DIR}/src/config.h)
configure_file(${CMAKE_SOURCE_DIR}/src/sndfile.h.in ${CMAKE_SOURCE_DIR}/src/sndfile.h)

#
# Final project settings
#

add_subdirectory(src)

add_library(sndfile SHARED
	$<TARGET_OBJECTS:source> 
	$<TARGET_OBJECTS:ALAC>
	$<TARGET_OBJECTS:G72x>
	$<TARGET_OBJECTS:GSM610>
	)

add_library(sndfile_static STATIC
	$<TARGET_OBJECTS:source> 
	$<TARGET_OBJECTS:ALAC>
	$<TARGET_OBJECTS:G72x>
	$<TARGET_OBJECTS:GSM610>
	)

target_link_libraries (sndfile LINK_PRIVATE ${EXTERNAL_XIPH_LIBS} LINK_PUBLIC m)
target_link_libraries (sndfile_static LINK_PRIVATE ${EXTERNAL_XIPH_LIBS} LINK_PUBLIC m)

set_target_properties (sndfile sndfile_static
			PROPERTIES
			VERSION ${PROJECT_VERSION}
			POSITION_INDEPENDENT_CODE TRUE)
target_include_directories (sndfile PUBLIC
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories (sndfile_static PUBLIC
		PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(programs)
if (ENABLE_TESTING)
enable_testing()
add_subdirectory(tests)
else ()
add_subdirectory(tests EXCLUDE_FROM_ALL)
endif ()
