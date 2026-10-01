# Overnight QA repair — 17 September 2026

The six red lanes share one stale test expectation, recorded as SAN-QA-016 and
CI-F20. No first-party memory-safety finding was observed. The repair changes
only `interpreter/tests/test_rxpa_concurrency.c`, plus planning/evidence.

## Original failure

Both scheduled workflows checked out develop
`59fc02eb905ea2a0878e4055b7114921d5e6a294`. GitHub's schedule header instead
identifies the workflow source on master `48ebc1f61`.

| Gate | Run | Observed outcome |
| --- | --- | --- |
| Deep Build QA | [35175162069](https://github.com/adesutherland/CREXX/actions/runs/35175162069) | Linux, Windows MinGW, ARM Mac and Intel Mac fail only `rxpa_bundled_fs_concurrency`; Release build comparisons and isolated stress pass. |
| Sanitizer QA | [35179649593](https://github.com/adesutherland/CREXX/actions/runs/35179649593) | Linux and ARM Mac fail the same assertion. Linux stops early; Mac runs all 2,345 tests. No ASan/LSan diagnostic in the downloaded artifacts. |

Full workflow logs are retained compressed, with JSON job identities and the
complete downloaded sanitizer artifacts. `debug-before.log` reproduces the
identical failure locally before editing:

```text
RXPA manifest query failed: rc=0 capabilities=0 handles=1/0
```

The library handle count is correct. Commit `03bf7855b` added the reusable
VM-owned fileguard used by the native llama installer manager and changed
rxfs from a blanket process-reentrant declaration to mixed V2 policy. The
concurrency test still demanded the old blanket flag and exited before its
two-VM calls. This is independent of llama.cpp inference or CUDA.

## Repair and focused checks

The test now requires complete session hooks, zero blanket capabilities,
reentrant policy for all 16 filesystem operations and session-affine policy for
the fileguard factory and three methods. Existing simultaneous VM calls,
surviving-VM use after peer teardown, and library-handle cleanup are preserved.
The generic manifest diagnostic now labels expected capability and prior handle
count explicitly.

Focused commands (normal Debug first, then maintained Apple ASan):

Debug passes 16/16 in 0.91 seconds; maintained Apple ASan passes the same
16/16 in 2.58 seconds. Build/test logs and `local-inputs.json` retain the
qualified input hashes. The panel includes all bundled plugin concurrency
checks, session-load rollback and all four filesystem VM/optimization cells.

```sh
cmake --build cmake-build-debug --target test_rxpa_concurrency rxfs_test --parallel 6
tools/asan-run.sh --build-dir cmake-build-debug --phase ctest \
  --regex '^(rxpa_bundled_.*_concurrency|rxpa_dynamic_session_factory_failure_rollback|rxfs_.*)$' \
  --test-jobs 1 --build-leaks off --leaks off --no-live-tail
tools/asan-run.sh --phase build --build-target test_rxpa_concurrency \
  --build-target rxfs_test --build-jobs 6 --build-leaks off --leaks off --no-live-tail
tools/asan-run.sh --phase ctest \
  --regex '^(rxpa_bundled_.*_concurrency|rxpa_dynamic_session_factory_failure_rollback|rxfs_.*)$' \
  --test-jobs 1 --build-leaks off --leaks off --no-live-tail
```

Apple LeakSanitizer is unavailable; the hosted Linux gate keeps both build-time
and test-time leak detection enabled. These are focused runs, not local broad
sanitizer qualification.

Three temporary copies of the production plugin are compiled as negative
controls, leaving tracked production sources unchanged: returning reentrant
policy for every procedure, returning session-affine policy for every procedure,
and removing the session destructor. All fail the corresponding new assertion
with exit 1. Exact mutations and diagnostics are in `negative-controls.log`.

## Hosted completion

Repair `d8f59732d4d4abeb18ec20509821fd30ecf8962e` is published to develop.
All required workflows have terminal success on that exact commit:

| Gate | Run | Result |
| --- | --- | --- |
| Deep Build QA | [35189896087](https://github.com/adesutherland/CREXX/actions/runs/35189896087) | Success: five comprehensive platforms, package/external-consumer checks, stress and Release build equivalence. |
| Sanitizer QA | [35189897956](https://github.com/adesutherland/CREXX/actions/runs/35189897956) | Success: Linux ASan/LSan and macOS arm64 ASan, 2,345/2,345 each. |
| Build CREXX | [35189886874](https://github.com/adesutherland/CREXX/actions/runs/35189886874) | Success: all cores, base plugins, optimizer parity and development snapshot publication. |
| CodeQL | [35189886774](https://github.com/adesutherland/CREXX/actions/runs/35189886774) | Success. |

Deep comprehensive counts: Linux and both Macs 2,331/2,331 each; Windows MinGW
2,253/2,253; Windows MSVC 2,135/2,135. Every platform passes the repaired
`rxpa_bundled_fs_concurrency` test, and three package/external-consumer tests
pass per platform. The complete Deep log and terminal job metadata are retained.

Both sanitizer platforms pass the original filesystem test (Linux 0.03 s,
Mac 0.09 s). Full CTest elapsed times are 4,698.61 s on Linux and 3,878.52 s
on Mac. Linux build, QA preparation and CTest logs all record
`ASAN_OPTIONS=detect_leaks=1`. The complete artifact scan finds no ASan/LSan
diagnostic. `sanitizer-green-artifacts.tar.gz`, the compressed workflow log and
terminal JSON retain this evidence. SAN-QA-016 and all CI-F20 criteria/steps
are closed. Final evidence-only bookkeeping is retained on
`temp/overnight-qa-20260917`; develop remains at the green repaired commit.

The manual Deep run uses the current develop workflow and its five-platform
matrix, including MSVC as well as MinGW. The old scheduled workflow ran four
platforms. Both qualify the same respective develop source SHA named above.
Original failures remain retained; this record does not close the parent
real-device/model/release acceptance criteria.
