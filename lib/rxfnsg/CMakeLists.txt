cmake_minimum_required(VERSION 3.24)

add_subdirectory(rexx)
add_subdirectory(tests_functional)

if(BUILD_TESTING)
  # Non-owning preparation boundary for the designed Phase 3 sanitizer slice.
  # CTest remains a separate tools/asan-run.sh phase and never builds the
  # active tree from inside its worker pool.
  add_custom_target(phase3-level-g-sanitizer-artifacts)
  add_dependencies(phase3-level-g-sanitizer-artifacts
    rxfnsg
    rxdas
    test_signal_block_levelg_bin
    test_address_after_signal_block_bin
    test_address_parallel_spawn_bin
    ts_unicode_casefold
    ts_unicode_case_mapping
    ts_unicode_grapheme
    ts_unicode_codec
    ts_unicode_normalization)
endif()
crexx_install_bytecode(rxfnsg "${CMAKE_BINARY_DIR}/bin/rxfnsg.rxbin")
