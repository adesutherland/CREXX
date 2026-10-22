cmake_minimum_required(VERSION 3.24)
project(CrexxSingleThreadVM C)

# Experimental, opt-in application-port build. The normal product build is
# unchanged. Use its generated version header and RXAS for test inputs.
set(CREXX_GENERATED_DIR "" CACHE PATH "Generated headers from a normal cREXX build")
if(NOT EXISTS "${CREXX_GENERATED_DIR}/crexx_version.h")
    message(FATAL_ERROR "Set CREXX_GENERATED_DIR to a cREXX build's generated directory")
endif()
get_filename_component(CREXX_SOURCE "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CREXX_VM_SLAB_SIZE 4096 CACHE STRING "Bytes per aligned VM allocator slab")
set(CREXX_VM_MAX_POOLED_SIZE 2048 CACHE STRING "Maximum pooled byte/value allocation")

set(vm_sources
    interpreter/rxvmintp.c interpreter/rxvmparse.c interpreter/rxvmparse_cursor.c
    interpreter/rxvmlinkindex.c interpreter/rxvmworker.c interpreter/rxvmprogram.c
    interpreter/rxvmactive.c interpreter/rxvmload.c interpreter/interrupt_single.c
    interpreter/rxenv.c interpreter/exitfunc.c interpreter/rxpacallmethod.c interpreter/rxpafuncs.c
    interpreter/rxvml/rxvm_run.c
    interpreter/rxvmref.c interpreter/rxpacompat.c interpreter/rxvmmemory.c
    interpreter/rxpashim.c interpreter/rxvmplugin/rxvmplugin_framework.c
    interpreter/rxvmplugin/rxvmplugins/mc_decimal/mc_decimal.c
    interpreter/rxvmplugin/rxvmplugins/mc_decimal/decnumber/decNumber.c
    interpreter/rxvmplugin/rxvmplugins/mc_decimal/decnumber/decContext.c
    platform/platform.c platform/platform_tso.c platform/platform_native.c platform/text_codec.c platform/oom.c platform/rxinteger.c third_party/avl_tree/avl_tree.c rxpa/rxpa.c
    binutils/rxbin.c binutils/rxbin007.c binutils/rxsignature.c binutils/rxcontract.c
    binutils/rxsha256.c binutils/rxgraph.c)
list(TRANSFORM vm_sources PREPEND "${CREXX_SOURCE}/")
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/vm_sources.txt"
    CONTENT "$<JOIN:${vm_sources},\n>\n")
add_library(rxvm_single_core STATIC ${vm_sources})
target_include_directories(rxvm_single_core PUBLIC
    "${CREXX_GENERATED_DIR}" "${CREXX_SOURCE}/interpreter"
    "${CREXX_SOURCE}/interpreter/rxvmplugin" "${CREXX_SOURCE}/assembler"
    "${CREXX_SOURCE}/binutils/include" "${CREXX_SOURCE}/rxpa"
    "${CREXX_SOURCE}/platform" "${CREXX_SOURCE}/third_party/avl_tree"
    "${CREXX_SOURCE}/third_party/utf8" "${CREXX_SOURCE}/interpreter/rxvml"
    "${CREXX_SOURCE}/interpreter/rxvmplugin/rxvmplugins/mc_decimal/decnumber")
target_compile_definitions(rxvm_single_core PUBLIC CREXX_VM_SINGLE_THREADED=1
    NTHREADED=1 CREXX_VM_HANDLER_PANEL=20 MANUAL_PLUGIN_LINK=1
    CREXX_VM_STATIC_ONLY=1 CREXX_VM_NO_SOCKETS=1 CREXX_VM_NO_CLOCK=1
    CREXX_VM_PORTABLE_ALLOC=1 CREXX_VM_COMPACT=1
    RXVM_MEMORY_SLAB_SIZE=${CREXX_VM_SLAB_SIZE}
    RXVM_MEMORY_MAX_STANDARD_SIZE=${CREXX_VM_MAX_POOLED_SIZE})
option(CREXX_MAINFRAME_ADDRESS "Link the selected CMS31/TSO31 native ADDRESS provider" OFF)
option(CREXX_VM_CLASSIC_PROVIDERS "Link portable Classic stream and native logical-file bindings" OFF)
if(CREXX_VM_CLASSIC_PROVIDERS)
    target_sources(rxvm_single_core PRIVATE
        "${CREXX_SOURCE}/lib/plugins/cstream/rxcstream.c"
        "${CMAKE_CURRENT_LIST_DIR}/mainframe_fs.c")
    target_compile_definitions(rxvm_single_core PUBLIC CREXX_VM_CLASSIC_PROVIDERS=1)
    set_source_files_properties("${CREXX_SOURCE}/lib/plugins/cstream/rxcstream.c"
        PROPERTIES COMPILE_DEFINITIONS "PLUGIN_ID=rxcstream")
    set_source_files_properties("${CMAKE_CURRENT_LIST_DIR}/mainframe_fs.c"
        PROPERTIES COMPILE_DEFINITIONS "PLUGIN_ID=rxfs")
