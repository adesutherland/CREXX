configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/headerless_simple.rexx
               ${CMAKE_CURRENT_BINARY_DIR}/headerless_simple.rexx
               COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/rexx_src/explicit_header.rexx
               ${CMAKE_CURRENT_BINARY_DIR}/explicit_header.rexx
               COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/bin/crxc.rexx
               ${CMAKE_CURRENT_BINARY_DIR}/crxc.rexx
               COPYONLY)

set(CREXX_BIN_TEST_RESOURCE_LOCK "crexx_bin_workspace")

set(CREXX_HELP_SMOKE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_crexx_help_smoke.cmake")
set(CREXX_HELP_SMOKE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CREXX_HELP_SMOKE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx help invocation failed: \\n\${out}\")\nendif()\n")
string(APPEND CREXX_HELP_SMOKE_CONTENT "string(FIND \"\${out}\" \"Arguments are:\" pos)\n")
string(APPEND CREXX_HELP_SMOKE_CONTENT "if(pos EQUAL -1)\n  message(FATAL_ERROR \"Missing help banner in output: \\n\${out}\")\nendif()\n")
file(GENERATE OUTPUT "${CREXX_HELP_SMOKE_SCRIPT}" CONTENT "${CREXX_HELP_SMOKE_CONTENT}")

add_test(NAME crexx_help_smoke
         COMMAND ${CMAKE_COMMAND} -P "${CREXX_HELP_SMOKE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CREXX_HEADERLESS_SMOKE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_crexx_headerless_smoke.cmake")
set(CREXX_HEADERLESS_SMOKE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"${CMAKE_CURRENT_BINARY_DIR}/headerless_simple.rexx\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CREXX_HEADERLESS_SMOKE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx headerless script failed: \\n\${out}\")\nendif()\n")
string(APPEND CREXX_HEADERLESS_SMOKE_CONTENT "if(NOT out STREQUAL \"HELLO\\n\")\n  message(FATAL_ERROR \"Unexpected headerless output. Got: \\n\${out}\")\nendif()\n")
file(GENERATE OUTPUT "${CREXX_HEADERLESS_SMOKE_SCRIPT}" CONTENT "${CREXX_HEADERLESS_SMOKE_CONTENT}")

add_test(NAME crexx_headerless_simple_smoke
         COMMAND ${CMAKE_COMMAND} -P "${CREXX_HEADERLESS_SMOKE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CREXX_EXPLICIT_SMOKE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_crexx_explicit_smoke.cmake")
set(CREXX_EXPLICIT_SMOKE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"${CMAKE_CURRENT_BINARY_DIR}/explicit_header.rexx\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CREXX_EXPLICIT_SMOKE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx explicit-header script failed: \\n\${out}\")\nendif()\n")
string(APPEND CREXX_EXPLICIT_SMOKE_CONTENT "if(NOT out STREQUAL \"EXPLICIT\\n\")\n  message(FATAL_ERROR \"Unexpected explicit-header output. Got: \\n\${out}\")\nendif()\n")
file(GENERATE OUTPUT "${CREXX_EXPLICIT_SMOKE_SCRIPT}" CONTENT "${CREXX_EXPLICIT_SMOKE_CONTENT}")

add_test(NAME crexx_explicit_header_smoke
         COMMAND ${CMAKE_COMMAND} -P "${CREXX_EXPLICIT_SMOKE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CREXX_CRXC_NOARGS_SMOKE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/run_crexx_crxc_noargs_smoke.cmake")
set(CREXX_CRXC_NOARGS_SMOKE_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"${CMAKE_CURRENT_BINARY_DIR}/crxc.rexx\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
string(APPEND CREXX_CRXC_NOARGS_SMOKE_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx crxc noargs expected rc 0, got \${res}. Output: \\n\${out}\")\nendif()\n")
string(APPEND CREXX_CRXC_NOARGS_SMOKE_CONTENT "if(NOT out STREQUAL \"usage: crxc.rexx crexx_home execName[.ext]\\n\")\n  message(FATAL_ERROR \"Unexpected crxc noargs output. Got: \\n\${out}\")\nendif()\n")
file(GENERATE OUTPUT "${CREXX_CRXC_NOARGS_SMOKE_SCRIPT}" CONTENT "${CREXX_CRXC_NOARGS_SMOKE_CONTENT}")

add_test(NAME crexx_crxc_noargs_smoke
         COMMAND ${CMAKE_COMMAND} -P "${CREXX_CRXC_NOARGS_SMOKE_SCRIPT}"
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(crexx_help_smoke PROPERTIES RESOURCE_LOCK ${CREXX_BIN_TEST_RESOURCE_LOCK})
set_tests_properties(crexx_headerless_simple_smoke PROPERTIES RESOURCE_LOCK ${CREXX_BIN_TEST_RESOURCE_LOCK})
set_tests_properties(crexx_explicit_header_smoke PROPERTIES RESOURCE_LOCK ${CREXX_BIN_TEST_RESOURCE_LOCK})
set_tests_properties(crexx_crxc_noargs_smoke PROPERTIES RESOURCE_LOCK ${CREXX_BIN_TEST_RESOURCE_LOCK})

function(add_crexx_compile_only_import_test NAME MAIN_FILE DEP_FILE MODE)
    get_filename_component(MAIN_BASENAME "${MAIN_FILE}" NAME)
    get_filename_component(MAIN_MODULE "${MAIN_FILE}" NAME_WE)
    get_filename_component(DEP_BASENAME "${DEP_FILE}" NAME)

    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}.cmake")
    set(SCRIPT_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/${NAME}_ws\")\n")
    string(APPEND SCRIPT_CONTENT "set(dep \"\${work}/dep\")\n")
    string(APPEND SCRIPT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
    string(APPEND SCRIPT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
    string(APPEND SCRIPT_CONTENT "file(MAKE_DIRECTORY \"\${dep}\")\n")
    string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${MAIN_FILE}\" \"\${work}/${MAIN_BASENAME}\" RESULT_VARIABLE res)\n")
    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage main source\")\nendif()\n")
    string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${DEP_FILE}\" \"\${dep}/${DEP_BASENAME}\" RESULT_VARIABLE res)\n")
    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage dependency source\")\nendif()\n")

    if(MODE STREQUAL "source")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"--source\" \"\${dep}\" \"-noexec\" \"${MAIN_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    elseif(MODE STREQUAL "rxas")
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"--import-rxas\" \"-i\${dep}\" \"-noexec\" \"${MAIN_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    else()
        message(FATAL_ERROR "Unknown mode for add_crexx_compile_only_import_test: ${MODE}")
    endif()

    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx compile-only import test failed: \\n\${out}\")\nendif()\n")
    string(APPEND SCRIPT_CONTENT "if(NOT EXISTS \"\${work}/${MAIN_MODULE}.rxbin\")\n  message(FATAL_ERROR \"Expected ${MAIN_MODULE}.rxbin was not produced\")\nendif()\n")

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

    add_test(NAME ${NAME}
             COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES RESOURCE_LOCK ${CREXX_BIN_TEST_RESOURCE_LOCK})
