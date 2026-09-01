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

if(DEFINED MAX_OUTPUT_BYTES AND NOT "${MAX_OUTPUT_BYTES}" STREQUAL "")
    file(SIZE "${OUTPUT_RXAS}" _output_size)
    if(_output_size GREATER MAX_OUTPUT_BYTES)
        message(FATAL_ERROR
                "rxdas output is unexpectedly large: ${_output_size} bytes "
                "(maximum ${MAX_OUTPUT_BYTES})")
    endif()
endif()

file(READ "${OUTPUT_RXAS}" _text)

foreach(_text_index RANGE 1 8)
    set(_text_var "EXPECT_TEXT_${_text_index}")
    if(DEFINED ${_text_var} AND NOT "${${_text_var}}" STREQUAL "")
        string(FIND "${_text}" "${${_text_var}}" _found_index)
        if(_found_index EQUAL -1)
            message(FATAL_ERROR
                    "Expected text missing from ${OUTPUT_RXAS}\n"
                    "Expected ${_text_index}: ${${_text_var}}")
        endif()
    endif()
endforeach()

foreach(_absent_index RANGE 1 8)
    set(_absent_var "EXPECT_ABSENT_${_absent_index}")
    if(DEFINED ${_absent_var} AND NOT "${${_absent_var}}" STREQUAL "")
        string(FIND "${_text}" "${${_absent_var}}" _found_index)
        if(NOT _found_index EQUAL -1)
            message(FATAL_ERROR
                    "Unexpected text present in ${OUTPUT_RXAS}\n"
                    "Unexpected: ${${_absent_var}}")
        endif()
    endif()
endforeach()
