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

set(mapped_fail_src "${WORK}/srcmap_mapped_fail.crexx")
file(WRITE "${mapped_fail_src}" [=[options levelb srcmap
@"demo.rxpp"
@3l"answer = SQUARE(totl + 1)"
@10c
answer = @+0+16{(@+7+4{totl@} + 1) * (@+7+4{totl@} + 1)@}
]=])

execute_process(
    COMMAND "${RXC}" --diagnostics raw "${mapped_fail_src}"
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

set(nested_src "${WORK}/srcmap_nested_fail.crexx")
file(WRITE "${nested_src}" [=[options levelb srcmap
@"nested.rxpp"
@7l"answer = OUTER(totl + 1)"
@10c
answer = @+0+17{(@+6+4{totl@} + 1)@}
]=])

execute_process(
    COMMAND "${RXC}" --diagnostics raw "${nested_src}"
    WORKING_DIRECTORY "${WORK}"
    OUTPUT_VARIABLE nested_stdout
    ERROR_VARIABLE nested_stderr
    RESULT_VARIABLE nested_res
)
if(nested_res EQUAL 0)
    message(FATAL_ERROR "rxc nested source-map diagnostic case unexpectedly succeeded")
endif()
if(NOT nested_stderr MATCHES "nested\\.rxpp @ 7:16")
    message(FATAL_ERROR "nested source-map diagnostic did not prefer the inner span:\n${nested_stderr}")
endif()
if(NOT nested_stderr MATCHES "totl")
    message(FATAL_ERROR "nested source-map diagnostic did not include mapped source text:\n${nested_stderr}")
endif()

set(malformed_src "${WORK}/srcmap_malformed.crexx")
file(WRITE "${malformed_src}" [=[options levelb srcmap
say "email@example.com"
]=])

execute_process(
    COMMAND "${RXC}" --diagnostics raw "${malformed_src}"
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

set(unbalanced_close_src "${WORK}/srcmap_unbalanced_close.crexx")
file(WRITE "${unbalanced_close_src}" [=[options levelb srcmap
say "done"@}
]=])

execute_process(
    COMMAND "${RXC}" --diagnostics raw "${unbalanced_close_src}"
    WORKING_DIRECTORY "${WORK}"
    OUTPUT_VARIABLE close_stdout
    ERROR_VARIABLE close_stderr
    RESULT_VARIABLE close_res
)
if(close_res EQUAL 0)
    message(FATAL_ERROR "rxc unmatched closing source-map span unexpectedly succeeded")
endif()
if(NOT close_stderr MATCHES "SRCMAP_UNBALANCED")
    message(FATAL_ERROR "unmatched closing span diagnostic was not reported:\n${close_stderr}")
endif()

set(unbalanced_open_src "${WORK}/srcmap_unbalanced_open.crexx")
file(WRITE "${unbalanced_open_src}" [=[options levelb srcmap
@"open.rxpp"
@1l"say ""done"""
@1+3{say "done"
]=])

execute_process(
    COMMAND "${RXC}" --diagnostics raw "${unbalanced_open_src}"
    WORKING_DIRECTORY "${WORK}"
    OUTPUT_VARIABLE open_stdout
    ERROR_VARIABLE open_stderr
    RESULT_VARIABLE open_res
)
if(open_res EQUAL 0)
    message(FATAL_ERROR "rxc unmatched opening source-map span unexpectedly succeeded")
endif()
if(NOT open_stderr MATCHES "SRCMAP_UNBALANCED")
    message(FATAL_ERROR "unmatched opening span diagnostic was not reported:\n${open_stderr}")
endif()
