# Commands

Configure and build the isolated Debug product dependency closure:

```bash
cmake -S . -B cmake-build-debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DCREXX_VM_PROFILING=OFF
cmake --build cmake-build-debug --parallel 10 --target \
  rxc rxas rxvm rxbvm library compiler_exit_bin run_tests_decimal \
  mc_decimal_full_tests db_decimal_tests
```

Run the focused Debug selection:

```bash
ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure \
  -R '^(nr09_numeric_context_contract|rxasdecimaltests|rxasdecimaltests-rxbvm|rxasdecimaltests-mc|rxasdecimaltests-mc-rxbvm|rxasdecimaltests-db|rxasdecimaltests-db-rxbvm|mc_decimal_full_tests|db_decimal_tests)$'
```

Configure and build the ordinary profiling-off Release dependency closure:

```bash
cmake -S . -B cmake-build-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCREXX_VM_PROFILING=OFF
cmake --build cmake-build-release --parallel 10 --target \
  rxc rxas rxvm rxbvm library compiler_exit_bin run_tests_decimal \
  mc_decimal_full_tests db_decimal_tests
```

Run the identical focused Release selection:

```bash
ctest --test-dir cmake-build-release --parallel 10 --output-on-failure \
  -R '^(nr09_numeric_context_contract|rxasdecimaltests|rxasdecimaltests-rxbvm|rxasdecimaltests-mc|rxasdecimaltests-mc-rxbvm|rxasdecimaltests-db|rxasdecimaltests-db-rxbvm|mc_decimal_full_tests|db_decimal_tests)$'
```

For each build, assemble
`tests/performance/decimal/rxas_numeric_context_observable.rxas` into a
`mktemp -d` directory, then run these six correctness cells from the build's
`bin` directory:

```bash
./rxvm library.rxbin observable.rxbin
./rxbvm library.rxbin observable.rxbin
./rxvm -p rxvm_mc_decimal library.rxbin observable.rxbin
./rxbvm -p rxvm_mc_decimal library.rxbin observable.rxbin
./rxvm -p rxvm_db_decimal library.rxbin observable.rxbin
./rxbvm -p rxvm_db_decimal library.rxbin observable.rxbin
```

No command in this bundle is a performance measurement.
