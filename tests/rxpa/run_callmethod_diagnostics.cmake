foreach(required RUNNER BINARY PROVIDER LIBRARY)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()
foreach(mode caught terminal replacement)
    set(arguments)
    if(NOT mode STREQUAL "caught")
        set(arguments -a "${mode}")
    endif()
    execute_process(COMMAND "${RUNNER}" --no-autoload "${BINARY}" "${PROVIDER}" "${LIBRARY}" ${arguments}
        RESULT_VARIABLE status OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr TIMEOUT 20)
    string(REPLACE "\r\n" "\n" stdout "${stdout}")
    if(mode STREQUAL "caught")
        if(NOT "${status}" STREQUAL "0" OR NOT "${stderr}" STREQUAL "" OR
           NOT "${stdout}" STREQUAL "PASS: RXPA CALLMETHOD invokes a cREXX object method\n")
            message(FATAL_ERROR "recovered callback changed: status=${status}, stdout=[${stdout}], stderr=[${stderr}]")
        endif()
    else()
        string(REGEX MATCHALL "PANIC:" panics "${stderr}")
        list(LENGTH panics count)
        if(NOT count EQUAL 1)
            message(FATAL_ERROR "${mode}: expected one terminal panic: [${stderr}]")
        endif()
        if(mode STREQUAL "terminal")
            # Callback status/message/origin policy stays the existing outer one.
            # The source chain independently identifies its real inner method call.
            if(NOT "${status}" STREQUAL "5" OR NOT "${stdout}" STREQUAL "CALLBACK_TERMINAL\n" OR
               NOT stderr MATCHES "PANIC: CALLMETHOD failed \\(SIGNAL DIVISION_BY_ZERO\\)" OR
               NOT stderr MATCHES "source: [^\n]*rxpa_callmethod\\.crexx:[0-9]+:[0-9]+: [^\n]*ignored = invoke_failure\\(receiver,0\\)" OR
               NOT stderr MATCHES "source: [^\n]*rxpa_callmethod\\.crexx:[0-9]+:[0-9]+: [^\n]*return innerfailure\\(value\\)")
                message(FATAL_ERROR "callback diagnostic context missing or public policy changed: status=${status}, stdout=[${stdout}], stderr=[${stderr}]")
            endif()
        elseif(NOT "${status}" STREQUAL "30" OR NOT "${stdout}" STREQUAL "CALLBACK_REPLACEMENT\n" OR
               NOT stderr MATCHES "PANIC: provider replacement \\(SIGNAL OTHER\\)" OR
               stderr MATCHES "return innerfailure|return 1 / value")
            message(FATAL_ERROR "provider setters retained callback context: status=${status}, stdout=[${stdout}], stderr=[${stderr}]")
        endif()
    endif()
endforeach()
