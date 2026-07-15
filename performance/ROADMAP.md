# cREXX performance roadmap

Last updated: 2026-07-15

This is the live performance-programme register. The definitions, evidence and
exit criteria come from the dated
[`performance programme report`](../docs/planning/release-1/performance-programme-report-2026-07-15.md).
Use this file for status, short dated notes, idea capture and links to retained
evidence. Do not rewrite the dated report to reflect later work.

Status values are `queued`, `in progress`, `blocked`, `decision required`,
`complete`, `rejected` and `superseded`. Only use `complete` when the report's
exit criterion is met.

## P0 no-regrets register

| ID | Activity | Status | Current note / next gate |
| --- | --- | --- | --- |
| NR-01 | Reproducible benchmark portfolio and runner | in progress | Approved workload coverage, serial raw-sample retention and the separate lifecycle lane are implemented; remaining exit work is machine-capturable full provenance and NR-11 publication policy. |
| NR-02 | Portfolio equivalence and optimizer-resistance ledger | complete | All approved steady-state/runtime cells and the lifecycle lane have correctness, equivalence, generated-form and retained-pilot evidence or an explicit `not comparable` disposition. |
| NR-03 | Automated performance evidence bundle | complete | One Level B command retains exact-hash Release timing, schema-3 profile/allocation evidence, RXSEQ N=2/3/4, ranked reports and paired deltas; the verified proof bundle is `evidence/2026-07-15-nr-03-automated-proof/`. |
| NR-04 | Opcode effects inventory | queued | Generate and validate read/write/kill/alias/reference/throw/branch/call facts before broad flow transforms. |
| NR-04A | Kind-index runtime metadata and scan counters | queued | Remove ordinary type/ADDRESS traversal through source/TRACE records without losing diagnostics. |
| NR-05 | Call-path census | queued | Portfolio counts by call kind/arity plus swaps, copies, return placement, frame reuse, selection and signal unwind. |
| NR-06 | Compiler call-window placement fast path | queued | Preserve the contiguous-window ABI and signal-unwind contract; schedule moves once and measure. |
| NR-07 | Direct compare-to-branch lowering | queued | Typed single-use Boolean results only; retain RXAS fallback and source/debug semantics. |
| NR-08 | Definite initialization and reference-lifetime facts | queued | Fail closed; prove dead `NULL`/`ENDLIFE` and copy removals with escape/reference tests. |
| NR-09 | RXSEQ candidate ledger | queued | Rank by workload, static site and module; record semantic status and measured disposition. |
| NR-10 | Cross-runtime forensic baselines | queued | Run the formal CREXX/ooRexx/NetRexx portfolio matrix incrementally as each NR-02 workload clears equivalence; include Regina for RexxCPS only and retain selected Java/C controls. |
| NR-11 | Performance governance and scorecard | queued | Agree Tier A/B, geometric-mean and outlier policies, regression budget and publication format. |

### NR-01 work notes

- 2026-07-15: Chose a top-level `performance/` operational workspace.
  `tests/benchmarks/` remains the portable language suite and
  `tests/performance/` remains the focused internal microbenchmark area.
- 2026-07-15: Added `portfolio/manifest.md` for the five-workload seed and added
  optional per-warmup/per-run serial sample CSV to `run_benchmarks.crexx`.
- 2026-07-15: Retained the first dated seed evidence bundle at
  `evidence/2026-07-15-seed-portfolio/`: clean targeted Release build, 13/13
  benchmark gate, five workloads, and 60 serial observations with exact
  provenance. The bundle is a capture-path baseline, not an approved portfolio
  threshold.
- 2026-07-15: Added the benchmark-native RexxCPS clause-rate observations to
  the evidence bundle. Three matching `rxvm` canonical runs reported a mean of
  845,834 CPS; this is distinct from the runner's startup-inclusive elapsed
  time.
