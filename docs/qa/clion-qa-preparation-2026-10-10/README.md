# CLion QA preparation and visible build diagnostics

CP-C-FIND-07 is implemented and qualified locally. Adrian authorized publication
on 10 October: the C-first split and follow-up repairs are published together
to `origin/develop`, with a clean local checkout. The [authoritative plan](../../ROADMAP.md#proposed-c-first-extraction-phase--2026-10-10)
owns acceptance and the [input manifest](qualified-inputs.json) pins changed
code, build and test inputs over `04f6052d9`.

The completed CLion run had 152 failures, all from missing preparation:
136 source/bytecode inputs and 16 executables, including nine cases CTest did
not launch. The [inventory](failure-inventory.json) names every failed test.
The raw run remains at `output/failed-clion-run-retained.log`; its empty-command
sections misleadingly say “Test Passed”, so classification uses the failed-test
list and actual command/output, not that footer alone.

The agreed QA tiers remain intact. Normal `all` keeps its original closure;
`qa-prep-all` is an explicit build target combining `qa-prep` and
`qa-prep-measurement`. CLion's local **All CTest → Targets to build** now names
that target, retaining the Build before-launch task and visible-output arguments.
Daily correctness uses `qa-comprehensive`; All CTest also includes qualification,
stress and serial measurement tests. Tests do not launch nested builds.

The compiler directory's new unit tests declared their prerequisites but lacked
directory QA finalization. Scheduling its existing finalizer connects the two
executables and owned parity test to their tiers. A Debug QA-only build restores
both deliberately absent unit executables; both controls pass. The final QA graph
covers every direct first-party executable, with no need for a prior `all` build.
CLion CMake reload completed and the corrected prerequisite was selected, saved
and verified in the native configuration dialog and saved settings.

Preparation exposed an additional fixture defect: SHA256 compilation imported
an unrelated July linked image from its shared output directory, reporting
`RXBIN_IMPORT_READ_ERROR`. SHA256, stats and vector providers now use private
compiler import roots containing their declared plugins. The old image is
retained; no malformed-import diagnostic or metadata validation was weakened.

The rejected CMake output-capture launcher is removed. The Level G builder
no longer emits routine WAVE/START/BUILT/SKIP/PUBLISHED notices at their sources;
all successful and failing child stdout/stderr remains visible. RXPP defaults
to quiet notices, honors `-VERBOSE 1`/`-VERBOSE 0`, and always reports warnings
and errors. A permanent diagnostic test verifies all three notice modes and
missing-library warnings in quiet mode. Intentional local bindings and unused
imports are corrected in LLM and preparation fixtures, without changing APIs
or algorithms. Lemon no longer requests precedence-conflict report notices.
Optional compiler caching is requested only when the tool exists. Provider
package metadata has file dependencies and outputs, so unchanged builds do no
packaging work; package schemas are unchanged.

Both existing Debug and Release `all qa-prep-all` builds pass. Final changed
fixture rebuilds have zero warnings, and unchanged builds report “no work to do”.
All 152 originally failed Release tests pass in 44.74 seconds. Affected Debug
controls pass 83/83 in 138.01 seconds; the final strengthened RXPP diagnostic
case additionally passes in 2.47 seconds. Successful and failing builder probes
show both child output streams; failure retains `rc=7` and nonzero builder status.
Provider manifest entries match file hashes, and two unchanged package builds
preserve manifest bytes and modification time.

The full existing Release suite passes **3,326/3,326** in **917.77 seconds**.
The final preparation-only wiring changes no runtime command or property across
all 3,326 tests; the retained runtime-contract digest is identical before/after.
Both profile QA-only builds and final two-unit controls pass, with absent-output
rebuild proof in Debug. Reuse the full runtime proof rather than repeat it for
a preparation edge that cannot change execution. No exclusions,
weakened assertions, warning suppression or compiler/metadata semantic changes
were introduced. These are functional checks; no performance verdict is claimed.
SAN-011, SAN-012 and SAN-QA-018 remain repaired, open and release-blocking,
owned by Codex at the next Release 1 release-QA gate. No new hosted or broad
sanitizer result is claimed. The earlier [capture receipt](../quiet-build-stdin-2026-10-10/quiet-build.md)
is explicitly historical; its source warning repairs and stdin proof remain valid.

Hosted workflow inspection: normal `build.yml` core jobs already use
`stage-product` followed by `qa-smoke`; deep jobs use `qa-comprehensive` and
`qa-qualification`. No workflow is changed and no full-fixture work is added to
ordinary product builds. Shared warning, fixture-root, packaging and unit-prep
repairs also apply to those jobs. The normal automatic product/smoke, optimizer-
parity and CodeQL workflows accompany this publication. Their successful
completion is expected; no hosted result has been observed or claimed here.
No manual overnight assurance run is dispatched, and no status-only follow-up
push is planned. The local 3,326-test proof is reused on unchanged qualified
code/test/build inputs; this delivery amendment changes documentation only.
