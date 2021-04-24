cmake_minimum_required(VERSION 3.17)

project(Compiler C)

set(CMAKE_C_STANDARD 90)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/lemon/lemon -c ${CMAKE_CURRENT_SOURCE_DIR}/opt_grmr.y
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/opt_grmr.y lemon
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/opt_grmr.c ${CMAKE_CURRENT_SOURCE_DIR}/opt_grmr.h
        COMMENT "Create Options Parser..."
)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/re2c/re2c -o opt_scan.c ${CMAKE_CURRENT_SOURCE_DIR}/opt_scan.re
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/opt_scan.re ${CMAKE_CURRENT_SOURCE_DIR}/opt_grmr.h re2c
        OUTPUT opt_scan.c
        COMMENT "Create Options Lexer..."
)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/lemon/lemon -p -s -c ${CMAKE_CURRENT_SOURCE_DIR}/rexbgrmr.y || (exit 0)
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rexbgrmr.y lemon
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/rexbgrmr.c ${CMAKE_CURRENT_SOURCE_DIR}/rexbgrmr.h
        COMMENT "Create REXX B Parser..."
)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/re2c/re2c -o rexbscan.c ${CMAKE_CURRENT_SOURCE_DIR}/rexbscan.re
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rexbscan.re ${CMAKE_CURRENT_SOURCE_DIR}/rexbgrmr.h re2c
        OUTPUT rexbscan.c
        COMMENT "Create REXX B Lexer..."
)

add_executable(rexx compiler.c ast.c compiler.h
               opt_scan.c opt_grmr.c opt_pars.c opt_grmr.h
               rexbscan.c rexbgrmr.c rexbpars.c rexbgrmr.h)
add_dependencies(rexx re2c lemon)
target_include_directories(rexx PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