- 2026-07-15: Added `portfolio/cross-runtime-plan.md`. The proposed Tier A
  planning range is roughly 12-16 workloads, but coverage cells and comparable
  ports—not a round number—are the completion gate.
- 2026-07-15: Comparative scope set: NR-02 covers every selected Tier A
  benchmark across CREXX, ooRexx and NetRexx, including porting, equivalence,
  optimizer-resistance evidence and an initial retained run. Regina is limited
  to RexxCPS. NR-10 receives each qualified workload for formal baseline and
  forensic capture.
- Remaining exit work: make full build/machine provenance machine-capturable
  and define the publication scorecard, common subset and regression policy
  under NR-11. The approved coverage categories and separate lifecycle lane
  are now implemented.

### NR-02 work notes

- 2026-07-15: Adrian approved a bounded 12-item Tier A portfolio: the five seed
  workloads; Bounce, Storage, List, Richards, a JSON parser capability workload
  and an RFC 4648 Base64 round trip; plus compile/load/first-result as a
  separately reported lifecycle lane. Queens, DeltaBlue, Havlak,
  filesystem-I/O work and focused Classic semantic probes remain Tier B/reserve
  candidates.

- 2026-07-15: Started the temporary resumable matrix in
  `NR-02-WORKLIST.md`, including provenance, license, correctness,
  equivalence, optimizer-resistance, pilot and disposition fields per runtime
  cell. Proposed a 16-workload Tier A working set against the named coverage
  gaps; final approval remains NR-11.
- 2026-07-15: Runtime inventory found Temurin JDK 26.0.1 and Regina 3.9.7.
  No ooRexx or NetRexx executable was present. The official NetRexx site names
  5.10-GA (2026-03-20) as current; it was installed reversibly under the user
  local tool area. A minimal compile/run passed on JDK 26.0.1 and retained the
  generated Java, Java 8 class file and disassembly.
- 2026-07-15: Qualified the available CREXX and NetRexx Sieve cells. Both
  preserve the 5,000-slot algorithm, accept runtime repetition perturbations,
  observe the 669-prime result and have retained serial 50-repetition pilots;
  the later ooRexx completion is recorded below.
- 2026-07-15: Qualified the available CREXX and NetRexx Permute cells. The
  NetRexx generated form retains object construction, recursion and paired
  swaps; both ports observe 8,660 calls and have retained serial
  50-repetition pilots. The later ooRexx completion is recorded below.
- 2026-07-15: Mandelbrot passed sizes 1/500/750 on CREXX and NetRexx and has
  retained size-500 pilots. NetRexx has no source `^` operator, so its exact
  checksum uses a timed arithmetic XOR helper plus a partial-byte padding
  loop. Keep that cell a disclosed adaptation pending common-aggregate
  equivalence review; do not hide the added work.
- 2026-07-15: Qualified the available CREXX and NetRexx Towers cells. Both
  retain disk allocation, pile operations and the 13-disk recursive move
  graph, observe 8,191 moves, pass repetition perturbations and have retained
  serial 10-repetition pilots. The later ooRexx disposition is recorded below.
- 2026-07-15: Added Classic procedural sources for all four non-RexxCPS seed
  workloads. Regina surrogate checks passed Sieve/Permute/Towers at 1/2 and
  Mandelbrot at size 1; these are source checks, not Regina portfolio or
  ooRexx qualification. Classic Mandelbrot's arithmetic XOR is impractically
  slow at full size under Regina, and Classic Towers' stem/node-id model does
  not preserve object allocator/dispatch cost. Both remain disclosed
  adaptations; their subsequent ooRexx review is recorded below.
- 2026-07-15: Retained the RexxCPS qualification slice for CREXX, Regina and
  NetRexx. Byte-exact Classic 2.2 and bundled NetRexx 2.1n sources, disclosed
  2.2n/2.2c adaptations, A/B opaque/result variants, serial pilots, generated
  Java/classes/disassembly and Regina trace counts are recorded under the
  dated NR-02 bundle. The samples are noisy and not an NR-10 ratio. Regina's
  RexxCPS-only NR-02 lane is complete; the later ooRexx completion is recorded
  below.
