if(NOT DEFINED RXPP_BIN)
    message(FATAL_ERROR "RXPP_BIN is required")
endif()
if(NOT DEFINED RXC_BIN)
    message(FATAL_ERROR "RXC_BIN is required")
endif()
if(NOT DEFINED INPUT_FILE)
    message(FATAL_ERROR "INPUT_FILE is required")
endif()
if(NOT DEFINED OUTPUT_FILE)
    message(FATAL_ERROR "OUTPUT_FILE is required")
endif()
if(NOT DEFINED MACLIB_FILE)
    message(FATAL_ERROR "MACLIB_FILE is required")
endif()

execute_process(
    COMMAND "${RXPP_BIN}" rxprecomp -I "${INPUT_FILE}" -o "${OUTPUT_FILE}" -m "${MACLIB_FILE}"
    RESULT_VARIABLE rxpp_res
    OUTPUT_VARIABLE rxpp_out
    ERROR_VARIABLE rxpp_err
)
if(NOT rxpp_res EQUAL 0)
    message(FATAL_ERROR "rxpp failed with ${rxpp_res}:\n${rxpp_out}${rxpp_err}")
endif()

file(READ "${OUTPUT_FILE}" output)
string(FIND "${output}" "__rexxscript_source = ''" init_pos)
string(FIND "${output}" "msg_1=111" body_pos)
string(FIND "${output}" "|| \";\"" separator_pos)
string(FIND "${output}" "REXXSCRIPT __rexxscript_source" call_pos)
string(FIND "${output}" "EXPOSE total, count" expose_pos)
string(FIND "${output}" "OUTPUT output" output_clause_pos)
string(FIND "${output}" "RESULT raw" result_clause_pos)
string(FIND "${output}" "VALUES __rexxscript_values" export_clause_pos)
string(FIND "${output}" "END REXXSCRIPT" marker_pos)
string(FIND "${output}" "say 'Captured output:'" after_pos)
if(init_pos EQUAL -1 OR body_pos EQUAL -1 OR separator_pos EQUAL -1 OR call_pos EQUAL -1 OR expose_pos EQUAL -1 OR output_clause_pos EQUAL -1 OR result_clause_pos EQUAL -1)
    message(FATAL_ERROR "REXXSCRIPT block was not collected correctly:\n${output}")
endif()
if(NOT marker_pos EQUAL -1)
    message(FATAL_ERROR "REXXSCRIPT block marker leaked into generated source:\n${output}")
endif()
if(NOT export_clause_pos EQUAL -1)
    message(FATAL_ERROR "REXXSCRIPT block unexpectedly emitted a COLLECT export bridge:\n${output}")
endif()
if(call_pos LESS body_pos OR after_pos LESS call_pos)
    message(FATAL_ERROR "REXXSCRIPT block was emitted in the wrong order:\n${output}")
endif()

set(output_base "${OUTPUT_FILE}.compiled")
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env RXCP_EXIT_MODULE=rxcexits
            "${RXC_BIN}" -o "${output_base}" "${OUTPUT_FILE}"
    RESULT_VARIABLE rxc_res
    OUTPUT_VARIABLE rxc_out
    ERROR_VARIABLE rxc_err
)
if(NOT rxc_res EQUAL 0)
    message(FATAL_ERROR "rxc failed to compile REXXSCRIPT block output with ${rxc_res}:\n${rxc_out}${rxc_err}\nGenerated:\n${output}")
endif()
