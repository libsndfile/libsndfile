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

add_definitions(-DPACKAGE_NAME="PROJECT_NAME")
add_definitions(-DPACKAGE_VERSION="${LIB_VERSION}")
add_definitions(-DCPU_CLIPS_POSITIVE=0)
add_definitions(-DCPU_CLIPS_NEGATIVE=0)

if (${CMAKE_SYSTEM_NAME} STREQUAL "OpenBSD")
	add_definitions(-DOS_IS_OPENBSD=1)
else ()
	add_definitions(-DOS_IS_OPENBSD=0)
endif ()

if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
	add_definitions(-DCOMPILER_IS_GCC)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -std=gnu99 -Wall -Wextra" CACHE STRING "" FORCE)
	set (CMAKE_CXX__FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=gnu99 -Wall -Wextra" CACHE STRING "" FORCE)
endif ()

#
#  Find external libs
#

include(${CMAKE_MODULE_PATH}/external_libs.cmake)
add_definitions(-DHAVE_EXTERNAL_XIPH_LIBS=0)

#
# Setup configuration
#

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
			LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/src/.libs
			POSITION_INDEPENDENT_CODE TRUE
			)

add_subdirectory(programs)
if (ENABLE_TESTING)
enable_testing()
add_subdirectory(tests)
else ()
add_subdirectory(tests EXCLUDE_FROM_ALL)
endif ()
