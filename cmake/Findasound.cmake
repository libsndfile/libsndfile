if (NOT MSVC)
    include(FindPkgConfig)
    pkg_check_modules(PC_ASOUND "asound")
    if (NOT PC_ASOUND_FOUND)
        pkg_check_modules(PC_ASOUND "asound")
    endif (NOT PC_ASOUND_FOUND)
    if (PC_ASOUND_FOUND)
        # some libraries install the headers is a subdirectory of the include dir
        # returned by pkg-config, so use a wildcard match to improve chances of finding
        # headers and SOs.
        set(PC_ASOUND_INCLUDE_HINTS ${PC_ASOUND_INCLUDE_DIRS} ${PC_ASOUND_INCLUDE_DIRS}/*)
        set(PC_ASOUND_LIBRARY_HINTS ${PC_ASOUND_LIBRARY_DIRS} ${PC_ASOUND_LIBRARY_DIRS}/*)
    endif(PC_ASOUND_FOUND)
endif (NOT MSVC)

find_path (
    ASOUND_INCLUDE_DIRS
    NAMES alsa/asoundlib.h
    HINTS ${PC_ASOUND_INCLUDE_HINTS}
)

find_library (
    ASOUND_LIBRARIES
    NAMES asound
    HINTS ${PC_ASOUND_LIBRARY_HINTS}
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    ASOUND
    REQUIRED_VARS ASOUND_LIBRARIES ASOUND_INCLUDE_DIRS
)
mark_as_advanced(
    ASOUND_FOUND
    ASOUND_LIBRARIES ASOUND_INCLUDE_DIRS
)
