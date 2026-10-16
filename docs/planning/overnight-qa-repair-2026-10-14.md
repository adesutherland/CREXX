# Overnight QA repair — 14 October 2026

## Vision and intended outcomes

Restore the overnight correctness checks and development publication after the
14 October Deep Build QA and Sanitizer QA failures. Work in the existing hotfix
checkout, synchronized to develop 50926edb8b6b36e98ba496b568c35bd6b544f5b8;
preserve unrelated work in the occupied develop checkout. Keep the approved
Level L grammar-source selection policy, source-byte assertions, strict macOS
installer validation and normal publication gates. Publish the tested repairs
to hotfix and develop. No language, compiler architecture or release change.

## Acceptance criteria

1. **AC-01 — verified:** Account for every failed overnight job using retained
   logs. Five Deep platforms fail the four source_data_contract_l modes; MinGW
   also fails cri17_attached_provider_opt because GCC ignores the fixture's
   __declspec(thread), making its current-session pointer shared. Both
   sanitizer jobs fail the same contract (Linux stops at its first failure).
   No ASan/LSan diagnostic occurs in the downloaded sanitizer build/CTest logs.
   Latest Build additionally fails both Mac plugin installers after Accepted
   notarization, with CloudKit network error -1005 / stapler exit 68.
2. **AC-02 — verified:** The source-data fixture respects the approved distinction
   between authored Level L grammars and B/G routine providers. All 16 existing
   B/C/G/L optimized/unoptimized, linked/unlinked contracts pass in normal Debug
   and maintained Apple ASan; all byte, import and rejection assertions remain.
3. **AC-03 — verified:** Mac installer stapling retries only temporary service exit
   68, stops after four attempts, and requires validation before continuation.
   Fixture proof covers recovery, permanent failure, exhausted retries and
   validation failure, including paths containing spaces. All three production
   packaging entry points use the helper, retaining spctl assessment.
4. **AC-04 — verified:** Core stage-product and required QA prerequisites build;
   focused normal and sanitizer checks pass. Retain exact qualified source-input
   hashes, commands and receipts before the substantive first push. Reuse the
   passing unaffected overnight gates rather than repeating full matrices.
5. **AC-05 — publication ready:** Commit separate reviewable fixture and packaging repairs,
   publish them together to hotfix/develop after AC-02–04 pass, verify remote
   refs, and fast-forward the existing develop checkout preserving dirty work.
   Normal automatic Build/CodeQL are expected to pass; report observed results
   separately, with no status-only push. Actual failures require substantive
   diagnosis/repair. No manual full overnight dispatch is a publication gate.
6. **AC-06 — verified locally / native CI expected:** Select CRI-17 fixture TLS by compiler (_MSC_VER versus
   __thread), preserving Windows locks, counters and deliberate worker overlap.
   Pass both opt/noopt tests and both native negative controls in Debug and
   maintained ASan. Label opt/noopt tests smoke so normal publication checks
   verify the native MinGW repair. Inspect artifact dependencies so fresh
   focused/smoke builds stage the provider files they require.
   The failed warning and observed session
   aliasing are the baseline proof; runtime/product logic remains unchanged.

## Implementation steps

1. **STEP-01 — done (AC-01):** Inspect live run/job results, download failed logs
   and sanitizer artifacts, synchronize hotfix, consult the approved Level L
   source-selection plan and owning authoring/import/ASan documentation.
2. **STEP-02 — done (AC-02/04):** Rebuild stale local tools to reproduce the
   current fixture failure, use Level B for the routine provider in the L case,
   and run all unchanged source-byte assertions in Debug and ASan. This is QA
   fixture repair, preserving research/level-l/PLAN.md's accepted grammar policy.
3. **STEP-03 — done (AC-03/04):** Introduce one bounded stapling helper at the
   core, plugin and retained-plugin packaging entry points; test its strict
   failure/retry behavior with a controlled xcrun fixture. No signing secrets or
   upstream engine rebuild is needed for this policy check.
4. **STEP-04 — publication ready (AC-04/05):** Inspect final diffs, retain receipt and hashes,
   commit compatible causes separately, synchronize and push hotfix/develop,
   preserve occupied checkout changes and report publication verification.
5. **STEP-05 — done (AC-01/04/06):** Retain the separate MinGW TLS warning and
   failure, reuse the owning VM's compiler-specific TLS spelling in the provider
   fixture, run the unchanged concurrency/lifecycle controls, and include those
   tests in normal smoke CI. The existing dynamic/static plugin dependencies
   stage this fixture's provider files; it has no separate metadata target.
   An attempted extra dependency was rejected by the focused build and removed,
   restoring the exact CMake input already qualified. Publish this cause as a
   separate compatible commit.

## Failed source evidence

- [Deep Build QA](https://github.com/adesutherland/CREXX/actions/runs/38502097931)
- [Sanitizer QA](https://github.com/adesutherland/CREXX/actions/runs/38508471135)
- [Build CREXX](https://github.com/adesutherland/CREXX/actions/runs/38453819158)

Scheduled workflow metadata names master 77ba820c, but their assurance gate
checks out develop 50926edb8. The actual failures belong to that develop source.
The compact permanent receipt and failed excerpts are retained in
[docs/qa/overnight-2026-10-14/](../qa/overnight-2026-10-14/README.md).
Sanitizer failures are functional fixture
failures, not SAN findings. Linux's stop-on-failure means its full CTest coverage
is incomplete; no sanitizer-clean or release-ready claim is made.

Normal Debug qualification passed: 16/16 source-data contracts (108.59 s),
4/4 CRI-17 concurrency/lifecycle controls (0.68 s), and stage-product/qa-prep.
Maintained Apple ASan passed all the same cases in two nonoverlapping groups:
12 typed source contracts (121.50 s), then four Classic C contracts and four
CRI-17 controls (65.62 s), following successful prerequisite/fixture builds.
All six stapling policy cases and shell syntax checks passed. No code/test
inputs have changed since their recorded hashes in the receipt's inputs.json.

All required local publication gates are green. The three compatible causes
are committed separately and pushed together to existing hotfix/develop; remote
ref and occupied-checkout integrity verification completes AC-05 and is reported
live in the conversation. The substantive first push includes the final receipt
and expected successful normal automatic CI/snapshot publication. Native MinGW
and real Apple service confirmation belong to those automatic jobs; no status-
only repository push or full manual overnight rerun is planned.
