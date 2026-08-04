# C1abc closeout commands

All commands ran from `/Users/adrian/CLionProjects/CREXX`. Verbose output was
redirected to the corresponding retained log.

## Ordinary Release build

```bash
cmake --build cmake-build-release --target rxc rxas rxvm rxbvm --parallel 10
```

## Focused Release correctness

```bash
ctest --test-dir cmake-build-release --parallel 10 --output-on-failure \
  -R '^(rxc_inline_byvalue_arg_reuse|inline_receiver_summary_fallback_binary_opt|inline_receiver_nested_this_direct_placement_opt|inline_receiver_detached_guard_opt|inline_receiver_production_ladder_opt|inline_receiver_reference_alias_opt|benchmark_awfy_towers_noopt|benchmark_awfy_towers_opt|benchmark_awfy_richards_noopt|benchmark_awfy_richards_opt)$'
```

## Debug build and reviewed golden transition

```bash
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```

After reviewing the seven exact optimized-RXAS deltas and refreshing their
goldens, the affected structural/runtime pairs and the new ladder proof ran as:

```bash
ctest --test-dir cmake-build-debug --parallel 15 --output-on-failure \
  -R '^(arg_semantics_object_opt|arg_semantics_object_run_opt|object_reference_regression_opt|object_reference_regression_run_opt|inline_test_object_return_assign_opt|inline_test_object_return_assign_run_opt|inline_test_object_return_expr_opt|inline_test_object_return_expr_run_opt|inline_test_object_const_arg_mutation_opt|inline_test_object_const_arg_mutation_run_opt|inline_test_object_expr_arg_opt|inline_test_object_expr_arg_run_opt|inline_test_class_methods_opt|inline_test_class_methods_run_opt|inline_receiver_production_ladder_opt)$'
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```

## Identity and replay integrity

```bash
shasum -a 256 \
  cmake-build-release/bin/rxc \
  cmake-build-release/bin/rxas \
  cmake-build-release/bin/rxvm \
  cmake-build-release/bin/rxbvm \
  cmake-build-release/tests/benchmarks/benchmark_awfy_richards_opt.rxas \
  cmake-build-release/tests/benchmarks/benchmark_awfy_towers_opt.rxas

(cd performance/evidence/2026-08-01-perf3-02-r1-repanel && \
  shasum -a 256 -c checksums.sha256)
```
