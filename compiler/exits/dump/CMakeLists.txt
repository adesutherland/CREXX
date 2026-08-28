cmake_minimum_required(VERSION 3.24)

# Build Dump.rxbin
# Needs token.rxbin for class metadata.
crexx_add_compiler_exit_component(exit_dump_bin Dump)

crexx_add_compiler_exit_runtime_test(test_dump test_dump.crexx)
