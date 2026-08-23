if(NOT DEFINED RXC OR NOT DEFINED RXAS OR NOT DEFINED RXLINK OR
   NOT DEFINED SOURCE OR NOT DEFINED IMPORT_PATH OR NOT DEFINED SCRATCH)
    message(FATAL_ERROR "unused-provider test arguments are incomplete")
endif()

file(REMOVE_RECURSE "${SCRATCH}")
file(MAKE_DIRECTORY "${SCRATCH}")
execute_process(
        COMMAND "${RXC}" -i "${IMPORT_PATH}" -o unused "${SOURCE}"
        WORKING_DIRECTORY "${SCRATCH}"
        RESULT_VARIABLE _compile_result
        OUTPUT_VARIABLE _compile_out
        ERROR_VARIABLE _compile_err)
if(NOT _compile_result EQUAL 0)
    message(FATAL_ERROR "unused-provider compile failed: ${_compile_out}${_compile_err}")
endif()
file(READ "${SCRATCH}/unused.rxas" _rxas)
if(_rxas MATCHES "=\"\\.provider")
    message(FATAL_ERROR "unused native declaration emitted a provider dependency")
endif()
execute_process(
        COMMAND "${RXAS}" -o unused unused
        WORKING_DIRECTORY "${SCRATCH}"
        RESULT_VARIABLE _assemble_result)
if(NOT _assemble_result EQUAL 0)
    message(FATAL_ERROR "unused-provider assembly failed")
endif()
execute_process(
        COMMAND "${RXLINK}" -p unused.rxproviders -o unused_linked unused.rxbin
        WORKING_DIRECTORY "${SCRATCH}"
        RESULT_VARIABLE _link_result
        OUTPUT_VARIABLE _link_out
        ERROR_VARIABLE _link_err)
if(NOT _link_result EQUAL 0)
    message(FATAL_ERROR "unused-provider link failed: ${_link_out}${_link_err}")
endif()
file(READ "${SCRATCH}/unused.rxproviders" _requirements)
if(NOT _requirements STREQUAL "CREXX-RXPA-REQUIREMENTS 1\n")
    message(FATAL_ERROR "unused native declaration reached requirements: ${_requirements}")
endif()
