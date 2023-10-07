cmake_minimum_required(VERSION 3.14)

project(Compiler C)

set(CMAKE_C_STANDARD 90)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/re2c/re2c -I ${CMAKE_CURRENT_SOURCE_DIR}/../utf8 -8 --input-encoding utf8 -o ${CMAKE_CURRENT_BINARY_DIR}/codepoint_lex.c ${CMAKE_CURRENT_SOURCE_DIR}/codepoint_lex.re
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../utf8/encoding.re codepoint_lex.re re2c
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/codepoint_lex.c
        COMMENT "Create Codepoint Segmentator ..."
)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/re2c/re2c -I ${CMAKE_CURRENT_SOURCE_DIR}/../utf8 -8 --input-encoding utf8 -o ${CMAKE_CURRENT_BINARY_DIR}/grapheme_lex.c ${CMAKE_CURRENT_SOURCE_DIR}/grapheme_lex.re
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../utf8/encoding.re grapheme_lex.re re2c
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/grapheme_lex.c
        COMMENT "Create Grapheme Segmentator ..."
)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/re2c/re2c -I ${CMAKE_CURRENT_SOURCE_DIR}/../utf8 -8 --input-encoding utf8 -o ${CMAKE_CURRENT_BINARY_DIR}/word_lex.c ${CMAKE_CURRENT_SOURCE_DIR}/word_lex.re
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../utf8/encoding.re word_lex.re re2c
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/word_lex.c
        COMMENT "Create Word Segmentator ..."
)

add_custom_command(
        COMMAND ${CMAKE_BINARY_DIR}/re2c/re2c -I ${CMAKE_CURRENT_SOURCE_DIR}/../utf8 -8 --input-encoding utf8 -o ${CMAKE_CURRENT_BINARY_DIR}/norm_unicode.c ${CMAKE_CURRENT_SOURCE_DIR}/norm_unicode.re
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../utf8/encoding.re norm_unicode.re re2c
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/norm_unicode.c
        COMMENT "Create Normaliser ..."
)

add_executable(utftest utftest.c GraphemeBreakTest.c WordBreakTest.c ${CMAKE_CURRENT_BINARY_DIR}/codepoint_lex.c ${CMAKE_CURRENT_BINARY_DIR}/grapheme_lex.c ${CMAKE_CURRENT_BINARY_DIR}/word_lex.c)
add_dependencies(utftest re2c)

add_executable(NormalizationTest NormalizationTest.c ${CMAKE_CURRENT_BINARY_DIR}/norm_unicode.c)
add_dependencies(NormalizationTest re2c)
