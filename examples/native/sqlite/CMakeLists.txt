cmake_minimum_required(VERSION 3.24)

set(_sqlite_address_demo_base
        "${CMAKE_BINARY_DIR}/examples/native/sqlite/sqlite_address_demo")
add_custom_command(
        OUTPUT "${_sqlite_address_demo_base}.rxbin"
        BYPRODUCTS "${_sqlite_address_demo_base}.rxas"
        COMMAND ${CMAKE_COMMAND} -E make_directory
                "${CMAKE_BINARY_DIR}/examples/native/sqlite"
        COMMAND $<TARGET_FILE:rxc>
                -i "${CMAKE_BINARY_DIR}/bin"
                -o "${_sqlite_address_demo_base}"
                "${CMAKE_CURRENT_SOURCE_DIR}/sqlite_address_demo.crexx"
        COMMAND $<TARGET_FILE:rxas>
                -o "${_sqlite_address_demo_base}"
                "${_sqlite_address_demo_base}"
        DEPENDS rxc rxas library classlib rxsqlite_address sqlite
                "${CMAKE_CURRENT_SOURCE_DIR}/sqlite_address_demo.crexx"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Building the rxsqlite ADDRESS consumer demo ..."
        VERBATIM)
add_custom_target(sqlite_address_demo_bin
        DEPENDS "${_sqlite_address_demo_base}.rxbin")

set(_dictionary_base
        "${CMAKE_BINARY_DIR}/examples/native/sqlite/data_dictionary")
add_custom_command(
        OUTPUT "${_dictionary_base}.rxbin"
        BYPRODUCTS "${_dictionary_base}.rxas"
        COMMAND ${CMAKE_COMMAND} -E make_directory
                "${CMAKE_BINARY_DIR}/examples/native/sqlite"
        COMMAND $<TARGET_FILE:rxc>
                -i "${CMAKE_BINARY_DIR}/bin"
                -o "${_dictionary_base}"
                "${CMAKE_CURRENT_SOURCE_DIR}/data_dictionary.crexx"
        COMMAND $<TARGET_FILE:rxas>
                -o "${_dictionary_base}"
                "${_dictionary_base}"
        DEPENDS rxc rxas library sqlite
                "${CMAKE_CURRENT_SOURCE_DIR}/data_dictionary.crexx"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Building the SQLite data dictionary module ..."
        VERBATIM)

set(_dictionary_test_base
        "${CMAKE_BINARY_DIR}/examples/native/sqlite/data_dictionary_test")
add_custom_command(
        OUTPUT "${_dictionary_test_base}.rxbin"
        BYPRODUCTS "${_dictionary_test_base}.rxas"
        COMMAND ${CMAKE_COMMAND} -E make_directory
                "${CMAKE_BINARY_DIR}/examples/native/sqlite"
        COMMAND $<TARGET_FILE:rxc>
                -i "${CMAKE_BINARY_DIR}/bin;${CMAKE_BINARY_DIR}/examples/native/sqlite"
                -o "${_dictionary_test_base}"
                "${CMAKE_CURRENT_SOURCE_DIR}/data_dictionary_test.crexx"
        COMMAND $<TARGET_FILE:rxas>
                -o "${_dictionary_test_base}"
                "${_dictionary_test_base}"
        DEPENDS rxc rxas library sqlite "${_dictionary_base}.rxbin"
                "${CMAKE_CURRENT_SOURCE_DIR}/data_dictionary_test.crexx"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Building the SQLite data dictionary check ..."
        VERBATIM)
add_custom_target(sqlite_data_dictionary_test_bin
        DEPENDS "${_dictionary_test_base}.rxbin")

set(_dictionary_facade_test_base
        "${CMAKE_BINARY_DIR}/examples/native/sqlite/data_dictionary_facade_test")
set(_dictionary_facade_base
        "${CMAKE_BINARY_DIR}/examples/native/sqlite/data_dictionary_facade")
add_custom_command(
        OUTPUT "${_dictionary_facade_base}.rxbin"
        BYPRODUCTS "${_dictionary_facade_base}.rxas"
        COMMAND ${CMAKE_COMMAND} -E make_directory
                "${CMAKE_BINARY_DIR}/examples/native/sqlite"
        COMMAND $<TARGET_FILE:rxc>
                -i "${CMAKE_BINARY_DIR}/bin"
                -o "${_dictionary_facade_base}"
                "${CMAKE_CURRENT_SOURCE_DIR}/data_dictionary_facade.crexx"
        COMMAND $<TARGET_FILE:rxas>
                -o "${_dictionary_facade_base}"
                "${_dictionary_facade_base}"
        DEPENDS rxc rxas library sqlite "${_dictionary_base}.rxbin"
                "${CMAKE_CURRENT_SOURCE_DIR}/data_dictionary_facade.crexx"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Building the procedural SQLite data dictionary facade ..."
        VERBATIM)
add_custom_target(sqlite_data_dictionary_facade_bin
        DEPENDS "${_dictionary_facade_base}.rxbin")
