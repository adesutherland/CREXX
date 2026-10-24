cmake_minimum_required(VERSION 3.24)

set(CREXX_EXAMPLE_ARTIFACT_ROOT "${CMAKE_BINARY_DIR}/example-artifacts")
set(CREXX_ARRAY_FORMATTING_MODULE
    "${CREXX_EXAMPLE_ARTIFACT_ROOT}/functions/array-formatting/arrayformatdemo.rxbin")

add_subdirectory(functions/array-formatting)
add_subdirectory(exits/pprint)
add_subdirectory(ui/text-inspector)
add_subdirectory(franz-lotto)
add_subdirectory(fortran)
add_subdirectory(forth)
add_subdirectory(php)

add_custom_target(crexx-examples)
add_dependencies(crexx-examples
        example_array_formatting_artifacts
        example_pprint_exit_artifacts
        example_franz_lotto_artifacts
        example_text_inspector_artifacts)

if(BUILD_TESTING)
    add_custom_target(qa-prep-examples)
    add_dependencies(qa-prep-examples
            example_array_formatting_artifacts
            example_pprint_exit_artifacts
            example_franz_lotto_artifacts
            example_text_inspector_artifacts)
endif()

# Install source demonstrations alongside their category documentation. Built
# example modules are installed by their owning subdirectories.
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
        DESTINATION examples
        FILES_MATCHING
        PATTERN "*.crexx"
        PATTERN "*.rxpp"
        PATTERN "*.rexx"
        PATTERN "*.c"
        PATTERN "*.h"
        PATTERN "*.md")

# Existing demo targets remain compatible; sources have one examples owner.
add_subdirectory(llm)
add_subdirectory(native/sqlite)
add_custom_target(crexx-demos)
add_dependencies(crexx-demos llm_generate_demos_bin llm_address_demos_bin sqlite_address_demo_bin)
