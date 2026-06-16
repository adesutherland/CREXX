# CREXX ASan and LSan Testing

Use `tools/asan-run.sh` for AddressSanitizer and LeakSanitizer runs. Do not
hand-run broad ASan builds or ctests unless the runner itself is broken.

## Runner

The runner keeps every command in a timestamped log directory:

```sh
tools/asan-run.sh --phase focused-lsan
tools/asan-run.sh --phase full --test-jobs 8
tools/asan-run.sh --phase build --build-target test_highlight_editor_diagnostics
tools/asan-run.sh --phase ctest --regex '^crexx_spaced_source_smoke$' --leaks on
```

Default build directory is `cmake-build-debugasan`.
The same runner can target the normal Debug tree with `--build-dir
cmake-build-debug`; use that for plain, non-sanitized validation when the Debug
tree is present.

The latest run is linked from:

```sh
cmake-build-debugasan/asan-logs/latest
```

Check or stop a run without hunting process IDs:

```sh
tools/asan-run.sh --status cmake-build-debugasan/asan-logs/latest
tools/asan-run.sh --kill cmake-build-debugasan/asan-logs/latest
```

For unattended agent sessions, approve the command prefix `tools/asan-run.sh`
once and run sanitizer work only through this script. Do not rely on ad hoc
`cmake`, `ctest`, `ps`, or `kill` commands while waiting for long sanitizer
runs; use `--status` and `--kill` on the run directory instead.

## Leak Policy

Build and test phases both use leak detection by default. The build invokes
`rxc`, `rxas`, `rxlink`, packers, and generated helper tools many times, so
build-time failures are part of the sanitizer surface and should be fixed rather
than bypassed.

```sh
tools/asan-run.sh --phase build
tools/asan-run.sh --phase full --leaks on
```

CTest commands should use leak detection for ownership work. `--phase full` and
`--phase focused-lsan` run CTest with `detect_leaks=1` by default. Use
`--build-leaks off` or `--leaks off` only when isolating non-leak sanitizer
failures such as use-after-free, buffer overflow, or timeout behaviour.

When test leak detection is on, CTest stops at the first failure by default.
This keeps the fix loop stepwise: run, read the first leak, fix it, rebuild, and
rerun. Use `--keep-going` only when deliberately collecting a broader failure
inventory.

Use serialized tests for focused leak triage:

```sh
tools/asan-run.sh --phase focused-lsan
```

That phase builds the needed targets with build leak detection on, then runs this
CTest surface with test leaks on and `--parallel 1`:

```text
linked_opt_runtime_artifacts_build
crexx_spaced_source_smoke
inline_cross_file_.*
source_import_.*
interface_.*import.*
```

## Full Workflow

For a proper full ASan/LSan check:

```sh
tools/asan-run.sh --phase full --test-jobs 8
```

This performs a full build first, then runs all CTests. The full build step is
important because many tests consume generated `.rxbin` artifacts directly. A
CTest-only run against a partially built tree can fail with missing module files
and should not be treated as a code failure.

For a stepwise full leak-clean loop after the tree is already built:

```sh
tools/asan-run.sh --phase ctest --leaks on --test-jobs 8
```

The first failing test is the next item to fix. After patching and rebuilding,
rerun with the same command or use a narrower `--regex`. If the patch changes
compiler, assembler, linker, runtime, or library code that can invalidate
generated `.rxbin` artifacts, prebuild those artifacts through the runner before
resuming a leak-on CTest range:

```sh
tools/asan-run.sh --phase build --build-target linked_opt_runtime_artifacts
```

For focused test regexes that normally require the linked-artifacts fixture,
exclude the automatic setup fixture after the prebuild so CTest does not launch
that build test under `detect_leaks=1`:

```sh
tools/asan-run.sh --phase ctest --regex '^testName$' --leaks on --fixture-exclude-setup linked_opt_runtime_artifacts
```

For long full runs, use CTest's index range through the runner to continue from
a known point after a fixed first failure:

```sh
tools/asan-run.sh --phase ctest --leaks on --test-jobs 8 --ctest-index '125,,'
```

The index value is passed to `ctest -I`. Because CTest may schedule tests by
cost rather than numeric ID, a late-numbered failure does not mean all lower
numbered tests have run. To avoid replaying an entire long range, generate an
exclude regex from the completed test names in the previous log and pass it
through `--exclude-regex-file FILE` so the runner remains the top-level command
and LSan does not run under a shell-wrapper/sandbox path. Keep
`--stop-on-failure` behaviour on for leak-clean loops unless deliberately
collecting an inventory.

For a narrow fix loop, rebuild changed targets through the runner, then run the
matching CTest regex:

```sh
tools/asan-run.sh --build-dir cmake-build-debug --phase build --build-target test_highlight_editor_diagnostics
tools/asan-run.sh --build-dir cmake-build-debug --phase ctest --regex '^highlight_editor_diagnostics$' --test-jobs 1

tools/asan-run.sh --phase build --build-target test_highlight_editor_diagnostics
tools/asan-run.sh --phase ctest --regex '^highlight_editor_diagnostics$' --leaks on --test-jobs 1
```

Use that Debug-then-ASan order after every sanitizer-related source fix when
the normal Debug tree is configured. The Debug run proves the command and test
still execute normally; the ASan/LSan run then proves the memory issue is gone.
If the Debug tree is unavailable or stale beyond quick repair, record that and
continue with the sanitizer tree rather than blocking the whole investigation.

`--build-target` is repeatable and intentionally valid only with
`--phase build`; use `--phase full` when a complete rebuild is required.
If `cmake-build-debug` is not configured locally, skip the plain Debug check and
record that the command was validated only in the sanitizer tree.

