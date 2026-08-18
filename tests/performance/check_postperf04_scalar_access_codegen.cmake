if(NOT DEFINED OPT_IMAGE OR NOT DEFINED NOOPT_IMAGE OR
   NOT DEFINED OPT_BINARY OR NOT DEFINED RXDAS)
    message(FATAL_ERROR
            "OPT_IMAGE, NOOPT_IMAGE, OPT_BINARY and RXDAS are required")
endif()

file(READ "${OPT_IMAGE}" opt_rxas)
file(READ "${NOOPT_IMAGE}" noopt_rxas)

foreach(accessor IN ITEMS
        "scalarintbox\\.read\\(\\)"
        "scalarintbox\\.write\\(\\)"
        "scalarfloatbox\\.read\\(\\)"
        "scalarfloatbox\\.write\\(\\)")
    if(opt_rxas MATCHES "call[0-9]* [^\n]*${accessor}")
        message(FATAL_ERROR
                "Optimized POSTPERF-04 image retains exact scalar accessor ${accessor}")
    endif()
    if(NOT noopt_rxas MATCHES "call[0-9]* [^\n]*${accessor}")
        message(FATAL_ERROR
                "No-opt POSTPERF-04 control does not retain scalar accessor ${accessor}")
    endif()
endforeach()

get_filename_component(opt_binary_directory "${OPT_BINARY}" DIRECTORY)
set(opt_disassembly "${opt_binary_directory}/postperf04_scalar_access_compare_opt.dis.rxas")
execute_process(
        COMMAND "${RXDAS}" -o "${opt_disassembly}" "${OPT_BINARY}"
        RESULT_VARIABLE rxdas_rc
        OUTPUT_VARIABLE rxdas_out
        ERROR_VARIABLE rxdas_err
)
if(NOT rxdas_rc EQUAL 0)
    message(FATAL_ERROR
            "POSTPERF-04 optimized disassembly failed\n"
            "stdout:\n${rxdas_out}\nstderr:\n${rxdas_err}")
endif()
file(READ "${opt_disassembly}" opt_rxbin_rxas)
string(FIND "${opt_rxbin_rxas}" "\nmain()" main_start)
string(FIND "${opt_rxbin_rxas}" "\nelapsedus()" main_end)
if(main_start LESS 0 OR main_end LESS 0 OR main_end LESS_EQUAL main_start)
    message(FATAL_ERROR
            "Could not isolate main() in optimized POSTPERF-04 disassembly")
endif()
math(EXPR main_length "${main_end} - ${main_start}")
string(SUBSTRING "${opt_rxbin_rxas}" ${main_start} ${main_length} main_rxas)
string(REGEX MATCHALL "(^|\n)[ \t]*assertinitialized[ \t]+"
        main_initialization_guards "${main_rxas}")
list(LENGTH main_initialization_guards main_initialization_guard_count)
if(NOT main_initialization_guard_count EQUAL 1)
    message(FATAL_ERROR
            "Optimized POSTPERF-04 main() must retain only the computed-receiver factory-call guard; found ${main_initialization_guard_count}")
endif()

foreach(control IN ITEMS
        "packedi64probe\\.read\\(\\)"
        "packedi64probe\\.write\\(\\)"
        "packedf32probe\\.read\\(\\)"
        "packedf32probe\\.write\\(\\)")
    if(NOT opt_rxas MATCHES "call[0-9]* [^\n]*${control}")
        message(FATAL_ERROR
                "Optimized POSTPERF-04 image unexpectedly rewrote packed control ${control}")
    endif()
endforeach()
