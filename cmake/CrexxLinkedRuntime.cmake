include(CMakeParseArguments)
include(${CMAKE_CURRENT_LIST_DIR}/CrexxTestAssertions.cmake)

set(_CREXX_LINKED_RUNTIME_MODULE_DIR "${CMAKE_CURRENT_LIST_DIR}")

function(crexx_write_linked_runtime_script)
    set(options ALLOW_RUN_FAILURE)
    set(oneValueArgs OUTPUT NAME WORKING_DIRECTORY RUNNER RXLINK PROGRAM
            EXPECTED_EXIT_CODE EXPECTED_FAILURE_DESCRIPTION)
    set(multiValueArgs EXTRA_ARGS REQUIRED_OUTPUT_REGEX FORBIDDEN_OUTPUT_REGEX)
    cmake_parse_arguments(CREXX "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT CREXX_OUTPUT OR NOT CREXX_NAME OR NOT CREXX_WORKING_DIRECTORY OR NOT CREXX_RUNNER OR NOT CREXX_RXLINK OR NOT CREXX_PROGRAM)
        message(FATAL_ERROR "crexx_write_linked_runtime_script requires OUTPUT, NAME, WORKING_DIRECTORY, RUNNER, RXLINK, and PROGRAM")
    endif()
    if(CREXX_ALLOW_RUN_FAILURE AND NOT "${CREXX_EXPECTED_EXIT_CODE}" STREQUAL "")
        message(FATAL_ERROR
                "${CREXX_NAME}: ALLOW_RUN_FAILURE and EXPECTED_EXIT_CODE are mutually exclusive")
    endif()

    set(_content "include([=[${_CREXX_LINKED_RUNTIME_MODULE_DIR}/CrexxLinkedRuntime.cmake]=])\n")
    string(APPEND _content "set(_crexx_extra_args)\n")
    foreach(_arg IN LISTS CREXX_EXTRA_ARGS)
        string(APPEND _content "list(APPEND _crexx_extra_args [=[${_arg}]=])\n")
    endforeach()
    string(APPEND _content
            "crexx_link_and_run(\n"
            "    NAME [=[${CREXX_NAME}]=]\n"
            "    WORKING_DIRECTORY [=[${CREXX_WORKING_DIRECTORY}]=]\n"
            "    RUNNER [=[${CREXX_RUNNER}]=]\n"
            "    RXLINK [=[${CREXX_RXLINK}]=]\n"
            "    PROGRAM [=[${CREXX_PROGRAM}]=]\n")
    if(CREXX_ALLOW_RUN_FAILURE)
        string(APPEND _content "    ALLOW_RUN_FAILURE\n")
    endif()
    if(NOT "${CREXX_EXPECTED_EXIT_CODE}" STREQUAL "")
        string(APPEND _content
                "    EXPECTED_EXIT_CODE [=[${CREXX_EXPECTED_EXIT_CODE}]=]\n"
                "    EXPECTED_FAILURE_DESCRIPTION [=[${CREXX_EXPECTED_FAILURE_DESCRIPTION}]=]\n")
    endif()
    string(APPEND _content "    EXTRA_ARGS \${_crexx_extra_args}\n")
    foreach(_regex IN LISTS CREXX_REQUIRED_OUTPUT_REGEX)
        string(APPEND _content "    REQUIRED_OUTPUT_REGEX [=[${_regex}]=]\n")
    endforeach()
    foreach(_regex IN LISTS CREXX_FORBIDDEN_OUTPUT_REGEX)
        string(APPEND _content "    FORBIDDEN_OUTPUT_REGEX [=[${_regex}]=]\n")
    endforeach()
    string(APPEND _content ")\n")

    file(GENERATE OUTPUT "${CREXX_OUTPUT}" CONTENT "${_content}")
endfunction()

function(crexx_write_runtime_script)
    set(options)
    set(oneValueArgs OUTPUT NAME WORKING_DIRECTORY RUNNER PROGRAM
            EXPECTED_EXIT_CODE EXPECTED_FAILURE_DESCRIPTION)
    set(multiValueArgs EXTRA_ARGS REQUIRED_OUTPUT_REGEX FORBIDDEN_OUTPUT_REGEX)
    cmake_parse_arguments(CREXX "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT CREXX_OUTPUT OR NOT CREXX_NAME OR NOT CREXX_WORKING_DIRECTORY OR NOT CREXX_RUNNER OR NOT CREXX_PROGRAM)
        message(FATAL_ERROR
                "crexx_write_runtime_script requires OUTPUT, NAME, WORKING_DIRECTORY, RUNNER, and PROGRAM")
    endif()

    set(_content "include([=[${_CREXX_LINKED_RUNTIME_MODULE_DIR}/CrexxLinkedRuntime.cmake]=])\n")
    string(APPEND _content "set(_crexx_extra_args)\n")
    foreach(_arg IN LISTS CREXX_EXTRA_ARGS)
        string(APPEND _content "list(APPEND _crexx_extra_args [=[${_arg}]=])\n")
    endforeach()
    string(APPEND _content
            "crexx_run_checked(\n"
            "    NAME [=[${CREXX_NAME}]=]\n"
            "    WORKING_DIRECTORY [=[${CREXX_WORKING_DIRECTORY}]=]\n"
            "    RUNNER [=[${CREXX_RUNNER}]=]\n"
            "    PROGRAM [=[${CREXX_PROGRAM}]=]\n"
            "    EXPECTED_EXIT_CODE [=[${CREXX_EXPECTED_EXIT_CODE}]=]\n"
            "    EXPECTED_FAILURE_DESCRIPTION [=[${CREXX_EXPECTED_FAILURE_DESCRIPTION}]=]\n"
            "    EXTRA_ARGS \${_crexx_extra_args}\n")
    foreach(_regex IN LISTS CREXX_REQUIRED_OUTPUT_REGEX)
        string(APPEND _content "    REQUIRED_OUTPUT_REGEX [=[${_regex}]=]\n")
    endforeach()
    foreach(_regex IN LISTS CREXX_FORBIDDEN_OUTPUT_REGEX)
        string(APPEND _content "    FORBIDDEN_OUTPUT_REGEX [=[${_regex}]=]\n")
    endforeach()
    string(APPEND _content ")\n")

    file(GENERATE OUTPUT "${CREXX_OUTPUT}" CONTENT "${_content}")
endfunction()

function(_crexx_resolve_module_path raw workdir out_var)
    if(IS_ABSOLUTE "${raw}")
        set(_candidate "${raw}")
    elseif(NOT "${workdir}" STREQUAL "")
        set(_candidate "${workdir}/${raw}")
    else()
        set(_candidate "${raw}")
    endif()

    if(EXISTS "${_candidate}")
        set(${out_var} "${_candidate}" PARENT_SCOPE)
    elseif(NOT "${_candidate}" MATCHES "\\.rxbin$" AND EXISTS "${_candidate}.rxbin")
        set(${out_var} "${_candidate}.rxbin" PARENT_SCOPE)
    else()
        set(${out_var} "" PARENT_SCOPE)
    endif()
endfunction()

function(_crexx_emit_stdout text path)
    if("${text}" STREQUAL "")
        return()
    endif()
    file(WRITE "${path}" "${text}")
    execute_process(COMMAND "${CMAKE_COMMAND}" -E cat "${path}")
endfunction()

function(_crexx_resolve_runner_path raw workdir out_var)
    if(IS_ABSOLUTE "${raw}")
        set(${out_var} "${raw}" PARENT_SCOPE)
    elseif(EXISTS "${workdir}/${raw}")
        set(${out_var} "${workdir}/${raw}" PARENT_SCOPE)
    elseif(WIN32 AND EXISTS "${workdir}/${raw}.exe")
        set(${out_var} "${workdir}/${raw}.exe" PARENT_SCOPE)
    else()
        set(${out_var} "${raw}" PARENT_SCOPE)
    endif()
endfunction()

function(crexx_link_and_run)
    set(options ALLOW_RUN_FAILURE)
    set(oneValueArgs NAME WORKING_DIRECTORY RUNNER RXLINK PROGRAM
            EXPECTED_EXIT_CODE EXPECTED_FAILURE_DESCRIPTION)
    set(multiValueArgs EXTRA_ARGS REQUIRED_OUTPUT_REGEX FORBIDDEN_OUTPUT_REGEX)
    cmake_parse_arguments(CREXX "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT CREXX_NAME OR NOT CREXX_WORKING_DIRECTORY OR NOT CREXX_RUNNER OR NOT CREXX_RXLINK OR NOT CREXX_PROGRAM)
        message(FATAL_ERROR "crexx_link_and_run requires NAME, WORKING_DIRECTORY, RUNNER, RXLINK, and PROGRAM")
    endif()
    if(CREXX_ALLOW_RUN_FAILURE AND NOT "${CREXX_EXPECTED_EXIT_CODE}" STREQUAL "")
        message(FATAL_ERROR
                "${CREXX_NAME}: ALLOW_RUN_FAILURE and EXPECTED_EXIT_CODE are mutually exclusive")
    endif()

    _crexx_resolve_module_path("${CREXX_PROGRAM}" "${CREXX_WORKING_DIRECTORY}" _program_path)
    if("${_program_path}" STREQUAL "")
        message(FATAL_ERROR "Unable to resolve primary program module: ${CREXX_PROGRAM}")
    endif()
    _crexx_resolve_runner_path("${CREXX_RUNNER}" "${CREXX_WORKING_DIRECTORY}" _runner_path)

    set(_modules "${_program_path}")
    set(_runtime_args)
    set(_all_remaining_runtime OFF)

    foreach(_arg IN LISTS CREXX_EXTRA_ARGS)
        if(_all_remaining_runtime)
            list(APPEND _runtime_args "${_arg}")
            continue()
        endif()

        if("${_arg}" MATCHES "^-")
            list(APPEND _runtime_args "${_arg}")
            if("${_arg}" STREQUAL "-a" OR "${_arg}" MATCHES "^-a.+")
                set(_all_remaining_runtime ON)
            endif()
            continue()
        endif()

        _crexx_resolve_module_path("${_arg}" "${CREXX_WORKING_DIRECTORY}" _resolved)
        if("${_resolved}" STREQUAL "")
            list(APPEND _runtime_args "${_arg}")
        else()
            list(APPEND _modules "${_resolved}")
        endif()
    endforeach()

    string(REGEX REPLACE "[^A-Za-z0-9_.-]" "_" _safe_name "${CREXX_NAME}")
    set(_temp_dir "${CREXX_WORKING_DIRECTORY}/linked_opt/${_safe_name}")
    file(REMOVE_RECURSE "${_temp_dir}")
    file(MAKE_DIRECTORY "${_temp_dir}")

    set(_linked_base "${_temp_dir}/${_safe_name}")

    execute_process(
            COMMAND "${CREXX_RXLINK}" -s -o "${_linked_base}" ${_modules}
            WORKING_DIRECTORY "${_temp_dir}"
            OUTPUT_VARIABLE _link_out
            ERROR_VARIABLE _link_err
            RESULT_VARIABLE _link_res
    )
    if(NOT _link_res EQUAL 0)
        _crexx_emit_stdout("${_link_out}${_link_err}" "${_temp_dir}/rxlink.log")
        message(FATAL_ERROR "rxlink failed for ${CREXX_NAME} with exit code ${_link_res}")
    endif()

    execute_process(
            COMMAND "${_runner_path}" "${_linked_base}.rxbin" ${_runtime_args}
            WORKING_DIRECTORY "${CREXX_WORKING_DIRECTORY}"
            OUTPUT_VARIABLE _run_out
            ERROR_VARIABLE _run_err
            RESULT_VARIABLE _run_res
    )

    _crexx_emit_stdout("${_run_out}${_run_err}" "${_temp_dir}/run.log")

    if(NOT "${CREXX_EXPECTED_EXIT_CODE}" STREQUAL "")
        crexx_assert_expected_failure(
                NAME "${CREXX_NAME}"
                RESULT "${_run_res}"
                EXPECTED_EXIT_CODE "${CREXX_EXPECTED_EXIT_CODE}"
                OUTPUT "${_run_out}${_run_err}"
                DESCRIPTION "${CREXX_EXPECTED_FAILURE_DESCRIPTION}"
                REQUIRED_OUTPUT_REGEX ${CREXX_REQUIRED_OUTPUT_REGEX}
                FORBIDDEN_OUTPUT_REGEX ${CREXX_FORBIDDEN_OUTPUT_REGEX}
        )
    elseif(NOT CREXX_ALLOW_RUN_FAILURE AND NOT _run_res EQUAL 0)
        message(FATAL_ERROR "runner failed for ${CREXX_NAME} with exit code ${_run_res}")
    endif()

    file(REMOVE_RECURSE "${_temp_dir}")
endfunction()

function(crexx_run_checked)
    set(options)
    set(oneValueArgs NAME WORKING_DIRECTORY RUNNER PROGRAM
            EXPECTED_EXIT_CODE EXPECTED_FAILURE_DESCRIPTION)
    set(multiValueArgs EXTRA_ARGS REQUIRED_OUTPUT_REGEX FORBIDDEN_OUTPUT_REGEX)
    cmake_parse_arguments(CREXX "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT CREXX_NAME OR NOT CREXX_WORKING_DIRECTORY OR NOT CREXX_RUNNER OR NOT CREXX_PROGRAM)
        message(FATAL_ERROR
                "crexx_run_checked requires NAME, WORKING_DIRECTORY, RUNNER, and PROGRAM")
    endif()

    _crexx_resolve_module_path("${CREXX_PROGRAM}" "${CREXX_WORKING_DIRECTORY}" _program_path)
    if("${_program_path}" STREQUAL "")
        message(FATAL_ERROR "Unable to resolve primary program module: ${CREXX_PROGRAM}")
    endif()
    _crexx_resolve_runner_path("${CREXX_RUNNER}" "${CREXX_WORKING_DIRECTORY}" _runner_path)

    string(REGEX REPLACE "[^A-Za-z0-9_.-]" "_" _safe_name "${CREXX_NAME}")
    set(_temp_dir "${CREXX_WORKING_DIRECTORY}/checked_runtime/${_safe_name}")
    file(REMOVE_RECURSE "${_temp_dir}")
    file(MAKE_DIRECTORY "${_temp_dir}")

    execute_process(
            COMMAND "${_runner_path}" "${_program_path}" ${CREXX_EXTRA_ARGS}
            WORKING_DIRECTORY "${CREXX_WORKING_DIRECTORY}"
            OUTPUT_VARIABLE _run_out
            ERROR_VARIABLE _run_err
            RESULT_VARIABLE _run_res
    )
    _crexx_emit_stdout("${_run_out}${_run_err}" "${_temp_dir}/run.log")

    if(NOT "${CREXX_EXPECTED_EXIT_CODE}" STREQUAL "")
        crexx_assert_expected_failure(
                NAME "${CREXX_NAME}"
                RESULT "${_run_res}"
                EXPECTED_EXIT_CODE "${CREXX_EXPECTED_EXIT_CODE}"
                OUTPUT "${_run_out}${_run_err}"
                DESCRIPTION "${CREXX_EXPECTED_FAILURE_DESCRIPTION}"
                REQUIRED_OUTPUT_REGEX ${CREXX_REQUIRED_OUTPUT_REGEX}
                FORBIDDEN_OUTPUT_REGEX ${CREXX_FORBIDDEN_OUTPUT_REGEX}
        )
    elseif(NOT _run_res EQUAL 0)
        message(FATAL_ERROR "runner failed for ${CREXX_NAME} with exit code ${_run_res}")
    endif()

    file(REMOVE_RECURSE "${_temp_dir}")
endfunction()
