cmake_minimum_required(VERSION 3.24)

set(CREXX_EXAMPLE_ARTIFACT_ROOT "${CMAKE_BINARY_DIR}/example-artifacts")
set(CREXX_ARRAY_FORMATTING_MODULE
    "${CREXX_EXAMPLE_ARTIFACT_ROOT}/functions/array-formatting/arrayformatdemo.rxbin")

add_subdirectory(functions/array-formatting)
add_subdirectory(exits/pprint)

add_custom_target(crexx-examples)
add_dependencies(crexx-examples
        example_array_formatting_artifacts
        example_pprint_exit_artifacts)

if(BUILD_TESTING)
    add_custom_target(qa-prep-examples)
    add_dependencies(qa-prep-examples
            example_array_formatting_artifacts
            example_pprint_exit_artifacts)
endif()

# Install source demonstrations alongside their category documentation. Built
# example modules are installed by their owning subdirectories.
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
        DESTINATION examples
        FILES_MATCHING
        PATTERN "*.crexx"
        PATTERN "*.rxpp"
        PATTERN "*.md")
