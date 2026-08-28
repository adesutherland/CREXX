cmake_minimum_required(VERSION 3.24)

project(PLATFORM C)

set(CMAKE_C_STANDARD 90)

add_library(platform platform.h platform.c rxinteger.h rxinteger.c)
target_include_directories(platform PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
if(WIN32)
    target_link_libraries(platform PUBLIC psapi)
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
