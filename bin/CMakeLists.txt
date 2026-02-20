cmake_minimum_required(VERSION 3.24)

project(driver C)

set(CMAKE_C_STANDARD 90)

add_custom_command(
        COMMAND $<TARGET_FILE:rxc> -i "\"${CMAKE_BINARY_DIR}/bin;${CMAKE_BINARY_DIR}/lib/plugins/sysinfo\"" -o crexx ${CMAKE_CURRENT_SOURCE_DIR}/crexx
        COMMAND $<TARGET_FILE:rxas> crexx
        COMMAND $<TARGET_FILE:rxcpack> crexx ${CMAKE_BINARY_DIR}/bin/library
        DEPENDS rxas rxc crexx.rexx library rxcpack rxpa _sysinfo
        OUTPUT crexx.c
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX compile, assemble and package crexx ..."
)

add_executable(crexx crexx.c)

configure_linker_for_static_lib_rel(crexx sysinfo _sysinfo)

target_link_libraries(crexx rxvml rxpashim)

add_dependencies(crexx rxvml rxpa avl_tree platform _sysinfo)
