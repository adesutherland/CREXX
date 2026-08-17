foreach(required IN ITEMS RXC BIN_DIR SOURCE WORK_DIR)
    if(NOT DEFINED ${required} OR "${${required}}" STREQUAL "")
        message(FATAL_ERROR "${required} is required")
    endif()
endforeach()

file(MAKE_DIRECTORY "${WORK_DIR}")
set(output "${WORK_DIR}/flow_copy_fixed_point_inline_result_opt")
execute_process(
    COMMAND "${RXC}" -i "${BIN_DIR}" -o "${output}" "${SOURCE}"
    RESULT_VARIABLE compile_result
    OUTPUT_VARIABLE compile_stdout
    ERROR_VARIABLE compile_stderr
)
if(NOT compile_result EQUAL 0)
    message(FATAL_ERROR
            "optimized compile failed (${compile_result})\n${compile_stdout}${compile_stderr}")
endif()

file(READ "${output}.rxas" rxas)

function(assert_instruction_count pattern expected description)
    string(REGEX MATCHALL "${pattern}" matches "${rxas}")
    list(LENGTH matches actual)
    if(NOT actual EQUAL expected)
        message(FATAL_ERROR
                "${description}: expected ${expected} matches, found ${actual}")
    endif()
endfunction()

# The correction must preserve the supported nested-inline shape. Only the
# genuinely recursive method remains a call in the optimized image.
assert_instruction_count("\n[ \t]+call[0-9]* [^\n]*\\.run\\(\\)" 0
                         "run method must remain inlined")
assert_instruction_count("\n[ \t]+call[0-9]* [^\n]*\\.answer\\(\\)" 0
                         "answer method must remain inlined")
string(REGEX MATCHALL
       "\n[ \t]+call[0-9]* [^\n]*\\.recursiveanswer\\(\\)"
       recursive_calls "${rxas}")
list(LENGTH recursive_calls recursive_count)
if(recursive_count LESS 1)
    message(FATAL_ERROR "genuinely recursive call was unexpectedly removed")
endif()

message("ok")
