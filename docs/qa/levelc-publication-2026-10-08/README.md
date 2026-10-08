# Level C local publication QA — 2026-10-08

Status: all requested local build, normal Debug and maintained macOS ASan gates
are verified. Publication is pending the final commit/push. The authoritative scope and acceptance
record is [LC-QA-PUB](../../planning/release-1/levelc-compatibility-worklist.md).
This receipt does not close wider Level C, Release 1 or cross-platform obligations.

## Repairs

1. Clean parallel CLion rebuilds failed in TRACE consumers with
   `CLASS_NOT_FOUND`. The generated helper needs `rexxvalue.RexxValue` from the
   consolidated runtime, but consumer compilation depended only on the bootstrap
   exit bundle. The TRACE runtime/capture rules now depend on `rxfnsc` and its
   published RXBIN. The exit bundle remains a prerequisite of the runtime.
   [Original failure](original-debug-rebuild.log),
   [clean Debug success](debug-clean-rebuild.log),
   [clean Release success](release-clean-rebuild.log).
2. The existing `structured_exit_debug_run` searched the shared compiler test
   directory's unrelated generated modules. It passed, but took 684.03 seconds
   and wrote multiple GiB of parser trace. The unchanged script passed in an
   isolated temporary directory in 4.45 seconds. Its maintained fixture now owns
   one private work/import directory, preserves `rxc -d2 -n`, assembly and VM
   execution, and compares the same exact output. The repaired Debug CTest passes
   in 3.93 seconds. [Probe](structured-exit-probe.log),
   [focused Debug receipt](structured-exit-debug.log).

3. Apple GNU Make 3.81 skipped a static consumer relink when the rebuilt archive
   and old executable shared one whole-second timestamp. Nanosecond receipts
   show the archive really was newer; the generated Makefile already tracked it.
   A timestamp-only archive touch correctly triggered relinking. The fixture now
   waits for a private filesystem timestamp newer than its initial consumers
   before changing the library, preserving the genuine 41-to-42 assertions.
   [Original commands](sdk-original-commands.log),
   [timestamps](sdk-timestamp-failure.json),
   [timestamp-only proof](sdk-timestamp-only-probe.log),
   [Debug pass](sdk-fixed-debug.log), [ASan pass](sdk-fixed-asan.log).

No compiler C, language rule, runtime ABI, optimizer policy or test assertion was
changed. Existing TRACE and structured-exit fixtures supply permanent coverage.

## Local results

| Gate | Command / scope | Result |
| --- | --- | --- |
| Debug clean rebuild | `cmake --build cmake-build-debug --clean-first --parallel 32` | PASS |
| Release clean rebuild | `cmake --build cmake-build-release --clean-first --parallel 32` | PASS; 189.00s |
| Debug preparation | `cmake --build cmake-build-debug --target qa-prep qa-prep-measurement --parallel 32` | PASS; 13.52s |
| Full normal Debug CTest | `ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure` | 3296/3296; zero failures/timeouts; 1828.68s |
| Changed debug fixture | `ctest --test-dir cmake-build-debug -R '^structured_exit_debug_run$' --parallel 1 --output-on-failure` | 1/1; 3.93s |
| Maintained macOS ASan full build | Runner below | PASS; no reported diagnostic |
| Maintained macOS ASan qa-prep | Runner below | PASS; no reported diagnostic |
| Maintained macOS ASan correctness CTest | Runner below plus focused repair/continuation | 3112/3112 unique passing cases; no memory diagnostic |
| Changed Release fixtures | Structured debug case plus staged installed SDK | 2/2; 1.29s and 20.63s |

Full [Debug test log](debug-full-ctest.log) and
[preparation log](debug-preparation.log) are retained here. The maintained runner
is `cmake-build-debugasan/asan-logs/20261008-120648-full`:

```sh
tools/asan-run.sh --phase full --build-jobs 4 --test-jobs 8   --build-leaks off --leaks off --exclude-label '^performance-measurement$'   --stop-on-failure --no-live-tail --tail-lines 12
```

Apple's runtime does not support LeakSanitizer. Only the documented
`performance-measurement` lane is excluded from the instrumented gate; the full
normal Debug run includes it. This is local macOS AddressSanitizer evidence,
not a Linux LeakSanitizer or cross-platform completion claim. Selected existing
profile settings are in [build-configurations.json](build-configurations.json).

## Input identity and reuse

The full normal Debug run used the [debug-run manifest](debug-run-inputs.json),
fingerprint `2d2e790dd89a6a196ffb6ab02d5810108293918062417228f0503e5ba7d4b71a`.
The subsequent private-directory and SDK timestamp repairs change only
`compiler/tests/CMakeLists.txt` and `tests/rxpa/rxpa_external_sdk_consumer.cmake`
among the 7,905 source/test/build files. [Reuse proof](test-registry-reuse.json)
identifies exactly those two affected fixtures; all other registrations, commands
and properties are identical. Both fixtures then passed normal Debug replays,
establishing 3,294 unchanged broad passes plus two focused final-input passes.
Both changed fixtures also pass in Release. Its first SDK replay omitted the
registered staging prerequisite after clean; [preparation](release-sdk-preparation.log)
and [final SDK pass](release-sdk-final.log) resolve that input failure.

The [final manifest](final-inputs.json) fingerprint is
`bdd088acf09b0b1102634adadc428b92bbc804fa418ecfbba39b8fec1d089bad`.
It covers tracked non-Markdown inputs outside `docs/` and historical
`performance/evidence/`, including source, build rules, tests, goldens and
workflows. Documentation/evidence-only commits do not change those inputs.
Separate concurrent architecture/roadmap proposal edits and the untracked local
PDF are preserved outside these repairs and evidence commits.

## Complete ASan coverage

The initial full run passed build/preparation and 2,943 CTests, then stopped on
the SDK timestamp collision. [Original failed CTest log](asan-initial-ctest.log)
remains a failed attempt. After the repair, the same SDK fixture passed normal
Debug and maintained ASan 1/1 (52.31s). A [verified compact selector](asan-resume-selection.json)
excluded exactly 2,944 completed passes; the [remaining run](asan-remaining-ctest.log)
passed 168/168 in 918.48s. The first literal-name exclusion exceeded CTest's regex
limit and was stopped; no duplicate results from that attempt are accepted.

[Coverage summary](asan-coverage-summary.json) verifies that the union of these
passing names equals all 3,112 eligible cases, with no missing tests. Only the SDK
test script changed after the initial sanitizer attempt, affecting that fixture;
all other passing inputs are unchanged. The private debug fixture passed ASan in
12.42s. Build, preparation, CTest and its redirected debug trace contain no
AddressSanitizer report. No sanitizer suppression, test exclusion or failure
waiver was added. The documented timing lane and Apple leak capability remain
as stated above. No first-party sanitizer finding was discovered.

## Publication

Qualified code/test revision: `17d8627788a82d8e3d8f304275bc2171ff7c3688`. The three repairs are
recorded in [qualified-revisions.json](qualified-revisions.json).

All local acceptance gates pass. Existing Level C compatibility obligations,
including LC-DOC-ISSUE-01 TIME clause refresh, remain open in the component
worklist. This qualifies development publication, not full Classic/Release 1
completion. No additional hosted overnight
matrix is dispatched for this development publication. Normal automatic
publication workflows are checked after pushing the qualified develop inputs.
