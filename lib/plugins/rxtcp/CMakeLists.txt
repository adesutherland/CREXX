cmake_minimum_required(VERSION 3.24)
project(rxtcp_plugin C)

set(CMAKE_C_STANDARD 90)
if(WIN32)
	link_libraries(ws2_32 wsock32)
endif()	
# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_rxtcp rxtcp.c)

# Build Test Rexx
add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc
        -i \"${CMAKE_BINARY_DIR}/lib/rxfns\;${CMAKE_CURRENT_BINARY_DIR}\"
        -o tcp_test \"${CMAKE_CURRENT_SOURCE_DIR}/tcp_test\" &&
        ${CMAKE_BINARY_DIR}/assembler/rxas tcp_test

        DEPENDS rxas rxc library _rxtcp ${CMAKE_CURRENT_SOURCE_DIR}/tcp_test.rexx
        OUTPUT tcp_test.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_custom_target(tcp_test ALL
        DEPENDS tcp_test.rxbin
        )

# Enable testing functionality
enable_testing()

# Basic test to run tcp_test.rxbin with the rxtcp plugin
add_test(NAME tcp_test
        COMMAND ${CMAKE_BINARY_DIR}/interpreter/rxvm tcp_test rx_rxtcp ${CMAKE_BINARY_DIR}/lib/rxfns/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
