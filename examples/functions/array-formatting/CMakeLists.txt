cmake_minimum_required(VERSION 3.24)

set(_array_formatting_modules)
foreach(_selector IN ITEMS arraydump arrayformat)
    set(_module "${CMAKE_CURRENT_BINARY_DIR}/${_selector}.rxbin")
    add_custom_command(
            OUTPUT "${_module}"
            COMMAND $<TARGET_FILE:rxc> -x -i "${CMAKE_BINARY_DIR}/bin"
                    -o "${_selector}" "${CMAKE_CURRENT_SOURCE_DIR}/${_selector}.crexx"
            COMMAND $<TARGET_FILE:rxas> -o "${_selector}" "${_selector}"
            DEPENDS rxc rxas library "${CMAKE_CURRENT_SOURCE_DIR}/${_selector}.crexx"
            BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/${_selector}.rxas"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            COMMENT "Building optimized ${_selector} function example")
    list(APPEND _array_formatting_modules "${_module}")
endforeach()

add_custom_command(
        OUTPUT "${CREXX_ARRAY_FORMATTING_MODULE}"
        COMMAND ${CMAKE_COMMAND} -E make_directory
                "${CREXX_EXAMPLE_ARTIFACT_ROOT}/functions/array-formatting"
        COMMAND ${CMAKE_COMMAND} -E cat ${_array_formatting_modules}
                > "${CREXX_ARRAY_FORMATTING_MODULE}"
        DEPENDS ${_array_formatting_modules}
        COMMENT "Bundling array-formatting function example")

set(_array_formatting_program
    "${CMAKE_CURRENT_BINARY_DIR}/array_formatting_example.rxbin")
add_custom_command(
        OUTPUT "${_array_formatting_program}"
        COMMAND $<TARGET_FILE:rxc> -x
                -i "${CREXX_EXAMPLE_ARTIFACT_ROOT}/functions/array-formatting"
                -i "${CMAKE_BINARY_DIR}/bin"
                -o array_formatting_example
                "${CMAKE_CURRENT_SOURCE_DIR}/array_formatting_example.crexx"
        COMMAND $<TARGET_FILE:rxas> -o array_formatting_example array_formatting_example
        DEPENDS rxc rxas library "${CREXX_ARRAY_FORMATTING_MODULE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/array_formatting_example.crexx"
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/array_formatting_example.rxas"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Building optimized array-formatting example program")

add_custom_target(example_array_formatting_artifacts
        DEPENDS "${CREXX_ARRAY_FORMATTING_MODULE}" "${_array_formatting_program}")

if(BUILD_TESTING)
    add_test(
            NAME example_array_formatting
            COMMAND ${CMAKE_COMMAND}
                    -DRXVM=${CREXX_RXVM_EXECUTABLE}
                    -DPROGRAM=${_array_formatting_program}
                    -DMODULE1=${CREXX_ARRAY_FORMATTING_MODULE}
                    -DMODULE2=${CMAKE_BINARY_DIR}/bin/library.rxbin
                    -DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/array_formatting_example.out
                    -P ${CMAKE_SOURCE_DIR}/examples/RunExample.cmake)
endif()

install(FILES "${CREXX_ARRAY_FORMATTING_MODULE}"
        DESTINATION examples/functions/array-formatting)

crexx_finalize_directory_qa_tiers()
