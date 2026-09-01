cmake_minimum_required(VERSION 3.16)

foreach(_required_var IN ITEMS RXAS WORKING_DIRECTORY)
    if(NOT DEFINED ${_required_var} OR "${${_required_var}}" STREQUAL "")
        message(FATAL_ERROR "Missing required variable ${_required_var}")
    endif()
endforeach()

file(MAKE_DIRECTORY "${WORKING_DIRECTORY}/reject_non_source_mnemonics")

function(expect_rxas_reject CASE_NAME LINE EXPECTED_DIAGNOSTIC)
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

    if(NOT _full_out MATCHES "${EXPECTED_DIAGNOSTIC}")
        message(FATAL_ERROR
                "Expected ${EXPECTED_DIAGNOSTIC} diagnostic for ${LINE}.\n"
                "Output:\n${_full_out}")
    endif()
endfunction()

expect_rxas_reject(inull "inull" "invalid instruction mnemonic")
expect_rxas_reject(interrupt "interrupt" "invalid instruction mnemonic")
expect_rxas_reject(iunknown "iunknown" "invalid instruction mnemonic")
expect_rxas_reject(opendll "opendll r0,r1,r2" "invalid instruction mnemonic")
expect_rxas_reject(dllparms "dllparms r0,r1,r2" "invalid instruction mnemonic")
expect_rxas_reject(reserved "reserved" "invalid instruction mnemonic")
expect_rxas_reject(reserved_514 "reserved_514" "invalid instruction mnemonic")

# Gate F retires the pre-release process and redirect instructions.  Their
# numeric slots remain reserved for stale-image diagnostics, but neither the
# old names nor the reserved names are legal source mnemonics.
expect_rxas_reject(spawn "spawn r0,r1,r2" "invalid instruction mnemonic")
expect_rxas_reject(redir2str "redir2str r0,r1" "invalid instruction mnemonic")
expect_rxas_reject(redir2arr "redir2arr r0,r1" "invalid instruction mnemonic")
expect_rxas_reject(str2redir "str2redir r0,r1" "invalid instruction mnemonic")
expect_rxas_reject(arr2redir "arr2redir r0,r1" "invalid instruction mnemonic")
expect_rxas_reject(nullredir "nullredir r0" "invalid instruction mnemonic")
expect_rxas_reject(reserved_466 "reserved_466" "invalid instruction mnemonic")
expect_rxas_reject(reserved_471 "reserved_471" "invalid instruction mnemonic")

# Cursor-bearing source mnemonics were deliberately retired by the cursorless
# value redesign. Keep this list explicit so a stale spelling cannot silently
# return through an unrelated opcode-table edit.
expect_rxas_reject(getstrpos "getstrpos r0,r1" "invalid instruction mnemonic")
expect_rxas_reject(setstrpos "setstrpos r0,r1" "invalid instruction mnemonic")
expect_rxas_reject(substr "substr r0,r1,r2" "invalid instruction mnemonic")
expect_rxas_reject(getbinpos "getbinpos r0,r1" "invalid instruction mnemonic")
expect_rxas_reject(setbinpos "setbinpos r0,r1" "invalid instruction mnemonic")

# The retained slice mnemonics require explicit start and length registers.
expect_rxas_reject(substring_three_operands "substring r0,r1,r2" "invalid operand")
expect_rxas_reject(bslice_three_operands "bslice r0,r1,r2" "invalid operand")
