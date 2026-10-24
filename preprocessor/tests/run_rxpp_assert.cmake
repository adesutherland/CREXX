cmake_minimum_required(VERSION 3.24)

file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

file(WRITE "${WORK}/assert_ok.rxpp" [=[##CFLAG nosrcmap nset
##SET DEBUG 1
##ASSERT DEBUG, "DEBUG must be defined"
##ASSERT DEBUG == "1", "DEBUG has the wrong value"
##ASSERT DEBUG != "0", "DEBUG has the disabled value"
##ASSERT platform != "", "RXPP platform is unavailable"
##IF DEBUG == "1"
say "value condition passed"
##ELSE
say "value condition failed"
##END
##IFN DEBUG == "0"
say "inverted value condition passed"
##END
say "assertions passed"
]=])

execute_process(
    COMMAND "${RXPP_BIN}" rxprecomp -I "${WORK}/assert_ok.rxpp"
            -o "${WORK}/assert_ok.crexx" -m "${MACLIB_FILE}"
    RESULT_VARIABLE ok_result
    OUTPUT_VARIABLE ok_output
    ERROR_VARIABLE ok_errors
    TIMEOUT 120)
if(NOT ok_result EQUAL 0)
    message(FATAL_ERROR "successful assertions failed with ${ok_result}:\n${ok_output}${ok_errors}")
endif()
file(READ "${WORK}/assert_ok.crexx" generated)
if(generated MATCHES "##ASSERT" OR generated MATCHES "RXPP_ASSERT_FAILED")
    message(FATAL_ERROR "assertion diagnostics leaked into generated CREXX:\n${generated}")
endif()
if(NOT generated MATCHES "value condition passed" OR generated MATCHES "value condition failed")
    message(FATAL_ERROR "##IF value comparison was not evaluated correctly:\n${generated}")
endif()
if(NOT generated MATCHES "inverted value condition passed")
    message(FATAL_ERROR "##IFN value comparison was not evaluated correctly:\n${generated}")
endif()

file(WRITE "${WORK}/assert_fail.rxpp" [=[##CFLAG nosrcmap nset
##SET DEBUG 1
##UNSET DEBUG
##ASSERT DEBUG, "DEBUG should be absent after ##UNSET"
say "must not be generated"
]=])

execute_process(
    COMMAND "${RXPP_BIN}" rxprecomp -I "${WORK}/assert_fail.rxpp"
            -o "${WORK}/assert_fail.crexx" -m "${MACLIB_FILE}"
    RESULT_VARIABLE fail_result
    OUTPUT_VARIABLE fail_output
    ERROR_VARIABLE fail_errors
    TIMEOUT 120)
set(fail_text "${fail_output}${fail_errors}")
if(NOT fail_result EQUAL 8)
    message(FATAL_ERROR "failed assertion returned ${fail_result}, expected 8:\n${fail_text}")
endif()
if(NOT fail_text MATCHES "RXPP_ASSERT_FAILED")
    message(FATAL_ERROR "failed assertion did not emit RXPP_ASSERT_FAILED:\n${fail_text}")
endif()
if(NOT fail_text MATCHES "DEBUG should be absent")
    message(FATAL_ERROR "failed assertion did not preserve its message:\n${fail_text}")
endif()
