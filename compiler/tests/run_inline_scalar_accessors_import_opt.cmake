file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

foreach(mode IN ITEMS source binary)
    set(cell "${WORK}/${mode}")
    file(MAKE_DIRECTORY "${cell}")
    execute_process(
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${DEP_SOURCE}"
                    "${cell}/inline_scalar_accessors_dep.crexx"
            RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Failed to stage ${mode} scalar-accessor dependency")
    endif()
    execute_process(
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${MAIN_SOURCE}"
                    "${cell}/inline_scalar_accessors_main.crexx"
            RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Failed to stage ${mode} scalar-accessor main")
    endif()

    if(mode STREQUAL "binary")
        execute_process(
                COMMAND "${RXC}" -i "${IMPORT_DIR}" -o inline_scalar_accessors_dep
                        "${cell}/inline_scalar_accessors_dep.crexx"
                WORKING_DIRECTORY "${cell}"
                OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE result)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "rxc failed on binary scalar-accessor dependency: ${out}${err}")
        endif()
        execute_process(
                COMMAND "${RXAS}" -o inline_scalar_accessors_dep.rxbin
                        inline_scalar_accessors_dep
                WORKING_DIRECTORY "${cell}"
                OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE result)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "rxas failed on binary scalar-accessor dependency: ${out}${err}")
        endif()
        file(REMOVE "${cell}/inline_scalar_accessors_dep.crexx"
                    "${cell}/inline_scalar_accessors_dep.rxas")
    endif()

    execute_process(
            COMMAND "${RXC}" -i "${cell}" -i "${IMPORT_DIR}"
                    -o inline_scalar_accessors_main
                    "${cell}/inline_scalar_accessors_main.crexx"
            WORKING_DIRECTORY "${cell}"
            OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "rxc failed on ${mode} scalar-accessor main: ${out}${err}")
    endif()

    file(READ "${cell}/inline_scalar_accessors_main.rxas" image)
    foreach(accessor IN ITEMS
            "intbox\\.write\\(\\)"
            "floatbox\\.read\\(\\)"
            "floatbox\\.write\\(\\)")
        if(image MATCHES "call[0-9]* [^\n]*inline_scalar_accessors_dep\\.${accessor}")
            message(FATAL_ERROR
                    "${mode} imported exact scalar accessor ${accessor} remained a call:\n${image}")
        endif()
    endforeach()

    execute_process(
            COMMAND "${RXAS}" -o inline_scalar_accessors_main.rxbin
                    inline_scalar_accessors_main
            WORKING_DIRECTORY "${cell}"
            OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "rxas failed on ${mode} scalar-accessor main: ${out}${err}")
    endif()

    if(mode STREQUAL "source")
        execute_process(
                COMMAND "${RXC}" -i "${IMPORT_DIR}" -o inline_scalar_accessors_dep
                        "${cell}/inline_scalar_accessors_dep.crexx"
                WORKING_DIRECTORY "${cell}"
                OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE result)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "rxc failed on source scalar-accessor dependency: ${out}${err}")
        endif()
        execute_process(
                COMMAND "${RXAS}" -o inline_scalar_accessors_dep.rxbin
                        inline_scalar_accessors_dep
                WORKING_DIRECTORY "${cell}"
                OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE result)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "rxas failed on source scalar-accessor dependency: ${out}${err}")
        endif()
    endif()

    foreach(runner IN ITEMS "${RXVM}" "${RXBVM}")
        execute_process(
                COMMAND "${runner}" "${LIBRARY}"
                        "${cell}/inline_scalar_accessors_dep.rxbin"
                        "${cell}/inline_scalar_accessors_main.rxbin"
                WORKING_DIRECTORY "${cell}"
                OUTPUT_VARIABLE run_out ERROR_VARIABLE err RESULT_VARIABLE result)
        string(REPLACE "\r\n" "\n" run_out "${run_out}")
        if(NOT result EQUAL 0 OR
           NOT run_out STREQUAL "PASS: exact scalar accessor imports\n")
            message(FATAL_ERROR
                    "${mode} scalar-accessor runtime mismatch for ${runner}: ${run_out}${err}")
        endif()
    endforeach()
endforeach()
