# Build Dummy.rxbin
crexx_add_compiler_exit_component(exit_dummy_bin Dummy)

crexx_add_compiler_exit_runtime_test(
        test_dummy
        test_dummy.crexx
        PASS_REGULAR_EXPRESSION "after-exit 99"
)
