# Overnight QA repair receipt — 14 October 2026

Owning scope and acceptance: [repair plan](../../planning/overnight-qa-repair-2026-10-14.md).
Base source: `50926edb8b6b36e98ba496b568c35bd6b544f5b8` (develop).
The scheduled workflow's event metadata names master; its assurance gate selects
and checks out this develop commit, confirmed in the checkout job log.

## Diagnosis

| Failed run | Observed failures | Cause and correction |
| --- | --- | --- |
| [Deep 38502097931](https://github.com/adesutherland/CREXX/actions/runs/38502097931) | Four `source_data_contract_l_*` tests on Linux, both Macs, MSVC and MinGW | The fixture writes a typed routine with a Level L header. Approved Level L source selection reserves these imports for generated grammar modules. Give this routine a Level B header for the L consumer; preserve all NUL, Unicode, source-import, newline and invalid-source assertions. |
| Same Deep run, MinGW | `cri17_attached_provider_opt`: both worker results have session ID 3 | GCC warns that `__declspec(thread)` is ignored in the test provider, making its current-session pointer shared. Select TLS by compiler, as in the owning VM, retaining Windows synchronization and the deliberate two-worker overlap. Add existing opt/noopt tests to the smoke tier so normal publication checks cover native MinGW. |
| [Sanitizer 38508471135](https://github.com/adesutherland/CREXX/actions/runs/38508471135) | Same L fixture on macOS and Linux; Linux stops on its first failure | Functional fixture failure. Downloaded build/CTest logs contain no ASan/LSan diagnostics. Linux's incomplete full CTest execution is not a sanitizer-clean result. |
| [Build 38453819158](https://github.com/adesutherland/CREXX/actions/runs/38453819158) | Both macOS plugin installers fail after Accepted notarization | CloudKit reports network connection lost (-1005), and `stapler staple` exits 68. Retry that service error at most four attempts (15/30/45-second delays); require successful stapling, strict ticket validation and the existing Gatekeeper assessment before publishing. Use one helper for core, plugin and retained-plugin packaging. |

The other Deep gates passed: isolated stress, all three Release build concurrency
settings, deterministic RXBIN comparison and Linux books. Latest publication
core platforms, Linux optimizer parity, Linux plugin and Windows plugin also
passed. These unchanged product inputs do not require another full overnight
matrix for a QA fixture and bounded packaging-retry repair.

## Qualification

1. Rebuilt current Debug tools and reproduced the exact L provider failure:
   [baseline log](source-data-baseline.log). Stale pre-integration local binaries
   originally passed, so they were not used as repair qualification.
2. `cmake --build cmake-build-debug --target stage-product qa-prep --parallel 10`
   passed: [build log](debug-build.log). This prepares the ordinary product and
   existing test artifacts; no product source changes were made.
3. `ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure -R
   '^source_data_contract_'`: **16/16 passed**, 108.59 seconds under concurrent
   rebuild load: [source-data log](source-data-debug.log).
4. Build `cri17_attached_provider_test`, `cri17_attached_provider_control-rxvml`
   and `cri17_attached_provider_control-rxbvml`; run `ctest --test-dir
   cmake-build-debug --parallel 4 --output-on-failure -R
   '^cri17_attached_provider'`: **4/4 passed**, 0.68 seconds: [CRI-17 log](cri17-debug.log).
   Existing scheduling/timeouts and concurrency inside the test are unchanged.
5. Maintained Apple ASan counterpart: **20/20 passed** using
   `tools/asan-run.sh`. Required compiler/library targets were built first;
   the 12 typed source-data cases ran when their prerequisites were ready,
   while the independent Classic library bootstrap completed. CRI-17 targets
   were then built, followed by the four Classic C cases and four CRI-17 controls.
   CTest commands used `--phase ctest --test-jobs 8 --leaks off --no-live-tail`
   with regexes `^source_data_contract_(b|g|l)_` and
   `^(source_data_contract_c_|cri17_attached_provider)` respectively.
   Retained proof: [12 typed cases](source-data-typed-asan.log),
   [eight remaining cases](source-data-classic-and-cri17-asan.log),
   [prerequisite rebuild](asan-prerequisite-build.log) and
   [CRI-17 build](cri17-asan-build.log), with matching runner option files.
   Leak detection is off because Apple's runtime lacks LSan. This is focused
   address-sanitizer validation, not Linux LSan closure or a full overnight pass.
   The prerequisite rebuild retains existing Apple SDK `sprintf` deprecation
   warnings in unchanged product code; this receipt does not claim a warning-
   free clean sanitizer build. No address-sanitizer diagnostic was emitted.
6. `python3 scripts/test-macos-stapling.py`: **6/6 policy controls passed**,
   covering immediate success, temporary recovery, terminal stapling error,
   bounded exhaustion, validation refusal and validation service failure;
   includes a package path with spaces: [fixture results](stapling-fixtures.log).
   `bash -n` passed for both shell scripts. Normal Mac core jobs run the fixture.
   Actual Apple-service ticket retrieval is verified by the automatic packaging
   jobs; this fixture does not claim a notarized/stapled artifact was produced.

[inputs.json](inputs.json) pins the unchanged product base and exact hashes of
all edited code/test/workflow inputs. [Failed excerpts](failed/) preserve the
failure inventory, MinGW warning and sanitizer diagnostic scans; full upstream
logs remain attached to their run IDs.

## Publication

All required local gates passed. This substantive payload publishes the three
reviewable causes together to existing hotfix and develop, preserving unrelated
dirty develop work. Remote refs and checkout integrity are verified live.
Normal automatic Build/CodeQL and snapshot packaging are expected to succeed;
their actual results remain GitHub evidence and conversation reporting. Do not
add a successful CI-status-only commit. A failed automatic gate requires
substantive diagnosis and repair. No release tag, full hosted rerun, global
sanitizer-clean claim or native Windows proof is implied by the local receipt.
