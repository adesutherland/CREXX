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
    interpreter/rxvmintp.c interpreter/rxvmworker.c interpreter/rxvmprogram.c
    interpreter/rxvmactive.c interpreter/rxvmload.c interpreter/interrupt_single.c
    interpreter/rxenv.c interpreter/exitfunc.c interpreter/rxpacallmethod.c interpreter/rxpafuncs.c
    interpreter/rxvml/rxvm_run.c
    interpreter/rxvmref.c interpreter/rxpacompat.c interpreter/rxvmmemory.c
    interpreter/rxpashim.c interpreter/rxvmplugin/rxvmplugin_framework.c
    interpreter/rxvmplugin/rxvmplugins/mc_decimal/mc_decimal.c
    interpreter/rxvmplugin/rxvmplugins/mc_decimal/decnumber/decNumber.c
    interpreter/rxvmplugin/rxvmplugins/mc_decimal/decnumber/decContext.c
    platform/platform.c platform/rxinteger.c avl_tree/avl_tree.c rxpa/rxpa.c
    binutils/rxbin.c binutils/rxbin007.c binutils/rxsignature.c
    binutils/rxsha256.c binutils/rxgraph.c)
list(TRANSFORM vm_sources PREPEND "${CREXX_SOURCE}/")
add_library(rxvm_single_core STATIC ${vm_sources})
target_include_directories(rxvm_single_core PUBLIC
    "${CREXX_GENERATED_DIR}" "${CREXX_SOURCE}/interpreter"
    "${CREXX_SOURCE}/interpreter/rxvmplugin" "${CREXX_SOURCE}/assembler"
    "${CREXX_SOURCE}/binutils/include" "${CREXX_SOURCE}/rxpa"
    "${CREXX_SOURCE}/platform" "${CREXX_SOURCE}/avl_tree"
    "${CREXX_SOURCE}/utf8" "${CREXX_SOURCE}/inc"
    "${CREXX_SOURCE}/interpreter/rxvmplugin/rxvmplugins/mc_decimal/decnumber")
target_compile_definitions(rxvm_single_core PUBLIC CREXX_VM_SINGLE_THREADED=1
    NTHREADED=1 CREXX_VM_HANDLER_PANEL=20 MANUAL_PLUGIN_LINK=1
    CREXX_VM_STATIC_ONLY=1 CREXX_VM_NO_SOCKETS=1 CREXX_VM_NO_CLOCK=1
    CREXX_VM_PORTABLE_ALLOC=1 CREXX_VM_COMPACT=1
    RXVM_MEMORY_SLAB_SIZE=${CREXX_VM_SLAB_SIZE}
    RXVM_MEMORY_MAX_STANDARD_SIZE=${CREXX_VM_MAX_POOLED_SIZE})
add_executable(rxbvm_single "${CREXX_SOURCE}/interpreter/rxvmmain.c")
target_link_libraries(rxbvm_single PRIVATE rxvm_single_core ${CMAKE_DL_LIBS} m)

include(CTest)
if(BUILD_TESTING)
    add_executable(test_single_state test_state.c)
    target_link_libraries(test_single_state PRIVATE rxvm_single_core ${CMAKE_DL_LIBS} m)
    add_test(NAME single_vm_state COMMAND test_single_state)
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