- 2026-07-15: Homebrew offered no ooRexx formula/cask and its `rexx` command
  was confirmed as Regina. Installed the official stable ooRexx 5.1.0 r12973
  universal portable build under `/Users/adrian/.local/opt/oorexx/5.1.0-12973`
  without changing the Regina symlink or shell startup files; retained source,
  checksum, version and minimal execution proof in the NR-02 evidence bundle.
- 2026-07-15: Completed the ooRexx RexxCPS cell: canonical and A/B opaque
  pilots passed, both variants observed `1|69|1.22694`, executable translated
  images ran, and a bounded count-one `TRACE I` retained 990 source plus 4,012
  intermediate kernel records. The full four-runtime RexxCPS NR-02 slice is
  qualified; formal repeated comparison remains NR-10.
- 2026-07-15: Qualified the ooRexx Sieve and Permute seed cells with 1/2
  perturbations, translated images and serial pilots. Towers also ran correctly
  but its stem/node-id representation removes the object allocation/dispatch
  pressure under study, so the current ooRexx cell is `not comparable` for the
  common score and remains only a disclosed procedural diagnostic.
- 2026-07-15: ooRexx Mandelbrot passed size 1 but failed the common correctness
  contract at size 500 (255 versus 191) and size 750 (128 versus 50) because
  decimal numeric semantics change boundary results. Both long serial negative
  runs are retained. The cell is `not comparable`; no runtime-specific answer
  was accepted and no pilot score entered the common matrix.
- 2026-07-15: Added `evidence/benchmark-median-summary.md` as the live compact
  cross-bundle comparison. Each row is one dated benchmark/run with platform
  median columns and sample counts. Warmups, correctness failures and invalid
  diagnostic/version modes are excluded explicitly; passing values are not
  discarded merely for looking extreme. Added the exact opaque A/B input map
  to the RexxCPS ledger so those diagnostic rows are interpretable.
- 2026-07-15: Repeated the cREXX portfolio after clean CLion Release/Debug
  rebuilds. The verified Apple-clang Release VM compile and link used
  `-O3 -DNDEBUG`; canonical RexxCPS recovered to an 857,561-CPS median versus
  843,110 in the seed bundle and 527,607 in the slower pilot. Opaque A/B also
  recovered to 840,065/836,741 CPS. Sieve, Permute and Mandelbrot remained in
  the seed band. Towers ranged from 688 to 771 ms across three passing series
  on the same image, so all samples are retained and a quiet NR-10 run remains
  the regression gate. See `evidence/2026-07-15-crexx-rexxcps-o3-rerun/`.
- 2026-07-15: Implemented and qualified the approved expansion across cREXX,
  ooRexx and NetRexx: Bounce, Storage, List, Richards, JSON and RFC 4648
  Base64, plus the cREXX-scripted compile/translate/assemble/load-first-result
  lane. All six workloads pass 1/2 perturbations on all three runtimes; cREXX
  optimized/unoptimized CTest passes. One-warmup/three-recorded pilots,
  ooRexx translated images, NetRexx generated Java/classes/disassembly and
  lifecycle samples are retained under
  `evidence/2026-07-15-nr-02-portfolio-expansion/`.
- 2026-07-15: Closed NR-02 with explicit comparability boundaries. cREXX
  Storage is `not comparable` because a wrapper object plus `.object[]`
  replaces each upstream array. JSON is a native-surface diagnostic because
  cREXX uses string/path parsing while ooRexx and NetRexx build object models.
  List/cREXX retains its weak-reference arena as a disclosed adaptation. The
  resulting capability/accuracy register is `capability-gaps.md`; references
  and binary-safe `.binary` storage are confirmed present, not missing.

### NR-03 work notes

