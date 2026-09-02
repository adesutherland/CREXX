if(NOT DEFINED GENERATED OR NOT EXISTS "${GENERATED}")
    message(FATAL_ERROR "generated Text Inspector source is missing: ${GENERATED}")
endif()

file(READ "${GENERATED}" generated_source)
if(NOT generated_source MATCHES "options[^\n]*levelg[^\n]*srcmap")
    message(FATAL_ERROR "RXPP output is not source-mapped Level G")
endif()
if(NOT generated_source MATCHES "add_spec")
    message(FATAL_ERROR "RXPP did not generate logical node construction")
endif()
if(NOT generated_source MATCHES "document.open.requested")
    message(FATAL_ERROR "RXPP did not preserve the semantic Open action")
endif()
if(NOT generated_source MATCHES "right.*lines" OR
   NOT generated_source MATCHES "line.*summary-divider")
    message(FATAL_ERROR "RXPP did not preserve relative layout declarations")
endif()

foreach(launcher IN ITEMS TUI_GENERATED GTK_GENERATED)
    if(DEFINED ${launcher})
        if(NOT EXISTS "${${launcher}}")
            message(FATAL_ERROR "generated launcher is missing: ${${launcher}}")
        endif()
        file(READ "${${launcher}}" launcher_source)
        if(NOT launcher_source MATCHES "options[^\n]*levelg" OR
           NOT launcher_source MATCHES "uiruntime\\(app\\)" OR
           NOT launcher_source MATCHES "driver.run\\(runtime\\)")
            message(FATAL_ERROR
                    "${launcher} does not contain the generated launcher: ${${launcher}}")
        endif()
    endif()
endforeach()
