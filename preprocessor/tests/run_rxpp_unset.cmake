if(NOT DEFINED RXPP_BIN)
    message(FATAL_ERROR "RXPP_BIN is required")
endif()
if(NOT DEFINED WORK)
    message(FATAL_ERROR "WORK is required")
endif()
if(NOT DEFINED MACLIB_FILE)
    message(FATAL_ERROR "MACLIB_FILE is required")
endif()

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

file(WRITE "${WORK}/unset.rxpp" [=[##CFLAG nosrcmap nset
##SET DEBUG enabled
##SET PRINTGEN ON
say "before={debug}"
##UNSET DEBUG
##UNSET PRINTGEN
say "after={debug}"
##IF DEBUG
say "if-present"
##ELSE
say "if-absent"
##END
say "protected={printgen}"
]=])

execute_process(
    COMMAND "${RXPP_BIN}" rxprecomp
            -I "${WORK}/unset.rxpp"
            -o "${WORK}/unset.crexx"
            -m "${MACLIB_FILE}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE errors
    TIMEOUT 120)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "RXPP ##UNSET test failed with ${result}:\n${output}${errors}")
endif()

file(READ "${WORK}/unset.crexx" generated)
if(NOT generated MATCHES "say \"before=enabled\"")
    message(FATAL_ERROR "##SET value was not expanded before ##UNSET:\n${generated}")
endif()
if(NOT generated MATCHES "say \"after=\\{debug\\}\"")
    message(FATAL_ERROR "##UNSET changed the unresolved placeholder unexpectedly:\n${generated}")
endif()
if(NOT generated MATCHES "say \"if-absent\"")
    message(FATAL_ERROR "##UNSET left the user variable present for ##IF:\n${generated}")
endif()
if(generated MATCHES "if-present")
    message(FATAL_ERROR "##IF still saw the unset variable:\n${generated}")
endif()
if(NOT generated MATCHES "say \"protected=all\"")
    message(FATAL_ERROR "##UNSET changed protected PRINTGEN variable:\n${generated}")
endif()
if(generated MATCHES "##SET" OR generated MATCHES "##UNSET")
    message(FATAL_ERROR "nset did not suppress directive comments:\n${generated}")
endif()
