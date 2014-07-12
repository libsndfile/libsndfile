
include (TestBigEndian)

function (try_compile_c_result C_FILE RESULT_NAME RESULT_PASS RESULT_FAIL)
	try_compile (COMPILE_RESULT
		${CMAKE_BINARY_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/CMake/compiler_is_gcc.c
		# OUTPUT_VARIABLE LOG2
		)

    if (${COMPILE_RESULT})
		set (${RESULT_NAME} ${RESULT_PASS} PARENT_SCOPE)
	else (${VARNAME})
		set (${RESULT_NAME} ${RESULT_FAIL} PARENT_SCOPE)
		endif (${COMPILE_RESULT})

	endfunction ()


try_compile_c_result (CMake/compiler_is_gcc.c COMPILER_IS_GCC 1 0)
