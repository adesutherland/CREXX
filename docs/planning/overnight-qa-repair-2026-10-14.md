# Overnight QA repair — 14 October 2026

## 22 October continuation — MSVC sustained-VM harness compilation

### Vision and intended outcomes

Restore the overnight Windows MSVC correctness lane on the existing hotfix
branch and publish the bounded repair to develop. The sustained-VM harness
includes private VM headers and must declare their C11/atomic requirements
on its own target, including when defined outside the interpreter directory.
Keep runtime behavior, sustained workload/oracles, platform coverage and the
approved sanitizer cadence unchanged. Preserve unrelated local work and
private develop commits; this is a build repair, not release promotion.

### Acceptance criteria

1. **O22-AC01 — done:** identify the actual failing source and generated command
   in the overnight MSVC job. The failure is compilation of
   `test_sustained_vm.c` without C11 or the MSVC atomic option, before CTest;
   the four other comprehensive platforms passed.
2. **O22-AC02 — implemented; actual MSVC proof pending:** declare C11 and the MSVC atomic option on each sustained
   harness target. Extend the existing real-MSVC preflight to require the
   sustained harness command as well as both attached-provider commands;
   each unchanged positive command compiles and removing only atomic support
   fails with the expected diagnostic. Run that preflight during normal MSVC
   publication before building the product as well as in overnight QA.
3. **O22-AC03 — done:** refresh the existing normal Debug core build and affected
   harness/fixtures, inspect generated commands, and pass the three existing
   sustained scenarios on the switch VM. No native ownership algorithm changes
   or new sanitizer campaign are required for target compile settings.
4. **O22-AC04 — publication prepared; hosted status pending:** review and publish only this repair and its durable plan
   on hotfix/develop. Verify remote identities, inspect normal automatic CI,
   and use its targeted MSVC preflight to verify the actual Windows compiler.
   Keep pending broad platform assurance explicit and remove task diagnostics.

### Implementation steps

1. **O22-STEP01 — done (AC01):** inspect the overnight jobs/log, read the owning
   build declarations and preflight, synchronize hotfix with remote develop,
   and record the bounded contract before implementation.
2. **O22-STEP02 — done locally (AC02):** apply target-local C11/atomic settings following
   the existing private-header consumer pattern and extend preflight discovery
   with exact required-source counts.
3. **O22-STEP03 — done (AC03), depends on STEP02:** reuse the prepared Debug tree,
   rebuild current core plus affected prerequisites and run the three existing
   switch-VM scenarios; inspect the final diff and generated commands.
4. **O22-STEP04 — publication prepared (AC04), depends on STEP03:** publish the reviewed repair,
   inspect the normal publication actual-MSVC preflight, verify refs and report
   hosted status. Do not dispatch a full deep or sanitizer matrix for this target-only repair.

## Current baseline recovery — Windows fixture paths and Apple ticket transport

### Vision and intended outcomes

Restore a publishable baseline on the existing hotfix/develop branches without
changing product semantics, the approved sanitizer selection or schedule policy.
Windows fixture ownership must accept a build rooted in a valid DOS short path
and reject outputs outside that tree. Miniature diagnostic tests must select
their generator explicitly. Apple installers retain notarization, stapling,
ticket validation and Gatekeeper checks; address the Intel hosted-runner
HTTP/3 transport failure with GitHub's documented TCP fallback, accepted by
Adrian. Preserve private research and Python-removal drafts.

### Acceptance criteria

1. **B21-AC01 — local controls done; Windows execution todo:** normalize both fixture containment operands consistently;
   forwarding, whole-owner failure and configuration registries pass on Windows
   and in focused ordinary Debug / maintained Apple ASan controls. Retain
   outside-tree rejection and original producer argument boundaries.
2. **B21-AC02 — done:** the late-diagnostic fixture runs with an explicit Ninja
   generator even when the host defaults to a multiconfig generator. Its
   semantic-pass negative control still rejects late sanitizer diagnostics.
3. **B21-AC03 — accepted; local controls done; hosted proof todo:** restrict the Apple transport workaround to disposable
   GitHub-hosted Intel Mac packaging jobs. A controlled shell fixture verifies
   the UDP/443 rule, enablement, platform restrictions and error propagation;
   notarization/stapling gates and bounded retries remain mandatory.
4. **B21-AC04 — todo:** publish compatible repairs, inspect actual Windows and
   Intel Mac results and confirm dev-snapshot publication. Reuse unchanged
   sanitizer proof; do not dispatch a new full sanitizer or deep matrix. Adrian's
   subsequent schedule-activation request is owned by SCOPE-AC07 in the sanitizer
   worklist; preserve that explicit workflow-only scope and remove task diagnostics.

