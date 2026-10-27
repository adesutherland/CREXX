cmake_minimum_required(VERSION 3.24)

# The PHP converter is an example/toolchain project. Generated grammar,
# reader, and translated cREXX artifacts stay in the build directory.
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
        DESTINATION examples/php
        FILES_MATCHING
        PATTERN "*.php"
        PATTERN "*.rxpp"
        PATTERN "*.levell"
        PATTERN "*.crexx"
        PATTERN "*.md")

# This source-only reader control belongs to ordinary QA, not native-memory panels.
if(BUILD_TESTING AND NOT CREXX_QA_AUDIT_NATIVE_SOURCES)
    set(_php_reader "${CMAKE_CURRENT_SOURCE_DIR}/php_reader.crexx")
    set(_php_group_template "${CMAKE_CURRENT_SOURCE_DIR}/php_reader_group_smoke.crexx.in")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_php_reader}")
    file(READ "${_php_reader}" _php_reader_source)
    string(FIND "${_php_reader_source}" "emit_expression: procedure" _php_expression_start)
    string(FIND "${_php_reader_source}" "\nemit_line: procedure" _php_expression_end)
    if(_php_expression_start LESS 0 OR _php_expression_end LESS_EQUAL _php_expression_start)
        message(FATAL_ERROR "PHP reader expression procedure boundaries changed")
    endif()
    math(EXPR _php_expression_length "${_php_expression_end} - ${_php_expression_start}")
    string(SUBSTRING "${_php_reader_source}" ${_php_expression_start} ${_php_expression_length}
        PHP_READER_EXPRESSION_PROCEDURES)
    set(_php_group_base "${CMAKE_CURRENT_BINARY_DIR}/php_reader_group_smoke")
    configure_file("${_php_group_template}" "${_php_group_base}.crexx" @ONLY)
    set(_php_group_modules "${CMAKE_BINARY_DIR}/bin/library.rxbin" "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin")
    crexx_add_import_root(example_php_group_imports
        DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/imports/group"
        OUTPUT_VARIABLE _php_group_imports FILES ${_php_group_modules} DEPENDS library rxfnsl)
    crexx_add_fixture_command(OUTPUT "${_php_group_base}-linked.rxbin"
        BYPRODUCTS "${_php_group_base}.rxas" "${_php_group_base}.rxbin"
        COMMAND $<TARGET_FILE:rxc> -x --no-exe-import
            -i "${CMAKE_CURRENT_BINARY_DIR}/imports/group" -o "${_php_group_base}" "${_php_group_base}.crexx"
        COMMAND $<TARGET_FILE:rxas> "${_php_group_base}"
        COMMAND $<TARGET_FILE:rxlink> -o "${_php_group_base}-linked" "${_php_group_base}.rxbin" ${_php_group_modules}
        DEPENDS rxc rxas rxlink example_php_group_imports ${_php_group_imports} ${_php_group_modules}
            "${_php_reader}" "${_php_group_template}" "${_php_group_base}.crexx"
        VERBATIM)
    add_custom_target(example_php_reader_group_artifact DEPENDS "${_php_group_base}-linked.rxbin")
    add_test(NAME example_php_reader_group
        COMMAND $<TARGET_FILE:${CREXX_DEFAULT_VM_TARGET}> "${_php_group_base}-linked.rxbin")
    set_tests_properties(example_php_reader_group PROPERTIES LABELS "examples;php;comprehensive" TIMEOUT 60)
    crexx_register_test_prep_targets(TESTS example_php_reader_group
        TARGETS example_php_reader_group_artifact ${CREXX_DEFAULT_VM_TARGET})
    crexx_finalize_directory_qa_tiers()
endif()
