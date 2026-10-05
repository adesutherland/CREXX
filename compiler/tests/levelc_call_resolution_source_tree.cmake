foreach(required RXC BINDIR SOURCE BUILD_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()

set(log "${BUILD_DIR}/levelc_call_resolution_source_tree.log")
execute_process(
        COMMAND "${RXC}" -d1 -i "${BINDIR}" -o levelc_call_resolution_source_tree "${SOURCE}"
        WORKING_DIRECTORY "${BUILD_DIR}"
        OUTPUT_FILE "${log}"
        ERROR_FILE "${log}"
        RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "CALL source tree compile failed; see ${log}")
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

foreach(shape IN ITEMS
        [=[LITERAL : "length"]=]
        [=[LITERAL : ".foo"]=]
        [=[LITERAL : "..foo"]=]
        [=[LITERAL : ".5abc"]=]
        [=[LITERAL : "."]=]
        [=[LITERAL : ".RESULT"]=]
        [=[STRING : "\'LENGTH\'"]=]
        [=[ARGS : ""]=]
        [=[NOVAL : ""]=])
    string(FIND "${raw}" "${shape}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "CALL source tree lost ${shape}; see ${log}")
    endif()
endforeach()
foreach(shape IN ITEMS
        [=[MEMBER_CALL : "applyCallResult"]=]
        [=[MEMBER_CALL : "hasReturnValue"]=]
        [=[FRAME_LABEL : ".foo:"]=]
        [=[FRAME_LABEL : "..foo:"]=]
        [=[FRAME_LABEL : ".5abc:"]=]
        [=[FRAME_LABEL : ".:"]=]
        [=[FRAME_LABEL : ".RESULT:"]=]
        [=[FUNCTION : "rexxclassicbiflength.rexxclassicbif_length"]=])
    string(FIND "${lowered}" "${shape}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "CALL canonical tree lost ${shape}; see ${log}")
    endif()
endforeach()
