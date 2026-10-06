foreach(required RXC BINDIR SOURCE BUILD_DIR)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "Missing ${required}")
    endif()
endforeach()

set(log "${BUILD_DIR}/levelc_pull_source_tree.log")
execute_process(
        COMMAND "${RXC}" -d1 -i "${BINDIR}" -o levelc_pull_source_tree "${SOURCE}"
        WORKING_DIRECTORY "${BUILD_DIR}"
        OUTPUT_FILE "${log}" ERROR_FILE "${log}"
        RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "PULL source-tree compile failed; see ${log}")
endif()
file(READ "${log}" output)
string(FIND "${output}" "--- STAGE_RAW ---" raw_start)
string(FIND "${output}" "--- STAGE_LEVELC_LOWERED ---" lowered_start)
string(FIND "${output}" "--- STAGE_FIXUP ---" lowered_end)
if(raw_start EQUAL -1 OR lowered_start LESS raw_start OR lowered_end LESS lowered_start)
    message(FATAL_ERROR "PULL source/canonical stages missing; see ${log}")
endif()
math(EXPR raw_length "${lowered_start} - ${raw_start}")
math(EXPR lowered_length "${lowered_end} - ${lowered_start}")
string(SUBSTRING "${output}" ${raw_start} ${raw_length} raw)
string(SUBSTRING "${output}" ${lowered_start} ${lowered_length} lowered)
if(NOT raw MATCHES "PULL : \"pull\"" OR
   NOT raw MATCHES "TEMPLATES :" OR
   NOT lowered MATCHES "MEMBER_CALL : \"pullText\"" OR
   NOT lowered MATCHES "ASSEMBLER : \"parseplan\"" OR
   lowered MATCHES "PULL : ")
    message(FATAL_ERROR "PULL tree lost its source form or retained an unlowered node; see ${log}")
endif()
