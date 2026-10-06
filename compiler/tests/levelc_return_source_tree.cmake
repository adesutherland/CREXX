foreach(required RXC BINDIR SOURCE BUILD_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()

set(log "${BUILD_DIR}/levelc_return_source_tree.log")
execute_process(
        COMMAND "${RXC}" -d1 -i "${BINDIR}" -o levelc_return_source_tree "${SOURCE}"
        WORKING_DIRECTORY "${BUILD_DIR}"
        OUTPUT_FILE "${log}"
        ERROR_FILE "${log}"
        RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "RETURN tree compile failed; see ${log}")
endif()
file(READ "${log}" output)
string(FIND "${output}" "--- STAGE_RAW ---" raw_start)
string(FIND "${output}" "--- STAGE_LEVELC_LOWERED ---" lowered_start)
string(FIND "${output}" "--- STAGE_FIXUP ---" lowered_end)
string(FIND "${output}" "--- STAGE_SYMBOLS" fixup_end)
if(raw_start EQUAL -1 OR lowered_start EQUAL -1 OR lowered_end EQUAL -1 OR fixup_end EQUAL -1)
    message(FATAL_ERROR "RETURN tree stages are missing; see ${log}")
endif()
math(EXPR raw_length "${lowered_start} - ${raw_start}")
math(EXPR lowered_length "${lowered_end} - ${lowered_start}")
math(EXPR fixup_length "${fixup_end} - ${lowered_end}")
string(SUBSTRING "${output}" ${raw_start} ${raw_length} raw)
string(SUBSTRING "${output}" ${lowered_start} ${lowered_length} lowered)
string(SUBSTRING "${output}" ${lowered_end} ${fixup_length} fixup)

string(REGEX MATCHALL "RETURN : \"return\"" raw_returns "${raw}")
list(LENGTH raw_returns raw_return_count)
if(NOT raw_return_count EQUAL 9)
    message(FATAL_ERROR "RETURN source tree lost a bare, valued or nested clause; see ${log}")
endif()
foreach(shape IN ITEMS
        [=[IF : "if"]=]
        [=[SELECT : "select"]=]
        [=[LABEL : "bare:"]=]
        [=[FUNCTION : "increment"]=])
    string(FIND "${raw}" "${shape}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "RETURN source tree lost ${shape}; see ${log}")
    endif()
endforeach()
foreach(shape IN ITEMS
        [=[MEMBER_CALL : "programReturnCode"]=]
        [=[MEMBER_CALL : "setReturnValue"]=]
        [=[MEMBER_CALL : "clearReturnValue"]=]
        [=[RETURN : "return" (19:24)]=]
        [=[RETURN : "return" (33:0)]=])
    string(FIND "${lowered}" "${shape}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "RETURN canonical tree lost ${shape}; see ${log}")
    endif()
endforeach()
string(FIND "${fixup}" [=[PROCEDURE : "main:"]=] main_position)
if(main_position EQUAL -1)
    message(FATAL_ERROR "RETURN fixup tree lost the implicit main wrapper; see ${log}")
endif()
