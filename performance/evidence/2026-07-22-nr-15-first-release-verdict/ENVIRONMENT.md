# NR-15 panel environment and command record

Date/time zone: 2026-07-22, Europe/London
Host: `Darwin 25.5.0 arm64`, macOS 26.5.2 (25F84)
CPU: Apple M5, 10 logical CPUs
CMake: 4.3.2
Compiler: Apple clang 21.0.0 (`clang-2100.1.1.101`)

The ordinary panel tree is `cmake-build-release` with `CMAKE_BUILD_TYPE=Release`,
`CMAKE_C_FLAGS_RELEASE=-O3 -DNDEBUG`, and `CREXX_VM_PROFILING=OFF`. The diagnostic
tree is `cmake-build-profile` with the same Release flags and
`CREXX_VM_PROFILING=ON`.

Repository identity was checked at both start and stop with:

```text
git branch --show-current
git rev-parse HEAD
git rev-parse origin/develop
git ls-remote origin refs/heads/develop
```

All four values identify `develop` and
`240b29f456e995928206f04285a7c319612ff022`. The starting worktree was clean.

Target-only production builds used:

```text
cmake --build cmake-build-release --target rxc rxas rxlink rxvm rxbvm library \
  performance_nr15_stem_semantics performance_nr15_stem_access \
  benchmark_rexxcps_levelb_opt_artifact benchmark_rexxcps_levelb_noopt_artifact \
  --parallel 10
```

Diagnostic VM builds used:

```text
cmake --build cmake-build-profile --target rxvm rxbvm --parallel 10
```

Profile captures used the exact retained linked image and the form:

```text
cmake-build-profile/bin/<vm> --profile-output <cell>.csv <image>.rxbin \
  -a <mode> <size> <iterations> <family>
```

The provisional opcode control was compiled, assembled and linked with the
ordinary Release `rxc`, `rxas`, and `rxlink -s`; every runtime invocation has a
separate raw log. Its source and linked evidence remain under
`panel/candidate-d-native-stem-opcodes/`, but the opcodes and VM handlers are no
longer present in production source.

The final focused check was:

```text
ctest --test-dir cmake-build-release --parallel 10 --output-on-failure \
  -FA linked_opt_runtime_artifacts \
  -R '^(13_stems_(noopt|opt)|repro_multi_tail_stems_(noopt|opt)|13_stems_run_(noopt|opt)|repro_multi_tail_stems_run_(noopt|opt)|trace_stem_sugar|nr15_stem_(semantics|access)_(rxvm|rxbvm)_(noopt|opt))$'
```

It passes 17/17. The Classic comparison also passes when run separately with
`regina` and ooRexx `rexx`; their outputs are retained in `raw/`.

The timings in this packet are explicitly one-shot PoC pilots. No power/thermal
claim, balanced order, warmup set or governed 12-round production sample is
attached to them. That sampling is intentionally deferred until Adrian selects
the ISA/representation architecture and a complete production candidate is
frozen.
