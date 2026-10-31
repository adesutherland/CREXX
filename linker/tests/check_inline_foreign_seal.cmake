file(REMOVE "${WORK}.rxbin" "${WORK}-linked.rxbin")

execute_process(
    COMMAND "${RXAS}" -n -o "${WORK}" "${SOURCE}"
    RESULT_VARIABLE assemble_result
    OUTPUT_VARIABLE assemble_stdout
    ERROR_VARIABLE assemble_stderr)
if(NOT assemble_result EQUAL 0)
    message(FATAL_ERROR "foreign selector fixture did not assemble: ${assemble_stdout}${assemble_stderr}")
endif()

execute_process(
    COMMAND "${RXLINK}" -r "${SOURCE}" -o "${WORK}-linked" "${WORK}"
    RESULT_VARIABLE link_result
    OUTPUT_VARIABLE link_stdout
    ERROR_VARIABLE link_stderr)
set(wrote_output FALSE)
if(EXISTS "${WORK}-linked.rxbin")
    set(wrote_output TRUE)
endif()
file(REMOVE "${WORK}.rxbin" "${WORK}-linked.rxbin")
if(link_result EQUAL 0 OR wrote_output OR
   NOT "${link_stdout}${link_stderr}" MATCHES "unsealed or mismatched inline operand")
    message(FATAL_ERROR "foreign inline seal was not rejected cleanly: result=${link_result} output=${wrote_output} ${link_stdout}${link_stderr}")
endif()
