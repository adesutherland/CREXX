# STEP06 qualification — 15 October 2026

Scope: the approved POC-01–10 C/G slice on develop baseline
`5f9feff27eaf114bd57f17e162d956270ff8fc54`. Adrian authorised coverage-gap
closure, Level B-only implementation and documentation with valid evidence
reuse. [F6-AC01–06](../../PLAN.md) own this increment; the
[coverage map](../../doc/poc-coverage.md) and [walkthrough](../../examples/step06/README.md)
are the enduring explanation. Full compiler replacement remains open.

No compiler, generic Level L, native compiler, assembler, linker or VM algorithm
changed. A new Level B consumer checks/saves the accepted source tree and both
IR/RXAS views. A Python script only orchestrates existing tools, compares output
and counts emitted records. It implements no compiler/semantic algorithm.

## Reused evidence and exact boundaries

At entry, all 182 hashes in the STEP05 input manifest matched the checkout.
All 26 recorded Debug/Release product hashes matched. Both retained standalone
B-only bootstrap images and verification stamps were rechecked. The
[reuse audit](reuse-audit.json) records the actual dependency/import inventory,
identity comparisons and subsequent test-only CMake changes.

The unchanged STEP05 receipt remains the evidence for both concrete VMs,
B-host modes, IR on/off controls, both assembler modes, the 31-program backend
corpus, 90-command provenance harness, independent non-C/G IR oracle, writer
and source-index cases, native comparison controls, output/ownership/refusal
contracts, fresh install and the 25 installed checks. These results are reused,
not counted as newly executed tests. STEP02–04 receipts remain historical;
where STEP05 changed a dependency, current STEP05 proof takes precedence.

The six frontend groups were refreshed because STEP05 changed the mapper after
the older dedicated frontend qualification. All 24 CTests passed, using both
B-host modes and both VMs: [frontend log](frontends.log). The ordinary product
build passed: [build log](product-build.log). No production input changed, so
there is no reason to repeat the unchanged broad product or generic suites.

## Added decisive proof

The narrow new aggregate has 54 commands per mode/VM:

- The Level B `backend-poc` consumer checks/saves C and G AST, frontend IR,
  optimised IR and writer-produced RXAS. Exact literal spelling and both
  source arms survive; only the selected emission graph loses division.
- Compiler CLI output exactly matches the service output. Both IR modes
  compile/assemble/link/run the walkthrough (`01, ok` for C; `1, ok` for G).
- Switching the condition to zero reaches division. G reports
  `DIVISION_BY_ZERO`; C reports `RXC-LC-42.3`, retaining the original helper
  fault and authored line 4, column 10. Removing THEN causes a located syntax
  failure and preserves the prior destination bytes with no leftover stage.
- Paired nested logical cases show C's eager RHS effects and G's skipped
  operands, plus early return from a one-iteration loop. Classic expression
  DIGITS/FUZZ operands each execute once; resetting FUZZ changes the comparison
  and resetting DIGITS restores the default.
- The process PATH is empty and target contracts live in an isolated directory.
  Baseline and optimised RXAS retain logical source names and have fewer
  instructions after optimisation. Counts exclude directives, labels and
  external declarations; full modules include entry wrappers/cold handlers.

Four additional installed native command invocations run from a copied prefix
containing only `llrxc-c`, `llrxc-g` and their admitted runtime packages, with
that prefix as PATH. No `rxc`, `rxc-c`, compiler libraries, assembler or linker
commands are present there. Each output matches the executed service/CLI RXAS
byte-for-byte. [Receipt](isolated-installed.json). The prior full installed
pipelines/service checks remain valid; this new proof specifically closes old
compiler unavailability at request time.

The reusable source/IR algorithms still have their independent non-C/G tests;
STEP06 does not duplicate those tests inside the compiler or change their
contracts. Existing deterministic work/index/byte/register failure checks are
mapped in the coverage document.

## Scheduling and preparation observations

The initial Debug aggregate passed in isolation in 31.271 seconds
([original receipt](isolated-debug-initial.json)). Its pre-correction shape counts
are historical; final registered/ASan receipts and `examples/step06/shapes.json`
contain the corrected counters. Two early
test-development assumptions were corrected before qualification: missing THEN
is detected at the following ELSE in this exact source, and the FUZZ comparison
requires DIGITS 4 (at DIGITS 3 both numbers round identically even with FUZZ 0).
Neither was a product regression. The instruction counter was also corrected
to exclude indented metadata and dotted external declarations; saved counts
were recomputed from the unchanged emitted files.

