# Commands

Build the current focused product:

```bash
cmake --build cmake-build-debug --parallel 10 --target \
  rxc rxas rxvm rxbvm mc_decimal_dynamic db_decimal_dynamic run_tests_decimal
```

Run the focused current suite:

```bash
ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure \
  -R '^(nr09_numeric_context_contract|rxasdecimaltests|rxasdecimaltests-rxbvm|rxasdecimaltests-mc|rxasdecimaltests-mc-rxbvm|rxasdecimaltests-db|rxasdecimaltests-db-rxbvm|mc_decimal_full_tests|db_decimal_tests)$'
```

Assemble the separately named diagnostic into a `mktemp -d` scratch directory:

```bash
cmake-build-debug/bin/rxas \
  -o "$decimal_tmp/rxas_numeric_context_observable" \
  tests/performance/decimal/rxas_numeric_context_observable.rxas
```

Run the resulting RXBIN with each VM using the static/default provider and the
two explicit providers, from `cmake-build-debug/bin` so plugin discovery is
identical:

```bash
./rxvm library.rxbin "$decimal_tmp/rxas_numeric_context_observable.rxbin"
./rxbvm library.rxbin "$decimal_tmp/rxas_numeric_context_observable.rxbin"
./rxvm -p rxvm_mc_decimal library.rxbin "$decimal_tmp/rxas_numeric_context_observable.rxbin"
./rxbvm -p rxvm_mc_decimal library.rxbin "$decimal_tmp/rxas_numeric_context_observable.rxbin"
./rxvm -p rxvm_db_decimal library.rxbin "$decimal_tmp/rxas_numeric_context_observable.rxbin"
./rxbvm -p rxvm_db_decimal library.rxbin "$decimal_tmp/rxas_numeric_context_observable.rxbin"
```

These are correctness commands. Their elapsed times are not retained or used.
