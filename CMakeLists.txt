#
# Initial CMake and project setup
#

# lowest version supporting portable endianness check
cmake_minimum_required (VERSION 3.0.0)

set(PROJECT_NAME libsndfile)
project(${PROJECT_NAME})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set_directory_properties(PROPERTIES EP_PREFIX ${CMAKE_BINARY_DIR}/library-build)

set(CMAKE_C_FLAGS "-std=gnu11 -O3")

#
# Setup libsndfile definitions
#

include(CheckIncludeFiles)
include(CheckTypeSize)
include(TestBigEndian)

set(TYPEOF_SF_COUNT_T int64_t)
set(SF_COUNT_MAX 0x7fffffffffffffffll)

add_definitions(-DPACKAGE_NAME="libsndfile")
add_definitions(-DPACKAGE_VERSION="1.0.28pre1")
add_definitions(-DCPU_CLIPS_POSITIVE=0)
add_definitions(-DCPU_CLIPS_NEGATIVE=0)

TEST_BIG_ENDIAN (BIG_ENDIAN)
if (${BIG_ENDIAN})
	add_definitions(-DWORDS_BIGENDIAN=1)
	add_definitions(-DCPU_IS_BIG_ENDIAN=1)
	add_definitions(-DCPU_IS_LITTLE_ENDIAN=0)
else (${LITTLE_ENDIAN})
	add_definitions(-DWORDS_BIGENDIAN=0)
	add_definitions(-DCPU_IS_LITTLE_ENDIAN=1)
	add_definitions(-DCPU_IS_BIG_ENDIAN=0)
endif ()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	add_definitions(-DOS_IS_WIN32=1)
	add_definitions(-DUSE_WINDOWS_API=1)
	add_definitions(-DWIN32_TARGET_DLL=1)
else (NOT ${WINDOWS})
	add_definitions(-DOS_IS_WIN32=0)
	add_definitions(-DUSE_WINDOWS_API=0)
	add_definitions(-DWIN32_TARGET_DLL=0)
endif ()

if (${CMAKE_SYSTEM_NAME} STREQUAL "OpenBSD")
	add_definitions(-DOS_IS_OPENBSD=1)
else ()
	add_definitions(-DOS_IS_OPENBSD=0)
endif ()

if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
	add_definitions(-DCOMPILER_IS_GCC)
endif ()

check_include_files(unistd.h HAVE_UNISTD_H)
if (HAVE_UNISTD_H)
	add_definitions(-DHAVE_UNISTD_H)
endif ()

check_type_size(int64_t SIZEOF_SF_COUNT_T)
if (SIZEOF_SF_COUNT_T)
	add_definitions(-DSIZEOF_SF_COUNT_T=${SIZEOF_SF_COUNT_T})
endif ()

#
# Final project settings
#

configure_file(${CMAKE_SOURCE_DIR}/src/sndfile.h.in ${CMAKE_SOURCE_DIR}/src/sndfile.h)

add_subdirectory(src)

add_library(sndfile SHARED
	$<TARGET_OBJECTS:source> 
	$<TARGET_OBJECTS:ALAC>
	$<TARGET_OBJECTS:G72x>
	$<TARGET_OBJECTS:GSM610>
	)

