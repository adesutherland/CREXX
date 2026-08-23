file(REMOVE_RECURSE "${WORK_ROOT}")
file(MAKE_DIRECTORY "${WORK_ROOT}")
set(prefix "${WORK_ROOT}/prefix")
set(consumer_source "${WORK_ROOT}/consumer-source")
set(consumer_build "${WORK_ROOT}/consumer-build")
set(mismatch_build "${WORK_ROOT}/mismatch-build")
set(plugin_dir "${consumer_build}/plugin")
set(command_log "${WORK_ROOT}/commands-and-results.txt")
set(manifest "${WORK_ROOT}/artifact-manifest.txt")

file(MAKE_DIRECTORY "${consumer_source}")
foreach(consumer_file IN ITEMS
        CMakeLists.txt
        sdk_probe.c
        sdk_probe.crexx
        sdk_const_probe.cpp
        rx_hash_installed.crexx
        rcc5de_installed.crexx)
    file(COPY_FILE
            "${CONSUMER_SOURCE_DIR}/${consumer_file}"
            "${consumer_source}/${consumer_file}")
endforeach()

function(run_checked description)
    set(options)
    set(one_value WORKING_DIRECTORY)
    set(multi_value COMMAND)
    cmake_parse_arguments(RUN "${options}" "${one_value}" "${multi_value}" ${ARGN})
    execute_process(
            COMMAND ${RUN_COMMAND}
            WORKING_DIRECTORY "${RUN_WORKING_DIRECTORY}"
            OUTPUT_VARIABLE out
            ERROR_VARIABLE err
            RESULT_VARIABLE res)
    file(APPEND "${command_log}"
            "\n[${description}]\ncommand=${RUN_COMMAND}\nexit=${res}\nstdout:\n${out}\nstderr:\n${err}\n")
    if(NOT res EQUAL 0)
        message(FATAL_ERROR "${description} failed (${res}).\nSTDOUT:\n${out}\nSTDERR:\n${err}")
    endif()
    set(last_stdout "${out}" PARENT_SCOPE)
    set(last_stderr "${err}" PARENT_SCOPE)
endfunction()

run_checked("build SDK consumer prerequisites"
        COMMAND "${CMAKE_COMMAND}" --build "${CREXX_BUILD_DIR}"
                --target cri07_rxpa_sdk_consumer_prereqs --parallel 4
        WORKING_DIRECTORY "${CREXX_BUILD_DIR}")

run_checked("install CREXX into fresh scratch prefix"
        COMMAND "${CMAKE_COMMAND}" --install "${CREXX_BUILD_DIR}" --prefix "${prefix}"
        WORKING_DIRECTORY "${WORK_ROOT}")

set(required_sdk_files
        "${prefix}/include/rxpa/crexxpa.h"
        "${prefix}/include/rxpa/crexx_version.h"
        "${prefix}/include/platform/rxinteger.h"
        "${prefix}/${PACKAGE_RELATIVE_DIR}/CREXXConfig.cmake"
        "${prefix}/${PACKAGE_RELATIVE_DIR}/CREXXConfigVersion.cmake"
        "${prefix}/${PACKAGE_RELATIVE_DIR}/CREXXTargets.cmake"
        "${prefix}/${PACKAGE_RELATIVE_DIR}/RXPluginFunction.cmake"
        "${prefix}/bin/rx_hash.rxplugin"
        "${prefix}/bin/providers/rx_hash.rxplugin"
        "${prefix}/bin/providers/rx_hash${STATIC_SUFFIX}"
        "${prefix}/bin/providers/rx_hash_static${STATIC_SUFFIX}"
        "${prefix}/bin/rxstats.rxplugin"
        "${prefix}/bin/providers/rxstats.rxplugin"
        "${prefix}/bin/providers/rxstats${STATIC_SUFFIX}"
        "${prefix}/bin/providers/rxstats_static${STATIC_SUFFIX}"
        "${prefix}/bin/rxvector.rxplugin"
        "${prefix}/bin/providers/rxvector.rxplugin"
        "${prefix}/bin/providers/rxvector${STATIC_SUFFIX}"
        "${prefix}/bin/providers/rxvector_static${STATIC_SUFFIX}"
        "${prefix}/bin/rxid.rxplugin"
        "${prefix}/bin/providers/rxid.rxplugin"
        "${prefix}/bin/providers/rxid${STATIC_SUFFIX}"
        "${prefix}/bin/providers/rxid_static${STATIC_SUFFIX}"
        "${prefix}/bin/rxfs.rxplugin"
        "${prefix}/bin/providers/rxfs.rxplugin"
        "${prefix}/bin/providers/rxfs${STATIC_SUFFIX}"
        "${prefix}/bin/providers/rxfs_static${STATIC_SUFFIX}"
        "${prefix}/bin/rxplatform.rxplugin"
        "${prefix}/bin/providers/rxplatform.rxplugin"
        "${prefix}/bin/providers/rxplatform${STATIC_SUFFIX}"
        "${prefix}/bin/providers/rxplatform_static${STATIC_SUFFIX}"
        "${prefix}/BUILDINFO"
        "${prefix}/VERSION")
