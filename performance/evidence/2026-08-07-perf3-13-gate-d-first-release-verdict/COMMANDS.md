# Commands

The ordinary profiling-off Release product and focused prerequisites were:

```sh
cmake -S . -B cmake-build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release --parallel 10 --target \
  rxbvm rxvm rxc test_rxvmmemory test_rxvmstem \
  test_rxvmstem_allocator_family ts_regvalue_tester \
  mc_decimal_test_harness mc_decimal_dynamic mc_decimal_dyn_test_harness \
  db_decimal_dynamic db_decimal_manual test_db_decimal_archive_link_bin
cmake --build cmake-build-release --parallel 10 --target \
  mc_decimal_manual_test_harness mc_decimal_full_tests db_decimal_tests \
  test_db_decimal_archive_link_bin
ctest --test-dir cmake-build-release --output-on-failure -R \
  '^(rxvmstem_storage|rxvmstem_allocator_family|rxvmmemory_allocator|ts_regvalue_tester|mc_decimal_test1|mc_decimal_test2|mc_decimal_test3|mc_decimal_dyn_test1|mc_decimal_manual_test1|mc_decimal_full_tests|db_decimal_tests|db_decimal_archive_link)$'
```

The decisive panel used the corrected Level B runner:

```sh
/private/tmp/crexx-rxvm-inline.yvLywZ/builds/baseline/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest core-four-manifest.txt --output-dir timing \
  --measurement timing --warmups 1 --runs 12
```

Artifact evidence used `shasum -a 256`, `stat -f '%N|%z'`, `otool -l` for the
`__text` section and adjacent sorted `nm -n` symbol addresses for flattened
`run()` size.
