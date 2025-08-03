cmake_minimum_required(VERSION 3.24)

project(Debugger C)

set(CMAKE_C_STANDARD 90)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc -i ${CMAKE_BINARY_DIR}/lib/rxfnsb -o rxdb ${CMAKE_CURRENT_SOURCE_DIR}/rxdb
        COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas rxdb
        COMMAND ${CMAKE_BINARY_DIR}/cpacker/rxcpack rxdb ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        DEPENDS rxas rxc rxdb.rexx library rxcpack
        OUTPUT rxdb.c
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX compile, assemble and package rxdb ..."
)

add_executable(rxdb rxdb.c)

target_link_libraries(rxdb rxvml machine avl_tree rxpa platform m)

add_dependencies(rxdb rxc rxas rxcpack library)

add_custom_target(rxdbcopy ALL DEPENDS rxdb)

add_custom_command(TARGET rxdbcopy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/debugger/rxdb${EXE_SUFFIX} ${CMAKE_BINARY_DIR}/bin/
)