foreach(required_file IN LISTS required_sdk_files)
    if(NOT EXISTS "${required_file}")
        message(FATAL_ERROR "Scratch SDK is missing ${required_file}")
    endif()
endforeach()

foreach(test_only_plugin IN ITEMS
        "${prefix}/bin/rx_rcc5f_stats_boxed.rxplugin"
        "${prefix}/bin/rx_rcc5f_stats_direct.rxplugin"
        "${prefix}/bin/rx_rxvector01_direct.rxplugin")
    if(EXISTS "${test_only_plugin}")
        message(FATAL_ERROR
                "Scratch SDK published test-only provider ${test_only_plugin}")
    endif()
endforeach()

foreach(package_file IN ITEMS
        "${prefix}/${PACKAGE_RELATIVE_DIR}/CREXXConfig.cmake"
        "${prefix}/${PACKAGE_RELATIVE_DIR}/CREXXTargets.cmake"
        "${prefix}/${PACKAGE_RELATIVE_DIR}/RXPluginFunction.cmake")
    file(READ "${package_file}" package_text)
    string(FIND "${package_text}" "${CREXX_SOURCE_DIR}" source_path_offset)
    if(NOT source_path_offset EQUAL -1)
        message(FATAL_ERROR "Installed metadata leaks CREXX source path: ${package_file}")
    endif()
endforeach()

execute_process(
        COMMAND "${CMAKE_COMMAND}" -S "${consumer_source}" -B "${mismatch_build}"
                -G "${CMAKE_GENERATOR_NAME}"
                "-DCMAKE_PREFIX_PATH=${prefix}"
                "-DEXPECTED_CREXX_CORE_VERSION=${CREXX_VERSION_CORE}"
                "-DEXPECTED_CREXX_PREFIX=${prefix}"
                "-DEXPECTED_CREXX_VERSION=definitely-not-${CREXX_VERSION}"
                "-DFORBIDDEN_CREXX_SOURCE_DIR=${CREXX_SOURCE_DIR}"
        OUTPUT_VARIABLE mismatch_out
        ERROR_VARIABLE mismatch_err
        RESULT_VARIABLE mismatch_res)
file(APPEND "${command_log}"
        "\n[reject version mismatch]\nexit=${mismatch_res}\nstdout:\n${mismatch_out}\nstderr:\n${mismatch_err}\n")
if(mismatch_res EQUAL 0 OR NOT "${mismatch_out}${mismatch_err}" MATCHES "CREXX SDK version mismatch")
    message(FATAL_ERROR "Version-mismatch configuration did not fail specifically.\n${mismatch_out}\n${mismatch_err}")
endif()

