cmake_minimum_required(VERSION 3.24)
project(rxtcp_plugin C)

set(CMAKE_C_STANDARD 90)
if(WIN32)
	link_libraries(ws2_32 wsock32)
endif()	

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_rxtcp rxtcp.c)

add_custom_target(tcp_test ALL)
crexx_add_rexx_opt_matrix(
        NAME tcp_test
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tcp_test.rexx
        TARGET_GROUP tcp_test
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS library _rxtcp
        RUNTIME_ARGS rx_rxtcp ${CMAKE_BINARY_DIR}/bin/library
)

enable_testing()

# Create the static link module
add_static_plugin_target(_rxtcp rxtcp.c)

#Add include directories - static version
target_include_directories(_rxtcp_static PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib/include
    ${CMAKE_SOURCE_DIR}/rxpa
)
