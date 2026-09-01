cmake_minimum_required(VERSION 3.24)

project(rx_hash_plugin C)

set(CMAKE_C_STANDARD 99)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

add_dynamic_plugin_target(_hash rx_hash.c)
target_include_directories(_hash PRIVATE
        ${CMAKE_SOURCE_DIR}/binutils/include)
target_link_libraries(_hash PRIVATE rxbin)

add_static_plugin_target(_hash rx_hash.c)
target_include_directories(_hash_static PRIVATE
        ${CMAKE_SOURCE_DIR}/binutils/include)
target_link_libraries(_hash_static PRIVATE rxbin)

add_rxpa_provider_package(_hash)

# The provider remains part of the default product build.  Keep it independent
# of targeted core-C compiler/VM builds so the bootstrap stage does not acquire
# a later Level B native-provider prerequisite.

if(BUILD_TESTING)
    add_custom_target(rx_hash_test ALL)
    set(_rx_hash_runners rxbvm)
    if(TARGET rxtvm)
        list(APPEND _rx_hash_runners rxtvm)
    endif()

    foreach(_rx_hash_runner IN LISTS _rx_hash_runners)
        crexx_add_rexx_opt_matrix(
                NAME rx_hash_${_rx_hash_runner}
                SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/rx_hash_test.crexx
                TARGET_GROUP rx_hash_test
                RUNNER ${_rx_hash_runner}
                PASS_TEST_NAME_ARGUMENT
                IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
                DEPENDS library _hash _hash_static
                RUNTIME_ARGS ${CMAKE_BINARY_DIR}/bin/library
                TEST_LABELS "library;plugins;rxpa;hash;sha256;checksum"
                REQUIRED_OUTPUT_REGEX "PASS: rxhash complete SHA-256 family and checksum vectors"
                FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
    endforeach()
endif()