run_checked("configure external plugin from installed CREXX package"
        COMMAND "${CMAKE_COMMAND}" -S "${consumer_source}" -B "${consumer_build}"
                -G "${CMAKE_GENERATOR_NAME}"
                "-DCMAKE_PREFIX_PATH=${prefix}"
                "-DEXPECTED_CREXX_CORE_VERSION=${CREXX_VERSION_CORE}"
                "-DEXPECTED_CREXX_PREFIX=${prefix}"
                "-DEXPECTED_CREXX_VERSION=${CREXX_VERSION}"
                "-DFORBIDDEN_CREXX_SOURCE_DIR=${CREXX_SOURCE_DIR}"
        WORKING_DIRECTORY "${WORK_ROOT}")

run_checked("build external plugin verbosely"
        COMMAND "${CMAKE_COMMAND}" --build "${consumer_build}"
                --target _cri07_sdk_probe _cri08_const_consumers --verbose
        WORKING_DIRECTORY "${WORK_ROOT}")

set(plugin "${plugin_dir}/rx_cri07_sdk_probe.rxplugin")
set(cpp_plugin "${plugin_dir}/rx_cri08_const_cpp.rxplugin")
if(NOT EXISTS "${plugin}")
    message(FATAL_ERROR "External plugin was not produced at ${plugin}")
endif()
if(NOT EXISTS "${cpp_plugin}")
    message(FATAL_ERROR "External C++ plugin was not produced at ${cpp_plugin}")
endif()

set(rxc "${prefix}/bin/rxc${EXE_SUFFIX}")
set(rxas "${prefix}/bin/rxas${EXE_SUFFIX}")
set(rxvm "${prefix}/bin/rxvm${EXE_SUFFIX}")
set(rxbvm "${prefix}/bin/rxbvm${EXE_SUFFIX}")
set(crexx "${prefix}/bin/crexx${EXE_SUFFIX}")

foreach(mode IN ITEMS opt noopt)
    set(mode_flag)
    if(mode STREQUAL "noopt")
        set(mode_flag -n)
    endif()
    run_checked("compile installed rx_hash consumer ${mode}"
            COMMAND "${rxc}" -i "${prefix}/bin" ${mode_flag}
                    -o "${WORK_ROOT}/rx-hash-installed-${mode}"
                    "${consumer_source}/rx_hash_installed.crexx"
            WORKING_DIRECTORY "${WORK_ROOT}")
    run_checked("assemble installed rx_hash consumer ${mode}"
            COMMAND "${rxas}" ${mode_flag}
                    -o "${WORK_ROOT}/rx-hash-installed-${mode}.rxbin"
                    "${WORK_ROOT}/rx-hash-installed-${mode}"
            WORKING_DIRECTORY "${WORK_ROOT}")
endforeach()

foreach(vm IN ITEMS "${rxvm}" "${rxbvm}")
    foreach(mode IN ITEMS opt noopt)
        run_checked("autoload installed rx_hash with ${vm} ${mode}"
                COMMAND "${vm}" "${WORK_ROOT}/rx-hash-installed-${mode}"
                        "${prefix}/bin/library"
                WORKING_DIRECTORY "${WORK_ROOT}")
        if(NOT last_stdout STREQUAL
           "EF7E301027F931DFBA06C7DED4EF305797F43CC115A664F9AF9B57D08C3172C2\n")
            message(FATAL_ERROR
                    "Installed rx_hash output mismatch for ${vm} ${mode}:\n${last_stdout}")
        endif()
    endforeach()
endforeach()

run_checked("native-package installed rx_hash consumer"
        COMMAND "${crexx}" -native rx_hash_installed.crexx
        WORKING_DIRECTORY "${consumer_source}")
run_checked("run native-packaged installed rx_hash consumer"
        COMMAND "${consumer_source}/rx_hash_installed${EXE_SUFFIX}"
        WORKING_DIRECTORY "${consumer_source}")
if(NOT last_stdout STREQUAL
   "EF7E301027F931DFBA06C7DED4EF305797F43CC115A664F9AF9B57D08C3172C2\n")
    message(FATAL_ERROR "Installed native rx_hash output mismatch:\n${last_stdout}")
endif()

