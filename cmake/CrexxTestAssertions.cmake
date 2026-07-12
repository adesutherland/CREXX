include(CMakeParseArguments)

function(crexx_assert_expected_failure)
    set(options)
    set(oneValueArgs NAME RESULT EXPECTED_EXIT_CODE OUTPUT DESCRIPTION)
    set(multiValueArgs REQUIRED_OUTPUT_REGEX FORBIDDEN_OUTPUT_REGEX)
    cmake_parse_arguments(CREXX "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT CREXX_NAME)
        message(FATAL_ERROR "crexx_assert_expected_failure requires NAME")
    endif()
    if("${CREXX_EXPECTED_EXIT_CODE}" STREQUAL "")
        message(FATAL_ERROR "crexx_assert_expected_failure requires EXPECTED_EXIT_CODE")
    endif()
    if("${CREXX_EXPECTED_EXIT_CODE}" STREQUAL "0")
        message(FATAL_ERROR "crexx_assert_expected_failure requires a nonzero EXPECTED_EXIT_CODE")
    endif()

    if(NOT "${CREXX_RESULT}" STREQUAL "${CREXX_EXPECTED_EXIT_CODE}")
        message(FATAL_ERROR
                "${CREXX_NAME}: expected exit code ${CREXX_EXPECTED_EXIT_CODE}, got ${CREXX_RESULT}")
    endif()

    foreach(_required_regex IN LISTS CREXX_REQUIRED_OUTPUT_REGEX)
        if(NOT "${CREXX_OUTPUT}" MATCHES "${_required_regex}")
            message(FATAL_ERROR
                    "${CREXX_NAME}: expected output to match: ${_required_regex}")
        endif()
    endforeach()

    foreach(_forbidden_regex IN LISTS CREXX_FORBIDDEN_OUTPUT_REGEX)
        if("${CREXX_OUTPUT}" MATCHES "${_forbidden_regex}")
            message(FATAL_ERROR
                    "${CREXX_NAME}: output unexpectedly matched: ${_forbidden_regex}")
        endif()
    endforeach()

    if(CREXX_DESCRIPTION)
        set(_description "${CREXX_DESCRIPTION}")
    else()
        set(_description "${CREXX_NAME} failed as expected")
    endif()
    message(STATUS
            "EXPECTED FAILURE: ${_description}, exit code ${CREXX_RESULT}; test passed")
endfunction()
