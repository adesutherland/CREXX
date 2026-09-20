file(MAKE_DIRECTORY "${WORK}")
set(cases
    tests/rxas_optimizer/entry_alias_runtime.rxas
    tests/rxas_optimizer/copy_acopy.rxas
    interpreter/tests/tests_signal_nested_pending.rxas
    interpreter/tests/tests_signal_branch_value.rxas
    interpreter/tests/tests_signal_mask_bounds.rxas
    interpreter/tests/tests_signal_pushpop.rxas
    interpreter/tests/tests_signal_call_unwind.rxas)
foreach(input IN LISTS cases)
    get_filename_component(name "${input}" NAME_WE)
    foreach(mode IN ITEMS optimized unoptimized)
        set(flags)
        if(mode STREQUAL unoptimized)
            set(flags -n)
        endif()
        set(output "${WORK}/${name}-${mode}")
        execute_process(COMMAND "${RXAS}" ${flags} -o "${output}" "${SOURCE}/${input}"
            RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE err)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "RXAS ${input}: ${result}: ${out}${err}")
        endif()
        execute_process(COMMAND "${REFERENCE}" "${output}.rxbin"
            RESULT_VARIABLE expected_rc OUTPUT_VARIABLE expected ERROR_VARIABLE expected_err)
        execute_process(COMMAND "${VM}" "${output}.rxbin"
            RESULT_VARIABLE actual_rc OUTPUT_VARIABLE actual ERROR_VARIABLE actual_err)
        file(WRITE "${output}.log" "Reference rc=${expected_rc}\n${expected}${expected_err}\nConstrained rc=${actual_rc}\n${actual}${actual_err}")
        if(NOT expected_rc EQUAL 0 OR NOT actual_rc EQUAL 0 OR
           NOT actual STREQUAL expected OR NOT actual_err STREQUAL expected_err)
            message(FATAL_ERROR "VM parity failed: ${input} ${mode}; see ${output}.log")
        endif()
    endforeach()
endforeach()
foreach(name IN ITEMS unavailable unhandled)
    execute_process(COMMAND "${RXAS}" -o "${WORK}/${name}" "${SOURCE}/ports/single-threaded/${name}.rxas"
        RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE err)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "RXAS ${name}: ${out}${err}")
    endif()
    execute_process(COMMAND "${VM}" "${WORK}/${name}.rxbin"
        RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE err)
    file(WRITE "${WORK}/${name}.log" "rc=${result}\n${out}${err}")
    if(name STREQUAL unavailable)
        if(NOT result EQUAL 0 OR NOT out MATCHES "PASS: 33 unavailable operations caught")
            message(FATAL_ERROR "Unavailable operation was not caught: ${result}: ${out}${err}")
        endif()
    elseif(NOT result EQUAL 12 OR NOT "${out}${err}" MATCHES "not supported in this build")
        message(FATAL_ERROR "Unhandled unavailable operation succeeded or lost diagnostic: ${result}: ${out}${err}")
    endif()
endforeach()
execute_process(COMMAND "${VM}" --rxvm-process-worker absent absent
    RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT result EQUAL 2 OR NOT err MATCHES "process workers are not supported")
    message(FATAL_ERROR "Worker entry did not reject: ${result}: ${out}${err}")
endif()

execute_process(COMMAND "${VM}" -p absent "${WORK}/entry_alias_runtime-optimized.rxbin"
    RESULT_VARIABLE result OUTPUT_VARIABLE out ERROR_VARIABLE err)
file(WRITE "${WORK}/dynamic-plugin.log" "rc=${result}\n${out}${err}")
if(result EQUAL 0 OR NOT "${out}${err}" MATCHES "dynamic plugins are not supported")
    message(FATAL_ERROR "Dynamic plugin did not reject: ${result}: ${out}${err}")
endif()
message(STATUS "PASS: 14 reference comparisons, 33 caught operations, unhandled operation, worker-entry and dynamic-plugin controls")
