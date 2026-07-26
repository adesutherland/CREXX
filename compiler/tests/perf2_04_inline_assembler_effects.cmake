file(MAKE_DIRECTORY "${WORK_DIR}")

set(expected "LITERAL\nMIXED ÄÖ\näöüé\nAB\nMiXeD äÖ\n")

foreach(mode opt noopt)
    set(base "${WORK_DIR}/perf2_04_inline_assembler_effects_${mode}")
    if(mode STREQUAL "noopt")
        set(mode_args -n)
    else()
        set(mode_args)
    endif()

    execute_process(
        COMMAND "${RXC}" -i "${BIN_DIR}" ${mode_args} -o "${base}" "${SOURCE}"
        WORKING_DIRECTORY "${WORK_DIR}"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE res)
    if(NOT res EQUAL 0)
        message(FATAL_ERROR "rxc ${mode} failed: ${out}${err}")
    endif()

    file(READ "${base}.rxas" rxas_text)
    if(mode STREQUAL "opt")
        if(rxas_text MATCHES "strupper[ \t]+r[0-9]+,[^r]" OR
           rxas_text MATCHES "strlower[ \t]+r[0-9]+,[^r]")
            message(FATAL_ERROR "optimized inline helpers emitted a non-local-register case operand:\n${rxas_text}")
        endif()
        if(rxas_text MATCHES "call[^\n]*caseupper\\(\\)" OR
           rxas_text MATCHES "call[^\n]*caselower\\(\\)")
            message(FATAL_ERROR "optimized generic case helpers retained a call:\n${rxas_text}")
        endif()
        string(REGEX MATCHALL "strupper[ \t]+r[0-9]+,r[0-9]+" upper_ops "${rxas_text}")
        list(LENGTH upper_ops upper_count)
        string(REGEX MATCHALL "strlower[ \t]+r[0-9]+,r[0-9]+" lower_ops "${rxas_text}")
        list(LENGTH lower_ops lower_count)
        if(NOT upper_count EQUAL 3 OR NOT lower_count EQUAL 1)
            message(FATAL_ERROR "optimized user helpers did not lower to the expected four register primitives:\n${rxas_text}")
        endif()
        if(rxas_text MATCHES "[ \t]scopy[ \t]")
            message(FATAL_ERROR "optimized owned case-helper returns retained a string copy:\n${rxas_text}")
        endif()
        if(rxas_text MATCHES "[ \t]null[ \t]")
            message(FATAL_ERROR "optimized owned case-helper results retained a default initialization:\n${rxas_text}")
        endif()
    else()
        if(NOT rxas_text MATCHES "strupper[ \t]+r[0-9]+,a1" OR
           NOT rxas_text MATCHES "strlower[ \t]+r[0-9]+,a1")
            message(FATAL_ERROR "no-opt normal-call bodies did not share the proved read-only input register:\n${rxas_text}")
        endif()
    endif()

    execute_process(
        COMMAND "${RXAS}" -o "${base}.rxbin" "${base}.rxas"
        WORKING_DIRECTORY "${WORK_DIR}"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE res)
    if(NOT res EQUAL 0)
        message(FATAL_ERROR "rxas ${mode} failed: ${out}${err}")
    endif()

    foreach(vm RXVM RXBVM)
        execute_process(
            COMMAND "${${vm}}" "${BIN_DIR}/library.rxbin" "${base}.rxbin"
            WORKING_DIRECTORY "${WORK_DIR}"
            OUTPUT_VARIABLE run_out
            ERROR_VARIABLE err
            RESULT_VARIABLE res)
        string(REPLACE "\r\n" "\n" run_out "${run_out}")
        if(NOT res EQUAL 0 OR NOT run_out STREQUAL expected)
            message(FATAL_ERROR "${vm} ${mode} mismatch: expected [${expected}], got [${run_out}], stderr [${err}]")
        endif()
    endforeach()
endforeach()
