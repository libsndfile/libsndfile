
function(lsf_version_majorversion LIBVERSION LIBVERSIONMAJOR)
	file(READ "${CMAKE_SOURCE_DIR}/configure.ac" CONF_AC_CONTENT)
	string(REGEX MATCH "AC_INIT\\(\\[libsndfile\\],\\[([0-9a-zA-Z\\.]+)\\]" MATCH_AC_INIT "{${CONF_AC_CONTENT}")
	set(LIBVERSION_VAL "${CMAKE_MATCH_1}")
	string(REGEX MATCH "^([0-9]+)" MATCH_MAJOR "${LIBVERSION_VAL}")
	if(NOT MATCH_AC_INIT OR NOT MATCH_MAJOR)
		message(FATAL_ERROR "Could not determine version")
		endif()
	set("${LIBVERSION}" "${LIBVERSION_VAL}" PARENT_SCOPE)
	set("${LIBVERSIONMAJOR}" "${MATCH_MAJOR}" PARENT_SCOPE)
	endfunction()

function (file_line_count filename variable)
	file(STRINGS "${filename}" filename_lines)
	list(LENGTH filename_lines line_count)

	set (${variable} ${line_count} PARENT_SCOPE)
	endfunction ()

function (assert_line_count_non_zero filename)

	file_line_count (${filename} line_count)

	if (${line_count} LESS 1)
		message (FATAL_ERROR "Line count of ${filename} is ${line_count}, which is less than expected.")
		endif ()

	endfunction ()
