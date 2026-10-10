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

## Renewed pipeline regression and explicit full QA — 10 October 2026

Adrian requested diagnosis, full sanitizer/deep QA and light monitoring with
repairs until green after Build CREXX 38083859225 failed on synchronized
`64e4a30a16a1207d2d0834e6053ff69df30bfe39`. This extends this authoritative
pipeline plan; the original AC/STEP IDs and prior receipts remain unchanged.
The earlier bounded-publication/later-gate decision does not substitute for the
full gates now explicitly requested. Current checkout is clean and matches
origin/develop. Evidence: `output/ci-regression-20261010.79vob9/`.

**Vision and intended outcome:** restore green current develop delivery and
complete full supported sanitizer and deep assurance while preserving the
approved Unicode/ordinal/stream and Classic compiler split/metadata contracts.
Fix actual failures at their owning sources, retain diagnostics, and distinguish
transport/service failures from product failures. Do not suppress tests,
change supported inline payload identities, or rebuild healthy jobs merely to
obtain more green results. Publish compatible fixes together and inspect jobs
at roughly ten-minute intervals while independent useful work continues.

**Additional numbered acceptance criteria:**

10. **CI-CS-AC-10 — open:** Windows MSVC and MinGW build the Classic source
    broker with no Windows/AST enum collisions. Preserve numeric AST/inline
    payload identities and typed source-import/editor behavior. Retain failed
    compile logs, a minimal namespace reproducer, focused local frontend/contract
    controls and actual repaired Windows product/deep jobs.
11. **CI-CS-AC-11 — open:** the normal Build and CodeQL workflows pass for the
    repaired develop delivery, including Intel Mac signing/notarization. The
    observed Apple offline/no-route error is treated as a transport failure;
    retry that operation/job if needed without waiving signing or notarization.
    Both workflows pass on `a69745746`, including Intel Mac notarization and
    all optional-plugin consumer packages. The next substantive repair will
    receive its own normal automatic publication checks.
12. **CI-CS-AC-12 — open:** full Sanitizer QA passes Linux x64 ASan/LSan with
    build/test leaks on and macOS arm64 ASan with the documented Apple LSan
    capability limit. Full Deep Build QA passes all five core platforms,
    comprehensive/qualification checks, Linux books and parallel product graph
    checks. Only the established performance-measurement sanitizer exclusion is
    allowed. Retain exact code/test/build identity; unchanged valid results may
    be reused across history/documentation-only changes.
13. **CI-CS-AC-13 — open:** monitor lightly until every required workflow is
    green; diagnose/reproduce and fix any new failure, record any first-party
    sanitizer finding immediately with its stable SAN entry and closure gates,
    and preserve unresolved SAN-011/012/SAN-QA-018 status until their recorded
    proof is satisfied. Finish with synchronized published develop and a clean
    workspace, reporting actual results and remaining scope accurately.

**Additional numbered implementation steps:**

9. **CI-CS-STEP-09 — complete; AC-10/11:** inspect current and original failed
   runs plus current owning docs; retain all failed logs and reconcile earlier
   fixes. Windows fails in rxcp_c_source_import.c through FLOAT/PATTERN/DECIMAL
   enum collisions with windows.h. Intel Mac failed notarytool --wait when its
   network route disappeared. The older missing stream archive is repaired.
10. **CI-CS-STEP-10 — local controls complete; hosted Windows proof open; AC-10:** retain the minimal collision control, reuse
    the existing compiler header-isolation pattern, then run focused normal
    Debug and maintained ASan frontend/import/contract checks. This is an
    implementation repair, not a new syntax or architectural design decision.
11. **CI-CS-STEP-11 — publication ready; AC-11/12:** prepare the fix receipt and expected
    publication status before the first push; publish the combined repair and
    dispatch full Deep Build QA (platform=all) and Sanitizer QA on the repaired
    revision. Normal Build/CodeQL run automatically. Depends on STEP-10.
12. **CI-CS-STEP-12 — open; AC-11/12/13:** use infrequent status reads, retain
    failed job logs/artifacts, and fix distinct reproduced causes in reviewable
    commits. Retry transient service failures narrowly. Do not cancel healthy
    work, busy-poll or repeat unchanged valid broad evidence. Close only after
    all explicitly requested gates pass; update sanitizer disposition only from
    actual focused/original/full-platform evidence.

