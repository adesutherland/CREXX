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
9. **CI-CS-AC-09 — verified, 2026-10-11:** SAN-QA-018 closes only after its permanent focused
   normal/sanitizer proof and full supported platform sanitizer gates. This
   concrete observed failure justifies a targeted repeat of Sanitizer QA;
   ordinary push checks remain separate. Keep this criterion visibly open
   while hosted platform proof runs; do not claim sanitizer-clean completion
   before those gates pass. The renewed full request below now satisfies them.
   **Adrian's explicit scope decision, 2026-10-10:** publish the fixes and
   assign remaining SAN-QA-018 platform proof to **Codex at the next Release 1
   release-QA gate**. This was outside the original bounded publication phase.
   Adrian's subsequent immediate full-QA request is fulfilled by the closure
   disposition linked below; no release-candidate or release claim is made.

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
8. **CI-CS-STEP-08 — complete through renewed full gate; AC-09:** after focused normal/ASan proof, publish
   compatible build/harness and numeric repair commits together. Per Adrian's
   explicit 2026-10-10 decision, **Codex originally owned full SAN-QA-018 platform closure
   at the next Release 1 release-QA gate**. The renewed request below completes
   it now. Reuse unchanged valid results and
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

10. **CI-CS-AC-10 — verified:** Windows MSVC and MinGW build the Classic source
    broker with no Windows/AST enum collisions. Preserve numeric AST/inline
    payload identities and typed source-import/editor behavior. Retain failed
    compile logs, a minimal namespace reproducer, focused local frontend/contract
    controls and actual repaired Windows product/deep jobs.
11. **CI-CS-AC-11 — verified:** the normal Build and CodeQL workflows pass for the
    repaired develop delivery, including Intel Mac signing/notarization. The
    observed Apple offline/no-route error is treated as a transport failure;
    retry that operation/job if needed without waiving signing or notarization.
    Both workflows pass on `a69745746`, including Intel Mac notarization and
    all optional-plugin consumer packages. The next substantive repair will
    receive its own normal automatic publication checks.
12. **CI-CS-AC-12 — verified:** full Sanitizer QA passes Linux x64 ASan/LSan with
    build/test leaks on and macOS arm64 ASan with the documented Apple LSan
    capability limit. Full Deep Build QA passes all five core platforms,
    comprehensive/qualification checks, Linux books and parallel product graph
    checks. Only the established performance-measurement sanitizer exclusion is
    allowed. Retain exact code/test/build identity; unchanged valid results may
    be reused across history/documentation-only changes.
13. **CI-CS-AC-13 — verified product proof; final documentation publication ready:** monitor lightly until every required workflow is
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
10. **CI-CS-STEP-10 — complete; AC-10:** retain the minimal collision control, reuse
    the existing compiler header-isolation pattern, then run focused normal
    Debug and maintained ASan frontend/import/contract checks. This is an
    implementation repair, not a new syntax or architectural design decision.
11. **CI-CS-STEP-11 — complete; AC-11/12:** prepare the fix receipt and expected
    publication status before the first push; publish the combined repair and
    dispatch full Deep Build QA (platform=all) and Sanitizer QA on the repaired
    revision. Normal Build/CodeQL run automatically. Depends on STEP-10.
12. **CI-CS-STEP-12 — complete; final documentation publication ready; AC-11/12/13:** use infrequent status reads, retain
    failed job logs/artifacts, and fix distinct reproduced causes in reviewable
    commits. Retry transient service failures narrowly. Do not cancel healthy
    work, busy-poll or repeat unchanged valid broad evidence. Close only after
    all explicitly requested gates pass; update sanitizer disposition only from
    actual focused/original/full-platform evidence.

**Local checkpoint before publication:** collision control and numeric enum identity control pass;
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

14. **CI-CS-AC-14 — verified:** both actual Windows Deep lanes pass the unchanged
    Classic frontend parity and Unicode contract, then install/package checks.
    Embed the standard UTF-8 process manifest in the core CLI executables for
    MSVC and MinGW. Record its Windows 10 version 1903+ capability boundary;
    do not claim older Windows Unicode path qualification.
