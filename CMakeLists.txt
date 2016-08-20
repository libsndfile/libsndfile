#
# Initial CMake and project setup
#

# lowest version supporting C_STANDARD property
cmake_minimum_required (VERSION 3.1.3)

# Options
option (BUILD_SHARED_LIBS "Build shared libraries" OFF)
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

#
# Setup configuration
#

configure_file(${CMAKE_SOURCE_DIR}/src/config.h.cmake ${CMAKE_SOURCE_DIR}/src/config.h)
configure_file(${CMAKE_SOURCE_DIR}/src/sndfile.h.in ${CMAKE_SOURCE_DIR}/src/sndfile.h)

#
# Final project settings
#

add_subdirectory(src)

add_subdirectory(programs)
if (ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
else ()
    add_subdirectory(tests EXCLUDE_FROM_ALL)
endif ()
