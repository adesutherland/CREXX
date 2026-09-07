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
if(NOT generated_source MATCHES "register_command\\(\\.uicommand" OR
   NOT generated_source MATCHES "ui.resource.choose")
    message(FATAL_ERROR "RXPP did not generate v0 command registration")
endif()
if(NOT generated_source MATCHES "document.open.requested")
    message(FATAL_ERROR "RXPP did not preserve the semantic Open action")
endif()
if(NOT generated_source MATCHES "right.*lines" OR
   NOT generated_source MATCHES "line.*summary-divider")
    message(FATAL_ERROR "RXPP did not preserve relative layout declarations")
endif()

foreach(launcher IN ITEMS TUI_GENERATED GTK_GENERATED ANSI_GENERATED)
    if(DEFINED ${launcher})
        file(READ "${${launcher}}" launcher_source)
        if(NOT launcher_source MATCHES "options[^\n]*levelg[^\n]*srcmap" OR
           NOT launcher_source MATCHES "make_text_inspector_session" OR
           NOT launcher_source MATCHES "capabilities" OR
           NOT launcher_source MATCHES "driver.run\\(session\\)")
            message(FATAL_ERROR "${launcher} must use the single session launcher")
        endif()
    endif()
endforeach()