endfunction()

function(add_crexx_nocompile_test NAME SOURCE_FILE)
    set(options NATIVE)
    set(oneValueArgs EXPECT_OUTPUT)
    cmake_parse_arguments(CREXXNC "${options}" "${oneValueArgs}" "" ${ARGN})

    get_filename_component(SOURCE_BASENAME "${SOURCE_FILE}" NAME)
    get_filename_component(SOURCE_MODULE "${SOURCE_FILE}" NAME_WE)
    set(SOURCE_EXECUTABLE "${SOURCE_MODULE}${CMAKE_EXECUTABLE_SUFFIX}")

    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}.cmake")
    set(SCRIPT_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/${NAME}_ws\")\n")
    string(APPEND SCRIPT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
    string(APPEND SCRIPT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
    string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\" \"\${work}/${SOURCE_BASENAME}\" RESULT_VARIABLE res)\n")
    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage source for ${NAME}\")\nendif()\n")
    string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"-noexec\" \"${SOURCE_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to prepare rxbin for ${NAME}.\\n\${out}\")\nendif()\n")

    string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\"")
    if(CREXXNC_NATIVE)
        string(APPEND SCRIPT_CONTENT " \"--verbose2\"")
    endif()
    string(APPEND SCRIPT_CONTENT " \"-nocompile\"")
    if(CREXXNC_NATIVE)
        string(APPEND SCRIPT_CONTENT " \"-native\"")
    endif()
    string(APPEND SCRIPT_CONTENT " \"${SOURCE_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx -nocompile test ${NAME} failed: \\n\${out}\")\nendif()\n")

    if(CREXXNC_NATIVE)
        string(APPEND SCRIPT_CONTENT "if(NOT EXISTS \"\${work}/${SOURCE_EXECUTABLE}\")\n  message(FATAL_ERROR \"Expected native executable ${SOURCE_EXECUTABLE} was not produced.\\ncrexx output:\\n\${out}\")\nendif()\n")
        if(CREXXNC_EXPECT_OUTPUT)
            string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"\${work}/${SOURCE_EXECUTABLE}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE native_out ERROR_VARIABLE native_err RESULT_VARIABLE native_res)\n")
            string(APPEND SCRIPT_CONTENT "if(NOT native_res EQUAL 0)\n  message(FATAL_ERROR \"Native executable ${SOURCE_EXECUTABLE} failed.\\nstdout:\\n\${native_out}\\nstderr:\\n\${native_err}\")\nendif()\n")
            string(APPEND SCRIPT_CONTENT "if(NOT native_out STREQUAL [=[${CREXXNC_EXPECT_OUTPUT}]=])\n  message(FATAL_ERROR \"Unexpected native output for ${NAME}.\\nGot:\\n\${native_out}\")\nendif()\n")
        endif()
    else()
        if(CREXXNC_EXPECT_OUTPUT)
            string(APPEND SCRIPT_CONTENT "if(NOT out STREQUAL [=[${CREXXNC_EXPECT_OUTPUT}]=])\n  message(FATAL_ERROR \"Unexpected crexx output for ${NAME}.\\nGot:\\n\${out}\")\nendif()\n")
        endif()
    endif()

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

    add_test(NAME ${NAME}
             COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES RESOURCE_LOCK ${CREXX_BIN_TEST_RESOURCE_LOCK})
