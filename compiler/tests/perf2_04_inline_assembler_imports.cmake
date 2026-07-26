file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")

set(expected "LITERAL\nMIXED ÄÖ\näöüé\nAB\nMiXeD äÖ\n")

function(run_checked label)
    execute_process(
        COMMAND ${ARGN}
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE res)
    if(NOT res EQUAL 0)
        message(FATAL_ERROR "${label} failed: ${out}${err}")
    endif()
endfunction()

function(check_main_rxas label path optimized)
    file(READ "${path}" rxas_text)
    if(optimized)
        if(rxas_text MATCHES "call[^\n]*perf2case\\.(caseupper|caselower)\\(\\)")
            message(FATAL_ERROR "${label} retained an imported helper call:\n${rxas_text}")
        endif()
        string(REGEX MATCHALL "strupper[ \t]+r[0-9]+,r[0-9]+" upper_ops "${rxas_text}")
        list(LENGTH upper_ops upper_count)
        string(REGEX MATCHALL "strlower[ \t]+r[0-9]+,r[0-9]+" lower_ops "${rxas_text}")
        list(LENGTH lower_ops lower_count)
        if(NOT upper_count EQUAL 3 OR NOT lower_count EQUAL 1)
            message(FATAL_ERROR "${label} did not reconstruct four imported assembler primitives:\n${rxas_text}")
        endif()
        if(rxas_text MATCHES "[ \t](scopy|null)[ \t]")
            message(FATAL_ERROR "${label} retained non-overlap formal/result scaffolding:\n${rxas_text}")
        endif()
    else()
        if(NOT rxas_text MATCHES "call[^\n]*perf2case\.caseupper\\(\\)" OR
           NOT rxas_text MATCHES "call[^\n]*perf2case\.caselower\\(\\)")
            message(FATAL_ERROR "${label} no-opt build did not retain normal-call fallback:\n${rxas_text}")
        endif()
    endif()
endfunction()

function(run_dual_vm label dep_rxbin main_rxbin)
    foreach(vm RXVM RXBVM)
        execute_process(
            COMMAND "${${vm}}" "${dep_rxbin}" "${main_rxbin}"
            OUTPUT_VARIABLE run_out
            ERROR_VARIABLE err
            RESULT_VARIABLE res)
        string(REPLACE "\r\n" "\n" run_out "${run_out}")
        if(NOT res EQUAL 0 OR NOT run_out STREQUAL expected)
            message(FATAL_ERROR "${label} ${vm} mismatch: expected [${expected}], got [${run_out}], stderr [${err}]")
        endif()
    endforeach()
endfunction()

foreach(mode opt noopt)
    set(mode_dir "${WORK_DIR}/${mode}")
    file(MAKE_DIRECTORY "${mode_dir}")
    if(mode STREQUAL "noopt")
        set(mode_args -n)
        set(optimized FALSE)
    else()
        set(mode_args)
        set(optimized TRUE)
    endif()

    run_checked("${mode} dependency compile"
        "${RXC}" --no-exe-import ${mode_args}
        -o "${mode_dir}/case_helpers" "${DEP_SOURCE}")
    run_checked("${mode} dependency assemble"
        "${RXAS}" -o "${mode_dir}/case_helpers.rxbin"
        "${mode_dir}/case_helpers.rxas")

    file(READ "${mode_dir}/case_helpers.rxas" dep_rxas)
    if(NOT dep_rxas MATCHES "strupper[ \t]+r[0-9]+,a1" OR
       NOT dep_rxas MATCHES "strlower[ \t]+r[0-9]+,a1")
        message(FATAL_ERROR "${mode} dependency did not share its classified read-only input:\n${dep_rxas}")
    endif()

    run_checked("${mode} source-import compile"
        "${RXC}" --no-exe-import ${mode_args} -s "${DEP_DIR}"
        -o "${mode_dir}/source_main" "${MAIN_SOURCE}")
    check_main_rxas("${mode} source import"
        "${mode_dir}/source_main.rxas" ${optimized})
    run_checked("${mode} source-import assemble"
        "${RXAS}" -o "${mode_dir}/source_main.rxbin"
        "${mode_dir}/source_main.rxas")
    run_dual_vm("${mode} source import"
        "${mode_dir}/case_helpers.rxbin" "${mode_dir}/source_main.rxbin")

    run_checked("${mode} binary-import compile"
        "${RXC}" --no-exe-import ${mode_args} -i "${mode_dir}"
        -o "${mode_dir}/binary_main" "${MAIN_SOURCE}")
    check_main_rxas("${mode} binary import"
        "${mode_dir}/binary_main.rxas" ${optimized})
    run_checked("${mode} binary-import assemble"
        "${RXAS}" -o "${mode_dir}/binary_main.rxbin"
        "${mode_dir}/binary_main.rxas")
    run_dual_vm("${mode} binary import"
        "${mode_dir}/case_helpers.rxbin" "${mode_dir}/binary_main.rxbin")
endforeach()
