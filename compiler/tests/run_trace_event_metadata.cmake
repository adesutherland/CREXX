if(NOT DEFINED RXC)
    message(FATAL_ERROR "RXC is required")
endif()

if(NOT DEFINED WORK)
    message(FATAL_ERROR "WORK is required")
endif()

file(MAKE_DIRECTORY "${WORK}")

set(SOURCE "${WORK}/trace_event_metadata.crexx")
set(OUT_BASE "${WORK}/trace_event_metadata")
set(ACTUAL "${WORK}/trace_event_metadata.actual")
set(EXPECTED "${WORK}/trace_event_metadata.expected")

file(WRITE "${SOURCE}" [[options levelb
main: procedure
  call test(1)
return

test: procedure
  arg i=.int
  a = i
  a = a + 1
  s = .string[]
  s[i] = "one"
  x = s[i]
  if a > 0 then say x
return
]])

execute_process(
    COMMAND "${RXC}" -n -o "${OUT_BASE}" "${SOURCE}"
    WORKING_DIRECTORY "${WORK}"
    RESULT_VARIABLE RXC_RESULT
    OUTPUT_VARIABLE RXC_OUT
    ERROR_VARIABLE RXC_ERR)

if(NOT RXC_RESULT EQUAL 0)
    message(FATAL_ERROR "rxc failed for trace-event metadata smoke test.\nstdout:\n${RXC_OUT}\nstderr:\n${RXC_ERR}")
endif()

file(READ "${OUT_BASE}.rxas" RXAS)
string(REPLACE "\n" ";" RXAS_LINES "${RXAS}")
set(METADATA "")
foreach(line IN LISTS RXAS_LINES)
    string(STRIP "${line}" stripped)
    if(stripped MATCHES "^\\.(srcstep|traceevent)( |$)")
        string(APPEND METADATA "${stripped}\n")
    endif()
endforeach()
file(WRITE "${ACTUAL}" "${METADATA}")

file(WRITE "${EXPECTED}" [[.srcstep 3 3 17 "trace_event_metadata.crexx" 2 1 16 "main: procedure"
.srcstep 1 1 17 "trace_event_metadata.crexx" 3 3 15 "  call test(1)"
.traceevent "L" 4 "R" "I" "r" 1 1 1 0 "" ""
.traceevent "F" 4 "R" "V" "r" 2 1 1 0 "test" ""
.srcstep 2 2 17 "trace_event_metadata.crexx" 4 1 7 "return"
.srcstep 14 14 17 "trace_event_metadata.crexx" 6 1 16 "test: procedure"
.srcstep 4 4 17 "trace_event_metadata.crexx" 7 7 13 "  arg i=.int"
.srcstep 5 5 17 "trace_event_metadata.crexx" 10 3 14 "  s = .string[]"
.srcstep 6 6 17 "trace_event_metadata.crexx" 8 3 8 "  a = i"
.traceevent "V" 6 "R" "I" "a" 1 6 6 0 "i" ""
.traceevent "A" 6 "R" "I" "r" 0 6 6 0 "a" ""
.srcstep 7 7 17 "trace_event_metadata.crexx" 9 3 12 "  a = a + 1"
.traceevent "V" 6 "R" "I" "r" 0 7 7 0 "a" ""
.traceevent "O" 4 "R" "I" "r" 0 7 7 0 "" ""
.traceevent "A" 6 "R" "I" "r" 0 7 7 0 "a" ""
.srcstep 8 8 17 "trace_event_metadata.crexx" 11 3 15 "  s[i] = \"one\""
.traceevent "V" 6 "R" "I" "a" 1 8 8 0 "i" ""
.traceevent "C" 4 "R" "I" "a" 1 8 8 0 "s" ""
.traceevent "L" 4 "R" "S" "r" 5 8 8 0 "" ""
.traceevent "A" 6 "R" "S" "r" 4 8 8 0 "s" ""
.srcstep 9 9 17 "trace_event_metadata.crexx" 12 3 10 "  x = s[i]"
.traceevent "V" 6 "R" "I" "a" 1 9 9 0 "i" ""
.traceevent "C" 4 "R" "I" "a" 1 9 9 0 "s" ""
.traceevent "V" 6 "R" "S" "r" 5 9 9 0 "s" ""
.traceevent "A" 6 "R" "S" "r" 3 9 9 0 "x" ""
.srcstep 11 11 17 "trace_event_metadata.crexx" 13 3 11 "  if a > 0 then say x"
.traceevent "V" 6 "R" "I" "r" 0 11 11 0 "a" ""
.traceevent "O" 4 "R" "B" "r" 5 11 11 0 "" ""
.srcstep 12 12 17 "trace_event_metadata.crexx" 13 12 16 "  if a > 0 then say x"
.srcstep 10 10 17 "trace_event_metadata.crexx" 13 17 22 "  if a > 0 then say x"
.traceevent "V" 6 "R" "S" "r" 3 10 10 0 "x" ""
.srcstep 13 13 17 "trace_event_metadata.crexx" 14 1 7 "return"
]])

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E compare_files "${EXPECTED}" "${ACTUAL}"
    RESULT_VARIABLE COMPARE_RESULT)

if(NOT COMPARE_RESULT EQUAL 0)
    find_program(DIFF_TOOL diff)
    if(DIFF_TOOL)
        execute_process(
            COMMAND "${DIFF_TOOL}" -u "${EXPECTED}" "${ACTUAL}"
            OUTPUT_VARIABLE DIFF_OUT
            ERROR_VARIABLE DIFF_ERR)
        message(FATAL_ERROR "trace-event metadata smoke test mismatch:\n${DIFF_OUT}${DIFF_ERR}")
    endif()
    file(READ "${EXPECTED}" EXPECTED_TEXT)
    file(READ "${ACTUAL}" ACTUAL_TEXT)
    message(FATAL_ERROR "trace-event metadata smoke test mismatch.\nExpected:\n${EXPECTED_TEXT}\nActual:\n${ACTUAL_TEXT}")
endif()
