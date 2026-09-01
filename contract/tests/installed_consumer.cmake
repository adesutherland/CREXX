file(REMOVE_RECURSE "${WORK}")
file(MAKE_DIRECTORY "${WORK}")
if(NOT DEFINED CONTRACT_TOOL_FILE_NAME OR CONTRACT_TOOL_FILE_NAME STREQUAL "")
    message(FATAL_ERROR "installed_consumer.cmake requires CONTRACT_TOOL_FILE_NAME")
endif()
set(prefix "${WORK}/prefix")
set(build "${WORK}/build")
set(log "${WORK}/commands-and-results.txt")

function(run_checked label)
    execute_process(
            COMMAND ${ARGN}
            RESULT_VARIABLE result
            OUTPUT_VARIABLE stdout
            ERROR_VARIABLE stderr)
    file(APPEND "${log}"
            "[${label}]\nstdout:\n${stdout}\nstderr:\n${stderr}\nexit=${result}\n\n")
    if(NOT result EQUAL 0)
        message(FATAL_ERROR "${label} failed (${result}): ${stderr}")
    endif()
endfunction()

run_checked(install
        "${CMAKE_COMMAND}" --install "${CREXX_BUILD}" --prefix "${prefix}")
run_checked(configure-external
        "${CMAKE_COMMAND}"
        -S "${EXTERNAL_SOURCE}"
        -B "${build}"
        -DCMAKE_PREFIX_PATH=${prefix}
        -DCMAKE_BUILD_TYPE=Debug)
run_checked(build-external
        "${CMAKE_COMMAND}" --build "${build}" --target external_operation_contract)
run_checked(compare-external
        "${CMAKE_COMMAND}"
        "-DACTUAL=${build}/external-operation.rxcontract.json"
        "-DEXPECTED=${EXTERNAL_GOLDEN}"
        -P "${CMAKE_CURRENT_LIST_DIR}/compare_text_files.cmake")

file(READ "${prefix}/lib/cmake/CREXX/CREXXConfig.cmake" installed_config)
file(READ "${prefix}/lib/cmake/CREXX/CrexxOperationContract.cmake" installed_helper)
if(installed_config MATCHES "${CREXX_SOURCE}" OR
   installed_helper MATCHES "${CREXX_SOURCE}")
    message(FATAL_ERROR "installed contract package leaks the CREXX source-tree path")
endif()

set(manifest "")
foreach(path IN ITEMS
        "${prefix}/bin/${CONTRACT_TOOL_FILE_NAME}"
        "${prefix}/lib/cmake/CREXX/CREXXConfig.cmake"
        "${prefix}/lib/cmake/CREXX/CrexxOperationContract.cmake"
        "${build}/external-operation.rxbin"
        "${build}/external-operation.rxcontract.json")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "installed-consumer artifact is missing: ${path}")
    endif()
    file(SHA256 "${path}" hash)
    string(APPEND manifest "${hash}  ${path}\n")
endforeach()
file(WRITE "${WORK}/artifact-manifest.sha256" "${manifest}")
