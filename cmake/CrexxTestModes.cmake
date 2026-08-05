include(CMakeParseArguments)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxLinkedRuntime.cmake)

function(_crexx_register_linked_opt_artifact TARGET_NAME)
    if(NOT TARGET linked_opt_runtime_artifacts)
        add_custom_target(linked_opt_runtime_artifacts)
    endif()
    add_dependencies(linked_opt_runtime_artifacts ${TARGET_NAME})
endfunction()

function(_crexx_register_runtime_test)
    set(options PASS_TEST_NAME_ARGUMENT)
    set(oneValueArgs NAME RUNNER PROGRAM WORKING_DIRECTORY
            EXPECTED_EXIT_CODE EXPECTED_FAILURE_DESCRIPTION)
    set(multiValueArgs RUNTIME_ARGS TEST_LABELS REQUIRED_OUTPUT_REGEX FORBIDDEN_OUTPUT_REGEX)
    cmake_parse_arguments(CREXX "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(_crexx_runner_cmd ${CREXX_RUNNER})
    if(NOT "${CREXX_RUNNER}" MATCHES "^\\$<" AND TARGET ${CREXX_RUNNER})
        set(_crexx_runner_cmd $<TARGET_FILE:${CREXX_RUNNER}>)
    endif()

    set(_crexx_labels ${CREXX_TEST_LABELS})
    set(_crexx_runtime_args ${CREXX_RUNTIME_ARGS})
    if(CREXX_PASS_TEST_NAME_ARGUMENT)
        list(APPEND _crexx_runtime_args -a ${CREXX_NAME})
    endif()
    if(CREXX_NAME MATCHES "_opt$")
        set(_crexx_script "${CMAKE_CURRENT_BINARY_DIR}/${CREXX_NAME}_linked_runtime.cmake")
        crexx_write_linked_runtime_script(
                OUTPUT "${_crexx_script}"
                NAME "${CREXX_NAME}"
                WORKING_DIRECTORY "${CREXX_WORKING_DIRECTORY}"
                RUNNER "${_crexx_runner_cmd}"
                RXLINK "$<TARGET_FILE:rxlink>"
                PROGRAM "${CREXX_PROGRAM}"
                EXPECTED_EXIT_CODE "${CREXX_EXPECTED_EXIT_CODE}"
                EXPECTED_FAILURE_DESCRIPTION "${CREXX_EXPECTED_FAILURE_DESCRIPTION}"
                EXTRA_ARGS ${_crexx_runtime_args}
                REQUIRED_OUTPUT_REGEX ${CREXX_REQUIRED_OUTPUT_REGEX}
                FORBIDDEN_OUTPUT_REGEX ${CREXX_FORBIDDEN_OUTPUT_REGEX}
        )
        add_test(
                NAME ${CREXX_NAME}
                COMMAND ${CMAKE_COMMAND} -P "${_crexx_script}"
                WORKING_DIRECTORY ${CREXX_WORKING_DIRECTORY}
        )
        list(APPEND _crexx_labels linked_opt)
    elseif(NOT "${CREXX_EXPECTED_EXIT_CODE}" STREQUAL "")
        set(_crexx_script "${CMAKE_CURRENT_BINARY_DIR}/${CREXX_NAME}_checked_runtime.cmake")
        crexx_write_runtime_script(
                OUTPUT "${_crexx_script}"
                NAME "${CREXX_NAME}"
                WORKING_DIRECTORY "${CREXX_WORKING_DIRECTORY}"
                RUNNER "${_crexx_runner_cmd}"
                PROGRAM "${CREXX_PROGRAM}"
                EXPECTED_EXIT_CODE "${CREXX_EXPECTED_EXIT_CODE}"
                EXPECTED_FAILURE_DESCRIPTION "${CREXX_EXPECTED_FAILURE_DESCRIPTION}"
                EXTRA_ARGS ${_crexx_runtime_args}
                REQUIRED_OUTPUT_REGEX ${CREXX_REQUIRED_OUTPUT_REGEX}
                FORBIDDEN_OUTPUT_REGEX ${CREXX_FORBIDDEN_OUTPUT_REGEX}
        )
        add_test(
                NAME ${CREXX_NAME}
                COMMAND ${CMAKE_COMMAND} -P "${_crexx_script}"
                WORKING_DIRECTORY ${CREXX_WORKING_DIRECTORY}
        )
    else()
        add_test(
                NAME ${CREXX_NAME}
                COMMAND ${_crexx_runner_cmd} ${CREXX_PROGRAM} ${_crexx_runtime_args}
                WORKING_DIRECTORY ${CREXX_WORKING_DIRECTORY}
        )
    endif()

    if(_crexx_labels)
        set_tests_properties(${CREXX_NAME} PROPERTIES LABELS "${_crexx_labels}")
    endif()
    # The linked-opt fixture builds generated runtime artifacts inside the
    # active build tree. Keep all helper-generated runtime tests behind that
    # setup step so no test executes while CTest is still driving a build.
    set_tests_properties(${CREXX_NAME} PROPERTIES FIXTURES_REQUIRED linked_opt_runtime_artifacts)
endfunction()

function(crexx_add_rexx_opt_matrix)
    set(options PASS_TEST_NAME_ARGUMENT)
    set(oneValueArgs NAME SOURCE WORKING_DIRECTORY TARGET_GROUP RUNNER COMPILER_TARGET ASSEMBLER_TARGET
            EXPECTED_EXIT_CODE EXPECTED_FAILURE_DESCRIPTION)
    set(multiValueArgs DEPENDS IMPORT_PATHS RXC_FLAGS RXAS_FLAGS RUNTIME_ARGS TEST_LABELS
            REQUIRED_OUTPUT_REGEX FORBIDDEN_OUTPUT_REGEX)
    cmake_parse_arguments(CREXX "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT CREXX_NAME OR NOT CREXX_SOURCE)
        message(FATAL_ERROR "crexx_add_rexx_opt_matrix requires NAME and SOURCE")
    endif()

    if(NOT CREXX_WORKING_DIRECTORY)
        set(CREXX_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif()
    if(NOT CREXX_RUNNER)
        set(CREXX_RUNNER $<TARGET_FILE:rxvm>)
    endif()
    if(NOT CREXX_COMPILER_TARGET)
        set(CREXX_COMPILER_TARGET rxc)
    endif()
    if(NOT CREXX_ASSEMBLER_TARGET)
        set(CREXX_ASSEMBLER_TARGET rxas)
    endif()
    if(CREXX_IMPORT_PATHS)
        string(JOIN "$<SEMICOLON>" _crexx_import_path ${CREXX_IMPORT_PATHS})
        set(_crexx_import_arg "\"${_crexx_import_path}\"")
    endif()

    set(_crexx_compile_depends
            ${CREXX_COMPILER_TARGET}
            ${CREXX_ASSEMBLER_TARGET}
            ${CREXX_SOURCE}
            ${CREXX_DEPENDS})
    if(CREXX_COMPILER_TARGET STREQUAL "rxc")
        list(APPEND _crexx_compile_depends compiler_exit_bin)
    endif()

    foreach(_crexx_mode IN ITEMS noopt opt)
        set(_crexx_output_base ${CREXX_NAME}_${_crexx_mode})
        set(_crexx_mode_rxc_flags ${CREXX_RXC_FLAGS})
        set(_crexx_mode_rxas_flags ${CREXX_RXAS_FLAGS})
        if(_crexx_mode STREQUAL "noopt")
            list(PREPEND _crexx_mode_rxc_flags -n)
            list(PREPEND _crexx_mode_rxas_flags -n)
        endif()

        set(_crexx_compile_cmd $<TARGET_FILE:${CREXX_COMPILER_TARGET}>)
        if(CREXX_IMPORT_PATHS)
            list(APPEND _crexx_compile_cmd -i ${_crexx_import_arg})
        endif()
        list(APPEND _crexx_compile_cmd ${_crexx_mode_rxc_flags} -o ${_crexx_output_base} ${CREXX_SOURCE})

        set(_crexx_assemble_cmd $<TARGET_FILE:${CREXX_ASSEMBLER_TARGET}>)
        list(APPEND _crexx_assemble_cmd ${_crexx_mode_rxas_flags} -o ${_crexx_output_base} ${_crexx_output_base})

        set(_crexx_artifact ${CREXX_WORKING_DIRECTORY}/${_crexx_output_base}.rxbin)
        add_custom_command(
                OUTPUT ${_crexx_artifact}
                BYPRODUCTS ${CREXX_WORKING_DIRECTORY}/${_crexx_output_base}.rxas
                COMMAND ${_crexx_compile_cmd}
                COMMAND ${_crexx_assemble_cmd}
                DEPENDS ${_crexx_compile_depends}
                WORKING_DIRECTORY ${CREXX_WORKING_DIRECTORY}
        )

        set(_crexx_artifact_target ${_crexx_output_base}_artifact)
        add_custom_target(${_crexx_artifact_target} DEPENDS ${_crexx_artifact})
        if(CREXX_TARGET_GROUP)
            add_dependencies(${CREXX_TARGET_GROUP} ${_crexx_artifact_target})
        endif()
        if(_crexx_mode STREQUAL "opt")
            _crexx_register_linked_opt_artifact(${_crexx_artifact_target})
        endif()

        set(_crexx_runtime_test_args)
        if(CREXX_PASS_TEST_NAME_ARGUMENT)
            list(APPEND _crexx_runtime_test_args PASS_TEST_NAME_ARGUMENT)
        endif()
        _crexx_register_runtime_test(
                NAME ${CREXX_NAME}_${_crexx_mode}
                RUNNER ${CREXX_RUNNER}
                PROGRAM ${_crexx_output_base}
                WORKING_DIRECTORY ${CREXX_WORKING_DIRECTORY}
                EXPECTED_EXIT_CODE "${CREXX_EXPECTED_EXIT_CODE}"
                EXPECTED_FAILURE_DESCRIPTION "${CREXX_EXPECTED_FAILURE_DESCRIPTION}"
                RUNTIME_ARGS ${CREXX_RUNTIME_ARGS}
                TEST_LABELS ${CREXX_TEST_LABELS}
                REQUIRED_OUTPUT_REGEX ${CREXX_REQUIRED_OUTPUT_REGEX}
                FORBIDDEN_OUTPUT_REGEX ${CREXX_FORBIDDEN_OUTPUT_REGEX}
                ${_crexx_runtime_test_args}
        )
    endforeach()
endfunction()

function(crexx_add_rxas_opt_matrix)
    set(options PASS_TEST_NAME_ARGUMENT)
    set(oneValueArgs NAME SOURCE WORKING_DIRECTORY TARGET_GROUP RUNNER ASSEMBLER_TARGET)
    set(multiValueArgs DEPENDS RXAS_FLAGS RUNTIME_ARGS TEST_LABELS)
    cmake_parse_arguments(CREXX "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT CREXX_NAME OR NOT CREXX_SOURCE)
        message(FATAL_ERROR "crexx_add_rxas_opt_matrix requires NAME and SOURCE")
    endif()

    if(NOT CREXX_WORKING_DIRECTORY)
        set(CREXX_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif()
    if(NOT CREXX_RUNNER)
        set(CREXX_RUNNER $<TARGET_FILE:rxvm>)
    endif()
    if(NOT CREXX_ASSEMBLER_TARGET)
        set(CREXX_ASSEMBLER_TARGET rxas)
    endif()

    foreach(_crexx_mode IN ITEMS noopt opt)
        set(_crexx_output_base ${CREXX_NAME}_${_crexx_mode})
        set(_crexx_mode_rxas_flags ${CREXX_RXAS_FLAGS})
        if(_crexx_mode STREQUAL "noopt")
            list(PREPEND _crexx_mode_rxas_flags -n)
        endif()

        set(_crexx_assemble_cmd $<TARGET_FILE:${CREXX_ASSEMBLER_TARGET}>)
        list(APPEND _crexx_assemble_cmd ${_crexx_mode_rxas_flags} -o ${_crexx_output_base} ${CREXX_SOURCE})

        set(_crexx_artifact ${CREXX_WORKING_DIRECTORY}/${_crexx_output_base}.rxbin)
        add_custom_command(
                OUTPUT ${_crexx_artifact}
                COMMAND ${_crexx_assemble_cmd}
                DEPENDS ${CREXX_ASSEMBLER_TARGET} ${CREXX_DEPENDS}
                WORKING_DIRECTORY ${CREXX_WORKING_DIRECTORY}
        )

        set(_crexx_artifact_target ${_crexx_output_base}_artifact)
        add_custom_target(${_crexx_artifact_target} DEPENDS ${_crexx_artifact})
        if(CREXX_TARGET_GROUP)
            add_dependencies(${CREXX_TARGET_GROUP} ${_crexx_artifact_target})
        endif()
        if(_crexx_mode STREQUAL "opt")
            _crexx_register_linked_opt_artifact(${_crexx_artifact_target})
        endif()

        set(_crexx_runtime_test_args)
        if(CREXX_PASS_TEST_NAME_ARGUMENT)
            list(APPEND _crexx_runtime_test_args PASS_TEST_NAME_ARGUMENT)
        endif()
        _crexx_register_runtime_test(
                NAME ${CREXX_NAME}_${_crexx_mode}
                RUNNER ${CREXX_RUNNER}
                PROGRAM ${_crexx_output_base}
                WORKING_DIRECTORY ${CREXX_WORKING_DIRECTORY}
                RUNTIME_ARGS ${CREXX_RUNTIME_ARGS}
                TEST_LABELS ${CREXX_TEST_LABELS}
                ${_crexx_runtime_test_args}
        )
    endforeach()
endfunction()
