cmake_minimum_required(VERSION 3.16)

foreach(_required_var IN ITEMS RXAS WORKING_DIRECTORY)
    if(NOT DEFINED ${_required_var} OR "${${_required_var}}" STREQUAL "")
        message(FATAL_ERROR "Missing required variable ${_required_var}")
    endif()
endforeach()

file(MAKE_DIRECTORY "${WORKING_DIRECTORY}/reject_non_source_mnemonics")

function(expect_rxas_reject CASE_NAME LINE)
    set(_source "${WORKING_DIRECTORY}/reject_non_source_mnemonics/${CASE_NAME}.rxas")
    set(_output "${WORKING_DIRECTORY}/reject_non_source_mnemonics/${CASE_NAME}")
    file(WRITE "${_source}" "main() .locals=3\n    ${LINE}\n    ret 0\n")

    execute_process(
            COMMAND "${RXAS}" -o "${_output}" "${_source}"
            WORKING_DIRECTORY "${WORKING_DIRECTORY}/reject_non_source_mnemonics"
            RESULT_VARIABLE _res
            OUTPUT_VARIABLE _out
            ERROR_VARIABLE _err
    )

    set(_full_out "${_out}${_err}")
    string(REPLACE "\r\n" "\n" _full_out "${_full_out}")
    string(REPLACE "\r" "\n" _full_out "${_full_out}")

    if(_res EQUAL 0)
        message(FATAL_ERROR
                "Expected rxas to reject ${LINE}, but it succeeded.\n"
                "Output:\n${_full_out}")
    endif()

    if(NOT _full_out MATCHES "invalid instruction mnemonic")
        message(FATAL_ERROR
                "Expected invalid mnemonic diagnostic for ${LINE}.\n"
                "Output:\n${_full_out}")
    endif()
endfunction()

expect_rxas_reject(inull "inull")
expect_rxas_reject(interrupt "interrupt")
expect_rxas_reject(iunknown "iunknown")
expect_rxas_reject(opendll "opendll r0,r1,r2")
expect_rxas_reject(dllparms "dllparms r0,r1,r2")
expect_rxas_reject(reserved "reserved")
expect_rxas_reject(reserved_514 "reserved_514")
