if (SPEEX_INCLUDE_DIR)
	set (SPEEX_FIND_QUIETLY TRUE)
endif ()

find_path (SPEEX_INCLUDE_DIR speex/speex.h)
find_library (SPEEX_LIBRARY NAMES speex)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SPEEX DEFAULT_MSG SPEEX_INCLUDE_DIR SPEEX_LIBRARY)

mark_as_advanced (SPEEX_INCLUDE_DIR SPEEX_LIBRARY)

set(SPEEX_INCLUDE_DIRS ${SPEEX_INCLUDE_DIR})
set(SPEEX_LIBRARIES ${SPEEX_LIBRARY})

