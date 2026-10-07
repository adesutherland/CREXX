# cREXX Performance Roadmap

Status: live performance companion, consolidated 2026-09-30; native-inference
qualification reconciled, KeyAccess verdicts accepted/closed, and Beta 6
selected as the Performance Beta on 2026-09-18.

Project-wide priority and release ordering belongs in
[`docs/ROADMAP.md`](../docs/ROADMAP.md). This file records only current
performance closeout, evidence-gated performance candidates, and their
selection state. It is not a second product roadmap.

Current measured results belong in [RESULTS.md](RESULTS.md), measurement
rules in [PERFORMANCE-GOVERNANCE.md](PERFORMANCE-GOVERNANCE.md), and durable
mechanism decisions in [DECISIONS.md](DECISIONS.md). Completed programme
ledgers and superseded runs are available in Git history.

## Current Position

**RXJSON-ACCESS-01 — published and normal hosted checks green
(18 September).** Five read-only JSON helpers borrow existing immutable binary
buffers, preserving public ownership and representation. Combined with RAG
bulk traversal and dictionary duplicate detection, the downstream query took
3.41/1.64/1.63 seconds versus 10.47 seconds, with identical results. The
[worklist](../docs/planning/rxjson-accessor-repair-20260918.md) owns the focused
and normal correctness evidence (451/451 unique passing cases). Repair
`65275452d` is published through hotfix to develop; exact-repair Build CREXX
and CodeQL passed, including all normal core/plugin jobs and optimizer parity.
Documentation-only concurrent integration/closeout preserves those qualified
product/test/build inputs. Global installation and release qualification are
separate from this source publication.
This is a bounded library repair, not a new compiler/ABI performance stage.

The [18 September optimization-boundary audit](../docs/planning/release-1/optimization-boundary-audit-2026-09-18.md)
records **OPT-BOUNDARY-01, repaired and published to develop**: shared SSA queries
now account for possible incoming argument/global aliases. The original probe
returns `42` in both modes and 102 affected tests pass. Repair `f7a8b08c1` was
published in the selected defect batch; its
[execution record](../docs/qa/beta3-defect-batch-2026-09-18/README.md)
tracks normal combined-head hosted checks. No no-alias calling restriction was introduced.
Adrian accepted OPT-BOUNDARY-02's RXC-only imported-inline boundary and
OPT-BOUNDARY-03's public status-bit contract. The audit owns those decisions
and evidence; neither acceptance is an outstanding architecture gate.

The completed Apple portfolio-v3 scorecard is strong overall: cREXX is well
ahead of ooRexx on the common five, ahead of genuine NetRexx on its comparable
common four, and split seven wins each with CPython across the fourteen Python
controls. The weaker rows are useful product-shape evidence, especially around
object ownership, nested containers, and complex graph workloads; they do not
authorize another general optimization programme.

Beta 3 now targets 2026-09-30 after the extended performance programme. Work
before that cut is limited to the remaining closeout obligations and
defect-driven qualification; the KeyAccess verdicts were accepted and closed
on 18 September. No new broad performance stage
belongs in beta 3. Adrian has selected core completion for 2026-11-30,
platform functional completion (including baseline Levels G/L) for 2027-01-31,
and **Beta 6 on 2027-03-31 as the Performance Beta**. February/March measures
and improves that complete platform; applications/tools/showcases continue
in parallel through RC1. Release 1 performance must be refreshed against the
exact April RC1/May 2027 candidate, not inferred from the August 2026 scorecard.

`POSTPERF-01` through `POSTPERF-05` are complete. No `POSTPERF-06` exists and
no new production compiler, RXAS, VM, language, RXBIN, ABI, or architecture
performance edit is automatically authorized. New production work requires a
separately selected gate and the first ordinary profiling-off Release verdict
defined in [`AGENTS.md`](AGENTS.md).

## Beta 6 Performance Beta — selected phase, mechanisms pending evidence

