include (CheckIncludeFile)
include (CheckTypeSize)
include (TestBigEndian)

function (mn_try_compile_c_result C_FILE RESULT_NAME RESULT_PASS RESULT_FAIL)
	try_compile (COMPILE_RESULT
		${CMAKE_BINARY_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/CMake/compiler_is_gcc.c
		# OUTPUT_VARIABLE LOG2
		)

    if (${COMPILE_RESULT})
		set (${RESULT_NAME} ${RESULT_PASS} PARENT_SCOPE)
	else (${COMPILE_RESULT})
		set (${RESULT_NAME} ${RESULT_FAIL} PARENT_SCOPE)
		endif (${COMPILE_RESULT})

	endfunction ()

function (mn_check_include_file HEADER_NAME RESULT_NAME)
	check_include_file (${HEADER_NAME} HEADER_${RESULT_NAME})

    if (HEADER_${RESULT_NAME})
		set (${RESULT_NAME} 1 PARENT_SCOPE)
	else (HEADER_${RESULT_NAME})
		set (${RESULT_NAME} 0 PARENT_SCOPE)
		endif (HEADER_${RESULT_NAME})

	set (HEADER_${RESULT_NAME}) # Clear the variable.
	endfunction ()

function (mn_check_type_size TYPE_NAME RESULT_SIZE)
	string (REPLACE "*" "_P"  TMP1 ${TYPE_NAME})
	string (REPLACE " " "_"  TMP2 ${TMP1})

	check_type_size (${TYPE_NAME} SIZE_${TMP2})

    if (SIZE_${TYPE_NAME})
		set (${RESULT_SIZE} ${SIZE_${TYPE_NAME}} PARENT_SCOPE)
	else (SIZE_${TYPE_NAME})
		set (${RESULT_SIZE} 0 PARENT_SCOPE)
		endif (SIZE_${TYPE_NAME})

	set (TMP1)
	set (TMP2)
	set (SIZE_${TMP2})
	endfunction ()