The maintained Apple ASan dependency refresh, before the aggregate ran, failed
with duplicate generation of `bootstrap/L2/levell-linked.rxbin` and
`ERROR: no root modules selected`. A parallel Make shared-output race is the
likely mechanism; this is not a sanitizer report or a C/G semantic failure.
Retained log:
`cmake-build-debugasan/asan-logs/20261015-210424-build/build.log`.
The same preparation was retried with `--build-jobs 1` through `tools/asan-run.sh`.
The existing Apple SDK sprintf deprecation warnings remain visible in that
native refresh; no suppressions or native-source changes were made.

**Preparation finding F6-PREP-01 is now repaired and locally closed.** Adrian
subsequently authorised producer-order repair and STEP06 publication. Both
consumers now wait for `levell_bootstrap`; file dependencies retain freshness.
The [repair receipt](../../../../../research/level-l/evidence/bootstrap-order-20261015/README.md)
retains the permanent old/fixed Make regression, real parallel preparation,
byte identities and new native-launcher checks. Serial preparation is no longer
required for this path. Full platform delivery stays under LLCOMP-AC12.
Apple LeakSanitizer remains unsupported. The focused ASan aggregate passed all 54 commands in 117.116 seconds with no
sanitizer finding: [commands](isolated-asan.json), [runner result](isolated-asan.log).
This is not a broad sanitizer-clean or cross-platform claim. The earlier
[preparation failure](preparation-failure.log) is retained separately.

## Acceptance and publication boundary

The four new CTests use `RUN_SERIAL`, the existing `llcompiler_backend` lock,
900-second outer and 300-second nested hang guards. Explicit producers include
both drivers, the B view consumer, Classic adapter, runtime packages, selected
VM and binutils. [Registration](registration.json), [new inputs](inputs.json).
All four registered aggregate CTests passed (216 commands total), both B-host
modes × both VMs: [log](registered.log),
[opt/rxbvm](llcompiler_poc_opt_rxbvm.json), [opt/rxtvm](llcompiler_poc_opt_rxtvm.json),
[no-opt/rxbvm](llcompiler_poc_noopt_rxbvm.json), [no-opt/rxtvm](llcompiler_poc_noopt_rxtvm.json).
Together with the 24 refreshed frontend groups, **28 distinct focused CTests**
passed. The final input hashes were checked after the runs. All 26 STEP05
Debug/Release product hashes still match; [new test/tool identities](products.json)
record the added consumers and maintained ASan tools. Both new producers are
reachable from the normal optimiser-parity preparation target:
[generated graph inspection](preparation-graph.json).

| Acceptance | Evidence and boundary |
|---|---|
| F6-AC01 | All POC-01–10 rows mapped; existing backend corpus/independent oracles reused, frontend groups refreshed, missing combined/control cases now permanent and passing. |
| F6-AC02 | Existing ownership/import/budget/writer/source failures retained, new reached/skipped and syntax/file-preservation checks in both languages and IR modes. |
| F6-AC03 | Actual complete before/after RXAS and separate shape counts, existing structural regressions/native controls, helper and remaining conservatism inventory. No speed claim. |
| F6-AC04 | Exact retained B-only bootstrap and full installed evidence, plus four new isolated installed requests with old compilers unavailable. Proper B remains open. |
| F6-AC05 | Both host modes/VMs and compiler controls; retained assembler modes and generic deterministic refusal tests; local/hosted/platform cells explicit. F6-PREP-01 is closed by the separately qualified producer-order repair; parallel preparation and the focused ASan aggregate pass. Full AC12 is open. |
| F6-AC06 | Reproducible actual walkthrough, architecture/user guide and coverage matrix; balanced call/import contracts proposed for STEP07 review. No later implementation authorised. |

F6-AC01–06 are verified for the agreed local Phase A scope. F6-STEP01–04 are
complete; the PoC is ready for Adrian's assessment and next-step decision.
This increment updates tests, walkthrough, coverage, architecture, user guide
and plan together. Its normal develop publication should include these assets
in one substantive change; no status-only commit is required after hosted CI.
Adrian subsequently authorised the preparation repair, STEP06 develop
publication and CI-monitor handover. The repair adds one focused build-order
CTest (29 distinct local CTests including the original 28), replays the original
54-command ASan preparation target with eight Make jobs, and checks four rebuilt
native command requests. Of the 26 previously matched products, 24 still match;
the two regenerated native launchers have fresh passing checks. All bytecode
identities match. Earlier hashes and receipts remain historical evidence.
Normal automatic workflows are expected to pass after authorised publication,
but no hosted result for STEP06 is claimed. This is not
a default compiler switch. STEP07 and proper B/INTERPRET/replacement decisions remain
separate approvals. No CI rerun or deep/overnight matrix was dispatched.

Retained text logs have trailing line whitespace removed for Git storage only;
[original and retained hashes](log-normalization.json) record that formatting.
Diagnostic content is unchanged.
