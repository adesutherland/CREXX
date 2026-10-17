cmake_minimum_required(VERSION 3.24)

# Native rxbuild command for .build definitions. The build definition remains
# a runtime input; only the grammar and reader are packaged into the command.
set(_crexx_rxbuild_root "${CMAKE_CURRENT_BINARY_DIR}/rxbuild")
set(_crexx_rxbuild_grammar_rxas "${_crexx_rxbuild_root}/rxbuildgrammar.rxas")
set(_crexx_rxbuild_grammar_rxbin "${_crexx_rxbuild_root}/rxbuildgrammar.rxbin")
set(_crexx_rxbuild_reader_rxas "${_crexx_rxbuild_root}/rxbuild.rxas")
set(_crexx_rxbuild_reader_rxbin "${_crexx_rxbuild_root}/rxbuild.rxbin")
set(_crexx_rxbuild_linked_rxbin "${_crexx_rxbuild_root}/rxbuild-linked.rxbin")
set(_crexx_rxbuild_source "${CMAKE_CURRENT_SOURCE_DIR}/build_reader.crexx")
set(_crexx_rxbuild_spec "${CMAKE_CURRENT_SOURCE_DIR}/grammar/build.levell")
set(_crexx_rxbuild_demo_source "${CMAKE_CURRENT_SOURCE_DIR}/build_demo.crexx")
set(_crexx_rxbuild_demo_rxas "${CMAKE_BINARY_DIR}/build/build_demo.rxas")
set(_crexx_rxbuild_demo_rxbin "${CMAKE_BINARY_DIR}/build/build_demo.rxbin")
add_custom_command(
        OUTPUT ${_crexx_rxbuild_grammar_rxbin}
        BYPRODUCTS ${_crexx_rxbuild_grammar_rxas}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_crexx_rxbuild_root}"
        COMMAND ${CMAKE_COMMAND}
                -DLEVEL_L=$<TARGET_FILE:rxlang>
                -DSPEC=${_crexx_rxbuild_spec}
                -DOUTPUT=${_crexx_rxbuild_grammar_rxas}
                -DFORMAT=rxas
                -P "${CMAKE_SOURCE_DIR}/langtools/cmake/generate.cmake"
        COMMAND $<TARGET_FILE:rxas> -o "${_crexx_rxbuild_root}/rxbuildgrammar"
                "${_crexx_rxbuild_grammar_rxas}"
        DEPENDS rxlang rxas "${_crexx_rxbuild_spec}"
                "${CMAKE_SOURCE_DIR}/langtools/cmake/generate.cmake"
        VERBATIM)
add_custom_command(
        OUTPUT ${_crexx_rxbuild_reader_rxbin}
        BYPRODUCTS ${_crexx_rxbuild_reader_rxas}
        COMMAND $<TARGET_FILE:rxc>
                --no-exe-import
                -i "${CMAKE_BINARY_DIR}/bin;${_crexx_rxbuild_root}"
                -o "${_crexx_rxbuild_root}/rxbuild"
                "${_crexx_rxbuild_source}"
        COMMAND $<TARGET_FILE:rxas>
                -o "${_crexx_rxbuild_root}/rxbuild"
                "${_crexx_rxbuild_reader_rxas}"
        DEPENDS rxc rxas ${_crexx_rxbuild_grammar_rxbin}
                "${_crexx_rxbuild_source}"
        VERBATIM)
add_custom_command(
        OUTPUT ${_crexx_rxbuild_demo_rxbin}
        BYPRODUCTS ${_crexx_rxbuild_demo_rxas}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/build"
        COMMAND $<TARGET_FILE:rxc>
                --no-exe-import
                -i "${CMAKE_BINARY_DIR}/bin"
                -o "${CMAKE_BINARY_DIR}/build/build_demo"
                "${_crexx_rxbuild_demo_source}"
        COMMAND $<TARGET_FILE:rxas>
                -o "${CMAKE_BINARY_DIR}/build/build_demo"
                "${_crexx_rxbuild_demo_rxas}"
        DEPENDS rxc rxas "${_crexx_rxbuild_demo_source}"
        VERBATIM)
add_custom_command(
        OUTPUT ${_crexx_rxbuild_linked_rxbin}
        COMMAND $<TARGET_FILE:rxlink> -s -o "${_crexx_rxbuild_root}/rxbuild-linked"
                "${_crexx_rxbuild_reader_rxbin}"
                "${_crexx_rxbuild_grammar_rxbin}"
                "${CMAKE_BINARY_DIR}/bin/library.rxbin"
                "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
        DEPENDS rxlink ${_crexx_rxbuild_reader_rxbin}
                ${_crexx_rxbuild_grammar_rxbin}
                library rxfnsl
        VERBATIM)
add_custom_command(
        OUTPUT "${_crexx_rxbuild_root}/rxbuild.c"
        COMMAND $<TARGET_FILE:rxcpack> -o "${_crexx_rxbuild_root}/rxbuild"
                "${_crexx_rxbuild_root}/rxbuild-linked"
        DEPENDS rxcpack ${_crexx_rxbuild_linked_rxbin}
        VERBATIM)
add_executable(rxbuild "${_crexx_rxbuild_root}/rxbuild.c")
configure_linker_for_static_lib(rxbuild fs)
configure_linker_for_static_lib(rxbuild _platform PROVIDER_ID rxplatform)
configure_linker_for_static_lib(rxbuild _hash)
target_link_libraries(rxbuild rxvml rxpashim)
add_dependencies(rxbuild fs fs_static _platform _platform_static _hash _hash_static)
add_custom_target(rxbuild_example DEPENDS ${_crexx_rxbuild_demo_rxbin})
add_dependencies(rxbuild rxbuild_example)


# Published products owned by this component.
crexx_install_targets(rxbuild)

if(BUILD_TESTING)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    foreach(_case missing blocked)
        add_test(NAME rxbuild_directory_${_case}
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/check-paths.py
                --rxbuild $<TARGET_FILE:rxbuild> --case ${_case})
        set_tests_properties(rxbuild_directory_${_case} PROPERTIES
            LABELS "tools;rxbuild;negative" TIMEOUT 120
            CREXX_PREP_TARGETS "rxbuild;crexx")
    endforeach()
    crexx_finalize_directory_qa_tiers()
endif()
