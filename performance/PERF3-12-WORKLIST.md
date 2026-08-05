# PERF3-12 current RexxCPS clause-lowering rereview

Status: complete — analysis-only evidence gate

Started: 2026-08-04

Completed: 2026-08-04

Purpose: use the accepted PERF3-06 product and the scalable PERF3-11 RXAS
graph/SSA infrastructure to identify, rank and assign general optimization
opportunities exposed by current RexxCPS clause lowering.  This activity
produces an implementation queue; it does not install an optimization.

## Authority and stop boundaries

- Starting branch: `codex/perf3-rxas-flow-infrastructure`.
- Starting repository commit: `209206f0f0f07cd709dcdd04665489756e0b433e`.
- Accepted product commit: `5fbe36049e26ee73ea0cf1720a7fc416f33d0fe2`;
  the intervening PERF3-06 commit changes only performance documentation and
  evidence.
- Runtime authority: the checksum-closed PERF3-06 ordinary profiling-off
  Release scorecard.  Profile elapsed time is diagnostic and cannot change
  its wall-clock verdict.
- Counts authority: fresh schema-5 `--profile=counts` capture under both
  `rxvm` and `rxbvm`, using the exact same optimized RexxCPS image and linked
  library.  A bounded `--smoke-count 1` run is permitted only as a labelled
  deterministic diagnostic linked to the retained canonical-default result.
- Source authority: canonical `tests/benchmarks/rexxcps_levelb.crexx`; opaque,
  modified, hand-lowered or no-TRACE forms may be used only as separately
  labelled ceilings or guards and cannot replace it.
- No compiler, RXAS optimizer, VM, library, benchmark, ISA, RXBIN, public ABI,
  language, signal or TRACE-semantics production edit is authorized here.
- Benchmark-specific recognition is prohibited.  A candidate must be stated
  as a reusable source/RXAS shape with independent guards.
- Any candidate selected from this gate receives a separate resumable
  implementation slice, design comparison, focused correctness gate and
  mandatory first ordinary profiling-off Release verdict.
- Commit locally when the evidence gate is complete; do not push.

## Questions this gate must answer

1. Which current clause families account for material dynamic work after the
   accepted PERF3-10 conversion proof and PERF3-11 proof migrations?
2. Which work is created unnecessarily by `rxc` lowering, and which work is
   ordinary RXAS that can be removed safely using existing graph, storage,
   component, signal, loop and SSA facts?
3. Where does a new proof capability have general leverage beyond RexxCPS?
4. Which apparent opportunities are library/runtime costs or unavoidable
   language semantics rather than optimization failures?
5. What dynamic ceiling, semantic obligations, proof owner, scale route and
   guard workload apply to each viable candidate?

## Required clause and mechanism coverage

The final ledger must cover, even when the measured disposition is zero or
deferred:

| Family | Required review |
| --- | --- |
| `R12-H01` variable/representation hoisting | Loop invariants, repeated loads, stable typed values, storage aliases, signals, ordered observations and loop-analysis scale. |
| `R12-C01` conversions and temporaries | Repeated XTOY materialization, typed-copy temporaries, component validity, nulls, write-once/single-use routes and signalling conversions. |
| `R12-P01` PARSE lowering | Prepared/exact paths, target materialization, string scans, compiler-only template facts and reusable RXAS consumers. |
| `R12-S01` stems and compound tails | Tail construction, conversion/concatenation, lookup boundaries, mutation/aliasing and reusable representation facts. |
| `R12-A01` ADDRESS lowering | Environment selection, command/function distinction, exposure anchors, callbacks, signalling and dynamic fallback. |
| `R12-T01` inactive TRACE | Near-zero inactive cost, ordered-event batching, event/value deletion, anchoring and enabled-trace fallback. |
| `R12-R01` temporary/register use | Dead temporaries, type-safe producer forwarding, live-range reuse and final register-number compaction without widening SSA scope. |
| `R12-I01` lowering/inlining ownership | Opportunities to simplify `rxc`, move reusable analysis to RXAS and redesign inlining only after RXAS consumers are proven. |

## Evidence contract

Retain one compact evidence root at
`performance/evidence/2026-08-04-perf3-12-rexxcps-clause-rereview/` containing:

- repository, detached-source, build, product, source, RXAS, RXBIN and library
  hashes;
- exact configure/build/profile commands and concise logs;
- pre/post host, power, load and process state;
- fresh raw schema-5 profiles, stdout and stderr for both VMs;
- profile-domain completeness and exact cross-VM count-parity audits;
- source-line/clause-family to generated-RXAS mapping;
- static and dynamic opcode, procedure, call, allocation, value-operation,
  branch and transition summaries;
- a ranked general-shape ledger with measured dynamic ceiling, proof owner,
  required capability, signal/TRACE obligations, scale route, guard workload
  and disposition;
- an implementation queue separating immediately feasible existing-SSA
  consumers, bounded new proof work, simple compiler emitter changes and
  runtime/library fallbacks; and
- root-relative recursive SHA-256 verification.

Every number must identify whether it is an observation, derived upper bound
or estimate.  Do not sum overlapping instruction, call, allocation or value
operation domains as if they were independent costs.

## Execution stages

### Stage 0 — control plane

- [x] Re-read repository and performance instructions plus the live roadmap.
- [x] Verify the starting branch, commit and clean worktree.
- [x] Confirm PERF3-06 as timing authority and PERF3-11 D0.5 as the current
  scalable SSA boundary.
