cmake_minimum_required(VERSION 3.24)
project(crexx_native_inference_controls LANGUAGES C CXX)

# Standalone QA control: no product provider and no broad CTest registration.
set(LLAMA_SOURCE_DIR "" CACHE PATH "Exact STEP-01 llama.cpp source tree")
set(LLAMA_LIBRARY_DIR "" CACHE PATH "Built pinned llama.cpp library directory")
option(CONTROL_BUILD_UPSTREAM "Build the pinned local upstream source in this isolated QA tree" OFF)
option(CONTROL_SANITIZE "Instrument the control and upstream CPU code with AddressSanitizer" OFF)
option(CONTROL_DEBUG_GGML_OPT "Use -O1 for Debug tensor kernels, retaining assertions and sanitizer instrumentation" ON)
set(CONTROL_MODEL_DIR "" CACHE PATH "Explicitly provisioned STEP-01 model cache; enables real-model QA")
set(CONTROL_GENERATED_MODEL "" CACHE FILEPATH "Explicitly generated random llama dense GGUF smoke fixture")
option(CONTROL_TEST_METAL "Enable real Metal controls on this host" OFF)
if(NOT EXISTS "${LLAMA_SOURCE_DIR}/include/llama.h")
    message(FATAL_ERROR "Set LLAMA_SOURCE_DIR to the verified STEP-01 source")
