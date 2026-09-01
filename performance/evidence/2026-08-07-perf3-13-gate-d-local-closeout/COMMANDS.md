# Commands

All commands ran from
`/private/tmp/crexx-rxvm-gated.6HZvFK/source` on the reserved Mac host.

## Focused correctness

```sh
ctest --test-dir cmake-build-debug --output-on-failure -R \
  '^(rxvmstem_storage|rxvmstem_allocator_family|rxvmmemory_allocator|ts_regvalue_tester|mc_decimal_test1|mc_decimal_test2|mc_decimal_test3|mc_decimal_dyn_test1|mc_decimal_manual_test1|mc_decimal_full_tests|db_decimal_tests|db_decimal_archive_link|testRexxClassicBifDatatype_noopt|testRexxClassicBifDatatype_opt)$'

ctest --test-dir cmake-build-release --output-on-failure -R \
  '^(rxvmstem_storage|rxvmstem_allocator_family|rxvmmemory_allocator|ts_regvalue_tester|mc_decimal_test1|mc_decimal_test2|mc_decimal_test3|mc_decimal_dyn_test1|mc_decimal_manual_test1|mc_decimal_full_tests|db_decimal_tests|db_decimal_archive_link|testRexxClassicBifDatatype_noopt|testRexxClassicBifDatatype_opt)$'
```

The no-opt `datatype` image was also run directly with both Debug `rxbvm` and
`rxtvm`.

## Broad Debug classification

```sh
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
ctest --test-dir cmake-build-debug --rerun-failed --parallel 1 \
  --output-on-failure
```

## AddressSanitizer

The tree was configured with Apple Clang `-fsanitize=address` and
`-fno-omit-frame-pointer`. All build and test actions then used the documented
runner:

```sh
tools/asan-run.sh --build-dir cmake-build-debugasan --phase build \
  --build-target rxbvm --build-target rxtvm \
  --build-target test_rxvmmemory --build-target test_rxvmstem \
  --build-target test_rxvmstem_allocator_family \
  --build-target ts_regvalue_tester \
  --build-target mc_decimal_test_harness \
  --build-target mc_decimal_dynamic \
  --build-target mc_decimal_dyn_test_harness \
  --build-target mc_decimal_manual_test_harness \
  --build-target mc_decimal_full_tests \
  --build-target db_decimal_dynamic --build-target db_decimal_manual \
  --build-target db_decimal_tests \
  --build-target test_db_decimal_archive_link_bin \
  --build-target testRexxClassicBifDatatype \
  --build-jobs 4 --build-leaks off --no-live-tail

tools/asan-run.sh --build-dir cmake-build-debugasan --phase ctest \
  --regex '^(rxvmstem_storage|rxvmstem_allocator_family|rxvmmemory_allocator|ts_regvalue_tester|mc_decimal_test1|mc_decimal_test2|mc_decimal_test3|mc_decimal_dyn_test1|mc_decimal_manual_test1|mc_decimal_full_tests|db_decimal_tests|db_decimal_archive_link)$' \
  --leaks off --test-jobs 1 --no-live-tail

tools/asan-run.sh --build-dir cmake-build-debugasan --phase ctest \
  --regex '^testRexxClassicBifDatatype_noopt$' \
  --fixture-exclude-setup linked_opt_runtime_artifacts \
  --leaks off --test-jobs 1 --no-live-tail
```

## Performance

Each retained block used:

```sh
/private/tmp/crexx-rxvm-inline.yvLywZ/builds/baseline/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest performance/evidence/2026-08-07-perf3-13-gate-d-first-release-verdict/core-four-manifest.txt \
  --output-dir OUTPUT/timing --measurement timing --warmups 1 --runs 12
```

## Install/product

```sh
cmake --build cmake-build-release --parallel 10
cmake --install cmake-build-release --prefix ISOLATED_PREFIX
ISOLATED_PREFIX/bin/rxvm \
  cmake-build-release/tests/benchmarks/benchmark_awfy_sieve_opt.rxbin \
  ISOLATED_PREFIX/bin/library.rxbin -a 1
```

