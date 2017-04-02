# CMake implementation of AutoGen
# Copyright (C) 2017 Anonymous Maarten <anonymous.maarten@gmail.com>

set(AUTOGEN_SCRIPT "${PROJECT_SOURCE_DIR}/CMake/CMakeAutoGenScript.cmake")

function(add_autogen_target INPUT OUTPUTDIR)
    set(OUTPUTFILES "${ARGN}")

    if (OUTPUTDIR)
        set(PREFIX "${OUTPUTDIR}/")
    else()
        set(PREFIX "")
    endif()

    set(ARTIFACTS)
    foreach(OUTPUTFILE ${OUTPUTFILES})
        list(APPEND ARTIFACTS "${PREFIX}${OUTPUTFILE}")
    endforeach()

    set(EXTRA_ARGS)
    if (AUTOGEN_DEBUG)
        list(APPEND EXTRA_ARGS "-DDEBUG=1")
    endif()
    if (OUTPUTDIR)
        list(APPEND EXTRA_ARGS "-DOUTPUTDIR=${OUTPUTDIR}")
    endif()

    add_custom_command(
        OUTPUT ${ARTIFACTS}
        COMMAND ${CMAKE_COMMAND} "-DDEFINITION=${CMAKE_CURRENT_SOURCE_DIR}/${INPUT}" ${EXTRA_ARGS} -P "${AUTOGEN_SCRIPT}"
        MAIN_DEPENDENCY "${INPUT}"
        DEPENDS "${AUTOGEN_SCRIPT}"
        COMMENT "AutoGen: parsing ${INPUT}, generating ${OUTPUTFILES}"
    )
endfunction()
