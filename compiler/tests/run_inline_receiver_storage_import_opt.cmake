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

    if(NOT dep_rxas MATCHES
       "inline_receiver_storage_dep\\.store\\.mutatecurrentthroughforwarder\"=\"\\.inline\" \"I7;c,1,1,1,0,4,9,")
        message(FATAL_ERROR
                "${import_mode} dependency lost transitive receiver-mutation control metadata")
    endif()
    if(NOT dep_rxas MATCHES
       "73657474657874,2E766F6964,[^;]*,m,1")
        message(FATAL_ERROR
                "${import_mode} dependency lost residual setText receiver-write evidence")
    endif()

    foreach(callable IN ITEMS currenttext mutatecurrentthroughforwarder classat setclass setclassandreport nameat setname tokentextat byteat setbyte)
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
        foreach(callable IN ITEMS currenttext mutatecurrentthroughforwarder classat setclass setclassandreport nameat setname tokentextat byteat setbyte)
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

        if(opt_mode STREQUAL "opt")
            string(REGEX MATCHALL "\n[ \t]+copy " optimized_value_copies "${main_rxas}")
            list(LENGTH optimized_value_copies optimized_value_copy_count)
            if(optimized_value_copy_count GREATER 7)
                message(FATAL_ERROR
                        "${import_mode} optimized receiver storage introduced an avoidable full-value copy: copies=${optimized_value_copy_count}, expected at most 7")
            endif()

            string(REGEX MATCHALL "\n[ \t]+mkref " receiver_reference_captures "${main_rxas}")
            string(REGEX MATCHALL "\n[ \t]+linkref " receiver_live_links "${main_rxas}")
            string(REGEX MATCHALL
                   "\.traceevent \"[A-Z]\" [^\n]* \"I\" [^\n]*\"__inline_receiver_ref_[^\"]*\""
                   integer_receiver_captures "${main_rxas}")
            list(LENGTH receiver_reference_captures receiver_reference_count)
            list(LENGTH receiver_live_links receiver_live_link_count)
            list(LENGTH integer_receiver_captures integer_receiver_capture_count)
            if(NOT receiver_reference_count EQUAL 4 OR
               NOT receiver_live_link_count EQUAL 4 OR
               integer_receiver_capture_count LESS 4)
                message(FATAL_ERROR
                        "${import_mode} optimized computed receivers did not retain four integer evaluate-once live storage links: mkref=${receiver_reference_count}, linkref=${receiver_live_link_count}, integer captures=${integer_receiver_capture_count}")
            endif()

            string(FIND "${main_rxas}"
                    "if stores[coerced_index].classAt(1)"
                    coerced_receiver_start)
            if(coerced_receiver_start LESS 0)
                message(FATAL_ERROR
                        "${import_mode} optimized coerced-index receiver section is missing")
            endif()
            string(SUBSTRING "${main_rxas}" ${coerced_receiver_start} -1 coerced_receiver_section)
            string(FIND "${coerced_receiver_section}" "\n   ftoi " coerced_ftoi)
            if(coerced_ftoi GREATER_EQUAL 0)
                string(SUBSTRING "${coerced_receiver_section}" ${coerced_ftoi} -1 coerced_after_ftoi)
                string(FIND "${coerced_after_ftoi}" "\n   icopy " coerced_icopy)
                string(FIND "${coerced_after_ftoi}" "\n   fcopy " coerced_fcopy)
            else()
                set(coerced_icopy -1)
                set(coerced_fcopy -1)
            endif()
            if(coerced_ftoi LESS 0 OR coerced_icopy LESS 0 OR
               (coerced_fcopy GREATER_EQUAL 0 AND coerced_fcopy LESS coerced_icopy))
                message(FATAL_ERROR
                        "${import_mode} optimized coerced-index receiver did not capture the evaluated integer value:\n${coerced_receiver_section}")
            endif()

            string(FIND "${main_rxas}"
                    "§inline_receiver_storage_main.holder.mutateandread() .locals="
                    holder_start)
            string(FIND "${main_rxas}"
                    "/* Imported Declaration from file: inline_receiver_storage_dep"
                    holder_end)
            if(holder_start LESS 0 OR holder_end LESS 0 OR holder_end LESS_EQUAL holder_start)
                message(FATAL_ERROR
                        "${import_mode} optimized class-attribute receiver section is missing")
            endif()
            math(EXPR holder_length "${holder_end} - ${holder_start}")
            string(SUBSTRING "${main_rxas}" ${holder_start} ${holder_length} holder_section)
            if(holder_section MATCHES "\n[ \t]+copy ")
                message(FATAL_ERROR
                        "${import_mode} optimized class-attribute receiver was deep-copied instead of live-linked:\n${holder_section}")
            endif()
            string(REGEX MATCH "linkattr1 [^\n]*,a1,1" holder_live_link "${holder_section}")
            string(FIND "${holder_section}"
                    "inline_receiver_storage_dep.token.settext()"
                    holder_mutator)
            if(holder_mutator GREATER_EQUAL 0)
                string(SUBSTRING "${holder_section}" ${holder_mutator} -1 holder_after_mutator)
                string(FIND "${holder_after_mutator}" "\n   unlink " holder_unlink)
            else()
                set(holder_unlink -1)
            endif()
            if(NOT holder_live_link OR holder_mutator LESS 0 OR holder_unlink LESS 0)
                message(FATAL_ERROR
                        "${import_mode} optimized class-attribute receiver lost its live link across the residual mutator:\n${holder_section}")
            endif()
        endif()

        foreach(callable IN ITEMS currenttext mutatecurrentthroughforwarder classat setclass setclassandreport nameat setname tokentextat byteat setbyte)
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
