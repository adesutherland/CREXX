foreach(required RUNNER CONSUMER PROVIDER)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()

# The consumer declares a foreign procedure whose linked start is inside its
# own main body. The terminal snapshot must still keep main's local callsite.
execute_process(COMMAND "${RUNNER}" --no-autoload "${CONSUMER}" "${PROVIDER}"
    RESULT_VARIABLE status OUTPUT_VARIABLE stdout ERROR_VARIABLE diagnostic TIMEOUT 20)
string(REPLACE "\r\n" "\n" stdout "${stdout}")
if(NOT "${status}" STREQUAL "30" OR
   NOT "${stdout}" STREQUAL "import-bound PASS\n" OR
   NOT diagnostic MATCHES "SIGNAL OTHER" OR
   NOT diagnostic MATCHES "source: import-cause\\.rexx:13:1:" OR
   NOT diagnostic MATCHES "source: import-call\\.rexx:12:3:")
    message(FATAL_ERROR "imported start truncated a local caller: status=${status}, stdout=[${stdout}], stderr=[${diagnostic}]")
endif()
string(REGEX MATCHALL "PANIC:" panics "${diagnostic}")
list(LENGTH panics panic_count)
string(REGEX MATCHALL "called from:" callers "${diagnostic}")
list(LENGTH callers caller_count)
if(NOT panic_count EQUAL 1 OR NOT caller_count EQUAL 1 OR
   diagnostic MATCHES "source: .*diagforeign")
    message(FATAL_ERROR "imported start changed diagnostic chain: [${diagnostic}]")
endif()