endif()
if(CONTROL_BUILD_UPSTREAM)
    set(LLAMA_BUILD_COMMON ON CACHE BOOL "" FORCE)
    set(LLAMA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(LLAMA_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
    set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(LLAMA_BUILD_MTMD OFF CACHE BOOL "" FORCE)
    if(CONTROL_SANITIZE)
        set(LLAMA_SANITIZE_ADDRESS ON CACHE BOOL "" FORCE)
        set(GGML_SANITIZE_ADDRESS ON CACHE BOOL "" FORCE)
    endif()
    add_subdirectory("${LLAMA_SOURCE_DIR}" upstream)
    if(CONTROL_DEBUG_GGML_OPT AND NOT MSVC)
        # Unoptimized numerical inner loops make the full fixed workload
        # impractical. Keep the adapter, llama.cpp, assertions and ASan intact.
        foreach(kernel ggml-base ggml-cpu ggml-metal)
            if(TARGET ${kernel})
                target_compile_options(${kernel} PRIVATE $<$<CONFIG:Debug>:-O1>)
            endif()
        endforeach()
    endif()
    set(CONTROL_LLAMA_LIBRARY llama)
    set(CONTROL_GGML_LIBRARY ggml)
    set(CONTROL_GGML_BASE_LIBRARY ggml-base)
    set(CONTROL_COMMON_LIBRARY llama-common)
    set(LLAMA_LIBRARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin")
else()
    if(CONTROL_SANITIZE)
        message(FATAL_ERROR "Sanitizer control requires CONTROL_BUILD_UPSTREAM so all native CPU code is instrumented")
    endif()
    find_library(CONTROL_LLAMA_LIBRARY NAMES llama PATHS "${LLAMA_LIBRARY_DIR}" NO_DEFAULT_PATH REQUIRED)
    find_library(CONTROL_GGML_LIBRARY NAMES ggml PATHS "${LLAMA_LIBRARY_DIR}" NO_DEFAULT_PATH REQUIRED)
    find_library(CONTROL_GGML_BASE_LIBRARY NAMES ggml-base PATHS "${LLAMA_LIBRARY_DIR}" NO_DEFAULT_PATH REQUIRED)
    find_library(CONTROL_COMMON_LIBRARY NAMES llama-common PATHS "${LLAMA_LIBRARY_DIR}" NO_DEFAULT_PATH REQUIRED)
endif()
find_package(Threads REQUIRED)
add_executable(native_inference_control direct_control.cpp)
target_compile_features(native_inference_control PRIVATE cxx_std_17)
target_compile_definitions(native_inference_control PRIVATE CONTROL_TIMING_GATE=$<IF:$<AND:$<CONFIG:Release>,$<NOT:$<BOOL:${CONTROL_SANITIZE}>>>,1,0>)
target_include_directories(native_inference_control PRIVATE
    "${LLAMA_SOURCE_DIR}/include" "${LLAMA_SOURCE_DIR}/ggml/include"
    "${LLAMA_SOURCE_DIR}/vendor")
target_link_libraries(native_inference_control PRIVATE
    "${CONTROL_LLAMA_LIBRARY}" "${CONTROL_GGML_LIBRARY}"
    "${CONTROL_GGML_BASE_LIBRARY}" Threads::Threads)
set_target_properties(native_inference_control PROPERTIES BUILD_RPATH "${LLAMA_LIBRARY_DIR}")
if(WIN32)
    target_link_libraries(native_inference_control PRIVATE psapi)
endif()
add_executable(native_inference_template_control template_control.cpp)
target_compile_features(native_inference_template_control PRIVATE cxx_std_17)
target_include_directories(native_inference_template_control PRIVATE
    "${LLAMA_SOURCE_DIR}/include" "${LLAMA_SOURCE_DIR}/ggml/include"
    "${LLAMA_SOURCE_DIR}/vendor" "${LLAMA_SOURCE_DIR}/common")
target_link_libraries(native_inference_template_control PRIVATE "${CONTROL_COMMON_LIBRARY}"
    "${CONTROL_LLAMA_LIBRARY}" "${CONTROL_GGML_LIBRARY}" "${CONTROL_GGML_BASE_LIBRARY}")
set_target_properties(native_inference_template_control PROPERTIES BUILD_RPATH "${LLAMA_LIBRARY_DIR}")
add_executable(native_inference_fixture_generator fixture_loader.cpp "${LLAMA_SOURCE_DIR}/tests/test-llama-archs.cpp")
set_source_files_properties("${LLAMA_SOURCE_DIR}/tests/test-llama-archs.cpp"
    PROPERTIES COMPILE_DEFINITIONS "main=llama_arch_fixture_main")
target_compile_features(native_inference_fixture_generator PRIVATE cxx_std_17)
target_include_directories(native_inference_fixture_generator PRIVATE
    "${LLAMA_SOURCE_DIR}/include" "${LLAMA_SOURCE_DIR}/ggml/include" "${LLAMA_SOURCE_DIR}/src"
    "${LLAMA_SOURCE_DIR}/vendor" "${LLAMA_SOURCE_DIR}/common")
target_link_libraries(native_inference_fixture_generator PRIVATE "${CONTROL_COMMON_LIBRARY}"
    "${CONTROL_LLAMA_LIBRARY}" "${CONTROL_GGML_LIBRARY}" "${CONTROL_GGML_BASE_LIBRARY}")
set_target_properties(native_inference_fixture_generator PROPERTIES BUILD_RPATH "${LLAMA_LIBRARY_DIR}")
if(CONTROL_SANITIZE)
    foreach(target native_inference_control native_inference_template_control native_inference_fixture_generator)
        target_compile_definitions(${target} PRIVATE CONTROL_ASAN=1)
        target_compile_options(${target} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address)
    endforeach()
endif()

enable_testing()
# This isolated QA project never registers tests in the root product pool.
# Fixed-workload Debug/ASan measurements support serial 600-second limits.
add_test(NAME native_inference_control_usage COMMAND native_inference_control --help)
add_test(NAME native_inference_embedding_tripwire COMMAND native_inference_control --test-tripwire)
if(CONTROL_MODEL_DIR)
    set(bge "${CONTROL_MODEL_DIR}/bge-small-en-v1.5-f16.gguf")
    set(smol "${CONTROL_MODEL_DIR}/smollm2-360m-instruct-q8_0.gguf")
    set(stories "${CONTROL_MODEL_DIR}/stories260K.gguf")
    foreach(model IN ITEMS "${bge}" "${smol}" "${stories}")
        if(NOT EXISTS "${model}")
            message(FATAL_ERROR "Provision and verify model: ${model}")
        endif()
    endforeach()
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/results")
    set(devices cpu)
    if(CONTROL_TEST_METAL)
        list(APPEND devices metal)
    endif()
    foreach(device IN LISTS devices)
        foreach(profile bge smol)
            add_test(NAME native_inference_${profile}_${device} COMMAND native_inference_control
                "${${profile}}" "${LLAMA_LIBRARY_DIR}" "${device}" "${profile}"
                "${CMAKE_CURRENT_BINARY_DIR}/results/${profile}-${device}.json" 100 20 4 2)
            set_tests_properties(native_inference_${profile}_${device} PROPERTIES RUN_SERIAL TRUE TIMEOUT 1800)
        endforeach()
        add_test(NAME native_inference_co_resident_${device} COMMAND native_inference_control
            --co-resident "${bge}" "${smol}" "${LLAMA_LIBRARY_DIR}" "${device}"
            "${CMAKE_CURRENT_BINARY_DIR}/results/co-resident-${device}.json")
        set_tests_properties(native_inference_co_resident_${device} PROPERTIES RUN_SERIAL TRUE TIMEOUT 1800)
    endforeach()
    # Explicit CPU configuration control: the two-thread Smol prefill exceeded
    # the conservative 250 ms call budget once on this M5. Keep that lane, and
    # qualify four threads separately without changing work or thresholds.
    add_test(NAME native_inference_smol_cpu_threads4 COMMAND native_inference_control
        "${smol}" "${LLAMA_LIBRARY_DIR}" cpu smol
        "${CMAKE_CURRENT_BINARY_DIR}/results/smol-cpu-threads4.json" 100 20 4 4)
    set_tests_properties(native_inference_smol_cpu_threads4 PROPERTIES RUN_SERIAL TRUE TIMEOUT 1800)
    add_test(NAME native_inference_scratch COMMAND native_inference_control "${stories}"
        "${LLAMA_LIBRARY_DIR}" cpu scratch "${CMAKE_CURRENT_BINARY_DIR}/results/scratch.json" 100 20 4 2)
    add_test(NAME native_inference_template COMMAND native_inference_template_control "${smol}"
        "${LLAMA_LIBRARY_DIR}/${CMAKE_SHARED_MODULE_PREFIX}ggml-cpu${CMAKE_SHARED_MODULE_SUFFIX}"
        "${CMAKE_CURRENT_BINARY_DIR}/results/template.json")
    set_tests_properties(native_inference_scratch native_inference_template PROPERTIES RUN_SERIAL TRUE TIMEOUT 120)
endif()
if(CONTROL_GENERATED_MODEL)
    if(NOT EXISTS "${CONTROL_GENERATED_MODEL}")
        message(FATAL_ERROR "Generate the explicit random GGUF fixture before testing")
    endif()
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/results")
    add_test(NAME native_inference_generated_scratch COMMAND native_inference_control
        "${CONTROL_GENERATED_MODEL}" "${LLAMA_LIBRARY_DIR}" cpu scratch
        "${CMAKE_CURRENT_BINARY_DIR}/results/generated-scratch.json" 100 20 4 2)
    set_tests_properties(native_inference_generated_scratch PROPERTIES RUN_SERIAL TRUE TIMEOUT 120)
endif()
