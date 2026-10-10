# Core stream packaging repair, 10 October 2026

## Vision and intended outcome

Restore normal `develop` snapshot delivery after Build CREXX run
[37987446535](https://github.com/adesutherland/CREXX/actions/runs/37987446535)
failed all four optional-plugin consumer jobs at revision `10e8184e3`.
The qualified core ZIP contains `rxcstream.rxplugin` but omits the canonical
static archive needed when the native driver links the Classic runtime.
Fresh product selections must build both forms, and extracted core ZIP testing
must exercise native encoded streams before the core is accepted for consumers.
The inventory also includes scheduled Deep Build QA `38018313879` and
Sanitizer QA `38022249457`, both checking the same `develop` revision (their
top-level schedule SHA belongs to the default branch). Complete pipeline repair
includes late test preparation, explicit UTF-8 fixture decoding on Windows and
exact whole-number BIF consumption under SAN-QA-018. All repairs preserve the
implemented Classic contract; no syntax or language-design decision is needed.

## Acceptance criteria

1. **CI-CS-AC-01 — verified:** all four failing jobs identify the same missing
   `rxcstream.a`/`.lib`; the actual downloaded macOS ARM64 core ZIP and its
   manifest confirm the omission. Retain the failing run and archive inspection
   in the development receipt.
2. **CI-CS-AC-02 — verified:** the product build graph selects `cstream_static`,
   and an ordinary product build produces the canonical and compatibility
   archives without relying on an earlier full/test build. Verify the generated
   graph and force the affected target's output to be rebuilt through the
   normal stage selection.
3. **CI-CS-AC-03 — verified:** the permanent core ZIP smoke compiles a Classic
   encoded-stream native consumer and runs its executable after relocation.
   Exact file bytes and decoded text must match. Replay against the failed
   downloaded ZIP (must fail), then the same payload with only the repaired
   stream archive restored (must pass).
4. **CI-CS-AC-04 — verified:** relevant package/workflow regressions pass; changed
   Python sources compile; retained evidence records the qualified code/build/
   test inputs and expected publication state. Reuse the unchanged successful
   product and optimizer checks from the failed workflow.
5. **CI-CS-AC-05 — publication-ready; verified by this changeset's push:** commit the repair, regression and receipt together
   on synchronized `develop` and push to `origin/develop`. The normal automatic
   Build CREXX and CodeQL workflows are expected to pass; their actual results
   belong to GitHub run records. Hosted completion is not an extra publication
   gate for this build-only repair, and no manual deep/sanitizer dispatch is
   required.
6. **CI-CS-AC-06 — verified:** all compiler tests, including the final Classic
   lifecycle harness, contribute their declared producer to QA preparation.
   Inspect the generated comprehensive prep graph and replay the lifecycle and
   native ADDRESS smoke after their producer build.
7. **CI-CS-AC-07 — verified:** the Unicode harness reads UTF-8 repository fixtures
   explicitly on Windows. A CP1252-default control must fail with the old code
   and pass with the repaired code; unchanged generated source remains UTF-8.
8. **CI-CS-AC-08 — verified:** the signed-64-bit maximum integer fixture passes
   with exact conversion in all four modes. Preserve existing output and
   range-error behavior, retain generated-code inspection, and replay relevant
   BIF checks in normal Debug and the maintained sanitizer build.
9. **CI-CS-AC-09 — open:** SAN-QA-018 closes only after its permanent focused
   normal/sanitizer proof and full supported platform sanitizer gates. This
   concrete observed failure justifies a targeted repeat of Sanitizer QA;
   ordinary push checks remain separate. Keep this criterion visibly open
   while hosted platform proof runs; do not claim sanitizer-clean completion.
   **Adrian's explicit scope decision, 2026-10-10:** publish the fixes and
   assign remaining SAN-QA-018 platform proof to **Codex at the next Release 1
   release-QA gate**. This criterion remains open for that gate, outside the
   bounded implementation/publication phase; no closure/clean claim is made.

## Implementation steps

1. **CI-CS-STEP-01 — complete; AC-01:** inspect current GitHub failures and
   owning packaging/build sources; download the exact failed core ZIP.
2. **CI-CS-STEP-02 — complete; AC-02:** add the static stream provider to the
   existing product stage that owns the Classic runtime; inspect/rebuild that
   selection. Depends on STEP-01.
3. **CI-CS-STEP-03 — complete; AC-03:** add the documented Classic encoded-stream
   fixture and run it as a relocated native consumer in core ZIP smoke. Replay
   missing/restored archive controls. Depends on STEP-01; successful repair
   replay depends on STEP-02.
4. **CI-CS-STEP-04 — complete; AC-04:** run package/workflow regressions and retain
   the focused build and actual-ZIP evidence with exact input hashes. Depends
   on STEP-02/03. Do not repeat unchanged broad compiler/runtime testing.
5. **CI-CS-STEP-05 — publication-ready; AC-05:** review the diff, synchronize `develop`,
   commit and push the complete repair. Depends on STEP-04; report hosted
   status accurately without a subsequent status-only commit.
6. **CI-CS-STEP-06 — complete; AC-06/07:** finalize QA registration after every
   compiler test and select UTF-8 in Unicode fixture reads. Preserve workload
   and scheduling; use focused missing-target and CP1252 controls.
7. **CI-CS-STEP-07 — complete; AC-08:** use the documented direct string-to-integer
   cast in validated whole-number BIF consumers, preserving RexxDoc/API tags.
   Retain existing boundary reproducer and inspect generated conversion ops.
   Register SAN-QA-018 before unrelated closeout. No compiler/VM core rewrite
   or language design gate is required for this contract repair.
8. **CI-CS-STEP-08 — assigned; AC-09:** after focused normal/ASan proof, publish
   compatible build/harness and numeric repair commits together. Per Adrian's
   explicit 2026-10-10 decision, **Codex owns full SAN-QA-018 platform closure
   at the next Release 1 release-QA gate**. Reuse unchanged valid results and
   report platform proof independently of bounded implementation completion.

## Retained evidence

The failing full job log is `/tmp/crexx-ci-failure.a8oRah`. The downloaded
macOS ARM64 core ZIP is under `/tmp/crexx-ci-artifacts.LftFtu/`.
The permanent development receipt is
`docs/qa/ci-core-stream-packaging-2026-10-10/README.md`.

The receipt now retains original/repaired/final ZIP controls and exact input
hashes. Final Debug and Release product builds pass; all 24 changed sources'
complete RexxDoc blocks are unchanged. The CP1252-default original control
fails as expected, and the final full Unicode contract passes (156 commands,
31.68 s, including exact INT64_MAX stream positions). The final extracted ZIP includes the repaired integer runtime and
passes both relocated native consumers. Normal BIF correctness uses 169
selected checks and their own exact producer targets: changing 54 conversions
across 24 sources warrants the full BIF family, while unrelated compiler/VM
source and prior successful product/parity evidence remain reusable.
Full platform sanitizer proof remains explicitly open under SAN-QA-018.
The 169-check normal BIF panel passes in 294.94 seconds. The exact final
six-check integer/lifecycle/native panel passes in normal Debug (19.05 seconds)
and maintained Apple ASan (30.65 seconds), following instrumented product and
lifecycle builds. The receipt retains the final six stream-cast delta and
unchanged panel reuse. All required local implementation/publication gates pass.
Expected final state is both compatible repair commits and their complete
receipt published in one push to `origin/develop`. AC-09 remains open for
Codex's explicitly assigned next Release 1 release-QA gate.
