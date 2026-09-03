cmake_minimum_required(VERSION 3.24)

project(rxsqlite_plugin C)

set(CMAKE_C_STANDARD 99)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# SQLite 3.53.2 is source-controlled so the normal product build remains
# deterministic and offline.  Compile the amalgamation once and place the same
# object into both provider delivery forms.  In particular, never inherit the
# retired demo's SQLITE_THREADSAFE=0 build.
add_library(crexx_sqlite3_objects OBJECT vendor/sqlite3.c)
set_target_properties(crexx_sqlite3_objects PROPERTIES
        POSITION_INDEPENDENT_CODE ON)
target_include_directories(crexx_sqlite3_objects PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor)
target_compile_definitions(crexx_sqlite3_objects PRIVATE
        SQLITE_THREADSAFE=1
        SQLITE_ENABLE_FTS5=1
        SQLITE_DQS=0
        SQLITE_OMIT_LOAD_EXTENSION=1
        SQLITE_ENABLE_API_ARMOR=1
        SQLITE_SECURE_DELETE=1)
if(MSVC)
    target_compile_options(crexx_sqlite3_objects PRIVATE /w)
else()
    target_compile_options(crexx_sqlite3_objects PRIVATE -w)
endif()

set(_rxsqlite_sources
        rxsqlite.c
        $<TARGET_OBJECTS:crexx_sqlite3_objects>)
add_dynamic_plugin_target(sqlite ${_rxsqlite_sources})
add_static_plugin_target(sqlite ${_rxsqlite_sources})
foreach(_rxsqlite_target IN ITEMS sqlite sqlite_static)
    target_include_directories(${_rxsqlite_target} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/vendor)
    target_link_libraries(${_rxsqlite_target} PRIVATE Threads::Threads)
endforeach()
add_rxpa_provider_package(sqlite)

# Build the Level-G ADDRESS facade as an ordinary installed cREXX module.  Its
# native declaration input is the same rxsqlite plugin installed for runtimes.
set(_rxsqlite_address_import_root
        "${CMAKE_CURRENT_BINARY_DIR}/address-imports")
crexx_add_import_root(rxsqlite_address_imports
        DIRECTORY "${_rxsqlite_address_import_root}"
        OUTPUT_VARIABLE _rxsqlite_address_import_files
        FILES
                "${CMAKE_BINARY_DIR}/bin/library.rxbin"
                "${CMAKE_BINARY_DIR}/bin/classlib.rxbin"
                "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
                "${CMAKE_BINARY_DIR}/bin/rxsqlite.rxplugin"
        DEPENDS library classlib compiler_exit_bin sqlite
        COMMENT "Staging deterministic rxsqlite ADDRESS imports ...")

set(_rxsqlite_address_base "${CMAKE_BINARY_DIR}/bin/rxsqlite_address")
add_custom_command(
        OUTPUT "${_rxsqlite_address_base}.rxbin"
        COMMAND $<TARGET_FILE:rxc>
                --no-exe-import
                -i "${_rxsqlite_address_import_root}"
                -o "${_rxsqlite_address_base}"
                "${CMAKE_CURRENT_SOURCE_DIR}/rexx/rxsqlite_address.crexx"
        COMMAND $<TARGET_FILE:rxas>
                -o "${_rxsqlite_address_base}"
                "${_rxsqlite_address_base}"
        COMMAND ${CMAKE_COMMAND} -E rm -f
                "${_rxsqlite_address_base}.rxas"
        DEPENDS
                rxc rxas rxsqlite_address_imports
                ${_rxsqlite_address_import_files}
                "${CMAKE_CURRENT_SOURCE_DIR}/rexx/rxsqlite_address.crexx"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Building the rxsqlite Level-G ADDRESS facade ..."
        VERBATIM)
add_custom_target(rxsqlite_address ALL
        DEPENDS "${_rxsqlite_address_base}.rxbin")

