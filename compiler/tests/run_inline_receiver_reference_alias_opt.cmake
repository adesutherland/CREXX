file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")

execute_process(
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${SOURCE}" "${WORK}/inline_receiver_reference_alias.crexx"
        RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Failed to stage direct-receiver reference-alias source")
endif()

execute_process(
        COMMAND "${RXC}" -i "${IMPORT_DIR}" -o inline_receiver_reference_alias
                "${WORK}/inline_receiver_reference_alias.crexx"
        WORKING_DIRECTORY "${WORK}"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "rxc failed on direct-receiver reference-alias case: ${out}${err}")
endif()

file(READ "${WORK}/inline_receiver_reference_alias.rxas" image)
if(image MATCHES [=[call [^\n]*inline_receiver_reference_alias\.box\.(get|set)\(\)]=])
    message(FATAL_ERROR "Proved direct getter/setter receiver was not inlined:\n${image}")
endif()
if(NOT image MATCHES [=[call[0-9]* [^\n]*inline_receiver_reference_alias\.box\.selfref\(\)]=])
    message(FATAL_ERROR "Reference-producing selfRef() unexpectedly left the conservative call path:\n${image}")
endif()

string(REPLACE "\n" "|" flat_image "${image}")
string(REGEX MATCH [=[\.traceevent "V"[^|]*"r" ([0-9]+)[^|]*"sample"[^|]*\|[ \t]*\.traceevent "A"[^|]*"r" ([0-9]+)[^|]*"§this"]=]
       receiver_binding "${flat_image}")
set(receiver_source_reg "${CMAKE_MATCH_1}")
set(receiver_this_reg "${CMAKE_MATCH_2}")
if(receiver_binding STREQUAL "" OR NOT receiver_source_reg STREQUAL receiver_this_reg)
    message(FATAL_ERROR "Reference-targeted direct receiver did not share the caller register:\n${image}")
endif()

execute_process(
        COMMAND "${RXAS}" -o inline_receiver_reference_alias.rxbin inline_receiver_reference_alias
        WORKING_DIRECTORY "${WORK}"
        OUTPUT_VARIABLE out
        ERROR_VARIABLE err
        RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "rxas failed on direct-receiver reference-alias case: ${out}${err}")
endif()

foreach(runner IN ITEMS "${RXVM}" "${RXBVM}")
    execute_process(
            COMMAND "${runner}" "${LIBRARY}" "${WORK}/inline_receiver_reference_alias.rxbin"
            WORKING_DIRECTORY "${WORK}"
            OUTPUT_VARIABLE run_out
            ERROR_VARIABLE err
            RESULT_VARIABLE result)
    string(REPLACE "\r\n" "\n" run_out "${run_out}")
    if(NOT result EQUAL 0 OR NOT run_out STREQUAL "beta\n")
        message(FATAL_ERROR "Reference-alias receiver runtime mismatch for ${runner}: ${run_out}${err}")
    endif()
endforeach()
