cmake_minimum_required(VERSION 3.24)

project(PLATFORM C)

set(CMAKE_C_STANDARD 90)

add_library(platform platform.h platform.c rxinteger.h rxinteger.c)
target_include_directories(platform PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
if(WIN32)
    target_link_libraries(platform PUBLIC psapi)
endif()

if(UNIX AND BUILD_TESTING)
    # Deliberately retain Linux aliases to test profile precedence on both hosts.
    add_executable(platform_cms_harness platform_cms_harness.c platform.c)
    target_compile_definitions(platform_cms_harness PRIVATE
            __MAINFRAME_LAB_VM370_4381__=1 __linux__=1 __linux=1)
    add_test(NAME platform_cms_selection COMMAND platform_cms_harness)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cms24")
    set_tests_properties(platform_cms_selection PROPERTIES
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cms24"
            LABELS "platform;cms" TIMEOUT 30)
    add_executable(platform_cms31_harness platform_cms_harness.c platform.c)
    target_compile_definitions(platform_cms31_harness PRIVATE
            __MAINFRAME_LAB_CMS20_ESA31__=1 __linux__=1 __linux=1)
    add_test(NAME platform_cms31_selection COMMAND platform_cms31_harness)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cms31")
    set_tests_properties(platform_cms31_selection PROPERTIES
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cms31"
            LABELS "platform;cms" TIMEOUT 30)
    crexx_register_test_prep_targets(TESTS platform_cms_selection
            TARGETS platform_cms_harness)
    crexx_register_test_prep_targets(TESTS platform_cms31_selection
            TARGETS platform_cms31_harness)
endif()

if((APPLE OR CMAKE_SYSTEM_NAME STREQUAL "Linux") AND BUILD_TESTING)
    # Link the real front ends to an injected text-stream provider. These are
    # host interface tests, not an implementation of the CMS runtime/code page.
    foreach(tool IN ITEMS rxc rxas)
        if(tool STREQUAL rxc)
            set(entry "${CMAKE_SOURCE_DIR}/compiler/rxc_main.c")
            set(frontend rxclib)
        else()
            set(entry "${CMAKE_SOURCE_DIR}/assembler/rxasmain.c")
            set(frontend rxaslib)
        endif()
        add_executable(${tool}_cms_text_harness EXCLUDE_FROM_ALL
                ${entry} platform.c rxinteger.c tests/cms_text_mock.c)
        target_include_directories(${tool}_cms_text_harness PRIVATE .
                "${CMAKE_SOURCE_DIR}/compiler" "${CMAKE_SOURCE_DIR}/assembler"
                "${CMAKE_BINARY_DIR}/assembler"
                "${CMAKE_SOURCE_DIR}/binutils/include")
        target_compile_definitions(${tool}_cms_text_harness PRIVATE
                __MAINFRAME_LAB_CMS20_ESA31__=1 CREXX_CMS_TEXT_IO=1 CREXX_CMS_DIRENT=1)
        set_property(TARGET ${tool}_cms_text_harness PROPERTY C_STANDARD 99)
        target_link_libraries(${tool}_cms_text_harness PRIVATE ${frontend})
        if(tool STREQUAL rxas)
            target_link_libraries(${tool}_cms_text_harness PRIVATE avl_tree)
        endif()
    endforeach()
    add_executable(platform_cms_directory_harness tests/cms_directory.c platform.c)
    target_include_directories(platform_cms_directory_harness PRIVATE .)
    target_compile_definitions(platform_cms_directory_harness PRIVATE
            __MAINFRAME_LAB_CMS20_ESA31__=1 CREXX_CMS_DIRENT=1 __linux__=1)
    set_property(TARGET platform_cms_directory_harness PROPERTY C_STANDARD 99)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cms-directory")
    add_test(NAME platform_cms_directory COMMAND platform_cms_directory_harness)
    set_tests_properties(platform_cms_directory PROPERTIES LABELS "platform;cms"
            TIMEOUT 30 WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/cms-directory")
    crexx_register_test_prep_targets(TESTS platform_cms_directory
            TARGETS platform_cms_directory_harness)

    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(cms_text_command "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/cms_text.py"
            --rxc $<TARGET_FILE:rxc> --rxas $<TARGET_FILE:rxas>
            --mock-rxc $<TARGET_FILE:rxc_cms_text_harness>
            --mock-rxas $<TARGET_FILE:rxas_cms_text_harness>
            --link $<TARGET_FILE:rxlink> --vm $<TARGET_FILE:rxbvm>
            --work "${CMAKE_CURRENT_BINARY_DIR}/cms-text")
    # Explicit target also permits isolated Debug/sanitizer timing.
    add_custom_target(check_cms_text COMMAND ${cms_text_command}
            DEPENDS rxc rxas rxlink rxbvm rxc_cms_text_harness rxas_cms_text_harness
            VERBATIM)
    add_test(NAME platform_cms_text COMMAND ${cms_text_command})
    # Native Debug: 1.82 s; maintained Apple ASan: 5.21 s (2026-09-21).
    # Serialize nested tool invocations; the deadline is hang protection.
    set_tests_properties(platform_cms_text PROPERTIES LABELS "platform;cms;text"
            RUN_SERIAL TRUE TIMEOUT 300)
    crexx_register_test_prep_targets(TESTS platform_cms_text
            TARGETS rxc rxas rxlink rxbvm rxc_cms_text_harness rxas_cms_text_harness)
endif()

if(BUILD_TESTING)
    crexx_schedule_directory_qa_tier_finalization()
endif()

if((APPLE OR CMAKE_SYSTEM_NAME STREQUAL "Linux") AND BUILD_TESTING)
    add_executable(platform_terminal_harness platform_terminal_harness.c)
    target_link_libraries(platform_terminal_harness PRIVATE platform)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_link_libraries(platform_terminal_harness PRIVATE util)
    endif()

    add_test(NAME platform_terminal_restore
            COMMAND $<TARGET_FILE:platform_terminal_harness> restore)
    set_tests_properties(platform_terminal_restore PROPERTIES
            LABELS "platform;terminal;tty"
            PASS_REGULAR_EXPRESSION "PASS: terminal restore semantics"
            TIMEOUT 10)

    add_test(NAME platform_terminal_disconnect
            COMMAND $<TARGET_FILE:platform_terminal_harness> disconnect)
    set_tests_properties(platform_terminal_disconnect PROPERTIES
            LABELS "platform;terminal;tty;disconnect"
            PASS_REGULAR_EXPRESSION "PASS: terminal disconnect semantics"
            TIMEOUT 10)
endif()
