cmake_minimum_required(VERSION 3.24)

project(Interpreter C)

include(${CMAKE_SOURCE_DIR}/cmake/CrexxLinkedRuntime.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CrexxConcurrencyTests.cmake)
find_package(Threads REQUIRED)

set(CMAKE_C_STANDARD 11) # required by the concurrency atomics
set(CMAKE_C_STANDARD_REQUIRED ON)
if(MSVC)
    # Visual Studio exposes its C11 atomic implementation behind this switch.
    add_compile_options($<$<COMPILE_LANGUAGE:C>:/experimental:c11atomics>)
    set(CMAKE_C_FLAGS_RELEASE "/O2 /Ob2 /DNDEBUG")
else()
    set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG") # Can do more I am sure but this is about as good as it get in practice
endif()
option(CREXX_VM_PROFILING
        "Compile timing and dynamic sequence profiling into the VM (runtime default remains off)"
        OFF)
set(CREXX_VM_HANDLER_PANEL "profile-20" CACHE STRING
        "VM instruction handler placement panel")
set_property(CACHE CREXX_VM_HANDLER_PANEL PROPERTY STRINGS
        all-inline all-outline profile-5 profile-10 profile-15 profile-20
        profile-30 max-eligible)
if(CREXX_VM_HANDLER_PANEL STREQUAL "all-inline")
    set(CREXX_VM_HANDLER_PANEL_VALUE 0)
elseif(CREXX_VM_HANDLER_PANEL STREQUAL "all-outline")
    set(CREXX_VM_HANDLER_PANEL_VALUE 1)
elseif(CREXX_VM_HANDLER_PANEL STREQUAL "profile-5")
    set(CREXX_VM_HANDLER_PANEL_VALUE 5)
elseif(CREXX_VM_HANDLER_PANEL STREQUAL "profile-10")
    set(CREXX_VM_HANDLER_PANEL_VALUE 10)
elseif(CREXX_VM_HANDLER_PANEL STREQUAL "profile-15")
    set(CREXX_VM_HANDLER_PANEL_VALUE 15)
elseif(CREXX_VM_HANDLER_PANEL STREQUAL "profile-20")
    set(CREXX_VM_HANDLER_PANEL_VALUE 20)
elseif(CREXX_VM_HANDLER_PANEL STREQUAL "profile-30")
    set(CREXX_VM_HANDLER_PANEL_VALUE 30)
elseif(CREXX_VM_HANDLER_PANEL STREQUAL "max-eligible")
    set(CREXX_VM_HANDLER_PANEL_VALUE 90)
else()
    message(FATAL_ERROR
            "CREXX_VM_HANDLER_PANEL must be all-inline, all-outline, profile-5, profile-10, profile-15, profile-20, profile-30, or max-eligible")
endif()
message(STATUS "cREXX VM handler panel: ${CREXX_VM_HANDLER_PANEL}")

if(CREXX_DEFAULT_VM_TARGET STREQUAL "rxtvm" AND NOT CREXX_THREADED_VM_SUPPORTED)
    message(FATAL_ERROR
            "The selected rxtvm default requires GNU/Clang labels-as-values support")
endif()

set(_rxvmintp_sanitizer_flags
        "${CMAKE_C_FLAGS} ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS}")
if(CMAKE_BUILD_TYPE)
    string(TOUPPER "${CMAKE_BUILD_TYPE}" _rxvmintp_build_type)
    string(APPEND _rxvmintp_sanitizer_flags
            " ${CMAKE_C_FLAGS_${_rxvmintp_build_type}}"
            " ${CMAKE_EXE_LINKER_FLAGS_${_rxvmintp_build_type}}"
            " ${CMAKE_SHARED_LINKER_FLAGS_${_rxvmintp_build_type}}")
