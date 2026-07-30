cmake_minimum_required(VERSION 3.24)
project(CREXXOperationContractConsumer NONE)

find_package(CREXX CONFIG REQUIRED)

if(NOT TARGET CREXX::rxc OR NOT TARGET CREXX::rxas OR
   NOT TARGET CREXX::crexx-contract)
    message(FATAL_ERROR "installed CREXX compiler, assembler, or contract tool target is missing")
endif()

set(contract_stem "${CMAKE_CURRENT_BINARY_DIR}/external-operation")
set(contract_rxas "${contract_stem}.rxas")
set(contract_rxbin "${contract_stem}.rxbin")
set(contract_json "${CMAKE_CURRENT_BINARY_DIR}/external-operation.rxcontract.json")

add_custom_command(
        OUTPUT "${contract_rxas}"
        COMMAND $<TARGET_FILE:CREXX::rxc>
                -x -n -o "${contract_stem}"
                "${CMAKE_CURRENT_SOURCE_DIR}/external_operation.crexx"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/external_operation.crexx"
                CREXX::rxc
        VERBATIM)
add_custom_command(
        OUTPUT "${contract_rxbin}"
        COMMAND $<TARGET_FILE:CREXX::rxas>
                -n -o "${contract_stem}" "${contract_rxas}"
        DEPENDS "${contract_rxas}" CREXX::rxas
        VERBATIM)

crexx_add_operation_contract(
        TARGET external_operation_contract
        RXBIN "${contract_rxbin}"
        OPERATION external_operation.operationcontract.execute
        CONTRACT_VERSION 1.0.0
        OUTPUT "${contract_json}"
        NULLABLE external_operation.resultcontract.problem
        ERROR_TYPES external_operation.errorcontract)
