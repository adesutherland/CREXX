cmake_minimum_required(VERSION 3.24)
project(llist_plugin C)

set(CMAKE_C_STANDARD 90)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_llist llist.c)

# Create the static link module
add_static_plugin_target(_llist llist.c)

add_custom_target(llist_test ALL)
crexx_add_rexx_opt_matrix(
        NAME llist_test
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/llist_test.rexx
        TARGET_GROUP llist_test
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS library _llist
        RUNTIME_ARGS rx_llist ${CMAKE_BINARY_DIR}/bin/library
)

enable_testing()
