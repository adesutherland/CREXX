# Replay commands

Fresh default and explicit profiling-off Release configurations:

```sh
cmake -S . -B "$DEFAULT_RELEASE" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake -S . -B "$EXPLICIT_RELEASE" -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=OFF -DCREXX_VM_HANDLER_PANEL=profile-20
cmake --build "$DEFAULT_RELEASE" --target rxtvm rxbvm --parallel 2
cmake --build "$EXPLICIT_RELEASE" --target rxtvm rxbvm --parallel 2
cmp "$DEFAULT_RELEASE/bin/rxtvm" "$EXPLICIT_RELEASE/bin/rxtvm"
cmp "$DEFAULT_RELEASE/bin/rxbvm" "$EXPLICIT_RELEASE/bin/rxbvm"
```

Broad profiling-off QA:

```sh
cmake --build "$DEFAULT_RELEASE" --parallel 10
ctest --test-dir "$DEFAULT_RELEASE" --parallel 30 --output-on-failure

cmake -S . -B "$DEFAULT_DEBUG" -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build "$DEFAULT_DEBUG" --parallel 10
ctest --test-dir "$DEFAULT_DEBUG" --parallel 30 --output-on-failure
```

Focused default profiler/documentation QA:

```sh
cmake -S . -B "$DEFAULT_PROFILE" -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_PROFILING=ON
cmake --build "$DEFAULT_PROFILE" --parallel 10 --target \
  test_rxvmprofile run_profiling_demo run_tests_procedure_profile rxseq
ctest --test-dir "$DEFAULT_PROFILE" --parallel 1 --output-on-failure -R \
'^(rxvmprofileaccounting|rxvmprofiletable|rxbvmprofiletable|rxvmprofilecsv|rxbvmprofilecsv|vmprofilingdocumentation)$'
```

All configurations were fresh and omitted `CREXX_VM_HANDLER_PANEL` except the
explicit identity control. Raw timing was not collected: exact binary identity
allows the already retained formal profile-20 timing to remain authoritative.
