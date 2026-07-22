# NR-27 whole-procedure RXAS flow-analysis worklist

Status: complete; accepted closeout validated
Started: 2026-07-22
Branch: `develop`
Accepted starting commit: `25cbca679bcee9dcc8413def4341a49044e1ff66`
Requested historical baseline: `4ab5f3d8d`
Upstream at start: `origin/develop`, 0 ahead / 0 behind
Starting worktree: clean

## Objective and boundary

Build a fail-closed, whole-procedure machine analysis in RXAS and use it to
determine a complete bounded panel of instruction-removing improvements. The
analysis must work identically for normal compiler output and hand-written RXAS.

No `rxc` hint, annotation, `.meta` extension, pseudo-opcode, sideband record,
RXAS syntax, canonical RXBIN or public ABI change is permitted. Ordinary
instructions and operands, procedure/label structure, canonical opcode effects,
existing metadata, TRACE records, `ENDLIFE`, constants and declarations are the
only inputs.

`ENDLIFE` remains an ordinary reference-lifetime operation. It is neither a
general value kill nor disposable cleanup and may not be weakened or removed to
enable another transformation.

## User-approved panel-first gate

The activity-specific sequencing instruction from Adrian on 2026-07-22
supersedes an early timing stop for each production slice:

1. Determine the complete candidate panel first.
2. Admit an implementation to the panel only when its mathematical semantic
   proof is closed, focused positive/negative correctness tests pass and it
   reduces static instructions plus bounded dynamic instructions where the
   retained evidence executes the site.
3. Retain rejected and deferred candidates with the missing proof or absence of
   instruction reduction; do not silently narrow the panel to successes.
4. Review the completed panel as a unit.
5. Freeze it, then run the proper profiling-off Release baseline using the
   normal formal sampling, drift-control and regression-guard policy.

Instruction reduction proves machine work was removed. It does not by itself
claim lower wall-clock time; the final consolidated baseline supplies that
separate evidence.

## Baseline acceptance

- [x] Verify branch, HEAD, upstream and worktree.
- [x] Record why the requested `4ab5f3d8d` baseline advanced: the sole successor
  is accepted NR-26 commit `25cbca679` (`perf: add typed semantic flow analysis`).
- [x] Confirm NR-27 is independent of NR-26 and consumes no NR-26 compiler fact.
- [x] Read root and performance instructions, live roadmap and historical
  Strand 4 terms.
- [x] Complete the focused RXAS/RXVM/effects, NR-04/08/09 and retained RXSEQ
  evidence audit. The current VM handlers require `MAY_THROW` on decimal
  literal load and `dcopy`, and prove no signal for one-/two-register `itof`
  and float comparisons; the canonical sidecar and metadata test now record
  those corrections.

## Design selection before production edits

### A. Extend the current queue with procedure-level blocks and bitset dataflow

Viable for the fact engine, but simply lifting the 20-item bound would also make
legacy `ANY_GAP` rules unbounded and repeatedly rescan a growing procedure.
That changes old rule selection and compile-time complexity accidentally.

### B. Build a small internal RXAS block IR and lower to existing instructions

Selected in a deliberately small form. Keep `instruction_queue` as the owned
instruction/metadata record, retire the stable local-peephole prefix into a
dynamically sized per-procedure stream, then attach blocks, successors,
predecessors and dataflow facts transiently. Emit the surviving original queue
records through the existing assembler functions. There is no new textual or
serialized representation.

### C. Sparse def-use chains on the existing stream

Use as a derived fact for single-use/reaching-definition queries, not as the
sole representation. Sparse chains alone make joins, reachability, exceptional
edges and metadata observation less explicit than the required CFG.

### D. SSA conversion

Deferred. The initial panel needs physical-register liveness, typed views,
reaching definitions and equivalence, not phi insertion and full renaming. SSA
would complicate physical-register reuse, metadata anchors and lowering without
closing a current proof gap. Reconsider only if a retained candidate cannot be
proved with the selected bitset/dataflow design.

### Selected pipeline

1. Run the existing bounded peephole over at most its current 20-item keyhole.
2. Move retired items, without copying token ownership, into a growable
   procedure stream instead of emitting them immediately.
3. At the next procedure/directive boundary or EOF, identify executable nodes,
   labels and observation records; build basic blocks and resolved successors.
4. Compute reachability and the minimum backward/forward facts justified by the
   candidate panel. Unknown/opaque effects invalidate only the affected fact
   families, while exceptional observation makes involved values conservatively
   live.
5. Apply one mechanically proved rewrite at a time, rebuild affected facts and
   iterate to a fixed point with an explicit termination measure: every accepted
   rewrite strictly reduces executable instruction count.
6. Emit the surviving original queue records through the existing RXAS binary
   path.

## Semantic model checklist

- [x] Basic blocks, label ownership and CFG successors/predecessors.
- [x] Entry reachability; an unresolved label, unknown opcode or indirect jump
  disables all NR-27 rewrites in that procedure.
