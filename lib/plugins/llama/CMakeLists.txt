# cREXX License (MIT). Optional pinned inference runtime, CPU/GPU together.
cmake_minimum_required(VERSION 3.24)
enable_language(CXX)
# On case-insensitive macOS filesystems generated/VERSION masks C++ <version>.
# This subtree needs no CREXX generated headers; the C RXPA targets receive
# their SDK include paths through CREXX::RXPA separately.
get_directory_property(_llama_includes INCLUDE_DIRECTORIES)
list(REMOVE_ITEM _llama_includes "${CMAKE_BINARY_DIR}/generated")
set_property(DIRECTORY PROPERTY INCLUDE_DIRECTORIES "${_llama_includes}")
include(FetchContent)
include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)
set(CREXX_LLAMA_ARCHIVE "" CACHE FILEPATH "Offline copy of the pinned llama.cpp source archive")
option(CREXX_LLAMA_METAL "Package the Metal backend" ${APPLE})
option(CREXX_LLAMA_CUDA "Package the CUDA backend (requires build-time toolkit)" OFF)
option(CREXX_LLAMA_VULKAN "Package the Vulkan backend (requires build-time SDK)" OFF)
set(CREXX_LLAMA_CUDA_NOTICE "$ENV{CREXX_LLAMA_CUDA_NOTICE}" CACHE FILEPATH
    "CUDA redistributable license/notice file to retain with provider consumers")