- [x] Lock this analysis-only worklist and candidate-family IDs.

### Stage 1 — exact product and profile qualification

- [x] Verify a clean detached source at the accepted product commit and prove
  that the control commit differs only by PERF3-06 documentation/evidence.
- [x] Configure an optimized `CREXX_VM_PROFILING=ON` build without changing
  the accepted product image or library.
- [x] Build both VMs, the profiling summarizer/driver and exact RexxCPS
  artifacts; retain verbose output only in logs.
- [x] Run the minimum focused profile/runtime correctness checks.
- [x] Hash and prove exact source/RXAS/RXBIN/library identity.

### Stage 2 — fresh dual-VM capture

- [x] Capture one deterministic bounded schema-5 profile under `rxvm`.
- [x] Capture the exact same cell under `rxbvm`.
- [x] Verify PASS/provenance markers, schema/domain completeness, zero invalid
  events/overflow, exact no-opt parity and material hot-kernel parity; audit the
  28 optimized tail-format/control instruction difference explicitly.
- [x] Retain profile outputs and host pre/post state without treating profiler
  elapsed as product performance.

### Stage 3 — clause and mechanism mapping

- [x] Map material source clause families to generated RXAS procedures,
  instructions, calls, allocations, value operations and branches.
- [x] Separate benchmark body, shared library and VM/runtime work.
- [x] Cover every `R12-*` family and record zero/immaterial findings.
- [x] Calculate non-overlapping dynamic ceilings where the profile supports
  them; label estimates and unavailable attribution explicitly.

### Stage 4 — proof ownership and implementation queue

- [x] State each candidate as a general reusable shape.
- [x] Classify the earliest safe owner: mechanical/local RXAS, existing sparse
  SSA/flow, bounded new RXAS proof, simple `rxc` emitter, compiler semantic
  proof, library or runtime.
- [x] Record signal, TRACE, reference, type, component, loop and scale
  obligations plus guard workloads.
- [x] Rank candidates by expected end-to-end leverage, confidence and
  implementation dependency.
- [x] Split selected work into separately approvable implementation slices;
  preserve negative and deferred rows.

### Stage 5 — validation and closeout

- [x] Independently recompute all reported profile totals and ceilings.
- [x] Verify recursive evidence checksums and `git diff --check`.
- [x] Update this worklist and `performance/ROADMAP.md` only after the evidence
  is complete.
- [x] Review the exact diff and commit locally without pushing.
- [x] Stop for Adrian to select the first implementation slice.

## Closeout decision

The checksum-closed evidence is retained in
[`2026-08-04-perf3-12-rexxcps-clause-rereview`](evidence/2026-08-04-perf3-12-rexxcps-clause-rereview/).
The leverage ranking is PARSE direct-destination transactions first, compound
tail representation/reuse second, and copied-XTOY temporary elimination third.
The implementation order differs deliberately: `PERF3-12A / R12-C01` is the
recommended first slice because current metadata and sparse component/use
proofs already cover its essential facts, the exact ceiling is 2,220,000
`DCOPY` instructions plus 97,680,000 decimal-copy bytes, and no ISA, runtime
flag or compiler semantic optimizer is required.

`PERF3-12B` compares existing `STEMGET2`/`STEMSET2` selection with loop-scoped
tail reuse. `PERF3-12C` owns the larger 9,240,000-dispatch PARSE ceiling but
must first replace conservative PARSE signal metadata with the exact
source/result-alias and failure-visible-write contract. `PERF3-12D` defers
inlining and register finalization until these consumers simplify the actual
body and temporary set. Inactive TRACE is guard-only and ADDRESS has zero
runtime work in this slice.

PERF3-12A and PERF3-12B are now complete. PERF3-12B selected the general
capability-lazy joined-key reuse route, removed 1.96 million hot CONCAT
dispatches and retained a fresh 348/348 Apple scorecard. PERF3-12C
transactional PARSE is therefore the next implementation slice in this queue.
Adrian has deliberately placed the separate RXVM per-worker arena/central
block-depot memory work first; that sequencing does not change PERF3-12C's
proof obligations or authorize it implicitly.

## K04e/current-product reassessment

The authorized post-K04e fixed-work reassessment completed on 2026-08-04
against clean detached accepted product `c44706350`. Current optimized totals
are 53,660,581/53,660,552 under `rxvm`/`rxbvm`, down
560,629/560,630 from the original PERF3-12 profiles. K04e's isolated hot-site
proof accounts for exactly 560,000 removed dispatches; the small remainder is
consistent with the accepted D0.6/current library and final-output paths.

Every ranked clause mechanism retains its exact material count: the PARSE
ceiling is 9.24M, compound tails 2.24M, copied XTOY 2.22M plus 97.68 MB, and
direct call/return 560k. Their share rises slightly because the total stream is
smaller. The leverage ranking and implementation queue therefore remain
unchanged. `PERF3-12A / R12-C01` copied-XTOY component placement is still the
next bounded implementation slice; it requires separate approval and its own
mandatory first ordinary Release verdict. Evidence:
[`2026-08-04-perf3-12-k04e-clause-reassessment`](evidence/2026-08-04-perf3-12-k04e-clause-reassessment/).

## Exit criterion

PERF3-12 completes when the fresh dual-VM profile is correctness- and
checksum-closed, every required family has an evidence-backed disposition,
and the roadmap contains a ranked set of general implementation slices with
explicit proof ownership and first-verdict boundaries.  Finding opportunities
does not authorize their implementation.