- 2026-07-15: Started the resumable implementation ledger in
  `NR-03-WORKLIST.md`. The ordinary Release build is the verified
  `-O3 -DNDEBUG`, profiling-off product baseline; no profiling build was
  configured at handover. Existing schema-v2 instrumentation and focused tests
  cover instruction/transition/procedure/native/call-mechanics timing,
  overflow/degraded status, and RXSEQ N=2/3/4 exact-module analysis. Code audit
  confirmed that allocation/value/frame counters are absent. A private
  profiling-schema extension is sufficient and does not require a language,
  RXBIN, public ABI, ownership, or architecture decision.
- 2026-07-15: Completed NR-03 with the Level B
  `tools/run_evidence_bundle.crexx` orchestration path and versioned
  `manifests/nr03-proof-v1.txt`. Schema 3 adds documented profiling-only
  allocation/value/frame counts, bytes, frame reuse/high water, explicit
  native-child accounting, and row status. The retained proof bundle at
  `evidence/2026-07-15-nr-03-automated-proof/` covers five exact
  optimized/unoptimized pairs, 40 serial ordinary-Release samples, 10 profile
  CSVs, 30 binary RXSEQ captures plus decoded candidate CSVs, generated paired
  summaries, and 311 independently reverified checksums. Release benchmark
  CTest passed 29/29 and profiling/RXSEQ CTest passed 11/11. Profiling-off
  help/symbol/compiler/preprocessor checks found no surviving instrumentation.
  The proof deliberately retains all passing samples; its timing deltas are
  observations, not a new regression policy or optimizer-causality claim.

## Cross-runtime portfolio execution order

The target is to run every approved Tier A workload on CREXX, ooRexx and
NetRexx. Regina runs RexxCPS only. Use explicit `not comparable` entries rather
than invented or semantically misleading ports. The detailed matrix and
coverage gaps are in
[`portfolio/cross-runtime-plan.md`](portfolio/cross-runtime-plan.md).

1. **NR-01 — complete coverage and port inventory.** Expand the seed portfolio,
   identify the source/port needed for CREXX, ooRexx and NetRexx, and classify
   each cell as canonical, equivalent port, disclosed adaptation, control or
   not comparable. Regina needs only the RexxCPS lane; Java/C remain selected
   controls rather than a mandatory full matrix.
2. **NR-02 — port, qualify and pilot-run comparisons.** Complete for the
   approved portfolio. Preserve the explicit `not comparable` and disclosed-
   adaptation decisions when transferring cells to NR-10/NR-11.
3. **NR-10 — retain formal comparative baselines.** Do not wait for the final
   workload before measuring: add same-host raw results implementation by
   implementation whenever a workload clears NR-02, while keeping incomplete
   matrices visibly incomplete.
4. **NR-11 — publish a scorecard.** Aggregate only the common qualified
   CREXX/ooRexx/NetRexx workload set; keep Regina's RexxCPS result and other
   non-common workloads visible but outside that portfolio aggregate.

## P1 bounded production register

| ID | Activity | Status | Current note / boundary |
| --- | --- | --- | --- |
| NR-12 | Extend read-only by-value and return copy coalescing | queued | No weakening of by-value isolation or reference semantics; see IDEA-CALL-01. |
| NR-13 | Redundant numeric-context setup elimination | queued | Effective procedure context must be identical. |
| NR-14 | Static/frozen `PARSE` lowering fast path | queued | General supported templates only; retain `parseExec` fallback. |
| NR-15 | General stem default/reset fast path | queued | Preserve generation/default/drop/tail semantics. |
| NR-16 | TRACE-off and same-ADDRESS-environment fast paths | queued | Preserve hooks, signals, mode changes and host callbacks. |
| NR-17 | Link-time direct provider/call resolution | queued | Preserve late-load and plugin fallback. |
| NR-18 | Safe RXAS rule harvest | queued | Require opcode effects/liveness proof and generated rule tests. |
| NR-19 | Optional C LTO/PGO/code-layout experiment | queued | Optional build feature; adopt only with repeatable supported-platform evidence. |
| NR-20 | Value/frame allocation counters and targeted pooling | queued | Gather counters first; preserve ownership and sanitizer gates. |

