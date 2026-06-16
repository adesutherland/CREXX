if(NOT DEFINED RXC)
    message(FATAL_ERROR "RXC is required")
endif()

if(NOT DEFINED RXAS)
    message(FATAL_ERROR "RXAS is required")
endif()

if(NOT DEFINED RXVM)
    message(FATAL_ERROR "RXVM is required")
endif()

if(NOT DEFINED BIN_DIR)
    message(FATAL_ERROR "BIN_DIR is required")
endif()

if(NOT DEFINED LIBRARY)
    message(FATAL_ERROR "LIBRARY is required")
endif()

if(NOT DEFINED WORK)
    message(FATAL_ERROR "WORK is required")
endif()

if(NOT DEFINED RXVM_TIMEOUT)
    set(RXVM_TIMEOUT 10)
endif()

file(MAKE_DIRECTORY "${WORK}")

function(run_stem_trace_case NAME TRACE_MODE EXPECTED)
    set(SOURCE "${WORK}/${NAME}.crexx")
    set(OUT_BASE "${WORK}/${NAME}")
    set(RXBIN "${WORK}/${NAME}.rxbin")

    file(WRITE "${SOURCE}" "options levelb\nimport rxfnsb\n\nmain: procedure\n  trace ${TRACE_MODE}\n  x = .stem()\n  x.blah = \"sss\"\n  x.tail1.tail2 = \"ttt\"\n  say x.blah\n  say x.tail1.tail2\n  return\n")

    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E env "RXCP_EXIT_MODULE=rxcexits" "${RXC}" -i "${BIN_DIR}" -n -o "${OUT_BASE}" "${SOURCE}"
        WORKING_DIRECTORY "${WORK}"
        RESULT_VARIABLE RXC_RESULT
        OUTPUT_VARIABLE RXC_OUT
        ERROR_VARIABLE RXC_ERR)

    if(NOT RXC_RESULT EQUAL 0)
        message(FATAL_ERROR "rxc failed for ${NAME}.\nstdout:\n${RXC_OUT}\nstderr:\n${RXC_ERR}")
    endif()

    execute_process(
        COMMAND "${RXAS}" -o "${RXBIN}" "${OUT_BASE}"
        WORKING_DIRECTORY "${WORK}"
        RESULT_VARIABLE RXAS_RESULT
        OUTPUT_VARIABLE RXAS_OUT
        ERROR_VARIABLE RXAS_ERR)

    if(NOT RXAS_RESULT EQUAL 0)
        message(FATAL_ERROR "rxas failed for ${NAME}.\nstdout:\n${RXAS_OUT}\nstderr:\n${RXAS_ERR}")
    endif()

    execute_process(
        COMMAND "${RXVM}" "${LIBRARY}" "${RXBIN}"
        WORKING_DIRECTORY "${WORK}"
        TIMEOUT "${RXVM_TIMEOUT}"
        RESULT_VARIABLE RXVM_RESULT
        OUTPUT_VARIABLE RXVM_OUT
        ERROR_VARIABLE RXVM_ERR)

    if(NOT RXVM_RESULT EQUAL 0)
        message(FATAL_ERROR "rxvm failed for ${NAME} with ${RXVM_RESULT}.\nstdout:\n${RXVM_OUT}\nstderr:\n${RXVM_ERR}")
    endif()

    string(REPLACE "\r\n" "\n" RXVM_OUT "${RXVM_OUT}")
    if(NOT RXVM_OUT STREQUAL EXPECTED)
        message(FATAL_ERROR "TRACE output mismatch for ${NAME}.\nExpected:\n${EXPECTED}\nActual:\n${RXVM_OUT}")
    endif()
endfunction()

set(EXPECTED_R [=[     6 *-*   x = .stem()
       >=>   ""
     7 *-*   x.blah = \"sss\"
       >=>   "sss"
     8 *-*   x.tail1.tail2 = \"ttt\"
       >=>   "ttt"
     9 *-*   say x.blah
       >V>   "sss"
sss
    10 *-*   say x.tail1.tail2
       >V>   "ttt"
ttt
    11 *-*   return
]=])

set(EXPECTED_I [=[     6 *-*   x = .stem()
       >F>   ""
     7 *-*   x.blah = \"sss\"
       >C>   "blah"
       >L>   "sss"
       >=>   "sss"
     8 *-*   x.tail1.tail2 = \"ttt\"
       >C>   "tail1.tail2"
       >L>   "ttt"
       >=>   "ttt"
     9 *-*   say x.blah
       >C>   "blah"
       >V>   "sss"
sss
    10 *-*   say x.tail1.tail2
       >C>   "tail1.tail2"
       >V>   "ttt"
ttt
    11 *-*   return
]=])

run_stem_trace_case(trace_stem_sugar_r "results" "${EXPECTED_R}")
run_stem_trace_case(trace_stem_sugar_i "intermediates" "${EXPECTED_I}")
