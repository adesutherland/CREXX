function(add_rxas_parser_test NAME SOURCE GOLDEN SCRIPT)
    set(_script "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
    set(_content "")
    string(APPEND _content
            "execute_process(\n"
            "  COMMAND \"$<TARGET_FILE:parser_tester>\" -q -p \"$<TARGET_FILE:rxas>\" -s \"${SOURCE}\" \"${SCRIPT}\"\n"
            "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
            "  OUTPUT_VARIABLE out\n"
            "  ERROR_VARIABLE err\n"
            "  RESULT_VARIABLE res\n"
            ")\n")
    string(APPEND _content
            "if(NOT res EQUAL 0)\n"
            "  message(FATAL_ERROR \"parser_tester failed with exit code \${res}\\nSTDERR:\\n\${err}\\nSTDOUT:\\n\${out}\")\n"
            "endif()\n"
            "string(REPLACE \"\\r\\n\" \"\\n\" out \"\${out}\")\n"
            "string(REGEX REPLACE \"(^|\\n)STATUS: [^\\n]*(\\n|$)\" \"\\\\1\" out \"\${out}\")\n"
            "file(READ \"${GOLDEN}\" golden)\n"
            "string(REPLACE \"\\r\\n\" \"\\n\" golden \"\${golden}\")\n"
            "if(NOT out STREQUAL golden)\n"
            "  message(FATAL_ERROR \"Output mismatch for ${NAME}\\nExpected:\\n\${golden}\\nGot:\\n\${out}\\nSTDERR:\\n\${err}\")\n"
            "endif()\n")
    file(GENERATE OUTPUT "${_script}" CONTENT "${_content}")

    add_test(NAME ${NAME}
             COMMAND ${CMAKE_COMMAND} -P "${_script}")
endfunction()

add_rxas_parser_test(test_rxas_01_init
        "${CMAKE_CURRENT_SOURCE_DIR}/test_01_init.rxas"
        "${CMAKE_CURRENT_SOURCE_DIR}/test_01_init.golden"
        "${CMAKE_CURRENT_SOURCE_DIR}/test_01_init.txt")

add_rxas_parser_test(test_rxas_02_edits
        "${CMAKE_CURRENT_SOURCE_DIR}/test_01_init.rxas"
        "${CMAKE_CURRENT_SOURCE_DIR}/test_02_edits.golden"
        "${CMAKE_CURRENT_SOURCE_DIR}/test_02_edits.txt")

add_rxas_parser_test(test_rxas_03_token_types
        "${CMAKE_CURRENT_SOURCE_DIR}/test_03_token_types.rxas"
        "${CMAKE_CURRENT_SOURCE_DIR}/test_03_token_types.golden"
        "${CMAKE_CURRENT_SOURCE_DIR}/test_03_token_types.txt")

add_rxas_parser_test(test_rxas_04_errors
        "${CMAKE_CURRENT_SOURCE_DIR}/test_04_errors.rxas"
        "${CMAKE_CURRENT_SOURCE_DIR}/test_04_errors.golden"
        "${CMAKE_CURRENT_SOURCE_DIR}/test_04_errors.txt")

add_test(NAME rxas_reject_non_source_mnemonics
         COMMAND ${CMAKE_COMMAND}
                 -DRXAS=$<TARGET_FILE:rxas>
                 -DWORKING_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}
                 -P ${CMAKE_CURRENT_SOURCE_DIR}/reject_non_source_mnemonics.cmake)
