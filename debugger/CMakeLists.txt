cmake_minimum_required(VERSION 3.24)

project(Debugger C)

set(CMAKE_C_STANDARD 90)

add_custom_command(
        COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin -o rxdb ${CMAKE_CURRENT_SOURCE_DIR}/rxdb
        COMMAND $<TARGET_FILE:rxas> rxdb
        COMMAND $<TARGET_FILE:rxcpack> rxdb ${CMAKE_BINARY_DIR}/bin/library
        DEPENDS rxas rxc rxdb.rexx library compiler_exit_bin rxcpack
        OUTPUT rxdb.c
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX compile, assemble and package rxdb ..."
)

add_executable(rxdb rxdb.c)

target_link_libraries(rxdb rxvml rxpashim)

add_dependencies(rxdb rxc rxas rxcpack library)
