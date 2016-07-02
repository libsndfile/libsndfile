
function (file_line_count filename variable)
	# Assume `find_progam (WC wc)` has already set this.
	if (NOT WC)
		message (FATAL_ERROR "Need the 'wc' program to find line coount.")
		endif ()

	if (NOT SED)
		message (FATAL_ERROR "Need the 'sed' program to find line coount.")
		endif ()

	if (NOT (EXISTS "${filename}"))
		message (FATAL_ERROR "File ${filename} does not exist.")
		endif ()

	execute_process (
		COMMAND ${WC} -l ${filename}
		COMMAND ${SED} "s/^[ ]*//"    # wc output on Mac has leading whitespace.
		COMMAND ${SED} "s/ .*//"
		OUTPUT_VARIABLE line_count
		)

	# Tedious!
	string (STRIP ${line_count} line_count)

	set (${variable} ${line_count} PARENT_SCOPE)
	endfunction ()

function (assert_line_count_non_zero filename)

	file_line_count (${filename} line_count)

	if (${line_count} LESS 1)
		message (FATAL_ERROR "Line count of ${filename} is ${line_count}, which is less than expected.")
		endif ()

	endfunction ()