## P1 architecture-neutral prototypes

These are time-boxed evidence activities, not production commitments.

| ID | Prototype | Status | Adoption question |
| --- | --- | --- | --- |
| NR-21 | Mapped-call descriptor | queued | Does it materially beat compiler placement while preserving aliases and cold unwind? |
| NR-22 | Compact runtime execution stream | queued | Does it improve total portfolio time on at least two architectures with canonical RXBIN unchanged? |
| NR-23 | Runtime quickening/superinstructions | queued | Can dequickening, signals, TRACE/debug, late load and both VM modes remain correct? |
| NR-24 | Profile-selected fusion | queued | Do RXSEQ-selected fusions repeat across the portfolio without code-size explosion? |
| NR-25 | Value hot/cold split or pool allocator | queued | Do cache/allocation counters justify the ownership and complexity cost? |

## Idea ledger

Ideas remain here until assessed, even when rejected. An idea can inform more
than one `NR-*` activity and does not alter their priority by itself.

### IDEA-CALL-01 — Read-only by-value formal invariant and call-flag simplification

- Status: captured; assessment not started
- Related activities: NR-05, NR-06, NR-12
- Hypothesis: strengthen the compiler invariant so a by-value formal proved
  read-only is never the destination of compiler-generated writes and always
  reuses the incoming argument register on every supported call/inlining path.
  With that invariant explicit, measure whether compiler call signalling and
  defensive-copy branches can be narrowed, and whether any uses of
  `REGTP_VAL` / `REGTP_NOTSYM` can be removed or replaced for non-optional,
  read-only calls.
- Existing foundation: `mark_const_args()` in `compiler/rxcp_opt.c` already
  sets `is_const_arg` for provably read-only by-value formals in optimized and
  no-opt builds. `compiler/rxcpemit.c` already avoids their defensive copy, and
  inlining has matching read-only/writable handling.
- Important distinction: `REGTP_VAL` also signals that an optional argument was
  supplied, while `REGTP_NOTSYM` lets a writable large-value formal reuse a
  temporary not backed by a caller-visible symbol. The current flags therefore
  cannot be removed wholesale merely because read-only formals alias safely.
- Semantic boundary: plain `ARG` remains caller-visible pass by value;
  `ARG expose` remains pass by reference. The assessment must include scalar,
  string, binary, array, object and explicit reference values; optional/default
  arguments; repeated actuals; mutation/rebinding; inlining; returns; signals;
  recursion; dynamic/native calls; and both optimized/no-opt output.
- Evidence needed: call census by formal kind; generated RXAS counts for
  `SETTP`, copy/swap and flag tests by static site; proof that every read-only
  formal path has no write; paired portfolio timings; and a written ABI/signal
  unwind impact assessment before any flag-contract change.
- Key pointers:
  `compiler/docs/emitter_architecture.md` section 4.1,
  `docs/books/crexx_language_reference/procedures_and_arguments.md`,
  `docs/ai-context/RXVM_INTERPRETER.md` copy/move/call sections,
  `compiler/rxcp_opt.c` (`mark_const_args`),
  `compiler/rxcpemit.c` (`ARG` emission),
  `compiler/rxcp_emit_expr.c` (call marshalling),
  `binutils/include/rxflags.h`, and
  `compiler/tests/rexx_src/inline_test_byvalue_arg_reuse.crexx`.
- Decision gate: first assess as a compiler-only copy/signalling simplification
  preserving the current ABI. Any language, public ABI or signal-unwind change
  requires Adrian's explicit approval.

## Architecture selection gate backlog

The gate remains unstarted. It will consume completed evidence from the
portfolio, compiler/semantic improvements, cross-platform VM matrix and
bounded prototypes. It must record selected and rejected paths, compatibility
effects, remaining performance outliers and the next bounded response.
