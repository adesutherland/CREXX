cmake_minimum_required(VERSION 3.24)
project(rxconsole_plugin C)
set(CMAKE_C_STANDARD 99)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxTestModes.cmake)
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)
# console.c remains a historical, unbuilt demonstration. This provider has an
# explicit lease, bounded event reads and VM-session cleanup; no GUI dependency.
set(_console_sources rxconsole.c rxconsole_core.c rxconsole_decode.c)
add_dynamic_plugin_target(console ${_console_sources})
add_static_plugin_target(console ${_console_sources})
foreach(target IN ITEMS console console_static)
    target_link_libraries(${target} PRIVATE Threads::Threads)
endforeach()
add_rxpa_provider_package(console)
if(BUILD_TESTING)
    add_custom_target(rxconsole_test_artifacts ALL)
    add_executable(rxconsole_decoder_test tests/test_decoder.c rxconsole_decode.c)
    target_include_directories(rxconsole_decoder_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME rxconsole_decoder COMMAND rxconsole_decoder_test)
    set_tests_properties(rxconsole_decoder PROPERTIES LABELS "library;console;input;regression")
    crexx_register_test_prep_targets(TESTS rxconsole_decoder TARGETS rxconsole_decoder_test)
    add_dependencies(rxconsole_test_artifacts rxconsole_decoder_test)
    crexx_add_rexx_opt_matrix(
            NAME rxconsole_levelb RUNNER rxvm
            SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_console.crexx
            TARGET_GROUP rxconsole_test_artifacts
            IMPORT_PATHS ${CMAKE_BINARY_DIR}/bin
            DEPENDS console console_static library
            RUNTIME_ARGS ${CMAKE_BINARY_DIR}/bin/library
            TEST_LABELS "library;console;levelb;rxpa"
            REQUIRED_OUTPUT_REGEX "PASS: rxconsole Level B contract"
            FORBIDDEN_OUTPUT_REGEX "FAIL:|PANIC:")
    if(UNIX)
        find_package(Python3 COMPONENTS Interpreter REQUIRED)
        add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/console_pty.rxbin"
                COMMAND $<TARGET_FILE:rxc> --no-exe-import -i "${CMAKE_BINARY_DIR}/bin"
                        -o console_pty "${CMAKE_CURRENT_SOURCE_DIR}/tests/console_pty.crexx"
                COMMAND $<TARGET_FILE:rxas> -o console_pty console_pty
                DEPENDS rxc rxas rxvm compiler_exit_bin library console
                        "$<TARGET_FILE:console>" tests/console_pty.crexx
                WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
        add_custom_target(rxconsole_pty_artifact DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/console_pty.rxbin")
        add_dependencies(rxconsole_test_artifacts rxconsole_pty_artifact)
        add_test(NAME rxconsole_pty
                COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/pty_checks.py
                        --build ${CMAKE_BINARY_DIR} --source ${CMAKE_SOURCE_DIR} --mode console)
        set_tests_properties(rxconsole_pty PROPERTIES TIMEOUT 60 LABELS "console;pty;regression")
        crexx_register_test_prep_targets(TESTS rxconsole_pty TARGETS rxconsole_test_artifacts)
    endif()
endif()
install(FILES README.md DESTINATION share/crexx/rxconsole)
