set(_autoload_work "${CMAKE_CURRENT_BINARY_DIR}/work")

add_custom_target(rxbin_autoload_test_prereqs
        DEPENDS rxc rxas rxdas rxlink rxvm rxbvm)

add_test(
        NAME rxbin_autoload_contract
        COMMAND ${CMAKE_COMMAND}
                -DRXC=$<TARGET_FILE:rxc>
                -DRXAS=$<TARGET_FILE:rxas>
                -DRXDAS=$<TARGET_FILE:rxdas>
                -DRXLINK=$<TARGET_FILE:rxlink>
                -DRXVM=${CREXX_RXVM_EXECUTABLE}
                -DRXBVM=$<TARGET_FILE:rxbvm>
                -DPROVIDER_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/provider/provider.crexx
                -DCONSUMER_SOURCE=${CMAKE_CURRENT_SOURCE_DIR}/consumer/consumer.crexx
                -DWORK=${_autoload_work}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/check_autoload.cmake)
set_tests_properties(rxbin_autoload_contract PROPERTIES
        LABELS "compiler;assembler;linker;interpreter;autoload;regression"
        CREXX_PREP_TARGETS rxbin_autoload_test_prereqs
        TIMEOUT 60)

crexx_label_smoke_tests(
        rxbin_autoload_contract
        PREP_TARGETS rxbin_autoload_test_prereqs)
