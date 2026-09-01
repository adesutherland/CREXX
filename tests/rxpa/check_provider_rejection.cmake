if(NOT DEFINED RUNNER OR NOT DEFINED PROGRAM OR NOT DEFINED WRONG_PLUGIN OR
   NOT DEFINED SCRATCH)
    message(FATAL_ERROR "provider rejection test arguments are incomplete")
endif()

file(REMOVE_RECURSE "${SCRATCH}")
file(MAKE_DIRECTORY "${SCRATCH}/bin" "${SCRATCH}/app/providers")
file(COPY_FILE "${RUNNER}" "${SCRATCH}/bin/rxvm" ONLY_IF_DIFFERENT)
file(COPY_FILE "${PROGRAM}" "${SCRATCH}/app/probe.rxbin" ONLY_IF_DIFFERENT)
file(COPY_FILE "${WRONG_PLUGIN}"
     "${SCRATCH}/app/providers/rx_rcc_provider.rxplugin" ONLY_IF_DIFFERENT)

execute_process(
        COMMAND "${SCRATCH}/bin/rxvm" "${SCRATCH}/app/probe.rxbin"
        RESULT_VARIABLE _result
        OUTPUT_VARIABLE _stdout
        ERROR_VARIABLE _stderr)
set(_output "${_stdout}${_stderr}")
if(_result EQUAL 0)
    message(FATAL_ERROR "wrong-identity provider unexpectedly executed")
endif()
if(NOT _output MATCHES
   "resolved to manifest id rx_rxpa_dynlink" OR
   NOT _output MATCHES
   "required RXPA provider rx_rcc_provider")
    message(FATAL_ERROR "provider rejection diagnostic was incomplete: ${_output}")
endif()
