foreach(required RXC BINDIR SOURCE BUILD_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()

set(log "${BUILD_DIR}/levelc_call_expression_source_tree.log")
execute_process(
        COMMAND "${RXC}" -d1 -i "${BINDIR}" -o levelc_call_expression_source_tree "${SOURCE}"
        WORKING_DIRECTORY "${BUILD_DIR}"
        OUTPUT_FILE "${log}"
        ERROR_FILE "${log}"
        RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "CALL expression tree compile failed; see ${log}")
endif()
file(READ "${log}" output)
string(FIND "${output}" "--- STAGE_RAW ---" raw_start)
string(FIND "${output}" "--- STAGE_LEVELC_LOWERED ---" lowered_start)
string(FIND "${output}" "--- STAGE_FIXUP ---" lowered_end)
if(raw_start EQUAL -1 OR lowered_start EQUAL -1 OR lowered_end EQUAL -1)
    message(FATAL_ERROR "CALL tree stages are missing; see ${log}")
endif()
math(EXPR raw_length "${lowered_start} - ${raw_start}")
math(EXPR lowered_length "${lowered_end} - ${lowered_start}")
string(SUBSTRING "${output}" ${raw_start} ${raw_length} raw)
string(SUBSTRING "${output}" ${lowered_start} ${lowered_length} lowered)

string(FIND "${raw}" [=[LITERAL : "relay"]=] relay_pos)
string(FIND "${raw}" [=[FUNCTION : "side"]=] side_pos)
string(FIND "${raw}" [=[NOVAL :]=] omission_pos)
string(FIND "${raw}" [=[LITERAL : "edges"]=] edges_pos)
string(FIND "${raw}" [=[LITERAL : "empty"]=] empty_pos)
string(FIND "${raw}" [=[TOKEN :]=] raw_token_pos)
if(relay_pos EQUAL -1 OR side_pos EQUAL -1 OR
   omission_pos EQUAL -1 OR edges_pos EQUAL -1 OR
   empty_pos LESS edges_pos OR NOT raw_token_pos EQUAL -1)
    message(FATAL_ERROR "CALL source tree lost expression or omitted-slot structure; see ${log}")
endif()
string(FIND "${lowered}" __rxcp_levelc_proc_RELAY procedure_pos)
string(FIND "${lowered}" RexxActivationArguments frame_pos)
string(FIND "${lowered}" [=[MEMBER_CALL : "append"]=] append_pos)
if(procedure_pos EQUAL -1 OR frame_pos EQUAL -1 OR append_pos EQUAL -1)
    message(FATAL_ERROR "CALL canonical tree lost local frame lowering; see ${log}")
endif()
