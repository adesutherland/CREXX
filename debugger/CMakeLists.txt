cmake_minimum_required(VERSION 3.24)

project(Debugger C)

set(CMAKE_C_STANDARD 90)

set(RXDB_IMPORT_ROOT "${CMAKE_CURRENT_BINARY_DIR}/imports/base")
crexx_add_import_root(debugger_imports
        DIRECTORY "${RXDB_IMPORT_ROOT}"
        OUTPUT_VARIABLE RXDB_IMPORT_FILES
        FILES
                "${CMAKE_BINARY_DIR}/bin/library.rxbin"
                "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
        DEPENDS
                library
                compiler_exit_bin
        COMMENT "Staging deterministic debugger imports ...")

set(_rxdb_gui_source_root "${CMAKE_CURRENT_BINARY_DIR}/sources/rxdb_gui")
set(_rxdb_gui_source "${_rxdb_gui_source_root}/rxdb_gui.crexx")
add_custom_command(
        BYPRODUCTS
                ${CMAKE_CURRENT_BINARY_DIR}/rxdb_gui.rxas
                ${_rxdb_gui_source}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_rxdb_gui_source_root}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/rxdb_gui.crexx"
                "${_rxdb_gui_source}"
        COMMAND $<TARGET_FILE:rxc>
                --no-exe-import
                -i "${RXDB_IMPORT_ROOT}"
                -o rxdb_gui
                "${_rxdb_gui_source}"
        COMMAND $<TARGET_FILE:rxas> rxdb_gui
        DEPENDS
                rxas
                rxc
                rxdb_gui.crexx
                debugger_imports
                ${RXDB_IMPORT_FILES}
        OUTPUT rxdb_gui.rxbin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX compile and assemble rxdb_gui ..."
        VERBATIM
)

set(RXDB_GUI_IMPORT_ROOT "${CMAKE_CURRENT_BINARY_DIR}/imports/rxdb_gui")
crexx_add_import_root(debugger_gui_imports
        DIRECTORY "${RXDB_GUI_IMPORT_ROOT}"
        OUTPUT_VARIABLE RXDB_GUI_IMPORT_FILES
        FILES "${CMAKE_CURRENT_BINARY_DIR}/rxdb_gui.rxbin"
        COMMENT "Staging deterministic rxdb GUI metadata ...")

set(_rxdb_source_root "${CMAKE_CURRENT_BINARY_DIR}/sources/rxdb")
set(_rxdb_source "${_rxdb_source_root}/rxdb.crexx")
add_custom_command(
        BYPRODUCTS
                ${CMAKE_CURRENT_BINARY_DIR}/rxdb.rxbin
                ${CMAKE_CURRENT_BINARY_DIR}/rxdb_linked.rxbin
                ${_rxdb_source}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_rxdb_source_root}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/rxdb.crexx"
                "${_rxdb_source}"
        COMMAND $<TARGET_FILE:rxc>
                --no-exe-import
                -i "${RXDB_IMPORT_ROOT}"
                -i "${RXDB_GUI_IMPORT_ROOT}"
                -o rxdb
                "${_rxdb_source}"
        COMMAND $<TARGET_FILE:rxas> rxdb
        COMMAND $<TARGET_FILE:rxlink> -s -o rxdb_linked rxdb rxdb_gui ${CMAKE_BINARY_DIR}/bin/library
        COMMAND $<TARGET_FILE:rxcpack> -o rxdb rxdb_linked
        DEPENDS
                rxas
                rxc
                rxlink
                rxcpack
                rxdb.crexx
                debugger_imports
                ${RXDB_IMPORT_FILES}
                debugger_gui_imports
                ${RXDB_GUI_IMPORT_FILES}
        OUTPUT rxdb.c
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX compile, assemble, link and package rxdb ..."
        VERBATIM
)

add_executable(rxdb rxdb.c)

target_link_libraries(rxdb rxvml rxpashim)

add_dependencies(rxdb rxc rxas rxlink rxcpack library)

if(BUILD_TESTING)
    add_test(NAME rxdb_usage_smoke
             COMMAND $<TARGET_FILE:rxdb> --help)
    set_tests_properties(rxdb_usage_smoke
            PROPERTIES PASS_REGULAR_EXPRESSION "Usage: rxdb \\[text\\|plain\\|llm\\]")
endif()

crexx_finalize_directory_qa_tiers()
