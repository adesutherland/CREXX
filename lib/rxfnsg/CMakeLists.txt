cmake_minimum_required(VERSION 3.24)

set(RXUNICODE_CASEFOLD_DATA
  "${CMAKE_CURRENT_SOURCE_DIR}/unicode/data/unicode-17.0.0/CaseFolding.txt")
set(RXUNICODE_CASEFOLD_TOOL_DIR
  "${CMAKE_CURRENT_BINARY_DIR}/rexx/unicode-tools")
set(RXUNICODE_CASEFOLD_TABLE_RXBIN
  "${RXUNICODE_CASEFOLD_TOOL_DIR}/casefold_table.rxbin")

add_subdirectory(rexx)
add_subdirectory(tests_functional)
