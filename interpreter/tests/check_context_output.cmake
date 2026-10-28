execute_process(COMMAND "${HOST}" "${LIBRARY}" "${FIXTURE}"
    RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 80)
if(NOT status EQUAL 0)
    message(FATAL_ERROR "Context output host failed (${status}):\n${output}\n${error}")
endif()
if(NOT output STREQUAL "DEFAULT_OUT\n" OR NOT error STREQUAL "DEFAULT_ERR\n")
    message(FATAL_ERROR "Wrong native output after scoped capture/reset:\nstdout=${output}\nstderr=${error}")
endif()
