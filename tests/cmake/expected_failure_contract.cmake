if(NOT ASSERTIONS_MODULE OR NOT WORKING_DIRECTORY)
    message(FATAL_ERROR
            "expected_failure_contract requires ASSERTIONS_MODULE and WORKING_DIRECTORY")
endif()

include("${ASSERTIONS_MODULE}")
file(MAKE_DIRECTORY "${WORKING_DIRECTORY}")

set(_failing_child "${WORKING_DIRECTORY}/expected_failure_child.cmake")
file(WRITE "${_failing_child}"
        "message(STATUS \"child emitted required marker\")\n"
        "message(FATAL_ERROR \"deliberate child failure\")\n")
execute_process(
        COMMAND "${CMAKE_COMMAND}" -P "${_failing_child}"
        RESULT_VARIABLE _child_result
        OUTPUT_VARIABLE _child_out
        ERROR_VARIABLE _child_err
)
set(_child_output "${_child_out}${_child_err}")
crexx_assert_expected_failure(
        NAME expected_failure_contract
        RESULT "${_child_result}"
        EXPECTED_EXIT_CODE 1
        OUTPUT "${_child_output}"
        DESCRIPTION "checked child failure"
        REQUIRED_OUTPUT_REGEX "child emitted required marker" "deliberate child failure"
)

set(_reject_success "${WORKING_DIRECTORY}/expected_failure_reject_success.cmake")
file(WRITE "${_reject_success}"
        "include([=[${ASSERTIONS_MODULE}]=])\n"
        "crexx_assert_expected_failure(\n"
        "  NAME reject_unexpected_success\n"
        "  RESULT 0\n"
        "  EXPECTED_EXIT_CODE 1)\n")
execute_process(
        COMMAND "${CMAKE_COMMAND}" -P "${_reject_success}"
        RESULT_VARIABLE _reject_result
        OUTPUT_VARIABLE _reject_out
        ERROR_VARIABLE _reject_err
)
set(_reject_output "${_reject_out}${_reject_err}")
if("${_reject_result}" STREQUAL "0")
    message(FATAL_ERROR "expected-failure assertion accepted an unexpected success")
endif()
if(NOT "${_reject_output}" MATCHES "expected exit code 1, got 0")
    message(FATAL_ERROR "unexpected-success rejection did not explain the exit mismatch")
endif()

message(STATUS "PASS: expected-failure assertion rejects false positives")
