file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

foreach(mode IN ITEMS opt noopt)
    if(mode STREQUAL "noopt")
        set(opt_args -n)
    else()
        set(opt_args)
    endif()

    execute_process(
            COMMAND "${RXC}" -i "${IMPORT_DIR}" ${opt_args}
                    -o "inline_receiver_signal_cleanup_${mode}" "${SOURCE}"
            WORKING_DIRECTORY "${WORK}"
            OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "rxc ${mode} failed:\n${out}${err}")
    endif()

    set(base "${WORK}/inline_receiver_signal_cleanup_${mode}")
    file(READ "${base}.rxas" compiler_rxas)
    if(mode STREQUAL "opt")
        if(NOT compiler_rxas MATCHES "linkattr1 r[0-9]+,[^\n]+" OR
           NOT compiler_rxas MATCHES
           "signalhandler0:[\n\r]+[ \t]+sigpop[^\n\r]*[\n\r]+[ \t]+unlink r[0-9]+")
            message(FATAL_ERROR
                    "optimized SIGNAL handler did not retain transient receiver unlink:\n${compiler_rxas}")
        endif()
        if(NOT compiler_rxas MATCHES
           "sigpop \"OTHER\"[^\n\r]*[\n\r]+[ \t]+unlink r[0-9]+")
            message(FATAL_ERROR
                    "optimized live-receiver SIGNAL handler lost its unlink:\n${compiler_rxas}")
        endif()
        if(compiler_rxas MATCHES
           "call[0-9]* [^\n]*§inline_receiver_signal_cleanup\.receivertoken\.mutateandsignal\(\)")
            message(FATAL_ERROR
                    "optimized regression retained ReceiverToken.mutateAndSignal():\n${compiler_rxas}")
        endif()
        if(compiler_rxas MATCHES
           "call[0-9]* [^\n]*§rexx\.rexx\.abbrev\(\)")
            message(FATAL_ERROR
                    "optimized regression retained the Rexx.abbrev() wrapper call:\n${compiler_rxas}")
        endif()
        if(NOT compiler_rxas MATCHES
           "call[0-9]* [^\n]*rxfnsb\.abbrev\(\)")
            message(FATAL_ERROR
                    "optimized regression lost the deliberate residual BIF call:\n${compiler_rxas}")
        endif()
    else()
        if(NOT compiler_rxas MATCHES
           "call[0-9]* [^\n]*§inline_receiver_signal_cleanup\.receivertoken\.mutateandsignal\(\)")
            message(FATAL_ERROR
                    "no-opt control unexpectedly removed ReceiverToken.mutateAndSignal():\n${compiler_rxas}")
        endif()
        if(NOT compiler_rxas MATCHES
           "call[0-9]* [^\n]*§rexx\.rexx\.abbrev\(\)")
            message(FATAL_ERROR
                    "no-opt control unexpectedly removed Rexx.abbrev():\n${compiler_rxas}")
        endif()
    endif()

    execute_process(
            COMMAND "${RXAS}" -o "${base}.rxbin" "${base}"
            WORKING_DIRECTORY "${WORK}"
            OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "rxas ${mode} failed:\n${out}${err}")
    endif()

    execute_process(
            COMMAND "${RXDAS}" -o "${base}.dis.rxas" "${base}.rxbin"
            WORKING_DIRECTORY "${WORK}"
            OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "rxdas ${mode} failed:\n${out}${err}")
    endif()
    if(mode STREQUAL "opt")
        file(READ "${base}.dis.rxas" final_rxas)
        if(NOT final_rxas MATCHES "linkattr1 r[0-9]+,[^\n]+" OR
           NOT final_rxas MATCHES
           "sigpop \"INVALID_ARGUMENTS\"[^\n\r]*[\n\r]+[ \t]+unlink r[0-9]+")
            message(FATAL_ERROR
                    "assembled optimized image lost transient receiver SIGNAL cleanup:\n${final_rxas}")
        endif()
        if(NOT final_rxas MATCHES
           "sigpop \"OTHER\"[^\n\r]*[\n\r]+[ \t]+unlink r[0-9]+")
            message(FATAL_ERROR
                    "assembled optimized image lost live-receiver SIGNAL cleanup:\n${final_rxas}")
        endif()
    endif()

    execute_process(
            COMMAND "${RXLINK}" -s -o "${WORK}/linked_${mode}"
                    "${base}.rxbin" "${LIBRARY}" "${CLASSLIB}"
            WORKING_DIRECTORY "${WORK}"
            OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "rxlink ${mode} failed:\n${out}${err}")
    endif()

    foreach(runner IN ITEMS "${RXTVM}" "${RXBVM}")
        execute_process(
                COMMAND "${runner}" "${WORK}/linked_${mode}.rxbin"
                WORKING_DIRECTORY "${WORK}"
                OUTPUT_VARIABLE run_out ERROR_VARIABLE err RESULT_VARIABLE rc)
        string(REPLACE "\r\n" "\n" run_out "${run_out}")
        if(NOT rc EQUAL 0 OR
           NOT run_out STREQUAL "PASS: inline receiver signal cleanup\n")
            message(FATAL_ERROR
                    "${mode} runtime mismatch for ${runner}: ${run_out}${err}")
        endif()
    endforeach()
endforeach()
