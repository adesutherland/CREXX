foreach(required RXC BINDIR SOURCE_DIR BUILD_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()

foreach(case IN ITEMS instruction patterns unicode)
    set(source "${SOURCE_DIR}/levelc_arg_${case}.rexx")
    set(log "${BUILD_DIR}/levelc_arg_${case}_source_tree.log")
    execute_process(
            COMMAND "${RXC}" -d1 -i "${BINDIR}" -o "levelc_arg_${case}_source_tree" "${source}"
            WORKING_DIRECTORY "${BUILD_DIR}"
            OUTPUT_FILE "${log}"
            ERROR_FILE "${log}"
            RESULT_VARIABLE result)
    file(READ "${log}" output)
    if(NOT output MATCHES "--- STAGE_RAW ---")
        message(FATAL_ERROR "${case} raw tree is missing (rxc ${result}); see ${log}")
    endif()
    if(case STREQUAL "instruction")
        if(NOT output MATCHES "TARGET : \"x\"[^\n]*\n[ ]+TEMPLATES :[^\n]*\n[ ]+TEMPLATES :[^\n]*\n[ ]+TARGET : \"z\"")
            message(FATAL_ERROR "ARG x,,z lost its empty middle template; see ${log}")
        endif()
    elseif(case STREQUAL "patterns")
        if(NOT output MATCHES "LEVELC_ARG : \"arg\"[^\n]*\n[ ]+TEMPLATES :[^\n]*\n[ ]+TEMPLATES :[^\n]*\n[ ]+TEMPLATES :[^\n]*\n[ ]+TARGET : \"pair_first\"")
            message(FATAL_ERROR "ARG , pair_first lost its leading empty template; see ${log}")
        endif()
        if(NOT output MATCHES "TARGET : \"first\"[^\n]*\n[ ]+TEMPLATES :[^\n]*\n[ ]+SAY :")
            message(FATAL_ERROR "ARG first, lost its trailing empty template; see ${log}")
        endif()
    else()
        string(FIND "${output}" "--- STAGE_LEVELC_LOWERED ---" lowered_start)
        string(FIND "${output}" "--- STAGE_FIXUP ---" lowered_end)
        if(lowered_start EQUAL -1 OR lowered_end EQUAL -1)
            message(FATAL_ERROR "Unicode ARG source or canonical tree is missing; see ${log}")
        endif()
        string(SUBSTRING "${output}" 0 ${lowered_start} raw)
        math(EXPR lowered_length "${lowered_end} - ${lowered_start}")
        string(SUBSTRING "${output}" ${lowered_start} ${lowered_length} lowered)
        string(FIND "${raw}" "LABEL : \"probe:\"" probe_start)
        string(FIND "${raw}" "LABEL : \"dot:\"" dot_start)
        string(FIND "${lowered}" "PROCEDURE : \"__rxcp_levelc_proc_PROBE:\"" lowered_probe_start)
        string(FIND "${lowered}" "PROCEDURE : \"__rxcp_levelc_proc_DOT:\"" lowered_dot_start)
        if(probe_start EQUAL -1 OR dot_start LESS probe_start OR
           lowered_probe_start EQUAL -1 OR lowered_dot_start LESS lowered_probe_start)
            message(FATAL_ERROR "Unicode ARG procedure ownership is missing; see ${log}")
        endif()
        math(EXPR raw_length "${dot_start} - ${probe_start}")
        string(SUBSTRING "${raw}" ${probe_start} ${raw_length} raw_probe)
        math(EXPR lowered_probe_length "${lowered_dot_start} - ${lowered_probe_start}")
        string(SUBSTRING "${lowered}" ${lowered_probe_start} ${lowered_probe_length} lowered_probe)
        if(NOT raw_probe MATCHES "IF : \"if\"" OR
           NOT raw_probe MATCHES "CALL : \"call\"")
            message(FATAL_ERROR "Nested source CALL is missing from probe; see ${log}")
        endif()
        if(NOT lowered_probe MATCHES "CALL : \"call\" \\(11:10\\)" OR
           NOT lowered_probe MATCHES "FUNCTION : \"__rxcp_levelc_proc_DOT\"" OR
           NOT lowered_probe MATCHES "VAR_SYMBOL : \"__rxcp_levelc_config_ref\"" OR
           NOT lowered_probe MATCHES "VAR_SYMBOL : \"__rxcp_levelc_call_activation_[0-9]+\"" OR
           lowered_probe MATCHES "LEVELC_ARG :")
            message(FATAL_ERROR "Nested canonical CALL frame or ARG lowering is invalid; see ${log}")
        endif()
    endif()
endforeach()