- [x] Physical register identity across distinct `r`, `a` and `g` classes.
- [x] Explicit reads, possible writes and proven kills from `rxop_effects()`.
- [x] Fixed/indexed/range implicit register effects.
- [x] Backward register and seven-typed-view live-in/live-out.
- [x] Transformation-scoped must-availability, may-reach, literal and
  copy/value-equivalence facts.
- [x] Transformation-scoped use census; no admitted rewrite requires a general
  unique-use table.
- [x] Calls, returns, branches and conservative asynchronous SIGNAL-handler
  observation edges.
- [x] Alias/reference create/read/write/release, lifetime and indirect-write
  invalidation.
- [x] Source metadata, register metadata and TRACE observation points.
- [x] Numeric-context operations invalidate facts; the only admitted conversion
  is context-independent `itof` with an unchanged integer view.
- [x] Physical-register reuse and register-token metadata observation.
- [x] `rxas -d` diagnostics name accepted/rejected copy candidates, missing
  facts, incomplete control flow and per-procedure counts.

## Candidate panel

Every row must end as `accepted`, `rejected` or `deferred`, with exact proof and
instruction evidence. A safe candidate that finds zero sites is not an accepted
improvement.

| ID | Candidate | Required proof | Status |
| --- | --- | --- | --- |
| P1a | `icopy`/`fcopy` and strict-string-use `scopy` propagation | source typed view unchanged on all paths; every may-reaching destination read is redirectable under a must-available fact; no TRACE/metadata/implicit/read-write observation | **accepted** |
| P1b | `dcopy` propagation | as P1a plus allocation, missing-source signal and decimal-plugin equivalence | **deferred**: current handler can allocate and signal; the proof is not closed |
| P2a | Identity full-value `copy rN,rN` | exact same physical register and no address-bound TRACE; VM `copy_value()` explicitly returns on identity | **accepted** for hand-written RXAS |
| P2b | Nonidentity full-value `copy` | ownership, attributes, all payloads, flags, reference state and cleanup remain equivalent | **deferred**: no component-complete ownership/cleanup proof |
| P3 | Dead-result elimination | destination dead on every successor and execution has no signal, allocation, alias, reference, TRACE, hidden release or other observable effect | **rejected** from this panel: nominal numeric writes can release hidden reference/native payload state; retained census also found zero sites |
| P4a | Repeated integer/bitwise-float load, `null`, and one-register `itof` | identical must-reaching value/view on every predecessor; no intervening component write/barrier/TRACE; conversion context-independent | **accepted** for hand-written RXAS |
| P4b | String/decimal/binary loads and other conversions | allocation/cursor/plugin/numeric-context and exceptional behavior identical | **deferred**: proof is not closed |
| P5a | Complete-CFG reachability cleanup | block unreachable from entry with every target resolved; remove address-bound TRACE/source-step records with its instructions | **accepted** |
| P5b | Additional NR-18 identity/dead-load/same-pair-SWAP harvest | complete effect/liveness/interrupt proof and an actual retained site | **deferred**: the current 19-image optimized source panel has zero identity-copy, same-register SWAP or adjacent identical-SWAP sites; dead numeric results fall under rejected P3 |
| P6a | Typed-copy compare preparation | P1a plus the existing compare accepts the unchanged original view; all uses are redirected atomically | **accepted** as a counted P1a subset; no new opcode |
| P6b | Conversion-to-compare preparation | original representation is directly accepted and conversion context/exception state is dead | **deferred**: no retained site with closed representation proof |

## Mathematical correctness obligations

For each accepted rewrite `I -> I'`, retain a short proof over the machine state:

- the same reachable successor state for every ordinary edge;
- the same externally observable state for every exceptional/TRACE/metadata
  observation edge;
- identical source-register values and typed views read by the survivor;
- identical ownership, reference identity/lifetime, attributes, flags, numeric
  context and cleanup where those components are observable;
- no new behavior for unknown, conservative, opaque or indirect effects; and
- a strictly smaller executable instruction multiset on every rewritten path.

The pass must terminate because it never inserts an executable instruction and
every accepted transformation removes at least one.

## Focused proof matrix

- [x] Generated positive and negative test per accepted legality condition.
- [x] Hand-written RXAS procedure without metadata optimizes successfully.
- [x] Complex procedure with branches plus calls/references/opaque operations
  still optimizes an independent proved region.
- [x] Join, loop/backedge, unreachable block and multi-return coverage.
- [x] Typed-view cross-use and same-physical-register negative coverage.
- [x] TRACE/source/register metadata relevant and unrelated coverage.
- [x] `ENDLIFE`, alias, reference, indirect write, asynchronous handler and
  may-throw coverage.
- [x] `-n` remains bytecode-optimization off and preserves source instructions.
- [x] Both VM modes execute runnable fixtures with identical expected results.

