cmake_minimum_required(VERSION 3.24)

foreach(_required_var IN ITEMS RXAS RXDAS SOURCE WORKING_DIRECTORY CASE)
    if(NOT DEFINED ${_required_var} OR "${${_required_var}}" STREQUAL "")
        message(FATAL_ERROR "Missing required variable ${_required_var}")
    endif()
endforeach()

file(MAKE_DIRECTORY "${WORKING_DIRECTORY}/${CASE}")
get_filename_component(_stem "${SOURCE}" NAME_WE)
set(_output_base "${WORKING_DIRECTORY}/${CASE}/${_stem}")
set(_rxbin "${_output_base}.rxbin")
set(_disassembly "${WORKING_DIRECTORY}/${CASE}/${_stem}.rxas")

execute_process(
        COMMAND "${RXAS}" ${RXAS_FLAGS} -o "${_output_base}" "${SOURCE}"
        WORKING_DIRECTORY "${WORKING_DIRECTORY}/${CASE}"
        RESULT_VARIABLE _rxas_rc
        OUTPUT_VARIABLE _rxas_out
        ERROR_VARIABLE _rxas_err
)

if(NOT _rxas_rc EQUAL 0)
    message(FATAL_ERROR
            "rxas failed for ${SOURCE}\n"
            "case: ${CASE}\n"
            "stdout:\n${_rxas_out}\n"
            "stderr:\n${_rxas_err}")
endif()

execute_process(
        COMMAND "${RXDAS}" -o "${_disassembly}" "${_rxbin}"
        WORKING_DIRECTORY "${WORKING_DIRECTORY}/${CASE}"
        RESULT_VARIABLE _rxdas_rc
        OUTPUT_VARIABLE _rxdas_out
        ERROR_VARIABLE _rxdas_err
)

if(NOT _rxdas_rc EQUAL 0)
    message(FATAL_ERROR
            "rxdas failed for ${_rxbin}\n"
            "case: ${CASE}\n"
            "stdout:\n${_rxdas_out}\n"
            "stderr:\n${_rxdas_err}")
endif()

file(READ "${_disassembly}" _disassembly_text)
string(REPLACE "\r\n" "\n" _disassembly_text "${_disassembly_text}")
string(REPLACE "\r" "\n" _disassembly_text "${_disassembly_text}")

function(_optimizer_fail MESSAGE)
    message(FATAL_ERROR
            "${MESSAGE}\n"
            "case: ${CASE}\n"
            "source: ${SOURCE}\n"
            "disassembly: ${_disassembly}\n"
            "--- disassembly ---\n${_disassembly_text}")
endfunction()

function(_require REGEX DESCRIPTION)
    if(NOT _disassembly_text MATCHES "${REGEX}")
        _optimizer_fail("Expected ${DESCRIPTION}: ${REGEX}")
    endif()
endfunction()

function(_forbid REGEX DESCRIPTION)
    if(_disassembly_text MATCHES "${REGEX}")
        _optimizer_fail("Did not expect ${DESCRIPTION}: ${REGEX}")
    endif()
endfunction()

function(_count REGEX EXPECTED DESCRIPTION)
    string(REGEX MATCHALL "${REGEX}" _matches "${_disassembly_text}")
    list(LENGTH _matches _actual)
    if(NOT _actual EQUAL EXPECTED)
        _optimizer_fail("Expected ${EXPECTED} ${DESCRIPTION}, found ${_actual}: ${REGEX}")
    endif()
endfunction()

set(_line_prefix "(^|\n)[ \t]*")
set(_line_suffix "[ \t]*(\\*|\n|$)")

if(CASE STREQUAL "fixed_registers_opt")
    foreach(_mnemonic IN ITEMS inc0 inc1 inc2 dec0 dec1 dec2)
        _require("${_line_prefix}${_mnemonic}${_line_suffix}" "${_mnemonic} shortcut")
    endforeach()
    foreach(_instruction IN ITEMS "inc r0" "inc r1" "inc r2" "dec r0" "dec r1" "dec r2")
        string(REPLACE " " "[ \t]+" _instruction_regex "${_instruction}")
        _forbid("${_line_prefix}${_instruction_regex}${_line_suffix}" "unoptimised ${_instruction}")
    endforeach()
    _require("${_line_prefix}inc[ \t]+r3${_line_suffix}" "non-shortcut inc r3")
    _require("${_line_prefix}dec[ \t]+r3${_line_suffix}" "non-shortcut dec r3")
elseif(CASE STREQUAL "fixed_registers_noopt")
    foreach(_instruction IN ITEMS "inc r0" "inc r1" "inc r2" "inc r3" "dec r0" "dec r1" "dec r2" "dec r3")
        string(REPLACE " " "[ \t]+" _instruction_regex "${_instruction}")
        _require("${_line_prefix}${_instruction_regex}${_line_suffix}" "unoptimised ${_instruction}")
    endforeach()
    foreach(_mnemonic IN ITEMS inc0 inc1 inc2 dec0 dec1 dec2)
        _forbid("${_line_prefix}${_mnemonic}${_line_suffix}" "shortcut ${_mnemonic} with -n")
    endforeach()
