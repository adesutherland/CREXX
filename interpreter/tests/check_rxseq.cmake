if(NOT DEFINED VM OR NOT DEFINED RXSEQ OR NOT DEFINED INPUT OR NOT DEFINED LENGTH)
    message(FATAL_ERROR "VM, RXSEQ, INPUT, and LENGTH are required")
endif()

set(PROFILE "${CMAKE_CURRENT_BINARY_DIR}/sequence-${LENGTH}.rxseq")
set(REPORT "${CMAKE_CURRENT_BINARY_DIR}/sequence-${LENGTH}.csv")
file(REMOVE "${PROFILE}" "${REPORT}")

execute_process(
        COMMAND "${VM}" "--sequence-count=${LENGTH}" "--sequence-output=${PROFILE}" "${INPUT}"
        RESULT_VARIABLE VM_RC
        OUTPUT_VARIABLE VM_STDOUT
        ERROR_VARIABLE VM_STDERR)
if(NOT VM_RC EQUAL 0)
    message(FATAL_ERROR "sequence VM failed (${VM_RC}): ${VM_STDERR}")
endif()
if(NOT EXISTS "${PROFILE}")
    message(FATAL_ERROR "sequence VM did not create ${PROFILE}")
endif()
file(READ "${PROFILE}" MAGIC LIMIT 8 HEX)
string(TOLOWER "${MAGIC}" MAGIC)
if(NOT MAGIC STREQUAL "525853455142494e")
    message(FATAL_ERROR "RXSEQ output does not have the binary RXSEQBIN magic: ${MAGIC}")
endif()

execute_process(
        COMMAND "${RXSEQ}" "${PROFILE}" "${INPUT}" "--output=${REPORT}"
        RESULT_VARIABLE RXSEQ_RC
        OUTPUT_VARIABLE RXSEQ_STDOUT
        ERROR_VARIABLE RXSEQ_STDERR)
if(NOT RXSEQ_RC EQUAL 0)
    message(FATAL_ERROR "rxseq failed (${RXSEQ_RC}): ${RXSEQ_STDERR}")
endif()
file(READ "${REPORT}" CONTENT)
if(LENGTH EQUAL 2)
    set(EXPECTED "6,2,1,3,candidate,\"IADD_REG_REG_INT\\(r1,r1,c1\\) \\| COPY_REG_REG\\(r2,r1\\)\"")
elseif(LENGTH EQUAL 3)
    set(EXPECTED "5,1,1,4,over_3_symbols,\"IADD_REG_REG_INT\\(r1,r1,c1\\) \\| COPY_REG_REG\\(r2,r1\\) \\| COPY_REG_REG\\(r3,r2\\)\"")
else()
    set(EXPECTED "5,1,1,6,over_3_symbols,\"IADD_REG_REG_INT\\(r1,r1,c1\\) \\| COPY_REG_REG\\(r2,r1\\) \\| COPY_REG_REG\\(r3,r2\\) \\| BLT_ID_REG_REG\\(c2,r1,r4\\)\"")
endif()
if(NOT CONTENT MATCHES "${EXPECTED}")
    message(FATAL_ERROR "expected dynamic count/mapping was not found in:\n${CONTENT}")
endif()
