function(expect_exists path label)
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Expected ${label} at ${path}")
    endif()
endfunction()

function(expect_absent path label)
    if(EXISTS "${path}")
        message(FATAL_ERROR "Unexpected ${label} at ${path}")
    endif()
endfunction()

function(run_tool label)
    execute_process(
            COMMAND ${ARGN}
            OUTPUT_VARIABLE out
            ERROR_VARIABLE err
            RESULT_VARIABLE res)
    if(NOT res EQUAL 0)
        message(FATAL_ERROR "${label} failed with rc ${res}\nSTDOUT:\n${out}\nSTDERR:\n${err}")
    endif()
endfunction()

function(run_tool_in label cwd)
    execute_process(
            COMMAND ${ARGN}
            WORKING_DIRECTORY "${cwd}"
            OUTPUT_VARIABLE out
            ERROR_VARIABLE err
            RESULT_VARIABLE res)
    if(NOT res EQUAL 0)
        message(FATAL_ERROR "${label} failed with rc ${res}\nSTDOUT:\n${out}\nSTDERR:\n${err}")
    endif()
endfunction()

set(work "${WORK_ROOT}/tool_output_path_policy_ws")
file(REMOVE_RECURSE "${work}")
file(MAKE_DIRECTORY "${work}/in" "${work}/out" "${work}/driver")

file(WRITE "${work}/in/manual.widget" "options levelb\nsay \"manual path\"\n")
file(WRITE "${work}/in/driver.widget" "options levelb\nsay \"driver path\"\n")

run_tool("rxc dotted output stem"
         "${RXC}" -i "${BIN_DIR}" -o "${work}/out/manual.custom" "${work}/in/manual.widget")
expect_exists("${work}/out/manual.custom.rxas" "rxc canonical .rxas output")
expect_absent("${work}/out/manual.custom" "extensionless rxc output")

run_tool("rxc exact .rxas output"
         "${RXC}" -i "${BIN_DIR}" -o "${work}/out/manual_exact.rxas" "${work}/in/manual.widget")
expect_exists("${work}/out/manual_exact.rxas" "rxc exact .rxas output")
expect_absent("${work}/out/manual_exact.rxas.rxas" "double-suffixed rxc output")

run_tool("rxas dotted output stem"
         "${RXAS}" -o "${work}/out/manual.custom" "${work}/out/manual.custom.rxas")
expect_exists("${work}/out/manual.custom.rxbin" "rxas canonical .rxbin output")
expect_absent("${work}/out/manual.custom" "extensionless rxas output")

run_tool("rxas exact .rxbin output"
         "${RXAS}" -o "${work}/out/manual_exact.rxbin" "${work}/out/manual.custom.rxas")
expect_exists("${work}/out/manual_exact.rxbin" "rxas exact .rxbin output")
expect_absent("${work}/out/manual_exact.rxbin.rxbin" "double-suffixed rxas output")

run_tool("rxlink dotted output stem"
         "${RXLINK}" -o "${work}/out/linked.custom" "${work}/out/manual.custom.rxbin")
expect_exists("${work}/out/linked.custom.rxbin" "rxlink canonical .rxbin output")
expect_absent("${work}/out/linked.custom" "extensionless rxlink output")

run_tool("rxlink exact .rxbin output"
         "${RXLINK}" -o "${work}/out/linked_exact.rxbin" "${work}/out/manual.custom.rxbin")
expect_exists("${work}/out/linked_exact.rxbin" "rxlink exact .rxbin output")
expect_absent("${work}/out/linked_exact.rxbin.rxbin" "double-suffixed rxlink output")

run_tool_in("crexx absolute source path" "${work}/driver"
            "${CREXX}" -noexec -keep "${work}/in/driver.widget")
expect_exists("${work}/in/driver.rxas" "crexx driver RXAS beside input")
expect_exists("${work}/in/driver.rxbin" "crexx driver RXBIN beside input")
expect_absent("${work}/driver/driver.rxas" "crexx driver RXAS in current directory")
expect_absent("${work}/driver/driver.rxbin" "crexx driver RXBIN in current directory")
