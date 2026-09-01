file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

# The ordinary (non-inlined) method body must also consume register.0.binary
# directly. If it regresses to the historical detached temporary, these
# instructions name a local register rather than receiver argument a1.
set(noopt "${WORK}/noopt")
file(MAKE_DIRECTORY "${noopt}")
execute_process(
        COMMAND "${RXC}" -i "${IMPORT_DIR}" -n -o inline_packed_accessors_dep
                "${DEP_SOURCE}"
        WORKING_DIRECTORY "${noopt}"
        OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "rxc failed on no-opt packed-accessor dependency: ${out}${err}")
endif()
file(READ "${noopt}/inline_packed_accessors_dep.rxas" noopt_image)
foreach(shape IN ITEMS
        "psetf a1,a2,a3"
        "pgetf r[0-9]+,a1,a2"
        "pseti a1,a2,a3"
        "pgeti r[0-9]+,a1,a2")
    if(NOT noopt_image MATCHES "${shape}")
        message(FATAL_ERROR
                "No-opt packed accessor did not use receiver storage directly (${shape}):\n${noopt_image}")
    endif()
endforeach()

foreach(mode IN ITEMS source binary)
    set(cell "${WORK}/${mode}")
    file(MAKE_DIRECTORY "${cell}")
    execute_process(
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${DEP_SOURCE}"
                    "${cell}/inline_packed_accessors_dep.crexx"
            RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Failed to stage ${mode} packed-accessor dependency")
    endif()
    execute_process(
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${MAIN_SOURCE}"
                    "${cell}/inline_packed_accessors_main.crexx"
            RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "Failed to stage ${mode} packed-accessor main")
    endif()

    if(mode STREQUAL "binary")
        execute_process(
                COMMAND "${RXC}" -i "${IMPORT_DIR}" -o inline_packed_accessors_dep
                        "${cell}/inline_packed_accessors_dep.crexx"
                WORKING_DIRECTORY "${cell}"
                OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE result)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "rxc failed on binary packed-accessor dependency: ${out}${err}")
        endif()
        execute_process(
                COMMAND "${RXAS}" -o inline_packed_accessors_dep.rxbin
                        inline_packed_accessors_dep
                WORKING_DIRECTORY "${cell}"
                OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE result)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "rxas failed on binary packed-accessor dependency: ${out}${err}")
        endif()
        execute_process(
                COMMAND "${RXDAS}" -o inline_packed_accessors_dep.roundtrip.rxas
                        inline_packed_accessors_dep.rxbin
                WORKING_DIRECTORY "${cell}"
                OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE result)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "rxdas failed on packed-accessor dependency: ${out}${err}")
        endif()
        file(READ "${cell}/inline_packed_accessors_dep.roundtrip.rxas" roundtrip)
        foreach(accessor IN ITEMS readfloat writefloat readint writeint)
            string(FIND "${roundtrip}"
                   ".meta \"inline_packed_accessors_dep.packedbox.${accessor}\"=\".inline\" \"I6;c,1,"
                   inline_position)
            if(inline_position EQUAL -1)
                message(FATAL_ERROR
                        "RXDAS roundtrip lost packed-accessor inline metadata for ${accessor}")
            endif()
        endforeach()
        execute_process(
                COMMAND "${RXAS}" -o inline_packed_accessors_dep.roundtrip.rxbin
                        inline_packed_accessors_dep.roundtrip.rxas
                WORKING_DIRECTORY "${cell}"
                OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE result)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "rxas failed on round-tripped packed-accessor dependency: ${out}${err}")
        endif()
        file(REMOVE "${cell}/inline_packed_accessors_dep.crexx"
                    "${cell}/inline_packed_accessors_dep.rxas"
                    "${cell}/inline_packed_accessors_dep.rxbin"
                    "${cell}/inline_packed_accessors_dep.roundtrip.rxas")
        file(RENAME "${cell}/inline_packed_accessors_dep.roundtrip.rxbin"
                    "${cell}/inline_packed_accessors_dep.rxbin")
    endif()

    execute_process(
            COMMAND "${RXC}" -i "${cell}" -i "${IMPORT_DIR}"
                    -o inline_packed_accessors_main
                    "${cell}/inline_packed_accessors_main.crexx"
            WORKING_DIRECTORY "${cell}"
            OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "rxc failed on ${mode} packed-accessor main: ${out}${err}")
    endif()

    file(READ "${cell}/inline_packed_accessors_main.rxas" image)
    foreach(accessor IN ITEMS readfloat writefloat readint writeint)
        if(image MATCHES
           "call[0-9]* [^\n]*inline_packed_accessors_dep\\.packedbox\\.${accessor}\\(\\)")
            message(FATAL_ERROR
                    "${mode} imported exact packed accessor ${accessor} remained a call:\n${image}")
        endif()
    endforeach()
    foreach(opcode IN ITEMS pgetf psetf pgeti pseti)
        if(NOT image MATCHES "${opcode} ")
            message(FATAL_ERROR
                    "${mode} imported packed accessor did not lower to ${opcode}:\n${image}")
        endif()
    endforeach()
    foreach(copy_or_borrow IN ITEMS bcopy link unlink)
        if(image MATCHES "[\n ]${copy_or_borrow} ")
            message(FATAL_ERROR
                    "${mode} imported packed accessor retained ${copy_or_borrow}:\n${image}")
        endif()
    endforeach()
    if(image MATCHES "pget[fi][^\n]*\n[ ]*[fi]copy " OR
       image MATCHES "pget[fi][^\n]*\n[ ]*br l[0-9]+bexprend")
        message(FATAL_ERROR
                "${mode} imported packed getter retained scalar handoff work:\n${image}")
    endif()
    string(REGEX MATCHALL "assertinitialized " receiver_asserts "${image}")
    list(LENGTH receiver_asserts receiver_assert_count)
    if(NOT receiver_assert_count EQUAL 1)
        message(FATAL_ERROR
                "${mode} expected one guard for only the uninitialized negative control, got ${receiver_assert_count}:\n${image}")
    endif()

    execute_process(
            COMMAND "${RXAS}" -o inline_packed_accessors_main.rxbin
                    inline_packed_accessors_main
            WORKING_DIRECTORY "${cell}"
            OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "rxas failed on ${mode} packed-accessor main: ${out}${err}")
    endif()

    if(mode STREQUAL "source")
        execute_process(
                COMMAND "${RXC}" -i "${IMPORT_DIR}" -o inline_packed_accessors_dep
                        "${cell}/inline_packed_accessors_dep.crexx"
                WORKING_DIRECTORY "${cell}"
                OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE result)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "rxc failed on source packed-accessor dependency: ${out}${err}")
        endif()
        execute_process(
                COMMAND "${RXAS}" -o inline_packed_accessors_dep.rxbin
                        inline_packed_accessors_dep
                WORKING_DIRECTORY "${cell}"
                OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE result)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "rxas failed on source packed-accessor dependency: ${out}${err}")
        endif()
    endif()

    foreach(runner IN ITEMS "${RXVM}" "${RXBVM}")
        execute_process(
                COMMAND "${runner}" "${LIBRARY}"
                        "${cell}/inline_packed_accessors_dep.rxbin"
                        "${cell}/inline_packed_accessors_main.rxbin"
                WORKING_DIRECTORY "${cell}"
                OUTPUT_VARIABLE run_out ERROR_VARIABLE err RESULT_VARIABLE result)
        string(REPLACE "\r\n" "\n" run_out "${run_out}")
        if(NOT result EQUAL 0 OR
           NOT run_out STREQUAL "PASS: exact packed accessor imports\n")
            message(FATAL_ERROR
                    "${mode} packed-accessor runtime mismatch for ${runner}: ${run_out}${err}")
        endif()
    endforeach()
endforeach()
