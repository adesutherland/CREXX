cmake_minimum_required(VERSION 3.24)

foreach(_required_var IN ITEMS RXAS_SOURCE RXBIN RXDAS DISASSEMBLY CASE)
    if(NOT DEFINED ${_required_var} OR "${${_required_var}}" STREQUAL "")
        message(FATAL_ERROR "Missing required variable ${_required_var}")
    endif()
endforeach()

if(NOT EXISTS "${RXAS_SOURCE}")
    message(FATAL_ERROR "Missing DECIMAL-01 RXAS source: ${RXAS_SOURCE}")
endif()
if(NOT EXISTS "${RXBIN}")
    message(FATAL_ERROR "Missing DECIMAL-01 RXBIN image: ${RXBIN}")
endif()

execute_process(
        COMMAND "${RXDAS}" -o "${DISASSEMBLY}" "${RXBIN}"
        RESULT_VARIABLE _rxdas_rc
        OUTPUT_VARIABLE _rxdas_out
        ERROR_VARIABLE _rxdas_err
)
if(NOT _rxdas_rc EQUAL 0)
    message(FATAL_ERROR
            "DECIMAL-01 optimizer integrity disassembly failed for ${CASE}\n"
            "stdout:\n${_rxdas_out}\n"
            "stderr:\n${_rxdas_err}")
endif()

file(READ "${RXAS_SOURCE}" _source_rxas)
file(READ "${DISASSEMBLY}" _rxas)
string(REPLACE "\r\n" "\n" _rxas "${_rxas}")
string(REPLACE "\r" "\n" _rxas "${_rxas}")

function(_require_minimum MNEMONIC MINIMUM)
    string(REGEX MATCHALL
            "(^|\n)([ \t]*[A-Za-z_][A-Za-z0-9_]*:[ \t]*)?[ \t]*${MNEMONIC}[ \t]+"
            _matches "${_rxas}")
    list(LENGTH _matches _actual)
    if(_actual LESS MINIMUM)
        message(FATAL_ERROR
                "DECIMAL-01 optimizer integrity failure for ${CASE}: "
                "expected at least ${MINIMUM} ${MNEMONIC} instructions, "
                "found ${_actual} in final disassembly ${DISASSEMBLY}")
    endif()
endfunction()

# These conservative floors are satisfied by both current opt and no-opt
# images. They prove every timed kernel family still contains runtime provider
# work without pinning harmless register allocation or inlining changes.
_require_minimum(stod 18)
_require_minimum(dtos 6)
_require_minimum(dadd 4)
_require_minimum(dsub 2)
_require_minimum(dmult 2)
_require_minimum(ddiv 2)
_require_minimum(deq 1)
_require_minimum(dlt 2)

# A decimal immediate is parsed and allocated by the VM on every instruction
# dispatch.  The arithmetic kernel deliberately initializes these two decimal
# values once before its hot loop.  Compiler constant propagation must not turn
# the loop's register operands back into repeatedly parsed immediates.
foreach(_hot_literal IN ITEMS "0\\.125d" "3\\.125d")
    if(_rxas MATCHES
            "(^|\n)[^\n]*(dadd|dsub|dmult|ddiv)[ \t]+[^\n]*${_hot_literal}")
        message(FATAL_ERROR
                "DECIMAL-01 optimizer integrity failure for ${CASE}: "
                "hot-loop decimal constant ${_hot_literal} was propagated "
                "into a repeatedly parsed literal operand in ${DISASSEMBLY}")
    endif()
endforeach()

if(NOT _source_rxas MATCHES "OPAQUE_SEED")
    message(FATAL_ERROR
            "DECIMAL-01 optimizer integrity failure for ${CASE}: "
            "runtime opaque-seed source metadata is absent from ${RXAS_SOURCE}")
endif()

message(STATUS
        "DECIMAL-01 optimizer integrity PASS: ${CASE} retains decimal "
        "parse/format, arithmetic and comparison instructions in final RXBIN")
