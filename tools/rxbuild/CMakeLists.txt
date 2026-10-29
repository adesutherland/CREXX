cmake_minimum_required(VERSION 3.24)

# Legacy compatibility command for .build definitions. The build definition
# remains a runtime input; only the grammar and reader are packaged into the
# separately named rxbuild-legacy executable.
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
        DEPENDS rxc rxas library compiler_exit_bin
                "${CMAKE_BINARY_DIR}/bin/library.rxbin"
                "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
                ${_crexx_rxbuild_grammar_rxbin}
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
        # Ordinary compilation loads the library and certified exits. Order
        # both producers and track their images for incremental recompilation.
        DEPENDS rxc rxas library compiler_exit_bin
                "${CMAKE_BINARY_DIR}/bin/library.rxbin"
                "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
                "${_crexx_rxbuild_demo_source}"
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
add_executable(rxbuild_legacy "${_crexx_rxbuild_root}/rxbuild.c")
set_target_properties(rxbuild_legacy PROPERTIES OUTPUT_NAME "rxbuild-legacy")
configure_linker_for_static_lib(rxbuild_legacy fs)
configure_linker_for_static_lib(rxbuild_legacy _platform PROVIDER_ID rxplatform)
configure_linker_for_static_lib(rxbuild_legacy _hash)
target_link_libraries(rxbuild_legacy rxvml rxpashim)
add_dependencies(rxbuild_legacy fs fs_static _platform _platform_static _hash _hash_static)
add_custom_target(rxbuild_example DEPENDS ${_crexx_rxbuild_demo_rxbin})
add_dependencies(rxbuild_legacy rxbuild_example)


# Published products owned by this component.
crexx_install_targets(rxbuild_legacy)

if(BUILD_TESTING)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    if(UNIX AND CMAKE_GENERATOR MATCHES "^(Ninja|Unix Makefiles)$")
        set(_rxbuild_order_command ${Python3_EXECUTABLE}
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/check-demo-build.py"
            --source "${CMAKE_SOURCE_DIR}" --bin "$<TARGET_FILE_DIR:rxc>"
            --cmake "${CMAKE_COMMAND}" --generator "${CMAKE_GENERATOR}"
            --make-program "${CMAKE_MAKE_PROGRAM}")
        # Measure with already prepared tools, without timing their rebuild.
        add_custom_target(rxbuild_demo_order_measure
            COMMAND ${_rxbuild_order_command} VERBATIM)
        # Isolated fixture: Debug/Ninja 6s, prepared Apple-ASan/Make tools 17s.
        # Serialize nested builds; the timeout is generous hang protection.
        add_test(NAME rxbuild_demo_build_order COMMAND ${_rxbuild_order_command})
        set_tests_properties(rxbuild_demo_build_order PROPERTIES
            PASS_REGULAR_EXPRESSION "RXBUILD_DEMO_BUILD[|]PASS"
            RUN_SERIAL TRUE TIMEOUT 600 LABELS "tools;rxbuild;cmake;essential"
            CREXX_PREP_TARGETS "rxc;rxas;rxbvm;library;compiler_exit_bin")
    endif()
    foreach(_case missing blocked)
        add_test(NAME rxbuild_directory_${_case}
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/check-paths.py
                --rxbuild $<TARGET_FILE:rxbuild_legacy> --case ${_case})
        set_tests_properties(rxbuild_directory_${_case} PROPERTIES
            LABELS "tools;rxbuild;negative" TIMEOUT 120
            CREXX_PREP_TARGETS "rxbuild_legacy;crexx")
    endforeach()
    crexx_finalize_directory_qa_tiers()
endif()

# Canonical graph build reader.  The legacy reader remains available as the
# separately named rxbuild-legacy compatibility executable.
set(_crexx_rxgraph_root "${CMAKE_CURRENT_BINARY_DIR}/rxgraphbuild")
set(_crexx_rxgraph_grammar_rxas "${_crexx_rxgraph_root}/graphgrammar.rxas")
set(_crexx_rxgraph_grammar_rxbin "${_crexx_rxgraph_root}/graphgrammar.rxbin")
set(_crexx_rxgraph_reader_rxas "${_crexx_rxgraph_root}/graph_reader.rxas")
set(_crexx_rxgraph_reader_rxbin "${_crexx_rxgraph_root}/graph_reader.rxbin")
set(_crexx_rxgraph_linked_rxbin "${_crexx_rxgraph_root}/graph_reader-linked.rxbin")
set(_crexx_rxgraph_source "${CMAKE_CURRENT_SOURCE_DIR}/graph/graph_reader.crexx")
set(_crexx_rxgraph_spec "${CMAKE_CURRENT_SOURCE_DIR}/graph/graph.levell")

