cmake_minimum_required(VERSION 3.24)

set(_pprint_module "${CMAKE_CURRENT_BINARY_DIR}/Pprint.rxbin")
set(_pprint_artifact_dir "${CREXX_EXAMPLE_ARTIFACT_ROOT}/exits/pprint")
set(_pprint_bundle "${_pprint_artifact_dir}/pprint_example.rxbin")

add_custom_command(
        OUTPUT "${_pprint_module}"
        COMMAND ${CMAKE_COMMAND} -E env RXCP_DISABLE_EXIT=1
                $<TARGET_FILE:rxc> -x
                -i "${CMAKE_BINARY_DIR}/compiler/exits"
                -i "${CMAKE_BINARY_DIR}/bin"
                -o Pprint "${CMAKE_CURRENT_SOURCE_DIR}/Pprint.crexx"
        COMMAND $<TARGET_FILE:rxas> -o Pprint Pprint
        DEPENDS rxc rxas library token_bin "${CMAKE_CURRENT_SOURCE_DIR}/Pprint.crexx"
                "${CMAKE_BINARY_DIR}/compiler/exits/token.rxbin"
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/Pprint.rxas"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Building optimized pprint compiler-exit example")

add_custom_command(
        OUTPUT "${_pprint_bundle}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_pprint_artifact_dir}"
        COMMAND ${CMAKE_COMMAND} -E cat
                "${CMAKE_BINARY_DIR}/compiler/exits/token.rxbin"
                "${_pprint_module}" > "${_pprint_bundle}"
        DEPENDS token_bin "${_pprint_module}"
        COMMENT "Bundling standalone pprint compiler-exit example")

set(_pprint_program "${CMAKE_CURRENT_BINARY_DIR}/pprint_example_program.rxbin")
add_custom_command(
        OUTPUT "${_pprint_program}"
        COMMAND ${CMAKE_COMMAND} -E env RXCP_EXIT_MODULE=pprint_example
                $<TARGET_FILE:rxc>
                -i "${_pprint_artifact_dir}"
                -i "${CREXX_EXAMPLE_ARTIFACT_ROOT}/functions/array-formatting"
                -i "${CMAKE_BINARY_DIR}/bin"
                -o pprint_example_program
                "${CMAKE_CURRENT_SOURCE_DIR}/pprint_example.crexx"
        COMMAND $<TARGET_FILE:rxas> -o pprint_example_program pprint_example_program
        DEPENDS rxc rxas library compiler_exit_bin
                "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
                "${_pprint_bundle}"
                "${CREXX_ARRAY_FORMATTING_MODULE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/pprint_example.crexx"
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/pprint_example_program.rxas"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Building program with standalone pprint exit example")

add_custom_target(example_pprint_exit_artifacts ALL
        DEPENDS "${_pprint_bundle}" "${_pprint_program}")
add_dependencies(example_pprint_exit_artifacts example_array_formatting_artifacts)

if(BUILD_TESTING)
    add_test(
            NAME example_pprint_exit_artifacts_build
            COMMAND ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}"
                    --target example_pprint_exit_artifacts)
    set_tests_properties(example_pprint_exit_artifacts_build PROPERTIES
            FIXTURES_SETUP example_pprint_exit_artifacts
            RUN_SERIAL TRUE)

    add_test(
            NAME example_pprint_exit
            COMMAND ${CMAKE_COMMAND}
                    -DRXVM=${CREXX_RXVM_EXECUTABLE}
                    -DPROGRAM=${_pprint_program}
                    -DMODULE1=${CREXX_ARRAY_FORMATTING_MODULE}
                    -DMODULE2=${CMAKE_BINARY_DIR}/bin/library.rxbin
                    -DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/pprint_example.out
                    -P ${CMAKE_SOURCE_DIR}/examples/RunExample.cmake)
    set_tests_properties(example_pprint_exit PROPERTIES
            FIXTURES_REQUIRED example_pprint_exit_artifacts)
endif()

install(FILES "${_pprint_bundle}"
        DESTINATION examples/exits/pprint)
