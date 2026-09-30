# Explicit STEP-06 correctness/scheduling measurements. No CTest registration.
# Included only for BUILD_TESTING with an explicit verified model directory.
add_custom_target(rxllama_qualify_old_host
    COMMAND $<TARGET_FILE:rxllama_text_old_host> $<TARGET_FILE:llama_provider>
    DEPENDS rxllama_text_old_host llama_provider_runtime_package
    VERBATIM)
foreach(_qualify_mode cpu required-gpu)
    string(REPLACE "-" "_" _qualify_target "${_qualify_mode}")
    add_custom_target(rxllama_generation_qualify_${_qualify_target}
        COMMAND ${CMAKE_COMMAND} -E time $<TARGET_FILE:rxllama_generation_bridge>
            "${_qualify_mode}" "${_smol}" "${_smol_hash}" "${_bge}" "${_bge_hash}"
        DEPENDS rxllama_generation_bridge llama_provider_runtime_package
        VERBATIM)
    add_custom_target(rxllama_embedding_qualify_${_qualify_target}
        COMMAND ${CMAKE_COMMAND} -E time $<TARGET_FILE:rxllama_embedding_bridge>
            "${_qualify_mode}" "${_bge}" "${_bge_hash}"
        DEPENDS rxllama_embedding_bridge llama_provider_runtime_package
        VERBATIM)
endforeach()
add_custom_target(rxllama_embedding_qualify_cross_device
    COMMAND ${CMAKE_COMMAND} -E time $<TARGET_FILE:rxllama_embedding_bridge>
        cross-device "${_bge}" "${_bge_hash}"
    DEPENDS rxllama_embedding_bridge llama_provider_runtime_package
    VERBATIM)
if(_llama_test_python)
    set(CREXX_LLAMA_QUALIFICATION_MODES "cpu" CACHE STRING
        "Explicit comma-separated STEP-06 hardware modes, e.g. cpu,required-gpu")
    set(_qualify_tools rxc rxas rxlink rxbvm library classlib rxfnsg
        llama_provider_runtime_package)
    if(TARGET rxtvm)
        list(APPEND _qualify_tools rxtvm)
    endif()
    foreach(_qualify_suite typed embedding-legacy generation generation-closeout package-typed package-generation)
        string(REPLACE "-" "_" _qualify_target "${_qualify_suite}")
        add_custom_target(rxllama_qualify_${_qualify_target}
            COMMAND ${CMAKE_COMMAND} -E time "${_llama_test_python}"
                "${CMAKE_SOURCE_DIR}/tests/native-inference/qualification_run.py"
                --build "${CMAKE_BINARY_DIR}" --source "${CMAKE_SOURCE_DIR}"
                --models "${CREXX_LLAMA_TEST_MODELS}"
                --output-root "${CMAKE_CURRENT_BINARY_DIR}/tests/qualification"
                --modes "${CREXX_LLAMA_QUALIFICATION_MODES}" --suite "${_qualify_suite}"
            DEPENDS ${_qualify_tools}
            VERBATIM)
        if(_qualify_suite MATCHES "^package-")
            # Full install inputs, including native compiler configuration and
            # runtime dependency metadata, must exist before the script installs.
            add_dependencies(rxllama_qualify_${_qualify_target}
                stage-c1-toolchain stage-product stage-optional crexx-provider-package)
        endif()
    endforeach()
endif()
