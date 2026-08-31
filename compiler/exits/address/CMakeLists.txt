cmake_minimum_required(VERSION 3.24)

set(LIBRPATH ${CMAKE_BINARY_DIR}/bin/library)

crexx_add_compiler_exit_component(exit_address_bin Address)

crexx_add_compiler_exit_runtime_test(
        test_address
        test_address.crexx
        EXTRA_DEPENDS library
        RUN_SUFFIX ${LIBRPATH}
)

crexx_add_compiler_exit_runtime_test(
        test_address_direct
        test_address_direct.crexx
        EXTRA_DEPENDS exit_test_support_bin
        RUN_SUFFIX ${CMAKE_BINARY_DIR}/bin/exittestsupport.rxbin ${CMAKE_BINARY_DIR}/bin/rxcexits.rxbin ${LIBRPATH}
)

crexx_add_compiler_exit_runtime_test(
        test_address_after_signal_block
        test_address_after_signal_block.crexx
        EXTRA_DEPENDS rxc
        RUN_SUFFIX ${LIBRPATH} -a $<TARGET_FILE:rxc> ${CMAKE_CURRENT_BINARY_DIR}/missing-after-signal.rxbin
)

add_executable(address_spawn_group_fixture address_spawn_group_fixture.c)

if(APPLE)
    set(_address_parallel_spawn_repetitions 512)
elseif(WIN32)
    set(_address_parallel_spawn_repetitions 32)
else()
    set(_address_parallel_spawn_repetitions 128)
endif()

# ASan makes each process launch substantially more expensive. Keep the full
# 4,096-launch race stress in ordinary Apple builds, while the sanitizer lane
# exercises the same eight-worker ownership path with 1,024 launches.
set(_address_parallel_spawn_sanitizer_flags
        "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS} ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS}")
if(CMAKE_BUILD_TYPE)
    string(TOUPPER "${CMAKE_BUILD_TYPE}" _address_parallel_spawn_build_type)
    string(APPEND _address_parallel_spawn_sanitizer_flags
            " ${CMAKE_C_FLAGS_${_address_parallel_spawn_build_type}}"
            " ${CMAKE_CXX_FLAGS_${_address_parallel_spawn_build_type}}"
            " ${CMAKE_EXE_LINKER_FLAGS_${_address_parallel_spawn_build_type}}"
            " ${CMAKE_SHARED_LINKER_FLAGS_${_address_parallel_spawn_build_type}}")
endif()
string(FIND "${_address_parallel_spawn_sanitizer_flags}"
        "-fsanitize=address" _address_parallel_spawn_asan_flag)
if(NOT WIN32 AND NOT _address_parallel_spawn_asan_flag EQUAL -1)
    set(_address_parallel_spawn_repetitions 128)
endif()

crexx_add_compiler_exit_runtime_test(
        test_address_parallel_spawn
        test_address_parallel_spawn.crexx
        EXTRA_DEPENDS library classlib address_spawn_group_fixture
        RUN_SUFFIX ${LIBRPATH} ${CMAKE_BINARY_DIR}/bin/classlib -a
                $<TARGET_FILE:address_spawn_group_fixture>
                8
                ${_address_parallel_spawn_repetitions}
        PASS_REGULAR_EXPRESSION
                "SUCCESS: parallel controlled child process groups"
)
set_tests_properties(
        test_address_parallel_spawn_noopt
        test_address_parallel_spawn_opt
        PROPERTIES
                LABELS "stress;interpreter;address;spawn;concurrency"
                RUN_SERIAL TRUE
                TIMEOUT 90
)
