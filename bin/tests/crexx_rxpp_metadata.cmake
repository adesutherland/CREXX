cmake_minimum_required(VERSION 3.24)
if(NOT DEFINED CREXX OR NOT DEFINED RXVME OR NOT DEFINED RXPP OR
   NOT DEFINED MACLIB OR NOT DEFINED WORK_ROOT OR NOT DEFINED CASE)
    message(FATAL_ERROR "RXPP metadata test requires tool paths, WORK_ROOT and CASE")
endif()

function(run_command label expected)
    execute_process(COMMAND ${ARGN}
            WORKING_DIRECTORY "${WORK_ROOT}"
            RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error
            TIMEOUT 40)
    if(expected STREQUAL "success" AND NOT result EQUAL 0)
        message(FATAL_ERROR "${label} failed (${result}):\n${output}${error}")
    elseif(expected STREQUAL "failure" AND result EQUAL 0)
        message(FATAL_ERROR "${label} incorrectly succeeded:\n${output}${error}")
    endif()
    set(last_output "${output}${error}" PARENT_SCOPE)
endfunction()

function(require_output text)
    string(FIND "${last_output}" "${text}" found)
    if(found EQUAL -1)
        message(FATAL_ERROR "Missing '${text}':\n${last_output}")
    endif()
endfunction()

function(require_products stem)
    foreach(extension IN ITEMS crexx rxas rxbin)
        if(NOT EXISTS "${WORK_ROOT}/${stem}.${extension}")
            message(FATAL_ERROR "Missing ${stem}.${extension}")
        endif()
    endforeach()
endfunction()

file(REMOVE_RECURSE "${WORK_ROOT}")
file(MAKE_DIRECTORY "${WORK_ROOT}/source" "${WORK_ROOT}/source with spaces")
file(WRITE "${WORK_ROOT}/dep.crexx" [=[options levelb
namespace rxppdep expose answer
answer: procedure = .int
  return 42
]=])
run_command("prepare external module" success "${CREXX}" --noexec dep.crexx)

if(CASE STREQUAL "link")
    file(WRITE "${WORK_ROOT}/source/root.rxpp" [=[##BUILDDIR output/nested
##EXTERNAL ../dep.rxbin
##LINK bundle
options levelb
say "LINK_ROOT_OK"
]=])
    run_command("link generated root" success "${CREXX}" source/root.rxpp)
    require_products("output/nested/root")
    if(last_output MATCHES "\nLINK_ROOT_OK\n")
        message(FATAL_ERROR "##LINK executed the program")
    endif()
    run_command("execute linked root" success "${RXVME}" output/nested/bundle.rxbin)
    require_output("LINK_ROOT_OK")
    # Linking one member must not silently skip later command-line inputs.
    file(WRITE "${WORK_ROOT}/later.crexx" "options levelb\nsay \"LATER\"\n")
    run_command("link then compile next member" success
            "${CREXX}" --noexec source/root.rxpp later.crexx)
    require_products("later")
    file(WRITE "${WORK_ROOT}/source/same.rxpp" [=[##EXTERNAL ../dep.rxbin
##LINK same
options levelb
say "SAME_OUTPUT_OK"
]=])
    run_command("preserve linked deliverable" success
            "${CREXX}" --nokeep source/same.rxpp)
    run_command("execute retained linked deliverable" success
            "${RXVME}" source/same.rxbin)
    require_output("SAME_OUTPUT_OK")
    if(EXISTS "${WORK_ROOT}/source/same.rxas")
        message(FATAL_ERROR "--nokeep retained assembly intermediate")
    endif()
elseif(CASE STREQUAL "link_invalid_source")
    file(WRITE "${WORK_ROOT}/source/invalid.rxpp" [=[##EXTERNAL ../dep.rxbin
##LINK invalid_bundle
options levelb
this is invalid source !!
]=])
    run_command("reject invalid link root" failure "${CREXX}" source/invalid.rxpp)
    if(EXISTS "${WORK_ROOT}/source/invalid_bundle.rxbin")
        message(FATAL_ERROR "Invalid source produced a successful linked image")
    endif()
elseif(CASE STREQUAL "external_bare")
    file(WRITE "${WORK_ROOT}/bare.rxpp"
            "##EXTERNAL dep.rxbin\noptions levelb\nsay \"BARE_OK\"\n")
    run_command("external beside bare source" success "${CREXX}" bare.rxpp)
    require_output("BARE_OK")
elseif(CASE STREQUAL "external_spaces")
    file(COPY_FILE "${WORK_ROOT}/dep.rxbin" "${WORK_ROOT}/source with spaces/dep.rxbin")
    file(WRITE "${WORK_ROOT}/source with spaces/spaces.rxpp"
            "##EXTERNAL dep.rxbin\noptions levelb\nsay \"SPACES_OK\"\n")
    run_command("external path containing spaces" success
            "${CREXX}" "source with spaces/spaces.rxpp")
    require_output("SPACES_OK")
elseif(CASE STREQUAL "external_names")
    file(WRITE "${WORK_ROOT}/names.rxpp" [=[##EXTERNAL dependency_long.rxbin
##EXTERNAL dep
##EXTERNAL dep
##EXTERNAL DEP
##EXTERNAL ../../outside.rxbin
options levelb
]=])
    run_command("external manifest names" success "${RXPP}"
            -i names.rxpp -o names.crexx -m "${MACLIB}")
    file(STRINGS "${WORK_ROOT}/names.inc" records REGEX "^external\\|")
    set(expected "external|dependency_long.rxbin;external|dep;external|DEP;external|../../outside.rxbin")
    if(NOT records STREQUAL expected)
        message(FATAL_ERROR "External names changed or disappeared:\n${records}\nExpected:\n${expected}")
    endif()
elseif(CASE STREQUAL "external_missing")
    file(WRITE "${WORK_ROOT}/missing.rxpp"
            "##EXTERNAL\noptions levelb\nsay \"SHOULD_NOT_RUN\"\n")
    run_command("missing external name" failure "${CREXX}" missing.rxpp)
    require_output("RXPP_EXTERNAL_REQUIRES_MODULE")
    if(EXISTS "${WORK_ROOT}/missing.rxbin")
        message(FATAL_ERROR "Missing external name still compiled")
    endif()
elseif(CASE STREQUAL "norun")
    file(WRITE "${WORK_ROOT}/quiet.rxpp"
            "##NORUN\noptions levelb\nsay \"SHOULD_NOT_RUN\"\n")
    run_command("compile-only member" success "${CREXX}" quiet.rxpp)
    require_products("quiet")
    if(last_output MATCHES "\nSHOULD_NOT_RUN\n")
        message(FATAL_ERROR "##NORUN executed the program")
    endif()
    # Reprocessing must replace the manifest, not retain stale NORUN metadata.
    file(WRITE "${WORK_ROOT}/quiet.rxpp" "options levelb\nsay \"RUN_AGAIN\"\n")
    run_command("replace old metadata" success "${CREXX}" quiet.rxpp)
    require_output("RUN_AGAIN")
    file(WRITE "${WORK_ROOT}/library.rxpp" [=[##NORUN
options levelb
namespace quietlib expose value
value: procedure = .int
  return 1
]=])
    file(WRITE "${WORK_ROOT}/later.crexx" "options levelb\nsay \"LATER_OK\"\n")
    run_command("NORUN is per member" success "${CREXX}" library.rxpp later.crexx)
    require_output("LATER_OK")
else()
    message(FATAL_ERROR "Unknown RXPP metadata case: ${CASE}")
endif()
