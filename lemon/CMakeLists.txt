cmake_minimum_required(VERSION 3.24)
project(Lemon C)

set(CMAKE_C_STANDARD 90)

enable_testing()

add_executable(lemon lemon.c)

# Suppress noisy warnings from external dependency when using Clang
if (CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(lemon PRIVATE
        -Wno-switch
        -Wno-switch-default
        -Wno-reserved-identifier
        -Wno-poison-system-directories
    )
endif()

add_custom_command(
        TARGET lemon PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/lempar.c
        ${CMAKE_CURRENT_BINARY_DIR}/lempar.c
)
