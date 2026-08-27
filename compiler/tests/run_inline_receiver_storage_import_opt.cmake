file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

foreach(import_mode IN ITEMS source binary)
    set(cell "${WORK}/${import_mode}")
    file(MAKE_DIRECTORY "${cell}")
    file(COPY "${DEP_SOURCE}" DESTINATION "${cell}")
    file(COPY "${MAIN_SOURCE}" DESTINATION "${cell}")

    execute_process(
            COMMAND "${RXC}" -i "${IMPORT_DIR}" -o inline_receiver_storage_dep
                    "${cell}/inline_receiver_storage_dep.crexx"
            WORKING_DIRECTORY "${cell}"
            OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "rxc failed on ${import_mode} receiver-storage dependency: ${out}${err}")
    endif()
    file(READ "${cell}/inline_receiver_storage_dep.rxas" dep_rxas)

    foreach(callable IN ITEMS currenttext classat setclass nameat setname tokentextat byteat setbyte)
        if(NOT dep_rxas MATCHES
           "\\.meta \"inline_receiver_storage_dep\\.store\\.${callable}\"=\"\\.inline\" \"I7;")
            message(FATAL_ERROR
                    "${import_mode} dependency lacks I7 receiver-storage metadata for ${callable}:\n${dep_rxas}")
        endif()
    endforeach()
    foreach(blocked IN ITEMS replaceclasses binaryvalue)
        if(dep_rxas MATCHES
           "\\.meta \"inline_receiver_storage_dep\\.store\\.${blocked}\"=\"\\.inline\"")
            message(FATAL_ERROR
                    "${import_mode} dependency unexpectedly exports ${blocked}:\n${dep_rxas}")
        endif()
    endforeach()

    execute_process(
            COMMAND "${RXAS}" -o inline_receiver_storage_dep.rxbin
                    inline_receiver_storage_dep
            WORKING_DIRECTORY "${cell}"
            OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE result)
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "rxas failed on ${import_mode} receiver-storage dependency: ${out}${err}")
    endif()

    if(import_mode STREQUAL "binary")
        execute_process(
                COMMAND "${RXDAS}" -o inline_receiver_storage_dep.roundtrip.rxas
                        inline_receiver_storage_dep.rxbin
                WORKING_DIRECTORY "${cell}"
                OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE result)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "rxdas failed on receiver-storage dependency: ${out}${err}")
        endif()
        file(READ "${cell}/inline_receiver_storage_dep.roundtrip.rxas" roundtrip)
        foreach(callable IN ITEMS currenttext classat setclass nameat setname tokentextat byteat setbyte)
            if(NOT roundtrip MATCHES
               "\\.meta \"inline_receiver_storage_dep\\.store\\.${callable}\"=\"\\.inline\" \"I7;")
                message(FATAL_ERROR "RXDAS roundtrip lost I7 metadata for ${callable}")
            endif()
        endforeach()
        file(REMOVE "${cell}/inline_receiver_storage_dep.crexx"
                    "${cell}/inline_receiver_storage_dep.rxas"
                    "${cell}/inline_receiver_storage_dep.roundtrip.rxas")
    endif()

    foreach(opt_mode IN ITEMS opt noopt)
        if(opt_mode STREQUAL "opt")
            set(opt_args)
        else()
            set(opt_args -n)
        endif()
        execute_process(
                COMMAND "${RXC}" -i "${cell}" -i "${IMPORT_DIR}" ${opt_args}
                        -o "inline_receiver_storage_main_${opt_mode}"
                        "${cell}/inline_receiver_storage_main.crexx"
                WORKING_DIRECTORY "${cell}"
                OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE result)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR
                    "rxc failed on ${import_mode}/${opt_mode} receiver-storage main: ${out}${err}")
        endif()
        file(READ "${cell}/inline_receiver_storage_main_${opt_mode}.rxas" main_rxas)

        foreach(callable IN ITEMS currenttext classat setclass nameat setname tokentextat byteat setbyte)
            if(opt_mode STREQUAL "opt" AND main_rxas MATCHES
               "call[0-9]* [^\n]*inline_receiver_storage_dep\\.store\\.${callable}\\(\\)")
                message(FATAL_ERROR
                        "${import_mode} optimized import retained ${callable}():\n${main_rxas}")
            elseif(opt_mode STREQUAL "noopt" AND NOT main_rxas MATCHES
                   "call[0-9]* [^\n]*inline_receiver_storage_dep\\.store\\.${callable}\\(\\)")
                message(FATAL_ERROR
                        "${import_mode} no-opt import lost ${callable}() control call:\n${main_rxas}")
            endif()
        endforeach()
        foreach(blocked IN ITEMS replaceclasses binaryvalue)
            if(NOT main_rxas MATCHES
               "call[0-9]* [^\n]*inline_receiver_storage_dep\\.store\\.${blocked}\\(\\)")
                message(FATAL_ERROR
                        "${import_mode}/${opt_mode} unexpectedly removed negative-control ${blocked}():\n${main_rxas}")
            endif()
        endforeach()
        if(opt_mode STREQUAL "opt" AND NOT main_rxas MATCHES
           "call[0-9]* [^\n]*inline_receiver_storage_dep\\.token\\.runtimetext\\(\\)")
            message(FATAL_ERROR
                    "${import_mode} optimized forwarding lost residual token.runtimeText() call:\n${main_rxas}")
        endif()

        execute_process(
                COMMAND "${RXAS}" -o "inline_receiver_storage_main_${opt_mode}.rxbin"
                        "inline_receiver_storage_main_${opt_mode}"
                WORKING_DIRECTORY "${cell}"
                OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE result)
        if(NOT result EQUAL 0)
            message(FATAL_ERROR
                    "rxas failed on ${import_mode}/${opt_mode} receiver-storage main: ${out}${err}")
        endif()
        foreach(runner IN ITEMS "${RXVM}" "${RXBVM}")
            execute_process(
                    COMMAND "${runner}" "${LIBRARY}"
                            "${cell}/inline_receiver_storage_dep.rxbin"
                            "${cell}/inline_receiver_storage_main_${opt_mode}.rxbin"
                    WORKING_DIRECTORY "${cell}"
                    OUTPUT_VARIABLE run_out ERROR_VARIABLE err RESULT_VARIABLE result)
            string(REPLACE "\r\n" "\n" run_out "${run_out}")
            if(NOT result EQUAL 0 OR
               NOT run_out STREQUAL "PASS: imported receiver storage\n")
                message(FATAL_ERROR
                        "${import_mode}/${opt_mode} runtime mismatch for ${runner}: ${run_out}${err}")
            endif()
        endforeach()
    endforeach()
endforeach()
