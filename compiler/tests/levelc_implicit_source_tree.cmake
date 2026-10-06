foreach(required RXC BINDIR SOURCE BUILD_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()

set(log "${BUILD_DIR}/levelc_implicit_source_tree.log")
execute_process(
        COMMAND "${RXC}" -d1 -i "${BINDIR}" -o levelc_implicit_source_tree "${SOURCE}"
        WORKING_DIRECTORY "${BUILD_DIR}"
        OUTPUT_FILE "${log}" ERROR_FILE "${log}"
        RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "implicit command source tree compile failed; see ${log}")
endif()
file(READ "${log}" output)
string(FIND "${output}" "--- STAGE_RAW ---" raw_start)
string(FIND "${output}" "--- STAGE_LEVELC_LOWERED ---" lowered_start)
string(FIND "${output}" "--- STAGE_FIXUP ---" lowered_end)
if(raw_start EQUAL -1 OR lowered_start EQUAL -1 OR lowered_end EQUAL -1)
    message(FATAL_ERROR "implicit command tree stages are missing; see ${log}")
endif()
math(EXPR raw_length "${lowered_start} - ${raw_start}")
math(EXPR lowered_length "${lowered_end} - ${lowered_start}")
string(SUBSTRING "${output}" ${raw_start} ${raw_length} raw)
string(SUBSTRING "${output}" ${lowered_start} ${lowered_length} lowered)

foreach(fragment IN ITEMS
        [=[IMPLICIT_CMD :]=]
        [=[VAR_SYMBOL : "command.tail"]=]
        [=[FUNCTION : "make_command"]=]
        [=[WARNING : "RXC-LC-IMPLICIT_ADDRESS]=])
    string(FIND "${raw}" "${fragment}" found)
    if(found EQUAL -1)
        message(FATAL_ERROR "implicit command authored tree lost ${fragment}; see ${log}")
    endif()
endforeach()
foreach(fragment IN ITEMS
        [=[FACTORY_CALL : "RexxClassicAddressCommand"]=]
        [=[MEMBER_CALL : "useActiveConnections"]=]
        [=[MEMBER_CALL : "addressEnvironment"]=]
        [=[MEMBER_CALL : "run"]=])
    string(FIND "${lowered}" "${fragment}" found)
    if(found EQUAL -1)
        message(FATAL_ERROR "implicit command lowered tree lost ${fragment}; see ${log}")
    endif()
endforeach()
string(FIND "${lowered}" "IMPLICIT_CMD" leaked)
if(NOT leaked EQUAL -1)
    message(FATAL_ERROR "implicit command node survived lowering; see ${log}")
endif()
