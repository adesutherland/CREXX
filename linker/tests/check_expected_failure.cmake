cmake_minimum_required(VERSION 3.24)

foreach(_required_var IN ITEMS RXLINK WORKING_DIRECTORY OUTPUT_BASENAME INPUTS EXPECT_TEXT)
    if(NOT DEFINED ${_required_var} OR "${${_required_var}}" STREQUAL "")
        message(FATAL_ERROR "Missing required variable ${_required_var}")
    endif()
endforeach()

set(_args -o "${OUTPUT_BASENAME}")
if(DEFINED CONTROL_FILE AND NOT "${CONTROL_FILE}" STREQUAL "")
    list(APPEND _args -c "${CONTROL_FILE}")
endif()
foreach(_input IN LISTS INPUTS)
    list(APPEND _args "${_input}")
endforeach()

execute_process(
        COMMAND "${RXLINK}" ${_args}
        WORKING_DIRECTORY "${WORKING_DIRECTORY}"
        RESULT_VARIABLE _rc
        OUTPUT_VARIABLE _out
        ERROR_VARIABLE _err
)

if(_rc EQUAL 0)
    message(FATAL_ERROR
            "rxlink unexpectedly succeeded\n"
            "stdout:\n${_out}\n"
            "stderr:\n${_err}")
endif()

set(_combined "${_out}\n${_err}")
string(FIND "${_combined}" "${EXPECT_TEXT}" _index)
if(_index EQUAL -1)
    message(FATAL_ERROR
            "Expected failure text not found: ${EXPECT_TEXT}\n"
            "stdout:\n${_out}\n"
            "stderr:\n${_err}")
endif()
