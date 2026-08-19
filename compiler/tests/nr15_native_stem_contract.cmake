cmake_minimum_required(VERSION 3.20)

foreach(required IN ITEMS RXAS RXLINK RXDAS RXVM RXBVM MUTATE_FEATURE
                          SOURCE WORK_DIR)
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

function(assert_runs image expected)
    foreach(runner IN ITEMS RXVM RXBVM)
        execute_process(
                COMMAND "${${runner}}" "${image}"
                OUTPUT_VARIABLE out
                ERROR_VARIABLE err
                RESULT_VARIABLE result)
        string(REPLACE "\r\n" "\n" out "${out}")
        string(REPLACE "\r" "\n" out "${out}")
        if(NOT result EQUAL 0 OR NOT out STREQUAL "${expected}")
            message(FATAL_ERROR
                    "${runner} mismatch for ${image}: rc=${result}\n${out}${err}")
        endif()
    endforeach()
endfunction()

set(expected "joined\ndefault\n1\nleft.right\njoined\n")

run_checked("native-stem assembly"
            "${RXAS}" -o "${WORK_DIR}/native_stem" "${SOURCE}")
assert_header("${WORK_DIR}/native_stem.rxbin" "04000000"
              "native-stem feature declaration")
assert_runs("${WORK_DIR}/native_stem.rxbin" "${expected}")

run_checked("native-stem link"
            "${RXLINK}" -o "${WORK_DIR}/native_stem_linked"
            "${WORK_DIR}/native_stem.rxbin")
assert_header("${WORK_DIR}/native_stem_linked.rxbin" "04000000"
              "linked native-stem feature declaration")
assert_runs("${WORK_DIR}/native_stem_linked.rxbin" "${expected}")

run_checked("native-stem disassembly"
            "${RXDAS}" -o "${WORK_DIR}/native_stem_roundtrip.rxas"
            "${WORK_DIR}/native_stem.rxbin")
file(READ "${WORK_DIR}/native_stem_roundtrip.rxas" disassembly)
foreach(opcode IN ITEMS steminit stemget stemset stemreset stemget2 stemset2
                        stemsize stemkeyat stemvalueat)
    if(NOT disassembly MATCHES "\n[ \t]+${opcode} ")
        message(FATAL_ERROR "disassembly omitted ${opcode}")
    endif()
endforeach()
run_checked("native-stem reassembly"
            "${RXAS}" -o "${WORK_DIR}/native_stem_roundtrip"
            "${WORK_DIR}/native_stem_roundtrip.rxas")
assert_header("${WORK_DIR}/native_stem_roundtrip.rxbin" "04000000"
              "round-trip native-stem feature declaration")
assert_runs("${WORK_DIR}/native_stem_roundtrip.rxbin" "${expected}")

run_checked("clear required native-stem feature"
            "${MUTATE_FEATURE}" "${WORK_DIR}/native_stem.rxbin"
            "${WORK_DIR}/missing_feature.rxbin" 0)
execute_process(
        COMMAND "${RXDAS}" "${WORK_DIR}/missing_feature.rxbin"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE result)
if(result EQUAL 0 OR NOT err MATCHES
        "opcode 641 requires feature flag 0x00000004")
    message(FATAL_ERROR
            "missing native-stem feature was not rejected precisely:\n${out}${err}")
endif()

run_checked("add unsupported feature"
            "${MUTATE_FEATURE}" "${WORK_DIR}/native_stem.rxbin"
            "${WORK_DIR}/unknown_feature.rxbin" 36)
execute_process(
        COMMAND "${RXDAS}" "${WORK_DIR}/unknown_feature.rxbin"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE result)
if(result EQUAL 0 OR NOT err MATCHES "unsupported feature flags 0x00000020")
    message(FATAL_ERROR
            "unknown RXBIN feature was not rejected precisely:\n${out}${err}")
endif()
