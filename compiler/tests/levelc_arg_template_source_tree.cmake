foreach(required RXC BINDIR SOURCE_DIR BUILD_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()

foreach(case IN ITEMS instruction patterns)
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
    else()
        if(NOT output MATCHES "LEVELC_ARG : \"arg\"[^\n]*\n[ ]+TEMPLATES :[^\n]*\n[ ]+TEMPLATES :[^\n]*\n[ ]+TEMPLATES :[^\n]*\n[ ]+TARGET : \"pair_first\"")
            message(FATAL_ERROR "ARG , pair_first lost its leading empty template; see ${log}")
        endif()
        if(NOT output MATCHES "TARGET : \"first\"[^\n]*\n[ ]+TEMPLATES :[^\n]*\n[ ]+SAY :")
            message(FATAL_ERROR "ARG first, lost its trailing empty template; see ${log}")
        endif()
    endif()
endforeach()
