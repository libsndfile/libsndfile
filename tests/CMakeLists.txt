set(TESTS	aiff_rw_test
		alaw_test 
		benchmark 
		channel_test 
		checksum_test 
		chunk_test 
		command_test 
		compression_size_test 
		cpp_test 
		dither_test 
		dwvw_test 
		error_test 
		external_libs_test 
		fix_this 
		floating_point_test
		format_check_test 
		headerless_test 
		header_test 
		largefile_test 
		locale_test 
		lossy_comp_test 
		long_read_write_test 
		misc_test 
		multi_file_test 
		ogg_test 
		pcm_test 
		peak_chunk_test 
		pipe_test 
		raw_test 
		rdwr_test 
		scale_clip_test 
		sfversion 
		stdin_test 
		stdio_test 
		stdout_test 
		string_test 
		ulaw_test 
		virtual_io_test 
		win32_ordinal_test 
		win32_test 
		write_read_test
	)

if (NOT WIN32)
	list (REMOVE_ITEM TESTS win32_ordinal_test win32_test)
endif ()

enable_testing()
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS ${TESTS})
set(CMAKE_CTEST_COMMAND ctest -V)

foreach (test ${TESTS})
	add_executable(${test} 
			${CMAKE_SOURCE_DIR}/tests/${test}
			${CMAKE_SOURCE_DIR}/tests/utils.c
			${CMAKE_SOURCE_DIR}/tests/generate.c
			${CMAKE_SOURCE_DIR}/tests/dft_cmp.c
			)
	target_link_libraries(${test} sndfile)
	# bit jenky, should be an easier way to exclude
	if (NOT (${test} MATCHES stdin_test OR ${test} MATCHES "stdout_test"))
		add_test(NAME ${test}  COMMAND ${test} all)
	endif()
    add_dependencies(check ${test})
endforeach ()
