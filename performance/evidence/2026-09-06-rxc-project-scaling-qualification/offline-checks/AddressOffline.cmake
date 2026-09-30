foreach(required_var CPRAG_RXC CPRAG_RXAS CPRAG_RXLINK CPRAG_RXVME CPRAG_RXBVM
        CPRAG_CREXX_BIN_DIR CPRAG_APPLICATION_DIR CPRAG_ADDRESS_RXBIN CPRAG_SCENARIO
        CPRAG_CONFIG_FIXTURE CPRAG_NATIVE_APPLICATION CPRAG_WORK_DIR)
    if(NOT DEFINED ${required_var} OR "${${required_var}}" STREQUAL "")
        message(FATAL_ERROR "${required_var} is required")
    endif()
endforeach()

file(REMOVE_RECURSE "${CPRAG_WORK_DIR}")
file(MAKE_DIRECTORY "${CPRAG_WORK_DIR}")
get_filename_component(address_dir "${CPRAG_ADDRESS_RXBIN}" DIRECTORY)
set(imports "${address_dir};${CPRAG_APPLICATION_DIR}/project;${CPRAG_CREXX_BIN_DIR}/providers;${CPRAG_CREXX_BIN_DIR}")

execute_process(
    COMMAND "${CPRAG_RXC}" -i "${imports}" -o "${CPRAG_WORK_DIR}/address_surface"
        "${CPRAG_SCENARIO}"
    RESULT_VARIABLE compile_result OUTPUT_VARIABLE compile_out ERROR_VARIABLE compile_err)
if(NOT compile_result EQUAL 0)
    message(FATAL_ERROR "ADDRESS RAG scenario compile failed:\n${compile_out}${compile_err}")
endif()
execute_process(
    COMMAND "${CPRAG_RXAS}" -o "${CPRAG_WORK_DIR}/address_surface"
        "${CPRAG_WORK_DIR}/address_surface"
    RESULT_VARIABLE assemble_result OUTPUT_VARIABLE assemble_out ERROR_VARIABLE assemble_err)
if(NOT assemble_result EQUAL 0)
    message(FATAL_ERROR "ADDRESS RAG scenario assembly failed:\n${assemble_out}${assemble_err}")
endif()

set(library "${CPRAG_WORK_DIR}/library")
execute_process(
    COMMAND "${CPRAG_NATIVE_APPLICATION}" --library "${library}"
        --config architecture-local --profile generic-profile --access admin
        --format json library init
    RESULT_VARIABLE init_result OUTPUT_VARIABLE init_out ERROR_VARIABLE init_err)
if(NOT init_result EQUAL 0)
    message(FATAL_ERROR "ADDRESS RAG test library init failed:\n${init_out}${init_err}")
endif()

set(link_inputs
    "${CPRAG_WORK_DIR}/address_surface.rxbin"
    "${CPRAG_ADDRESS_RXBIN}"
    "${CPRAG_APPLICATION_DIR}/project/crexxrag-project.rxbin"
    "${CPRAG_CREXX_BIN_DIR}/rxfnsg.rxbin"
    "${CPRAG_CREXX_BIN_DIR}/classlib.rxbin"
    "${CPRAG_CREXX_BIN_DIR}/library.rxbin")
execute_process(
    COMMAND "${CPRAG_RXLINK}" -s -r address_surface
        -m "${CPRAG_WORK_DIR}/address_surface.map"
        -p "${CPRAG_WORK_DIR}/address_surface.rxproviders"
        -o "${CPRAG_WORK_DIR}/address_surface_linked"
        ${link_inputs}
    RESULT_VARIABLE link_result OUTPUT_VARIABLE link_out ERROR_VARIABLE link_err)
if(NOT link_result EQUAL 0)
    message(FATAL_ERROR "ADDRESS RAG scenario link failed:\n${link_out}${link_err}")
endif()
foreach(runtime IN ITEMS "${CPRAG_RXVME}" "${CPRAG_RXBVM}")
    execute_process(
        COMMAND "${runtime}" --provider-path "${CPRAG_CREXX_BIN_DIR}/providers"
            -l "${CPRAG_WORK_DIR}" "${CPRAG_WORK_DIR}/address_surface_linked"
            -a "${library}" "${CPRAG_CONFIG_FIXTURE}"
        RESULT_VARIABLE run_result OUTPUT_VARIABLE run_out ERROR_VARIABLE run_err
        TIMEOUT 60)
    if(NOT run_result EQUAL 0 OR NOT run_out MATCHES "ADDRESS_OFFLINE_OK")
        message(FATAL_ERROR "ADDRESS RAG scenario failed:\n${run_out}${run_err}")
    endif()
endforeach()

message(STATUS "Offline ADDRESS OPEN/STATUS/CLOSE passed on both VMs")
