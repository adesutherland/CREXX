cmake_minimum_required(VERSION 3.24)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Address.rxbin
        COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1 $<TARGET_FILE:rxc> -x -i ${CREXX_EXIT_IMPORT_PATH} -o Address ${CMAKE_CURRENT_SOURCE_DIR}/Address.rexx
        COMMAND $<TARGET_FILE:rxas> -o Address Address
        DEPENDS rxc rxas library ${CMAKE_CURRENT_SOURCE_DIR}/Address.rexx ${CMAKE_CURRENT_BINARY_DIR}/../token.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Building Address.rxbin"
)
add_custom_target(exit_address_bin DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Address.rxbin)

crexx_add_compiler_exit_runtime_test(
        test_address
        test_address.rexx
        RUN_SUFFIX ${LIBRPATH}
)

crexx_add_compiler_exit_runtime_test(
        test_address_direct
        test_address_direct.rexx
        EXTRA_DEPENDS ${CMAKE_BINARY_DIR}/bin/exittestsupport.rxbin
        RUN_SUFFIX ${CMAKE_BINARY_DIR}/bin/exittestsupport.rxbin ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin ${LIBRPATH}
)