add_custom_command(
        OUTPUT "${_dictionary_facade_test_base}.rxbin"
        BYPRODUCTS "${_dictionary_facade_test_base}.rxas"
        COMMAND ${CMAKE_COMMAND} -E make_directory
                "${CMAKE_BINARY_DIR}/examples/native/sqlite"
        COMMAND $<TARGET_FILE:rxc>
                -i "${CMAKE_BINARY_DIR}/bin;${CMAKE_BINARY_DIR}/examples/native/sqlite"
                -o "${_dictionary_facade_test_base}"
                "${CMAKE_CURRENT_SOURCE_DIR}/data_dictionary_facade_test.crexx"
        COMMAND $<TARGET_FILE:rxas>
                -o "${_dictionary_facade_test_base}"
                "${_dictionary_facade_test_base}"
        DEPENDS rxc rxas library sqlite "${_dictionary_base}.rxbin"
                "${_dictionary_facade_base}.rxbin"
                "${CMAKE_CURRENT_SOURCE_DIR}/data_dictionary_facade.crexx"
                "${CMAKE_CURRENT_SOURCE_DIR}/data_dictionary_facade_test.crexx"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Building the procedural SQLite data dictionary facade check ..."
        VERBATIM)
add_custom_target(sqlite_data_dictionary_facade_test_bin
        DEPENDS "${_dictionary_facade_test_base}.rxbin")

set(_dictionary_report_base
        "${CMAKE_BINARY_DIR}/examples/native/sqlite/data_dictionary_report")
add_custom_command(
        OUTPUT "${_dictionary_report_base}.rxbin"
        BYPRODUCTS "${_dictionary_report_base}.rxas"
        COMMAND ${CMAKE_COMMAND} -E make_directory
                "${CMAKE_BINARY_DIR}/examples/native/sqlite"
        COMMAND $<TARGET_FILE:rxc>
                -i "${CMAKE_BINARY_DIR}/bin"
                -o "${_dictionary_report_base}"
                "${CMAKE_CURRENT_SOURCE_DIR}/data_dictionary_report.crexx"
        COMMAND $<TARGET_FILE:rxas>
                -o "${_dictionary_report_base}"
                "${_dictionary_report_base}"
        DEPENDS rxc rxas library
                "${CMAKE_CURRENT_SOURCE_DIR}/data_dictionary_report.crexx"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Building the SQLite data dictionary reporting module ..."
        VERBATIM)
add_custom_target(sqlite_data_dictionary_report_bin
        DEPENDS "${_dictionary_report_base}.rxbin")

set(_dictionary_report_test_base
        "${CMAKE_BINARY_DIR}/examples/native/sqlite/data_dictionary_report_test")
add_custom_command(
        OUTPUT "${_dictionary_report_test_base}.rxbin"
        BYPRODUCTS "${_dictionary_report_test_base}.rxas"
        COMMAND ${CMAKE_COMMAND} -E make_directory
                "${CMAKE_BINARY_DIR}/examples/native/sqlite"
        COMMAND $<TARGET_FILE:rxc>
                -i "${CMAKE_BINARY_DIR}/bin;${CMAKE_BINARY_DIR}/examples/native/sqlite"
                -o "${_dictionary_report_test_base}"
                "${CMAKE_CURRENT_SOURCE_DIR}/data_dictionary_report_test.crexx"
        COMMAND $<TARGET_FILE:rxas>
                -o "${_dictionary_report_test_base}"
                "${_dictionary_report_test_base}"
        DEPENDS rxc rxas library "${_dictionary_report_base}.rxbin"
                "${CMAKE_CURRENT_SOURCE_DIR}/data_dictionary_report_test.crexx"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Building the SQLite data dictionary reporting check ..."
        VERBATIM)
add_custom_target(sqlite_data_dictionary_report_test_bin
        DEPENDS "${_dictionary_report_test_base}.rxbin")

set(_dictionary_sample_base
        "${CMAKE_BINARY_DIR}/examples/native/sqlite/data_dictionary_sample")
add_custom_command(
        OUTPUT "${_dictionary_sample_base}.rxbin"
        BYPRODUCTS "${_dictionary_sample_base}.rxas"
        COMMAND ${CMAKE_COMMAND} -E make_directory
                "${CMAKE_BINARY_DIR}/examples/native/sqlite"
        COMMAND $<TARGET_FILE:rxc>
                -i "${CMAKE_BINARY_DIR}/bin;${CMAKE_BINARY_DIR}/examples/native/sqlite"
                -o "${_dictionary_sample_base}"
                "${CMAKE_CURRENT_SOURCE_DIR}/data_dictionary_sample.crexx"
        COMMAND $<TARGET_FILE:rxas>
                -o "${_dictionary_sample_base}"
                "${_dictionary_sample_base}"
        DEPENDS rxc rxas library sqlite "${_dictionary_base}.rxbin"
                "${_dictionary_report_base}.rxbin"
                "${CMAKE_CURRENT_SOURCE_DIR}/data_dictionary_sample.crexx"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Building the SQLite data dictionary sample ..."
        VERBATIM)
add_custom_target(sqlite_data_dictionary_sample_bin
        DEPENDS "${_dictionary_sample_base}.rxbin")
