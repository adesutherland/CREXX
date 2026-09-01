file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")

set(expected "3\n日🙂\na🙂🙂\nBeta\né日\n233\n")

function(run_checked label)
    execute_process(
        COMMAND ${ARGN}
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE res
        ENCODING UTF-8)
    if(NOT res EQUAL 0)
        message(FATAL_ERROR "${label} failed: ${out}${err}")
    endif()
endfunction()

function(run_dual_vm label rxbin)
    foreach(vm RXVM RXBVM)
        execute_process(
            COMMAND "${${vm}}" "${rxbin}"
            OUTPUT_VARIABLE run_out
            ERROR_VARIABLE err
            RESULT_VARIABLE res
            ENCODING UTF-8)
        string(REPLACE "\r\n" "\n" run_out "${run_out}")
        if(NOT res EQUAL 0 OR NOT run_out STREQUAL expected)
            message(FATAL_ERROR
                "${label} ${vm} mismatch: expected [${expected}], got [${run_out}], stderr [${err}]")
        endif()
    endforeach()
endfunction()

run_checked("optimized user-body compile"
    "${RXC}" --no-exe-import -o "${WORK_DIR}/selected_opt" "${SELECTED_SOURCE}")
file(READ "${WORK_DIR}/selected_opt.rxas" opt_rxas)
if(opt_rxas MATCHES "[ \t]call[0-9]*[ \t]" OR
   opt_rxas MATCHES "[ \t](strlen|substring|strchar|padstr|fndnblnk|fndblnk)[ \t]" OR
   opt_rxas MATCHES "^user_(length|substr|word|char)\\(\\)" OR
   NOT opt_rxas MATCHES "selected\\.crexx")
    message(FATAL_ERROR
        "optimized ordinary user bodies did not reach the literal-result ceiling:\n${opt_rxas}")
endif()
run_checked("optimized user-body assemble"
    "${RXAS}" -o "${WORK_DIR}/selected_opt.rxbin" "${WORK_DIR}/selected_opt.rxas")
run_dual_vm("optimized user body" "${WORK_DIR}/selected_opt.rxbin")

run_checked("no-opt user-body compile"
    "${RXC}" --no-exe-import -n -o "${WORK_DIR}/selected_noopt" "${SELECTED_SOURCE}")
file(READ "${WORK_DIR}/selected_noopt.rxas" noopt_rxas)
foreach(proc user_length user_substr user_word user_char)
    if(NOT noopt_rxas MATCHES "call[0-9]* .*${proc}\\(\\)" OR
       NOT noopt_rxas MATCHES "${proc}\\(\\) \\.locals=")
        message(FATAL_ERROR
            "no-opt build did not retain ordinary ${proc} call/body:\n${noopt_rxas}")
    endif()
endforeach()

set(array_expected "TEMP:seed\n")
foreach(mode opt noopt)
    if(mode STREQUAL "noopt")
        set(mode_args -n)
    else()
        set(mode_args)
    endif()
    run_checked("${mode} array-result compile"
        "${RXC}" --no-exe-import ${mode_args}
        -o "${WORK_DIR}/array_${mode}" "${ARRAY_SOURCE}")
    run_checked("${mode} array-result assemble"
        "${RXAS}" -o "${WORK_DIR}/array_${mode}.rxbin"
        "${WORK_DIR}/array_${mode}.rxas")
    foreach(vm RXVM RXBVM)
        execute_process(
            COMMAND "${${vm}}" "${WORK_DIR}/array_${mode}.rxbin"
            OUTPUT_VARIABLE array_out
            ERROR_VARIABLE array_err
            RESULT_VARIABLE array_res
            ENCODING UTF-8)
        string(REPLACE "\r\n" "\n" array_out "${array_out}")
        if(NOT array_res EQUAL 0 OR NOT array_out STREQUAL array_expected)
            message(FATAL_ERROR
                "${mode} array-result ${vm}: expected [${array_expected}], got [${array_out}], stderr [${array_err}]")
        endif()
    endforeach()
endforeach()
run_checked("no-opt user-body assemble"
    "${RXAS}" -o "${WORK_DIR}/selected_noopt.rxbin" "${WORK_DIR}/selected_noopt.rxas")
run_dual_vm("no-opt user body" "${WORK_DIR}/selected_noopt.rxbin")

file(SIZE "${WORK_DIR}/selected_opt.rxas" opt_size)
file(SIZE "${WORK_DIR}/selected_noopt.rxas" noopt_size)
if(NOT opt_size LESS noopt_size)
    message(FATAL_ERROR
        "optimized user-body RXAS did not shrink: opt=${opt_size}, noopt=${noopt_size}")
endif()

set(cursor_expected "1\n4\n")
foreach(mode opt noopt)
    if(mode STREQUAL "noopt")
        set(mode_args -n)
    else()
        set(mode_args)
    endif()
    run_checked("${mode} cursor-inline compile"
        "${RXC}" --no-exe-import ${mode_args}
        -o "${WORK_DIR}/cursor_${mode}" "${CURSOR_SOURCE}")
    file(READ "${WORK_DIR}/cursor_${mode}.rxas" cursor_rxas)
    if(mode STREQUAL "opt")
        if(cursor_rxas MATCHES "call[0-9]* .*scan_blank\\(\\)" OR
           NOT cursor_rxas MATCHES "[ \t]fndblnk[ \t]")
            message(FATAL_ERROR
                "optimized explicit-position helper was not safely inlined:\n${cursor_rxas}")
        endif()
    else()
        if(NOT cursor_rxas MATCHES "call[0-9]* .*scan_blank\\(\\)")
            message(FATAL_ERROR
                "no-opt explicit-position helper lost its normal call:\n${cursor_rxas}")
        endif()
    endif()
    run_checked("${mode} cursor-inline assemble"
        "${RXAS}" -o "${WORK_DIR}/cursor_${mode}.rxbin"
        "${WORK_DIR}/cursor_${mode}.rxas")
    foreach(vm RXVM RXBVM)
        execute_process(
            COMMAND "${${vm}}" "${WORK_DIR}/cursor_${mode}.rxbin"
            OUTPUT_VARIABLE cursor_out
            ERROR_VARIABLE cursor_err
            RESULT_VARIABLE cursor_res)
        string(REPLACE "\r\n" "\n" cursor_out "${cursor_out}")
        if(NOT cursor_res EQUAL 0 OR NOT cursor_out STREQUAL cursor_expected)
            message(FATAL_ERROR
                "${mode} explicit-position ${vm}: expected [${cursor_expected}], got [${cursor_out}], stderr [${cursor_err}]")
        endif()
    endforeach()
endforeach()