foreach(mode IN ITEMS opt noopt)
    set(mode_flag)
    if(mode STREQUAL "noopt")
        set(mode_flag -n)
    endif()
    run_checked("compile installed RCC-5 consumer ${mode}"
            COMMAND "${rxc}" -i "${prefix}/bin" ${mode_flag}
                    -o "${WORK_ROOT}/rcc5de-installed-${mode}"
                    "${consumer_source}/rcc5de_installed.crexx"
            WORKING_DIRECTORY "${WORK_ROOT}")
    run_checked("assemble installed RCC-5 consumer ${mode}"
            COMMAND "${rxas}" ${mode_flag}
                    -o "${WORK_ROOT}/rcc5de-installed-${mode}.rxbin"
                    "${WORK_ROOT}/rcc5de-installed-${mode}"
            WORKING_DIRECTORY "${WORK_ROOT}")
endforeach()

foreach(vm IN ITEMS "${rxvm}" "${rxbvm}")
    foreach(mode IN ITEMS opt noopt)
        run_checked("autoload installed RCC-5 providers with ${vm} ${mode}"
                COMMAND "${vm}" "${WORK_ROOT}/rcc5de-installed-${mode}"
                        "${prefix}/bin/library" "${prefix}/bin/rxfnsg"
                WORKING_DIRECTORY "${WORK_ROOT}")
        if(NOT last_stdout STREQUAL "PASS: installed RCC-5 providers\n")
            message(FATAL_ERROR
                    "Installed RCC-5 output mismatch for ${vm} ${mode}:\n${last_stdout}")
        endif()
    endforeach()
endforeach()

run_checked("native-package installed RCC-5 consumer"
        COMMAND "${crexx}" -native -l rxfnsg rcc5de_installed.crexx
        WORKING_DIRECTORY "${consumer_source}")
run_checked("run native-packaged installed RCC-5 consumer"
        COMMAND "${consumer_source}/rcc5de_installed${EXE_SUFFIX}"
        WORKING_DIRECTORY "${consumer_source}")
if(NOT last_stdout STREQUAL "PASS: installed RCC-5 providers\n")
    message(FATAL_ERROR "Installed native RCC-5 output mismatch:\n${last_stdout}")
endif()

execute_process(
        COMMAND "${rxc}" -i "${prefix}/bin" -o "${WORK_ROOT}/missing-import"
                "${consumer_source}/sdk_probe.crexx"
        WORKING_DIRECTORY "${WORK_ROOT}"
        OUTPUT_VARIABLE missing_import_out
        ERROR_VARIABLE missing_import_err
        RESULT_VARIABLE missing_import_res)
file(APPEND "${command_log}"
        "\n[reject missing compiler import path]\nexit=${missing_import_res}\nstdout:\n${missing_import_out}\nstderr:\n${missing_import_err}\n")
if(missing_import_res EQUAL 0)
    message(FATAL_ERROR "Compiler unexpectedly found the external plugin without its import path")
endif()

foreach(mode IN ITEMS opt noopt)
    set(mode_flag)
    if(mode STREQUAL "noopt")
        set(mode_flag -n)
    endif()
    run_checked("compile external cREXX importer ${mode}"
            COMMAND "${rxc}" -i "${plugin_dir}" -i "${prefix}/bin" ${mode_flag}
                    -o "${WORK_ROOT}/sdk-probe-${mode}"
                    "${consumer_source}/sdk_probe.crexx"
            WORKING_DIRECTORY "${WORK_ROOT}")
    run_checked("assemble external cREXX importer ${mode}"
            COMMAND "${rxas}" ${mode_flag}
                    -o "${WORK_ROOT}/sdk-probe-${mode}.rxbin"
                    "${WORK_ROOT}/sdk-probe-${mode}"
            WORKING_DIRECTORY "${WORK_ROOT}")
endforeach()

execute_process(
        COMMAND "${rxvm}" "${WORK_ROOT}/sdk-probe-opt"
                rx_cri07_sdk_probe "${prefix}/bin/library"
        WORKING_DIRECTORY "${WORK_ROOT}"
        OUTPUT_VARIABLE missing_runtime_out
        ERROR_VARIABLE missing_runtime_err
        RESULT_VARIABLE missing_runtime_res)
