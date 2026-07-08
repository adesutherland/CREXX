cmake_minimum_required(VERSION 3.16)

foreach(_required_var IN ITEMS RXAS WORKING_DIRECTORY)
    if(NOT DEFINED ${_required_var} OR "${${_required_var}}" STREQUAL "")
        message(FATAL_ERROR "Missing required variable ${_required_var}")
    endif()
endforeach()

file(MAKE_DIRECTORY "${WORKING_DIRECTORY}/constant_aliases")

function(run_rxas_case CASE_NAME SOURCE EXPECT_SUCCESS EXPECT_REGEX)
    set(_source "${WORKING_DIRECTORY}/constant_aliases/${CASE_NAME}.rxas")
    set(_output "${WORKING_DIRECTORY}/constant_aliases/${CASE_NAME}")
    file(WRITE "${_source}" "${SOURCE}")

    execute_process(
            COMMAND "${RXAS}" -o "${_output}" "${_source}"
            WORKING_DIRECTORY "${WORKING_DIRECTORY}/constant_aliases"
            RESULT_VARIABLE _res
            OUTPUT_VARIABLE _out
            ERROR_VARIABLE _err
    )

    set(_full_out "${_out}${_err}")
    string(REPLACE "\r\n" "\n" _full_out "${_full_out}")
    string(REPLACE "\r" "\n" _full_out "${_full_out}")

    if(EXPECT_SUCCESS AND NOT _res EQUAL 0)
        message(FATAL_ERROR
                "Expected rxas to accept ${CASE_NAME}, but it failed.\n"
                "Output:\n${_full_out}")
    endif()

    if(NOT EXPECT_SUCCESS AND _res EQUAL 0)
        message(FATAL_ERROR
                "Expected rxas to reject ${CASE_NAME}, but it succeeded.\n"
                "Output:\n${_full_out}")
    endif()

    if(NOT "${EXPECT_REGEX}" STREQUAL "" AND NOT _full_out MATCHES "${EXPECT_REGEX}")
        message(FATAL_ERROR
                "Expected diagnostic ${EXPECT_REGEX} for ${CASE_NAME}.\n"
                "Output:\n${_full_out}")
    endif()
endfunction()

run_rxas_case(success
".globals=0
.const table binary 0x00112233
.const key string \"index\"
main() .locals=4
    load r1,0x
    load r2,2
    bresize r1,r2
    load r2,1
    bcopy r1,table,r2
    load r2,0
    bcmps r2,table,key
    ret 0
" TRUE "")

run_rxas_case(duplicate
".globals=0
.const table binary 0x00
.const table binary 0x01
main() .locals=1
    ret 0
" FALSE "duplicate constant alias")

run_rxas_case(wrong_kind
".globals=0
.const table binary \"not-binary\"
main() .locals=1
    ret 0
" FALSE "binary constant alias requires a hex literal")

run_rxas_case(undefined
".globals=0
main() .locals=2
    blen r1,table
    ret 0
" FALSE "invalid operand")