## Triage Rules

* Read `build.log`, `ctest.log`, or the current log through the runner status
  command. Avoid streaming broad build or ctest output directly.
* When CTest reports missing `.rxbin` modules, rerun through `--phase full` or
  `--phase build` before investigating source code.
* Treat third-party tool leaks separately from CREXX-generated code, compiler,
  linker, and runtime leaks. Prefer a focused regex before adding suppressions.
* If a leak is in a fetched dependency, keep any workaround tracked in this
  repository and record the upstream fix that should be carried back. Do not
  patch `cmake-build-*/_deps` by hand as the only fix; that disappears on a
  clean configure.
* Parser-tool syntax-highlighting CTests run the sibling
  `DSL-Syntax-Highlighter` `parser_tester` and inherit the runner's sanitizer
  environment. Keep these tests under leak detection; they cover
  `parser_tester`, `rxc --syntaxhighlight`, and `rxas --syntaxhighlight`.
* Keep LSan focused runs serialized so the first leak report is attributable to
  one test.
* If a run must be stopped, use `tools/asan-run.sh --kill RUN_DIR`.

## Parser-Tool LSan Coverage

The DSLSH `parser_tester` CTests must not be disabled just because they are
awkward under ASan. Keep them enabled so the ASan build exercises the same
syntax-highlighting and RXAS parser-tool regressions as Debug/Release builds.

The upstream DSLSH cleanup fixes are in the sibling checkout:

* `free_code_buffer()` owns and frees `CodeBuffer.ep_rules`.
* `parser_tester` frees `CommunicationFunctions` and `CodeBuffer` resources on
  each `INIT` replacement and process exit.
* `parser_tester` stops/waits for parser worker threads before freeing related
  buffers.
* DSLSH has its own `tools/asan-run.sh` runner and focused parser-tester LSan
  regression coverage.

Latest sibling DSLSH focused ASan/LSan validation: build and test leak detection
enabled, 4/4 focused tests passed, log directory
`cmake-build-asan/asan-logs/20260616-172720-focused-lsan`.

CREXX no longer carries parser-tool LSan suppressions and no longer appends
`detect_leaks=0` inside generated parser-tool CTest wrappers. The validated
focused parser-tool LSan commands are:

```sh
tools/asan-run.sh --phase ctest --regex '^syntaxhighlight_.*' --leaks on --test-jobs 1
tools/asan-run.sh --phase ctest --regex '^test_rxas_0[1-4]' --leaks on --test-jobs 1
```

Validation on 2026-06-16:

* Debug focused parser-tool tests passed: 62 compiler syntax-highlighting tests
  and 4 RXAS parser tests.
* ASan/LSan focused parser-tool tests passed with `detect_leaks=1`: 62 compiler
  syntax-highlighting tests and 4 RXAS parser tests.
* Full CREXX ASan/LSan CTest passed: 1295/1295, log directory
  `cmake-build-debugasan/asan-logs/20260616-110335-full`.

## 2026-06-16 Baseline Notes

The ASan runner now keeps leak detection enabled for build-time tools by
default. A source audit found no CREXX CMake test wrappers or parser-tool tests
that force `detect_leaks=0`; the only remaining leak-off paths are explicit
diagnostic runner options documented above.

ReleaseASAN was not completed end-to-end on this machine because the optimized
ASan build is too slow here. The previous ReleaseASAN build failure in `rxas`
was fixed and the exact failing target now builds with `detect_leaks=1`:

```sh
tools/asan-run.sh --build-dir cmake-build-releaseasan --phase build --build-target library-rxas --build-jobs 3
```

The `recv390` Release warnings were fixed and the focused targets build cleanly
in both Release and ReleaseASAN:

```sh
cmake --build cmake-build-release --target _recv390 _recv390_static
tools/asan-run.sh --build-dir cmake-build-releaseasan --phase build --build-target _recv390 --build-target _recv390_static
```

For the slow ReleaseASAN interpreter compile, `rxvmintp.c` is compiled at `-O1`
only when AddressSanitizer is enabled in `Release` or `RelWithDebInfo`. The
generated ReleaseASAN compile commands were checked and include the extra `-O1`
override. The full ReleaseASAN build and CTest should be rerun on a faster
machine.

Build-time review note: a no-op Release build reports `ninja: no work to do`,
so the current slowdown is not a perpetual rebuild loop. The expensive path is a
clean build or any source change that touches `interpreter/rxvmintp.c`. CMake
currently compiles that large translation unit separately for eight VM/test
targets (`rxvm`, `rxbvm`, `rxvme`, `rxbvme`, `rxvml`, `rxbvml`, `testvm`, and
`funcvm`). The Release `.ninja_log` shows about 5957 seconds of aggregate latest
`rxvmintp.c` compile time across those eight object files on this host. The
ASan `-O1` override currently covers the six interpreter-directory targets but
not the two RXPA test VMs, which are defined separately in `tests/rxpa`.

Normal Debug validation passed:

* Full Debug build passed, log `/tmp/crexx-debug-all.Iexmmm.log`.
* Full Debug CTest outside the restricted Codex sandbox passed 1295/1295, log
  `/tmp/crexx-debug-ctest-unsandboxed.gLGFm8.log`.
* A prior full Debug CTest inside the sandbox was 1291/1295 because the four
  loopback socket tests were blocked by sandbox networking; the same socket
  tests passed outside the sandbox in
  `/tmp/crexx-debug-socket-ctest-unsandboxed.MbMdoc.log`.