file(APPEND "${command_log}"
        "\n[reject missing runtime plugin path]\nexit=${missing_runtime_res}\nstdout:\n${missing_runtime_out}\nstderr:\n${missing_runtime_err}\n")
if(missing_runtime_res EQUAL 0)
    message(FATAL_ERROR "Runtime unexpectedly loaded the external plugin without its plugin path")
endif()

foreach(vm IN ITEMS "${rxvm}" "${rxbvm}")
    foreach(mode IN ITEMS opt noopt)
        run_checked("load external plugin with ${vm} ${mode}"
                COMMAND "${vm}" "${WORK_ROOT}/sdk-probe-${mode}"
                        "${plugin_dir}/rx_cri07_sdk_probe"
                        "${plugin_dir}/rx_cri08_const_cpp"
                        "${prefix}/bin/library"
                WORKING_DIRECTORY "${WORK_ROOT}")
        string(FIND "${last_stdout}" "SDK_VERSION=${CREXX_VERSION_DISPLAY}" version_marker)
        string(FIND "${last_stdout}" "SDK_ADD_RESULT=42" sum_marker)
        if(version_marker EQUAL -1 OR sum_marker EQUAL -1)
            message(FATAL_ERROR "External plugin output mismatch for ${vm} ${mode}:\n${last_stdout}")
        endif()
        string(FIND "${last_stdout}" "SDK_COPY_RESULT=copy-owned" copy_marker)
        if(copy_marker EQUAL -1)
            message(FATAL_ERROR "External plugin copy contract failed for ${vm} ${mode}:\n${last_stdout}")
        endif()
        string(FIND "${last_stdout}" "SDK_CPP_RESULT=cpp-immutable" cpp_marker)
        if(cpp_marker EQUAL -1)
            message(FATAL_ERROR "External C++ plugin result failed for ${vm} ${mode}:\n${last_stdout}")
        endif()
    endforeach()
endforeach()

file(WRITE "${manifest}"
        "crexx_version=${CREXX_VERSION}\n"
        "crexx_version_display=${CREXX_VERSION_DISPLAY}\n"
        "scratch_prefix=${prefix}\n"
        "cmake_package=${prefix}/${PACKAGE_RELATIVE_DIR}\n"
        "compiler_import_path=${plugin_dir};${prefix}/bin\n"
        "runtime_plugin_path=${plugin_dir};${prefix}/bin\n")
set(manifest_files ${required_sdk_files}
        "${consumer_build}/crexx-sdk-paths.txt"
        "${plugin}"
        "${cpp_plugin}"
        "${WORK_ROOT}/sdk-probe-opt.rxas"
        "${WORK_ROOT}/sdk-probe-opt.rxbin"
        "${WORK_ROOT}/sdk-probe-noopt.rxas"
        "${WORK_ROOT}/sdk-probe-noopt.rxbin"
        "${WORK_ROOT}/rx-hash-installed-opt.rxas"
        "${WORK_ROOT}/rx-hash-installed-opt.rxbin"
        "${WORK_ROOT}/rx-hash-installed-noopt.rxas"
        "${WORK_ROOT}/rx-hash-installed-noopt.rxbin"
        "${consumer_source}/rx_hash_installed${EXE_SUFFIX}"
        "${WORK_ROOT}/rcc5de-installed-opt.rxas"
        "${WORK_ROOT}/rcc5de-installed-opt.rxbin"
        "${WORK_ROOT}/rcc5de-installed-noopt.rxas"
        "${WORK_ROOT}/rcc5de-installed-noopt.rxbin"
        "${consumer_source}/rcc5de_installed${EXE_SUFFIX}")
foreach(manifest_file IN LISTS manifest_files)
    file(SHA256 "${manifest_file}" manifest_hash)
    file(APPEND "${manifest}" "${manifest_hash}  ${manifest_file}\n")
endforeach()

message(STATUS "CRI-07 scratch SDK manifest: ${manifest}")
