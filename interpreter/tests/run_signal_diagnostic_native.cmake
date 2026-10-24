foreach(required RUNNER BINARY INITIALIZER)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()
execute_process(COMMAND "${RUNNER}" "${BINARY}" "${INITIALIZER}"
    RESULT_VARIABLE status OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr TIMEOUT 60)
string(REPLACE "\r\n" "\n" stdout "${stdout}")
string(REPLACE "\r\n" "\n" stderr "${stderr}")
if(NOT "${status}" STREQUAL "0" OR
   NOT "${stdout}" STREQUAL "PASS: signal diagnostic ownership, scoped initialization and worker reuse\n")
    message(FATAL_ERROR "native diagnostic lifecycle: status=${status}, stdout=[${stdout}], stderr=[${stderr}]")
endif()
if(INSTRUMENTED)
    # This dedicated fault variant accounts for its existing backend telemetry.
    # Empty-stderr proof belongs to the ordinary-library invocation above it.
    string(REPLACE "\n" ";" lines "${stderr}")
    foreach(line IN LISTS lines)
        if(NOT line STREQUAL "" AND NOT line MATCHES "^VM instrumentation: PASS begins=[0-9]+ retires=[0-9]+ terminals=[0-9]+ frames=[0-9]+ branches=[0-9]+ calls=[0-9]+ returns=[0-9]+ interrupts=[0-9]+/[0-9]+/[0-9]+/[0-9]+ polls=[0-9]+ scans=[0-9]+$")
            message(FATAL_ERROR "unexpected fault-variant diagnostic: [${stderr}]")
        endif()
    endforeach()
    execute_process(COMMAND "${RUNNER}" "${BINARY}" "${INITIALIZER}" unavailable
        RESULT_VARIABLE status OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr TIMEOUT 20)
    string(REPLACE "\r\n" "\n" stdout "${stdout}")
    if(NOT "${status}" STREQUAL "0" OR
       NOT "${stdout}" STREQUAL "PASS: unavailable diagnostic preserves original signal and recovery\n" OR
       NOT stderr MATCHES "PANIC: A \\(SIGNAL OTHER\\)" OR
       NOT stderr MATCHES "diagnostic callers unavailable \\(allocation or retain failure\\)" OR
       NOT stderr MATCHES "source: cause-A\\.rexx:31:1:")
        message(FATAL_ERROR "unavailable diagnostic replaced the event: status=${status}, stdout=[${stdout}], stderr=[${stderr}]")
    endif()
    string(REGEX MATCHALL "PANIC:" panics "${stderr}")
    list(LENGTH panics count)
    if(NOT count EQUAL 1 OR stderr MATCHES "AddressSanitizer|LeakSanitizer|runtime error:|FAIL:|unavailable-call\\.rexx")
        message(FATAL_ERROR "unavailable diagnostic leaked or obscured another diagnostic: [${stderr}]")
    endif()
elseif(NOT "${stderr}" STREQUAL "")
    message(FATAL_ERROR "handled/initializer/context/worker paths must stay silent: [${stderr}]")
endif()
