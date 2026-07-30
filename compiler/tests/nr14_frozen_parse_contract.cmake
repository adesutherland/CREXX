cmake_minimum_required(VERSION 3.20)

foreach(required IN ITEMS RXC RXAS RXLINK RXDAS RXVM RXBVM MUTATE_FEATURE
                          BIN_DIR SOURCE_DIR WORK_DIR)
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
            RESULT_VARIABLE result
            ENCODING UTF-8)
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

function(assert_match_count variable pattern expected description)
    string(REGEX MATCHALL "${pattern}" matches "${${variable}}")
    list(LENGTH matches count)
    if(NOT count EQUAL expected)
        message(FATAL_ERROR
                "${description}: expected ${expected} matches, found ${count}")
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
                RESULT_VARIABLE result
                ENCODING UTF-8)
        string(REPLACE "\r\n" "\n" out "${out}")
        string(REPLACE "\r" "\n" out "${out}")
        if(NOT result EQUAL 0 OR NOT out STREQUAL "${expected}")
            message(FATAL_ERROR
                    "${runner} mismatch for ${image}: rc=${result}\n${out}${err}")
        endif()
    endforeach()
endfunction()

set(source "${SOURCE_DIR}/nr14_frozen_parse_contract.crexx")
set(expected "alpha|beta|gamma tail\nalpha|beta|gamma tail\nalpha|beta|gamma\nab東京|cd\n333||\none|two|three|four|five|six|seven eight nine\none|two|three|four|five|six|seven|eight nine\nleft|middle|right\n345|6789|3456789\na|c|d\nsecond|third\n")

run_checked("optimized rxc"
            "${RXC}" -i "${BIN_DIR}" -o "${WORK_DIR}/parse_opt" "${source}")
run_checked("no-opt rxc"
            "${RXC}" -i "${BIN_DIR}" -n -o "${WORK_DIR}/parse_noopt" "${source}")

foreach(mode IN ITEMS parse_opt parse_noopt)
    file(READ "${WORK_DIR}/${mode}.rxas" rxas_text)
    foreach(opcode IN ITEMS parsewords3 parsewords3d parsepos2 parseplan)
        assert_matches(rxas_text "\n[ \t]+${opcode} "
                       "${mode} ${opcode} form")
    endforeach()
    assert_match_count(rxas_text "\n[ \t]+parsewords3 " 7
                       "${mode} exact and chained parsewords3 forms")
    assert_match_count(rxas_text "\n[ \t]+parsewords3d " 1
                       "${mode} exact parsewords3d form")
    assert_match_count(rxas_text "\n[ \t]+parsepos2 " 1
                       "${mode} exact parsepos2 form")
    assert_match_count(rxas_text "\n[ \t]+parseplan " 4
                       "${mode} generic prepared forms")
    assert_matches(rxas_text "50010800"
                   "${mode} prepared descriptor header")
    run_checked("${mode} rxas"
                "${RXAS}" -o "${WORK_DIR}/${mode}" "${WORK_DIR}/${mode}.rxas")
    assert_header("${WORK_DIR}/${mode}.rxbin" "02000000"
                  "frozen-PARSE feature declaration")
    assert_runs("${WORK_DIR}/${mode}.rxbin" "${expected}")
endforeach()

run_checked("frozen-PARSE link"
            "${RXLINK}" -o "${WORK_DIR}/parse_linked"
            "${WORK_DIR}/parse_opt.rxbin")
assert_header("${WORK_DIR}/parse_linked.rxbin" "02000000"
              "linked frozen-PARSE feature declaration")
assert_runs("${WORK_DIR}/parse_linked.rxbin" "${expected}")

run_checked("frozen-PARSE disassembly"
            "${RXDAS}" -o "${WORK_DIR}/parse_roundtrip.rxas"
            "${WORK_DIR}/parse_opt.rxbin")
file(READ "${WORK_DIR}/parse_roundtrip.rxas" disassembly)
foreach(opcode IN ITEMS parsewords3 parsewords3d parsepos2 parseplan)
    assert_matches(disassembly "\n[ \t]+${opcode} "
                   "disassembled ${opcode} form")
endforeach()
run_checked("frozen-PARSE reassembly"
            "${RXAS}" -o "${WORK_DIR}/parse_roundtrip"
            "${WORK_DIR}/parse_roundtrip.rxas")
assert_header("${WORK_DIR}/parse_roundtrip.rxbin" "02000000"
              "round-trip frozen-PARSE feature declaration")
assert_runs("${WORK_DIR}/parse_roundtrip.rxbin" "${expected}")

run_checked("clear required feature"
            "${MUTATE_FEATURE}" "${WORK_DIR}/parse_opt.rxbin"
            "${WORK_DIR}/missing_feature.rxbin" 0)
execute_process(
        COMMAND "${RXDAS}" "${WORK_DIR}/missing_feature.rxbin"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE result
        ENCODING UTF-8)
if(result EQUAL 0 OR NOT err MATCHES "opcode 405 requires feature flag 0x00000002")
    message(FATAL_ERROR "missing frozen-PARSE feature was not rejected precisely:\n${out}${err}")
endif()

run_checked("add unsupported feature"
            "${MUTATE_FEATURE}" "${WORK_DIR}/parse_opt.rxbin"
            "${WORK_DIR}/unknown_feature.rxbin" 10)
execute_process(
        COMMAND "${RXDAS}" "${WORK_DIR}/unknown_feature.rxbin"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE result
        ENCODING UTF-8)
if(result EQUAL 0 OR NOT err MATCHES "unsupported feature flags 0x00000008")
    message(FATAL_ERROR "unknown RXBIN feature was not rejected precisely:\n${out}${err}")
endif()

set(invalid_source "${SOURCE_DIR}/nr14_invalid_parseplan.crexx")
run_checked("invalid-descriptor rxc"
            "${RXC}" -i "${BIN_DIR}" -o "${WORK_DIR}/invalid_parseplan"
            "${invalid_source}")
run_checked("invalid-descriptor rxas"
            "${RXAS}" -o "${WORK_DIR}/invalid_parseplan"
            "${WORK_DIR}/invalid_parseplan.rxas")
run_checked("clear prepared-plan required feature"
            "${MUTATE_FEATURE}" "${WORK_DIR}/invalid_parseplan.rxbin"
            "${WORK_DIR}/invalid_parseplan_missing_feature.rxbin" 0)
execute_process(
        COMMAND "${RXDAS}" "${WORK_DIR}/invalid_parseplan_missing_feature.rxbin"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE result
        ENCODING UTF-8)
if(result EQUAL 0 OR NOT err MATCHES "opcode 410 requires feature flag 0x00000002")
    message(FATAL_ERROR
            "missing prepared-plan feature was not rejected precisely:\n${out}${err}")
endif()
foreach(runner IN ITEMS RXVM RXBVM)
    execute_process(
            COMMAND "${${runner}}" "${WORK_DIR}/invalid_parseplan.rxbin"
            OUTPUT_VARIABLE out
            ERROR_VARIABLE err
            RESULT_VARIABLE result
            ENCODING UTF-8)
    if(result EQUAL 0 OR NOT "${out}${err}" MATCHES "INVALID_ARGUMENTS")
        message(FATAL_ERROR
                "${runner} accepted an invalid prepared descriptor: rc=${result}\n${out}${err}")
    endif()
endforeach()
