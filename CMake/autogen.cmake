
function (lsf_autogen dir basefilename)
	# Only generate the file if it does not already exist.
	if (NOT (EXISTS "${CMAKE_SOURCE_DIR}/${dir}/${basefilename}.c"))

		# If it doesn't exist, but we don't have autogen its an error.
		if (NOT AUTOGEN)
			message (FATAL_ERROR "Need GNU autogen to generate '${dir}/${basefilename}.c'.")
			endif ()

		execute_process (
				COMMAND ${AUTOGEN} --writable ${basefilename}.def
				WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/${dir}
				)
		endif ()

	endfunction ()
