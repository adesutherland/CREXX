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
target_link_libraries(rxc_single_compiler PUBLIC rxvm_single_embed
    "${CREXX_HOST_BUILD}/bin/librxaslib.a")
add_executable(rxc_single "${CREXX_SOURCE}/compiler/rxc_main.c")
target_link_libraries(rxc_single PRIVATE rxc_single_compiler ${CMAKE_DL_LIBS} m)
