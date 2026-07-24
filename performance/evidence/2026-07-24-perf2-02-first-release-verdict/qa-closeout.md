# PERF2-02 post-acceptance QA closeout

Date: 2026-07-24

Adrian accepted the favorable first Release verdict and authorized all
remaining QA plus a local commit. This record covers the broad gates that were
deliberately deferred until that decision.

## Build regression and repair

The first full Debug build exposed a clean/full-build regression in the
pre-existing schema-5 value telemetry, not in Q3b. `rxvmintp.c` invokes
`RXVM_INSTRUMENTATION_VALUE_TYPED`; the normal and profiling instrumentation
backends defined it, but the test instrumentation backend did not. As a result,
the instrumented object variants failed to compile when the full build reached
them.

The repair adds the missing no-op macro to `rxvminstrument_test.h`, matching
that backend's existing contract. Focused instrumented-object builds then pass,
as do the complete builds. Rebuilding the ordinary Release product after the
repair reproduced the exact first-verdict binary hashes, confirming that the
test-only repair did not alter the measured product:

- `rxvm`: `717c3be993fcdcd7c5082baed0ff532c201d9b010f3ca049f982a14da28ec8d2`
- `rxbvm`: `e443520e367f676773ff1e7f7d7e891eb3120af4eb70d7829c23a9faa7c23f07`

## Broad validation

| Gate | Result |
| --- | --- |
| Full Debug build | PASS, 1,243 Ninja actions |
| Full Debug CTest | PASS, 1,907/1,907 in 162.23 s |
| Full ordinary profiling-off Release build | PASS, 1,509 Ninja actions |
| Full Release CTest | PASS, 1,907/1,907 in 53.25 s |
| Full macOS AddressSanitizer build | PASS, 1,247 Ninja actions |
| Full AddressSanitizer CTest | PASS, 1,907/1,907 in 488.41 s |
| ASan finding scan | PASS, no AddressSanitizer error, summary or runtime-error marker |
| Maintained Level B performance-tool self-tests | PASS, 6/6 |

The repository sanitizer runner's default leak setting stops immediately on
this Apple runtime because `detect_leaks` is unsupported. The successful full
run used `--build-leaks off --leaks off`; this is an AddressSanitizer result,
not a LeakSanitizer claim. Its retained runner directory is
`cmake-build-debugasan/asan-logs/20260724-094546-full`.

## Install, native toolchain and compatibility

`cmake --install` into a fresh isolated prefix installed 133 files. From that
prefix, the installed `crexx -native` compiled the installed
`examples/hello.crexx` into a Mach-O ARM64 executable. Running it printed:

```text
4711
hello CREXX world!
```

Both installed VM variants then executed the exact accepted pre-change
optimized Bounce RXBIN and library. Each returned result 1331 and
`PASS: AWFY Bounce`. This also gives an installed-tree public-bytecode
compatibility check, in addition to the first verdict's 34 balanced executions
of the same retained image in both current product VMs.

The configured CMake tree exposes `install`, `install/local` and
`install/strip`, but no CPack or `package` target. The isolated installed-tree,
native-compilation and compatibility checks are therefore the applicable
packaging gate.

## Evidence integrity

- The accepted PERF2-01 baseline manifest verifies every retained row.
- The PERF2-02 bounded PoC manifest verifies 136/136 non-manifest files.
- The staged maintained-source/document scope passes `git diff --check`. Raw
  disassembly, allocator/vmmap captures and prototype patch evidence retain
  their original whitespace and are governed by their checksum manifests.
- This first-verdict bundle is checksum-closed after the closeout edits.