endif()
string(FIND "${_rxvmintp_sanitizer_flags}" "-fsanitize=address" _rxvmintp_asan_flag)
if(NOT _rxvmintp_asan_flag EQUAL -1
        AND (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo"))
    # GCC 11 spends tens of minutes compiling this translation unit at -O2 with ASan.
    set_source_files_properties(rxvmintp.c PROPERTIES COMPILE_OPTIONS "-O1")
endif()

set(RXVM_CORE_OBJECT_SOURCES
        rxvmintp.h
        rxvmintp.c
        rxvmhandlerpolicy.h
        rxvmhandlers_core.inc
        rxvmhandlers_control.inc
        rxvmhandlers_numeric.inc
        rxvmhandlers_string.inc
        rxvmhandlers_system.inc
        rxvmhandlers_channel.inc
        rxvmworker.h
        rxvmworker.c
        rxvmprogram.h
        rxvmprogram.c
        rxvmexecutor.h
        rxvmexecutor.c
        rxvmchannel.h
        rxvmchannel_internal.h
        rxvmchannel.c
        rxvmchannel_process.h
        rxvmchannel_process.c
        rxvmprocessprotocol.h
        rxvmprocessprotocol.c
        rxvmprocessworker.h
        rxvmprocessworker.c
        rxvmbyteendpoint.c
        rxvmchannel_byte.c
        rxvmchannel_child.c
        rxvmactive.c
        rxvmstem.h
        rxvmload.c
        interrupt.c
        ../avl_tree/rxastree.h
        rxsignal.h
        ../utf8/utf.h
        ../binutils/include/rxbin.h
        ../binutils/include/rxsignature.h
        rxspawn.c
        rxcrexxcmd.c
        rxcrexxcmd.h
        exitfunc.c
        rxpafuncs.c
        rxvml/rxvm_run.c)
if(CREXX_VM_PROFILING)
    list(APPEND RXVM_CORE_OBJECT_SOURCES
            ../binutils/include/opdata.c
            ../binutils/rxopmeta.c
            rxvmprofile.h
            rxvmprofile.c
            rxvminstrument_profile.h
            rxseqfile.h
            rxseqfile.c
            rxvmsequence.h
            rxvmsequence.c)
endif()

function(crexx_add_rxvm_core_objects target_name)
    add_library(${target_name} OBJECT ${RXVM_CORE_OBJECT_SOURCES})
    crexx_apply_vm_compile_resource_pool(${target_name})
    target_compile_definitions(${target_name} PRIVATE
            CREXX_VM_HANDLER_PANEL=${CREXX_VM_HANDLER_PANEL_VALUE})
    if(CREXX_VM_PROFILING)
        target_compile_definitions(${target_name} PRIVATE
                CREXX_VM_MEMORY_CENSUS=1)
    endif()
    target_include_directories(${target_name} PRIVATE
            .
            ./rxvmplugin
            ../assembler
            ../binutils/include
            ${CMAKE_SOURCE_DIR}/rxpa
            ../platform
            ../avl_tree
            ../utf8
            ../inc
            rxvmplugin/rxvmplugins/mc_decimal/decnumber)
    # These objects are also archived into rxvml, which is linked into
    # libcrexxsaa.so on ELF platforms.
    set_target_properties(${target_name} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endfunction()

set(RXVM_PLATFORM_OBJECT_SOURCES
        rxvmsock.c
        rxvmsock.h)

function(crexx_add_rxvm_platform_objects target_name)
    add_library(${target_name} OBJECT ${RXVM_PLATFORM_OBJECT_SOURCES})
    target_include_directories(${target_name} PRIVATE
            .
            ./rxvmplugin
            ../assembler
            ../binutils/include
            ${CMAKE_SOURCE_DIR}/rxpa
            ../platform
            ../avl_tree
            ../utf8
            ../inc
            rxvmplugin/rxvmplugins/mc_decimal/decnumber)
    # These objects are also archived into rxvml, which is linked into
    # libcrexxsaa.so on ELF platforms.
    set_target_properties(${target_name} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endfunction()

crexx_add_rxvm_core_objects(rxbvm_core_objects)
target_compile_definitions(rxbvm_core_objects PRIVATE NTHREADED=1)
if(CREXX_THREADED_VM_SUPPORTED)
    crexx_add_rxvm_core_objects(rxtvm_core_objects)
endif()
if(CREXX_VM_PROFILING)
    target_compile_definitions(rxbvm_core_objects PRIVATE
            CREXX_VM_PROFILING=1
            RXVM_INSTRUMENTATION_BACKEND="rxvminstrument_profile.h")
    if(CREXX_THREADED_VM_SUPPORTED)
        target_compile_definitions(rxtvm_core_objects PRIVATE
                CREXX_VM_PROFILING=1
                RXVM_INSTRUMENTATION_BACKEND="rxvminstrument_profile.h")
    endif()
endif()
if(BUILD_TESTING)
    crexx_add_rxvm_core_objects(rxbvm_instrumented_core_objects)
    target_compile_definitions(rxbvm_instrumented_core_objects PRIVATE
            NTHREADED=1
            RXVM_INSTRUMENTATION_BACKEND="rxvminstrument_test.h")
    if(CREXX_THREADED_VM_SUPPORTED)
        crexx_add_rxvm_core_objects(rxtvm_instrumented_core_objects)
        target_compile_definitions(rxtvm_instrumented_core_objects PRIVATE
                RXVM_INSTRUMENTATION_BACKEND="rxvminstrument_test.h")
    endif()
endif()
if(CREXX_DEFAULT_VM_TARGET STREQUAL "rxtvm")
    set(CREXX_DEFAULT_VM_CORE_OBJECT_TARGET rxtvm_core_objects)
    set(CREXX_DEFAULT_VM_USES_SWITCH OFF)
else()
    set(CREXX_DEFAULT_VM_CORE_OBJECT_TARGET rxbvm_core_objects)
    set(CREXX_DEFAULT_VM_USES_SWITCH ON)
endif()
set(CREXX_DEFAULT_VM_CORE_OBJECT_TARGET "${CREXX_DEFAULT_VM_CORE_OBJECT_TARGET}"
        CACHE INTERNAL "Object target used by compiler-selected VM consumers" FORCE)
set(CREXX_DEFAULT_VM_USES_SWITCH "${CREXX_DEFAULT_VM_USES_SWITCH}"
        CACHE INTERNAL "Whether the compiler-selected VM uses switch dispatch" FORCE)
crexx_add_rxvm_platform_objects(rxvm_platform_notls_objects)

add_library(rxvmref STATIC rxvmref.c rxvmref.h rxpacompat.c rxpacompat.h)
target_include_directories(rxvmref PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(rxvmmemory STATIC rxvmmemory.c rxvmmemory.h)
target_include_directories(rxvmmemory PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(rxvmmemory PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(CREXX_VM_PROFILING)
    # Keep the detailed allocator census out of the ordinary product.  The
    # public definition keeps the diagnostic stats structure consistent for
    # direct consumers of rxvmmemory in this profiling-only build.
    target_compile_definitions(rxvmmemory PUBLIC
            CREXX_VM_MEMORY_CENSUS=1)
endif()
target_link_libraries(rxvmref PUBLIC rxvmmemory Threads::Threads)

add_subdirectory(rxvmplugin)

set(CREXX_SOCKET_TARGET_LIBS)
set(CREXX_SOCKET_MANUAL_LINK_LIBS)
set(CREXX_NATIVE_RUNTIME_LINK_FLAGS)
if(WIN32)
    # advapi32 supplies the OS CSPRNG used for runtime-scoped channel
    # capability identities; every VM embedding must retain that dependency.
    set(CREXX_SOCKET_TARGET_LIBS ws2_32 advapi32)
    set(CREXX_SOCKET_MANUAL_LINK_LIBS -lws2_32 -ladvapi32)
    if(MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
        list(APPEND CREXX_NATIVE_RUNTIME_LINK_FLAGS advapi32.lib)
    else()
        list(APPEND CREXX_NATIVE_RUNTIME_LINK_FLAGS -ladvapi32)
    endif()
endif()

if(WIN32)
    set(CREXX_DEFAULT_TLS_BACKEND SCHANNEL)
elseif(APPLE)
    set(CREXX_DEFAULT_TLS_BACKEND NETWORK)
else()
    set(CREXX_DEFAULT_TLS_BACKEND OPENSSL)
endif()

set(CREXX_ENABLE_TLS "${CREXX_DEFAULT_TLS_BACKEND}" CACHE STRING "TLS backend for VM core sockets: OFF, OPENSSL, NETWORK, or SCHANNEL")
set_property(CACHE CREXX_ENABLE_TLS PROPERTY STRINGS OFF OPENSSL NETWORK SCHANNEL)
option(CREXX_TLS_STATIC_OPENSSL "Prefer static OpenSSL libraries for the VM core socket TLS backend" OFF)
string(TOUPPER "${CREXX_ENABLE_TLS}" CREXX_ENABLE_TLS_NORMALIZED)
if(CREXX_ENABLE_TLS_NORMALIZED STREQUAL "ON")
    set(CREXX_ENABLE_TLS_NORMALIZED "OPENSSL")
endif()

if(CREXX_ENABLE_TLS_NORMALIZED STREQUAL "OPENSSL")
    set(OPENSSL_USE_STATIC_LIBS ${CREXX_TLS_STATIC_OPENSSL})
    find_package(OpenSSL REQUIRED)
    list(APPEND CREXX_SOCKET_TARGET_LIBS OpenSSL::SSL OpenSSL::Crypto)
    list(APPEND CREXX_SOCKET_MANUAL_LINK_LIBS ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
    list(APPEND CREXX_NATIVE_RUNTIME_LINK_FLAGS -lssl -lcrypto)
elseif(CREXX_ENABLE_TLS_NORMALIZED STREQUAL "NETWORK")
    if(NOT APPLE)
        message(FATAL_ERROR "CREXX_ENABLE_TLS=NETWORK is only supported on Apple platforms.")
    endif()
    find_library(CREXX_NETWORK_FRAMEWORK Network REQUIRED)
    find_library(CREXX_SECURITY_FRAMEWORK Security REQUIRED)
    find_library(CREXX_COREFOUNDATION_FRAMEWORK CoreFoundation REQUIRED)
    list(APPEND CREXX_SOCKET_TARGET_LIBS
            ${CREXX_NETWORK_FRAMEWORK}
            ${CREXX_SECURITY_FRAMEWORK}
            ${CREXX_COREFOUNDATION_FRAMEWORK})
    list(APPEND CREXX_SOCKET_MANUAL_LINK_LIBS
            -framework Network
            -framework Security
            -framework CoreFoundation)
    list(APPEND CREXX_NATIVE_RUNTIME_LINK_FLAGS
            -framework Network
            -framework Security
            -framework CoreFoundation)
elseif(CREXX_ENABLE_TLS_NORMALIZED STREQUAL "SCHANNEL")
    if(NOT WIN32)
        message(FATAL_ERROR "CREXX_ENABLE_TLS=SCHANNEL is only supported on Windows.")
    endif()
    list(APPEND CREXX_SOCKET_TARGET_LIBS secur32 crypt32)
    list(APPEND CREXX_SOCKET_MANUAL_LINK_LIBS -lsecur32 -lcrypt32)
    if(MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
        list(APPEND CREXX_NATIVE_RUNTIME_LINK_FLAGS secur32.lib crypt32.lib)
    else()
        list(APPEND CREXX_NATIVE_RUNTIME_LINK_FLAGS -lsecur32 -lcrypt32)
    endif()
elseif(NOT CREXX_ENABLE_TLS_NORMALIZED STREQUAL "OFF")
    message(FATAL_ERROR "Unsupported CREXX_ENABLE_TLS value '${CREXX_ENABLE_TLS}'. Use OFF, OPENSSL, NETWORK, or SCHANNEL.")
endif()
string(JOIN " " CREXX_NATIVE_RUNTIME_LINK_FLAGS_TEXT ${CREXX_NATIVE_RUNTIME_LINK_FLAGS})
set(CREXX_NATIVE_RUNTIME_LINK_FLAGS_TEXT "${CREXX_NATIVE_RUNTIME_LINK_FLAGS_TEXT}"
        CACHE INTERNAL "Native executable link flags required by the configured VM runtime" FORCE)
set(CREXX_NATIVE_RUNTIME_LINK_FLAGS_ARGS "${CREXX_NATIVE_RUNTIME_LINK_FLAGS}"
        CACHE INTERNAL "Native executable link flags as an argv-safe CMake list" FORCE)
message(STATUS "cREXX VM core socket TLS backend: ${CREXX_ENABLE_TLS_NORMALIZED}")

crexx_add_rxvm_platform_objects(rxvm_platform_objects)
if(CREXX_VM_PROFILING)
    target_compile_definitions(rxvm_platform_objects PRIVATE CREXX_VM_PROFILING=1)
    target_compile_definitions(rxvm_platform_notls_objects PRIVATE CREXX_VM_PROFILING=1)
endif()
if(CREXX_ENABLE_TLS_NORMALIZED STREQUAL "OPENSSL")
    target_compile_definitions(rxvm_platform_objects PRIVATE CREXX_TLS_OPENSSL=1)
elseif(CREXX_ENABLE_TLS_NORMALIZED STREQUAL "NETWORK")
    target_compile_definitions(rxvm_platform_objects PRIVATE CREXX_TLS_NETWORK=1)
    target_compile_options(rxvm_platform_objects PRIVATE -fblocks)
elseif(CREXX_ENABLE_TLS_NORMALIZED STREQUAL "SCHANNEL")
    target_compile_definitions(rxvm_platform_objects PRIVATE CREXX_TLS_SCHANNEL=1)
endif()

##########################################
# Bare Interpreter
# Portable switch-dispatch version.
add_executable(rxbvm rxvmmain.c $<TARGET_OBJECTS:rxbvm_core_objects>)
target_link_libraries(rxbvm rxvmplugin mc_decimal_manual avl_tree rxpa rxpashim)
target_include_directories(rxbvm PRIVATE . ./rxvmplugin
        ../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8 ../inc)
target_compile_definitions(rxbvm PRIVATE NTHREADED=1)
set(CREXX_CONCRETE_VM_TARGETS rxbvm)

# Direct-threaded version.  MSVC and unknown compilers deliberately do not
# compile this GNU/Clang extension.
if(CREXX_THREADED_VM_SUPPORTED)
    add_executable(rxtvm rxvmmain.c $<TARGET_OBJECTS:rxtvm_core_objects>)
    target_link_libraries(rxtvm rxvmplugin mc_decimal_manual avl_tree rxpa rxpashim)
    target_include_directories(rxtvm PRIVATE . ./rxvmplugin
            ../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8 ../inc)
    list(APPEND CREXX_CONCRETE_VM_TARGETS rxtvm)
endif()

# Keep rxvm as the stable build and executable product path.  Recreate the
# compatibility entry point on every rxvm target build so removing an otherwise
# up-to-date symlink/copy cannot leave the product target incomplete.
if(WIN32)
    add_custom_target(rxvm ALL
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<TARGET_FILE:${CREXX_DEFAULT_VM_TARGET}>"
                    "$<TARGET_FILE_DIR:${CREXX_DEFAULT_VM_TARGET}>/rxvm${CMAKE_EXECUTABLE_SUFFIX}"
            DEPENDS ${CREXX_DEFAULT_VM_TARGET}
            COMMENT "Creating rxvm copy of ${CREXX_DEFAULT_VM_TARGET}"
            VERBATIM)
else()
    add_custom_target(rxvm ALL
            COMMAND ${CMAKE_COMMAND} -E rm -f
                    "$<TARGET_FILE_DIR:${CREXX_DEFAULT_VM_TARGET}>/rxvm${CMAKE_EXECUTABLE_SUFFIX}"
            COMMAND ${CMAKE_COMMAND} -E create_symlink
                    "$<TARGET_FILE_NAME:${CREXX_DEFAULT_VM_TARGET}>"
                    "$<TARGET_FILE_DIR:${CREXX_DEFAULT_VM_TARGET}>/rxvm${CMAKE_EXECUTABLE_SUFFIX}"
            DEPENDS ${CREXX_DEFAULT_VM_TARGET}
            COMMENT "Creating rxvm symlink to ${CREXX_DEFAULT_VM_TARGET}"
            VERBATIM)
endif()
message(STATUS
        "cREXX product VM: rxvm -> ${CREXX_DEFAULT_VM_TARGET} (${CMAKE_C_COMPILER_ID})")

# Offline second-stage analyser for dynamic instruction-sequence profiles.
add_executable(rxseq rxseqmain.c rxseqfile.c)
target_include_directories(rxseq PRIVATE . ../binutils/include ../platform ../inc)
target_link_libraries(rxseq PRIVATE rxbin)

if(BUILD_TESTING)
    add_executable(test_rxseqfile tests/test_rxseqfile.c rxseqfile.c)
    target_include_directories(test_rxseqfile PRIVATE
            . ../binutils/include ../platform ../inc)
    add_test(NAME rxseqfileformat COMMAND test_rxseqfile)
    set_tests_properties(rxseqfileformat PROPERTIES
            LABELS "interpreter;profiling;sequence")
endif()

if(BUILD_TESTING)
    add_executable(rxbvm_instrumented rxvmmain.c $<TARGET_OBJECTS:rxbvm_instrumented_core_objects>)
    target_link_libraries(rxbvm_instrumented rxvmplugin mc_decimal_manual avl_tree rxpa rxpashim)
    target_include_directories(rxbvm_instrumented PRIVATE . ./rxvmplugin
            ../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8 ../inc)
    target_compile_definitions(rxbvm_instrumented PRIVATE NTHREADED=1)

    if(CREXX_THREADED_VM_SUPPORTED)
        add_executable(rxtvm_instrumented rxvmmain.c $<TARGET_OBJECTS:rxtvm_instrumented_core_objects>)
        target_link_libraries(rxtvm_instrumented rxvmplugin mc_decimal_manual avl_tree rxpa rxpashim)
        target_include_directories(rxtvm_instrumented PRIVATE . ./rxvmplugin
                ../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8 ../inc)
    endif()

    if(CREXX_DEFAULT_VM_TARGET STREQUAL "rxtvm")
        add_executable(rxvm_instrumented ALIAS rxtvm_instrumented)
    else()
        add_executable(rxvm_instrumented ALIAS rxbvm_instrumented)
    endif()
endif()

if(BUILD_TESTING AND CREXX_VM_PROFILING)
    add_executable(test_rxvmprofile
            tests/test_rxvmprofile.c
            $<TARGET_OBJECTS:${CREXX_DEFAULT_VM_CORE_OBJECT_TARGET}>
            $<TARGET_OBJECTS:rxvm_platform_objects>)
    target_include_directories(test_rxvmprofile PRIVATE
            .
            ../assembler
            ../binutils/include
            ../platform
            ../avl_tree
            ${CMAKE_SOURCE_DIR}/rxpa)
    target_compile_definitions(test_rxvmprofile PRIVATE CREXX_VM_PROFILING=1)
    if(CREXX_DEFAULT_VM_USES_SWITCH)
        target_compile_definitions(test_rxvmprofile PRIVATE NTHREADED=1)
    endif()
    target_link_libraries(test_rxvmprofile PRIVATE
            rxvmplugin mc_decimal_manual avl_tree rxpa rxpashim rxvmref rxbin
            ${CREXX_SOCKET_TARGET_LIBS})
    add_test(NAME rxvmprofileaccounting COMMAND test_rxvmprofile)
    set_tests_properties(rxvmprofileaccounting PROPERTIES
            LABELS "interpreter;profiling")
endif()

##########################################
# Interpreter linked to CREXX Stdlib

# Packed Library
add_custom_command(
        OUTPUT library.c
        COMMAND $<TARGET_FILE:rxcpack> -o library ${CMAKE_BINARY_DIR}/bin/library
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS library rxcpack ${CMAKE_BINARY_DIR}/bin/library.rxbin
        COMMENT "C Packing cREXX RXFNSB Library ..."
)
add_custom_target(packed_library ALL DEPENDS library.c)

# Compiler-selected product version.
add_executable(rxvme rxvmmain.c ${CMAKE_CURRENT_BINARY_DIR}/library.c
        $<TARGET_OBJECTS:${CREXX_DEFAULT_VM_CORE_OBJECT_TARGET}>)
target_compile_definitions(rxvme PRIVATE LINK_CREXX_LIB=1)
if(CREXX_DEFAULT_VM_USES_SWITCH)
    target_compile_definitions(rxvme PRIVATE NTHREADED=1)
endif()
# set_property(TARGET rxvme PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set_property(TARGET rxvme PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
# set_property(TARGET rxvme PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
target_link_libraries(rxvme rxvmplugin mc_decimal_manual avl_tree rxpa rxpashim)
target_include_directories(rxvme PRIVATE . ./rxvmplugin
		../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8 ../inc)
add_dependencies(rxvme packed_library)

# Bytecode Version
add_executable(rxbvme rxvmmain.c ${CMAKE_CURRENT_BINARY_DIR}/library.c $<TARGET_OBJECTS:rxbvm_core_objects>)
target_compile_definitions(rxbvme PRIVATE NTHREADED=1 LINK_CREXX_LIB=1)
target_link_libraries(rxbvme rxvmplugin mc_decimal_manual avl_tree rxpa rxpashim)
target_include_directories(rxbvme PRIVATE . ./rxvmplugin
		../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8 ../inc)
add_dependencies(rxbvme packed_library)

##########################################
# C Library Interpreter -  to be linked to make a standalone CREXX exe

# Compiler-selected product library.
add_library(rxvml STATIC rxvmlib.c rxvml.c
        $<TARGET_OBJECTS:${CREXX_DEFAULT_VM_CORE_OBJECT_TARGET}>)
if(CREXX_DEFAULT_VM_USES_SWITCH)
    target_compile_definitions(rxvml PRIVATE NTHREADED=1)
endif()
# set_property(TARGET rxvml PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# set_property(TARGET rxvml PROPERTY INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
# set_property(TARGET rxvml PROPERTY INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL TRUE)
target_link_libraries(rxvml rxvmplugin mc_decimal_manual avl_tree rxpa)
target_include_directories(rxvml PRIVATE . ./rxvmplugin
		../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8 ../inc)

# Bytecode Version
add_library(rxbvml STATIC rxvmlib.c rxvml.c $<TARGET_OBJECTS:rxbvm_core_objects>)
target_link_libraries(rxbvml rxvmplugin mc_decimal_manual avl_tree rxpa)
target_include_directories(rxbvml PRIVATE . ./rxvmplugin
		../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa ../platform ../avl_tree ../utf8 ../inc)
target_compile_definitions(rxbvml PRIVATE NTHREADED=1)

function(crexx_enable_vm_platform target_name)
    target_sources(${target_name} PRIVATE $<TARGET_OBJECTS:rxvm_platform_objects>)
    if(CREXX_SOCKET_TARGET_LIBS)
        target_link_libraries(${target_name} ${CREXX_SOCKET_TARGET_LIBS})
    endif()
endfunction()

set(_crexx_vm_core_targets rxbvm rxvme rxbvme rxvml rxbvml)
if(CREXX_THREADED_VM_SUPPORTED)
    list(APPEND _crexx_vm_core_targets rxtvm)
endif()
foreach(_rxvm_core_target IN LISTS _crexx_vm_core_targets)
    target_link_libraries(${_rxvm_core_target} rxvmref rxbin)
    crexx_enable_vm_platform(${_rxvm_core_target})
    if(CREXX_VM_PROFILING)
        target_compile_definitions(${_rxvm_core_target} PRIVATE CREXX_VM_PROFILING=1)
    endif()
endforeach()
if(BUILD_TESTING)
    add_executable(test_rxcrexxcmd_flush tests/test_rxcrexxcmd_flush.c)
    target_include_directories(test_rxcrexxcmd_flush PRIVATE
            . ./rxvmplugin ../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa
            ../platform ../avl_tree ../utf8 ../inc)
    target_link_libraries(test_rxcrexxcmd_flush PRIVATE rxvml)
    add_test(NAME crexx_address_unredirected_flush COMMAND test_rxcrexxcmd_flush)
    set_tests_properties(crexx_address_unredirected_flush PROPERTIES
            LABELS "interpreter;address;runtime")
endif()
if(BUILD_TESTING)
    set(_crexx_vm_instrumented_targets rxbvm_instrumented)
    if(CREXX_THREADED_VM_SUPPORTED)
        list(APPEND _crexx_vm_instrumented_targets rxtvm_instrumented)
    endif()
    foreach(_rxvm_instrumented_target IN LISTS _crexx_vm_instrumented_targets)
        target_link_libraries(${_rxvm_instrumented_target} rxvmref rxbin)
        crexx_enable_vm_platform(${_rxvm_instrumented_target})
    endforeach()
endif()

# libcrexxsaa is intentionally thin and shared. On ELF platforms, every static
# archive pulled into it must be compiled as PIC.
foreach(_crexxsaa_static_dep rxvml rxvmref rxvmplugin mc_decimal_manual avl_tree rxpa rxbin platform decnumber)
    if(TARGET ${_crexxsaa_static_dep})
        set_target_properties(${_crexxsaa_static_dep} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    endif()
endforeach()

add_library(crexxsaa SHARED crexxsaa.c crexxsaa.h)
target_compile_definitions(crexxsaa PRIVATE CREXXSAA_BUILDING_LIBRARY)
target_link_libraries(crexxsaa PUBLIC rxvml rxvmplugin mc_decimal_manual avl_tree rxpa)
target_include_directories(crexxsaa PUBLIC
        .
        ./rxvmplugin
        ../assembler
        ../binutils/include
        ${CMAKE_SOURCE_DIR}/rxpa
        ../platform
        ../avl_tree
        ../utf8
        ../inc)
set_target_properties(crexxsaa PROPERTIES
        OUTPUT_NAME crexxsaa
        POSITION_INDEPENDENT_CODE ON)
install(TARGETS crexxsaa
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION bin
        ARCHIVE DESTINATION lib)
install(FILES crexxsaa.h DESTINATION include)

add_executable(crexxsaa_tool crexxsaa_tool.c)
target_link_libraries(crexxsaa_tool PRIVATE crexxsaa)
target_include_directories(crexxsaa_tool PRIVATE .)
set_target_properties(crexxsaa_tool PROPERTIES
        OUTPUT_NAME crexxsaa
        ARCHIVE_OUTPUT_NAME crexxsaa_tool)
if(APPLE)
    set_target_properties(crexxsaa_tool PROPERTIES
            BUILD_RPATH "${CMAKE_BINARY_DIR}/bin"
            INSTALL_RPATH "@loader_path")
elseif(UNIX)
    set_target_properties(crexxsaa_tool PROPERTIES
            BUILD_RPATH "${CMAKE_BINARY_DIR}/bin"
            INSTALL_RPATH "$ORIGIN")
endif()
install(TARGETS crexxsaa_tool RUNTIME DESTINATION bin)

# RXPA Shim Library - Provides standard rxpa_ symbols for standalone targets
add_library(rxpashim STATIC rxpashim.c)
target_include_directories(rxpashim PUBLIC . ${CMAKE_SOURCE_DIR}/rxpa)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
include(${CMAKE_SOURCE_DIR}/cmake/CrexxSmokeTests.cmake)

# Value test executable
add_executable(ts_regvalue_tester tests/ts_regvalue_tester.c rxvalue.h rxvmvars.h)
target_include_directories(ts_regvalue_tester PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} . ../utf8 ../platform ../binutils/include)
target_link_libraries(ts_regvalue_tester rxvmref platform ${CREXX_MATH_LIBRARY})

add_executable(test_rxvmstem tests/test_rxvmstem.c rxvalue.h rxvmvars.h rxvmstem.h)
target_include_directories(test_rxvmstem PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} . ../utf8 ../platform ../binutils/include)
target_link_libraries(test_rxvmstem rxvmref platform ${CREXX_MATH_LIBRARY})

add_executable(test_rxvmstem_allocator_family
        tests/test_rxvmstem_allocator_family.c
        rxvalue.h rxvmvars.h rxvmstem.h)
target_include_directories(test_rxvmstem_allocator_family PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR} . ../utf8 ../platform ../binutils/include)
target_link_libraries(test_rxvmstem_allocator_family
        rxvmref platform ${CREXX_MATH_LIBRARY})

add_executable(test_rxvmmemory tests/test_rxvmmemory.c)
target_include_directories(test_rxvmmemory PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_rxvmmemory PRIVATE rxvmmemory Threads::Threads)

add_executable(test_rxvmworker tests/test_rxvmworker.c rxvmworker.c)
target_include_directories(test_rxvmworker PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_rxvmworker PRIVATE rxvmmemory Threads::Threads)

add_executable(test_rxvmchannel_registry
        tests/test_rxvmchannel_registry.c)
target_include_directories(test_rxvmchannel_registry PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../inc ../assembler ../binutils/include ../platform ../avl_tree
        ../utf8 ${CMAKE_SOURCE_DIR}/rxpa)
target_link_libraries(test_rxvmchannel_registry PRIVATE rxvml Threads::Threads)

add_executable(test_rxvmchannel_process
        tests/test_rxvmchannel_process.c)
target_include_directories(test_rxvmchannel_process PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../inc ../assembler ../binutils/include ../platform ../avl_tree
        ../utf8 ${CMAKE_SOURCE_DIR}/rxpa)
target_link_libraries(test_rxvmchannel_process PRIVATE rxvml Threads::Threads)

add_executable(test_rxvmbyteendpoint tests/test_rxvmbyteendpoint.c)
target_include_directories(test_rxvmbyteendpoint PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../inc ../assembler ../binutils/include ../platform ../avl_tree
        ../utf8 ${CMAKE_SOURCE_DIR}/rxpa)
target_link_libraries(test_rxvmbyteendpoint PRIVATE rxvml Threads::Threads)

add_executable(test_rxvmchannel_byte tests/test_rxvmchannel_byte.c)
target_include_directories(test_rxvmchannel_byte PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ../inc ../assembler ../binutils/include ../platform ../avl_tree
        ../utf8 ${CMAKE_SOURCE_DIR}/rxpa)
target_link_libraries(test_rxvmchannel_byte PRIVATE rxvml Threads::Threads)

add_executable(test_rxvmactive tests/test_rxvmactive.c)
target_include_directories(test_rxvmactive PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ./rxvmplugin ../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa
        ../platform ../avl_tree ../utf8 ../inc)
target_link_libraries(test_rxvmactive PRIVATE rxvml Threads::Threads)

add_executable(test_rxvmplugin_context tests/test_rxvmplugin_context.c)
target_include_directories(test_rxvmplugin_context PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ./rxvmplugin ../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa
        ../platform ../avl_tree ../utf8 ../inc)
target_link_libraries(test_rxvmplugin_context PRIVATE
        rxvml rxvmplugin mc_decimal_manual Threads::Threads)
add_dependencies(test_rxvmplugin_context mc_decimal_dynamic)

add_executable(test_rxpa_concurrency tests/test_rxpa_concurrency.c)
target_include_directories(test_rxpa_concurrency PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ./rxvmplugin ../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa
        ../platform ../avl_tree ../utf8 ../inc)
target_link_libraries(test_rxpa_concurrency rxvml Threads::Threads)

add_executable(test_rxpa_odbc tests/test_rxpa_odbc.c)
target_include_directories(test_rxpa_odbc PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ./rxvmplugin ../assembler ../binutils/include ${CMAKE_SOURCE_DIR}/rxpa
        ../platform ../avl_tree ../utf8 ../inc)
target_link_libraries(test_rxpa_odbc rxvml Threads::Threads)

add_executable(test_rxpa_odbc_old_host tests/test_rxpa_odbc_old_host.c)
target_include_directories(test_rxpa_odbc_old_host PRIVATE
        ${CMAKE_SOURCE_DIR}/rxpa ../platform)
target_link_libraries(test_rxpa_odbc_old_host rxpa Threads::Threads)

add_executable(test_jtable_policy tests/test_jtable_policy.c)
target_include_directories(test_jtable_policy PRIVATE ../binutils/include)

add_executable(test_signal_mask tests/test_signal_mask.c rxsignal.h)
target_include_directories(test_signal_mask PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(mutate_jtable_rxbin tests/mutate_jtable_rxbin.c)
target_link_libraries(mutate_jtable_rxbin PRIVATE rxbin)
target_include_directories(mutate_jtable_rxbin PRIVATE
        ../binutils/include
        ../platform)

add_executable(mutate_retired_process_opcode
        tests/mutate_retired_process_opcode.c)
target_link_libraries(mutate_retired_process_opcode PRIVATE rxbin)
target_include_directories(mutate_retired_process_opcode PRIVATE
        ../binutils/include
        ../platform)

add_executable(test_rxpa_utf_validation
        tests/test_rxpa_utf_validation.c
        rxpafuncs.c
        rxvmactive.c
        rxvmworker.c
        rxvalue.h
        rxvmvars.h)
target_include_directories(test_rxpa_utf_validation PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        .
        ../assembler
        ../binutils/include
        ${CMAKE_SOURCE_DIR}/rxpa
        ../platform
        ../avl_tree
        ../utf8
        ../inc)
target_link_libraries(test_rxpa_utf_validation rxvmref platform ${CREXX_MATH_LIBRARY})

# Enable testing
enable_testing()

add_test(
        NAME rxvm_product_entry_point
        COMMAND ${CMAKE_COMMAND}
                "-DRXVM=${CREXX_RXVM_EXECUTABLE}"
                "-DCONCRETE_VM=$<TARGET_FILE:${CREXX_DEFAULT_VM_TARGET}>"
                "-DCONCRETE_NAME=$<TARGET_FILE_NAME:${CREXX_DEFAULT_VM_TARGET}>"
                -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_rxvm_product.cmake)
set_tests_properties(rxvm_product_entry_point PROPERTIES
        LABELS "interpreter;build;dispatch")

add_test(NAME rxvmstem_storage COMMAND test_rxvmstem)
add_test(NAME rxvmstem_allocator_family COMMAND test_rxvmstem_allocator_family)
add_test(NAME rxvmmemory_allocator COMMAND test_rxvmmemory)
add_test(NAME rxvmworker_lifecycle COMMAND test_rxvmworker)
add_test(NAME rxvmchannel_provider_registry COMMAND test_rxvmchannel_registry)
set_tests_properties(rxvmchannel_provider_registry PROPERTIES
        LABELS "interpreter;perf3;gate-f;f1c;channels;provider-registry")
add_test(NAME rxvmbyteendpoint_substrate COMMAND test_rxvmbyteendpoint)
set_tests_properties(rxvmbyteendpoint_substrate PROPERTIES
        LABELS "interpreter;perf3;gate-f;f1d;channels;byte-endpoint;lifecycle")
add_test(NAME rxvmchannel_byte_provider COMMAND test_rxvmchannel_byte)
set_tests_properties(rxvmchannel_byte_provider PROPERTIES
        LABELS "interpreter;perf3;gate-f;f1d;channels;byte-endpoint;provider-reference")
add_test(NAME rxvmactive_isolation COMMAND test_rxvmactive)
add_test(NAME rxvmplugin_context_ownership
        COMMAND test_rxvmplugin_context
                "$<TARGET_FILE_DIR:mc_decimal_dynamic>"
                "rxvm_mc_decimal"
                "decnumber")
add_test(NAME rxpa_static_catalogue_replay
        COMMAND test_rxpa_concurrency static)
add_test(NAME rxpa_static_session_lifecycle
        COMMAND test_rxpa_concurrency session)
add_test(NAME rxpa_session_factory_failure_rollback
        COMMAND test_rxpa_concurrency session-failure)
add_test(NAME rxpa_session_manifest_fail_closed
        COMMAND test_rxpa_concurrency session-malformed)
add_test(NAME rxpa_float_static_alias_binding
        COMMAND test_rxpa_concurrency float-static)
add_test(NAME rxpa_legacy_call_serialization
        COMMAND test_rxpa_concurrency legacy)
add_test(NAME rxpa_process_reentrant_overlap
        COMMAND test_rxpa_concurrency reentrant)
add_test(NAME rxpa_legacy_recursive_reentry
        COMMAND test_rxpa_concurrency recursive)
add_test(NAME rxpa_branch_free_load_binding
        COMMAND test_rxpa_concurrency binding)
add_test(NAME rxpa_bound_legacy_serialization
        COMMAND test_rxpa_concurrency bound-legacy)
add_test(NAME rxpa_legacy_transition_quiescence
        COMMAND test_rxpa_concurrency transition)
set_tests_properties(rxvmmemory_allocator PROPERTIES
        LABELS "interpreter;memory")
set_tests_properties(rxvmworker_lifecycle PROPERTIES
        LABELS "interpreter;memory;worker")
set_tests_properties(rxvmactive_isolation PROPERTIES
        LABELS "interpreter;worker;signals")
set_tests_properties(rxvmplugin_context_ownership PROPERTIES
        LABELS "interpreter;worker;plugins;decimal")
set_tests_properties(
        rxpa_static_catalogue_replay
        rxpa_static_session_lifecycle
        rxpa_session_factory_failure_rollback
        rxpa_session_manifest_fail_closed
        rxpa_float_static_alias_binding
        rxpa_legacy_call_serialization
        rxpa_process_reentrant_overlap
        rxpa_legacy_recursive_reentry
        rxpa_branch_free_load_binding
        rxpa_bound_legacy_serialization
        rxpa_legacy_transition_quiescence
        PROPERTIES LABELS "interpreter;worker;plugins;rxpa")
set_tests_properties(rxvmstem_storage PROPERTIES
        LABELS "interpreter;stem;nr15")
set_tests_properties(rxvmstem_allocator_family PROPERTIES
        LABELS "interpreter;stem;memory")

crexx_label_concurrency_tests(SP01
        rxvmmemory_allocator
        rxvmworker_lifecycle
        rxvmactive_isolation
        rxvmplugin_context_ownership
        rxpa_static_catalogue_replay
        rxpa_static_session_lifecycle
        rxpa_session_factory_failure_rollback
        rxpa_session_manifest_fail_closed
        rxpa_float_static_alias_binding
        rxpa_legacy_call_serialization
        rxpa_process_reentrant_overlap
        rxpa_legacy_recursive_reentry
        rxpa_branch_free_load_binding
        rxpa_bound_legacy_serialization
        rxpa_legacy_transition_quiescence)
crexx_label_concurrency_tests(SP02 rxvmchannel_provider_registry)
crexx_label_concurrency_tests(SP03
        rxvmchannel_provider_registry
        rxvmbyteendpoint_substrate
        rxvmchannel_byte_provider)
crexx_label_concurrency_tests(SP04
        rxvmchannel_provider_registry
        rxvmbyteendpoint_substrate
        rxvmchannel_byte_provider)

add_test(
        NAME crexxsaa_cache_tool_location
        COMMAND crexxsaa_tool --cache-dir ${CMAKE_CURRENT_BINARY_DIR}/crexxsaa-cache-tool-test --location)
set_tests_properties(crexxsaa_cache_tool_location PROPERTIES
        PASS_REGULAR_EXPRESSION "crexxsaa-cache-tool-test")

add_test(
        NAME crexxsaa_cache_tool_clear
        COMMAND crexxsaa_tool --cache-dir ${CMAKE_CURRENT_BINARY_DIR}/crexxsaa-cache-tool-test --clear --list)
set_tests_properties(crexxsaa_cache_tool_clear PROPERTIES
        PASS_REGULAR_EXPRESSION "cleared")

add_test(
        NAME rxpa_utf_validation
        COMMAND test_rxpa_utf_validation
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(rxpa_utf_validation PROPERTIES LABELS "interpreter;rxpa;utf")

function(crexx_add_vm_expected_failure_test)
    set(options)
    set(oneValueArgs NAME RUNNER EXPECTED_EXIT_CODE EXPECTED_FAILURE_DESCRIPTION)
    set(multiValueArgs RXVM_ARGS REQUIRED_OUTPUT_REGEX FORBIDDEN_OUTPUT_REGEX TEST_LABELS)
    cmake_parse_arguments(CREXX "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT CREXX_NAME OR NOT CREXX_RUNNER OR "${CREXX_EXPECTED_EXIT_CODE}" STREQUAL "")
        message(FATAL_ERROR
                "crexx_add_vm_expected_failure_test requires NAME, RUNNER, and EXPECTED_EXIT_CODE")
    endif()

    set(_crexx_script "${CMAKE_CURRENT_BINARY_DIR}/${CREXX_NAME}_expected_failure.cmake")
    crexx_write_runtime_script(
            OUTPUT "${_crexx_script}"
            NAME "${CREXX_NAME}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            RUNNER "${CREXX_RUNNER}"
            PROGRAM "${CREXX_RXVM_ARGS}"
            EXPECTED_EXIT_CODE "${CREXX_EXPECTED_EXIT_CODE}"
            EXPECTED_FAILURE_DESCRIPTION "${CREXX_EXPECTED_FAILURE_DESCRIPTION}"
            REQUIRED_OUTPUT_REGEX ${CREXX_REQUIRED_OUTPUT_REGEX}
            FORBIDDEN_OUTPUT_REGEX ${CREXX_FORBIDDEN_OUTPUT_REGEX}
    )
    add_test(
            NAME ${CREXX_NAME}
            COMMAND ${CMAKE_COMMAND} -P "${_crexx_script}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    if(CREXX_TEST_LABELS)
        set_tests_properties(${CREXX_NAME} PROPERTIES LABELS "${CREXX_TEST_LABELS}")
    endif()
endfunction()

function(crexx_add_vm_test)
    set(options)
    set(oneValueArgs NAME PASS_REGULAR_EXPRESSION EXPECTED_EXIT_CODE EXPECTED_FAILURE_DESCRIPTION)
    set(multiValueArgs RXVM_ARGS REQUIRED_OUTPUT_REGEX FORBIDDEN_OUTPUT_REGEX)
    cmake_parse_arguments(CREXX "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(CREXX_PASS_REGULAR_EXPRESSION AND NOT "${CREXX_EXPECTED_EXIT_CODE}" STREQUAL "")
        message(FATAL_ERROR
                "${CREXX_NAME}: expected-failure tests must not use PASS_REGULAR_EXPRESSION")
    endif()

    if(NOT "${CREXX_EXPECTED_EXIT_CODE}" STREQUAL "")
        crexx_add_vm_expected_failure_test(
                NAME ${CREXX_NAME}
                RUNNER ${CREXX_RXVM_EXECUTABLE}
                RXVM_ARGS ${CREXX_RXVM_ARGS}
                EXPECTED_EXIT_CODE ${CREXX_EXPECTED_EXIT_CODE}
                EXPECTED_FAILURE_DESCRIPTION "${CREXX_EXPECTED_FAILURE_DESCRIPTION}"
                REQUIRED_OUTPUT_REGEX ${CREXX_REQUIRED_OUTPUT_REGEX}
                FORBIDDEN_OUTPUT_REGEX ${CREXX_FORBIDDEN_OUTPUT_REGEX}
        )
    else()
        add_test(
                NAME ${CREXX_NAME}
                COMMAND ${CREXX_RXVM_EXECUTABLE} ${CREXX_RXVM_ARGS}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

        if(CREXX_PASS_REGULAR_EXPRESSION)
            set_tests_properties(${CREXX_NAME} PROPERTIES
                    PASS_REGULAR_EXPRESSION "${CREXX_PASS_REGULAR_EXPRESSION}")
        endif()
    endif()
endfunction()

function(crexx_add_rxas_vm_binary SOURCE_NAME DESCRIPTION)
    add_custom_command(
            OUTPUT ${SOURCE_NAME}.rxbin
            COMMAND $<TARGET_FILE:rxas> -o ${SOURCE_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/tests/${SOURCE_NAME}
            DEPENDS tests/${SOURCE_NAME}.rxas rxas rxvm mc_decimal_dynamic
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "cREXX RXAS Instruction Tests (${DESCRIPTION}) ..."
    )

    add_custom_target(run_${SOURCE_NAME} ALL DEPENDS ${SOURCE_NAME}.rxbin)
    add_dependencies(run_${SOURCE_NAME} rxvm)
endfunction()

function(crexx_add_generated_jtable_matrix ALGORITHM DISPLAY_NAME)
    set(SOURCE_NAME tests_jump_table_${ALGORITHM}_sources)
    set(GENERATED_SOURCE ${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_NAME}.rxas)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_jump_table_linear_sources.rxas MATRIX_SOURCE)
    string(REPLACE " linear" " ${ALGORITHM}" MATRIX_SOURCE "${MATRIX_SOURCE}")
    string(REPLACE "Linear" "${DISPLAY_NAME}" MATRIX_SOURCE "${MATRIX_SOURCE}")
    file(GENERATE OUTPUT ${GENERATED_SOURCE} CONTENT "${MATRIX_SOURCE}")

    add_custom_command(
            OUTPUT ${SOURCE_NAME}.rxbin
            COMMAND $<TARGET_FILE:rxas> -o ${SOURCE_NAME} ${GENERATED_SOURCE}
            DEPENDS tests/tests_jump_table_linear_sources.rxas rxas rxvm mc_decimal_dynamic
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "cREXX RXAS Instruction Tests (${ALGORITHM} jump table source matrix) ..."
    )
    add_custom_target(run_${SOURCE_NAME} ALL DEPENDS ${SOURCE_NAME}.rxbin)
    add_dependencies(run_${SOURCE_NAME} rxvm)
    crexx_add_vm_test(
            NAME rxasjumptable${ALGORITHM}sources
            RXVM_ARGS ${SOURCE_NAME}
            PASS_REGULAR_EXPRESSION "PASS: Jump Table ${DISPLAY_NAME} Source Matrix")
endfunction()

function(crexx_add_jtable_corruption_base ALGORITHM)
    set(BASE_NAME tests_jump_table_corruption_${ALGORITHM})
    set(BASE_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/${BASE_NAME}.rxas")
    set(BASE_RXBIN "${CMAKE_CURRENT_BINARY_DIR}/${BASE_NAME}.rxbin")
    set(BASE_TARGET prepare_${BASE_NAME})

    if(TARGET ${BASE_TARGET})
        return()
    endif()

    set(ALGORITHM "${ALGORITHM}")
    configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_jump_table_corruption.rxas.in
            ${BASE_SOURCE}
            @ONLY)

    add_custom_command(
            OUTPUT ${BASE_RXBIN}
            COMMAND $<TARGET_FILE:rxas> -o ${BASE_NAME} ${BASE_SOURCE}
            DEPENDS ${BASE_SOURCE} rxas
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "cREXX RXAS jump-table corruption base (${ALGORITHM}) ...")

    add_custom_target(${BASE_TARGET} DEPENDS ${BASE_RXBIN})
endfunction()

function(crexx_add_jtable_corruption_fixture NAME ALGORITHM MUTATION)
    set(BASE_NAME tests_jump_table_corruption_${ALGORITHM})
    set(BASE_RXBIN "${CMAKE_CURRENT_BINARY_DIR}/${BASE_NAME}.rxbin")
    set(BASE_TARGET prepare_${BASE_NAME})
    set(CORRUPT_RXBIN "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.rxbin")

    crexx_add_jtable_corruption_base(${ALGORITHM})

    add_custom_command(
            OUTPUT ${CORRUPT_RXBIN}
            COMMAND $<TARGET_FILE:mutate_jtable_rxbin>
                    ${BASE_RXBIN} ${CORRUPT_RXBIN} ${MUTATION}
            DEPENDS ${BASE_RXBIN} ${BASE_TARGET} mutate_jtable_rxbin
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Mutating jump table ${NAME} (${MUTATION}) ...")

    add_custom_target(run_${NAME} ALL DEPENDS ${CORRUPT_RXBIN})
    add_dependencies(run_${NAME} ${BASE_TARGET})
    add_test(
            NAME ${NAME}_rxvm
            COMMAND ${CMAKE_COMMAND}
                    -DRUNNER=${CREXX_RXVM_EXECUTABLE}
                    -DRXBIN=${CORRUPT_RXBIN}
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_jtable_corruption.cmake
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME}_rxvm PROPERTIES
            LABELS "interpreter;rxas;jtable;corruption")
endfunction()

function(crexx_add_rxas_vm_failure_regex NAME SOURCE_NAME EXPECTED_REGEX)
    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
    set(SCRIPT_CONTENT "")
    set(DOLLAR "$")

    string(APPEND SCRIPT_CONTENT
            "execute_process(\n"
            "  COMMAND \"$<TARGET_FILE:rxas>\" -o \"${SOURCE_NAME}\" \"${CMAKE_CURRENT_SOURCE_DIR}/tests/${SOURCE_NAME}.rxas\"\n"
            "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
            "  OUTPUT_VARIABLE rxas_out\n"
            "  ERROR_VARIABLE rxas_err\n"
            "  RESULT_VARIABLE rxas_res\n"
            ")\n"
            "if(NOT rxas_res EQUAL 0)\n"
            "  message(FATAL_ERROR \"rxas failed for ${SOURCE_NAME}.\\nSTDOUT:\\n${DOLLAR}{rxas_out}\\nSTDERR:\\n${DOLLAR}{rxas_err}\")\n"
            "endif()\n"
            "execute_process(\n"
            "  COMMAND \"${CREXX_RXVM_EXECUTABLE}\" \"${SOURCE_NAME}\"\n"
            "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
            "  OUTPUT_VARIABLE rxvm_out\n"
            "  ERROR_VARIABLE rxvm_err\n"
            "  RESULT_VARIABLE rxvm_res\n"
            ")\n"
            "set(full_out \"${DOLLAR}{rxvm_out}${DOLLAR}{rxvm_err}\")\n"
            "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${DOLLAR}{full_out}\")\n"
            "if(NOT full_out MATCHES \"${EXPECTED_REGEX}\")\n"
            "  message(FATAL_ERROR \"Expected ${SOURCE_NAME} to report ${EXPECTED_REGEX}.\\nSTDOUT/STDERR:\\n${DOLLAR}{full_out}\\nExit: ${DOLLAR}{rxvm_res}\")\n"
            "endif()\n")

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

    add_test(
            NAME ${NAME}
            COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES LABELS "interpreter;rxas;utf")
endfunction()

function(crexx_add_rxas_assembly_failure_regex NAME SOURCE_NAME EXPECTED_REGEX)
    set(SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.cmake")
    set(SCRIPT_CONTENT "")
    set(DOLLAR "$")

    string(APPEND SCRIPT_CONTENT
            "execute_process(\n"
            "  COMMAND \"$<TARGET_FILE:rxas>\" -o \"${SOURCE_NAME}\" \"${CMAKE_CURRENT_SOURCE_DIR}/tests/${SOURCE_NAME}.rxas\"\n"
            "  WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"\n"
            "  OUTPUT_VARIABLE rxas_out\n"
            "  ERROR_VARIABLE rxas_err\n"
            "  RESULT_VARIABLE rxas_res\n"
            ")\n"
            "set(full_out \"${DOLLAR}{rxas_out}${DOLLAR}{rxas_err}\")\n"
            "string(REPLACE \"\\r\\n\" \"\\n\" full_out \"${DOLLAR}{full_out}\")\n"
            "if(rxas_res EQUAL 0)\n"
            "  message(FATAL_ERROR \"Expected rxas to reject ${SOURCE_NAME}.\\nSTDOUT/STDERR:\\n${DOLLAR}{full_out}\")\n"
            "endif()\n"
            "if(NOT full_out MATCHES \"${EXPECTED_REGEX}\")\n"
            "  message(FATAL_ERROR \"Expected ${SOURCE_NAME} to report ${EXPECTED_REGEX}.\\nSTDOUT/STDERR:\\n${DOLLAR}{full_out}\\nExit: ${DOLLAR}{rxas_res}\")\n"
            "endif()\n")

    file(GENERATE OUTPUT "${SCRIPT_FILE}" CONTENT "${SCRIPT_CONTENT}")

    add_test(
            NAME ${NAME}
            COMMAND ${CMAKE_COMMAND} -P "${SCRIPT_FILE}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${NAME} PROPERTIES LABELS "interpreter;rxas;jtable")
endfunction()

# Value Tests
add_test(
		NAME ts_regvalue_tester
		COMMAND ts_regvalue_tester
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
add_test(
        NAME test_jtable_policy
        COMMAND test_jtable_policy
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(test_jtable_policy PROPERTIES LABELS "interpreter;rxas;jtable")
add_test(
        NAME test_signal_mask
        COMMAND test_signal_mask
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(test_signal_mask PROPERTIES LABELS "interpreter;signal")

# Instruction Sanity Tests - Raise uncaptured signal
add_custom_command(
		OUTPUT tests_signals.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_signals ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_signals
		DEPENDS tests/tests_signals.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Instruction Tests (raise uncaptured signal) ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_signals ALL DEPENDS tests_signals.rxbin)

crexx_add_vm_test(
        NAME rxassignalstests
        RXVM_ARGS tests_signals
        EXPECTED_EXIT_CODE 30
        EXPECTED_FAILURE_DESCRIPTION "uncaught SIGNAL OTHER from tests_signals"
        REQUIRED_OUTPUT_REGEX
            "7\\. Should signal with a PANIC"
            "PANIC: Test 7 message \\(SIGNAL OTHER\\)"
)
if(BUILD_TESTING)
    set(_instrumented_signal_required_output
            "PANIC: Test 7 message \\(SIGNAL OTHER\\)"
            "VM instrumentation: PASS .*branches=[1-9][0-9]* calls=[1-9][0-9]* returns=[1-9][0-9]* interrupts=[1-9][0-9]*/[1-9][0-9]*/[1-9][0-9]*/[1-9][0-9]*")
    crexx_add_vm_expected_failure_test(
            NAME rxvminstrumentedsignaltests
            RUNNER $<TARGET_FILE:rxvm_instrumented>
            RXVM_ARGS tests_signals
            EXPECTED_EXIT_CODE 30
            EXPECTED_FAILURE_DESCRIPTION "instrumented VM uncaught SIGNAL OTHER"
            REQUIRED_OUTPUT_REGEX ${_instrumented_signal_required_output}
            TEST_LABELS interpreter instrumentation signal)
    crexx_add_vm_expected_failure_test(
            NAME rxbvminstrumentedsignaltests
            RUNNER $<TARGET_FILE:rxbvm_instrumented>
            RXVM_ARGS tests_signals
            EXPECTED_EXIT_CODE 30
            EXPECTED_FAILURE_DESCRIPTION "instrumented threaded VM uncaught SIGNAL OTHER"
            REQUIRED_OUTPUT_REGEX ${_instrumented_signal_required_output}
            TEST_LABELS interpreter instrumentation signal)
endif()

# Instruction Sanity Tests - fallback panic location and source reporting
add_custom_command(
		OUTPUT tests_panic_source.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_panic_source ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_panic_source
		DEPENDS tests/tests_panic_source.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Instruction Tests (fallback panic source reporting) ..."
)

add_custom_target(run_tests_panic_source ALL DEPENDS tests_panic_source.rxbin)

crexx_add_vm_test(
        NAME rxaspanicaddresstests
        RXVM_ARGS tests_panic_source
        EXPECTED_EXIT_CODE 6
        EXPECTED_FAILURE_DESCRIPTION "conversion panic with bytecode address"
        REQUIRED_OUTPUT_REGEX
            "PANIC: \\(SIGNAL CONVERSION_ERROR\\)"
            "address [0-9]+ \\(0x[0-9a-fA-F]+\\)"
)
crexx_add_vm_test(
        NAME rxaspanicsourcetests
        RXVM_ARGS tests_panic_source
        EXPECTED_EXIT_CODE 6
        EXPECTED_FAILURE_DESCRIPTION "conversion panic with source location"
        REQUIRED_OUTPUT_REGEX
            "PANIC: \\(SIGNAL CONVERSION_ERROR\\)"
            "source: panic_source\\.rexx:7:3:"
)

# Instruction Sanity Tests - breakpoint reports the next instruction address
add_custom_command(
		OUTPUT tests_breakpoint_address.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_breakpoint_address ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_breakpoint_address
		DEPENDS tests/tests_breakpoint_address.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Instruction Tests (breakpoint address reporting) ..."
)

add_custom_target(run_tests_breakpoint_address ALL DEPENDS tests_breakpoint_address.rxbin)

crexx_add_vm_test(NAME rxasbreakpointaddresstests RXVM_ARGS tests_breakpoint_address PASS_REGULAR_EXPRESSION "bp-source: say 'target'.*breakpoint-tests-passed")
crexx_add_vm_test(NAME rxasbreakpointtraceeventtests RXVM_ARGS tests_breakpoint_address PASS_REGULAR_EXPRESSION "bp-symbol: target.*breakpoint-tests-passed")
if(BUILD_TESTING)
    add_test(NAME rxvminstrumentedbreakpointtests
            COMMAND $<TARGET_FILE:rxvm_instrumented> tests_breakpoint_address
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    add_test(NAME rxbvminstrumentedbreakpointtests
            COMMAND $<TARGET_FILE:rxbvm_instrumented> tests_breakpoint_address
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(rxvminstrumentedbreakpointtests rxbvminstrumentedbreakpointtests PROPERTIES
            PASS_REGULAR_EXPRESSION "VM instrumentation: PASS .*returns=[1-9][0-9]* interrupts=[1-9][0-9]*/[1-9][0-9]*/[1-9][0-9]*/0"
            LABELS "interpreter;instrumentation;breakpoint")
endif()

# Instruction Sanity Tests - register flag partitioning
crexx_add_rxas_vm_binary(tests_register_flags "register flag partitioning")
crexx_add_vm_test(NAME rxasregisterflagtests RXVM_ARGS tests_register_flags PASS_REGULAR_EXPRESSION "PASS: Register Flag Partition")

# Instruction Sanity Tests - UTF status cache flags
crexx_add_rxas_vm_binary(tests_utf_flags "UTF flag cache")
crexx_add_vm_test(NAME rxasutfflagtests RXVM_ARGS tests_utf_flags PASS_REGULAR_EXPRESSION "PASS: UTF Flag Cache")
crexx_add_rxas_vm_failure_regex(rxasutfboundaryfreadlinetests tests_utf_freadline_boundary "UNICODE_ERROR")
crexx_add_rxas_vm_failure_regex(rxasutfboundaryfreadcdpttests tests_utf_freadcdpt_boundary "UNICODE_ERROR")

# Instruction Sanity Tests - bulk attribute insert/delete
crexx_add_rxas_vm_binary(tests_attributes "attribute bulk insert/delete")
crexx_add_vm_test(NAME rxasattributebulktests RXVM_ARGS tests_attributes PASS_REGULAR_EXPRESSION "PASS: Attribute Bulk Ops")

# Instruction Sanity Tests - reference values
crexx_add_rxas_vm_binary(tests_references "reference value instructions")
crexx_add_vm_test(NAME rxasreferencetests RXVM_ARGS tests_references PASS_REGULAR_EXPRESSION "PASS: Reference RXAS Ops")
crexx_add_vm_test(NAME rxasreferencecatchtests RXVM_ARGS tests_references PASS_REGULAR_EXPRESSION "caught-reference-invalid.*PASS: Reference RXAS Ops")

# PERF2-02 exact direct MKREF guards and mandatory fallback paths
crexx_add_rxas_vm_binary(tests_perf2_reference_guards "PERF2-02 direct reference guards")
crexx_add_vm_test(NAME rxasperf2referenceguardtests RXVM_ARGS tests_perf2_reference_guards PASS_REGULAR_EXPRESSION "PASS: PERF2-02 Reference Guards")

# PERF2-05 R2a private reference-descriptor materialization and fallbacks
crexx_add_rxas_vm_binary(tests_perf2_partial_reference_materialization "PERF2-05 partial reference materialization")
crexx_add_vm_test(NAME rxasperf2partialreferencetests RXVM_ARGS tests_perf2_partial_reference_materialization PASS_REGULAR_EXPRESSION "PASS: PERF2-05 Partial Reference Materialization")

# PERF2-05 R1a private exact local-reference relink and fallbacks
crexx_add_rxas_vm_binary(tests_perf2_exact_relink "PERF2-05 exact reference relink")
crexx_add_vm_test(NAME rxasperf2exactrelinktests RXVM_ARGS tests_perf2_exact_relink PASS_REGULAR_EXPRESSION "PASS: PERF2-05 Exact Reference Relink")

# Instruction Sanity Tests - object initialization lifecycle
crexx_add_rxas_vm_binary(tests_object_initialization "object initialization lifecycle instructions")
crexx_add_vm_test(NAME rxasobjectinitializationtests RXVM_ARGS tests_object_initialization PASS_REGULAR_EXPRESSION "PASS: Object Initialization RXAS Ops")
crexx_add_vm_test(NAME rxasobjectinitializationcatchtests RXVM_ARGS tests_object_initialization PASS_REGULAR_EXPRESSION "caught-object-not-initialized.*PASS: Object Initialization RXAS Ops")

# Instruction Sanity Tests - focused signal handler mode coverage
crexx_add_rxas_vm_binary(tests_signal_call_attrs "signal call argument attributes")
crexx_add_vm_test(NAME rxassignalattrcodetests RXVM_ARGS tests_signal_call_attrs PASS_REGULAR_EXPRESSION "attr-code: 30.*attr-source: signal other")
crexx_add_vm_test(NAME rxassignalattrnametests RXVM_ARGS tests_signal_call_attrs PASS_REGULAR_EXPRESSION "attr-name: OTHER.*attr-source: signal other")
crexx_add_vm_test(NAME rxassignalattrpayloadtests RXVM_ARGS tests_signal_call_attrs PASS_REGULAR_EXPRESSION "attr-payload: attr payload.*attr-source: signal other")
crexx_add_vm_test(NAME rxassignalattrsourcetests RXVM_ARGS tests_signal_call_attrs PASS_REGULAR_EXPRESSION "attr-source: signal other")

crexx_add_rxas_vm_binary(tests_signal_handler_modes "signal handler response modes")
crexx_add_vm_test(NAME rxassignalmodeignoretests RXVM_ARGS tests_signal_handler_modes PASS_REGULAR_EXPRESSION "mode-ignore-ok.*mode-ret-after")
crexx_add_vm_test(NAME rxassignalmodebranchtests RXVM_ARGS tests_signal_handler_modes PASS_REGULAR_EXPRESSION "mode-branch-hit.*mode-ret-after")
crexx_add_vm_test(NAME rxassignalmodecallbranchtests RXVM_ARGS tests_signal_handler_modes PASS_REGULAR_EXPRESSION "mode-callbr-handler: callbr payload.*mode-ret-after")
crexx_add_vm_test(NAME rxassignalmodereturntests RXVM_ARGS tests_signal_handler_modes PASS_REGULAR_EXPRESSION "mode-ret-after")

crexx_add_rxas_vm_binary(tests_signal_call_unwind "signal call-window mapping unwind")
crexx_add_vm_test(NAME rxassignalcallunwindtests RXVM_ARGS tests_signal_call_unwind PASS_REGULAR_EXPRESSION "PASS: Signal Call Mapping Unwind")

crexx_add_rxas_vm_binary(tests_signal_mask_bounds "signal mask boundary handling")
crexx_add_vm_test(NAME rxassignalmaskotherignoretests RXVM_ARGS tests_signal_mask_bounds PASS_REGULAR_EXPRESSION "mask-other-ignore-ok.*mask-tests-passed")
crexx_add_vm_test(NAME rxassignalmaskbreakpointignoretests RXVM_ARGS tests_signal_mask_bounds PASS_REGULAR_EXPRESSION "mask-breakpoint-ignore-ok.*mask-tests-passed")

crexx_add_rxas_vm_binary(tests_signal_silent_halt "signal silent halt response")
crexx_add_vm_test(NAME rxassignalsilenthalttests RXVM_ARGS tests_signal_silent_halt)

crexx_add_rxas_vm_binary(tests_signal_frame_scope "signal frame handler table scope")
crexx_add_vm_test(NAME rxassignalframescopechildtests RXVM_ARGS tests_signal_frame_scope PASS_REGULAR_EXPRESSION "frame-handler: from child.*frame-main-after")
crexx_add_vm_test(NAME rxassignalframescopeparenttests RXVM_ARGS tests_signal_frame_scope PASS_REGULAR_EXPRESSION "frame-handler: after child ignore.*frame-main-after")

crexx_add_rxas_vm_binary(tests_signal_nested_pending "nested pending signal delivery")
crexx_add_vm_test(NAME rxassignalnestedpendingtests RXVM_ARGS tests_signal_nested_pending PASS_REGULAR_EXPRESSION "nested:outer-before>outer-after>inner>main-after")

crexx_add_rxas_vm_binary(tests_signal_actions "action-aware signal handlers")
crexx_add_vm_test(NAME rxassignalactionskiptests RXVM_ARGS tests_signal_actions PASS_REGULAR_EXPRESSION "action-skip-after.*action-dynamic-after")
crexx_add_vm_test(NAME rxassignalactiondynamictests RXVM_ARGS tests_signal_actions PASS_REGULAR_EXPRESSION "action-handler: dynamic payload.*action-dynamic-after")

crexx_add_rxas_vm_binary(tests_signal_retry_retired "retired signal retry marker")
crexx_add_vm_test(
        NAME rxassignalretryretiredtests
        RXVM_ARGS tests_signal_retry_retired
        EXPECTED_EXIT_CODE 30
        EXPECTED_FAILURE_DESCRIPTION "retired signal retry marker fails safely"
        REQUIRED_OUTPUT_REGEX
            "PANIC: retired retry marker \\(SIGNAL OTHER\\)"
        FORBIDDEN_OUTPUT_REGEX
            "FAIL: retired retry marker resumed execution")

crexx_add_rxas_vm_binary(tests_signal_pushpop "signal handler push/pop scope")
crexx_add_vm_test(NAME rxassignalpushpoprestoretests RXVM_ARGS tests_signal_pushpop PASS_REGULAR_EXPRESSION "pushpop-restored-ignore.*pushpop-after-restored-ignore")
crexx_add_vm_test(NAME rxassignalpushpopbranchtests RXVM_ARGS tests_signal_pushpop PASS_REGULAR_EXPRESSION "pushpop-branch-hit.*pushpop-after-restored-ignore")
crexx_add_vm_test(NAME rxassignalpushpophandlerrestoretests RXVM_ARGS tests_signal_pushpop PASS_REGULAR_EXPRESSION "pushpop-after-restored-ignore")

crexx_add_rxas_vm_binary(tests_signal_branch_value "signal branch with bound value")
crexx_add_vm_test(NAME rxassignalbrvrawtests RXVM_ARGS tests_signal_branch_value PASS_REGULAR_EXPRESSION "brv-has-raw: 1.*brv-payload: value payload")
crexx_add_vm_test(NAME rxassignalbrvnametests RXVM_ARGS tests_signal_branch_value PASS_REGULAR_EXPRESSION "brv-name: OTHER.*brv-payload: value payload")
crexx_add_vm_test(NAME rxassignalbrvpayloadtests RXVM_ARGS tests_signal_branch_value PASS_REGULAR_EXPRESSION "brv-payload: value payload")

# NR-09 large-instruction batch
crexx_add_rxas_vm_binary(tests_nr09_superinstructions "NR-09 large instructions")
crexx_add_vm_test(NAME rxasnr09superinstructiontests RXVM_ARGS tests_nr09_superinstructions PASS_REGULAR_EXPRESSION "PASS: NR-09 Superinstructions")

# Basic Instruction Tests
add_custom_command(
		OUTPUT tests_basic.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_basic ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_basic
		DEPENDS tests/tests_basic.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Basic Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_basic ALL DEPENDS tests_basic.rxbin)

crexx_add_vm_test(NAME rxasbasictests RXVM_ARGS tests_basic)

# The broad functional suite follows the product rxvm target.  Keep one small
# explicit contract test per concrete engine so dispatch-specific build or
# preparation failures are still caught.
foreach(_crexx_concrete_vm IN LISTS CREXX_CONCRETE_VM_TARGETS)
    add_test(
            NAME rxasdispatchcontract-${_crexx_concrete_vm}
            COMMAND $<TARGET_FILE:${_crexx_concrete_vm}> tests_basic
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(rxasdispatchcontract-${_crexx_concrete_vm} PROPERTIES
            LABELS "interpreter;rxas;dispatch")
    crexx_label_concurrency_tests(SP02
            rxasdispatchcontract-${_crexx_concrete_vm})
endforeach()

if(BUILD_TESTING AND CREXX_VM_PROFILING)
    crexx_add_rxas_vm_binary(tests_sequence_profile "dynamic sequence profile")
    crexx_add_rxas_vm_binary(tests_procedure_profile "procedure timing profile")
    crexx_add_rxas_vm_binary(tests_call_census "NR-05 call census")

    set(PROFILING_DOC_EXAMPLE
            ${CMAKE_SOURCE_DIR}/docs/books/crexx_programming_guide/examples/profiling_demo.rxas)
    add_custom_command(
            OUTPUT profiling_demo.rxbin
            COMMAND $<TARGET_FILE:rxas> -o profiling_demo ${PROFILING_DOC_EXAMPLE}
            DEPENDS ${PROFILING_DOC_EXAMPLE} rxas rxvm mc_decimal_dynamic
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "cREXX VM profiling documentation example ...")
    add_custom_target(run_profiling_demo ALL DEPENDS profiling_demo.rxbin)
    add_dependencies(run_profiling_demo rxvm rxbvm)

    add_test(NAME rxvmprofiletable
            COMMAND ${CREXX_RXVM_EXECUTABLE} --profile tests_procedure_profile)
    add_test(NAME rxbvmprofiletable
            COMMAND $<TARGET_FILE:rxbvm> --profile tests_procedure_profile)
    set_tests_properties(rxvmprofiletable PROPERTIES
            PASS_REGULAR_EXPRESSION "Procedures and methods.*procedure_profile.worker.*[ ]2[ ]+2[ ]+0"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            LABELS "interpreter;profiling")
    set_tests_properties(rxbvmprofiletable PROPERTIES
            PASS_REGULAR_EXPRESSION "Procedures and methods.*procedure_profile.worker.*[ ]2[ ]+2[ ]+0"
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            LABELS "interpreter;profiling")

    add_test(NAME rxvmprofilecsv
            COMMAND ${CMAKE_COMMAND}
                    "-DVM=${CREXX_RXVM_EXECUTABLE}"
                    "-DINPUT=${CMAKE_CURRENT_BINARY_DIR}/tests_procedure_profile.rxbin"
                    "-DOUTPUT=${CMAKE_CURRENT_BINARY_DIR}/rxvm-profile.CSV"
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_vm_profile_csv.cmake)
    add_test(NAME rxbvmprofilecsv
            COMMAND ${CMAKE_COMMAND}
                    "-DVM=$<TARGET_FILE:rxbvm>"
                    "-DINPUT=${CMAKE_CURRENT_BINARY_DIR}/tests_procedure_profile.rxbin"
                    "-DOUTPUT=${CMAKE_CURRENT_BINARY_DIR}/rxbvm-profile.csv"
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_vm_profile_csv.cmake)
    set_tests_properties(rxvmprofilecsv rxbvmprofilecsv PROPERTIES
            LABELS "interpreter;profiling")

    function(crexx_add_vm_call_census_test NAME VM_TARGET INPUT_TARGET CASE)
        if(VM_TARGET STREQUAL "rxvm")
            set(_vm_executable "${CREXX_RXVM_EXECUTABLE}")
        else()
            set(_vm_executable "$<TARGET_FILE:${VM_TARGET}>")
        endif()
        add_test(
                NAME ${NAME}
                COMMAND ${CMAKE_COMMAND}
                        "-DVM=${_vm_executable}"
                        "-DINPUT=${CMAKE_CURRENT_BINARY_DIR}/${INPUT_TARGET}.rxbin"
                        "-DOUTPUT=${CMAKE_CURRENT_BINARY_DIR}/${NAME}.csv"
                        "-DCASE=${CASE}"
                        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_vm_call_census.cmake)
        set_tests_properties(${NAME} PROPERTIES
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                LABELS "interpreter;profiling;call-census")
    endfunction()

    crexx_add_vm_call_census_test(
            rxvmcallcensus rxvm tests_call_census mechanics)
    crexx_add_vm_call_census_test(
            rxbvmcallcensus rxbvm tests_call_census mechanics)
    crexx_add_vm_call_census_test(
            rxvmcallcensusselectors rxvm tests_selector_descriptors selectors)
    crexx_add_vm_call_census_test(
            rxbvmcallcensusselectors rxbvm tests_selector_descriptors selectors)
    crexx_add_vm_call_census_test(
            rxvmcallcensussignalunwind rxvm tests_signal_call_unwind signal_unwind)
    crexx_add_vm_call_census_test(
            rxbvmcallcensussignalunwind rxbvm tests_signal_call_unwind signal_unwind)

    add_test(NAME rxvmsequence2
            COMMAND ${CMAKE_COMMAND}
                    "-DVM=${CREXX_RXVM_EXECUTABLE}"
                    "-DRXSEQ=$<TARGET_FILE:rxseq>"
                    "-DINPUT=${CMAKE_CURRENT_BINARY_DIR}/tests_sequence_profile.rxbin"
                    -DLENGTH=2
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_rxseq.cmake)
    add_test(NAME rxbvmsequence3
            COMMAND ${CMAKE_COMMAND}
                    "-DVM=$<TARGET_FILE:rxbvm>"
                    "-DRXSEQ=$<TARGET_FILE:rxseq>"
                    "-DINPUT=${CMAKE_CURRENT_BINARY_DIR}/tests_sequence_profile.rxbin"
                    -DLENGTH=3
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_rxseq.cmake)
    add_test(NAME rxvmsequence4
            COMMAND ${CMAKE_COMMAND}
                    "-DVM=${CREXX_RXVM_EXECUTABLE}"
                    "-DRXSEQ=$<TARGET_FILE:rxseq>"
                    "-DINPUT=${CMAKE_CURRENT_BINARY_DIR}/tests_sequence_profile.rxbin"
                    -DLENGTH=4
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_rxseq.cmake)
    add_test(NAME rxseqmodulemismatch
            COMMAND ${CMAKE_COMMAND}
                    "-DVM=${CREXX_RXVM_EXECUTABLE}"
                    "-DRXSEQ=$<TARGET_FILE:rxseq>"
                    "-DINPUT=${CMAKE_CURRENT_BINARY_DIR}/tests_sequence_profile.rxbin"
                    "-DWRONG_INPUT=${CMAKE_CURRENT_BINARY_DIR}/tests_basic.rxbin"
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_rxseq_mismatch.cmake)
    add_test(NAME vmprofilingdocumentation
            COMMAND ${CMAKE_COMMAND}
                    "-DVM=${CREXX_RXVM_EXECUTABLE}"
                    "-DRXSEQ=$<TARGET_FILE:rxseq>"
                    "-DINPUT=${CMAKE_CURRENT_BINARY_DIR}/profiling_demo.rxbin"
                    "-DOUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}"
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_vm_profiling_docs.cmake)
    set_tests_properties(
            rxvmsequence2 rxbvmsequence3 rxvmsequence4 rxseqmodulemismatch
            vmprofilingdocumentation
            PROPERTIES LABELS "interpreter;profiling;sequence")
endif()

# Integer Math Instruction Tests
add_custom_command(
		OUTPUT tests_int_math.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_int_math ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_int_math
		DEPENDS tests/tests_int_math.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Integer Math Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_int_math ALL DEPENDS tests_int_math.rxbin)

crexx_add_vm_test(NAME rxasintmathtests RXVM_ARGS tests_int_math)

crexx_add_rxas_vm_binary(tests_int_contract "signed 64-bit integer contract")
crexx_add_vm_test(
        NAME rxasintcontracttests
        RXVM_ARGS tests_int_contract
        PASS_REGULAR_EXPRESSION "PASS: Signed 64-bit Integer Contract")
crexx_add_rxas_assembly_failure_regex(
        rxasintoverflowpositive tests_int_overflow_positive
        "integer literal is outside the signed 64-bit range")
crexx_add_rxas_assembly_failure_regex(
        rxasintoverflownegative tests_int_overflow_negative
        "integer literal is outside the signed 64-bit range")
set_tests_properties(rxasintoverflowpositive rxasintoverflownegative PROPERTIES
        LABELS "interpreter;rxas;integer;negative")

# Float Math Instruction Tests
add_custom_command(
		OUTPUT tests_float_math.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_float_math ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_float_math
		DEPENDS tests/tests_float_math.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Float Math Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_float_math ALL DEPENDS tests_float_math.rxbin)

crexx_add_vm_test(NAME rxasfloatmathtests RXVM_ARGS tests_float_math)

# Float Constant Pool Format and Runtime Tests
add_custom_command(
        OUTPUT tests_float_pool.rxbin
        COMMAND $<TARGET_FILE:rxas> -o tests_float_pool ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_float_pool
        DEPENDS tests/tests_float_pool.rxas rxas rxvm mc_decimal_dynamic
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX RXAS Float Constant Pool Tests ..."
)

add_custom_target(run_tests_float_pool ALL DEPENDS tests_float_pool.rxbin)

crexx_add_vm_test(NAME rxasfloatpooltests RXVM_ARGS tests_float_pool)

add_executable(test_float_pool_format tests/test_float_pool_format.c)
target_link_libraries(test_float_pool_format PRIVATE rxbin)
target_include_directories(test_float_pool_format PRIVATE
        ${CMAKE_SOURCE_DIR}/platform
        ${CMAKE_SOURCE_DIR}/binutils/include)
add_dependencies(test_float_pool_format run_tests_float_pool)
add_test(NAME float_pool_format_check COMMAND test_float_pool_format WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Portable RXBIN 007 compact-instruction format tests
add_custom_command(
        OUTPUT tests_compact_format.rxbin
        COMMAND $<TARGET_FILE:rxas> -o tests_compact_format ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_compact_format
        DEPENDS tests/tests_compact_format.rxas rxas rxvm mc_decimal_dynamic
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "cREXX RXBIN 007 portable-format tests ..."
)

add_custom_target(run_tests_compact_format ALL DEPENDS tests_compact_format.rxbin)

crexx_add_vm_test(NAME rxascompactformattests RXVM_ARGS tests_compact_format)

add_executable(test_compact_format tests/test_compact_format.c)
target_link_libraries(test_compact_format PRIVATE rxbin)
target_include_directories(test_compact_format PRIVATE
        ${CMAKE_SOURCE_DIR}/platform
        ${CMAKE_SOURCE_DIR}/binutils/include)
add_dependencies(test_compact_format run_tests_compact_format)
add_test(NAME compact_format_check COMMAND test_compact_format WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# String Instruction Tests
add_custom_command(
		OUTPUT tests_string.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_string ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_string
		DEPENDS tests/tests_string.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS String Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_string ALL DEPENDS tests_string.rxbin)

crexx_add_vm_test(NAME rxasstringtests RXVM_ARGS tests_string)

# Logic and Flow Instruction Tests
add_custom_command(
		OUTPUT tests_logic_flow.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_logic_flow ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_logic_flow
		DEPENDS tests/tests_logic_flow.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Logic and Flow Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_logic_flow ALL DEPENDS tests_logic_flow.rxbin)

crexx_add_vm_test(NAME rxaslogicflowtests RXVM_ARGS tests_logic_flow)

# System and I/O Instruction Tests
add_custom_command(
		OUTPUT tests_system_io.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_system_io ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_system_io
		DEPENDS tests/tests_system_io.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS System and I/O Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_system_io ALL DEPENDS tests_system_io.rxbin)

crexx_add_vm_test(NAME rxassystemiotests RXVM_ARGS tests_system_io)

# Binary buffer, I/O, and explicit slice instruction tests
crexx_add_rxas_vm_binary(tests_binary "binary buffer and I/O")
crexx_add_vm_test(NAME rxasbinaryslicetests RXVM_ARGS tests_binary PASS_REGULAR_EXPRESSION "PASS: Binary Buffer IO and Explicit Slice")
# The authored RXAS uses fixed scratch filenames.  Keep repeated or overlapping
# invocations from truncating those files.
set_tests_properties(rxasbinaryslicetests PROPERTIES
        RESOURCE_LOCK rxas_binary_slice_scratch)
crexx_add_rxas_vm_binary(tests_binary_memory "typed binary memory operations")
crexx_add_vm_test(NAME rxasbinarymemorytests RXVM_ARGS tests_binary_memory PASS_REGULAR_EXPRESSION "PASS: Binary Memory Typed Ops")

# Gate F F1c executes the same local-channel image on both concrete VMs.
add_custom_command(
        OUTPUT test_gate_f_channel_local.rxbin
        COMMAND $<TARGET_FILE:rxas> -o test_gate_f_channel_local
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_gate_f_channel_local
        DEPENDS tests/test_gate_f_channel_local.rxas rxas rxbvm
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Assembling Gate F F1c local channel fixture ...")
add_custom_target(test_gate_f_channel_local_bin ALL
        DEPENDS test_gate_f_channel_local.rxbin)
add_test(NAME gate_f_channel_local_rxbvm
        COMMAND $<TARGET_FILE:rxbvm> test_gate_f_channel_local
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(gate_f_channel_local_rxbvm PROPERTIES
        LABELS "interpreter;perf3;gate-f;f1c;channels;lifecycle"
        PASS_REGULAR_EXPRESSION "PASS: Gate F F1c local channel"
        TIMEOUT 30)
crexx_label_concurrency_tests(SP02 gate_f_channel_local_rxbvm)
crexx_label_concurrency_tests(SP04 gate_f_channel_local_rxbvm)
crexx_label_concurrency_tests(SP08 gate_f_channel_local_rxbvm)
if(CREXX_THREADED_VM_SUPPORTED)
    add_dependencies(test_gate_f_channel_local_bin rxtvm)
    add_test(NAME gate_f_channel_local_rxtvm
            COMMAND $<TARGET_FILE:rxtvm> test_gate_f_channel_local
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(gate_f_channel_local_rxtvm PROPERTIES
            LABELS "interpreter;perf3;gate-f;f1c;channels;lifecycle"
            PASS_REGULAR_EXPRESSION "PASS: Gate F F1c local channel"
            TIMEOUT 30)
    crexx_label_concurrency_tests(SP02 gate_f_channel_local_rxtvm)
    crexx_label_concurrency_tests(SP04 gate_f_channel_local_rxtvm)
    crexx_label_concurrency_tests(SP08 gate_f_channel_local_rxtvm)
endif()

# Gate F F1e executes one transport/conformance image on both concrete VMs.
add_custom_command(
        OUTPUT test_gate_f_channel_process.rxbin
        COMMAND $<TARGET_FILE:rxas> -o test_gate_f_channel_process
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_gate_f_channel_process
        DEPENDS tests/test_gate_f_channel_process.rxas rxas rxbvm
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Assembling Gate F F1e process channel fixture ...")
add_custom_target(test_gate_f_channel_process_bin ALL
        DEPENDS test_gate_f_channel_process.rxbin)
add_dependencies(test_rxvmchannel_process
        test_gate_f_channel_process_bin rxbvm)
add_test(NAME rxvmchannel_process_crash_replacement
        COMMAND test_rxvmchannel_process
                ${CMAKE_CURRENT_BINARY_DIR}/test_gate_f_channel_process.rxbin)
set_tests_properties(rxvmchannel_process_crash_replacement PROPERTIES
        LABELS "interpreter;perf3;gate-f;f1e;channels;process;crash;lifecycle"
        PASS_REGULAR_EXPRESSION
        "PASS: Gate F F1e process crash and replacement"
        TIMEOUT 45)
crexx_label_concurrency_tests(SP04 rxvmchannel_process_crash_replacement)
crexx_label_concurrency_tests(SP08 rxvmchannel_process_crash_replacement)
add_test(NAME gate_f_channel_process_rxbvm
        COMMAND $<TARGET_FILE:rxbvm> test_gate_f_channel_process
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
set_tests_properties(gate_f_channel_process_rxbvm PROPERTIES
        LABELS "interpreter;perf3;gate-f;f1e;channels;process;lifecycle"
        PASS_REGULAR_EXPRESSION "PASS: Gate F F1e process channel"
        TIMEOUT 45)
crexx_label_concurrency_tests(SP04 gate_f_channel_process_rxbvm)
crexx_label_concurrency_tests(SP08 gate_f_channel_process_rxbvm)
if(CREXX_THREADED_VM_SUPPORTED)
    add_dependencies(test_gate_f_channel_process_bin rxtvm)
    add_test(NAME gate_f_channel_process_rxtvm
            COMMAND $<TARGET_FILE:rxtvm> test_gate_f_channel_process
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(gate_f_channel_process_rxtvm PROPERTIES
            LABELS "interpreter;perf3;gate-f;f1e;channels;process;lifecycle"
            PASS_REGULAR_EXPRESSION "PASS: Gate F F1e process channel"
            TIMEOUT 45)
    crexx_label_concurrency_tests(SP04 gate_f_channel_process_rxtvm)
    crexx_label_concurrency_tests(SP08 gate_f_channel_process_rxtvm)
endif()

# Gate F replaces the pre-release spawn/redirect opcodes with the five channel
# operations.  Source assembly rejects the retired mnemonics, while a stale
# binary retaining numeric slot 466 must halt explicitly in both VM dispatches.
add_custom_command(
        OUTPUT tests_retired_process_opcode_base.rxbin
        COMMAND $<TARGET_FILE:rxas> -o tests_retired_process_opcode_base
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_retired_process_opcode
        DEPENDS tests/tests_retired_process_opcode.rxas rxas
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Assembling retired process-opcode base fixture ...")
set(_retired_process_opcode_bins)
foreach(_retired_process_opcode RANGE 466 471)
    set(_retired_process_opcode_bin
            "tests_retired_process_opcode_${_retired_process_opcode}.rxbin")
    add_custom_command(
            OUTPUT "${_retired_process_opcode_bin}"
            COMMAND $<TARGET_FILE:mutate_retired_process_opcode>
                    ${CMAKE_CURRENT_BINARY_DIR}/tests_retired_process_opcode_base.rxbin
                    ${CMAKE_CURRENT_BINARY_DIR}/${_retired_process_opcode_bin}
                    ${_retired_process_opcode}
            DEPENDS tests_retired_process_opcode_base.rxbin
                    mutate_retired_process_opcode
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Injecting retired process opcode ${_retired_process_opcode} into stale RXBIN fixture ...")
    list(APPEND _retired_process_opcode_bins
            "${_retired_process_opcode_bin}")
endforeach()
add_custom_target(test_retired_process_opcode_bin ALL
        DEPENDS ${_retired_process_opcode_bins})
add_dependencies(test_retired_process_opcode_bin rxbvm)
foreach(_retired_process_opcode RANGE 466 471)
    crexx_add_vm_expected_failure_test(
            NAME retired_process_opcode_${_retired_process_opcode}_rxbvm
            RUNNER $<TARGET_FILE:rxbvm>
            RXVM_ARGS tests_retired_process_opcode_${_retired_process_opcode}
            EXPECTED_EXIT_CODE 10
            EXPECTED_FAILURE_DESCRIPTION "retired process opcode fails explicitly"
            REQUIRED_OUTPUT_REGEX "UNKNOWN_INSTRUCTION"
            TEST_LABELS "interpreter;rxas;gate-f;f1d;compatibility")
    crexx_label_concurrency_tests(SP02
            retired_process_opcode_${_retired_process_opcode}_rxbvm)
    if(CREXX_THREADED_VM_SUPPORTED)
        crexx_add_vm_expected_failure_test(
                NAME retired_process_opcode_${_retired_process_opcode}_rxtvm
                RUNNER $<TARGET_FILE:rxtvm>
                RXVM_ARGS tests_retired_process_opcode_${_retired_process_opcode}
                EXPECTED_EXIT_CODE 10
                EXPECTED_FAILURE_DESCRIPTION "retired process opcode fails explicitly"
                REQUIRED_OUTPUT_REGEX "UNKNOWN_INSTRUCTION"
                TEST_LABELS "interpreter;rxas;gate-f;f1d;compatibility")
        crexx_label_concurrency_tests(SP02
                retired_process_opcode_${_retired_process_opcode}_rxtvm)
    endif()
endforeach()
if(CREXX_THREADED_VM_SUPPORTED)
    add_dependencies(test_retired_process_opcode_bin rxtvm)
endif()

add_custom_command(
        OUTPUT test_gate_f_channel_benchmark.rxbin
        COMMAND $<TARGET_FILE:rxas> -o test_gate_f_channel_benchmark
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_gate_f_channel_benchmark
        DEPENDS tests/test_gate_f_channel_benchmark.rxas rxas rxbvm
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Assembling Gate F F1b channel benchmark ...")
add_custom_target(test_gate_f_channel_benchmark_bin ALL
        DEPENDS test_gate_f_channel_benchmark.rxbin)
crexx_add_rxas_vm_binary(tests_jump_table "linear jump table dispatch")
crexx_add_vm_test(NAME rxasjumptabletests RXVM_ARGS tests_jump_table PASS_REGULAR_EXPRESSION "PASS: Jump Table Linear Ops")
crexx_add_rxas_vm_binary(tests_jump_table_linear_sources "linear jump table source matrix")
crexx_add_vm_test(NAME rxasjumptablelinearsources RXVM_ARGS tests_jump_table_linear_sources PASS_REGULAR_EXPRESSION "PASS: Jump Table Linear Source Matrix")
crexx_add_generated_jtable_matrix(openhash OpenHash)
crexx_add_generated_jtable_matrix(acph ACPH)
crexx_add_generated_jtable_matrix(auto Auto)
crexx_add_rxas_vm_binary(tests_jump_table_hash_edges "open-hash and ACPH jump table edge cases")
crexx_add_vm_test(NAME rxasjumptablehashedges RXVM_ARGS tests_jump_table_hash_edges PASS_REGULAR_EXPRESSION "PASS: Jump Table Hash Edge Cases")
crexx_add_rxas_vm_binary(tests_jump_table_auto_edges "auto jump table policy paths")
crexx_add_vm_test(NAME rxasjumptableautoedges RXVM_ARGS tests_jump_table_auto_edges PASS_REGULAR_EXPRESSION "PASS: Jump Table Auto Policy Paths")
crexx_add_rxas_vm_binary(tests_jump_table_rexx_strings "Rexx padded and numeric string jump tables")
crexx_add_vm_test(NAME rxasjumptablerexxstrings RXVM_ARGS tests_jump_table_rexx_strings PASS_REGULAR_EXPRESSION "PASS: Jump Table Rexx String Ops")
crexx_add_rxas_vm_binary(tests_jump_table_boundaries "jump table syntax and value boundaries")
crexx_add_vm_test(NAME rxasjumptableboundaries RXVM_ARGS tests_jump_table_boundaries PASS_REGULAR_EXPRESSION "PASS: Jump Table Boundaries")
crexx_add_jtable_corruption_fixture(rxasjtablecorruptalgorithm linear algorithm)
crexx_add_jtable_corruption_fixture(rxasjtablecorruptheader linear header)
crexx_add_jtable_corruption_fixture(rxasjtablecorruptlinearkey linear linear-key-offset)
crexx_add_jtable_corruption_fixture(rxasjtablecorruptlineartarget linear linear-target)
crexx_add_jtable_corruption_fixture(rxasjtablecorruptopenslot openhash open-slot-offset)
crexx_add_jtable_corruption_fixture(rxasjtablecorruptacphslot acph acph-slot-kind)
crexx_add_jtable_corruption_fixture(rxasjtablecorruptacphtarget acph acph-target)
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadnolabel tests_jump_table_bad_no_label "\\.jcase must decorate a same-line label")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadunknown tests_jump_table_bad_unknown "unknown jump table")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadjumpbsvariable tests_jump_table_bad_jumpbs_variable "jumpbs requires fixed-length")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadduplicate tests_jump_table_bad_duplicate "duplicate jump table case key")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadtypemismatch tests_jump_table_bad_type_mismatch "jump table key type mismatch")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadempty tests_jump_table_bad_empty "jump table has no cases")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadduplicatebinary tests_jump_table_bad_duplicate_binary "duplicate jump table case key")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadduplicatetable tests_jump_table_bad_duplicate_table "duplicate jump table declaration")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadalgorithmname tests_jump_table_bad_algorithm_name "jump table algorithm must be auto, linear, openhash, or acph")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadoperand tests_jump_table_bad_operand "jump table instruction requires a \\.jtable name operand")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadnumerickey tests_jump_table_bad_numeric_key "jumpn case key must be a non-NaN numeric string")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadnumericduplicate tests_jump_table_bad_numeric_duplicate "duplicate jump table case key after canonicalization")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadpaddedduplicate tests_jump_table_bad_padded_duplicate "duplicate jump table case key after canonicalization")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadmatchmode tests_jump_table_bad_match_mode "cannot mix exact, padded, and numeric")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadoutsideprocedure tests_jump_table_bad_outside_procedure "Jump tables can only be declared inside a procedure")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadcaseonly tests_jump_table_bad_case_only "unknown jump table")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadprocedurescope tests_jump_table_bad_procedure_scope "unknown jump table")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadjumpbsempty tests_jump_table_bad_jumpbs_empty "jumpbs requires fixed-length non-empty binary keys")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadutf8 tests_jump_table_bad_utf8 "jump table string key is not valid UTF-8")
crexx_add_rxas_assembly_failure_regex(rxasjumptablebadnumericzeroduplicate tests_jump_table_bad_numeric_zero_duplicate "duplicate jump table case key after canonicalization")

# Socket Instruction Tests
add_custom_command(
		OUTPUT tests_socket.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_socket ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_socket
		DEPENDS tests/tests_socket.rxas rxas rxvm rxbvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Socket Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_socket ALL DEPENDS tests_socket.rxbin)

crexx_add_vm_test(NAME rxassockettests RXVM_ARGS tests_socket PASS_REGULAR_EXPRESSION "PASS: Core Socket Instructions")

# Decimal Instruction Tests
add_custom_command(
		OUTPUT tests_decimal.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_decimal ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_decimal
		DEPENDS tests/tests_decimal.rxas rxas rxvm mc_decimal_dynamic db_decimal_dynamic db_decimal_manual
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Decimal Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_decimal ALL DEPENDS tests_decimal.rxbin)

crexx_add_vm_test(NAME rxasdecimaltests RXVM_ARGS tests_decimal)
crexx_add_vm_test(NAME rxasdecimaltests-mc RXVM_ARGS -p rxvm_mc_decimal tests_decimal)
crexx_add_vm_test(NAME rxasdecimaltests-db RXVM_ARGS -p rxvm_db_decimal tests_decimal)

# Conversion and Formatting Instruction Tests
add_custom_command(
		OUTPUT tests_conversion.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_conversion ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_conversion
		DEPENDS tests/tests_conversion.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Conversion Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_conversion ALL DEPENDS tests_conversion.rxbin)

crexx_add_vm_test(NAME rxasconversiontests RXVM_ARGS tests_conversion)

# Object and Attribute Instruction Tests
add_custom_command(
		OUTPUT tests_objects.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_objects ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_objects
		DEPENDS tests/tests_objects.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Object Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_objects ALL DEPENDS tests_objects.rxbin)

crexx_add_vm_test(NAME rxasobjecttests RXVM_ARGS tests_objects)

# Reflection and Introspection Instruction Tests
add_custom_command(
		OUTPUT tests_reflection.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_reflection ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_reflection
		DEPENDS tests/tests_reflection.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Reflection Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_reflection ALL DEPENDS tests_reflection.rxbin)

crexx_add_vm_test(NAME rxasreflectiontests RXVM_ARGS tests_reflection)

# Interface dispatch instruction tests
add_custom_command(
		OUTPUT tests_interfaces.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_interfaces ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_interfaces
		DEPENDS tests/tests_interfaces.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Interface Dispatch Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_interfaces ALL DEPENDS tests_interfaces.rxbin)

crexx_add_vm_test(NAME rxasinterfacetests RXVM_ARGS tests_interfaces PASS_REGULAR_EXPRESSION "PASS: Interface Dispatch Suite")
crexx_add_rxas_vm_binary(tests_selector_descriptors "descriptor selector contracts")
crexx_add_vm_test(NAME rxasselectordescriptortests RXVM_ARGS tests_selector_descriptors PASS_REGULAR_EXPRESSION "PASS: Descriptor Selector Contracts")
crexx_add_rxas_vm_failure_regex(rxasselectordescriptorbadmethod tests_selector_descriptor_bad_method "rxsig1\\|describe\\|\\.string\\|")
crexx_add_rxas_vm_failure_regex(rxasselectordescriptorbadfactory tests_selector_descriptor_bad_factory "selector_bad_factory\\.shape")

# Stress and Edge Case Instruction Tests
add_custom_command(
		OUTPUT tests_stress.rxbin
		COMMAND $<TARGET_FILE:rxas> -o tests_stress ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests_stress
		DEPENDS tests/tests_stress.rxas rxas rxvm mc_decimal_dynamic
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "cREXX RXAS Stress Instruction Tests ..."
)

# Create a custom target that depends on the custom command's output
add_custom_target(run_tests_stress ALL DEPENDS tests_stress.rxbin)

crexx_add_vm_test(NAME rxasstresstests RXVM_ARGS tests_stress)

# Focused deterministic coverage for remaining RXAS instruction families
crexx_add_rxas_vm_binary(tests_instruction_gaps "remaining deterministic instruction gaps")
crexx_add_vm_test(NAME rxasinstructiongaptests RXVM_ARGS tests_instruction_gaps PASS_REGULAR_EXPRESSION "PASS: RXAS Instruction Gap Coverage")
crexx_add_rxas_vm_binary(tests_wide_operands "arbitrary operand pipeline")
crexx_add_vm_test(NAME rxaswideoperandtests RXVM_ARGS tests_wide_operands PASS_REGULAR_EXPRESSION "PASS: Arbitrary Operand Pipeline")

# Re-entrancy Test
add_custom_command(
        OUTPUT test_reentrancy.rxbin
        COMMAND $<TARGET_FILE:rxc> -o test_reentrancy ${CMAKE_CURRENT_SOURCE_DIR}/tests/rexx/test_reentrancy.crexx
        COMMAND $<TARGET_FILE:rxas> test_reentrancy
        DEPENDS tests/rexx/test_reentrancy.crexx rxc rxas compiler_exit_bin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling test_reentrancy.crexx ..."
)
add_custom_target(test_reentrancy_bin ALL DEPENDS test_reentrancy.rxbin)

add_executable(test_reentrancy tests/test_reentrancy.c)
target_include_directories(test_reentrancy PRIVATE ${CMAKE_SOURCE_DIR}/inc)
target_link_libraries(test_reentrancy rxvml)
add_dependencies(test_reentrancy test_reentrancy_bin)

add_test(NAME reentrancy_check COMMAND test_reentrancy WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# RCC-3: declarative module initializers. Compile the source surface, preserve
# per-module metadata through rxlink, and retain an RXAS-only cycle fixture so
# cycle rejection does not depend on source-level circular imports.
add_custom_command(
        OUTPUT test_initializer_dependency.rxbin
               test_initializer_root.rxbin
               test_initializer_late.rxbin
               test_initializer_failure.rxbin
               test_initializers_linked.rxbin
               test_initializer_cycle_a.rxbin
               test_initializer_cycle_b.rxbin
        COMMAND $<TARGET_FILE:rxc> -o test_initializer_dependency
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/rexx/test_initializer_dependency.crexx
        COMMAND $<TARGET_FILE:rxas> test_initializer_dependency
        COMMAND $<TARGET_FILE:rxc> -o test_initializer_root
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/rexx/test_initializer_root.crexx
        COMMAND $<TARGET_FILE:rxas> test_initializer_root
        COMMAND $<TARGET_FILE:rxc> -o test_initializer_late
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/rexx/test_initializer_late.crexx
        COMMAND $<TARGET_FILE:rxas> test_initializer_late
        COMMAND $<TARGET_FILE:rxc> -o test_initializer_failure
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/rexx/test_initializer_failure.crexx
        COMMAND $<TARGET_FILE:rxas> test_initializer_failure
        COMMAND $<TARGET_FILE:rxlink> -o test_initializers_linked
                test_initializer_root test_initializer_dependency
        COMMAND $<TARGET_FILE:rxas> -o test_initializer_cycle_a
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_initializer_cycle_a
        COMMAND $<TARGET_FILE:rxas> -o test_initializer_cycle_b
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_initializer_cycle_b
        DEPENDS tests/rexx/test_initializer_dependency.crexx
                tests/rexx/test_initializer_root.crexx
                tests/rexx/test_initializer_late.crexx
                tests/rexx/test_initializer_failure.crexx
                tests/test_initializer_cycle_a.rxas
                tests/test_initializer_cycle_b.rxas
                rxc rxas rxlink compiler_exit_bin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling RCC-3 module initializer fixtures ..."
)
add_custom_target(test_module_initializer_bins ALL
        DEPENDS test_initializer_dependency.rxbin
                test_initializer_root.rxbin
                test_initializer_late.rxbin
                test_initializer_failure.rxbin
                test_initializers_linked.rxbin
                test_initializer_cycle_a.rxbin
                test_initializer_cycle_b.rxbin)

function(crexx_configure_module_initializer_test TARGET_NAME)
    target_include_directories(${TARGET_NAME} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/rxvmplugin
            ${CMAKE_SOURCE_DIR}/inc
            ${CMAKE_SOURCE_DIR}/assembler
            ${CMAKE_SOURCE_DIR}/binutils/include
            ${CMAKE_SOURCE_DIR}/rxpa
            ${CMAKE_SOURCE_DIR}/platform
            ${CMAKE_SOURCE_DIR}/avl_tree
            ${CMAKE_SOURCE_DIR}/utf8)
    add_dependencies(${TARGET_NAME} test_module_initializer_bins)
    add_test(
            NAME ${TARGET_NAME}
            COMMAND ${TARGET_NAME}
                    ${CMAKE_CURRENT_BINARY_DIR}/test_initializer_root.rxbin
                    ${CMAKE_CURRENT_BINARY_DIR}/test_initializer_dependency.rxbin
                    ${CMAKE_CURRENT_BINARY_DIR}/test_initializers_linked.rxbin
                    ${CMAKE_CURRENT_BINARY_DIR}/test_initializer_late.rxbin
                    ${CMAKE_CURRENT_BINARY_DIR}/test_initializer_cycle_a.rxbin
                    ${CMAKE_CURRENT_BINARY_DIR}/test_initializer_cycle_b.rxbin
                    ${CMAKE_CURRENT_BINARY_DIR}/test_initializer_failure.rxbin
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${TARGET_NAME} PROPERTIES
            LABELS "interpreter;rcc-3;initializers")
endfunction()

add_executable(module_initializers-rxvml tests/test_module_initializers.c)
target_link_libraries(module_initializers-rxvml PRIVATE rxvml)
crexx_configure_module_initializer_test(module_initializers-rxvml)

add_executable(module_initializers-rxbvml tests/test_module_initializers.c)
target_link_libraries(module_initializers-rxbvml PRIVATE rxbvml)
target_compile_definitions(module_initializers-rxbvml PRIVATE NTHREADED=1)
crexx_configure_module_initializer_test(module_initializers-rxbvml)

if(CREXX_THREADED_VM_SUPPORTED)
    add_executable(module_initializers-rxtvml
            tests/test_module_initializers.c
            rxvml.c
            $<TARGET_OBJECTS:rxtvm_core_objects>)
    target_link_libraries(module_initializers-rxtvml
            rxvmplugin mc_decimal_manual avl_tree rxpa rxvmref rxbin)
    crexx_enable_vm_platform(module_initializers-rxtvml)
    if(CREXX_VM_PROFILING)
        target_compile_definitions(module_initializers-rxtvml PRIVATE
                CREXX_VM_PROFILING=1)
    endif()
    crexx_configure_module_initializer_test(module_initializers-rxtvml)
endif()

# PERF3-13 E4a/E4b: retain independent loads as the correctness control, then
# prove runtime-owned immutable generations with worker-owned overlays.
add_custom_command(
        OUTPUT test_program_generation_control.rxbin
               test_program_generation_late.rxbin
        COMMAND $<TARGET_FILE:rxc> -o test_program_generation_control
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/rexx/test_program_generation_control.crexx
        COMMAND $<TARGET_FILE:rxas> test_program_generation_control
        COMMAND $<TARGET_FILE:rxc> -o test_program_generation_late
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/rexx/test_program_generation_late.crexx
        COMMAND $<TARGET_FILE:rxas> test_program_generation_late
        DEPENDS tests/rexx/test_program_generation_control.crexx
                tests/rexx/test_program_generation_late.crexx
                rxc rxas compiler_exit_bin
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Compiling PERF3-13 E4 program-generation controls ..."
)
add_custom_target(test_program_generation_control_bins ALL
        DEPENDS test_program_generation_control.rxbin
                test_program_generation_late.rxbin)

function(crexx_configure_program_generation_control TARGET_NAME)
    target_include_directories(${TARGET_NAME} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/rxvmplugin
            ${CMAKE_SOURCE_DIR}/inc
            ${CMAKE_SOURCE_DIR}/assembler
            ${CMAKE_SOURCE_DIR}/binutils/include
            ${CMAKE_SOURCE_DIR}/rxpa
            ${CMAKE_SOURCE_DIR}/platform
            ${CMAKE_SOURCE_DIR}/avl_tree
            ${CMAKE_SOURCE_DIR}/utf8)
    add_dependencies(${TARGET_NAME} test_program_generation_control_bins)
    add_test(
            NAME ${TARGET_NAME}
            COMMAND ${TARGET_NAME}
                    ${CMAKE_CURRENT_BINARY_DIR}/test_program_generation_control.rxbin
                    ${CMAKE_CURRENT_BINARY_DIR}/test_program_generation_late.rxbin
                    ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}-archive.rxbin
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${TARGET_NAME} PROPERTIES
            LABELS "interpreter;perf3;gate-e;e4a;e4b")
    crexx_label_concurrency_tests(SP01 ${TARGET_NAME})
endfunction()

add_executable(program_generation_control-rxvml
        tests/test_program_generation_control.c)
target_link_libraries(program_generation_control-rxvml PRIVATE rxvml)
crexx_configure_program_generation_control(program_generation_control-rxvml)

add_executable(program_generation_control-rxbvml
        tests/test_program_generation_control.c)
target_link_libraries(program_generation_control-rxbvml PRIVATE rxbvml)
target_compile_definitions(program_generation_control-rxbvml PRIVATE NTHREADED=1)
crexx_configure_program_generation_control(program_generation_control-rxbvml)

# The compiler-selected rxvml may itself use switch dispatch. Build a test-only
# direct-threaded RXVML executable where supported so E4 always covers both
# concrete execution-image forms without adding another product library.
if(CREXX_THREADED_VM_SUPPORTED)
    add_executable(program_generation_control-rxtvml
            tests/test_program_generation_control.c
            rxvml.c
            $<TARGET_OBJECTS:rxtvm_core_objects>)
    target_link_libraries(program_generation_control-rxtvml
            rxvmplugin mc_decimal_manual avl_tree rxpa rxvmref rxbin)
    crexx_enable_vm_platform(program_generation_control-rxtvml)
    if(CREXX_VM_PROFILING)
        target_compile_definitions(program_generation_control-rxtvml PRIVATE
                CREXX_VM_PROFILING=1)
    endif()
    crexx_configure_program_generation_control(program_generation_control-rxtvml)
endif()

# PERF3-13 E5 private industrial executor. It uses the native
# POSIX carrier on Apple/Linux. Windows resolves QueueUserAPC2 dynamically and
# uses a special user APC when the runtime probe succeeds; when it cannot, the
# worker immutably selects the duplicate sparse compatibility owner before
# preparation. Native-capable and non-targetable execution retain the ordinary
# E4 owner. This is test-only and introduces no public worker, channel or RXVML
# ABI.
if(BUILD_TESTING AND
   (APPLE OR WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Linux"))
    include(${CMAKE_SOURCE_DIR}/rxpa/RXPluginFunction.cmake)
    add_dynamic_plugin_target(_e5native
            tests/e5_native_return_plugin.c)
    add_static_plugin_target(_e5native
            tests/e5_native_return_plugin.c)
    if(APPLE)
        set(_crexx_e5_doorbell_platform_label macos)
        set(_crexx_e5_doorbell_backend posix)
    elseif(WIN32)
        set(_crexx_e5_doorbell_platform_label windows)
        set(_crexx_e5_doorbell_backend windows-special-apc)
    else()
        set(_crexx_e5_doorbell_platform_label linux)
        set(_crexx_e5_doorbell_backend posix)
    endif()
    set(CREXX_E5_TEST_RXBIN "" CACHE FILEPATH
            "Qualified E5 worker RXBIN to reuse across Windows compiler lanes")
    if(CREXX_E5_TEST_RXBIN)
        add_custom_command(
                OUTPUT test_persistent_worker_executor.rxbin
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${CREXX_E5_TEST_RXBIN}
                        ${CMAKE_CURRENT_BINARY_DIR}/test_persistent_worker_executor.rxbin
                DEPENDS ${CREXX_E5_TEST_RXBIN}
                COMMENT "Copying qualified PERF3-13 E5 RXBIN fixture ...")
    else()
        add_custom_command(
                OUTPUT test_persistent_worker_executor.rxbin
                COMMAND $<TARGET_FILE:rxc> -i ${CMAKE_BINARY_DIR}/bin
                        -o test_persistent_worker_executor
                        ${CMAKE_CURRENT_SOURCE_DIR}/tests/rexx/test_persistent_worker_executor.crexx
                COMMAND $<TARGET_FILE:rxas> test_persistent_worker_executor
                DEPENDS tests/rexx/test_persistent_worker_executor.crexx
                        rxc rxas compiler_exit_bin _e5native
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Compiling PERF3-13 E5 native-doorbell control ...")
    endif()
    add_custom_target(test_persistent_worker_executor_bin ALL
            DEPENDS test_persistent_worker_executor.rxbin)

    set(CREXX_E5_SPARSE_TEST_RXBIN "" CACHE FILEPATH
            "Qualified E5 sparse-progress RXBIN to reuse across Windows compiler lanes")
    if(CREXX_E5_SPARSE_TEST_RXBIN)
        add_custom_command(
                OUTPUT test_persistent_worker_sparse.rxbin
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${CREXX_E5_SPARSE_TEST_RXBIN}
                        ${CMAKE_CURRENT_BINARY_DIR}/test_persistent_worker_sparse.rxbin
                DEPENDS ${CREXX_E5_SPARSE_TEST_RXBIN}
                COMMENT "Copying qualified PERF3-13 E5 sparse-progress RXBIN ...")
    else()
        add_custom_command(
                OUTPUT test_persistent_worker_sparse.rxbin
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_persistent_worker_sparse.rxas
                        ${CMAKE_CURRENT_BINARY_DIR}/test_persistent_worker_sparse.rxas
                COMMAND $<TARGET_FILE:rxas> test_persistent_worker_sparse
                DEPENDS tests/test_persistent_worker_sparse.rxas rxas
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Assembling PERF3-13 E5 sparse progress fixture ...")
    endif()
    add_custom_target(test_persistent_worker_sparse_bin ALL
            DEPENDS test_persistent_worker_sparse.rxbin)

    function(crexx_configure_persistent_worker_executor TARGET_NAME)
        target_include_directories(${TARGET_NAME} PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/rxvmplugin
                ${CMAKE_SOURCE_DIR}/inc
                ${CMAKE_SOURCE_DIR}/assembler
                ${CMAKE_SOURCE_DIR}/binutils/include
                ${CMAKE_SOURCE_DIR}/rxpa
                ${CMAKE_SOURCE_DIR}/platform
                ${CMAKE_SOURCE_DIR}/avl_tree
                ${CMAKE_SOURCE_DIR}/utf8)
        target_link_libraries(${TARGET_NAME} Threads::Threads)
        configure_linker_for_static_lib(${TARGET_NAME} _e5native)
        target_link_libraries(${TARGET_NAME} rxpashim)
        add_dependencies(${TARGET_NAME} _e5native_static)
        add_dependencies(${TARGET_NAME}
                test_persistent_worker_executor_bin
                test_persistent_worker_sparse_bin)
        add_test(
                NAME ${TARGET_NAME}
                COMMAND ${CMAKE_COMMAND} -E env
                        CREXX_VM_E5_DOORBELL=${_crexx_e5_doorbell_backend}
                        $<TARGET_FILE:${TARGET_NAME}>
                        ${CMAKE_CURRENT_BINARY_DIR}/test_persistent_worker_executor.rxbin
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        set_tests_properties(${TARGET_NAME} PROPERTIES
                LABELS "interpreter;perf3;gate-e;e5;private;industrial;${_crexx_e5_doorbell_platform_label}"
                TIMEOUT 120)
        crexx_label_concurrency_tests(SP01 ${TARGET_NAME})
        add_test(
                NAME ${TARGET_NAME}-doorbell-stress
                COMMAND ${CMAKE_COMMAND} -E env
                        CREXX_VM_E5_DOORBELL=${_crexx_e5_doorbell_backend}
                        $<TARGET_FILE:${TARGET_NAME}>
                        --doorbell-stress
                        ${CMAKE_CURRENT_BINARY_DIR}/test_persistent_worker_executor.rxbin
                        100)
        set_tests_properties(${TARGET_NAME}-doorbell-stress PROPERTIES
                LABELS "interpreter;perf3;gate-e;e5;private;industrial;race;lifecycle;${_crexx_e5_doorbell_platform_label}"
                TIMEOUT 120)
        crexx_label_concurrency_stress_tests(SP01
                ${TARGET_NAME}-doorbell-stress)
        crexx_label_concurrency_tests(SP08
                ${TARGET_NAME}-doorbell-stress)
        add_test(
                NAME ${TARGET_NAME}-sparse-owner
                COMMAND ${CMAKE_COMMAND} -E env
                        CREXX_VM_E5_DOORBELL=${_crexx_e5_doorbell_backend}
                        CREXX_VM_E5_FORCE_SPARSE_OWNER=1
                        $<TARGET_FILE:${TARGET_NAME}>
                        ${CMAKE_CURRENT_BINARY_DIR}/test_persistent_worker_executor.rxbin
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        set_tests_properties(${TARGET_NAME}-sparse-owner PROPERTIES
                LABELS "interpreter;perf3;gate-e;e5;private;industrial;fallback;sparse;${_crexx_e5_doorbell_platform_label}"
                TIMEOUT 120)
        crexx_label_concurrency_tests(SP01 ${TARGET_NAME}-sparse-owner)
        add_test(
                NAME ${TARGET_NAME}-sparse-owner-stress
                COMMAND ${CMAKE_COMMAND} -E env
                        CREXX_VM_E5_DOORBELL=${_crexx_e5_doorbell_backend}
                        CREXX_VM_E5_FORCE_SPARSE_OWNER=1
                        $<TARGET_FILE:${TARGET_NAME}>
                        --doorbell-stress
                        ${CMAKE_CURRENT_BINARY_DIR}/test_persistent_worker_executor.rxbin
                        100)
        set_tests_properties(${TARGET_NAME}-sparse-owner-stress PROPERTIES
                LABELS "interpreter;perf3;gate-e;e5;private;industrial;fallback;sparse;race;lifecycle;${_crexx_e5_doorbell_platform_label}"
                TIMEOUT 120)
        crexx_label_concurrency_stress_tests(SP01
                ${TARGET_NAME}-sparse-owner-stress)
        crexx_label_concurrency_tests(SP08
                ${TARGET_NAME}-sparse-owner-stress)
        add_test(
                NAME ${TARGET_NAME}-sparse-progress
                COMMAND ${CMAKE_COMMAND} -E env
                        CREXX_VM_E5_DOORBELL=${_crexx_e5_doorbell_backend}
                        CREXX_VM_E5_FORCE_SPARSE_OWNER=1
                        $<TARGET_FILE:${TARGET_NAME}>
                        --sparse-progress
                        ${CMAKE_CURRENT_BINARY_DIR}/test_persistent_worker_sparse.rxbin)
        set_tests_properties(${TARGET_NAME}-sparse-progress PROPERTIES
                LABELS "interpreter;perf3;gate-e;e5;private;industrial;fallback;sparse;${_crexx_e5_doorbell_platform_label}"
                TIMEOUT 30)
        crexx_label_concurrency_tests(SP01 ${TARGET_NAME}-sparse-progress)
        crexx_label_concurrency_tests(SP08 ${TARGET_NAME}-sparse-progress)
        add_test(
                NAME ${TARGET_NAME}-native-return
                COMMAND ${CMAKE_COMMAND} -E env
                        CREXX_VM_E5_DOORBELL=${_crexx_e5_doorbell_backend}
                        CREXX_VM_E5_FORCE_SPARSE_OWNER=1
                        $<TARGET_FILE:${TARGET_NAME}>
                        --native-return
                        ${CMAKE_CURRENT_BINARY_DIR}/test_persistent_worker_executor.rxbin)
        set_tests_properties(${TARGET_NAME}-native-return PROPERTIES
                LABELS "interpreter;perf3;gate-e;e5;private;industrial;fallback;sparse;native;lifecycle;${_crexx_e5_doorbell_platform_label}"
                TIMEOUT 30)
        crexx_label_concurrency_tests(SP01 ${TARGET_NAME}-native-return)
        crexx_label_concurrency_tests(SP08 ${TARGET_NAME}-native-return)
        add_test(
                NAME ${TARGET_NAME}-e6-worker-topology-w8
                COMMAND ${CMAKE_COMMAND} -E env
                        CREXX_VM_E5_DOORBELL=${_crexx_e5_doorbell_backend}
                        $<TARGET_FILE:${TARGET_NAME}>
                        --worker-topology
                        ${CMAKE_CURRENT_BINARY_DIR}/test_persistent_worker_executor.rxbin
                        8)
        set_tests_properties(
                ${TARGET_NAME}-e6-worker-topology-w8
                PROPERTIES
                LABELS "interpreter;gate-e;e6;private;worker;topology;${_crexx_e5_doorbell_platform_label}"
                PROCESSORS 8
                TIMEOUT 30)
        crexx_label_concurrency_tests(
                SP01 ${TARGET_NAME}-e6-worker-topology-w8)
        crexx_label_concurrency_tests(
                SP08 ${TARGET_NAME}-e6-worker-topology-w8)
        foreach(_crexx_e6_workers IN ITEMS 1 2 4 8)
            foreach(_crexx_e6_workload IN ITEMS spin churn)
                if("${_crexx_e6_workload}" STREQUAL "spin")
                    set(_crexx_e6_iterations 65536)
                else()
                    set(_crexx_e6_iterations 8192)
                endif()
                add_test(
                        NAME ${TARGET_NAME}-e6-scale-${_crexx_e6_workload}-w${_crexx_e6_workers}
                        COMMAND ${CMAKE_COMMAND} -E env
                                CREXX_VM_E5_DOORBELL=${_crexx_e5_doorbell_backend}
                                $<TARGET_FILE:${TARGET_NAME}>
                                --benchmark executor${_crexx_e6_workers}
                                ${CMAKE_CURRENT_BINARY_DIR}/test_persistent_worker_executor.rxbin
                                256 ${_crexx_e6_iterations}
                                ${_crexx_e6_workload})
                set_tests_properties(
                        ${TARGET_NAME}-e6-scale-${_crexx_e6_workload}-w${_crexx_e6_workers}
                        PROPERTIES
                        LABELS "interpreter;perf3;performance-measurement;gate-e;e6;private;scale;${_crexx_e6_workload};${_crexx_e5_doorbell_platform_label}"
                        PROCESSORS ${_crexx_e6_workers}
                        # These are timing-sensitive performance measurements.
                        # Running multiple cells beside compiler-heavy tests makes
                        # their timeout and recorded wall time meaningless,
                        # especially under sanitizer instrumentation.
                        RUN_SERIAL TRUE
                        TIMEOUT 120)
            endforeach()
        endforeach()
        if(WIN32)
            add_test(
                    NAME ${TARGET_NAME}-doorbell-fallback
                    COMMAND ${CMAKE_COMMAND} -E env
                            CREXX_VM_E5_DOORBELL=${_crexx_e5_doorbell_backend}
                            CREXX_VM_E5_WINDOWS_APC_FORCE_UNAVAILABLE=1
                            $<TARGET_FILE:${TARGET_NAME}>
                            --doorbell-fallback
                            ${CMAKE_CURRENT_BINARY_DIR}/test_persistent_worker_executor.rxbin)
            set_tests_properties(
                    ${TARGET_NAME}-doorbell-fallback PROPERTIES
                    LABELS "interpreter;perf3;gate-e;e5;private;industrial;fallback;windows"
                    TIMEOUT 30)
            crexx_label_concurrency_tests(SP01
                    ${TARGET_NAME}-doorbell-fallback)
            add_test(
                    NAME ${TARGET_NAME}-fallback
                    COMMAND ${CMAKE_COMMAND} -E env
                            CREXX_VM_E5_DOORBELL=${_crexx_e5_doorbell_backend}
                            CREXX_VM_E5_WINDOWS_APC_FORCE_UNAVAILABLE=1
                            $<TARGET_FILE:${TARGET_NAME}>
                            ${CMAKE_CURRENT_BINARY_DIR}/test_persistent_worker_executor.rxbin
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
            set_tests_properties(${TARGET_NAME}-fallback PROPERTIES
                    LABELS "interpreter;perf3;gate-e;e5;private;industrial;fallback;windows"
                    TIMEOUT 120)
            crexx_label_concurrency_tests(SP01 ${TARGET_NAME}-fallback)
            add_test(
                    NAME ${TARGET_NAME}-fallback-stress
                    COMMAND ${CMAKE_COMMAND} -E env
                            CREXX_VM_E5_DOORBELL=${_crexx_e5_doorbell_backend}
                            CREXX_VM_E5_WINDOWS_APC_FORCE_UNAVAILABLE=1
                            $<TARGET_FILE:${TARGET_NAME}>
                            --doorbell-stress
                            ${CMAKE_CURRENT_BINARY_DIR}/test_persistent_worker_executor.rxbin
                            100)
            set_tests_properties(${TARGET_NAME}-fallback-stress PROPERTIES
                    LABELS "interpreter;perf3;gate-e;e5;private;industrial;fallback;race;lifecycle;windows"
                    TIMEOUT 120)
            crexx_label_concurrency_stress_tests(SP01
                    ${TARGET_NAME}-fallback-stress)
            crexx_label_concurrency_tests(SP08
                    ${TARGET_NAME}-fallback-stress)
            add_test(
                    NAME ${TARGET_NAME}-sparse-progress-fallback
                    COMMAND ${CMAKE_COMMAND} -E env
                            CREXX_VM_E5_DOORBELL=${_crexx_e5_doorbell_backend}
                            CREXX_VM_E5_WINDOWS_APC_FORCE_UNAVAILABLE=1
                            $<TARGET_FILE:${TARGET_NAME}>
                            --sparse-progress
                            ${CMAKE_CURRENT_BINARY_DIR}/test_persistent_worker_sparse.rxbin)
            set_tests_properties(
                    ${TARGET_NAME}-sparse-progress-fallback PROPERTIES
                    LABELS "interpreter;perf3;gate-e;e5;private;industrial;fallback;sparse;windows"
                    TIMEOUT 30)
            crexx_label_concurrency_tests(SP01
                    ${TARGET_NAME}-sparse-progress-fallback)
            crexx_label_concurrency_tests(SP08
                    ${TARGET_NAME}-sparse-progress-fallback)
        endif()
    endfunction()

    add_executable(persistent_worker_executor-rxvml
            tests/test_persistent_worker_executor.c)
    target_link_libraries(persistent_worker_executor-rxvml rxvml)
    crexx_configure_persistent_worker_executor(persistent_worker_executor-rxvml)

    add_executable(persistent_worker_executor-rxbvml
            tests/test_persistent_worker_executor.c)
    target_link_libraries(persistent_worker_executor-rxbvml rxbvml)
    target_compile_definitions(persistent_worker_executor-rxbvml PRIVATE NTHREADED=1)
    crexx_configure_persistent_worker_executor(persistent_worker_executor-rxbvml)

    if(CREXX_THREADED_VM_SUPPORTED)
        add_executable(persistent_worker_executor-rxtvml
                tests/test_persistent_worker_executor.c
                rxvml.c
                $<TARGET_OBJECTS:rxtvm_core_objects>)
        target_link_libraries(persistent_worker_executor-rxtvml
                rxvmplugin mc_decimal_manual avl_tree rxpa rxvmref rxbin)
        crexx_enable_vm_platform(persistent_worker_executor-rxtvml)
        if(CREXX_VM_PROFILING)
            target_compile_definitions(persistent_worker_executor-rxtvml PRIVATE
                    CREXX_VM_PROFILING=1)
        endif()
        crexx_configure_persistent_worker_executor(persistent_worker_executor-rxtvml)
    endif()
endif()

# Archive-link regression test for manual db_decimal packaging.
if(MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
    add_executable(test_db_decimal_archive_link
            tests/test_db_decimal_archive_link.c)
    target_include_directories(test_db_decimal_archive_link PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/rxvmplugin
            ${CMAKE_SOURCE_DIR}/inc)
    target_link_libraries(test_db_decimal_archive_link
            rxvml
            rxpashim
            rxvmplugin
            rxvmref
            rxvmmemory
            rxbin
            platform
            decnumber
            avl_tree
            rxpa
            db_decimal_manual
            ${CREXX_SOCKET_TARGET_LIBS})
    add_custom_target(test_db_decimal_archive_link_bin ALL
            DEPENDS test_db_decimal_archive_link)
    add_test(NAME db_decimal_archive_link
            COMMAND $<TARGET_FILE:test_db_decimal_archive_link>
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
else()
    set(_crexx_archive_link_c_flags_text "${CMAKE_C_FLAGS}")
    set(_crexx_archive_link_exe_flags_text "${CMAKE_EXE_LINKER_FLAGS}")
    if(CMAKE_BUILD_TYPE)
        string(TOUPPER "${CMAKE_BUILD_TYPE}" _crexx_archive_link_build_type)
        string(APPEND _crexx_archive_link_c_flags_text " ${CMAKE_C_FLAGS_${_crexx_archive_link_build_type}}")
        string(APPEND _crexx_archive_link_exe_flags_text " ${CMAKE_EXE_LINKER_FLAGS_${_crexx_archive_link_build_type}}")
    endif()
    separate_arguments(_crexx_archive_link_c_flags NATIVE_COMMAND "${_crexx_archive_link_c_flags_text}")
    separate_arguments(_crexx_archive_link_exe_flags NATIVE_COMMAND "${_crexx_archive_link_exe_flags_text}")
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_db_decimal_archive_link
            COMMAND ${CMAKE_C_COMPILER}
                    ${_crexx_archive_link_c_flags}
                    -O0
                    -I${CMAKE_CURRENT_SOURCE_DIR}
                    -I${CMAKE_CURRENT_SOURCE_DIR}/rxvmplugin
                    -I${CMAKE_SOURCE_DIR}/inc
                    -o ${CMAKE_CURRENT_BINARY_DIR}/test_db_decimal_archive_link
                    ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_db_decimal_archive_link.c
                    $<TARGET_FILE:rxvml>
                    $<TARGET_FILE:rxpashim>
                    $<TARGET_FILE:rxvmplugin>
                    $<TARGET_FILE:rxvmref>
                    $<TARGET_FILE:rxvmmemory>
                    $<TARGET_FILE:rxbin>
                    $<TARGET_FILE:platform>
                    $<TARGET_FILE:decnumber>
                    $<TARGET_FILE:avl_tree>
                    $<TARGET_FILE:rxpa>
                    ${_crexx_archive_link_exe_flags}
                    -lm
                    $<TARGET_FILE:db_decimal_manual>
                    ${CREXX_SOCKET_MANUAL_LINK_LIBS}
            DEPENDS
                    tests/test_db_decimal_archive_link.c
                    rxvml
                    rxpashim
                    rxvmplugin
                    rxvmref
                    rxvmmemory
                    rxbin
                    platform
                    decnumber
                    avl_tree
                    rxpa
                    db_decimal_manual
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Linking archive-level db_decimal regression harness ..."
            VERBATIM
    )
    add_custom_target(test_db_decimal_archive_link_bin ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/test_db_decimal_archive_link)
    add_test(NAME db_decimal_archive_link COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_db_decimal_archive_link WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

crexx_label_smoke_tests(
        crexxsaa_cache_tool_location
        rxpa_utf_validation
        ts_regvalue_tester
        rxassignalstests
        rxasregisterflagtests
        rxasutfflagtests
        rxasreferencetests
        rxasbasictests
        rxasstringtests
        rxasdecimaltests-mc
        rxasinterfacetests
        compact_format_check
        reentrancy_check
        db_decimal_archive_link
)
