if(NOT DEFINED CREXX_BIN OR NOT DEFINED WORK OR NOT DEFINED MODE)
    message(FATAL_ERROR "CREXX_BIN, WORK and MODE are required")
endif()
if(NOT MODE STREQUAL "opt" AND NOT MODE STREQUAL "noopt")
    message(FATAL_ERROR "MODE must be opt or noopt")
endif()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")
file(WRITE "${WORK}/alias.rxpp" [=[##CFLAG nosrcmap
##DALIAS config, routes
##config aliased
first
##END
##DATA direct
second
##END
say aliased.1
say direct.1
]=])

set(options)
if(MODE STREQUAL "noopt")
    list(APPEND options --nooptimize)
endif()
execute_process(
    COMMAND "${CREXX_BIN}" ${options} alias.rxpp
    WORKING_DIRECTORY "${WORK}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE errors
    TIMEOUT 120)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "RXPP alias ${MODE} failed: ${result}\n${output}\n${errors}")
endif()
string(FIND "${output}${errors}" "12345" marker)
if(NOT marker EQUAL -1)
    message(FATAL_ERROR "RXPP emitted its internal alias marker: ${output}${errors}")
endif()
string(FIND "${output}" "first\nsecond\n" expected)
if(expected EQUAL -1)
    message(FATAL_ERROR "Alias and direct DATA values differ: ${output}")
endif()
file(READ "${WORK}/alias.crexx" generated)
foreach(assignment IN ITEMS "aliased.1='first'" "direct.1='second'")
    string(FIND "${generated}" "${assignment}" found)
    if(found EQUAL -1)
        message(FATAL_ERROR "Missing generated assignment ${assignment}: ${generated}")
    endif()
endforeach()

foreach(invalid_case IN ITEMS reserved malformed duplicate)
    if(invalid_case STREQUAL "reserved")
        set(invalid_source "##DALIAS IF\nsay 'SHOULD_NOT_RUN'\n")
    elseif(invalid_case STREQUAL "malformed")
        set(invalid_source "##DALIAS 1FORTH\nsay 'SHOULD_NOT_RUN'\n")
    else()
        set(invalid_source "##DALIAS CONFIG, config\nsay 'SHOULD_NOT_RUN'\n")
    endif()
    file(WRITE "${WORK}/${invalid_case}.rxpp" "${invalid_source}")
    execute_process(
        COMMAND "${CREXX_BIN}" ${options} "${invalid_case}.rxpp"
        WORKING_DIRECTORY "${WORK}"
        RESULT_VARIABLE invalid_result
        OUTPUT_VARIABLE invalid_output
        ERROR_VARIABLE invalid_errors
        TIMEOUT 120)
    if(invalid_result EQUAL 0)
        message(FATAL_ERROR "Invalid DALIAS ${invalid_case} unexpectedly succeeded")
    endif()
    string(FIND "${invalid_output}${invalid_errors}" "DALIAS" invalid_marker)
    if(invalid_marker EQUAL -1)
        message(FATAL_ERROR "Invalid DALIAS ${invalid_case} produced no useful diagnostic: ${invalid_output}${invalid_errors}")
    endif()
endforeach()
