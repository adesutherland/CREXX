# Compiler CI preparation repair

15 October 2026. Baseline `87ee68b317ed751d75eb954c50cfdde20a824d00`.
The repair and approved STEP03 frontends are delivered together on `develop`,
with separate reviewable commits. Publication was authorised after the focused
checks below. Normal Build and CodeQL results remain hosted evidence; this
receipt does not claim the new publication has passed hosted CI.

## Failure and repair

[Build 38732017168](https://github.com/adesutherland/CREXX/actions/runs/38732017168),
[parity job 116411945505](https://github.com/adesutherland/CREXX/actions/runs/38732017168/job/116411945505),
passed 932/940 tests. Eight STEP02 IR demo/contract tests failed because their
linked RXBIN files were absent. The [failed excerpt](hosted-failure-excerpt.log)
retains the observed paths and result.

The compiler directory registered per-test `CREXX_PREP_TARGETS`, but did not
finalize its directory QA tiers. Consequently the global preparation target
never collected those producers. Their `ALL` targets made ordinary product
builds work and masked the missing QA-only dependencies.

Schedule the existing `crexx_finalize_directory_qa_tiers` helper through
`crexx_schedule_directory_qa_tier_finalization` at directory entry. Its deferred
call runs after all tests in that directory are declared, including STEP03,
and before root preparation finalization. This preserves the existing build
and test ownership model. No compiler algorithm or runtime change is involved.

## Focused reproduction and qualification

- The generated Ninja graph originally had no `llcompiler` dependencies under
  `qa-prep-optimizer-parity`. After reconfiguration it contains all six relevant
  producers: two demo modes, two contract modes and two frontend test modes.
  See the [graph excerpt](graph-after.txt).
- Deliberately remove only those six generated linked test images, retaining
  their hashes and temporary backups. Build **only** `qa-prep-optimizer-parity`,
  without invoking the six producers directly. All six images are recreated.
  [Removed image identities](missing-images.json), [command](prep-command.json),
  [build log](prep-build.log). This is a missing-image preparation test using
  retained prerequisite tools/libraries, not a claim of a fully cold build.
- Run the eight STEP02 and 24 STEP03 tests: **32/32 passed**, across both VMs
  and host compile modes. [Command](tests-command.json), [log](focused-tests.log).
  The same registered tests remain the permanent clean-CI regression; no
  nested-build CTest or new long-running matrix was introduced.
- Normal product build passed without warnings or errors.
  [Command](product-build-command.json), [log](product-build.log).
- Recheck retained STEP03 identities: all 53 tools/images and 14 generated
  examples match; 20/21 source/build inputs match. The sole changed input is
  compiler `CMakeLists.txt`, qualified here. [Comparison](retained-evidence-check.json).
  The prior 152 distinct CTests, B-only bootstrap and no-opt closure evidence
  therefore remain valid for unchanged compiler code; the 32 repeated tests
  are not counted as 32 additional distinct tests.

The preparation build also rebuilt unrelated Text Inspector targets and emitted
two transient `RXBIN_IMPORT_READ_ERROR` diagnostics for its concurrently
produced `text_inspector_test_host.rxbin`. Preparation exited zero. The completed
image exists, all four focused Text Inspector core/TUI tests passed, and the
subsequent product build was diagnostic-free. [Focused command](text-inspector-command.json)
and [log](text-inspector.log). The shared import-directory producer overlap is
separate from the missing Level L preparation edges; no unrelated source was
changed and no general parallel-import repair is claimed.

No overnight, full parity or sanitizer matrix was run locally for this narrow
build-graph repair. The normal hosted parity job will exercise the clean Linux
path. Unrelated `build/` files and mainframe driver locks are preserved.
