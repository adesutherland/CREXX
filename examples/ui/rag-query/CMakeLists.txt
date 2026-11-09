cmake_minimum_required(VERSION 3.24)

set(_rag_query_app "${CMAKE_CURRENT_BINARY_DIR}/rag_query_app.rxbin")
add_custom_command(OUTPUT "${_rag_query_app}"
        COMMAND $<TARGET_FILE:rxc> --no-exe-import
                -i "${CMAKE_BINARY_DIR}/bin" -o rag_query_app
                "${CMAKE_CURRENT_SOURCE_DIR}/rag_query_app.crexx"
        COMMAND $<TARGET_FILE:rxas> -o rag_query_app rag_query_app
        DEPENDS rxc rxas compiler_exit_bin ui ui_contract
                "${CMAKE_CURRENT_SOURCE_DIR}/rag_query_app.crexx"
        BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/rag_query_app.rxas"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
add_custom_target(example_ui_rag_query DEPENDS "${_rag_query_app}")

foreach(_launcher IN ITEMS rag_query_web_local rag_query_web_hosted)
    set(_output "${CMAKE_CURRENT_BINARY_DIR}/${_launcher}.rxbin")
    add_custom_command(OUTPUT "${_output}"
            COMMAND $<TARGET_FILE:rxc> --no-exe-import
                    -i "${CMAKE_CURRENT_BINARY_DIR}" -i "${CMAKE_BINARY_DIR}/bin"
                    -o ${_launcher} "${CMAKE_CURRENT_SOURCE_DIR}/${_launcher}.crexx"
            COMMAND $<TARGET_FILE:rxas> -o ${_launcher} ${_launcher}
            DEPENDS rxc rxas compiler_exit_bin example_ui_rag_query ui_web
                    "${CMAKE_CURRENT_SOURCE_DIR}/${_launcher}.crexx"
            BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/${_launcher}.rxas"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VERBATIM)
    add_custom_target(${_launcher}_artifact DEPENDS "${_output}")
endforeach()
add_custom_target(example_ui_rag_query_browser)
add_dependencies(example_ui_rag_query_browser rag_query_web_local_artifact
        rag_query_web_hosted_artifact)

install(FILES "${_rag_query_app}"
        "${CMAKE_CURRENT_BINARY_DIR}/rag_query_web_local.rxbin"
        "${CMAKE_CURRENT_BINARY_DIR}/rag_query_web_hosted.rxbin"
        rag-query.package.json hosted_service.mjs query-http.mjs query-api-server.mjs
        DESTINATION examples/ui/rag-query)
set_property(GLOBAL APPEND PROPERTY CREXX_INSTALLED_PRODUCT_TARGETS
        example_ui_rag_query_browser)

if(BUILD_TESTING)
    find_program(CREXX_UI_NODE_EXECUTABLE NAMES node)
    if(CREXX_UI_NODE_EXECUTABLE)
        add_test(NAME ui_rag_query_effect
                COMMAND "${CREXX_UI_NODE_EXECUTABLE}"
                        "${CMAKE_CURRENT_SOURCE_DIR}/test-rag-query.mjs" "${CMAKE_BINARY_DIR}")
        set_tests_properties(ui_rag_query_effect PROPERTIES TIMEOUT 30
                LABELS "examples;ui;browser;rag;effects"
                CREXX_PREP_TARGETS "example_ui_rag_query_browser;rxvm")
        add_test(NAME ui_rag_query_hosted_service
                COMMAND "${CREXX_UI_NODE_EXECUTABLE}"
                        "${CMAKE_CURRENT_SOURCE_DIR}/test-hosted-service.mjs")
        set_tests_properties(ui_rag_query_hosted_service PROPERTIES TIMEOUT 30
                LABELS "examples;ui;browser;rag;service")
        add_test(NAME ui_rag_query_http_executor
                COMMAND "${CREXX_UI_NODE_EXECUTABLE}"
                        "${CMAKE_CURRENT_SOURCE_DIR}/test-query-http.mjs")
        set_tests_properties(ui_rag_query_http_executor PROPERTIES TIMEOUT 30
                LABELS "examples;ui;browser;rag;service")
    endif()
endif()
