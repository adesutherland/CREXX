cmake_minimum_required(VERSION 3.00)

project(Interpreter C)

set(CMAKE_C_STANDARD 90)

add_executable(rxvm rxvmmain.c rxvmintp.h rxvmintp.c)

add_dependencies(rxvm machine)

target_link_libraries(rxvm machine avl_tree platform)

target_include_directories(rxvm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../assembler ../machine ../platform)