endif()
set(CREXX_COMMAND_INCLUDE "" CACHE PATH "Verified selected SDK include directory with command.h")
set(CREXX_COMMAND_LIBRARIES "" CACHE STRING "Selected SDK command adapter libraries/objects")
option(CREXX_BUILD_COMPILER "Build RXC against the constrained embedded VM" OFF)
if(CREXX_MAINFRAME_ADDRESS)
    if(CREXX_BUILD_COMPILER)
        message(FATAL_ERROR "Native RXC needs declaration metadata separate from the runtime provider; use mainframe-build.crexx")
    endif()
    if(NOT EXISTS "${CREXX_COMMAND_INCLUDE}/command.h")
        message(FATAL_ERROR "Native ADDRESS requires the current selected SDK command.h")
    endif()
    target_sources(rxvm_single_core PRIVATE
        "${CREXX_SOURCE}/platform/mainframe_command.c"
        "${CREXX_SOURCE}/platform/mainframe_command_cms.c"
        "${CREXX_SOURCE}/platform/mainframe_command_tso.c"
        "${CREXX_SOURCE}/lib/plugins/mainframe_address/mainframe_address.c")
    target_include_directories(rxvm_single_core PRIVATE "${CREXX_COMMAND_INCLUDE}")
    target_compile_definitions(rxvm_single_core PUBLIC CREXX_MAINFRAME_ADDRESS=1)
    set_source_files_properties("${CREXX_SOURCE}/lib/plugins/mainframe_address/mainframe_address.c"
        PROPERTIES COMPILE_DEFINITIONS "PLUGIN_ID=rxmainframe_address")
    target_link_libraries(rxvm_single_core PUBLIC ${CREXX_COMMAND_LIBRARIES})
endif()
# RXC calls the public embedding API; this does not include a VM CLI main.
add_library(rxvm_single_embed STATIC "${CREXX_SOURCE}/interpreter/rxvml.c")
target_link_libraries(rxvm_single_embed PUBLIC rxvm_single_core)
if(CREXX_BUILD_COMPILER)
    include("${CMAKE_CURRENT_LIST_DIR}/compiler.cmake")
endif()

add_executable(rxbvm_single "${CREXX_SOURCE}/interpreter/rxvmmain.c")
target_link_libraries(rxbvm_single PRIVATE rxvm_single_core ${CMAKE_DL_LIBS} m)

include(CTest)
set(CREXX_PARSE_CURSOR_FIXTURE "" CACHE FILEPATH "Core-only native PARSE cursor smoke bytecode")
if(BUILD_TESTING AND CREXX_PARSE_CURSOR_FIXTURE)
    add_test(NAME single_vm_parse_cursor COMMAND rxbvm_single --no-autoload "${CREXX_PARSE_CURSOR_FIXTURE}")
    set_tests_properties(single_vm_parse_cursor PROPERTIES
        PASS_REGULAR_EXPRESSION "PARSE_CURSOR_CORE_PASS" TIMEOUT 90)
endif()
set(CREXX_CLASSIC_PROVIDER_FIXTURE "" CACHE FILEPATH "Linked canonical Classic/G provider smoke bytecode")
if(BUILD_TESTING AND CREXX_VM_CLASSIC_PROVIDERS AND CREXX_CLASSIC_PROVIDER_FIXTURE)
    add_test(NAME single_vm_classic_providers COMMAND rxbvm_single ${CREXX_CLASSIC_PROVIDER_FIXTURE})
    set_tests_properties(single_vm_classic_providers PROPERTIES
        PASS_REGULAR_EXPRESSION "NATIVE CLASSIC PROVIDERS PASS")
endif()
if(BUILD_TESTING)
    add_executable(test_single_state test_state.c)
    target_link_libraries(test_single_state PRIVATE rxvm_single_core ${CMAKE_DL_LIBS} m)
    add_test(NAME single_vm_state COMMAND test_single_state)
    add_executable(test_single_embed test_embed.c)
    target_link_libraries(test_single_embed PRIVATE rxvm_single_embed ${CMAKE_DL_LIBS} m)
    set(CREXX_EMBEDDING_FIXTURE "" CACHE FILEPATH "Compiled embedding.crexx bytecode")
    if(CREXX_EMBEDDING_FIXTURE)
        add_test(NAME single_vm_embedding COMMAND test_single_embed "${CREXX_EMBEDDING_FIXTURE}")
    endif()
endif()
set(CREXX_REFERENCE_VM "" CACHE FILEPATH "Normal switch VM for comparison")
set(CREXX_RXAS "" CACHE FILEPATH "Native RXAS for test fixtures")
if(BUILD_TESTING AND CREXX_REFERENCE_VM AND CREXX_RXAS)
    add_test(NAME single_vm_parity COMMAND "${CMAKE_COMMAND}"
        "-DVM=$<TARGET_FILE:rxbvm_single>" "-DREFERENCE=${CREXX_REFERENCE_VM}"
        "-DRXAS=${CREXX_RXAS}" "-DSOURCE=${CREXX_SOURCE}"
        "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/tests" -P "${CMAKE_CURRENT_SOURCE_DIR}/TestVM.cmake")
    set_tests_properties(single_vm_parity PROPERTIES RUN_SERIAL TRUE TIMEOUT 180)
endif()

# This explicit target also allows isolated Debug/sanitizer timing before a
# larger compiler test is registered in a broad CTest schedule.
set(CREXX_REFERENCE_COMPILER "" CACHE FILEPATH "Normal pinned compiler for parity")
if(BUILD_TESTING AND CREXX_BUILD_COMPILER AND CREXX_REFERENCE_COMPILER)
    add_custom_target(check_single_rxc
        COMMAND "${CMAKE_COMMAND}" "-DCOMPILER=$<TARGET_FILE:rxc_single>"
        "-DREFERENCE=${CREXX_REFERENCE_COMPILER}" "-DRXAS=${CREXX_HOST_BUILD}/bin/rxas"
        "-DVM=$<TARGET_FILE:rxbvm_single>" "-DLIBDIR=${CREXX_HOST_BUILD}/bin"
        "-DSOURCE=${CREXX_SOURCE}" "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/compiler-tests"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/TestCompiler.cmake"
        DEPENDS rxc_single rxbvm_single VERBATIM)
endif()
