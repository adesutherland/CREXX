cmake_minimum_required(VERSION 3.24)

project(Debugger C)

set(CMAKE_C_STANDARD 90)

add_custom_command(
        BYPRODUCTS
                ${CMAKE_CURRENT_BINARY_DIR}/rxdb_gui.rxas
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o rxdb_gui ${CMAKE_CURRENT_SOURCE_DIR}/rxdb_gui
        COMMAND $<TARGET_FILE:rxas> rxdb_gui
        DEPENDS rxas rxc rxdb_gui.crexx library compiler_exit_bin
        OUTPUT rxdb_gui.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX compile and assemble rxdb_gui ..."
)

add_custom_command(
        BYPRODUCTS
                ${CMAKE_CURRENT_BINARY_DIR}/rxdb.rxbin
                ${CMAKE_CURRENT_BINARY_DIR}/rxdb_linked.rxbin
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -i ${CMAKE_CURRENT_BINARY_DIR} -o rxdb ${CMAKE_CURRENT_SOURCE_DIR}/rxdb
        COMMAND $<TARGET_FILE:rxas> rxdb
        COMMAND $<TARGET_FILE:rxlink> -s -o rxdb_linked rxdb rxdb_gui ${CMAKE_BINARY_DIR}/bin/library
        COMMAND $<TARGET_FILE:rxcpack> -o rxdb rxdb_linked
        DEPENDS rxas rxc rxlink rxdb.crexx rxdb_gui.rxbin library compiler_exit_bin rxcpack
        OUTPUT rxdb.c
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX compile, assemble, link and package rxdb ..."
)

add_executable(rxdb rxdb.c)

target_link_libraries(rxdb rxvml rxpashim)

add_dependencies(rxdb rxc rxas rxlink rxcpack library)

if(BUILD_TESTING)
    add_test(NAME rxdb_usage_smoke
             COMMAND $<TARGET_FILE:rxdb> --help)
    set_tests_properties(rxdb_usage_smoke
            PROPERTIES PASS_REGULAR_EXPRESSION "Usage: rxdb \\[text\\|plain\\|llm\\]")
endif()
