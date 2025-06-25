cmake_minimum_required(VERSION 3.24)
project(precomp_plugin C)

set(CMAKE_C_STANDARD 99)

# Include RXPA Build System
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)

# Create the static link module
add_static_plugin_target(precomp precomp.c)

# Create the dynamic link module - this is needed for rxc to compile rxpp.rexx
add_dynamic_plugin_target(precomp precomp.c)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/compiler/rxc -i "\"${CMAKE_BINARY_DIR}/lib/rxfnsb;${CMAKE_CURRENT_BINARY_DIR}\"" -o rxpp ${CMAKE_CURRENT_SOURCE_DIR}/rxpp
        COMMAND ${CMAKE_BINARY_DIR}/assembler/rxas rxpp
        COMMAND ${CMAKE_BINARY_DIR}/cpacker/rxcpack rxpp ${CMAKE_BINARY_DIR}/lib/rxfnsb/library
        DEPENDS precomp rxas rxc rxpp.rexx library rxcpack rxpa
        OUTPUT rxpp.c
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "rxpp compile, assemble and package rxpp ..."
)

add_executable(rxpp rxpp.c)
target_link_libraries(rxpp precomp_static rxvml rxpa machine avl_tree platform m)
add_dependencies(rxpp rxvml machine rxpa avl_tree platform)
# This is needed to ensure that the static library is linked into the executable and ** initalized at runtime **
configure_linker_for_static_lib(rxpp precomp)

add_custom_target(rxppcopy ALL DEPENDS ${CMAKE_BINARY_DIR}/rxpp)

add_custom_command(TARGET rxppcopy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/lib/plugins/precomp/rxpp${EXE_SUFFIX} ${CMAKE_BINARY_DIR}/bin/
)