add_custom_command(
        OUTPUT ${_crexx_rxgraph_grammar_rxbin}
        BYPRODUCTS ${_crexx_rxgraph_grammar_rxas}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_crexx_rxgraph_root}"
        COMMAND ${CMAKE_COMMAND}
                -DLEVEL_L=$<TARGET_FILE:rxlang>
                -DSPEC=${_crexx_rxgraph_spec}
                -DOUTPUT=${_crexx_rxgraph_grammar_rxas}
                -DFORMAT=rxas
                -P "${CMAKE_SOURCE_DIR}/langtools/cmake/generate.cmake"
        COMMAND $<TARGET_FILE:rxas> -o "${_crexx_rxgraph_root}/graphgrammar"
                "${_crexx_rxgraph_grammar_rxas}"
        DEPENDS rxlang rxas "${_crexx_rxgraph_spec}"
                "${CMAKE_SOURCE_DIR}/langtools/cmake/generate.cmake"
        VERBATIM)

add_custom_command(
        OUTPUT ${_crexx_rxgraph_reader_rxbin}
        BYPRODUCTS ${_crexx_rxgraph_reader_rxas}
        COMMAND $<TARGET_FILE:rxc>
                --no-exe-import
                -i "${CMAKE_BINARY_DIR}/bin;${_crexx_rxgraph_root}"
                -o "${_crexx_rxgraph_root}/graph_reader"
                "${_crexx_rxgraph_source}"
        COMMAND $<TARGET_FILE:rxas>
                -o "${_crexx_rxgraph_root}/graph_reader"
                "${_crexx_rxgraph_reader_rxas}"
        DEPENDS rxc rxas library compiler_exit_bin
                "${CMAKE_BINARY_DIR}/bin/library.rxbin"
                "${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin"
                ${_crexx_rxgraph_grammar_rxbin}
                "${_crexx_rxgraph_source}"
        VERBATIM)

add_custom_command(
        OUTPUT ${_crexx_rxgraph_linked_rxbin}
        COMMAND $<TARGET_FILE:rxlink> -s -o "${_crexx_rxgraph_root}/graph_reader-linked"
                ${_crexx_rxgraph_reader_rxbin}
                ${_crexx_rxgraph_grammar_rxbin}
                "${CMAKE_BINARY_DIR}/bin/library.rxbin"
                "${CMAKE_BINARY_DIR}/bin/rxfnsl.rxbin"
        DEPENDS rxlink ${_crexx_rxgraph_reader_rxbin}
                ${_crexx_rxgraph_grammar_rxbin}
                library rxfnsl
        VERBATIM)

add_custom_command(
        OUTPUT "${_crexx_rxgraph_root}/rxgraphbuild.c"
        COMMAND $<TARGET_FILE:rxcpack> -o "${_crexx_rxgraph_root}/rxgraphbuild"
                "${_crexx_rxgraph_root}/graph_reader-linked"
        DEPENDS rxcpack ${_crexx_rxgraph_linked_rxbin}
        VERBATIM)

add_executable(rxbuild "${_crexx_rxgraph_root}/rxgraphbuild.c")
configure_linker_for_static_lib(rxbuild fs)
configure_linker_for_static_lib(rxbuild _platform PROVIDER_ID rxplatform)
configure_linker_for_static_lib(rxbuild _hash)
target_link_libraries(rxbuild rxvml rxpashim)
add_dependencies(rxbuild fs fs_static _platform _platform_static _hash _hash_static)
if(TARGET crexx)
    add_dependencies(crexx rxbuild)
endif()
crexx_install_targets(rxbuild)

add_custom_target(rxbuild_graph_example
        COMMAND $<TARGET_FILE:rxbuild>
                "${CMAKE_CURRENT_SOURCE_DIR}/graph/example.graph.build"
        DEPENDS rxbuild
        VERBATIM)

if(BUILD_TESTING)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_test(NAME rxbuild_graph
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/graph/tests/check-graph-build.py
                --rxbuild $<TARGET_FILE:rxbuild>
                --source "${CMAKE_SOURCE_DIR}")
    set_tests_properties(rxbuild_graph PROPERTIES
        LABELS "tools;rxbuild;essential" TIMEOUT 180
        CREXX_PREP_TARGETS "rxbuild;crexx;${CREXX_DEFAULT_VM_TARGET}")
endif()
