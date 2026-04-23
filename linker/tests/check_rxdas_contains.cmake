cmake_minimum_required(VERSION 3.24)

foreach(_required_var IN ITEMS RXDAS WORKING_DIRECTORY INPUT_RXBIN OUTPUT_RXAS EXPECT_TEXT_1 EXPECT_TEXT_2)
    if(NOT DEFINED ${_required_var} OR "${${_required_var}}" STREQUAL "")
        message(FATAL_ERROR "Missing required variable ${_required_var}")
    endif()
endforeach()

execute_process(
        COMMAND "${RXDAS}" -o "${OUTPUT_RXAS}" "${INPUT_RXBIN}"
        WORKING_DIRECTORY "${WORKING_DIRECTORY}"
        RESULT_VARIABLE _rc
        OUTPUT_VARIABLE _out
        ERROR_VARIABLE _err
)

if(NOT _rc EQUAL 0)
    message(FATAL_ERROR
            "rxdas failed\n"
            "stdout:\n${_out}\n"
            "stderr:\n${_err}")
endif()

file(READ "${OUTPUT_RXAS}" _text)
string(FIND "${_text}" "${EXPECT_TEXT_1}" _index_1)
string(FIND "${_text}" "${EXPECT_TEXT_2}" _index_2)

if(_index_1 EQUAL -1 OR _index_2 EQUAL -1)
    message(FATAL_ERROR
            "Expected text missing from ${OUTPUT_RXAS}\n"
            "Expected 1: ${EXPECT_TEXT_1}\n"
            "Expected 2: ${EXPECT_TEXT_2}")
endif()
