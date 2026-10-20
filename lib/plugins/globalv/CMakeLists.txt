cmake_minimum_required(VERSION 3.24)

project(rxglobalv_plugin C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

set(_globalv_upstream_dir "${CMAKE_CURRENT_SOURCE_DIR}/upstream")
set(_globalv_core_sources
        "${_globalv_upstream_dir}/gvcore.c"
        "${_globalv_upstream_dir}/gvcmd.c"
        "${_globalv_upstream_dir}/gvfs_pc.c")

add_executable(globalv
        ${_globalv_core_sources}
        "${_globalv_upstream_dir}/gvpc.c")
target_include_directories(globalv PRIVATE
        "${_globalv_upstream_dir}")
set_target_properties(globalv PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
crexx_install_targets(globalv)

add_dynamic_plugin_target(globalv_plugin
        PROVIDER_ID rxglobalv
        ${_globalv_core_sources}
        rxglobalv.c)
add_static_plugin_target(globalv_plugin
        PROVIDER_ID rxglobalv
        ${_globalv_core_sources}
        rxglobalv.c)
target_include_directories(globalv_plugin PRIVATE
        "${_globalv_upstream_dir}")
target_include_directories(globalv_plugin_static PRIVATE
        "${_globalv_upstream_dir}")
target_link_libraries(globalv_plugin PRIVATE Threads::Threads)
target_link_libraries(globalv_plugin_static PRIVATE Threads::Threads)
add_rxpa_provider_package(globalv_plugin INSTALL PROVIDER_ID rxglobalv)

# The ADDRESS facade is installed separately from library.rxbin. Applications
# opt in by loading rxglobalv_address.rxbin together with rxglobalv.rxplugin.
set(_globalv_address_import_root
        "${CMAKE_CURRENT_BINARY_DIR}/address-imports")
crexx_add_import_root(rxglobalv_address_imports
        DIRECTORY "${_globalv_address_import_root}"
        OUTPUT_VARIABLE _globalv_address_import_files
        FILES
                "${CMAKE_BINARY_DIR}/bin/library.rxbin"
                "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
                "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
                "${CMAKE_BINARY_DIR}/bin/rxglobalv.rxplugin"
        DEPENDS library classlib compiler_exit_bin globalv_plugin
        COMMENT "Staging deterministic rxglobalv ADDRESS imports ...")

set(_globalv_address_base "${CMAKE_BINARY_DIR}/bin/rxglobalv_address")
add_custom_command(
        OUTPUT "${_globalv_address_base}.rxbin"
        COMMAND $<TARGET_FILE:rxc>
                --no-exe-import
                -i "${_globalv_address_import_root}"
                -o "${_globalv_address_base}"
                "${CMAKE_CURRENT_SOURCE_DIR}/rexx/globalv_address.crexx"
        COMMAND $<TARGET_FILE:rxas>
                -o "${_globalv_address_base}"
                "${_globalv_address_base}"
        COMMAND ${CMAKE_COMMAND} -E rm -f
                "${_globalv_address_base}.rxas"
        DEPENDS
                rxc rxas rxglobalv_address_imports
                ${_globalv_address_import_files}
                "${CMAKE_CURRENT_SOURCE_DIR}/rexx/globalv_address.crexx"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Building the rxglobalv ADDRESS facade ..."
        VERBATIM)
add_custom_target(rxglobalv_address ALL
        DEPENDS "${_globalv_address_base}.rxbin")

if(BUILD_TESTING)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    add_custom_target(globalv_test_artifacts ALL)
    crexx_add_rexx_opt_matrix(
            NAME globalv_address
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tests/globalv_address_test.crexx
            TARGET_GROUP globalv_test_artifacts
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            DEPENDS library classlib globalv_plugin rxglobalv_address
            RUNTIME_ARGS
                    ${CMAKE_BINARY_DIR}/bin/library
                    ${CMAKE_BINARY_DIR}/bin/classlib
                    ${CMAKE_BINARY_DIR}/bin/rxglobalv_address
            TEST_LABELS "library;plugins;rxpa;globalv;address;regression"
            REQUIRED_OUTPUT_REGEX "PASS: GLOBALV ADDRESS contract"
            FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
    crexx_add_rexx_opt_matrix(
            NAME globalv_direct
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tests/globalv_direct_test.crexx
            TARGET_GROUP globalv_test_artifacts
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            DEPENDS library classlib globalv_plugin
            RUNTIME_ARGS
                    ${CMAKE_BINARY_DIR}/bin/library
                    ${CMAKE_BINARY_DIR}/bin/classlib
            TEST_LABELS "library;plugins;rxpa;globalv;direct;regression"
            REQUIRED_OUTPUT_REGEX "PASS: GLOBALV DIRECT contract"
            FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
    set_tests_properties(globalv_address_noopt globalv_address_opt PROPERTIES
            ENVIRONMENT
                    "GLOBALV_HOME=${CMAKE_CURRENT_BINARY_DIR}/test-home;GLOBALV_SESSION=${CMAKE_CURRENT_BINARY_DIR}/test-session"
            RESOURCE_LOCK globalv_storage
            TIMEOUT 60)
    set_tests_properties(globalv_direct_noopt globalv_direct_opt PROPERTIES
            ENVIRONMENT
                    "GLOBALV_HOME=${CMAKE_CURRENT_BINARY_DIR}/direct-test-home;GLOBALV_SESSION=${CMAKE_CURRENT_BINARY_DIR}/direct-test-session"
            RESOURCE_LOCK globalv_storage
            TIMEOUT 60)
    add_test(NAME globalv_lock
            COMMAND ${Python3_EXECUTABLE}
                    ${CMAKE_CURRENT_SOURCE_DIR}/tests/globalv_lock_test.py
                    $<TARGET_FILE:globalv>)
    set_tests_properties(globalv_lock PROPERTIES
            ENVIRONMENT
                    "GLOBALV_HOME=${CMAKE_CURRENT_BINARY_DIR}/lock-test-home;GLOBALV_SESSION=${CMAKE_CURRENT_BINARY_DIR}/lock-test-session"
            RESOURCE_LOCK globalv_storage
            LABELS "globalv;regression"
            TIMEOUT 60)
endif()

crexx_finalize_directory_qa_tiers()
crexx_install_bytecode(rxglobalv_address "${_globalv_address_base}.rxbin")
