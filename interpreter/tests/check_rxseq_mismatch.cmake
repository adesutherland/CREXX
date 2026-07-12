if(NOT DEFINED VM OR NOT DEFINED RXSEQ OR NOT DEFINED INPUT OR NOT DEFINED WRONG_INPUT)
    message(FATAL_ERROR "VM, RXSEQ, INPUT, and WRONG_INPUT are required")
endif()

set(PROFILE "${CMAKE_CURRENT_BINARY_DIR}/sequence-mismatch.rxseq")
file(REMOVE "${PROFILE}")
execute_process(
        COMMAND "${VM}" --sequence-count=2 "--sequence-output=${PROFILE}" "${INPUT}"
        RESULT_VARIABLE VM_RC
        ERROR_VARIABLE VM_STDERR)
if(NOT VM_RC EQUAL 0)
    message(FATAL_ERROR "sequence VM failed (${VM_RC}): ${VM_STDERR}")
endif()

execute_process(
        COMMAND "${RXSEQ}" "${PROFILE}" "${WRONG_INPUT}"
        RESULT_VARIABLE RXSEQ_RC
        OUTPUT_VARIABLE RXSEQ_STDOUT
        ERROR_VARIABLE RXSEQ_STDERR)
if(RXSEQ_RC EQUAL 0)
    message(FATAL_ERROR "rxseq accepted the wrong module")
endif()
if(NOT RXSEQ_STDERR MATCHES "profiled module is missing|module content hash mismatch")
    message(FATAL_ERROR "unexpected mismatch diagnostic: ${RXSEQ_STDERR}")
endif()

set(LEGACY_TEXT "${CMAKE_CURRENT_BINARY_DIR}/sequence-invalid-text.rxseq")
file(WRITE "${LEGACY_TEXT}" "RXSEQ\t1\nlength\t2\n")
execute_process(
        COMMAND "${RXSEQ}" "${LEGACY_TEXT}" "${INPUT}"
        RESULT_VARIABLE TEXT_RC
        ERROR_VARIABLE TEXT_STDERR)
if(TEXT_RC EQUAL 0 OR NOT TEXT_STDERR MATCHES "invalid RXSEQ binary magic")
    message(FATAL_ERROR "rxseq accepted or misdiagnosed text input: ${TEXT_STDERR}")
endif()
