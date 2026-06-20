cmake_minimum_required(VERSION 3.24)

project(driver C)

set(CMAKE_C_STANDARD 90)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
file(WRITE ${CMAKE_BINARY_DIR}/bin/crexx_native_libs "${CREXX_NATIVE_RUNTIME_LINK_FLAGS_TEXT}\n")
set(CREXX_NATIVE_C_FLAGS_TEXT "${CMAKE_C_FLAGS}")
set(CREXX_NATIVE_EXE_LINKER_FLAGS_TEXT "${CMAKE_EXE_LINKER_FLAGS}")
if(CMAKE_BUILD_TYPE)
    string(TOUPPER "${CMAKE_BUILD_TYPE}" CREXX_NATIVE_BUILD_TYPE)
    string(APPEND CREXX_NATIVE_C_FLAGS_TEXT " ${CMAKE_C_FLAGS_${CREXX_NATIVE_BUILD_TYPE}}")
    string(APPEND CREXX_NATIVE_EXE_LINKER_FLAGS_TEXT " ${CMAKE_EXE_LINKER_FLAGS_${CREXX_NATIVE_BUILD_TYPE}}")
endif()
string(STRIP "${CREXX_NATIVE_C_FLAGS_TEXT}" CREXX_NATIVE_C_FLAGS_TEXT)
string(STRIP "${CREXX_NATIVE_EXE_LINKER_FLAGS_TEXT}" CREXX_NATIVE_EXE_LINKER_FLAGS_TEXT)
file(WRITE ${CMAKE_BINARY_DIR}/bin/crexx_native_cflags "${CREXX_NATIVE_C_FLAGS_TEXT}\n")
file(WRITE ${CMAKE_BINARY_DIR}/bin/crexx_native_ldflags "${CREXX_NATIVE_EXE_LINKER_FLAGS_TEXT}\n")

add_custom_command(
        BYPRODUCTS
                ${CMAKE_CURRENT_BINARY_DIR}/crexx.rxbin
                ${CMAKE_CURRENT_BINARY_DIR}/crexx_linked.rxbin
        COMMAND $<TARGET_FILE:rxc> -i "\"${CMAKE_BINARY_DIR}/bin;${CMAKE_BINARY_DIR}/lib/plugins/system\"" -o crexx ${CMAKE_CURRENT_SOURCE_DIR}/crexx
        COMMAND $<TARGET_FILE:rxas> crexx
        COMMAND $<TARGET_FILE:rxlink> -s -o crexx_linked crexx ${CMAKE_BINARY_DIR}/bin/library
        COMMAND $<TARGET_FILE:rxcpack> -o crexx crexx_linked
        DEPENDS rxas rxc rxlink crexx.crexx library ${CMAKE_BINARY_DIR}/bin/library.rxbin rxfnsc ${CMAKE_BINARY_DIR}/bin/rxfnsc.rxbin rexxscript ${CMAKE_BINARY_DIR}/bin/rexxscript.rxbin rxcpack rxpa _system _system_static compiler_exit_bin
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/crexx.c
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX compile, assemble, link and package crexx ..."
)

add_executable(crexx ${CMAKE_CURRENT_BINARY_DIR}/crexx.c)

configure_linker_for_static_lib_rel(crexx system _system)

target_link_libraries(crexx rxvml rxpashim)

add_dependencies(crexx rxvme rxvml rxpa avl_tree platform _system _system_static)

add_subdirectory(tests)
