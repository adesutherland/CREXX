# Source closure comes from the actual normal compiler targets. Included C
# fragments and headers are excluded by the owning target's source properties.
set(CREXX_HOST_BUILD "" CACHE PATH "Normal build with RXC/RXAS generators and libraries")
set(manifest "${CREXX_HOST_BUILD}/mainframe/compiler_sources.txt")
if(NOT EXISTS "${manifest}")
    message(FATAL_ERROR "Configure/build normal rxc first and set CREXX_HOST_BUILD")
endif()
file(STRINGS "${manifest}" compiler_sources)
add_library(rxc_single_compiler STATIC ${compiler_sources})
target_include_directories(rxc_single_compiler PRIVATE "${CREXX_SOURCE}/compiler"
    "${CREXX_HOST_BUILD}/compiler" "${CREXX_HOST_BUILD}/assembler")
if(EMSCRIPTEN)
    # Reuse prepared scanner/parser C, never a native archive or cross-built
    # generator. The normal owning target supplies the assembler source list.
    set(assembler_manifest "${CREXX_HOST_BUILD}/mainframe/assembler_sources.txt")
    if(NOT EXISTS "${assembler_manifest}")
        message(FATAL_ERROR "Build normal rxas first and set CREXX_HOST_BUILD")
    endif()
    file(STRINGS "${assembler_manifest}" assembler_sources)
    add_library(rxas_single_lib STATIC ${assembler_sources})
    target_include_directories(rxas_single_lib PRIVATE "${CREXX_SOURCE}/assembler"
        "${CREXX_HOST_BUILD}/assembler")
    target_compile_definitions(rxas_single_lib PRIVATE _POSIX_C_SOURCE=200809L)
    target_link_libraries(rxas_single_lib PUBLIC rxvm_single_core)
    target_compile_definitions(rxc_single_compiler PRIVATE _POSIX_C_SOURCE=200809L
        CREXX_COMPILER_EMBEDDED_VM=1)
    target_link_libraries(rxc_single_compiler PUBLIC rxvm_single_embed rxas_single_lib)
    add_executable(rxas_single "${CREXX_SOURCE}/assembler/rxasmain.c")
    target_include_directories(rxas_single PRIVATE "${CREXX_SOURCE}/assembler"
        "${CREXX_HOST_BUILD}/assembler")
    target_link_libraries(rxas_single PRIVATE rxas_single_lib m)
else()
    target_link_libraries(rxc_single_compiler PUBLIC rxvm_single_embed
        "${CREXX_HOST_BUILD}/bin/librxaslib.a")
endif()
add_executable(rxc_single "${CREXX_SOURCE}/compiler/rxc_main.c")
target_link_libraries(rxc_single PRIVATE rxc_single_compiler ${CMAKE_DL_LIBS} m)
if(EMSCRIPTEN)
    foreach(tool IN ITEMS rxc_single rxas_single)
        set_target_properties(${tool} PROPERTIES SUFFIX ".mjs")
        target_link_options(${tool} PRIVATE
            -sMODULARIZE=1 -sEXPORT_ES6=1 -sENVIRONMENT=worker,node
            -sINVOKE_RUN=0 -sEXIT_RUNTIME=1 -sALLOW_MEMORY_GROWTH=1
            "-sEXPORTED_FUNCTIONS=['_main']"
            "-sEXPORTED_RUNTIME_METHODS=['callMain','FS']")
    endforeach()
endif()
