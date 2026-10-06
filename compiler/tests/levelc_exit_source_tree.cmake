foreach(required RXC BINDIR SOURCE BUILD_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()

set(log "${BUILD_DIR}/levelc_exit_source_tree.log")
execute_process(
        COMMAND "${RXC}" -d1 -i "${BINDIR}" -o levelc_exit_source_tree "${SOURCE}"
        WORKING_DIRECTORY "${BUILD_DIR}"
        OUTPUT_FILE "${log}"
        ERROR_FILE "${log}"
        RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "EXIT tree compile failed; see ${log}")
endif()
file(READ "${log}" output)
string(FIND "${output}" "--- STAGE_RAW ---" raw_start)
string(FIND "${output}" "--- STAGE_LEVELC_LOWERED ---" lowered_start)
string(FIND "${output}" "--- STAGE_FIXUP ---" lowered_end)
if(raw_start EQUAL -1 OR lowered_start EQUAL -1 OR lowered_end EQUAL -1)
    message(FATAL_ERROR "EXIT tree stages are missing; see ${log}")
endif()
math(EXPR raw_length "${lowered_start} - ${raw_start}")
math(EXPR lowered_length "${lowered_end} - ${lowered_start}")
string(SUBSTRING "${output}" ${raw_start} ${raw_length} raw)
string(SUBSTRING "${output}" ${lowered_start} ${lowered_length} lowered)
string(REGEX MATCHALL "EXIT : \"exit\"" raw_exits "${raw}")
list(LENGTH raw_exits raw_exit_count)
if(NOT raw_exit_count EQUAL 10)
    message(FATAL_ERROR "EXIT source tree lost a bare, valued or nested clause; see ${log}")
endif()
string(REGEX MATCHALL "MEMBER_CALL : \"requestProgramExit\"" lowered_exits "${lowered}")
list(LENGTH lowered_exits lowered_exit_count)
if(NOT lowered_exit_count EQUAL raw_exit_count)
    message(FATAL_ERROR "EXIT canonical tree lost a completion request; see ${log}")
endif()
foreach(shape IN ITEMS
        [=[IF : "if"]=]
        [=[SELECT : "select"]=]
        [=[FUNCTION : "stop"]=]
        [=[LABEL : "private:"]=])
    string(FIND "${raw}" "${shape}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "EXIT source tree lost ${shape}; see ${log}")
    endif()
endforeach()
foreach(shape IN ITEMS
        [=[MEMBER_CALL : "programExitRequested"]=]
        [=[RETURN : "return"]=])
    string(FIND "${lowered}" "${shape}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "EXIT canonical tree lost ${shape}; see ${log}")
    endif()
endforeach()