if(BUILD_TESTING)
    add_executable(rxsqlite_thread_test tests/rxsqlite_thread_test.c)
    set_target_properties(rxsqlite_thread_test PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
    target_include_directories(rxsqlite_thread_test PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/vendor
            ${CMAKE_SOURCE_DIR}/rxpa
            ${CMAKE_SOURCE_DIR}/platform
            ${CMAKE_BINARY_DIR}/generated)
    target_link_libraries(rxsqlite_thread_test PRIVATE
            crexx_sqlite3_objects Threads::Threads)
    add_test(NAME rxsqlite_thread_sessions
            COMMAND rxsqlite_thread_test
                    "${CMAKE_CURRENT_BINARY_DIR}/rxsqlite-thread.db")
    set_tests_properties(rxsqlite_thread_sessions PROPERTIES
            LABELS "library;plugins;rxpa;sqlite;concurrency;regression"
            PASS_REGULAR_EXPRESSION
                    "PASS: rxsqlite thread sessions=4 rows=400 diagnostics_isolated=1 handle_isolation=1 fullmutex=1"
            TIMEOUT 60)
    crexx_register_test_prep_targets(
            TESTS rxsqlite_thread_sessions
            TARGETS rxsqlite_thread_test)

    add_custom_target(rxsqlite_test_artifacts ALL)
    set(_rxsqlite_runners rxbvm)
    if(TARGET rxtvm)
        list(APPEND _rxsqlite_runners rxtvm)
    endif()
    foreach(_rxsqlite_runner IN LISTS _rxsqlite_runners)
        crexx_add_rexx_opt_matrix(
                NAME rxsqlite_typed_${_rxsqlite_runner}
                SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tests/rxsqlite_test.crexx
                TARGET_GROUP rxsqlite_test_artifacts
                RUNNER ${_rxsqlite_runner}
                PASS_TEST_NAME_ARGUMENT
                IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
                DEPENDS library sqlite sqlite_static
                RUNTIME_ARGS ${CMAKE_BINARY_DIR}/bin/library
                TEST_LABELS "library;plugins;rxpa;sqlite;regression"
                REQUIRED_OUTPUT_REGEX "PASS: rxsqlite typed provider boundary"
                FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
        crexx_add_rexx_opt_matrix(
                NAME rxsqlite_file_${_rxsqlite_runner}
                SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tests/rxsqlite_file_test.crexx
                TARGET_GROUP rxsqlite_test_artifacts
                RUNNER ${_rxsqlite_runner}
                PASS_TEST_NAME_ARGUMENT
                IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
                DEPENDS library sqlite sqlite_static
                RUNTIME_ARGS ${CMAKE_BINARY_DIR}/bin/library
                TEST_LABELS "library;plugins;rxpa;sqlite;filesystem;concurrency;regression"
                REQUIRED_OUTPUT_REGEX "PASS: rxsqlite file provider"
                FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
    endforeach()

    crexx_add_rexx_opt_matrix(
            NAME rxsqlite_attached
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tests/rxsqlite_attached_test.crexx
            TARGET_GROUP rxsqlite_test_artifacts
            RXC_FLAGS -x
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            DEPENDS library classlib sqlite sqlite_static rxsqlite_address
            RUNTIME_ARGS
                    ${CMAKE_BINARY_DIR}/bin/library
                    ${CMAKE_BINARY_DIR}/bin/classlib
            TEST_LABELS "library;plugins;rxpa;sqlite;concurrency;cri-17;regression"
            REQUIRED_OUTPUT_REGEX
                    "PASS: rxsqlite resolves in attached local-task VMs"
            FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
    set_tests_properties(rxsqlite_attached_noopt rxsqlite_attached_opt PROPERTIES
            ENVIRONMENT "CREXX_PROVIDER_PATH=${CMAKE_BINARY_DIR}/bin/providers"
            TIMEOUT 60)

    crexx_add_rexx_opt_matrix(
            NAME rxsqlite_address
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tests/rxsqlite_address_test.crexx
            TARGET_GROUP rxsqlite_test_artifacts
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            DEPENDS library classlib sqlite sqlite_static rxsqlite_address
            RUNTIME_ARGS
                    ${CMAKE_BINARY_DIR}/bin/library
                    ${CMAKE_BINARY_DIR}/bin/classlib
                    ${CMAKE_BINARY_DIR}/bin/rxsqlite_address
            TEST_LABELS "library;plugins;rxpa;sqlite;address;regression"
            REQUIRED_OUTPUT_REGEX
                    "PASS: ADDRESS SQLITE commands functions bindings output diagnostics instances=2"
            FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
endif()

crexx_finalize_directory_qa_tiers()
