# Measured in isolation: Debug 44.5s, Apple ASan 48.3s (15 September 2026).
# CI also invokes this harness once against its final signed/staged payload.
find_package(Python3 COMPONENTS Interpreter REQUIRED)
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/tests/release-smoke-default-vm.txt"
    CONTENT "${CREXX_DEFAULT_VM_TARGET}\n")
add_executable(rxllama_release_engine_smoke "${CMAKE_SOURCE_DIR}/tests/native-inference/release_engine_smoke.cpp")
target_compile_features(rxllama_release_engine_smoke PRIVATE cxx_std_17)
target_include_directories(rxllama_release_engine_smoke PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(rxllama_release_engine_smoke PRIVATE crexx_llama_bridge llama ggml ggml-base Threads::Threads)
set_target_properties(rxllama_release_engine_smoke PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/tests"
    BUILD_WITH_INSTALL_RPATH TRUE INSTALL_RPATH "${_llama_origin}")
set(_llama_release_smoke_command
    "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tests/native-inference/release_smoke.py"
        --build "${CMAKE_BINARY_DIR}" --source "${CMAKE_SOURCE_DIR}"
        --helper $<TARGET_FILE:rxllama_release_engine_smoke>
        --output-root "${CMAKE_CURRENT_BINARY_DIR}/tests/release-smoke")
add_custom_target(rxllama_release_smoke_prerequisites
    DEPENDS rxllama_release_engine_smoke llama_provider_runtime_package
        crexx-provider-package stage-c1-toolchain stage-product stage-optional
        rxc rxas rxlink rxbvm library classlib rxfnsg
    VERBATIM)
add_custom_target(rxllama_release_smoke_measure
    COMMAND ${_llama_release_smoke_command}
    DEPENDS rxllama_release_smoke_prerequisites VERBATIM)
add_test(NAME rxllama_release_package_smoke COMMAND ${_llama_release_smoke_command})
set_tests_properties(rxllama_release_package_smoke PROPERTIES
    LABELS "library;rxpa;llama;qualification" RUN_SERIAL TRUE TIMEOUT 3600)
crexx_register_test_prep_targets(TESTS rxllama_release_package_smoke
    TARGETS rxllama_release_smoke_prerequisites)
