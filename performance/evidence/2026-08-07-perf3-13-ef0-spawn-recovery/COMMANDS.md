# EF-0 retained commands

All commands ran from `/Users/adrian/CLionProjects/CREXX` on Darwin arm64
25.5.0 with Apple Clang 21.0.0 and CMake 4.3.2.

## Baseline

```sh
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```

The 30-way result was followed by a serial regex union containing the 22
handover names plus direct-VM, optimized and capture controls. Per-test output
was retained outside the repository while the baseline was classified.

## Ordinary Debug and Release

```sh
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure

cmake -S . -B cmake-build-release \
  -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_PROFILING=OFF
cmake --build cmake-build-release --parallel 10
cmake --build cmake-build-release --parallel 10 \
  --target rxbvm rxbvml rxbvme rxvm
```

Focused CTest invocations used the exact handover list plus these controls:

```text
dynamic_interface_load_rxvm
ts_address_crexx_opt
ts_address_capture_noopt
ts_address_capture_opt
ts_address_capture_rxbvm_noopt
ts_address_capture_rxbvm_opt
ts_address_spawn_transfer_noopt
ts_address_spawn_transfer_opt
ts_address_spawn_transfer_rxbvm_noopt
ts_address_spawn_transfer_rxbvm_opt
ts_address_spawn_transfer_rxtvm_noopt
ts_address_spawn_transfer_rxtvm_opt
benchmark_rexxcps_levelb_noopt
benchmark_rexxcps_levelb_opt
test_rexxcps_decimal_register_integrity
```

## AddressSanitizer

```sh
cmake -S . -B cmake-build-debugasan \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCREXX_VM_PROFILING=OFF \
  -DCMAKE_C_FLAGS='-fsanitize=address -fno-omit-frame-pointer' \
  -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address' \
  -DCMAKE_SHARED_LINKER_FLAGS='-fsanitize=address'

tools/asan-run.sh \
  --phase build \
  --build-dir cmake-build-debugasan \
  --build-jobs 4 \
  --build-leaks off

tools/asan-run.sh \
  --phase ctest \
  --build-dir cmake-build-debugasan \
  --test-jobs 1 \
  --leaks off \
  --regex '^(dynamic_interface_load_(rxvm|driver)|ts_address_crexx_(noopt|opt)|ts_address_capture(_rxbvm)?_(noopt|opt)|ts_address_spawn_transfer(_rxbvm|_rxtvm)?_(noopt|opt)|crexx_headerless_simple_smoke|crexx_headerless_rexx_levelc_smoke|crexx_explicit_header_smoke|crexx_args_smoke|crexx_rexxscript_smoke|crexx_status_diagnostics|crexx_spaced_source_smoke|crexx_crxc_noargs_smoke|crexx_tool_output_path_policy|crexx_diagnostic_plumbing|crexx_source_root_compile_only|crexx_rxas_import_compile_only|crexx_nocompile_exec_smoke|crexx_native_nocompile_smoke|crexx_native_explicit_header_smoke|crexx_native_address_crexx_smoke|crexx_native_default_link_strip|crexx_native_linkmap_keep_source|crexx_native_link_keep_inline|crexx_native_nokeep_cleanup)$'
```

The runner retained the passing full-build record at
`cmake-build-debugasan/asan-logs/20260807-131632-build` and the passing 34-test
record at `cmake-build-debugasan/asan-logs/20260807-133301-ctest` in the local
build tree.
