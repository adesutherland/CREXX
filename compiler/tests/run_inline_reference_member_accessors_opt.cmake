file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

execute_process(
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${SOURCE}"
                "${WORK}/inline_reference_member_accessors.crexx"
        RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Failed to stage reference-member accessor source")
endif()

execute_process(
        COMMAND "${RXC}" -i "${IMPORT_DIR}" -o inline_reference_member_accessors
                "${WORK}/inline_reference_member_accessors.crexx"
        WORKING_DIRECTORY "${WORK}"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "rxc failed on reference-member accessor case: ${out}${err}")
endif()

file(READ "${WORK}/inline_reference_member_accessors.rxas" image)
if(image MATCHES [=[call[0-9]* [^\n]*inline_reference_member_accessors\.node\.(next|setnext)\(\)]=])
    message(FATAL_ERROR "Exact reference getter/setter remained on the call path:\n${image}")
endif()
if(image MATCHES [=[call[0-9]* [^\n]*inline_reference_member_accessors\.refholder\.get\(\)]=])
    message(FATAL_ERROR "Exact scalar-reference getter remained on the call path:\n${image}")
endif()
if(NOT image MATCHES [=[call[0-9]* [^\n]*inline_reference_member_accessors\.node\.observednext\(\)]=])
    message(FATAL_ERROR "Side-effecting reference getter did not remain fail closed:\n${image}")
endif()

execute_process(
        COMMAND "${RXAS}" -o inline_reference_member_accessors.rxbin
                inline_reference_member_accessors
        WORKING_DIRECTORY "${WORK}"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "rxas failed on reference-member accessor case: ${out}${err}")
endif()

foreach(runner IN ITEMS "${RXVM}" "${RXBVM}")
    execute_process(
            COMMAND "${runner}" "${LIBRARY}"
                    "${WORK}/inline_reference_member_accessors.rxbin"
            WORKING_DIRECTORY "${WORK}"
            OUTPUT_VARIABLE run_out
            ERROR_VARIABLE err
            RESULT_VARIABLE result)
    string(REPLACE "\r\n" "\n" run_out "${run_out}")
    if(NOT result EQUAL 0 OR
       NOT run_out STREQUAL "PASS: Inline Reference Member Accessors\n")
        message(FATAL_ERROR
                "Reference-member accessor runtime mismatch for ${runner}: ${run_out}${err}")
    endif()
endforeach()
