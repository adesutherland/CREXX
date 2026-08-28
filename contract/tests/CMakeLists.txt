add_test(
        NAME crexx_contract_generation
        COMMAND ${CMAKE_COMMAND}
                -DRXC=$<TARGET_FILE:rxc>
                -DRXAS=$<TARGET_FILE:rxas>
                -DCONTRACT_TOOL=$<TARGET_FILE:crexx-contract>
                -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/operation_contract.crexx
                -DINVALID_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/operation_contract_invalid.crexx
                -DAMBIGUOUS_RXAS=${CMAKE_CURRENT_SOURCE_DIR}/ambiguous_contract.rxas
                -DGOLDEN=${CMAKE_CURRENT_SOURCE_DIR}/operation_contract.rxcontract.json
                -DWORK=${CMAKE_CURRENT_BINARY_DIR}/generation
                -P ${CMAKE_CURRENT_SOURCE_DIR}/contract_generation.cmake)

set_tests_properties(crexx_contract_generation PROPERTIES
        LABELS "contract;compiler;binutils")
set_property(TEST crexx_contract_generation APPEND PROPERTY
        CREXX_PREP_TARGETS rxc rxas crexx-contract)

add_test(
        NAME crexx_contract_installed_consumer
        COMMAND ${CMAKE_COMMAND}
                -DCREXX_BUILD=${CMAKE_BINARY_DIR}
                -DCREXX_SOURCE=${CMAKE_SOURCE_DIR}
                -DCONTRACT_TOOL_FILE_NAME=$<TARGET_FILE_NAME:crexx-contract>
                -DEXTERNAL_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/external
                -DEXTERNAL_GOLDEN=${CMAKE_CURRENT_SOURCE_DIR}/external/external_operation.rxcontract.json
                -DWORK=${CMAKE_CURRENT_BINARY_DIR}/installed-consumer
                -P ${CMAKE_CURRENT_SOURCE_DIR}/installed_consumer.cmake)
set_tests_properties(crexx_contract_installed_consumer PROPERTIES
        LABELS "contract;install;external-consumer"
        RUN_SERIAL TRUE)

crexx_finalize_directory_qa_tiers()
