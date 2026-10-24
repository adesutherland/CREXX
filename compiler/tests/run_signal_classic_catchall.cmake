cmake_minimum_required(VERSION 3.24)

foreach(required IN ITEMS MODE RXC RXAS RXLINK RXBVM LIBRARY SOURCE EXPECTED WORK)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
        message(FATAL_ERROR "Missing required variable: ${required}")
    endif()
endforeach()
if(NOT MODE STREQUAL "opt" AND NOT MODE STREQUAL "noopt")
    message(FATAL_ERROR "Unknown Classic catch-all compiler mode: ${MODE}")
endif()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}/imports")
get_filename_component(source_name "${SOURCE}" NAME)
file(COPY "${SOURCE}" DESTINATION "${WORK}")
file(COPY_FILE "${LIBRARY}" "${WORK}/imports/library.rxbin")

set(compiler_flags)
if(MODE STREQUAL "noopt")
    list(APPEND compiler_flags -n)
endif()
execute_process(
        COMMAND "${RXC}" ${compiler_flags} -x --no-exe-import
                -i "${WORK}/imports" -o program "${source_name}"
        WORKING_DIRECTORY "${WORK}"
        OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE status TIMEOUT 60)
if(NOT status EQUAL 0 OR NOT out STREQUAL "" OR NOT err STREQUAL "")
    message(FATAL_ERROR "Classic catch-all ${MODE} compilation failed (${status}):\n${out}${err}")
endif()
execute_process(
        COMMAND "${RXAS}" -o program.rxbin program.rxas
        WORKING_DIRECTORY "${WORK}"
        OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE status TIMEOUT 60)
if(NOT status EQUAL 0 OR NOT out STREQUAL "" OR NOT err STREQUAL "")
    message(FATAL_ERROR "Classic catch-all ${MODE} assembly failed (${status}):\n${out}${err}")
endif()

# Source metadata is part of this fixture's unconditional origin oracle.
# The general linked runtime helper deliberately strips it with rxlink -s.
execute_process(
        COMMAND "${RXLINK}" -o linked program.rxbin imports/library.rxbin
        WORKING_DIRECTORY "${WORK}"
        OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE status TIMEOUT 60)
if(NOT status EQUAL 0 OR NOT out STREQUAL "" OR NOT err STREQUAL "")
    message(FATAL_ERROR "Classic catch-all ${MODE} retained-source linking failed (${status}):\n${out}${err}")
endif()

file(READ "${EXPECTED}" expected)
string(REPLACE "\r\n" "\n" expected "${expected}")
set(runners "${RXBVM}")
if(DEFINED RXTVM AND NOT RXTVM STREQUAL "")
    list(APPEND runners "${RXTVM}")
endif()
list(REMOVE_DUPLICATES runners)
foreach(runner IN LISTS runners)
    execute_process(
            COMMAND "${runner}" "${WORK}/linked.rxbin"
            WORKING_DIRECTORY "${WORK}"
            OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE status TIMEOUT 60)
    string(REPLACE "\r\n" "\n" out "${out}")
    if(NOT status EQUAL 0 OR NOT out STREQUAL expected OR NOT err STREQUAL "")
        message(FATAL_ERROR "Classic catch-all ${MODE} failed on ${runner} (${status}):\nExpected:\n${expected}\nOutput:\n${out}\nDiagnostics:\n${err}")
    endif()
endforeach()
