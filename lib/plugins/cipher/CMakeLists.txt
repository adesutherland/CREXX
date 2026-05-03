cmake_minimum_required(VERSION 3.24)
project(cipher_plugin C)

set(CMAKE_C_STANDARD 90)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)

# Including RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create dynamic plugin module
add_dynamic_plugin_target(_cipher cipher.c)

add_custom_target(cipher_test ALL)
crexx_add_rexx_opt_matrix(
        NAME cipher_test
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/cipher_test.rexx
        TARGET_GROUP cipher_test
        IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS library _cipher
        RUNTIME_ARGS rx_cipher ${CMAKE_BINARY_DIR}/bin/library
)

enable_testing()

# Create the static link module
add_static_plugin_target(_cipher cipher.c)

#Add include directories - static version
target_include_directories(_cipher_static PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib/include
    ${CMAKE_SOURCE_DIR}/rxpa
)