This phase is part of the [Release 1 plan](../docs/release-1-plan.md), serving
R1-AC-04. Record ideas now, prepare shared proof/framework prerequisites during
core development, establish the functionally complete Beta 5 baseline, and
select bounded production work for February/March. Known defects remain
immediate work; January/February is not a reason to postpone their repair.

| Candidate | Question for the complete product | Required selection evidence |
| --- | --- | --- |
| VM-MEMO-01 | Can adaptive memoization avoid expensive repeated pure procedure calls with recurring runtime arguments? | Procedure/callee effect proof; meaningful input repetition and residual cost; bounded cache keys/results/ownership and miss/disable policy; direct versus cached Release comparison including state/memory cost. Purity is proved, profitability is learned. |
| VM-FUSION-OWNERSHIP-01 | Do the current R1/R2 static fusions still justify their maintenance, and should each be retained, removed or selected explicitly by RXAS? | Matched R1-off/R2-off/both-off controls, both-engine semantics and intermediate-entry/debug/signal preservation; explicit ISA decision for migration. |
| VM-QUICKEN-REVIEW-01 | Is another runtime-dependent site worth adaptive handler specialization beyond existing caches/direct fast paths? | Current expensive client and observed behavior; incremental win over the best existing/direct/cache/static form; worker, invalidation, re-entry and observation contracts. Historical Q7 rejection remains client-specific evidence. |
| PERF-COMPILER-01 / RXAS-MAINT-01 | Which source/inlining or machine transformations remain useful on the complete C/G/L product? | Current emitted-shape/cost evidence; coherent AST rewriting; common RXAS facts, typed proof plans and edit contracts; no transported compiler-only assertion. |
| PERF-COMPILE-LATENCY-01 | Can a no-optimizer assembler build and in-memory RXC-to-RXAS handoff materially shorten compile/load/first-result without losing diagnostics or correctness? | Compare current `rxc -n`/`rxas -n` and file-based `crexx` against in-memory text and, only if justified, typed handoff; report cold/repeated latency, RSS, artifact/package size and later execution cost separately. The [compiler-profile roadmap](../docs/ROADMAP.md#compiler-profiles-and-low-latency-compilation--comp-pipe-01) owns language and architecture decisions. |
| PERF-RUNTIME-01 / PERF-REVIEW-01 / PERF-NEXT-02 | Which numeric/conversion, string-copy, signal or object/graph costs remain material? | Current profiles and representative workloads, including C/G/L and real consumers; equivalent semantics, ownership and exact negative controls. |
| PERF-LINK-01 | Can loading, binding, dead-code or artifact work improve real application lifecycle? | Separate startup/steady-state/memory/artifact verdicts with late-load, plugin and installed-application correctness. |

The candidate descriptions below remain the detailed idea ledger. Inclusion
means evaluate and decide, not promise every mechanism or a predetermined
speedup. Keep JIT/backend research and broad host-opcode migration outside the
selected phase unless Adrian explicitly changes their existing dispositions.

Any new public purity annotation, effect assertion, ISA or serialized handoff
contract must be settled before January's platform freeze or receive an
explicit exception. Runtime learning cannot prove purity by observing repeated
results. Preserve the accepted RXC/RXAS boundaries and shared proof machinery.

Use the governed portfolio plus suitable application/C/G/L controls; preserve
the canonical workload and comparator rules. Keep correctness, first ordinary
Release verdicts, stop/revert decisions, lifecycle/RSS/artifact costs and
cross-platform qualification visible. Freeze optimization implementation at
Beta 6; April provides exact-candidate verification and defect repair.

## Activity Register

Native-inference qualification is owned by the
[parent plan](../docs/planning/native-inference-backlog.md) and
[pipeline plan](../docs/planning/native-inference-ci.md). Accepted embedding
and generation overhead observations do not qualify untested devices or models.
NI-S4-P01 remains an accepted observation with unresolved cause; no further
upstream timing investigation is selected. Channel request lifetime is owned by
[its current plan](../concurrency/CHANNEL-REQUEST-LIFETIME.md).

| ID | Status | Work | Exit or decision |
| --- | --- | --- | --- |
| PERF-CLOSEOUT-01 | required beta 3 closeout | Resolve the remaining formal Linux QA-C obligation for the frozen Apple Stage 5 evidence, and run the named exact-SHA hosted gates for the 2026-09-30 beta 3 candidate. Do not relabel an unmatched newer run as the missing counterpart to the retained `81f159186` scorecard. | The retained scorecard has an explicit Linux disposition, and the exact beta 3 candidate has the required cross-platform evidence. |
| PERF-CLOSEOUT-02 | closed 2026-09-18 | Adrian accepted both KEYACCESS-01 and KEYACCESS-02 first Release verdicts and directed that the implementations be retained and the work closed. Recorded Windows results are about 187x faster insertion and 18.6x faster negative lookup; full-key comparison and actual-error logging remain. | [Acceptance recorded](DECISIONS.md#keyaccess-decision-2026-09-18--accept-retain-and-close); no further verdict decision or rework pending. Normal release-candidate qualification is separate. |
| PERF-BETA6-01 | phase selected 2026-09-18; mechanism selection pending | Use the functionally complete Beta 5 platform as the baseline for the February/March Performance Beta; evaluate the recorded candidates above through bounded design and first-Release-verdict gates. | R1-AC-04: accepted improvements and explicit reject/defer/regression decisions, with implementation frozen at 2027-03-31. |
| PERF-RELEASE1-01 | planned release verification | After the 2027-03-31 optimization freeze, qualify the frozen portfolio and run the smallest decisive exact-candidate scorecard needed for Release 1. This is verification following the Performance Beta, not a further optimization programme. | The 2027-04-15 RC1 and 2027-05-01 Release 1 candidate have correctness-gated, platform-labelled throughput/lifecycle/RSS/artifact evidence and explicit dispositions for material regressions. |
| PERF-REVIEW-01 | evidence-gated; no automatic change | Review CD, DeltaBlue, Towers, and Havlak only for a safe, general, material mechanism. Preserve workload equivalence and treat an inconclusive result as a retained finding. | Select one bounded mechanism with a first Release verdict, or defer the row without production change. |
| PERF-NEXT-01 | Beta 5 Level G baseline design input | Use Storage/List and related graph evidence to inform the Level G ownership and nested-container workstream in `docs/ROADMAP.md`. Settle required product capabilities before January; the Performance Beta does not silently introduce them later. | An approved baseline/ownership/container contract and equivalent benchmark controls exist before implementation timing is compared; broader capabilities remain separately selected. |
| PERF-NEXT-02 | later evidence queue | Retain NBody and Permute as product questions, not assumed optimizer defects. Re-profile against the then-current product before selecting work. | Current evidence identifies a general mechanism and passes the normal selection gate, or the item remains deferred. |

## Candidate Detail And Deferred Ideas

The Beta 6 evaluation queue above references these stable IDs. The phase is
selected; individual implementations are not active merely because their ideas
are recorded here or appeared in the completed programme ledger:

- `PERF-COMPILER-01`: later Level L inline slices, bounded register
  finalisation, hoisting, and late-inlining consumers. Start from a current
  profile and reuse the graph/proof service; do not broaden an unsupported
  inline shape as a shortcut.
- `PERF-COMPILE-LATENCY-01`: Adrian proposed a lean no-optimizer assembler and
  compiler-to-assembler handoff on 2026-10-07 as foundations for a compiled
  execution mode and possible later INTERPRET design. The first hypothesis is
  that removing an intermediate `.rxas` file or unused optimizer setup reduces
  cold and repeated lifecycle cost. Existing `-n` switches and the RXAS buffer
  scanner are controls, not evidence of a speedup. A typed direct interface
  risks duplicated validation, metadata/source drift and a new cross-tool ABI;
  a separate executable risks package and code-size growth. Require matched
  correctness and first ordinary Release verdicts for any production edit.
  Product-contract design may be selected before Beta 6; this ledger records
  the performance hypothesis, not its schedule. Candidate only: no
  implementation or architecture selection follows from recording it here;
  Classic INTERPRET remains parked in its own worklist.
- `PERF-RUNTIME-01`: numeric value caching, string-copy fast paths, signal
  specialization, and related runtime ideas. Each needs an attributable current
  workload, semantic guards, and a regression budget before selection.
- `VM-MEMO-01`: user-proposed procedure-result memoization, captured
  18 September. Hypothesis: repeated runtime argument values at expensive pure
  procedures justify a bounded result cache even when RXAS cannot fold the
  calls statically. Opcode effect metadata is a starting point, not a
  procedure-purity certificate: prove argument/global/reference dependencies,
  externally visible writes and transitive callees, and define errors,
  observation modes, cache keys, mutable-result ownership and binding lifetime.
  A possible first scope is fixed scalar inputs/results with no mutable
  external dependencies. Require current repetition/cost evidence and an
  ordinary Release cache-on/off comparison including memory and miss costs.
  Candidate only; no annotation, transported proof, ISA or implementation
  decision selected. Memoization does not require adaptive opcode rewriting.
- `PERF-LINK-01`: VM/link and dead-code hygiene. Keep artifact size, lifecycle,
  throughput, and late-loaded/plugin behaviour as separate verdicts.
- `PERF-ISA-01`: file-I/O call migration and measured RXAS instruction-family
  review. These remain post-Release-1 design studies; no opcode or RXBIN change
  follows from the old ledger alone.
- `PERF-JIT-01`: MIR/JIT/LLVM-style backend research. This remains exploratory
  and outside the interpreter and bytecode release contract.
- `RXAS-MAINT-01`: proposed incremental proof/typed-plan/application
  consolidation, preserving the cheap local engine and RXC AST framework.
  Existing shared APIs are the foundation; duplicated semantic scans and
  output-changing structural refactors are the risks. Require per-family
  contracts, unchanged-output controls and targeted regressions. No framework
  replacement is selected; see the [follow-up proposal](../docs/planning/release-1/optimization-maintainability-and-fusion-ownership.md).
- `VM-FUSION-OWNERSHIP-01`: review independent private VM pattern selection,
  comparing removal, migration to an explicit RXAS-selected guarded instruction
  and bounded retention where the measured tradeoff favors it. First measure
  matched current R1-off,
  R2-off and both-off controls; preserve alias, lifetime, intermediate entry,
  signal, TRACE/debug and both-engine semantics. Historic 1–3% List gains are
  not current whole-product evidence. No new fusion, ISA change or implementation
  is selected; the same proposal owns the decision gate.
- `VM-QUICKEN-REVIEW-01`: original objective and current feasibility reviewed
  on 18 September in the same follow-up. Q7's reference/COPY prototype did not
  beat the best direct/compiler placement; the accepted production outcome was
  a direct MKREF improvement. Current interface caches already retain runtime
  type/target facts, distinct from static R1/R2 fusion. A new adaptive family
  needs a current hot runtime-dependent cost left after those caches, a bounded
  direct/cache/adaptive comparison, and worker, invalidation, observation and
  lifecycle proof. Review complete; no new experiment or general framework
  selected. Historical Q7 rejection is not a universal impossibility result.

## Retained baseline and decisions

PERF3 and POSTPERF-01 through POSTPERF-05 are complete. Accepted mechanisms,
negative decisions and reopening triggers are consolidated in
[DECISIONS.md](DECISIONS.md). The
[Stage 5 baseline](evidence/2026-08-18-performance-closeout-stage5/) retains
its final samples and identities while the formal Linux QA-C criterion is open.
It does not qualify a later release candidate.

The normalization-certificate formal verdict remains visibly unresolved in
[UNICODE-CERT-01](UNICODE-CERT-01-WORKLIST.md). Numeric runtime caching remains
unselected under [VALUE-CACHE-01](VALUE-CACHE-01-WORKLIST.md); use current
optimized residual-cost evidence before reopening it.
