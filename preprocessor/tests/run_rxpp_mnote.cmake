cmake_minimum_required(VERSION 3.24)

if(NOT DEFINED RXPP_BIN OR NOT DEFINED INPUT_FILE OR NOT DEFINED OUTPUT_FILE OR NOT DEFINED MACLIB_FILE)
    message(FATAL_ERROR "RXPP_BIN, INPUT_FILE, OUTPUT_FILE, and MACLIB_FILE are required")
endif()

execute_process(
    COMMAND "${RXPP_BIN}" -I "${INPUT_FILE}" -O "${OUTPUT_FILE}" -M "${MACLIB_FILE}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE errors
    ENCODING UTF-8)
set(full_output "${output}${errors}")
if(NOT result EQUAL 0)
    message(FATAL_ERROR "MNOTE preprocessing failed with ${result}:\n${full_output}")
endif()
file(READ "${OUTPUT_FILE}" generated)
if(NOT generated MATCHES "/\\* MNOTE ##OUTER: outer compatibility path \\*/")
    message(FATAL_ERROR "Outer MNOTE was not generated:\n${generated}")
endif()
if(NOT generated MATCHES "/\\* MNOTE ##INNER: inner compatibility path \\*/")
    message(FATAL_ERROR "Nested MNOTE was not generated:\n${generated}")
endif()
if(NOT generated MATCHES "say \"nested source\"")
    message(FATAL_ERROR "Nested generated source was lost:\n${generated}")
endif()
if(generated MATCHES "@\\.mnote@")
    message(FATAL_ERROR "Private MNOTE marker leaked into generated CREXX:\n${generated}")
endif()
