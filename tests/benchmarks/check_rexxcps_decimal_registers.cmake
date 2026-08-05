if(NOT DEFINED RXAS OR RXAS STREQUAL "")
    message(FATAL_ERROR "RXAS is required")
endif()
if(NOT EXISTS "${RXAS}")
    message(FATAL_ERROR "RexxCPS RXAS does not exist: ${RXAS}")
endif()

file(READ "${RXAS}" _rxas)

# RexxCPS deliberately prepares its Classic decimal constants once.  Literal
# decimal arithmetic would make the VM allocate, parse and clear a temporary
# value on every dispatch.
if(_rxas MATCHES
        "(^|\n)[^\n]*(dadd|dsub|dmult|ddiv)[ \t]+[^\n]*(1d|1\\.1d|2d|2\\.2d|5d|99\\.7d)")
    message(FATAL_ERROR
            "RexxCPS decimal register integrity failure: hot arithmetic uses "
            "a repeatedly parsed decimal immediate in ${RXAS}")
endif()

# The write-once positive BY value remains a register, but its sign is known.
# Do not regress to the general dynamic-sign loop test.
if(_rxas MATCHES "doneg1|doneg2")
    message(FATAL_ERROR
            "RexxCPS decimal loop integrity failure: immutable positive BY "
            "value was lowered through the dynamic-sign path in ${RXAS}")
endif()

if(NOT _rxas MATCHES "(^|\n)[ \t]*dadd[ \t]+r[0-9]+,r[0-9]+,r[0-9]+")
    message(FATAL_ERROR
            "RexxCPS decimal register integrity failure: no register/register "
            "decimal add remains in ${RXAS}")
endif()
