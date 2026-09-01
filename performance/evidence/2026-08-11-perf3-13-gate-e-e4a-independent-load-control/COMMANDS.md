# Commands

Working directory for every command:
`/Users/adrian/CLionProjects/CREXX`.

## Debug

```sh
cmake --build cmake-build-debug \
  --target program_generation_control-rxvml \
           program_generation_control-rxbvml \
           program_generation_control-rxtvml \
  --parallel 10

ctest --test-dir cmake-build-debug \
  -R '^program_generation_control-' \
  --output-on-failure -V
```

Result: 3/3 passed.

## Apple AddressSanitizer

The existing sanitizer tree was refreshed with `cmake -S . -B
cmake-build-debugasan`, then every build/test action ran through the repository
runner. The product/switch targets built in the first runner invocation; after
that invocation exposed a stale multi-target Makefile lookup for the newly
generated direct-threaded target, the same target built successfully in the
required runner as a single target.

```sh
tools/asan-run.sh --phase build \
  --build-target program_generation_control-rxvml \
  --build-target program_generation_control-rxbvml \
  --build-target program_generation_control-rxtvml \
  --build-jobs 4 --build-leaks off

tools/asan-run.sh --phase build \
  --build-target program_generation_control-rxtvml \
  --build-jobs 4 --build-leaks off

tools/asan-run.sh --phase ctest \
  --regex '^program_generation_control-' \
  --leaks off --test-jobs 3
```

Result: 3/3 passed. Apple LeakSanitizer is unsupported, so
`ASAN_OPTIONS=detect_leaks=0`; normal Debug teardown still asserts exact live
allocation state.

## Adjacent Debug and full Debug

```sh
ctest --test-dir cmake-build-debug \
  -R '^(program_generation_control-|reentrancy_check$|rxasdispatchcontract-|ts_loadmodule_(noopt|opt)$|rxas_optimizer_barrier_metaloadmodule$)' \
  --parallel 10 --output-on-failure

ctest --test-dir cmake-build-debug \
  --parallel 30 --output-on-failure
```

Results: adjacent panel 10/10; full Debug 2,037/2,037.

## Ordinary profiling-off Release

`cmake-build-release/CMakeCache.txt` records `CMAKE_BUILD_TYPE=Release` and
`CREXX_VM_PROFILING=OFF`.

```sh
cmake --build cmake-build-release \
  --target program_generation_control-rxvml \
           program_generation_control-rxbvml \
           program_generation_control-rxtvml \
  --parallel 10

ctest --test-dir cmake-build-release \
  -R '^program_generation_control-' \
  --output-on-failure -V
```

Result: 3/3 passed.

## Direct deterministic replay

```sh
cmake-build-debug/interpreter/program_generation_control-rxvml \
  cmake-build-debug/interpreter/test_program_generation_control.rxbin \
  cmake-build-debug/interpreter/test_program_generation_late.rxbin

cmake-build-debug/interpreter/program_generation_control-rxbvml \
  cmake-build-debug/interpreter/test_program_generation_control.rxbin \
  cmake-build-debug/interpreter/test_program_generation_late.rxbin

cmake-build-debug/interpreter/program_generation_control-rxtvml \
  cmake-build-debug/interpreter/test_program_generation_control.rxbin \
  cmake-build-debug/interpreter/test_program_generation_late.rxbin
```

All three executions report the same structural counts and all controls `PASS`.