### Implementation steps

1. **B21-STEP01 — done (AC01/02/03):** inspect the failed jobs and retained Windows
   CTest diagnostics; compare fixture paths/generator handling and GitHub's
   confirmed Intel HTTP/3 failure in actions/runner-images#14924.
2. **B21-STEP02 — done locally (AC01/02):** repair containment normalization and pin the
   miniature generator; run existing policy/ownership and late-failure controls
   in ordinary Debug then maintained Apple ASan. Preserve native Windows proof
   as a separate outstanding check.
3. **B21-STEP03 — done locally; accepted by Adrian (AC03):** prepare a
   runner-restricted Bash helper and controlled shell test; include the approved
   network change at the Intel core/plugin packaging boundaries.
4. **B21-STEP04 — todo (AC04), depends on STEP02 and the STEP03 decision:** push
   the compatible repair set to hotfix/develop, monitor the normal publication
   jobs lightly and reconcile occupied checkouts without publishing private work.

## 18 October continuation — binary-import registration and Windows service oracle

### Vision and intended outcomes

Adrian requested work in the existing hotfix checkout, synchronization with
HEAD, repair of failing overnight QA and publication to develop. Hotfix was
fast-forwarded to develop `ff97dbfbb443a2dd44f9ed08c007186ec326776a`, preserving
unrelated files in both checkouts. Restore the existing import regressions and
the independent Level L CLI/service comparison on native Windows. Preserve
compiler/runtime/language semantics, exact service artifact bytes, and strict
binary-file publication assertions. Publish the qualified repairs together.

### Acceptance criteria

