find_package(Vorbis)
if (${VORBIS_FOUND})
    INCLUDE_DIRECTORIES(${VORBIS_INCLUDE_DIR})
elseif ($(ENABLE_EXTERNAL_LIBS))
    ExternalProject_Add(
        Vorbis
        GIT_REPOSITORY      "https://github.com/xiph/vorbis"
        SOURCE_DIR          ${CMAKE_CURRENT_SOURCE_DIR}/lib/vorbis
        UPDATE_COMMAND      ""
        INSTALL_COMMAND     ""
        LOG_DOWNLOAD        ON
        LOG_UPDATE          ON
        LOG_CONFIGURE       ON
        LOG_BUILD           ON
        LOG_TEST            ON
        LOG_INSTALL         ON
        ExternalProject_Get_Property(FLAC SOURCE_DIR)
        ExternalProject_Get_Property(FLAC BINARY_DIR)

        SET(VORBIS_SOURCE_DIR ${SOURCE_DIR})
        SET(VORBIS_BINARY_DIR ${BINARY_DIR})
        SET(VORBIS_LIBRARIES ${FLAC_BINARY_DIR}/lib/.libs/libFLAC.dylib)
        SET(DEPENDENCIES ${DEPENDENCIES} FLAC)
    )
endif ()
set(EXTERNAL_XIPH_LIBS ${EXTERNAL_XIPH_LIBS} ${VORBIS_LIBRARIES})

find_package(FLAC)
if (${FLAC_FOUND})
    INCLUDE_DIRECTORIES(${FLAC_INCLUDE_DIR})
elseif ($(ENABLE_EXTERNAL_LIBS))
    ExternalProject_Add(
        FLAC
        DEPENDS             Vorbis
        GIT_REPOSITORY      "https://github.com/xiph/flac"
        SOURCE_DIR          ${CMAKE_CURRENT_SOURCE_DIR}/lib/flac
        PATCH_COMMAND       ${CMAKE_CURRENT_SOURCE_DIR}/lib/flac/autogen.sh
        CONFIGURE_COMMAND   ${CMAKE_CURRENT_SOURCE_DIR}/lib/flac/configure --prefix=<INSTALL_DIR>
        BUILD_COMMAND       ${MAKE}
        UPDATE_COMMAND      ""
        INSTALL_COMMAND     ""
        LOG_DOWNLOAD        ON
        LOG_UPDATE          ON
        LOG_CONFIGURE       ON
        LOG_BUILD           ON
        LOG_TEST            ON
        LOG_INSTALL         ON
        ExternalProject_Get_Property(FLAC SOURCE_DIR)
        ExternalProject_Get_Property(FLAC BINARY_DIR)

        SET(FLAC_SOURCE_DIR ${SOURCE_DIR})
        SET(FLAC_BINARY_DIR ${BINARY_DIR})
        SET(FLAC_LIBRARIES ${FLAC_BINARY_DIR}/lib/.libs/libFLAC.dylib)
        SET(DEPENDENCIES ${DEPENDENCIES} FLAC)
    )
endif ()
set(EXTERNAL_XIPH_LIBS ${EXTERNAL_XIPH_LIBS} ${FLAC_LIBRARIES})
