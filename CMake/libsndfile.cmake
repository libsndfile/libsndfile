
include (CheckTypeSize)
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

check_type_size (double SIZEOF_DOUBLE)
check_type_size (float SIZEOF_FLOAT)
check_type_size (int SIZEOF_INT)
check_type_size (int64_t SIZEOF_INT64_T)
check_type_size (loff_t SIZEOF_LOFF_T)
check_type_size (long SIZEOF_LONG)
check_type_size (long\ long SIZEOF_LONG_LONG)
check_type_size (offt64_t SIZEOF_OFF64_T)
check_type_size (off_t SIZEOF_OFF_T)
check_type_size (short SIZEOF_SHORT)
check_type_size (size_t SIZEOF_SIZE_T)
check_type_size (ssize_t SIZEOF_SSIZE_T)
check_type_size (void* SIZEOF_VOIDP)
check_type_size (wchar_t SIZEOF_WCHAR_T)

set (SIZEOF_SF_COUNT_T ${SIZEOF_INT64_T})

try_compile_c_result (CMake/compiler_is_gcc.c COMPILER_IS_GCC 1 0)
