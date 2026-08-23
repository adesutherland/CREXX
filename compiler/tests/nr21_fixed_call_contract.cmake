cmake_minimum_required(VERSION 3.20)

foreach(required IN ITEMS RXC RXAS RXLINK RXDAS RXVM RXBVM MUTATE_FEATURE
                          BIN_DIR SOURCE_DIR BASIC_RXAS WORK_DIR)
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

function(assert_matches variable pattern description)
    string(REGEX MATCH "${pattern}" match "${${variable}}")
    if(match STREQUAL "")
        message(FATAL_ERROR "${description}: expected pattern was absent")
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

set(source "${SOURCE_DIR}/nr21_fixed_call_contract.crexx")
set(expected "0\n1\n3\n6\n10\n15\n")

run_checked("optimized rxc"
            "${RXC}" -i "${BIN_DIR}" -o "${WORK_DIR}/fixed_opt" "${source}")
run_checked("no-opt rxc"
            "${RXC}" -i "${BIN_DIR}" -n -o "${WORK_DIR}/fixed_noopt" "${source}")

foreach(mode IN ITEMS fixed_opt fixed_noopt)
    file(READ "${WORK_DIR}/${mode}.rxas" rxas_text)
    assert_matches(rxas_text "\n[ \t]+call [^\n]*,zero\\(\\)\n"
                   "arity-zero direct form")
    foreach(arity RANGE 1 4)
        assert_matches(rxas_text "\n[ \t]+call${arity} "
                       "fixed arity-${arity} direct form")
    endforeach()
    assert_matches(rxas_text
                   "\n[ \t]+load r[0-9]+,5\n[ \t]+[^\n]*call [^\n]*,five\\(\\),r[0-9]+"
                   "arity-five counted fallback")

    run_checked("${mode} rxas"
                "${RXAS}" -o "${WORK_DIR}/${mode}" "${WORK_DIR}/${mode}.rxas")
    assert_header("${WORK_DIR}/${mode}.rxbin" "01000000"
                  "fixed-call feature declaration")
    assert_runs("${WORK_DIR}/${mode}.rxbin" "${expected}")
endforeach()

run_checked("fixed-call link"
            "${RXLINK}" -o "${WORK_DIR}/fixed_linked"
            "${WORK_DIR}/fixed_opt.rxbin")
assert_header("${WORK_DIR}/fixed_linked.rxbin" "01000000"
              "linked fixed-call feature declaration")
assert_runs("${WORK_DIR}/fixed_linked.rxbin" "${expected}")

run_checked("fixed-call disassembly"
            "${RXDAS}" -o "${WORK_DIR}/fixed_roundtrip.rxas"
            "${WORK_DIR}/fixed_opt.rxbin")
file(READ "${WORK_DIR}/fixed_roundtrip.rxas" disassembly)
foreach(arity RANGE 1 4)
    assert_matches(disassembly "\n[ \t]+call${arity} "
                   "disassembled fixed arity-${arity} form")
endforeach()
run_checked("fixed-call reassembly"
            "${RXAS}" -o "${WORK_DIR}/fixed_roundtrip"
            "${WORK_DIR}/fixed_roundtrip.rxas")
assert_header("${WORK_DIR}/fixed_roundtrip.rxbin" "01000000"
              "round-trip fixed-call feature declaration")
assert_runs("${WORK_DIR}/fixed_roundtrip.rxbin" "${expected}")

run_checked("zero-feature compatibility fixture"
            "${RXAS}" -o "${WORK_DIR}/zero_feature" "${BASIC_RXAS}")
assert_header("${WORK_DIR}/zero_feature.rxbin" "00000000"
              "zero-feature compatibility image")
foreach(runner IN ITEMS RXVM RXBVM)
    run_checked("${runner} zero-feature compatibility"
                "${${runner}}" "${WORK_DIR}/zero_feature.rxbin")
endforeach()

run_checked("clear required feature"
            "${MUTATE_FEATURE}" "${WORK_DIR}/fixed_opt.rxbin"
            "${WORK_DIR}/missing_feature.rxbin" 0)
execute_process(
        COMMAND "${RXDAS}" "${WORK_DIR}/missing_feature.rxbin"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE result)
if(result EQUAL 0 OR NOT err MATCHES "opcode 401 requires feature flag 0x00000001")
    message(FATAL_ERROR "missing fixed-call feature was not rejected precisely:\n${out}${err}")
endif()

run_checked("add unsupported feature"
            "${MUTATE_FEATURE}" "${WORK_DIR}/fixed_opt.rxbin"
            "${WORK_DIR}/unknown_feature.rxbin" 65)
execute_process(
        COMMAND "${RXDAS}" "${WORK_DIR}/unknown_feature.rxbin"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE result)
if(result EQUAL 0 OR NOT err MATCHES "unsupported feature flags 0x00000040")
    message(FATAL_ERROR "unknown RXBIN feature was not rejected precisely:\n${out}${err}")
endif()
