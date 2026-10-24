foreach(required RUNNER BINARY ORIGIN_BINARY)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()

# Unknown input must be rejected before any signal body runs. This independently
# prevents a broken argv extraction from silently exercising a default mode.
execute_process(COMMAND "${RUNNER}" --no-autoload "${BINARY}" -a invalid-diagnostic-mode
    RESULT_VARIABLE status OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr TIMEOUT 20)
string(REPLACE "\r\n" "\n" stdout "${stdout}")
if(NOT "${status}" STREQUAL "64" OR NOT "${stdout}" STREQUAL "invalid-mode REJECTED\n" OR
   NOT "${stderr}" STREQUAL "")
    message(FATAL_ERROR "mode routing did not reject unknown input: status=${status}, stdout=[${stdout}], stderr=[${stderr}]")
endif()

function(run_case mode status stdout)
    execute_process(COMMAND "${RUNNER}" --no-autoload "${BINARY}" -a "${mode}"
        RESULT_VARIABLE actual_status OUTPUT_VARIABLE actual_stdout
        ERROR_VARIABLE diagnostic TIMEOUT 20)
    string(REPLACE "\r\n" "\n" actual_stdout "${actual_stdout}")
    if(NOT "${actual_status}" STREQUAL "${status}" OR
       NOT "${actual_stdout}" STREQUAL "${stdout}\n")
        message(FATAL_ERROR "${mode}: status=${actual_status}, stdout=[${actual_stdout}], stderr=[${diagnostic}]")
    endif()
    string(REGEX MATCHALL "PANIC:" panics "${diagnostic}")
    list(LENGTH panics panic_count)
    if(NOT panic_count EQUAL 1)
        message(FATAL_ERROR "${mode}: expected one terminal PANIC: [${diagnostic}]")
    endif()
    foreach(pattern IN LISTS ARGN)
        if(NOT diagnostic MATCHES "${pattern}")
            message(FATAL_ERROR "${mode}: missing ${pattern}: [${diagnostic}]")
        endif()
    endforeach()
    if(diagnostic MATCHES "B-call\\.rexx|cause-B\\.rexx|rethrow-site\\.rexx|empty-rethrow\\.rexx|nested-rethrow\\.rexx")
        message(FATAL_ERROR "${mode}: unrelated later source: [${diagnostic}]")
    endif()
    if(mode STREQUAL "fresh" AND diagnostic MATCHES "A-call\\.rexx|cause-A\\.rexx")
        message(FATAL_ERROR "fresh event retained an ignored relay's context: [${diagnostic}]")
    endif()
    if(mode STREQUAL "empty" AND diagnostic MATCHES "called from:")
        message(FATAL_ERROR "captured-empty event acquired a rethrow caller: [${diagnostic}]")
    endif()
    if(mode STREQUAL "pseudo")
        string(REGEX MATCHALL "called from:" callers "${diagnostic}")
        list(LENGTH callers caller_count)
        if(NOT caller_count EQUAL 1 OR
           diagnostic MATCHES "handler-call\\.rexx|handler-cause\\.rexx")
            message(FATAL_ERROR "signal handler introduced a pseudo-call edge: [${diagnostic}]")
        endif()
    endif()
endfunction()

run_case(stored 30 "stored-event PASS"
    "SIGNAL OTHER" "source: cause-A\\.rexx:31:1:" "source: A-call\\.rexx:11:2:")
run_case(alias 30 "stored-event PASS"
    "SIGNAL OTHER" "source: cause-A\\.rexx:31:1:" "source: A-call\\.rexx:11:2:")
run_case(callstored 30 "call-event PASS"
    "SIGNAL OTHER" "source: cause-A\\.rexx:31:1:" "source: call-handler-A\\.rexx:91:2:")
run_case(nested 30 "nested-event PASS"
    "SIGNAL OTHER" "source: cause-A\\.rexx:31:1:"
    "source: nested-inner\\.rexx:42:3:" "source: nested-outer\\.rexx:41:2:")
run_case(fresh 30 "fresh-event PASS" "PANIC: fresh" "source: fresh-origin\\.rexx:71:3:")
run_case(empty 30 "empty-event PASS" "PANIC: empty" "source: empty-origin\\.rexx:5:1:")
run_case(action 30 "action-event PASS" "SIGNAL OTHER" "source: action-call\\.rexx:51:2:")
run_case(pseudo 30 "pseudo-event PASS" "PANIC: initial interruption"
    "source: pseudo-edge\\.rexx:99:1:" "source: pseudo-call\\.rexx:98:2:")

# Existing typed/NUL/object/operand-alias assertions must remain silent when caught.
execute_process(COMMAND "${RUNNER}" --no-autoload "${ORIGIN_BINARY}"
    RESULT_VARIABLE status OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr TIMEOUT 20)
string(REPLACE "\r\n" "\n" stdout "${stdout}")
if(NOT "${status}" STREQUAL "0" OR NOT "${stderr}" STREQUAL "" OR
   NOT "${stdout}" STREQUAL "invalid-origin-rejected\ninvalid-name-rejected\nPASS: Signal Origin Payload\n")
    message(FATAL_ERROR "caught raw events changed: status=${status}, stdout=[${stdout}], stderr=[${stderr}]")
endif()