elseif(CASE STREQUAL "implicit_inc0_relevant")
    _require("${_line_prefix}inc0${_line_suffix}" "inc0")
    _count("${_line_prefix}swap[ \t]+r0,r1${_line_suffix}" 2 "r0/r1 swaps preserved across inc0")
elseif(CASE STREQUAL "implicit_inc0_unrelated")
    _require("${_line_prefix}inc0${_line_suffix}" "inc0")
    _count("${_line_prefix}swap[ \t]+r3,r4${_line_suffix}" 0 "unrelated r3/r4 swaps")
elseif(CASE STREQUAL "loadint_relevant")
    _require("${_line_prefix}load[ \t]+0,1${_line_suffix}" "implicit load 0,1")
    _count("${_line_prefix}swap[ \t]+r0,r1${_line_suffix}" 2 "r0/r1 swaps preserved across load 0,1")
elseif(CASE STREQUAL "loadint_unrelated")
    _require("${_line_prefix}load[ \t]+0,1${_line_suffix}" "implicit load 0,1")
    _count("${_line_prefix}swap[ \t]+r3,r4${_line_suffix}" 0 "unrelated r3/r4 swaps")
elseif(CASE STREQUAL "linkarg_relevant")
    _require("${_line_prefix}linkarg[ \t]+r0,0${_line_suffix}" "implicit argument register linkarg")
    _count("${_line_prefix}swap[ \t]+a0[^\n]*,a1[^\n]*(\n|$)" 2 "a0/a1 swaps preserved across linkarg r0,0")
elseif(CASE STREQUAL "linkarg_unrelated")
    _require("${_line_prefix}linkarg[ \t]+r0,0${_line_suffix}" "implicit argument register linkarg")
    _count("${_line_prefix}swap[ \t]+r3,r4${_line_suffix}" 0 "unrelated r3/r4 swaps")
elseif(CASE STREQUAL "copy_acopy_opt")
    _require("${_line_prefix}copy[ \t]+r1,r0${_line_suffix}" "full copy")
    _forbid("${_line_prefix}acopy[ \t]+r1,r0${_line_suffix}" "redundant acopy after full copy")
elseif(CASE STREQUAL "copy_acopy_noopt")
    _require("${_line_prefix}copy[ \t]+r1,r0${_line_suffix}" "full copy")
    _require("${_line_prefix}acopy[ \t]+r1,r0${_line_suffix}" "acopy preserved with -n")
elseif(CASE STREQUAL "duplicate_link_read")
    _count("${_line_prefix}link[ \t]+r[0-9]+,r0${_line_suffix}" 1 "linked reads")
    _count("${_line_prefix}unlink[ \t]+r[0-9]+${_line_suffix}" 1 "read unlinks")
    _require("${_line_prefix}scopy[ \t]+r4,r2${_line_suffix}" "second read reuses first detached copy")
elseif(CASE STREQUAL "duplicate_link_read_bcopy")
    _count("${_line_prefix}link[ \t]+r[0-9]+,r0${_line_suffix}" 1 "linked binary reads")
    _count("${_line_prefix}unlink[ \t]+r[0-9]+${_line_suffix}" 1 "binary read unlinks")
    _require("${_line_prefix}bcopy[ \t]+r4,r2${_line_suffix}" "second binary read reuses first detached copy")
elseif(CASE STREQUAL "duplicate_linkattr_read")
    _count("${_line_prefix}linkattr1[ \t]+r[0-9]+,r0,1${_line_suffix}" 2 "attribute links including setup write")
    _count("${_line_prefix}unlink[ \t]+r[0-9]+${_line_suffix}" 2 "attribute unlinks including setup write")
    _require("${_line_prefix}scopy[ \t]+r4,r2${_line_suffix}" "second attribute read reuses first detached copy")
elseif(CASE STREQUAL "duplicate_linkattr_read_bcopy")
    _count("${_line_prefix}linkattr1[ \t]+r[0-9]+,r0,1${_line_suffix}" 2 "binary attribute links including setup write")
    _count("${_line_prefix}unlink[ \t]+r[0-9]+${_line_suffix}" 2 "binary attribute unlinks including setup write")
    _require("${_line_prefix}bcopy[ \t]+r4,r2${_line_suffix}" "second binary attribute read reuses first detached copy")
elseif(CASE STREQUAL "duplicate_linkattr_different")
    _count("${_line_prefix}linkattr1[ \t]+r[0-9]+,r0,[12]${_line_suffix}" 4 "attribute links for two setup writes and two reads")
    _count("${_line_prefix}unlink[ \t]+r[0-9]+${_line_suffix}" 4 "attribute unlinks for two setup writes and two reads")
    _require("${_line_prefix}scopy[ \t]+r4,r3${_line_suffix}" "second read remains linked to the second attribute slot")
    _forbid("${_line_prefix}scopy[ \t]+r4,r2${_line_suffix}" "cross-slot detached-copy reuse")
elseif(CASE MATCHES "^barrier_")
    _count("${_line_prefix}swap[ \t]+r1,r2${_line_suffix}" 2 "r1/r2 swaps preserved by optimiser barrier")
else()
    _optimizer_fail("Unknown optimiser check case")
endif()