set(_llama_runtime_search_dirs "")
if(WIN32)
    # GET_RUNTIME_DEPENDENCIES needs explicit MinGW redistributable locations;
    # it does not infer the compiler's DLL directory from the development PATH.
    get_filename_component(_llama_compiler_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
    list(APPEND _llama_runtime_search_dirs "${_llama_compiler_dir}")
endif()
if(CREXX_LLAMA_CUDA)
    find_package(CUDAToolkit REQUIRED)
    list(APPEND _llama_runtime_search_dirs "${CUDAToolkit_BIN_DIR}" "${CUDAToolkit_LIBRARY_DIR}")
    if(NOT EXISTS "${CREXX_LLAMA_CUDA_NOTICE}")
        message(FATAL_ERROR "CUDA packaging requires CREXX_LLAMA_CUDA_NOTICE with the redistributable license/notice text")
    endif()
endif()
set(_llama_sha 5266f24da75dc449bd56cbed7addb9c8e4a6a73e)
set(_llama_archive_sha 2de0d87eda4696e9f6bbd771d4c623267f4e95856cce6f99793f91522f993e43)
if(CREXX_LLAMA_ARCHIVE)
    set(_llama_url "${CREXX_LLAMA_ARCHIVE}")
elseif(CREXX_ALLOW_NETWORK_DOWNLOADS)
    set(_llama_url "https://codeload.github.com/ggml-org/llama.cpp/tar.gz/${_llama_sha}")
else()
    message(FATAL_ERROR "ENABLE_LLAMA requires CREXX_LLAMA_ARCHIVE or CREXX_ALLOW_NETWORK_DOWNLOADS=ON")
endif()
FetchContent_Declare(crexx_llama_source URL "${_llama_url}"
    URL_HASH "SHA256=${_llama_archive_sha}" DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
# Keep options scoped here: unrelated CREXX targets retain their delivery form.
set(BUILD_SHARED_LIBS ON)
set(GGML_BACKEND_DL ON CACHE BOOL "" FORCE)
set(GGML_NATIVE OFF CACHE BOOL "" FORCE)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$")
    set(GGML_CPU_ALL_VARIANTS ON CACHE BOOL "" FORCE)
endif()
set(GGML_METAL ${CREXX_LLAMA_METAL} CACHE BOOL "" FORCE)
set(GGML_METAL_EMBED_LIBRARY ON CACHE BOOL "" FORCE)
set(GGML_CUDA ${CREXX_LLAMA_CUDA} CACHE BOOL "" FORCE)
set(GGML_VULKAN ${CREXX_LLAMA_VULKAN} CACHE BOOL "" FORCE)
set(GGML_BLAS OFF CACHE BOOL "" FORCE)
set(GGML_OPENMP OFF CACHE BOOL "" FORCE)
# A portable baseline must also work on older Intel hosts. Additional CPU
# variants can be qualified without changing the model/ownership contract.
foreach(feature AVX AVX2 AVX512 FMA F16C BMI2)
    set(GGML_${feature} OFF CACHE BOOL "" FORCE)
endforeach()
foreach(part COMMON TESTS TOOLS EXAMPLES SERVER APP UI MTMD)
    set(LLAMA_BUILD_${part} OFF CACHE BOOL "" FORCE)
endforeach()
set(LLAMA_BUILD_IS_DEV OFF CACHE BOOL "" FORCE)
set(LLAMA_BUILD_COMMIT "${_llama_sha}")
# The source archive is below the CREXX checkout, so upstream's git probing
# would otherwise report the parent CREXX SHA as a ggml revision.
set(GIT_EXE "")
FetchContent_MakeAvailable(crexx_llama_source)
set(_llama_provider_dir "${CMAKE_BINARY_DIR}/bin/providers")
get_property(_llama_cpu_targets DIRECTORY "${crexx_llama_source_SOURCE_DIR}/ggml/src" PROPERTY BUILDSYSTEM_TARGETS)
# CPU variants are declared in the ggml-cpu subdirectory by the pinned source.
get_property(_llama_cpu_impl_targets DIRECTORY "${crexx_llama_source_SOURCE_DIR}/ggml/src/ggml-cpu" PROPERTY BUILDSYSTEM_TARGETS)
list(APPEND _llama_cpu_targets ${_llama_cpu_impl_targets})
set(_llama_backends "")
foreach(target IN LISTS _llama_cpu_targets)
    if(target MATCHES "^ggml-cpu" AND NOT target MATCHES "-feats$")
        get_target_property(_kind ${target} TYPE)
        if(_kind STREQUAL "MODULE_LIBRARY")
            list(APPEND _llama_backends ${target})
        endif()
    endif()
endforeach()
if(NOT _llama_backends AND TARGET ggml-cpu)
    set(_llama_backends ggml-cpu)
endif()
list(REMOVE_DUPLICATES _llama_backends)
foreach(backend metal cuda vulkan)
    if(TARGET ggml-${backend})
        list(APPEND _llama_backends ggml-${backend})
    endif()
endforeach()
set(_llama_runtime llama ggml ggml-base ${_llama_backends})
if(APPLE)
    set(_llama_origin "@loader_path")
else()
    set(_llama_origin "$ORIGIN")
endif()
foreach(target IN LISTS _llama_runtime)
    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME "crexx-${target}"
        LIBRARY_OUTPUT_DIRECTORY "${_llama_provider_dir}"
        RUNTIME_OUTPUT_DIRECTORY "${_llama_provider_dir}"
        ARCHIVE_OUTPUT_DIRECTORY "${_llama_provider_dir}"
        BUILD_WITH_INSTALL_RPATH TRUE INSTALL_RPATH "${_llama_origin}")
    if(NOT MSVC AND target MATCHES "^ggml")
        target_compile_options(${target} PRIVATE $<$<CONFIG:Debug>:-O1>)
    endif()
endforeach()
set_target_properties(llama PROPERTIES OUTPUT_NAME crexx-llama-engine)
add_library(crexx_llama_bridge SHARED bridge.cpp ${CMAKE_SOURCE_DIR}/binutils/rxsha256.c)
# Source/configuration identity is inspectable independently of upstream's pin.
set(_bridge_identity "${_llama_sha};${CMAKE_CXX_COMPILER_ID};${CMAKE_CXX_COMPILER_VERSION};${CMAKE_CXX_FLAGS};${CMAKE_CXX_FLAGS_DEBUG};${CMAKE_CXX_FLAGS_RELEASE};${_llama_backends}")
foreach(source bridge.cpp bridge.h generation.h generation_utf8.h package.hpp rxllama.c typed.h CMakeLists.txt WindowsBootstrap.cmake)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
    file(SHA256 "${CMAKE_CURRENT_SOURCE_DIR}/${source}" source_hash)
    string(APPEND _bridge_identity ";${source}:${source_hash}")
endforeach()
string(SHA256 _bridge_hash "${_bridge_identity}")
target_compile_features(crexx_llama_bridge PRIVATE cxx_std_17)
target_compile_definitions(crexx_llama_bridge PRIVATE CREXX_LLAMA_BUILD
    CREXX_LLAMA_BUILD_ID="${_bridge_hash}:$<CONFIG>"
    CREXX_LLAMA_PLATFORM="${CMAKE_SYSTEM_NAME}" CREXX_LLAMA_ARCH="${CMAKE_SYSTEM_PROCESSOR}")
target_include_directories(crexx_llama_bridge PRIVATE
    "${crexx_llama_source_SOURCE_DIR}/vendor" "${CMAKE_SOURCE_DIR}/binutils/include")
target_link_libraries(crexx_llama_bridge PRIVATE llama ggml ggml-base Threads::Threads ${CMAKE_DL_LIBS})
set_target_properties(crexx_llama_bridge PROPERTIES
    OUTPUT_NAME crexx-llama VERSION 1 SOVERSION 1
    LIBRARY_OUTPUT_DIRECTORY "${_llama_provider_dir}"
    RUNTIME_OUTPUT_DIRECTORY "${_llama_provider_dir}"
    ARCHIVE_OUTPUT_DIRECTORY "${_llama_provider_dir}"
    BUILD_WITH_INSTALL_RPATH TRUE INSTALL_RPATH "${_llama_origin}")
add_dynamic_plugin_target(llama_provider PROVIDER_ID rxllama rxllama.c)
add_static_plugin_target(llama_provider PROVIDER_ID rxllama rxllama.c)
foreach(target llama_provider llama_provider_static)
    target_link_libraries(${target} PRIVATE crexx_llama_bridge)
endforeach()
set_target_properties(llama_provider PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "${_llama_origin};${_llama_origin}/providers")
set(_llama_notices "${_llama_provider_dir}/rxllama-NOTICES.txt")
file(READ "${crexx_llama_source_SOURCE_DIR}/LICENSE" _llama_license)
file(WRITE "${_llama_notices}" "llama.rexx runtime uses llama.cpp ${_llama_sha} and ggml.\n\n${_llama_license}")
if(CREXX_LLAMA_CUDA)
    file(READ "${CREXX_LLAMA_CUDA_NOTICE}" _llama_cuda_notice)
    file(APPEND "${_llama_notices}" "\nCUDA redistributable dependency notices:\n\n${_llama_cuda_notice}\n")
endif()
set(_llama_extra_runtime "${_llama_notices}")
if(MSVC)
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
    include(InstallRequiredSystemLibraries)
    list(APPEND _llama_extra_runtime ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})
