execute_process(
        COMMAND "${RXC}" -i "${IMPORT_PATH}" -o "${OUTPUT_BASE}" "${SOURCE}"
        RESULT_VARIABLE compile_result
        OUTPUT_VARIABLE compile_output
        ERROR_VARIABLE compile_error
        TIMEOUT 20)

if(NOT compile_result EQUAL 0)
    message(FATAL_ERROR
            "RXPA native return-class import did not complete (${compile_result}):\n"
            "${compile_output}${compile_error}")
endif()

message(STATUS "PASS: RXPA native return-class metadata import completed")
