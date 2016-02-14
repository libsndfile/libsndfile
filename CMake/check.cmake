include (CheckFunctionExists)
include (CheckIncludeFile)
include (CheckLibraryExists)
include (CheckTypeSize)
include (TestBigEndian)

function (lsf_try_compile_c_result c_file result_name result_pass result_fail)
	try_compile (compile_result
		${CMAKE_BINARY_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/${c_file}
		OUTPUT_VARIABLE LOG2
		)

    if (${compile_result})
		set (${result_name} ${result_pass} PARENT_SCOPE)
	else (${compile_result})
		set (${result_name} ${result_fail} PARENT_SCOPE)
		endif (${compile_result})

	endfunction ()

function (lsf_check_include_file header_name result_name)
	check_include_file (${header_name} header_${result_name})

    if (header_${result_name})
		set (${result_name} 1 PARENT_SCOPE)
	else (header_${result_name})
		set (${result_name} 0 PARENT_SCOPE)
		endif (header_${result_name})

	unset (header_${result_name})
	endfunction ()

function (lsf_check_type_size type_name result_size)
	string (REPLACE "*" "_P"  tmp1 ${type_name})
	string (REPLACE " " "_"  tmp2 ${tmp1})

	check_type_size (${type_name} size_${tmp2})

    if (size_${type_name})
		set (${result_size} ${size_${type_name}} PARENT_SCOPE)
	else (size_${type_name})
		set (${result_size} 0 PARENT_SCOPE)
		endif (size_${type_name})

	unset (tmp1)
	unset (tmp2)
	unset (size_${tmp2})
	endfunction ()

function (lsf_check_function_exists func_name result_name)
	check_function_exists (${func_name} func_${result_name})

    if (func_${result_name})
		set (${result_name} 1 PARENT_SCOPE)
	else (func_${result_name})
		set (${result_name} 0 PARENT_SCOPE)
		endif (func_${result_name})

	unset (func_${result_name})
	endfunction ()

# Unix does not link libm by default while windows does. We therefore have
# a special function for testing math functions.
function (lsf_check_math_function_exists func_name result_name)
	if (${UNIX})
		check_library_exists (m ${func_name} "" func_${result_name})
	else (${UNIX})
		check_function_exists (${func_name} func_${result_name})
		endif (${UNIX})

    if (func_${result_name})
		set (${result_name} 1 PARENT_SCOPE)
	else (func_${result_name})
		set (${result_name} 0 PARENT_SCOPE)
		endif (func_${result_name})

	unset (func_${result_name})
	endfunction ()

function (lsf_check_library_exists lib_name lib_func location result_name)
	check_library_exists (${lib_name} ${lib_func} "${location}" lib_${result_name})

    if (lib_${result_name})
		set (${result_name} 1 PARENT_SCOPE)
	else (lib_${result_name})
		set (${result_name} 0 PARENT_SCOPE)
		endif (lib_${result_name})

	unset (lib_${result_name})
	endfunction ()

