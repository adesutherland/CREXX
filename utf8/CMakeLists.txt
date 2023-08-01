cmake_minimum_required(VERSION 3.14)

project(Compiler C)

set(CMAKE_C_STANDARD 90)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/re2c/re2c -I ${CMAKE_CURRENT_SOURCE_DIR}/../utf8 -8 --input-encoding utf8 -o ${CMAKE_CURRENT_BINARY_DIR}/char_seg.c ${CMAKE_CURRENT_SOURCE_DIR}/char_seg.re
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../utf8/encoding.re char_seg.re re2c
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/char_seg.c
        COMMENT "Create Grapheme Segmentator ..."
)

add_executable(char_seg utftest.c GraphemeBreakTest.c ${CMAKE_CURRENT_BINARY_DIR}/char_seg.c)

add_dependencies(char_seg re2c)
