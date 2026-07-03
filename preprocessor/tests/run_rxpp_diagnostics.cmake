cmake_minimum_required(VERSION 3.24)

file(MAKE_DIRECTORY "${WORK}")

function(expect_contains label text expected)
    string(FIND "${text}" "${expected}" pos)
    if(pos EQUAL -1)
        message(FATAL_ERROR "${label} did not contain expected text '${expected}':\n${text}")
    endif()
endfunction()

function(expect_not_contains label text unexpected)
    string(FIND "${text}" "${unexpected}" pos)
    if(NOT pos EQUAL -1)
        message(FATAL_ERROR "${label} contained unexpected text '${unexpected}':\n${text}")
    endif()
endfunction()

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
            "CREXX_DIAGNOSTICS=raw"
            "CREXX_MESSAGE_PATH=${MESSAGE_PATH}"
            "${RXPP_BIN}"
    WORKING_DIRECTORY "${WORK}"
    RESULT_VARIABLE raw_res
    OUTPUT_VARIABLE raw_out
    ERROR_VARIABLE raw_err)
set(raw_full "${raw_out}${raw_err}")
if(NOT raw_res EQUAL 8)
    message(FATAL_ERROR "raw no-source diagnostic returned ${raw_res}, expected 8:\n${raw_full}")
endif()
expect_contains("raw no-source diagnostic" "${raw_full}" "RXPP_NO_SOURCE_FILE")
expect_not_contains("raw no-source diagnostic" "${raw_full}" "No source file")

set(double_input "${WORK}/already_srcmap.rxpp")
set(double_output "${WORK}/already_srcmap.out.crexx")
file(WRITE "${double_input}" "options levelb srcmap\nsay \"already mapped\"\n")

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
            "CREXX_DIAGNOSTICS=localized"
            "CREXX_DIAGNOSTIC_LOCALE=de_DE"
            "CREXX_MESSAGE_PATH=${MESSAGE_PATH}"
            "${RXPP_BIN}" -I "${double_input}" -O "${double_output}" -M "${MACLIB_FILE}"
    WORKING_DIRECTORY "${WORK}"
    RESULT_VARIABLE de_res
    OUTPUT_VARIABLE de_out
    ERROR_VARIABLE de_err)
set(de_full "${de_out}${de_err}")
if(NOT de_res EQUAL 8)
    message(FATAL_ERROR "German already-srcmap diagnostic returned ${de_res}, expected 8:\n${de_full}")
endif()
expect_contains("German already-srcmap diagnostic" "${de_full}" "RXPP_ALREADY_SOURCE_MAPPED: Eingabe enthält bereits options srcmap")
expect_contains("German already-srcmap hint" "${de_full}" "RXPP_ALREADY_SOURCE_MAPPED_HINT: Führen Sie rxc")

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
            "CREXX_DIAGNOSTICS=localized"
            "CREXX_DIAGNOSTIC_LOCALE=nl_NL"
            "CREXX_MESSAGE_PATH=${MESSAGE_PATH}"
            "${RXPP_BIN}" -I "${double_input}" -O "${double_output}" -M "${MACLIB_FILE}"
    WORKING_DIRECTORY "${WORK}"
    RESULT_VARIABLE nl_res
    OUTPUT_VARIABLE nl_out
    ERROR_VARIABLE nl_err)
set(nl_full "${nl_out}${nl_err}")
if(NOT nl_res EQUAL 8)
    message(FATAL_ERROR "Dutch already-srcmap diagnostic returned ${nl_res}, expected 8:\n${nl_full}")
endif()
expect_contains("Dutch already-srcmap diagnostic" "${nl_full}" "RXPP_ALREADY_SOURCE_MAPPED: Invoer bevat al options srcmap")
expect_contains("Dutch already-srcmap hint" "${nl_full}" "RXPP_ALREADY_SOURCE_MAPPED_HINT: Voer rxc uit")

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
            "CREXX_DIAGNOSTICS=raw"
            "CREXX_MESSAGE_PATH=${MESSAGE_PATH}"
            "${RXPP_BIN}" -I "${double_input}" -O "${double_output}" -M "${MACLIB_FILE}"
    WORKING_DIRECTORY "${WORK}"
    RESULT_VARIABLE raw_double_res
    OUTPUT_VARIABLE raw_double_out
    ERROR_VARIABLE raw_double_err)
set(raw_double_full "${raw_double_out}${raw_double_err}")
if(NOT raw_double_res EQUAL 8)
    message(FATAL_ERROR "raw already-srcmap diagnostic returned ${raw_double_res}, expected 8:\n${raw_double_full}")
endif()
expect_contains("raw already-srcmap diagnostic" "${raw_double_full}" "RXPP_ALREADY_SOURCE_MAPPED file=\"${double_input}\"")
expect_not_contains("raw already-srcmap diagnostic" "${raw_double_full}" "Input already contains options srcmap")

set(warn_input "${WORK}/warning.rxpp")
set(warn_output "${WORK}/warning.out.crexx")
set(missing_maclib "${WORK}/missing_maclib.rexx")
file(WRITE "${warn_input}" "options levelb\nsay \"warning path\"\n")
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E env
            "CREXX_DIAGNOSTICS=localized"
            "CREXX_DIAGNOSTIC_LOCALE=en_GB"
            "CREXX_MESSAGE_PATH=${MESSAGE_PATH}"
            "${RXPP_BIN}" -I "${warn_input}" -O "${warn_output}" -M "${missing_maclib}" -VERBOSE
    WORKING_DIRECTORY "${WORK}"
    RESULT_VARIABLE warn_res
    OUTPUT_VARIABLE warn_out
    ERROR_VARIABLE warn_err)
set(warn_full "${warn_out}${warn_err}")
if(NOT warn_res EQUAL 0)
    message(FATAL_ERROR "localized warning diagnostic run failed with ${warn_res}:\n${warn_full}")
endif()
expect_contains("localized warning diagnostic" "${warn_full}" "RXPP_MACLIB_MISSING: Macro library was not found or is not accessible")
expect_contains("localized system warning diagnostic" "${warn_full}" "RXPP_SYSTEM_MACLIB_MISSING: System macro library was not found or is not accessible")
