# Build recipe for building programs in the programs/ directory.
function (lsf_build_program prog_name extra_libs)
	add_executable (programs/${prog_name}
		programs/common.c
		programs/${prog_name}.c
		)
	target_link_libraries (programs/${prog_name} sndfile ${extra_libs})
	endfunction ()


# Build recipe for building C tests in the src/ directory.
function (lsf_build_src_test_c test_name extra_files)
	add_executable (src/${test_name}
		src/${test_name}.c
		${extra_files}
		)
	target_link_libraries (src/${test_name} m)
	set_target_properties (src/${test_name}
		PROPERTIES
		EXCLUDE_FROM_DEFAULT_BUILD TRUE
		EXCLUDE_FROM_ALL TRUE
		)
	add_dependencies (check src/${test_name})
	endfunction ()

# Build recipe for building C tests in the tests/ directory.
function (lsf_build_test_c test_name extra_files)
	add_executable (tests/${test_name}
		tests/utils.c
		tests/${test_name}.c
		${extra_files}
		)
	target_link_libraries (tests/${test_name} sndfile)
	set_target_properties (tests/${test_name}
		PROPERTIES
		EXCLUDE_FROM_DEFAULT_BUILD TRUE
		EXCLUDE_FROM_ALL TRUE
		)
	add_dependencies (check tests/${test_name})
	endfunction ()

# Build recipe for building C++ tests in the tests/ directory.
function (lsf_build_test_cc test_name)
	add_executable (tests/${test_name}
		tests/utils.c
		tests/${test_name}.cc
		)
	target_link_libraries (tests/${test_name} sndfile)
	set_target_properties (tests/${test_name}
		PROPERTIES
		EXCLUDE_FROM_DEFAULT_BUILD TRUE
		EXCLUDE_FROM_ALL TRUE
		)
	add_dependencies (check tests/${test_name})
	endfunction ()
