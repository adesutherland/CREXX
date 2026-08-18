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

# POSTPERF-05 deliberately retains these larger sites when their final cleaned
# expansion is not better than the executable call plus body. The paired
# optimized/noopt runtime test remains the stale-result correctness proof; this
# structural contract prevents fixed-point oscillation back to expansion.
assert_instruction_count("\n[ \t]+call[0-9]* [^\n]*\\.run\\(\\)" 1
                         "run method must retain the bounded call fallback")
assert_instruction_count("\n[ \t]+call[0-9]* [^\n]*\\.answer\\(\\)" 1
                         "answer method must retain the bounded call fallback")
string(REGEX MATCHALL
       "\n[ \t]+call[0-9]* [^\n]*\\.recursiveanswer\\(\\)"
       recursive_calls "${rxas}")
list(LENGTH recursive_calls recursive_count)
if(recursive_count LESS 1)
    message(FATAL_ERROR "genuinely recursive call was unexpectedly removed")
endif()

message("ok")
