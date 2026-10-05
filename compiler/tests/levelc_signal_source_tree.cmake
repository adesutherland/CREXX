foreach(required RXC BINDIR SOURCE BUILD_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()

set(log "${BUILD_DIR}/levelc_signal_source_tree.log")
execute_process(
        COMMAND "${RXC}" -d1 -i "${BINDIR}" -o "levelc_signal_source_tree" "${SOURCE}"
        WORKING_DIRECTORY "${BUILD_DIR}"
        OUTPUT_FILE "${log}"
        ERROR_FILE "${log}"
        RESULT_VARIABLE result)
file(READ "${log}" output)
if(NOT output MATCHES "--- STAGE_RAW ---")
    message(FATAL_ERROR "SIGNAL raw tree is missing (rxc ${result}); see ${log}")
endif()

string(REGEX MATCHALL "LEVELC_SIGNAL_VALUE : \"value\"" value_nodes "${output}")
list(LENGTH value_nodes value_count)
if(NOT value_count EQUAL 2 OR
   NOT output MATCHES "LEVELC_SIGNAL_VALUE : \"\"[^\n]*\n[ ]+STRING : \"target\"" OR
   NOT output MATCHES "LEVELC_SIGNAL : \"signal\"[^\n]*\n[ ]+LITERAL : \"target\"" OR
   NOT output MATCHES "LEVELC_SIGNAL : \"signal\"[^\n]*\n[ ]+STRING : \"target\"" OR
   NOT output MATCHES "LEVELC_SIGNAL_VALUE : \"value\"[^\n]*\n[ ]+STRING : \"target\"" OR
   NOT output MATCHES "LEVELC_SIGNAL_VALUE : \"value\"[^\n]*\n[ ]+VAR_SYMBOL : \"target\"" OR
   NOT output MATCHES "LITERAL : \"on\"[^\n]*\n[ ]+LITERAL : \"syntax\"" OR
   NOT output MATCHES "LITERAL : \"off\"[^\n]*\n[ ]+LITERAL : \"syntax\"")
    message(FATAL_ERROR "SIGNAL direct, VALUE, ON or OFF source shape is wrong; see ${log}")
endif()
