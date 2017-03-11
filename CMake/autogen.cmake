
function (lsf_autogen dir basefilename extensions)
	set(artifacts)
	foreach(extension ${extensions})
		set(artifact "${dir}/${basefilename}.${extension}")
		list(APPEND artifacts "${artifact}")
		endforeach()
	if (NOT AUTOGEN)
		message (FATAL_ERROR "Need GNU autogen to generate '${artifacts}'.")
		endif ()

	add_custom_command(
		OUTPUT ${artifacts}
		COMMAND "${AUTOGEN}" --writable -L "${CMAKE_CURRENT_SOURCE_DIR}/${dir}" "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/${basefilename}.def"
		COMMENT "autogen: ${artifacts}"
		MAIN_DEPENDENCY "${dir}/${basefilename}.def" "${dir}/${basefilename}.tpl"
		WORKING_DIRECTORY "${dir}"
		)

	endfunction ()
