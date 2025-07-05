cmake_minimum_required(VERSION 3.24)

project(cpacker C)

set(CMAKE_C_STANDARD 90)

add_executable(rxcpack rxcpack.c)

add_custom_target(rxcpackcopy ALL DEPENDS ${CMAKE_BINARY_DIR}/rxcpack)

add_custom_command(TARGET rxcpackcopy POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/cpacker/rxcpack${EXE_SUFFIX} ${CMAKE_BINARY_DIR}/bin/
)