1. **O18-AC01 — verified:** account for every failed job in
   [Deep Build QA 39090761473](https://github.com/adesutherland/CREXX/actions/runs/39090761473)
   and [Sanitizer QA 39095054676](https://github.com/adesutherland/CREXX/actions/runs/39095054676).
   Both qualified develop `db2fabdd6cbd036874a27fcb6d14832a696e647e`, despite
   scheduled-workflow metadata naming master. All five Deep platforms fail
   three misspelled `driverary_*.cmake` paths; Unix platforms also fail the
   standalone bootstrap install-helper call. Windows service cases report
   only `CLI/service parity`. macOS ASan fails the same three paths and
   bootstrap fixture; Linux stops at the first missing path. Inspected build
   and CTest sanitizer artifacts contain no ASan/LSan or UBSan diagnostic.
2. **O18-AC02 — verified:** register the three existing `binary_*.cmake` scripts
   correctly; their original assertions pass in current normal Debug and
   maintained Apple ASan. Preserve the existing serial scheduling/900-second
   backstop for the global-import matrix; no matrix expansion.
3. **O18-AC03 — verified locally:** reproduce Windows-style CRLF CLI output against the
   existing service fixture, then compare the CLI text after CRLF-to-LF
   conversion with the exact service artifact. Preserve every other byte and
   binary-publication assertion. Opt/noopt cases on both VMs pass with native
   and CRLF CLI oracles. Include these small service cases in normal smoke QA
   so Windows publication checks exercise the repaired comparison.
4. **O18-AC04 — verified locally:** current core `stage-product` and focused prerequisites
   build without warnings; affected import correctness and service regressions
   pass. Reuse the unchanged bootstrap/oracle-preparation proof at
   `d166e13c2`/`ff97dbfbb` recorded in `research/level-l/PLAN.md` rather than
   reimplementing or repeating it. Record commands, qualified inputs, results
   and platform limits here before the substantive push. No full manual
   overnight dispatch is needed for these QA-only repairs.
5. **O18-AC05 — publication prepared:** publish separate reviewable causes together to hotfix
   and develop, verify remote refs and synchronize the existing develop
   checkout while preserving its unrelated dirty files. Normal automatic
   Build/CodeQL are expected to pass; report only observed hosted outcomes,
   without status-only commits. Delete task-created temporary diagnostics.

### Implementation steps

1. **O18-STEP01 — done (AC01):** inspect jobs and single temporary copies of
   failed logs/artifacts, synchronize hotfix, inspect the owning plans,
   authoring guide and sanitizer protocol, and identify already integrated
   repairs. Record this continuation before implementation.
2. **O18-STEP02 — done (AC02/04):** correct the three script references,
   regenerate current build declarations, and run their unchanged Debug and
   maintained-ASan tests with refreshed product prerequisites.
3. **O18-STEP03 — done (AC03/04):** reproduce the CRLF oracle discrepancy,
   correct only the test's platform text comparison, retain the independent
   service assertions, label service smoke coverage, and run both oracle
   forms and the relevant ordinary correctness panel. No language or
   architecture decision is introduced.
4. **O18-STEP04 — locally qualified; publication prepared (AC04/05):** summarize final qualification and limits,
   review diffs, commit each cause, check develop for concurrent changes,
   publish and reconcile both checkouts without discarding unrelated work.

### Qualification and publication

Qualified inputs: base `ff97dbfbb443a2dd44f9ed08c007186ec326776a` plus the
three CMake script-name corrections, service fixture/registration changes and
the current documentation diff. Compiler, runtime and generator implementation
inputs are unchanged by this repair.

- Normal Debug `stage-product`, `rxvme` and `compiler_exit_bin` completed;
  `cmake --build cmake-build-debug --target qa-prep-smoke --parallel 8`
  then passed without warnings. The first multi-target Make invocation refreshed
  the old build graph but could not resolve the newly introduced service target
  from its initially loaded Makefile; the fresh preparation invocation resolved
  and built both service targets. This was build-tree refresh, not a product
  failure or a source repair.
- `ctest --test-dir cmake-build-debug --tests-from-file <selection>
  --parallel 30 --output-on-failure` passed **213/213**, 621.12 seconds.
  The disposable selection contained every `essential`/`smoke` test plus the
  seven import cases named in O18-AC02/STEP03: the three binary tests,
  `global_import_types`, both `source_import_private_dependency_*` cases and
  `inline_return_conversions`. This exercises the compiler, assembler, linker
  and both VM dispatches; the original import assertions remain intact.
- Before repair, injecting CRLF into the existing generated CLI oracle at the
  base revision made all four opt/noopt and VM service cases fail only
  `CLI/service parity`, matching the native Windows logs. With the repair,
  all four CRLF cases pass; changing one other ASCII byte makes all four fail
  the same parity assertion. Native-LF cases pass in the 213-test panel.
  Private oracle and binary-output files are automatically removed.
- Maintained Apple ASan prerequisite build passed via `tools/asan-run.sh
  --build-dir cmake-build-debugasan --phase build --build-target stage-product
  --build-target rxvme --build-target compiler_exit_bin --build-target
  levell_services_opt --build-target levell_services_noopt --build-jobs 6
  --build-leaks off --leaks off --no-live-tail`. It emits existing Apple SDK
  `sprintf` deprecation warnings in unchanged C sources; no suppression or
  product change is introduced here. Apple LeakSanitizer is unsupported.
  The focused command below passed **11/11**, 482.54 seconds, without an
  AddressSanitizer diagnostic. The global-import matrix passed in 336.78
  seconds under its unchanged serial scheduling and 900-second limit.
  This is focused local Apple ASan proof, not a full sanitizer, Linux leak,
  native Windows or release qualification.
- The separate latest completed Build failure at
  [39108991776](https://github.com/adesutherland/CREXX/actions/runs/39108991776)
  accepted notarization and recovered stapling on its fourth attempt, then
  exhausted all four validation attempts on CloudKit connection loss
  (`NSURLErrorDomain -1005`, exit 68). The bounded retry helper behaved as
  specified. Keep its strict validation gate; this external service failure
  does not justify changing retry policy or bypassing validation.

Focused sanitizer command (the same seven import cases and four service cases
also passed in the normal Debug selection):

```sh
tools/asan-run.sh --build-dir cmake-build-debugasan --phase ctest \
  --regex '^(binary_(forward_dependencies|global_import_types|storage_selector_dependencies)|global_import_types|source_import_private_dependency_.*|inline_return_conversions|levell_services_.*)$' \
  --test-jobs 4 --build-leaks off --leaks off --no-live-tail
```

Final diff review and generated CTest inspection confirm the three script
paths exist, all four service cases carry smoke coverage, and the existing
global-import scheduling/timeout is unchanged. Both integrated preparation
repairs retain their earlier qualification. All required local publication
gates pass. Prepare the two distinct causes as separate commits and publish
them together to hotfix/develop; normal automatic Build/CodeQL are expected
to pass, with real Apple ticket-service availability remaining external.
Remote-ref and occupied-checkout integrity verification completes O18-AC05
and is reported live, without a status-only repository push. Temporary command
logs, downloaded artifacts, oracle files and test workspaces are deleted after
extracting this summary. The unrelated develop work and hotfix Python cache
are preserved.

## 17 October continuation — validation ticket-service recovery

The authorised CI monitor observed a remaining gap in AC-03 at develop
`e9d614b17092618674de6349c974d966d623292d`. The
[Intel Mac job](https://github.com/adesutherland/CREXX/actions/runs/39065095277/job/117500123658)
passed all 205 tests and extracted-core/SDK packaging; Apple accepted notarization
and stapling succeeded. The separate validation request then lost its CloudKit
connection (`NSURLErrorDomain -1005`, exit 68), outside the helper's retry loop.

**Outcome:** recover temporary ticket-service failures in either stapling or
validation while retaining strict publication gates. Extend AC-03's existing
four-attempt, exit-68-only policy to each action independently; never repeat
successful stapling merely because validation failed. Every other error and
exhausted retry must propagate, and all callers retain their `spctl` assessment.
Work in the existing main develop checkout under the 17 October repair authority;
the historical hotfix instructions below describe the earlier delivery only.

6. **STEP-06 — done (AC-03):** extend the controlled command fixture to cover
   validation recovery, both retry caps, permanent errors after a transient error,
   exact command ordering, diagnostics, delays, usage and paths with spaces;
   then share the bounded retry implementation between the two actions.
7. **STEP-07 — locally qualified, publication prepared (AC-04/05):** run the
   focused fixture and shell syntax checks, reuse unchanged product/build
   qualification, update the signing guide,
   publish only this repair to develop and retarget CI. Delete temporary output;
   local command fixtures do not establish real Apple-service success.

**Qualification:** against the unchanged helper at `e9d614b17092618674de6349c974d966d623292d`,
the new validation-recovery case failed with exit 68 as expected. With the helper
change, `python3 -B scripts/test-macos-stapling.py` passed all 14 policy cases.
`bash -n scripts/staple-macos-installer.sh scripts/repackage-macos-plugin.sh`
also passed. The fixture checks exact command order, per-action attempt limits,
delays, exit status, visible diagnostics and argument handling without live Apple
requests. No workflow, product, compiler, runtime or build input changed. Reuse
the preceding warning-free Debug `stage-product` build and this revision's hosted
core product, functional smoke and package-smoke passes; only the signing helper,
its controlled fixture and documentation change. This is local retry-policy proof,
not real Apple-service, sanitizer, cross-platform or release qualification.
Normal push-triggered Build/CodeQL will verify publication; report their outcome
in conversation without a status-only follow-up commit. Temporary diagnostics
are deleted after extracting this finding.

## Vision and intended outcomes

Restore the overnight correctness checks and development publication after the
14 October Deep Build QA and Sanitizer QA failures. Work in the existing hotfix
checkout, synchronized to develop 50926edb8b6b36e98ba496b568c35bd6b544f5b8;
preserve unrelated work in the occupied develop checkout. Keep the approved
Level L grammar-source selection policy, source-byte assertions, strict macOS
installer validation and normal publication gates. Publish the tested repairs
to hotfix and develop. No language, compiler architecture or release change.

## Acceptance criteria

1. **AC-01 — verified:** Account for every failed overnight job using inspected
   logs and the concise findings below. Five Deep platforms fail the four
   source_data_contract_l modes; MinGW
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
3. **AC-03 — verified locally, real service confirmation pending:** Mac installer
   stapling and validation each retry only temporary service exit 68, stop after four attempts
   per action, and require successful validation before continuation.
   Fixture proof covers recovery, permanent failure, exhausted retries and
   validation failure, including paths containing spaces. All three production
   packaging entry points use the helper, retaining spctl assessment.
4. **AC-04 — verified:** Core stage-product and required QA prerequisites build;
   focused normal and sanitizer checks pass. Record qualified inputs, commands,
   results and limitations in this plan before the substantive first push. Reuse
   the passing unaffected overnight gates rather than repeating full matrices.
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
4. **STEP-04 — publication ready (AC-04/05):** Inspect final diffs, summarize qualification,
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
The linked hosted runs own their logs; the findings and local qualification are
summarized here. Sanitizer failures are functional fixture
failures, not SAN findings. Linux's stop-on-failure means its full CTest coverage
is incomplete; no sanitizer-clean or release-ready claim is made.

Normal Debug qualification passed: 16/16 source-data contracts (108.59 s),
4/4 CRI-17 concurrency/lifecycle controls (0.68 s), and stage-product/qa-prep.
Maintained Apple ASan passed all the same cases in two nonoverlapping groups:
12 typed source contracts (121.50 s), then four Classic C contracts and four
CRI-17 controls (65.62 s), following successful prerequisite/fixture builds.
The original six stapling policy cases and shell syntax checks passed; the
17 October continuation above extends that policy's validation coverage.

All required local publication gates are green. The three compatible causes
are committed separately and pushed together to existing hotfix/develop; remote
ref and occupied-checkout integrity verification completes AC-05 and is reported
live in the conversation. The substantive first push includes this qualification
summary and expected successful normal automatic CI/snapshot publication. Native MinGW
and real Apple service confirmation belong to those automatic jobs; no status-
only repository push or full manual overnight rerun is planned.
