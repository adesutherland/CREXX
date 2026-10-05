foreach(required RXC BINDIR SOURCE BUILD_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()

set(log "${BUILD_DIR}/levelc_procedure_source_tree.log")
execute_process(
        COMMAND "${RXC}" -d1 -i "${BINDIR}" -o levelc_procedure_source_tree "${SOURCE}"
        WORKING_DIRECTORY "${BUILD_DIR}"
        OUTPUT_FILE "${log}"
        ERROR_FILE "${log}"
        RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "PROCEDURE tree compile failed; see ${log}")
endif()
file(READ "${log}" output)
string(FIND "${output}" "--- STAGE_RAW ---" raw_start)
string(FIND "${output}" "--- STAGE_LEVELC_LOWERED ---" lowered_start)
string(FIND "${output}" "--- STAGE_FIXUP ---" lowered_end)
if(raw_start EQUAL -1 OR lowered_start EQUAL -1 OR lowered_end EQUAL -1)
    message(FATAL_ERROR "PROCEDURE tree stages are missing; see ${log}")
endif()
math(EXPR raw_length "${lowered_start} - ${raw_start}")
math(EXPR lowered_length "${lowered_end} - ${lowered_start}")
string(SUBSTRING "${output}" ${raw_start} ${raw_length} raw)
string(SUBSTRING "${output}" ${lowered_start} ${lowered_length} lowered)

string(FIND "${raw}" [=[LEVELC_PROCEDURE : "procedure"]=] procedure_pos)
string(FIND "${raw}" [=[VAR_TARGET : "a.key"]=] compound_pos)
string(FIND "${raw}" [=[VAR_REFERENCE : "list"]=] indirect_pos)
string(FIND "${lowered}" [=[MEMBER_CALL : "exposeSymbol"]=] direct_call_pos)
string(FIND "${lowered}" [=[MEMBER_CALL : "exposeIndirect"]=] indirect_call_pos)
string(FIND "${lowered}" "LEVELC_PROCEDURE" leaked_procedure_pos)
string(FIND "${lowered}" "VAR_REFERENCE" leaked_reference_pos)
if(procedure_pos EQUAL -1 OR compound_pos EQUAL -1 OR indirect_pos EQUAL -1 OR
   direct_call_pos EQUAL -1 OR indirect_call_pos LESS direct_call_pos OR
   NOT leaked_procedure_pos EQUAL -1 OR NOT leaked_reference_pos EQUAL -1)
    message(FATAL_ERROR "PROCEDURE source/canonical tree lost its ordered EXPOSE contract; see ${log}")
endif()