endif()
add_rxpa_provider_package(llama_provider PROVIDER_ID rxllama
    LINK_TARGETS crexx_llama_bridge
    RUNTIME_TARGETS crexx_llama_bridge ${_llama_runtime}
    ENGINE_ID "${_llama_sha}"
    RUNTIME_FILES ${_llama_extra_runtime}
    RUNTIME_SEARCH_DIRECTORIES ${_llama_runtime_search_dirs}
    BACKEND_TARGETS ${_llama_backends})
if(WIN32)
    # LoadLibrary cannot resolve the public bin/rxllama plugin's imports from
    # bin/providers. Keep a small executable-adjacent bootstrap; GPU libraries
    # remain in the complete provider package, loaded by verified absolute path.
    set(_bootstrap_input "${CMAKE_CURRENT_BINARY_DIR}/rxllama-bootstrap-$<CONFIG>.cmake")
    set(_bootstrap_content "include(\"${CMAKE_CURRENT_BINARY_DIR}/rxllama-package-$<CONFIG>.cmake\")\n")
    foreach(target crexx_llama_bridge llama ggml ggml-base)
        string(APPEND _bootstrap_content "list(APPEND bootstrap_roots \"$<TARGET_FILE:${target}>\")\n")
    endforeach()
    foreach(runtime IN LISTS CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
        string(APPEND _bootstrap_content "list(APPEND bootstrap_runtime \"${runtime}\")\n")
    endforeach()
    string(APPEND _bootstrap_content "set(inventory \"${CMAKE_CURRENT_BINARY_DIR}/tests/release-smoke-bootstrap-files.txt\")\n")
    file(GENERATE OUTPUT "${_bootstrap_input}" CONTENT "${_bootstrap_content}")
    add_custom_command(TARGET llama_provider_runtime_package POST_BUILD
        COMMAND ${CMAKE_COMMAND} "-DINPUT=${_bootstrap_input}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/WindowsBootstrap.cmake" VERBATIM)
endif()
add_executable(crexx-provider-package provider_package.cpp ${CMAKE_SOURCE_DIR}/binutils/rxsha256.c)
target_compile_features(crexx-provider-package PRIVATE cxx_std_17)
target_include_directories(crexx-provider-package PRIVATE
    "${crexx_llama_source_SOURCE_DIR}/vendor" "${CMAKE_SOURCE_DIR}/binutils/include")
target_compile_definitions(crexx-provider-package PRIVATE
    CREXX_LLAMA_PLATFORM="${CMAKE_SYSTEM_NAME}" CREXX_LLAMA_ARCH="${CMAKE_SYSTEM_PROCESSOR}")
set_target_properties(crexx-provider-package PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
if(BUILD_TESTING)
    target_compile_definitions(crexx_llama_bridge PRIVATE CREXX_LLAMA_TESTING=1)
    include("${CMAKE_SOURCE_DIR}/tests/native-inference/ReleaseSmoke.cmake")
    add_test(NAME rxllama_package_windows_paths
        COMMAND ${CMAKE_COMMAND} "-DROOT=${CMAKE_SOURCE_DIR}"
            -P "${CMAKE_SOURCE_DIR}/tests/native-inference/package_windows_paths.cmake")
    set_tests_properties(rxllama_package_windows_paths PROPERTIES
        LABELS "library;rxpa;llama;filesystem;correctness")
    foreach(standard 17 20)
        set(path_test rxllama_package_utf8_cxx${standard})
        add_executable(${path_test} "${CMAKE_SOURCE_DIR}/tests/native-inference/package_utf8.cpp")
        set_target_properties(${path_test} PROPERTIES
            CXX_STANDARD ${standard} CXX_STANDARD_REQUIRED TRUE CXX_EXTENSIONS FALSE
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests")
        target_include_directories(${path_test} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
            "${crexx_llama_source_SOURCE_DIR}/vendor" "${CMAKE_SOURCE_DIR}/binutils/include")
        target_compile_definitions(${path_test} PRIVATE
            CREXX_LLAMA_PLATFORM="${CMAKE_SYSTEM_NAME}" CREXX_LLAMA_ARCH="${CMAKE_SYSTEM_PROCESSOR}")
        add_test(NAME ${path_test} COMMAND ${path_test})
        set_tests_properties(${path_test} PROPERTIES LABELS "library;rxpa;llama;filesystem;correctness")
        crexx_register_test_prep_targets(TESTS ${path_test} TARGETS ${path_test})
    endforeach()
    add_executable(rxllama_text_old_host ${CMAKE_SOURCE_DIR}/tests/native-inference/text_old_host.c)
    target_include_directories(rxllama_text_old_host PRIVATE ${CMAKE_SOURCE_DIR}/rxpa)
    target_link_libraries(rxllama_text_old_host PRIVATE rxpa CREXX::RXPA)
    set_target_properties(rxllama_text_old_host PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests")
    add_executable(rxllama_embedding_reference ${CMAKE_SOURCE_DIR}/tests/native-inference/embedding_reference_control.cpp)
    target_compile_features(rxllama_embedding_reference PRIVATE cxx_std_17)
    target_include_directories(rxllama_embedding_reference PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(rxllama_embedding_reference PRIVATE crexx_llama_bridge llama ggml ggml-base Threads::Threads)
    if(WIN32)
        target_link_libraries(rxllama_embedding_reference PRIVATE psapi)
    endif()
    set_target_properties(rxllama_embedding_reference PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests")
    add_executable(rxllama_embedding_bridge ${CMAKE_SOURCE_DIR}/tests/native-inference/embedding_bridge.cpp)
    target_compile_features(rxllama_embedding_bridge PRIVATE cxx_std_17)
    target_include_directories(rxllama_embedding_bridge PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(rxllama_embedding_bridge PRIVATE crexx_llama_bridge llama ggml ggml-base Threads::Threads)
    if(WIN32)
        target_link_libraries(rxllama_embedding_bridge PRIVATE psapi)
    endif()
    set_target_properties(rxllama_embedding_bridge PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests")
    # Generation controls stay explicit until STEP-06 establishes sanitizer scheduling.
    add_executable(rxllama_generation_bridge ${CMAKE_SOURCE_DIR}/tests/native-inference/generation_bridge.cpp)
    target_compile_features(rxllama_generation_bridge PRIVATE cxx_std_17)
    target_include_directories(rxllama_generation_bridge PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(rxllama_generation_bridge PRIVATE crexx_llama_bridge llama ggml ggml-base Threads::Threads)
    if(WIN32)
        target_link_libraries(rxllama_generation_bridge PRIVATE psapi)
    endif()
    set_target_properties(rxllama_generation_bridge PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests")
    add_executable(rxllama_generation_reference ${CMAKE_SOURCE_DIR}/tests/native-inference/generation_reference_control.cpp)
    target_compile_features(rxllama_generation_reference PRIVATE cxx_std_17)
    target_include_directories(rxllama_generation_reference PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(rxllama_generation_reference PRIVATE crexx_llama_bridge llama ggml ggml-base Threads::Threads)
    if(WIN32)
        target_link_libraries(rxllama_generation_reference PRIVATE psapi)
    endif()
    set_target_properties(rxllama_generation_reference PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests")
    # Explicit target only until isolated Debug/ASan timing establishes CTest properties.
    add_executable(rxllama_bridge_lifecycle ${CMAKE_SOURCE_DIR}/tests/native-inference/bridge_lifecycle.cpp)
    target_compile_features(rxllama_bridge_lifecycle PRIVATE cxx_std_17)
    target_include_directories(rxllama_bridge_lifecycle PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(rxllama_bridge_lifecycle PRIVATE crexx_llama_bridge Threads::Threads)
    if(WIN32)
        target_link_libraries(rxllama_bridge_lifecycle PRIVATE psapi)
    endif()
    add_test(NAME rxllama_backend_probe_cycle COMMAND rxllama_bridge_lifecycle --probe-cycle)
    set_tests_properties(rxllama_backend_probe_cycle PROPERTIES RUN_SERIAL TRUE TIMEOUT 30 LABELS "library;rxpa;llama;regression")
    crexx_register_test_prep_targets(TESTS rxllama_backend_probe_cycle TARGETS llama_provider_runtime_package rxllama_bridge_lifecycle)
    set_target_properties(rxllama_bridge_lifecycle PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests")
endif()
set(CREXX_LLAMA_TEST_MODELS "" CACHE PATH "Explicit local STEP-01 model directory for opt-in provider QA")
install(FILES README.md installation.md models.md reference.md qualification.md
    DESTINATION share/crexx/llama COMPONENT llama-docs)
install(DIRECTORY examples/ DESTINATION share/crexx/llama/examples
    COMPONENT llama-docs FILES_MATCHING PATTERN "*.crexx" PATTERN "*.md")
if(BUILD_TESTING AND CREXX_LLAMA_TEST_MODELS)
    set(_bge "${CREXX_LLAMA_TEST_MODELS}/bge-small-en-v1.5-f16.gguf")
    set(_smol "${CREXX_LLAMA_TEST_MODELS}/smollm2-360m-instruct-q8_0.gguf")
    set(_bge_hash f0b2fef971e8366438bfd2d9aefea1b0115919389448806d290237f638bae999)
    set(_smol_hash 48ab3034d0dd401fbc721eb1df3217902fee7dab9078992d66431f09b7750201)
    find_program(_llama_test_python NAMES python3 python)
    include("${CMAKE_SOURCE_DIR}/tests/native-inference/Qualification.cmake")
    if(_llama_test_python AND CREXX_LLAMA_METAL)
        # Explicit isolated target until normal/ASan timings qualify scheduling.
        # Python is a test harness dependency only, never a product dependency.
        add_custom_target(rxllama_package_measure
            COMMAND "${_llama_test_python}" "${CMAKE_SOURCE_DIR}/tests/native-inference/step03_install_qualification.py"
                "${CMAKE_BINARY_DIR}" "${CMAKE_SOURCE_DIR}" "${CREXX_LLAMA_TEST_MODELS}"
                "${CMAKE_CURRENT_BINARY_DIR}/tests/package-measure"
            DEPENDS llama_provider_runtime_package crexx-provider-package crexx rxvme rxc rxas rxlink rxbvm library
                ui_dialogs ui_terminal_view ui_ansi ui_tui example_text_inspector_artifacts
            VERBATIM)
    endif()
    set(_modes cpu)
    if(CREXX_LLAMA_METAL)
        list(APPEND _modes required-gpu)
    endif()
    foreach(mode IN LISTS _modes)
        add_test(NAME rxllama_bridge_bge_${mode} COMMAND rxllama_bridge_lifecycle
            "${mode}" "${_bge}" "${_bge_hash}" bge-small-en-v1.5)
        add_test(NAME rxllama_bridge_smol_${mode} COMMAND rxllama_bridge_lifecycle
            "${mode}" "${_smol}" "${_smol_hash}" smollm2-360m-instruct)
        add_test(NAME rxllama_bridge_both_${mode} COMMAND rxllama_bridge_lifecycle
            "${mode}" "${_bge}" "${_bge_hash}" "${_smol}" "${_smol_hash}")
        set_tests_properties(rxllama_bridge_bge_${mode} rxllama_bridge_smol_${mode}
            rxllama_bridge_both_${mode} PROPERTIES RUN_SERIAL TRUE TIMEOUT 1800
            LABELS "library;rxpa;llama;model-provisioned")
        crexx_register_test_prep_targets(
            TESTS rxllama_bridge_bge_${mode} rxllama_bridge_smol_${mode} rxllama_bridge_both_${mode}
            TARGETS llama_provider_runtime_package rxllama_bridge_lifecycle)
    endforeach()
    # Measured in isolation: Debug 7.26 s, Apple ASan 19.83 s (2026-09-14).
    # Keep nested compilers/VMs out of the broad competing pool.
    add_test(NAME rxllama_toolchain_lifecycle COMMAND ${CMAKE_COMMAND}
        "-DBUILD=${CMAKE_BINARY_DIR}" "-DSOURCE=${CMAKE_SOURCE_DIR}"
        "-DOUTPUT=${CMAKE_CURRENT_BINARY_DIR}/tests/toolchain-ctest"
        -DMODE=cpu "-DMODEL=${_bge}" "-DHASH=${_bge_hash}" -DPROFILE=bge-small-en-v1.5
        -P "${CMAKE_SOURCE_DIR}/tests/native-inference/step03_toolchain.cmake")
    set_tests_properties(rxllama_toolchain_lifecycle PROPERTIES RUN_SERIAL TRUE TIMEOUT 1800
        LABELS "library;rxpa;llama;model-provisioned")
    crexx_register_test_prep_targets(TESTS rxllama_toolchain_lifecycle
        TARGETS llama_provider_runtime_package rxc rxas rxlink rxbvm library)
    # Retain the explicit isolated measurement entry point.
    add_custom_target(rxllama_toolchain_measure
        COMMAND ${CMAKE_COMMAND} -E time ${CMAKE_COMMAND}
            "-DBUILD=${CMAKE_BINARY_DIR}" "-DSOURCE=${CMAKE_SOURCE_DIR}"
            "-DOUTPUT=${CMAKE_CURRENT_BINARY_DIR}/tests/toolchain-measure"
            -DMODE=cpu "-DMODEL=${_bge}" "-DHASH=${_bge_hash}" -DPROFILE=bge-small-en-v1.5
            -P "${CMAKE_SOURCE_DIR}/tests/native-inference/step03_toolchain.cmake"
        DEPENDS llama_provider_runtime_package rxc rxas rxlink rxbvm library
        VERBATIM)
endif()

# Publish the registered test prerequisites to qa-prep; otherwise an existing
# lifecycle executable can retain an older internal bridge argument layout.
crexx_finalize_directory_qa_tiers()
