if(NOT DEFINED RXC)
    message(FATAL_ERROR "RXC is required")
endif()
if(NOT DEFINED WORK)
    message(FATAL_ERROR "WORK is required")
endif()

file(MAKE_DIRECTORY "${WORK}")

set(ok_src "${WORK}/srcmap_ok.crexx")
set(ok_out "${WORK}/srcmap_ok")
file(WRITE "${ok_src}" [=[options levelb srcmap
@"ok.rxpp"
@1l"say ""a@@b"""
@1+9{say "a@@b"@}
]=])

execute_process(
    COMMAND "${RXC}" --diagnostics raw -o "${ok_out}" "${ok_src}"
    WORKING_DIRECTORY "${WORK}"
    OUTPUT_VARIABLE ok_stdout
    ERROR_VARIABLE ok_stderr
    RESULT_VARIABLE ok_res
)
if(NOT ok_res EQUAL 0)
    message(FATAL_ERROR "rxc srcmap strip/escape case failed with ${ok_res}:\n${ok_stdout}${ok_stderr}")
endif()

file(READ "${ok_out}.rxas" ok_rxas)
string(FIND "${ok_rxas}" "ok.rxpp" ok_file_pos)
if(ok_file_pos EQUAL -1)
    message(FATAL_ERROR "srcstep did not use mapped file:\n${ok_rxas}")
endif()
string(FIND "${ok_rxas}" "say \"a@b\"" ok_say_pos)
if(ok_say_pos EQUAL -1)
    message(FATAL_ERROR "escaped @ was not stripped into generated code:\n${ok_rxas}")
endif()
string(FIND "${ok_rxas}" "@1+9" stale_marker_pos)
if(NOT stale_marker_pos EQUAL -1)
    message(FATAL_ERROR "source-map marker leaked into rxas:\n${ok_rxas}")
endif()

set(fail_src "${WORK}/srcmap_fail.crexx")
file(WRITE "${fail_src}" [=[options levelb srcmap
@"demo.rxpp"
@3l"answer = SQUARE(totl + 1)"
@10c
answer = @+0+16{(@+7+4{totl@} + 1) * (@+7+4{totl@} + 1)@}
]=])

execute_process(
    COMMAND "${RXC}" --diagnostics raw "${fail_src}"
    WORKING_DIRECTORY "${WORK}"
    OUTPUT_VARIABLE fail_stdout
    ERROR_VARIABLE fail_stderr
    RESULT_VARIABLE fail_res
)
if(fail_res EQUAL 0)
    message(FATAL_ERROR "rxc srcmap diagnostic case unexpectedly succeeded")
endif()
if(NOT fail_stderr MATCHES "demo\\.rxpp @ 3:17")
    message(FATAL_ERROR "diagnostic was not remapped to original source:\n${fail_stderr}")
endif()
if(NOT fail_stderr MATCHES "totl")
    message(FATAL_ERROR "diagnostic did not include mapped source text:\n${fail_stderr}")
endif()

set(bad_src "${WORK}/srcmap_bad.crexx")
file(WRITE "${bad_src}" [=[options levelb srcmap
say "email@example.com"
]=])

execute_process(
    COMMAND "${RXC}" --diagnostics raw "${bad_src}"
    WORKING_DIRECTORY "${WORK}"
    OUTPUT_VARIABLE bad_stdout
    ERROR_VARIABLE bad_stderr
    RESULT_VARIABLE bad_res
)
if(bad_res EQUAL 0)
    message(FATAL_ERROR "rxc malformed source-map case unexpectedly succeeded")
endif()
if(NOT bad_stderr MATCHES "SRCMAP_MALFORMED")
    message(FATAL_ERROR "malformed source-map diagnostic was not reported:\n${bad_stderr}")
endif()
