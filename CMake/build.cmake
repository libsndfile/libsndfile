# Build recipe for building programs in the programs/ directory.
function (lsf_build_program prog_name)
	add_executable (${prog_name}
		programs/common.c
		programs/${prog_name}.c
		)
	target_link_libraries (${prog_name} sndfile)
	set_target_properties (${prog_name}
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY programs
		)
	endfunction ()

function (lsf_build_program_extra prog_name extra_libs)
	add_executable (${prog_name}
		programs/common.c
		programs/${prog_name}.c
		)
	target_link_libraries (${prog_name} sndfile ${extra_libs})
	set_target_properties (${prog_name}
		PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY programs
		)
	endfunction ()

# Build recipe for building C tests in the src/ directory.
function (lsf_build_src_test_c test_name extra_files)
	add_executable (${test_name}
		src/${test_name}.c
		${extra_files}
		)
	target_link_libraries (${test_name} m)
	target_include_directories(${test_name} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/src")
	target_include_directories(${test_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
	set_target_properties (${test_name}
		PROPERTIES
		EXCLUDE_FROM_DEFAULT_BUILD TRUE
		EXCLUDE_FROM_ALL TRUE
		RUNTIME_OUTPUT_DIRECTORY src
		)
	add_dependencies (check ${test_name})
	endfunction ()

# Build recipe for building C tests in the tests/ directory.
function (lsf_build_test_c test_name extra_files)
	add_executable (${test_name}
		"tests/${test_name}.c"
		"tests/utils.c"
		"tests/utils.h"
		${extra_files}
		)
	target_link_libraries (${test_name} sndfile)
	target_include_directories(${test_name} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/tests")
	target_include_directories(${test_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
	set_target_properties (${test_name}
		PROPERTIES
		EXCLUDE_FROM_DEFAULT_BUILD TRUE
		EXCLUDE_FROM_ALL TRUE
		RUNTIME_OUTPUT_DIRECTORY tests
		)
	add_dependencies (check ${test_name})
	endfunction ()

# Build recipe for building C++ tests in the tests/ directory.
function (lsf_build_test_cc test_name)
	add_executable (${test_name}
		"tests/${test_name}.cc"
		"tests/utils.c"
		"tests/utils.h"
		)
	target_link_libraries (${test_name} sndfile)
	target_include_directories(${test_name} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/tests")
	target_include_directories(${test_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
	set_target_properties (${test_name}
		PROPERTIES
		EXCLUDE_FROM_DEFAULT_BUILD TRUE
		EXCLUDE_FROM_ALL TRUE
		RUNTIME_OUTPUT_DIRECTORY tests
		)
	add_dependencies (check ${test_name})
	endfunction ()