## Frozen panel evidence

The panel was frozen after the proof review, before any wall-clock timing.

- The corrected 19-image optimized-source census is
  **46,469 -> 45,476 executable instructions**: **-993** across nine changed
  images, with no image growth. Debug accounting reconciles exactly as 979
  unreachable instructions plus 14 typed-copy eliminations over 154
  procedures. Four compare consumers are included in those 14 redirects.
- Richards retains the only canonical hot-path P1 site in the small timing
  portfolio: `richardsscheduler.schedule` is 53 -> 51 instructions. Against
  the retained pre-NR-27 module and the same linked library, repetition 1
  executes **9,179,035 -> 9,119,155 instructions** in both `rxvm` and `rxbvm`,
  exactly **-59,880 (-0.652356%)**, while preserving queue count 23,246, hold
  count 9,297 and PASS output.
- The isolated hand-RXAS panel fixture, assembled by the retained pre-NR-27
  assembler and the candidate assembler after the same local peephole, is
  **32 -> 25 static instructions**. It executes **28 -> 22 instructions** in
  both VMs and proves dynamic removal for typed copies, identity full copy,
  redundant identical load and redundant `itof`; its unreachable load is
  correctly absent from the dynamic delta.
- P4 `null` is present structurally, but the existing bounded peephole first
  combines the adjacent pair to `nulln`; NR-27 therefore reports no separate
  retained `redundant-init` site in this fixture or the 19-image census.
- P5 reachability contributes 979 static removals and zero dynamic removals by
  definition. It is a code-size/preparation result, not a runtime-dispatch
  claim.
- The current optimized source panel contains zero identity copy, same-register
  SWAP or adjacent identical-SWAP sites. Their absence is retained rather than
  converted into a production benefit claim.

## Panel evidence ledger

For every candidate retain:

- exact legality conditions and effect-completeness result;
- positive/negative generated fixtures;
- before/after RXAS and RXBIN executable instruction counts;
- bounded dynamic reduction from retained or refreshed exact-image evidence;
- accepted/rejected optimizer diagnostics;
- code-size, preparation and lifecycle observations; and
- disposition independent of later wall-clock timing.

## First formal Release verdict

The ordinary profiling-off Release comparison used the complete five-workload
common CREXX set—Sieve, Permute, Bounce, Richards and Base64—under both VM
modes. Exact baseline and candidate linked images use their matching libraries,
while both variants use the same candidate VM binary per mode to isolate the
assembler-produced product. Source and TRACE metadata are retained.

All 708 executions pass their observable-result check. Governance expansion
left 34 paired rounds for Sieve, Permute, Bounce and Base64 and 36 for Richards,
with no sample removed. The paired result is mostly neutral/noisy. Richards on
`rxbvm` is the sole clear lane at -0.238% median elapsed with a mean 95%
interval of -0.705% to -0.097%; Richards on `rxvm` is +0.066% with an interval
crossing zero. No workload paired median hits the 3% guard.

The equal-weight five-workload ratio-of-medians throughput geometric mean is
-0.552% on `rxvm` and +0.323% on `rxbvm`; neither hits the 1% common-portfolio
guard. This is a no-regression verdict, not evidence of a release-wide speedup.
The panel remains provisional and uncommitted at the mandatory review stop.
Exact evidence is under
`performance/evidence/2026-07-22-nr-27-panel-verdict/`.

## Accepted closeout

Adrian accepted the panel on 2026-07-22 and requested local commit plus a
separate second opportunity review. The unchanged implementation then passed a
full Debug build and **1,885/1,885** CTests with
`ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure` in
145.65 seconds. Evidence checksums and `git diff --check` also pass. No
sanitizer, install/package, cross-platform or additional timing campaign was
added because the accepted closeout path did not require them.

## Ordered execution

1. [x] Finish semantic/effects and retained-evidence audit.
2. [x] Implement procedure-stream ownership and regression-test unchanged local
   peephole output before enabling a global rewrite.
3. [x] Implement CFG/reachability plus effect-backed register extraction.
4. [x] Add liveness/reaching-definitions/typed-view facts incrementally.
5. [x] Evaluate P1-P6 in instruction-reduction order, retaining negative rows.
6. [x] Run the focused correctness and instruction-count matrix needed to
   freeze the panel; broad closeout remains outside the pre-verdict scope.
7. [x] Complete the panel review and remove P3 after its hidden-lifetime proof
   failed; strengthen indirect control flow to a procedure-wide fail-closed
   rule before freezing.
8. [x] Freeze the panel and run the consolidated ordinary profiling-off Release
   formal baseline in both VMs with unrelated controls.
9. [x] Report the performance verdict and stop for Adrian; do not commit or push
   unless requested.
10. [x] After Adrian's acceptance, run the full Debug build and broad CTest,
    checksum/review the exact diff and create the requested local commit without
    pushing.
