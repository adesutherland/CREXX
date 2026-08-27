foreach(_required IN ITEMS RXC IMPORT_DIR WORK SOURCE)
    if(NOT DEFINED ${_required} OR "${${_required}}" STREQUAL "")
        message(FATAL_ERROR "Missing required variable ${_required}")
    endif()
endforeach()

file(MAKE_DIRECTORY "${WORK}")
execute_process(
        COMMAND "${CMAKE_COMMAND}" -E env CREXX_DIAGNOSTICS=raw RXCP_EXIT_MODULE=rxcexits
                "${RXC}" -i "${IMPORT_DIR}" -n -o cri18_exit_scratch "${SOURCE}"
        WORKING_DIRECTORY "${WORK}"
        OUTPUT_VARIABLE _stdout
        ERROR_VARIABLE _stderr
        RESULT_VARIABLE _result)
set(_output "${_stdout}${_stderr}")

if(NOT _result EQUAL 0)
    message(FATAL_ERROR "CRI-18 exit scratch source failed to compile with ${_result}\n${_output}")
endif()
if(_output MATCHES "Internal warning in exit_fragment" OR
   _output MATCHES "#NOT_IN_SAME_SCOPE")
    message(FATAL_ERROR "Exit-generated private scratch bindings were not explicitly declared\n${_output}")
endif()

set(_rxas_path "${WORK}/cri18_exit_scratch.rxas")
if(NOT EXISTS "${_rxas_path}")
    message(FATAL_ERROR "Expected generated assembly ${_rxas_path}")
endif()
file(READ "${_rxas_path}" _rxas)
foreach(_declaration IN ITEMS
        "__rxtrace_mode=.string"
        "_q_info=.string"
        "__rxaddr_request=.addressrequest"
        "__rxaddr_response=.addressresponse"
        "__rxaddr_stem_1=.standardaddressstem"
        "__rxaddr_i_1=.int"
        "__rxaddr_binding_1=.addressbinding"
        "__rxaddr_count_1=.float"
        "__rxsignal_name=.string"
        "__rxsignal_payload=.string"
        "__rc=.int")
    string(FIND "${_rxas}" "${_declaration}" _position)
    if(_position EQUAL -1)
        message(FATAL_ERROR "Missing explicit generated declaration '${_declaration}' in ${_rxas_path}")
    endif()
endforeach()
