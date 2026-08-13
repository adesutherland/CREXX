# PERF3-13 E5 Windows and combined-PoC commands

Run builds and tests sequentially per Windows build tree. Do not overlap a
build and CTest in the same tree because generated RXBIN cleanup can race open
Windows file handles.

## Fixture policy

An ordinary source-complete build leaves both cache variables empty:

```text
-DCREXX_E5_TEST_RXBIN=
-DCREXX_E5_SPARSE_TEST_RXBIN=
```

Additional compiler lanes may reuse qualified fixtures to save build time:

```text
-DCREXX_E5_TEST_RXBIN=C:/absolute/path/test_persistent_worker_executor.rxbin
-DCREXX_E5_SPARSE_TEST_RXBIN=C:/absolute/path/test_persistent_worker_sparse.rxbin
```

Both source inputs remain mandatory in the commit even when qualified RXBINs
are reused.

## MinGW GCC Release

Run in a shell whose `PATH` contains the selected MinGW runtime:

```powershell
cmake -S . -B build-windows-gcc-release -G Ninja `
  -DCMAKE_BUILD_TYPE=Release -DCREXX_VM_PROFILING=OFF
cmake --build build-windows-gcc-release --parallel 8 --target `
  rxc rxas rxlink rxvm test_rxvmactive `
  persistent_worker_executor-rxvml `
  persistent_worker_executor-rxbvml `
  persistent_worker_executor-rxtvml
ctest --test-dir build-windows-gcc-release `
  -R '^(rxvmactive|persistent_worker_executor-)' --output-on-failure
```

Expected focused count on Windows: 19.

## MSVC Debug

Run from an x64 Visual Studio developer shell. Parser mode is disabled only
because the optional sibling syntax-highlighter is not native-MSVC portable.

```powershell
cmake -S . -B build-windows-msvc-debug -G Ninja `
  -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=cl `
  -DCREXX_VM_PROFILING=OFF -DENABLE_PARSER_MODE=OFF
cmake --build build-windows-msvc-debug --parallel 8 --target `
  rxc rxas rxlink rxvm test_rxvmactive `
  persistent_worker_executor-rxvml `
  persistent_worker_executor-rxbvml
ctest --test-dir build-windows-msvc-debug `
  -R '^(rxvmactive|persistent_worker_executor-)' --output-on-failure
```

Expected focused count on Windows: 13. MSVC has no computed-goto engine.

## Clang with the MSVC ABI

Run from an x64 Visual Studio developer shell so the Windows SDK and MSVC
runtime environment are available. Use an LLVM Windows-MSVC distribution, not
a `windows-gnu` target:

```powershell
cmake -S . -B build-windows-clang-msvc-debug -G Ninja `
  -DCMAKE_BUILD_TYPE=Debug `
  -DCMAKE_C_COMPILER=C:/absolute/path/clang.exe `
  -DCMAKE_CXX_COMPILER=C:/absolute/path/clang++.exe `
  -DCMAKE_C_COMPILER_TARGET=x86_64-pc-windows-msvc `
  -DCMAKE_CXX_COMPILER_TARGET=x86_64-pc-windows-msvc `
  -DCREXX_VM_PROFILING=OFF -DENABLE_PARSER_MODE=OFF
cmake --build build-windows-clang-msvc-debug --parallel 8 --target `
  rxc rxas rxlink rxvm test_rxvmactive `
  persistent_worker_executor-rxvml `
  persistent_worker_executor-rxbvml `
  persistent_worker_executor-rxtvml
ctest --test-dir build-windows-clang-msvc-debug `
  -R '^(rxvmactive|persistent_worker_executor-)' --output-on-failure
```

Expected focused count on Windows: 19.

## macOS and Linux continuation

The existing retained macOS/Linux command records remain authoritative for
their native-signal carrier and performance panels. The minimum consolidation
check is:

```sh
cmake -S . -B cmake-build-mthread-debug -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DCREXX_VM_PROFILING=OFF
cmake --build cmake-build-mthread-debug --parallel 10 --target \
  test_rxvmactive \
  persistent_worker_executor-rxvml \
  persistent_worker_executor-rxbvml \
  persistent_worker_executor-rxtvml
ctest --test-dir cmake-build-mthread-debug \
  -R '^(rxvmactive|persistent_worker_executor-)' \
  --output-on-failure --timeout 120
```

On these platforms CMake supplies `CREXX_VM_POC_DOORBELL=posix`. Windows CTest
supplies `windows-special-apc` and registers the extra forced-fallback,
fallback-stress and sparse-progress cases.
