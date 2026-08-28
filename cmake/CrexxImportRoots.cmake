include_guard(GLOBAL)

# Publish an exact set of compiler metadata inputs into a private directory.
# Consumers must still name the returned files in DEPENDS so Ninja can verify
# the generated-file relationship as well as the target-level ordering edge.
function(crexx_add_import_root TARGET_NAME)
    set(options)
    set(one_value_args DIRECTORY OUTPUT_VARIABLE COMMENT)
    set(multi_value_args FILES DEPENDS)
    cmake_parse_arguments(
            CREXX_IMPORT
            "${options}"
            "${one_value_args}"
            "${multi_value_args}"
            ${ARGN})

    if(NOT CREXX_IMPORT_DIRECTORY)
        message(FATAL_ERROR
                "crexx_add_import_root(${TARGET_NAME}) requires DIRECTORY")
    endif()
    if(NOT CREXX_IMPORT_FILES)
        message(FATAL_ERROR
                "crexx_add_import_root(${TARGET_NAME}) requires FILES")
    endif()

    set(_outputs)
    set(_output_names)
    foreach(_input IN LISTS CREXX_IMPORT_FILES)
        get_filename_component(_name "${_input}" NAME)
        if(_name IN_LIST _output_names)
            message(FATAL_ERROR
                    "crexx_add_import_root(${TARGET_NAME}) has duplicate output ${_name}")
        endif()
        list(APPEND _output_names "${_name}")
        list(APPEND _outputs "${CREXX_IMPORT_DIRECTORY}/${_name}")
    endforeach()

    set(_comment "Staging deterministic compiler imports for ${TARGET_NAME} ...")
    if(CREXX_IMPORT_COMMENT)
        set(_comment "${CREXX_IMPORT_COMMENT}")
    endif()

    add_custom_command(
            OUTPUT ${_outputs}
            COMMAND ${CMAKE_COMMAND} -E make_directory
                    "${CREXX_IMPORT_DIRECTORY}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${CREXX_IMPORT_FILES}
                    "${CREXX_IMPORT_DIRECTORY}"
            DEPENDS ${CREXX_IMPORT_FILES} ${CREXX_IMPORT_DEPENDS}
            COMMENT "${_comment}"
            VERBATIM)
    add_custom_target(${TARGET_NAME} DEPENDS ${_outputs})

    if(CREXX_IMPORT_OUTPUT_VARIABLE)
        set(${CREXX_IMPORT_OUTPUT_VARIABLE} ${_outputs} PARENT_SCOPE)
    endif()
endfunction()
