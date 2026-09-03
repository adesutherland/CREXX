cmake_minimum_required(VERSION 3.24)

set(_sqlite_address_demo_base
        "${CMAKE_BINARY_DIR}/demos/native/sqlite/sqlite_address_demo")
add_custom_command(
        OUTPUT "${_sqlite_address_demo_base}.rxbin"
        BYPRODUCTS "${_sqlite_address_demo_base}.rxas"
        COMMAND ${CMAKE_COMMAND} -E make_directory
                "${CMAKE_BINARY_DIR}/demos/native/sqlite"
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
