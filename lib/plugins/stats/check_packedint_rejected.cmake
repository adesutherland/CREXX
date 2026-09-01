if(NOT DEFINED RXC OR NOT DEFINED SOURCE OR NOT DEFINED IMPORT_PATH OR
   NOT DEFINED OUTPUT_BASE)
    message(FATAL_ERROR
            "RXC, SOURCE, IMPORT_PATH, and OUTPUT_BASE are required")
endif()

file(REMOVE "${OUTPUT_BASE}.rxas")
execute_process(
        COMMAND "${RXC}" -i "${IMPORT_PATH}" -o "${OUTPUT_BASE}" "${SOURCE}"
        RESULT_VARIABLE compile_result
        OUTPUT_VARIABLE compile_stdout
        ERROR_VARIABLE compile_stderr)

if(compile_result EQUAL 0)
    message(FATAL_ERROR
            "rxstats accepted .packedint where .packedfloat is required")
endif()

set(compile_output "${compile_stdout}${compile_stderr}")
if(NOT compile_output MATCHES "#TYPE_MISMATCH")
    message(FATAL_ERROR
            "rxstats .packedint rejection lacked TYPE_MISMATCH:\n${compile_output}")
endif()

message(STATUS "PASS: rxstats rejects .packedint at compile time")
