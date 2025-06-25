cmake_minimum_required(VERSION 3.24)

project(Disassembler C)

set(CMAKE_C_STANDARD 90)

add_executable(rxdas rxdamain.c rxdadism.h rxdadism.c ../machine/rxbin.h)

add_dependencies(rxas machine avl_tree platform)

target_link_libraries(rxdas machine avl_tree platform)

target_include_directories(rxdas PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../assembler ../machine ../platform)

add_custom_target(rxdascopy ALL DEPENDS ${CMAKE_BINARY_DIR}/rxc)
add_custom_command(TARGET rxdascopy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/disassembler/rxdas ${CMAKE_BINARY_DIR}/bin/
)
