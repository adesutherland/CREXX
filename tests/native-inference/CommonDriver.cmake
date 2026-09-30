# Isolated measurements, 17 September 2026: Debug 51.5s, Apple ASan 85.4s.
# Compilation/linking and HTTP owner processes run serially inside this harness.
find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_executable(rxllama_model_compatibility "${CMAKE_SOURCE_DIR}/tests/native-inference/model_compatibility.cpp")
target_compile_features(rxllama_model_compatibility PRIVATE cxx_std_17)
target_include_directories(rxllama_model_compatibility PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(rxllama_model_compatibility PRIVATE crexx_llama_bridge llama ggml ggml-base Threads::Threads)
set_target_properties(rxllama_model_compatibility PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests")
set(_llm_common_command "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tests/native-inference/common_driver_run.py"
    --build "${CMAKE_BINARY_DIR}" --source "${CMAKE_SOURCE_DIR}"
    --output "${CMAKE_CURRENT_BINARY_DIR}/tests/common-driver")
add_custom_target(rxllama_common_prerequisites
    DEPENDS rxllama_model_compatibility rxc rxas rxlink rxbvm library classlib rxfnsg
        llama_provider_runtime_package crexx-provider-package
    VERBATIM)
if(TARGET rxtvm)
    add_dependencies(rxllama_common_prerequisites rxtvm)
endif()
add_custom_target(rxllama_common_measure
    COMMAND ${CMAKE_COMMAND} -E time ${_llm_common_command}
    DEPENDS rxllama_common_prerequisites VERBATIM)
add_test(NAME rxllama_common_drivers COMMAND ${_llm_common_command})
set_tests_properties(rxllama_common_drivers PROPERTIES
    LABELS "library;rxpa;llama;correctness" RUN_SERIAL TRUE TIMEOUT 1800)
crexx_register_test_prep_targets(TESTS rxllama_common_drivers
    TARGETS rxllama_common_prerequisites)
