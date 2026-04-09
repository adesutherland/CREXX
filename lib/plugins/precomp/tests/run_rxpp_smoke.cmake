execute_process(
    COMMAND "${RXPP_BIN}" rxprecomp -I "${INPUT_FILE}" -o "${OUTPUT_FILE}" -m "${MACLIB_FILE}"
    RESULT_VARIABLE res
)
if(NOT res EQUAL 0)
    message(FATAL_ERROR "rxpp failed with ${res}")
endif()

file(READ "${OUTPUT_FILE}" out_content)
if(NOT out_content MATCHES "say \"Hello\"")
    message(FATAL_ERROR "Missing say \"Hello\" in output")
endif()
if(NOT out_content MATCHES "say 3\\*3")
    message(FATAL_ERROR "Missing say 3*3 in output")
endif()