15. **CI-CS-AC-15 — verified:** broker argv serialization preserves empty arguments,
    whitespace, quotes and backslash runs, including trailing backslashes.
    Retain permanent focused controls of the production quoting helper in
    Debug and maintained ASan, plus the existing end-to-end typed import proof.
    Reuse unchanged Unix full-gate evidence; dispatch only the repaired Windows
    Deep scope after the new publication. Normal Build/CodeQL remain required.

13. **CI-CS-STEP-13 — complete; AC-14/15:** retain failed logs/artifacts and
    reconcile exact failed test inventories. The permanent workloads already
    reproduce both observed causes; namespace compilation repair is verified
    by normal Windows product builds. No memory diagnostic is reported here.
14. **CI-CS-STEP-14 — complete; AC-14/15:** embed the standard Windows process
    manifest, quote each broker argument using CRT rules, retain focused
    boundary controls and Debug-then-ASan evidence. Preserve process ownership,
    authored diagnostics and frontend/metadata separation.
    Debug and maintained Apple ASan pass the nine-vector production quoting
    helper (1/1, 0.38 s / 0.40 s); Debug compiler entry points build cleanly.
    All 15 matrix/package workflow controls pass. Unix preprocessing is
    byte-identical to `a69745746`; the added manifest acts only on Windows.
15. **CI-CS-STEP-15 — complete; AC-11–15:** publish the bounded Windows repair with
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

## Mandatory sanitizer disposition and final acceptance — 11 October 2026

The required [SAN-011/012/SAN-QA-018 closure disposition](../../SANITIZER-WORKLIST.md#11-october-closure-disposition)
verifies the retained focused Debug/maintained-sanitizer commands, their input
hashes, the original generators in fresh full builds, and both complete full
sanitizer platforms. Linux passes 3,122/3,122 with build/test leaks on; macOS
passes 3,126/3,126 with its documented unavailable LSan. No sanitizer finding
or failed/timed-out test appears in the downloaded build/preparation/CTest logs.
This fulfills AC-09 and the sanitizer parts of AC-12/13; the historical
later-gate assignments and bounded receipts above retain their original scope.

Published product `5e8275768355fa3dc84c24e815de91d6d63a6ca3` passes normal
[Build CREXX](https://github.com/adesutherland/CREXX/actions/runs/38090392553)
and [CodeQL](https://github.com/adesutherland/CREXX/actions/runs/38090392224).
The [paired Windows Deep replay](https://github.com/adesutherland/CREXX/actions/runs/38090414968)
passes 2,891/2,891 MSVC and 3,011/3,011 MinGW correctness checks, plus each
lane's 3/3 install/package/external-consumer checks. Both previously failing
Classic tests pass on both Windows toolchains. The unchanged successful Unix,
books, isolated stress and product/RXBIN graph jobs from
[full Deep 38086282692](https://github.com/adesutherland/CREXX/actions/runs/38086282692)
complete the full requested Deep scope. The Windows receipt's byte-identical
Unix preprocessing, unchanged matrix controls and focused helper proof retain
[full Sanitizer QA](https://github.com/adesutherland/CREXX/actions/runs/38086283792)
on `a69745746` for the final Unix product inputs. This verifies AC-10–12/14/15.

All product and finding-closure proof is complete. Expected final publication is
the required canonical sanitizer dispositions and this acceptance record on
synchronized `origin/develop`, with a clean workspace and the heartbeat paused.
Only these documentation records change after `5e8275768`; their publication
reuses the qualified non-documentation tree hash recorded in the worklist and
does not launch duplicate CI through a status-only push. GitHub's completed
runs remain the workflow evidence. No new product change, release, unsupported
Apple leak coverage, older-Windows Unicode-path qualification or upstream
inference-engine sanitizer coverage is claimed.
