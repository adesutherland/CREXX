if(NOT DEFINED CREXX OR NOT DEFINED RXVME OR NOT DEFINED WORK_ROOT)
    message(FATAL_ERROR "crexx project-build test requires CREXX, RXVME and WORK_ROOT")
endif()

function(run_checked label)
    execute_process(
            COMMAND ${ARGN}
            RESULT_VARIABLE command_rc
            OUTPUT_VARIABLE command_out
            ERROR_VARIABLE command_err)
    if(NOT command_rc EQUAL 0)
        message(FATAL_ERROR
                "${label} failed (${command_rc})\nstdout:\n${command_out}\nstderr:\n${command_err}")
    endif()
    set(last_output "${command_out}${command_err}" PARENT_SCOPE)
endfunction()

function(require_text label text pattern)
    if(NOT "${text}" MATCHES "${pattern}")
        message(FATAL_ERROR
                "${label} did not contain ${pattern}\n${text}")
    endif()
endfunction()

function(reject_text label text pattern)
    if("${text}" MATCHES "${pattern}")
        message(FATAL_ERROR
                "${label} unexpectedly contained ${pattern}\n${text}")
    endif()
endfunction()

file(REMOVE_RECURSE "${WORK_ROOT}")
file(MAKE_DIRECTORY "${WORK_ROOT}/source" "${WORK_ROOT}/output")

set(alpha "${WORK_ROOT}/source/p4alpha.crexx")
set(beta "${WORK_ROOT}/source/p4beta.crexx")
set(provider "${WORK_ROOT}/source/autoloaddep.crexx")
set(consumer "${WORK_ROOT}/source/autoload_consumer.crexx")
file(WRITE "${alpha}" [=[options levelb
namespace p4alpha expose alpha
alpha: procedure = .string
  return "alpha"
]=])
file(WRITE "${beta}" [=[options levelb
namespace p4beta expose beta
beta: procedure = .string
  return "beta"
]=])
file(WRITE "${provider}" [=[options levelb
namespace autoloaddep expose hello
hello: procedure = .string
  return "autoload-ok"
]=])
file(WRITE "${consumer}" [=[options levelb
import autoloaddep
main: procedure = .int
  if autoloaddep..hello() <> "autoload-ok" then return 1
  say "PASS: crexx project autoload"
  return 0
]=])

set(library_stem "${WORK_ROOT}/output/project_library")
run_checked("clean optimized library build"
        "${CREXX}" --library "${library_stem}" "${alpha}" "${beta}" --jobs 2)
require_text("clean optimized library build" "${last_output}"
        "WAVE: project compile/assemble jobs=2")
require_text("clean optimized library build" "${last_output}"
        "BARRIER: link library")
require_text("clean optimized library build" "${last_output}"
        "PUBLISHED: library")
if(NOT EXISTS "${library_stem}.rxbin")
    message(FATAL_ERROR "clean library build did not publish ${library_stem}.rxbin")
endif()
file(SHA256 "${library_stem}.rxbin" clean_hash)

run_checked("immediate project no-op"
        "${CREXX}" --library "${library_stem}" "${alpha}" "${beta}" --jobs 2)
require_text("immediate project no-op" "${last_output}"
        "SKIP: project current")
reject_text("immediate project no-op" "${last_output}"
        "WAVE:|BARRIER:|PUBLISHED:")
file(SHA256 "${library_stem}.rxbin" no_op_hash)
if(NOT no_op_hash STREQUAL clean_hash)
    message(FATAL_ERROR "immediate no-op changed the published library")
endif()

file(APPEND "${beta}" "\n/* changed declared source */\n")
run_checked("changed declared source closure"
        "${CREXX}" --library "${library_stem}" "${alpha}" "${beta}" --jobs 2)
require_text("changed declared source closure" "${last_output}"
        "WAVE: project compile/assemble jobs=2")
require_text("changed declared source closure" "${last_output}"
        "PUBLISHED: library")

run_checked("forced project rebuild"
        "${CREXX}" --library "${library_stem}" "${alpha}" "${beta}"
        --jobs 2 --rebuild)
require_text("forced project rebuild" "${last_output}"
        "WAVE: project compile/assemble jobs=2")

set(noopt_stem "${WORK_ROOT}/output/project_library_noopt")
run_checked("explicit non-optimized library build"
        "${CREXX}" --library "${noopt_stem}" "${alpha}" "${beta}"
        --jobs auto --nooptimize)
if(NOT EXISTS "${noopt_stem}.rxbin")
    message(FATAL_ERROR "non-optimized library build did not publish its RXBIN")
endif()

set(provider_stem "${WORK_ROOT}/output/packaged_provider")
set(consumer_stem "${WORK_ROOT}/output/autoload_tool")
run_checked("packaged provider library"
        "${CREXX}" --library "${provider_stem}" "${provider}" --jobs 2)
run_checked("autoload consumer tool"
        "${CREXX}" --tool "${consumer_stem}" "${consumer}" --jobs 2
        -i "${WORK_ROOT}/output")
run_checked("execute autoload consumer"
        "${RXVME}" -l "${WORK_ROOT}/output" "${consumer_stem}.rxbin")
require_text("execute autoload consumer" "${last_output}"
        "PASS: crexx project autoload")

file(SHA256 "${consumer_stem}.rxbin" before_failure_hash)
file(WRITE "${consumer}" "options levelb\nthis is not valid cREXX source !!!\n")
execute_process(
        COMMAND "${CREXX}" --tool "${consumer_stem}" "${consumer}" --jobs 2
                -i "${WORK_ROOT}/output"
        RESULT_VARIABLE failure_rc
        OUTPUT_VARIABLE failure_out
        ERROR_VARIABLE failure_err)
if(failure_rc EQUAL 0)
    message(FATAL_ERROR
            "invalid source unexpectedly succeeded\n${failure_out}\n${failure_err}")
endif()
file(SHA256 "${consumer_stem}.rxbin" after_failure_hash)
if(NOT after_failure_hash STREQUAL before_failure_hash)
    message(FATAL_ERROR "failed build changed the previously published tool")
endif()

message(STATUS "crexx project build clean/no-op/change/rebuild/noopt/autoload/failure-publication checks passed")