endfunction()

function(add_crexx_native_test NAME SOURCE_FILE)
    set(options EXPECT_NO_LINKED_INTERMEDIATE)
    set(oneValueArgs EXPECT_OUTPUT MAP_FILE)
    set(multiValueArgs CREXX_ARGS EXPECT_DAS_CONTAINS EXPECT_DAS_ABSENT EXPECT_EXISTS EXPECT_ABSENT)
    cmake_parse_arguments(CREXXN "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    get_filename_component(SOURCE_BASENAME "${SOURCE_FILE}" NAME)
    get_filename_component(SOURCE_MODULE "${SOURCE_FILE}" NAME_WE)
    set(SOURCE_EXECUTABLE "${SOURCE_MODULE}${CMAKE_EXECUTABLE_SUFFIX}")

    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/run_${NAME}.cmake")
    set(SCRIPT_CONTENT "set(work \"${CMAKE_CURRENT_BINARY_DIR}/${NAME}_ws\")\n")
    string(APPEND SCRIPT_CONTENT "file(REMOVE_RECURSE \"\${work}\")\n")
    string(APPEND SCRIPT_CONTENT "file(MAKE_DIRECTORY \"\${work}\")\n")
    string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}\" \"\${work}/${SOURCE_BASENAME}\" RESULT_VARIABLE res)\n")
    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"Failed to stage source for ${NAME}\")\nendif()\n")

    string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:crexx>\" \"--verbose2\"")
    foreach(_arg IN LISTS CREXXN_CREXX_ARGS)
        string(APPEND SCRIPT_CONTENT " \"${_arg}\"")
    endforeach()
    string(APPEND SCRIPT_CONTENT " \"-native\" \"${SOURCE_BASENAME}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE out ERROR_VARIABLE out RESULT_VARIABLE res)\n")
    string(APPEND SCRIPT_CONTENT "if(NOT res EQUAL 0)\n  message(FATAL_ERROR \"crexx native test ${NAME} failed: \\n\${out}\")\nendif()\n")
    string(APPEND SCRIPT_CONTENT "if(NOT EXISTS \"\${work}/${SOURCE_EXECUTABLE}\")\n  message(FATAL_ERROR \"Expected native executable ${SOURCE_EXECUTABLE} was not produced.\\ncrexx output:\\n\${out}\")\nendif()\n")
    if(NOT CREXXN_EXPECT_NO_LINKED_INTERMEDIATE)
        string(APPEND SCRIPT_CONTENT "if(NOT EXISTS \"\${work}/${SOURCE_MODULE}_linked.rxbin\")\n  message(FATAL_ERROR \"Expected linked intermediate ${SOURCE_MODULE}_linked.rxbin was not produced\")\nendif()\n")
    endif()

    if(CREXXN_EXPECT_OUTPUT)
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"\${work}/${SOURCE_EXECUTABLE}\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE native_out ERROR_VARIABLE native_err RESULT_VARIABLE native_res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT native_res EQUAL 0)\n  message(FATAL_ERROR \"Native executable ${SOURCE_EXECUTABLE} failed.\\nstdout:\\n\${native_out}\\nstderr:\\n\${native_err}\")\nendif()\n")
        string(APPEND SCRIPT_CONTENT "if(NOT native_out STREQUAL [=[${CREXXN_EXPECT_OUTPUT}]=])\n  message(FATAL_ERROR \"Unexpected native output for ${NAME}.\\nGot:\\n\${native_out}\")\nendif()\n")
    endif()

    if(CREXXN_MAP_FILE)
        string(APPEND SCRIPT_CONTENT "if(NOT EXISTS \"\${work}/${CREXXN_MAP_FILE}\")\n  message(FATAL_ERROR \"Expected link map ${CREXXN_MAP_FILE} was not produced\")\nendif()\n")
    endif()

    foreach(_path IN LISTS CREXXN_EXPECT_EXISTS)
        string(APPEND SCRIPT_CONTENT "if(NOT EXISTS \"\${work}/${_path}\")\n  message(FATAL_ERROR \"Expected file ${_path} was not produced\")\nendif()\n")
    endforeach()

    foreach(_path IN LISTS CREXXN_EXPECT_ABSENT)
        string(APPEND SCRIPT_CONTENT "if(EXISTS \"\${work}/${_path}\")\n  message(FATAL_ERROR \"Unexpected file ${_path} remains after native build\")\nendif()\n")
    endforeach()

    if(CREXXN_EXPECT_DAS_CONTAINS OR CREXXN_EXPECT_DAS_ABSENT)
        string(APPEND SCRIPT_CONTENT "execute_process(COMMAND \"$<TARGET_FILE:rxdas>\" \"${SOURCE_MODULE}_linked\" WORKING_DIRECTORY \"\${work}\" OUTPUT_VARIABLE das_out ERROR_VARIABLE das_err RESULT_VARIABLE das_res)\n")
        string(APPEND SCRIPT_CONTENT "if(NOT das_res EQUAL 0)\n  message(FATAL_ERROR \"rxdas failed for ${NAME}.\\nstdout:\\n\${das_out}\\nstderr:\\n\${das_err}\")\nendif()\n")
        foreach(_needle IN LISTS CREXXN_EXPECT_DAS_CONTAINS)
            string(APPEND SCRIPT_CONTENT "string(FIND \"\${das_out}\" [=[${_needle}]=] pos)\n")
            string(APPEND SCRIPT_CONTENT "if(pos EQUAL -1)\n  message(FATAL_ERROR \"Missing expected rxdas text [=[${_needle}]=] for ${NAME}.\\nOutput:\\n\${das_out}\")\nendif()\n")
        endforeach()
        foreach(_needle IN LISTS CREXXN_EXPECT_DAS_ABSENT)
            string(APPEND SCRIPT_CONTENT "string(FIND \"\${das_out}\" [=[${_needle}]=] pos)\n")
            string(APPEND SCRIPT_CONTENT "if(NOT pos EQUAL -1)\n  message(FATAL_ERROR \"Unexpected rxdas text [=[${_needle}]=] for ${NAME}.\\nOutput:\\n\${das_out}\")\nendif()\n")
        endforeach()
    endif()

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

    add_test(NAME ${NAME}
             COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES RESOURCE_LOCK ${CREXX_BIN_TEST_RESOURCE_LOCK})