**Current checkpoint:** collision control and numeric enum identity control pass;
normal Debug and maintained Apple ASan frontend/contract panels pass 4/4
(48.17 s / 119.26 s). The repair receipt is
[ci-windows-classic-broker-2026-10-10](../../qa/ci-windows-classic-broker-2026-10-10/README.md).
The substantive fix is ready for the first push with expected green normal
publication and explicitly requested full sanitizer/deep workflows. Actual
hosted proof and AC-10–13 remain open until verified. Thread heartbeat
`repair-crexx-ci-until-green` checks/resumes every ten minutes, with current
phase/revision/run IDs retained in the ignored evidence active-runs.json.

## Windows host-text boundary exposed by full Deep QA

Full Deep run `38086282692` on `a69745746` passed Linux, both Macs,
books, stress and all product-graph comparisons. Both Windows lanes built
successfully but failed exactly `classic_frontend_parity` and
`classic_unicode_contract_opt`. Their retained LastTestsFailed/LastTest logs
are under the evidence directory. The first failure occurs when the Classic
broker treats ACP-encoded path arguments as UTF-8; the second loses the
Unicode provider filename to `??.rexx` during narrow CRT startup. The broker
also passes unquoted spaced paths to `_wspawnv`, whose documented argument
serialization splits them. These are host encoding/argument boundary repairs,
not changes to language syntax, stream codecs or metadata.

**Intended outcome:** core Windows tools exchange UTF-8 command-line and
filesystem names on the supported modern Windows runners; the typed broker
retains every argument, including spaces, backslashes and Unicode, without a
shell. Existing Latin byte ordinals and explicit external stream encodings
remain unchanged. Keep the existing Unicode filename regression enabled.

14. **CI-CS-AC-14 — open:** both actual Windows Deep lanes pass the unchanged
    Classic frontend parity and Unicode contract, then install/package checks.
    Embed the standard UTF-8 process manifest in the core CLI executables for
    MSVC and MinGW. Record its Windows 10 version 1903+ capability boundary;
    do not claim older Windows Unicode path qualification.
15. **CI-CS-AC-15 — open:** broker argv serialization preserves empty arguments,
    whitespace, quotes and backslash runs, including trailing backslashes.
    Retain permanent focused controls of the production quoting helper in
    Debug and maintained ASan, plus the existing end-to-end typed import proof.
    Reuse unchanged Unix full-gate evidence; dispatch only the repaired Windows
    Deep scope after the new publication. Normal Build/CodeQL remain required.

13. **CI-CS-STEP-13 — complete; AC-14/15:** retain failed logs/artifacts and
    reconcile exact failed test inventories. The permanent workloads already
    reproduce both observed causes; namespace compilation repair is verified
    by normal Windows product builds. No memory diagnostic is reported here.
14. **CI-CS-STEP-14 — local controls complete; hosted proof open; AC-14/15:** embed the standard Windows process
    manifest, quote each broker argument using CRT rules, retain focused
    boundary controls and Debug-then-ASan evidence. Preserve process ownership,
    authored diagnostics and frontend/metadata separation.
    Debug and maintained Apple ASan pass the nine-vector production quoting
    helper (1/1, 0.38 s / 0.40 s); Debug compiler entry points build cleanly.
    All 15 matrix/package workflow controls pass. Unix preprocessing is
    byte-identical to `a69745746`; the added manifest acts only on Windows.
15. **CI-CS-STEP-15 — open; AC-11–15:** publish the bounded Windows repair with
    complete receipt and expected publication state. Add a paired Windows-only
    Deep selection so both failed lanes run concurrently without repeating
    healthy Unix/books/stress/graph work. Verify ordinary matrix selection is
    unchanged. Leave the running Linux/macOS sanitizer jobs alone; their
    original platform inputs are unaffected by Windows-only execution changes.

The Windows host-text repair receipt is
`docs/qa/ci-windows-hosttext-2026-10-10/README.md`. Expected first-push status is
green normal Build/CodeQL with the paired Windows-only Deep replay launched.
Actual hosted acceptance remains open, with exact run IDs in active-runs.json.
Existing full Unix sanitizer proof plus the newly added focused helper covers
the final Unix delta; no unchanged full lane is restarted merely for a new SHA.
