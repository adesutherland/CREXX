cmake_minimum_required(VERSION 3.20)

foreach(required IN ITEMS RXAS RXLINK RXDAS MUTATE_FEATURE SOURCE WORK_DIR)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
        message(FATAL_ERROR "Missing required -D${required}=...")
    endif()
endforeach()

file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")

function(run_checked description)
    execute_process(
            COMMAND ${ARGN}
            OUTPUT_VARIABLE out
            ERROR_VARIABLE err
            RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${description} failed:\n${out}${err}")
    endif()
endfunction()

function(assert_header path expected description)
    file(READ "${path}" header_flags OFFSET 12 LIMIT 4 HEX)
    string(TOLOWER "${header_flags}" header_flags)
    if(NOT header_flags STREQUAL "${expected}")
        message(FATAL_ERROR
                "${description}: expected ${expected}, found ${header_flags}")
    endif()
endfunction()

run_checked("channel assembly"
            "${RXAS}" -o "${WORK_DIR}/channel" "${SOURCE}")
assert_header("${WORK_DIR}/channel.rxbin" "08000000"
              "channel feature declaration")

run_checked("channel link"
            "${RXLINK}" -o "${WORK_DIR}/channel_linked"
            "${WORK_DIR}/channel.rxbin")
assert_header("${WORK_DIR}/channel_linked.rxbin" "08000000"
              "linked channel feature declaration")

run_checked("channel disassembly"
            "${RXDAS}" -o "${WORK_DIR}/channel_roundtrip.rxas"
            "${WORK_DIR}/channel.rxbin")
file(READ "${WORK_DIR}/channel_roundtrip.rxas" disassembly)
foreach(opcode IN ITEMS chanopen chanstart chanwait chancancel chanclose)
    if(NOT disassembly MATCHES "\n[ \t]+${opcode} ")
        message(FATAL_ERROR "disassembly omitted ${opcode}")
    endif()
endforeach()
run_checked("channel reassembly"
            "${RXAS}" -o "${WORK_DIR}/channel_roundtrip"
            "${WORK_DIR}/channel_roundtrip.rxas")
assert_header("${WORK_DIR}/channel_roundtrip.rxbin" "08000000"
              "round-trip channel feature declaration")

run_checked("clear required channel feature"
            "${MUTATE_FEATURE}" "${WORK_DIR}/channel.rxbin"
            "${WORK_DIR}/missing_feature.rxbin" 0)
execute_process(
        COMMAND "${RXDAS}" "${WORK_DIR}/missing_feature.rxbin"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE result)
if(result EQUAL 0 OR NOT err MATCHES
        "opcode 650 requires feature flag 0x00000008")
    message(FATAL_ERROR
            "missing channel feature was not rejected precisely:\n${out}${err}")
endif()

run_checked("add unsupported feature"
            "${MUTATE_FEATURE}" "${WORK_DIR}/channel.rxbin"
            "${WORK_DIR}/unknown_feature.rxbin" 40)
execute_process(
        COMMAND "${RXDAS}" "${WORK_DIR}/unknown_feature.rxbin"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE result)
if(result EQUAL 0 OR NOT err MATCHES "unsupported feature flags 0x00000020")
    message(FATAL_ERROR
            "unknown RXBIN feature was not rejected precisely:\n${out}${err}")
endif()

file(WRITE "${WORK_DIR}/wrong_arity.rxas"
     ".globals=0\nmain() .locals=5\n chanopen r0,r1,r2,r3\n ret\n")
execute_process(
        COMMAND "${RXAS}" -o "${WORK_DIR}/wrong_arity"
                "${WORK_DIR}/wrong_arity.rxas"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE result)
if(result EQUAL 0 OR NOT "${out}${err}" MATCHES "invalid operand")
    message(FATAL_ERROR
            "wrong channel arity was not rejected precisely:\n${out}${err}")
endif()

file(WRITE "${WORK_DIR}/duplicate_output.rxas"
     ".globals=0\nmain() .locals=5\n chanopen r0,r0,r2,r3,r4\n ret\n")
execute_process(
        COMMAND "${RXAS}" -o "${WORK_DIR}/duplicate_output"
                "${WORK_DIR}/duplicate_output.rxas"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE result)
if(result EQUAL 0 OR NOT "${out}${err}" MATCHES
        "channel instruction output registers must be distinct")
    message(FATAL_ERROR
            "duplicate channel outputs were not rejected precisely:\n${out}${err}")
endif()
