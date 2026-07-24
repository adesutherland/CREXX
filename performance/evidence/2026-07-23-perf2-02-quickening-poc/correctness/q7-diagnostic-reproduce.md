# Reproduction commands

All paths below are scratch or read-only accepted-product paths.

Configure and build the diagnostic-only VMs:

```sh
cd /private/tmp/crexx-perf2-02.kJ5sZT/q7-core/perf2-02-q7-diagnostics/diagnostic-src
cmake -S . -B cmake-build-perf2-q7-diag -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_PROFILING=OFF \
  -DBUILD_TESTING=ON \
  -DCMAKE_C_FLAGS=-DPERF2_Q7_DIAGNOSTICS
cmake --build cmake-build-perf2-q7-diag \
  --target rxvm rxbvm --parallel 10
```

Run the one-process canonical and guard cases:

```sh
cd /private/tmp/crexx-perf2-02.kJ5sZT/q7-core/perf2-02-q7-diagnostics
commands/run-short.sh
```

Assemble and run the diagnostic-only COPY-transition fixture:

```sh
cd /private/tmp/crexx-perf2-02.kJ5sZT/q7-core/perf2-02-q7-diagnostics
product_bin=/private/tmp/crexx-perf2-01-product.dbLYqo/cmake-build-perf2-release/bin
"$product_bin/rxas" -o raw/q7_copy_transition.rxbin \
  raw/q7_copy_transition.rxas
for vm in rxvm rxbvm; do
  diagnostic-src/cmake-build-perf2-q7-diag/bin/$vm \
    raw/q7_copy_transition.rxbin "$product_bin/library.rxbin"
done
```

Run the diagnostic first-hit repeat and deterministic static captures:

```sh
commands/run-copy-timing-sample.sh
commands/capture-static.sh
```

No command above invokes the formal timing harness or writes an accepted
product artifact.
