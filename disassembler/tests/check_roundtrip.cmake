cmake_minimum_required(VERSION 3.24)

foreach(_required_var IN ITEMS RXDAS RXAS RXVM RXBVM INPUT_RXBIN WORKING_DIRECTORY TEST_BASENAME)
    if(NOT DEFINED ${_required_var} OR "${${_required_var}}" STREQUAL "")
        message(FATAL_ERROR "Missing required variable ${_required_var}")
    endif()
endforeach()

function(_normalize_text INPUT OUTPUT)
    set(_value "${${INPUT}}")
    string(REPLACE "\r\n" "\n" _value "${_value}")
    string(REPLACE "\r" "\n" _value "${_value}")
    set(${OUTPUT} "${_value}" PARENT_SCOPE)
endfunction()

set(_disassembly "${WORKING_DIRECTORY}/${TEST_BASENAME}.roundtrip.rxas")
set(_reassembled_base "${TEST_BASENAME}.roundtrip")
set(_reassembled_rxbin "${WORKING_DIRECTORY}/${_reassembled_base}.rxbin")

execute_process(
        COMMAND "${RXDAS}" -o "${_disassembly}" "${INPUT_RXBIN}"
        WORKING_DIRECTORY "${WORKING_DIRECTORY}"
        RESULT_VARIABLE _rxdas_rc
        OUTPUT_VARIABLE _rxdas_out
        ERROR_VARIABLE _rxdas_err
)

if(NOT _rxdas_rc EQUAL 0)
    message(FATAL_ERROR
            "rxdas failed for ${INPUT_RXBIN}\n"
            "stdout:\n${_rxdas_out}\n"
            "stderr:\n${_rxdas_err}")
endif()

if(DEFINED REQUIRE_NO_UNKNOWN AND REQUIRE_NO_UNKNOWN)
    file(READ "${_disassembly}" _disassembly_text)
    string(FIND "${_disassembly_text}" "UNKNOWN" _unknown_index)
    if(NOT _unknown_index EQUAL -1)
        message(FATAL_ERROR
                "Disassembly still contains UNKNOWN entries for ${INPUT_RXBIN}\n"
                "disassembly: ${_disassembly}")
    endif()
endif()

execute_process(
        COMMAND "${RXAS}" -o "${_reassembled_base}" "${_disassembly}"
        WORKING_DIRECTORY "${WORKING_DIRECTORY}"
        RESULT_VARIABLE _rxas_rc
        OUTPUT_VARIABLE _rxas_out
        ERROR_VARIABLE _rxas_err
)

if(NOT _rxas_rc EQUAL 0)
    message(FATAL_ERROR
            "rxas failed for ${_disassembly}\n"
            "stdout:\n${_rxas_out}\n"
            "stderr:\n${_rxas_err}")
endif()

foreach(_runner_var IN ITEMS RXVM RXBVM)
    execute_process(
            COMMAND "${${_runner_var}}" "${INPUT_RXBIN}"
            WORKING_DIRECTORY "${WORKING_DIRECTORY}"
            RESULT_VARIABLE _orig_rc
            OUTPUT_VARIABLE _orig_out
            ERROR_VARIABLE _orig_err
    )

    execute_process(
            COMMAND "${${_runner_var}}" "${_reassembled_rxbin}"
            WORKING_DIRECTORY "${WORKING_DIRECTORY}"
            RESULT_VARIABLE _re_rc
            OUTPUT_VARIABLE _re_out
            ERROR_VARIABLE _re_err
    )

    _normalize_text(_orig_out _orig_out_norm)
    _normalize_text(_orig_err _orig_err_norm)
    _normalize_text(_re_out _re_out_norm)
    _normalize_text(_re_err _re_err_norm)

    if(NOT _orig_rc EQUAL _re_rc)
        message(FATAL_ERROR
                "${_runner_var} exit code mismatch for ${INPUT_RXBIN}\n"
                "original=${_orig_rc} reassembled=${_re_rc}")
    endif()

    if(NOT _orig_out_norm STREQUAL _re_out_norm)
        message(FATAL_ERROR
                "${_runner_var} stdout mismatch for ${INPUT_RXBIN}\n"
                "original:\n${_orig_out_norm}\n"
                "reassembled:\n${_re_out_norm}")
    endif()

    if(NOT _orig_err_norm STREQUAL _re_err_norm)
        message(FATAL_ERROR
                "${_runner_var} stderr mismatch for ${INPUT_RXBIN}\n"
                "original:\n${_orig_err_norm}\n"
                "reassembled:\n${_re_err_norm}")
    endif()
endforeach()
