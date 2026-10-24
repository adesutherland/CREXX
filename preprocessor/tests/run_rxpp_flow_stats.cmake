cmake_minimum_required(VERSION 3.24)

file(MAKE_DIRECTORY "${WORK}")
set(output_file "${WORK}/rxpp_flow_stats.out.crexx")

execute_process(
    COMMAND "${RXPP_BIN}" -I "${INPUT_FILE}" -O "${output_file}" -M "${MACLIB_FILE}"
    WORKING_DIRECTORY "${WORK}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE stdout_text
    ERROR_VARIABLE stderr_text
    ENCODING UTF-8)

set(full_output "${stdout_text}${stderr_text}")
if(NOT result EQUAL 0)
    message(FATAL_ERROR "flow/stats preprocessing failed with ${result}:\n${full_output}")
endif()

foreach(expected IN ITEMS
        "RXPP-FLOW phase=pass1 action=end"
        "RXPP-FLOW phase=pass2 action=cflag"
        "RXPP-FLOW phase=pass3 action=macro"
        "RXPP-FLOW phase=pass3 action=conditional_keep"
        "RXPP-STATS input_lines="
        "RXPP-STATS directives=")
    string(FIND "${full_output}" "${expected}" found)
    if(found EQUAL -1)
        message(FATAL_ERROR "missing '${expected}' in flow/stats output:\n${full_output}")
    endif()
endforeach()

file(READ "${output_file}" generated)
foreach(unexpected IN ITEMS "RXPP-FLOW" "RXPP-STATS")
    string(FIND "${generated}" "${unexpected}" found)
    if(NOT found EQUAL -1)
        message(FATAL_ERROR "${unexpected} leaked into generated CREXX:\n${generated}")
    endif()
endforeach()

string(FIND "${generated}" "say 2+2" generated_ok)
if(generated_ok EQUAL -1)
    message(FATAL_ERROR "macro expansion was not preserved:\n${generated}")
endif()
