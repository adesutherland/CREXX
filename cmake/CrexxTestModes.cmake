include(CMakeParseArguments)

function(_crexx_register_runtime_test)
    set(options)
    set(oneValueArgs NAME RUNNER PROGRAM WORKING_DIRECTORY)
    set(multiValueArgs RUNTIME_ARGS TEST_LABELS)
    cmake_parse_arguments(CREXX "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_test(
            NAME ${CREXX_NAME}
            COMMAND ${CREXX_RUNNER} ${CREXX_PROGRAM} ${CREXX_RUNTIME_ARGS}
            WORKING_DIRECTORY ${CREXX_WORKING_DIRECTORY}
    )

    if(CREXX_TEST_LABELS)
        set_tests_properties(${CREXX_NAME} PROPERTIES LABELS "${CREXX_TEST_LABELS}")
    endif()
endfunction()

function(crexx_add_rexx_opt_matrix)
    set(options)
    set(oneValueArgs NAME SOURCE WORKING_DIRECTORY TARGET_GROUP RUNNER COMPILER_TARGET ASSEMBLER_TARGET)
    set(multiValueArgs DEPENDS IMPORT_PATHS RXC_FLAGS RXAS_FLAGS RUNTIME_ARGS TEST_LABELS)
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
                COMMAND ${_crexx_compile_cmd}
                COMMAND ${_crexx_assemble_cmd}
                DEPENDS ${CREXX_COMPILER_TARGET} ${CREXX_ASSEMBLER_TARGET} ${CREXX_SOURCE} ${CREXX_DEPENDS}
                WORKING_DIRECTORY ${CREXX_WORKING_DIRECTORY}
        )

        add_custom_target(${_crexx_output_base}_artifact DEPENDS ${_crexx_artifact})
        if(CREXX_TARGET_GROUP)
            add_dependencies(${CREXX_TARGET_GROUP} ${_crexx_output_base}_artifact)
        endif()

        _crexx_register_runtime_test(
                NAME ${CREXX_NAME}_${_crexx_mode}
                RUNNER ${CREXX_RUNNER}
                PROGRAM ${_crexx_output_base}
                WORKING_DIRECTORY ${CREXX_WORKING_DIRECTORY}
                RUNTIME_ARGS ${CREXX_RUNTIME_ARGS}
                TEST_LABELS ${CREXX_TEST_LABELS}
        )
    endforeach()
endfunction()

function(crexx_add_rxas_opt_matrix)
    set(options)
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

        add_custom_target(${_crexx_output_base}_artifact DEPENDS ${_crexx_artifact})
        if(CREXX_TARGET_GROUP)
            add_dependencies(${CREXX_TARGET_GROUP} ${_crexx_output_base}_artifact)
        endif()

        _crexx_register_runtime_test(
                NAME ${CREXX_NAME}_${_crexx_mode}
                RUNNER ${CREXX_RUNNER}
                PROGRAM ${_crexx_output_base}
                WORKING_DIRECTORY ${CREXX_WORKING_DIRECTORY}
                RUNTIME_ARGS ${CREXX_RUNTIME_ARGS}
                TEST_LABELS ${CREXX_TEST_LABELS}
        )
    endforeach()
endfunction()