endfunction()

add_crexx_compile_only_import_test(crexx_source_root_compile_only rexx_src/source_import_main.rexx rexx_src/source_import_dep.rexx source)
add_crexx_compile_only_import_test(crexx_rxas_import_compile_only rexx_src/rxas_import_main.rexx rexx_src/wrapperasm.rxas rxas)
add_crexx_nocompile_test(
        crexx_nocompile_exec_smoke
        rexx_src/explicit_header.rexx
        EXPECT_OUTPUT "EXPLICIT\n"
)
add_crexx_nocompile_test(
        crexx_native_nocompile_smoke
        rexx_src/explicit_header.rexx
        NATIVE
        EXPECT_OUTPUT "EXPLICIT\n"
)
add_crexx_native_test(
        crexx_native_explicit_header_smoke
        rexx_src/explicit_header.rexx
        EXPECT_OUTPUT "EXPLICIT\n"
)
add_crexx_native_test(
        crexx_native_default_link_strip
        rexx_src/explicit_header.rexx
        EXPECT_DAS_ABSENT ".srcfile="
)
add_crexx_native_test(
        crexx_native_linkmap_keep_source
        rexx_src/explicit_header.rexx
        CREXX_ARGS "--linkmap" "explicit_header.map" "--link-keep-source"
        MAP_FILE "explicit_header.map"
        EXPECT_DAS_CONTAINS ".srcfile="
)
add_crexx_native_test(
        crexx_native_nokeep_cleanup
        rexx_src/explicit_header.rexx
        CREXX_ARGS "-nokeep"
        EXPECT_OUTPUT "EXPLICIT\n"
        EXPECT_NO_LINKED_INTERMEDIATE
        EXPECT_ABSENT "explicit_header.rxas" "explicit_header.rxbin" "explicit_header_linked.rxbin" "explicit_header.c"
)
