foreach(_required IN ITEMS CREXX SOURCE WORK_ROOT)
    if(NOT DEFINED ${_required} OR "${${_required}}" STREQUAL "")
        message(FATAL_ERROR "Missing required argument ${_required}")
    endif()
endforeach()

function(assert_fresh_artifact _path _sentinel)
    if(NOT EXISTS "${_path}")
        message(FATAL_ERROR "Expected fresh artifact was not produced: ${_path}")
    endif()
    file(READ "${_path}" _content)
    string(FIND "${_content}" "${_sentinel}" _sentinel_pos)
    if(NOT _sentinel_pos EQUAL -1)
        message(FATAL_ERROR "Driver left the stale artifact untouched: ${_path}")
    endif()
    file(SIZE "${_path}" _size)
    if(_size LESS 64)
        message(FATAL_ERROR "Fresh artifact is unexpectedly small (${_size} bytes): ${_path}")
    endif()
endfunction()

set(_stale_sentinel "CREXX_DRIVER_STALE_ARTIFACT_SENTINEL")

# Reproduce the reported rexxcps_levelb path: rxc must return to the driver and
# rxas must replace an already-present rxbin rather than leaving it stale.
set(_rexxcps_work "${WORK_ROOT}/rexxcps")
file(REMOVE_RECURSE "${_rexxcps_work}")
file(MAKE_DIRECTORY "${_rexxcps_work}")
file(COPY_FILE "${SOURCE}" "${_rexxcps_work}/rexxcps_levelb.crexx")
file(WRITE "${_rexxcps_work}/rexxcps_levelb.rxas" "${_stale_sentinel}\n")
file(WRITE "${_rexxcps_work}/rexxcps_levelb.rxbin" "${_stale_sentinel}\n")

execute_process(
        COMMAND "${CREXX}" rexxcps_levelb.crexx -verbose3 -noexec -nocolor
        WORKING_DIRECTORY "${_rexxcps_work}"
        RESULT_VARIABLE _rexxcps_result
        OUTPUT_VARIABLE _rexxcps_stdout
        ERROR_VARIABLE _rexxcps_stderr
        TIMEOUT 60)

if(NOT _rexxcps_result EQUAL 0)
    message(FATAL_ERROR
            "crexx did not complete the reported rexxcps_levelb compile path (rc=${_rexxcps_result}).\n"
            "stdout:\n${_rexxcps_stdout}\n"
            "stderr:\n${_rexxcps_stderr}")
endif()
assert_fresh_artifact("${_rexxcps_work}/rexxcps_levelb.rxas" "${_stale_sentinel}")
assert_fresh_artifact("${_rexxcps_work}/rexxcps_levelb.rxbin" "${_stale_sentinel}")
foreach(_stage IN ITEMS "rxc      - Compiled" "rxas     - Assembled")
    string(FIND "${_rexxcps_stdout}" "${_stage}" _stage_pos)
    if(_stage_pos EQUAL -1)
        message(FATAL_ERROR
                "crexx verbose output did not confirm stage '${_stage}'.\n"
                "stdout:\n${_rexxcps_stdout}\n"
                "stderr:\n${_rexxcps_stderr}")
    endif()
endforeach()

# Exercise the same driver output/error capture boundary with enough valid
# CRI-18 diagnostics to exceed ordinary pipe-buffer sizes. The diagnostics are
# intentional: each second assignment denotes a different implicit binding.
set(_stress_work "${WORK_ROOT}/diagnostic_stress")
file(REMOVE_RECURSE "${_stress_work}")
file(MAKE_DIRECTORY "${_stress_work}")
set(_stress_source "options levelb\n\nmain: procedure\n  return\n")
foreach(_index RANGE 1 800)
    string(APPEND _stress_source
           "\nstress_${_index}: procedure\n"
           "  do\n"
           "    value = 1\n"
           "  end\n"
           "  value = 2\n"
           "  return\n")
endforeach()
file(WRITE "${_stress_work}/warning_flood.crexx" "${_stress_source}")
file(WRITE "${_stress_work}/warning_flood.rxas" "${_stale_sentinel}\n")
file(WRITE "${_stress_work}/warning_flood.rxbin" "${_stale_sentinel}\n")

execute_process(
        COMMAND "${CREXX}" warning_flood.crexx -noexec -nocolor
        WORKING_DIRECTORY "${_stress_work}"
        RESULT_VARIABLE _stress_result
        OUTPUT_VARIABLE _stress_stdout
        ERROR_VARIABLE _stress_stderr
        TIMEOUT 60)

if(NOT _stress_result EQUAL 0)
    message(FATAL_ERROR
            "crexx stalled or failed while capturing a large valid diagnostic stream (rc=${_stress_result}).\n"
            "stdout:\n${_stress_stdout}\n"
            "stderr:\n${_stress_stderr}")
endif()
assert_fresh_artifact("${_stress_work}/warning_flood.rxas" "${_stale_sentinel}")
assert_fresh_artifact("${_stress_work}/warning_flood.rxbin" "${_stale_sentinel}")
string(LENGTH "${_stress_stderr}" _stress_stderr_size)
if(_stress_stderr_size LESS 65536)
    message(FATAL_ERROR
            "Diagnostic stress did not exceed 64 KiB (${_stress_stderr_size} bytes); test no longer exercises the capture boundary")
endif()
string(FIND "${_stress_stderr}" "800 warning(s) in source file" _warning_summary_pos)
if(_warning_summary_pos EQUAL -1)
    message(FATAL_ERROR
            "Diagnostic stress did not retain the expected CRI-18 warning summary.\n"
            "stderr:\n${_stress_stderr}")
endif()
