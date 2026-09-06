if(NOT DEFINED CREXX OR NOT DEFINED RXVME OR NOT DEFINED WORK_ROOT OR
   NOT DEFINED SOURCE_ROOT)
    message(FATAL_ERROR
            "crexx project-build test requires CREXX, RXVME, WORK_ROOT and SOURCE_ROOT")
endif()

function(run_checked label)
    message(STATUS "project contract: ${label}")
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

function(verify_project_no_op label output_file expected_hash)
    require_text("${label}" "${last_output}" "SKIP: project current")
    reject_text("${label}" "${last_output}" "WAVE:|BARRIER:|PUBLISHED:")
    file(SHA256 "${output_file}" no_op_hash)
    if(NOT no_op_hash STREQUAL expected_hash)
        message(FATAL_ERROR "${label} changed ${output_file}")
    endif()
endfunction()

file(REMOVE_RECURSE "${WORK_ROOT}")
file(MAKE_DIRECTORY "${WORK_ROOT}/source" "${WORK_ROOT}/output")

set(alpha "${WORK_ROOT}/source/projectalpha.crexx")
set(beta "${WORK_ROOT}/source/projectbeta.crexx")
set(provider "${WORK_ROOT}/source/autoloaddep.crexx")
set(consumer "${WORK_ROOT}/source/autoload_consumer.crexx")
set(class_provider "${SOURCE_ROOT}/project_link_class_provider.crexx")
set(class_consumer "${SOURCE_ROOT}/project_link_class_consumer.crexx")
file(WRITE "${alpha}" [=[options levelb
namespace projectalpha expose alpha
alpha: procedure = .string
  return "alpha"
]=])
file(WRITE "${beta}" [=[options levelb
namespace projectbeta expose beta
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

execute_process(
        COMMAND "${CREXX}" --tool ignored-output "${consumer}"
        RESULT_VARIABLE renamed_mode_rc
        OUTPUT_VARIABLE renamed_mode_out
        ERROR_VARIABLE renamed_mode_err)
if(renamed_mode_rc EQUAL 0)
    message(FATAL_ERROR "retired --tool spelling unexpectedly succeeded")
endif()
require_text("retired program-mode spelling"
        "${renamed_mode_out}${renamed_mode_err}"
        "--tool has been renamed to --program")

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
        "WAVE: project compile/assemble jobs=1")
require_text("independent member retained" "${last_output}"
        "SKIP: project member current projectalpha")
reject_text("independent member not compiled" "${last_output}"
        "START: project member projectalpha")
require_text("changed declared source closure" "${last_output}"
        "PUBLISHED: library")

# A missing or corrupt member snapshot must not authorize reuse after an edit.
file(WRITE "${library_stem}.crexx-build/members/projectalpha/dependencies.snapshot" "broken")
file(APPEND "${beta}" "\n/* another independent edit */\n")
run_checked("malformed dependency snapshot"
        "${CREXX}" --library "${library_stem}" "${alpha}" "${beta}" --jobs 2)
require_text("malformed dependency snapshot" "${last_output}" "WAVE: project compile/assemble jobs=2")

# Compiler options participate in both the project and member action keys.
foreach(option IN ITEMS --nooptimize --optimize --import-rxas --no-localisation)
    run_checked("changed compiler option ${option}"
            "${CREXX}" --library "${library_stem}" "${alpha}" "${beta}" --jobs 2 "${option}")
    require_text("changed compiler option ${option}" "${last_output}" "WAVE: project compile/assemble jobs=2")
endforeach()

run_checked("forced project rebuild"
        "${CREXX}" --library "${library_stem}" "${alpha}" "${beta}"
        --jobs 2 --rebuild)
require_text("forced project rebuild" "${last_output}"
        "WAVE: project compile/assemble jobs=2")

# Presence, even with an empty value, disables compiler exits. Locale/diagnostic
# environment also belongs to the action key, not just explicit CLI flags.
foreach(environment IN ITEMS "RXCP_DISABLE_EXIT=" "RXCP_DISABLE_EXIT=1" "CREXX_DIAGNOSTIC_LOCALE=de_DE")
    run_checked("changed compiler environment ${environment}" "${CMAKE_COMMAND}" -E env
            "${environment}" "${CREXX}" --library "${library_stem}" "${alpha}" "${beta}" --jobs 2)
    require_text("changed compiler environment" "${last_output}" "WAVE: project compile/assemble jobs=2")
    run_checked("unchanged compiler environment ${environment}" "${CMAKE_COMMAND}" -E env
            "${environment}" "${CREXX}" --library "${library_stem}" "${alpha}" "${beta}" --jobs 2)
    require_text("unchanged compiler environment" "${last_output}" "SKIP: project current")
endforeach()
run_checked("restored compiler environment" "${CREXX}" --library "${library_stem}" "${alpha}" "${beta}" --jobs 2)
require_text("restored compiler environment" "${last_output}" "WAVE: project compile/assemble jobs=2")

# Exercise tool identity and implicit executable-root changes in an isolated
# package. Never change the compiler/package which is running other tests.
if(UNIX)
    get_filename_component(tool_bin "${CREXX}" DIRECTORY)
    set(copy_prefix "${WORK_ROOT}/toolchain")
    file(MAKE_DIRECTORY "${copy_prefix}/bin")
    file(GLOB copy_inputs "${tool_bin}/*.rxbin" "${tool_bin}/*.rxplugin"
            "${tool_bin}/*.dylib" "${tool_bin}/*.so")
    file(COPY ${copy_inputs} "${CREXX}" "${tool_bin}/rxas" "${tool_bin}/rxlink"
            DESTINATION "${copy_prefix}/bin")
    file(COPY_FILE "${tool_bin}/rxc" "${copy_prefix}/bin/rxc.real")
    file(WRITE "${copy_prefix}/bin/rxc" [=[#!/bin/sh
exec "${0%/*}/rxc.real" "$@"
]=])
    file(CHMOD "${copy_prefix}/bin/rxc" "${copy_prefix}/bin/rxc.real"
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
    set(tool_args --library "${WORK_ROOT}/output/tool_identity" "${alpha}" "${beta}" --jobs 2)
    run_checked("isolated toolchain build" "${CMAKE_COMMAND}" -E env
            "CREXX_HOME=${copy_prefix}" "${copy_prefix}/bin/crexx" ${tool_args})
    file(APPEND "${copy_prefix}/bin/rxc" "# changed compiler launcher identity\n")
    run_checked("changed compiler content" "${CMAKE_COMMAND}" -E env
            "CREXX_HOME=${copy_prefix}" "${copy_prefix}/bin/crexx" ${tool_args})
    require_text("changed compiler content" "${last_output}" "WAVE: project compile/assemble jobs=2")
    run_checked("toolchain no-op" "${CMAKE_COMMAND}" -E env
            "CREXX_HOME=${copy_prefix}" "${copy_prefix}/bin/crexx" ${tool_args})
    require_text("toolchain no-op" "${last_output}" "SKIP: project current")

    file(MAKE_DIRECTORY "${WORK_ROOT}/extra-source")
    file(WRITE "${WORK_ROOT}/extra-source/newprovider.crexx"
            "options levelb\nnamespace newprovider expose answer\nanswer: procedure = .int\n  return 42\n")
    run_checked("extra provider compile" "${tool_bin}/rxc"
            -o "${WORK_ROOT}/extra-source/newprovider" "${WORK_ROOT}/extra-source/newprovider.crexx")
    run_checked("extra provider assemble" "${tool_bin}/rxas"
            -o "${WORK_ROOT}/extra-source/newprovider" "${WORK_ROOT}/extra-source/newprovider")
    file(COPY "${WORK_ROOT}/extra-source/newprovider.rxbin" DESTINATION "${copy_prefix}/bin")
    run_checked("new implicit installed import" "${CMAKE_COMMAND}" -E env
            "CREXX_HOME=${copy_prefix}" "${copy_prefix}/bin/crexx" ${tool_args})
    require_text("new implicit installed import" "${last_output}" "WAVE: project compile/assemble jobs=2")

    # Deterministic moving-input failure: each compiler launcher edits this
    # test's source after the controller has constructed its action keys.
    file(SHA256 "${WORK_ROOT}/output/tool_identity.rxbin" before_moving_hash)
    file(READ "${alpha}" alpha_before_moving)
    file(WRITE "${copy_prefix}/bin/rxc"
        "#!/bin/sh\nprintf '\n/* changed during compile */\n' >> '${alpha}'\nexec \"\${0%/*}/rxc.real\" \"\$@\"\n")
    execute_process(COMMAND "${CMAKE_COMMAND}" -E env "CREXX_HOME=${copy_prefix}"
        "${copy_prefix}/bin/crexx" ${tool_args}
        RESULT_VARIABLE moving_rc OUTPUT_VARIABLE moving_out ERROR_VARIABLE moving_err)
    if(moving_rc EQUAL 0 OR NOT "${moving_out}${moving_err}" MATCHES "project inputs changed during compilation")
        message(FATAL_ERROR "moving input did not abort publication: ${moving_out}${moving_err}")
    endif()
    file(SHA256 "${WORK_ROOT}/output/tool_identity.rxbin" after_moving_hash)
    if(NOT before_moving_hash STREQUAL after_moving_hash)
        message(FATAL_ERROR "moving-input failure replaced the published artifact")
    endif()
    foreach(stem IN ITEMS projectalpha projectbeta)
        if(EXISTS "${WORK_ROOT}/output/tool_identity.crexx-build/members/${stem}/action.sha256")
            message(FATAL_ERROR "moving-input wave retained a member action stamp")
        endif()
    endforeach()
    file(WRITE "${alpha}" "${alpha_before_moving}")
endif()

set(noopt_stem "${WORK_ROOT}/output/project_library_noopt")
run_checked("explicit non-optimized library build"
        "${CREXX}" --library "${noopt_stem}" "${alpha}" "${beta}"
        --jobs auto --nooptimize)
if(NOT EXISTS "${noopt_stem}.rxbin")
    message(FATAL_ERROR "non-optimized library build did not publish its RXBIN")
endif()

foreach(project_mode IN ITEMS library program)
    foreach(optimize_mode IN ITEMS optimized noopt)
        set(project_stem
                "${WORK_ROOT}/output/class_method_${project_mode}_${optimize_mode}")
        set(project_args
                "--${project_mode}" "${project_stem}"
                "${class_provider}" "${class_consumer}" --jobs 2)
        if(optimize_mode STREQUAL "noopt")
            list(APPEND project_args --nooptimize)
        endif()

        run_checked("${optimize_mode} class-method ${project_mode} build"
                "${CREXX}" ${project_args})
        require_text("${optimize_mode} class-method ${project_mode} build"
                "${last_output}" "WAVE: project compile/assemble jobs=2")
        require_text("${optimize_mode} class-method ${project_mode} build"
                "${last_output}" "BARRIER: link ${project_mode}")
        require_text("${optimize_mode} class-method ${project_mode} build"
                "${last_output}" "PUBLISHED: ${project_mode}")
        if(NOT EXISTS "${project_stem}.rxbin")
            message(FATAL_ERROR
                    "${optimize_mode} class-method ${project_mode} did not publish its RXBIN")
        endif()

        run_checked("execute ${optimize_mode} class-method ${project_mode}"
                "${RXVME}" "${project_stem}.rxbin")
        require_text("execute ${optimize_mode} class-method ${project_mode}"
                "${last_output}" "PASS: project source-import class method")

        file(SHA256 "${project_stem}.rxbin" project_hash)
        run_checked("${optimize_mode} class-method ${project_mode} no-op"
                "${CREXX}" ${project_args})
        verify_project_no_op(
                "${optimize_mode} class-method ${project_mode} no-op"
                "${project_stem}.rxbin" "${project_hash}")
    endforeach()
endforeach()

# An imported implementation is a dependency even with an unchanged signature:
# ordinary optimisation can put that implementation into the consumer image.
set(dependency_stem "${WORK_ROOT}/output/dependency_program")
set(dependency_args --program "${dependency_stem}" "${provider}" "${consumer}" "${alpha}" --jobs 2)
run_checked("initial source dependency program" "${CREXX}" ${dependency_args})
file(READ "${provider}" provider_source)
string(REPLACE "autoload-ok" "autoload-new" provider_changed "${provider_source}")
file(WRITE "${provider}" "${provider_changed}")
run_checked("imported implementation edit" "${CREXX}" ${dependency_args})
require_text("imported implementation edit" "${last_output}" "WAVE: project compile/assemble jobs=2")
require_text("imported implementation edit" "${last_output}" "START: project member autoload_consumer")
require_text("unrelated member retained" "${last_output}" "SKIP: project member current projectalpha")
execute_process(COMMAND "${RXVME}" "${dependency_stem}.rxbin" RESULT_VARIABLE changed_rc)
if(changed_rc EQUAL 0)
    message(FATAL_ERROR "consumer kept the old inline implementation")
endif()

# An additional exported callable is also a source dependency change.
string(REPLACE "expose hello" "expose hello extra" provider_contract "${provider_source}")
string(APPEND provider_contract "\nextra: procedure = .int\n  return 42\n")
file(WRITE "${provider}" "${provider_contract}")
run_checked("exported contract edit" "${CREXX}" ${dependency_args})
require_text("exported contract edit" "${last_output}" "WAVE: project compile/assemble jobs=2")
require_text("contract consumer selected" "${last_output}" "START: project member autoload_consumer")
run_checked("restored imported implementation executes" "${RXVME}" "${dependency_stem}.rxbin")

# Candidate discovery must notice a newly added source and changed namespace
# headers, including a source excluded from the previous compilation.
set(hidden "${WORK_ROOT}/source/hidden.crexx")
file(WRITE "${hidden}" "options levelb\nnamespace hidden expose value\nvalue: procedure = .int\n  return 7\n")
run_checked("new source candidate" "${CREXX}" ${dependency_args})
require_text("new source candidate" "${last_output}" "WAVE: project compile/assemble jobs=3")
file(WRITE "${hidden}" "options levelb\nnamespace autoloaddep expose value\nvalue: procedure = .int\n  return 7\n")
run_checked("excluded source changes namespace" "${CREXX}" ${dependency_args})
require_text("excluded source changes namespace" "${last_output}" "START: project member autoload_consumer")
file(REMOVE "${hidden}")
file(WRITE "${provider}" "${provider_source}")
run_checked("removed source candidate" "${CREXX}" ${dependency_args})
require_text("removed source candidate" "${last_output}" "WAVE: project compile/assemble jobs=3")

set(provider_stem "${WORK_ROOT}/output/packaged_provider")
set(consumer_stem "${WORK_ROOT}/output/autoload_program")
run_checked("packaged provider library"
        "${CREXX}" --library "${provider_stem}" "${provider}" --jobs 2)
run_checked("autoload consumer program"
        "${CREXX}" --program "${consumer_stem}" "${consumer}" --jobs 2
        -i "${WORK_ROOT}/output")
run_checked("execute autoload consumer"
        "${RXVME}" -l "${WORK_ROOT}/output" "${consumer_stem}.rxbin")
require_text("execute autoload consumer" "${last_output}"
        "PASS: crexx project autoload")

set(native_program_stem "${WORK_ROOT}/output/autoload_native_program")
run_checked("native autoload consumer program"
        "${CREXX}" --program "${native_program_stem}" "${consumer}" --jobs 2
        -i "${WORK_ROOT}/output" -l "${provider_stem}" --native)
set(native_program "${native_program_stem}")
if(WIN32)
    string(APPEND native_program ".exe")
endif()
if(NOT EXISTS "${native_program}")
    message(FATAL_ERROR "native program build did not publish ${native_program}")
endif()
run_checked("native program immediate no-op"
        "${CREXX}" --program "${native_program_stem}" "${consumer}" --jobs 2
        -i "${WORK_ROOT}/output" -l "${provider_stem}" --native)
require_text("native program immediate no-op" "${last_output}"
        "SKIP: project current")
require_text("native program immediate no-op" "${last_output}"
        "SKIP: native program current")
run_checked("execute native autoload consumer" "${native_program}")
require_text("execute native autoload consumer" "${last_output}"
        "PASS: crexx project autoload")

file(SHA256 "${consumer_stem}.rxbin" before_failure_hash)
file(WRITE "${consumer}" "options levelb\nthis is not valid cREXX source !!!\n")
execute_process(
        COMMAND "${CREXX}" --program "${consumer_stem}" "${consumer}" --jobs 2
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
    message(FATAL_ERROR "failed build changed the previously published program")
endif()

message(STATUS "crexx project build clean/no-op/change/rebuild/noopt/class-method/autoload/failure-publication checks passed")
