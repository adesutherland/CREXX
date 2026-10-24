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
string(FIND "${output}" "records.1='first record'" first_pos)
string(FIND "${output}" "records.2='second record'" second_pos)
string(FIND "${output}" "call capture_data records" callback_pos)
string(FIND "${output}" "legacy.1='legacy record'" legacy_pos)
string(FIND "${output}" "call capture_data legacy" legacy_callback_pos)
string(FIND "${output}" "relation_source.1='relation line'" relation_pos)
string(FIND "${output}" "call capture_data relation_source" relation_callback_pos)
string(FIND "${output}" "program_source.1='program line'" program_pos)
string(FIND "${output}" "library_source.1='library line'" library_pos)
string(FIND "${output}" "call capture_data library_source" library_callback_pos)
string(FIND "${output}" "stream_source.1='stream line one'" stream_pos)
string(FIND "${output}" "stream_source.2='stream line two'" stream_second_pos)
string(FIND "${output}" "call capture_data stream_source" stream_callback_pos)
if(first_pos EQUAL -1 OR second_pos EQUAL -1 OR callback_pos EQUAL -1
        OR relation_pos EQUAL -1 OR relation_callback_pos EQUAL -1
        OR program_pos EQUAL -1 OR library_pos EQUAL -1 OR library_callback_pos EQUAL -1
        OR stream_pos EQUAL -1 OR stream_second_pos EQUAL -1 OR stream_callback_pos EQUAL -1)
    message(FATAL_ERROR "RXPP did not emit the data callback form correctly:\n${output}")
endif()
if(callback_pos LESS second_pos)
    message(FATAL_ERROR "RXPP emitted the data callback before the complete array:\n${output}")
endif()
if(legacy_pos EQUAL -1 OR NOT legacy_callback_pos EQUAL -1)
    message(FATAL_ERROR "RXPP changed legacy ##DATA behavior:\n${output}")
endif()
if(library_callback_pos LESS library_pos)
    message(FATAL_ERROR "RXPP emitted the ##LIBRARY callback before the complete array:\n${output}")
endif()
if(relation_callback_pos LESS relation_pos)
    message(FATAL_ERROR "RXPP emitted the ##RELATION callback before the complete array:\n${output}")
endif()
if(stream_callback_pos LESS stream_second_pos)
    message(FATAL_ERROR "RXPP emitted the ##STREAM callback before the complete array:\n${output}")
endif()

set(output_base "${OUTPUT_FILE}.compiled")
execute_process(
    COMMAND "${RXC_BIN}" -o "${output_base}" "${OUTPUT_FILE}"
    RESULT_VARIABLE rxc_res
    OUTPUT_VARIABLE rxc_out
    ERROR_VARIABLE rxc_err
)
if(NOT rxc_res EQUAL 0)
    message(FATAL_ERROR "rxc failed to compile data callback output with ${rxc_res}:\n${rxc_out}${rxc_err}\nGenerated:\n${output}")
endif()
