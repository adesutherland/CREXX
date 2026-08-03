# PERF3-11 scalable RXAS flow and signal-proof infrastructure

Status: **complete — D0.5 scale verdict passed**

Architecture approved: 2026-08-02

Purpose: replace the growing collection of candidate-by-candidate RXAS flow
solvers with one scalable, per-procedure analysis foundation.  The foundation
must model symbolic storage, component values, calls, mutable signal policy and
normal/skip/handler/unwind continuations accurately enough to support reusable
proofs, loop analysis and later register allocation without changing emitted
code merely by being present.

## Authority and mandatory stops

Adrian approved the architecture and this staged implementation plan on
2026-08-02.  Execution is authorized subject to the mandatory gates below.
No push is authorized.

Once implementation is authorized, this activity may refactor internal RXAS
flow construction, split the implementation into coherent internal modules,
add instruction-effect metadata and add focused semantic/scaling tests.  It
must initially preserve public RXAS, RXBIN and VM behaviour and must not remove
an existing tactical rewrite until the new proof gives equivalent or stronger
fail-closed coverage.

Changing an instruction from signalling to non-signalling, changing the point
at which its writes become visible, changing its signal payload/address, or
changing continuation behaviour is a separate semantic decision.  PERF3-11 may
measure and propose such a change, but must stop with an instruction-by-
instruction contract and compatibility impact for Adrian's approval before
editing the VM or declaring the new contract in RXAS metadata.

Gate 1 passed on 2026-08-02. Adrian selected S1-S5 and additionally required
the coarse `RXOP_SEM_MAY_THROW` flag to be corrected or retired coherently with
the new metadata. The implementation retires it: generic effects retain their
existing non-signal bit values, while the separate 650-entry signal sidecar is
the sole signal-capability and failure-phase authority.

The first optimizer consumer that changes ordinary emitted code triggers the
mandatory first profiling-off Release runtime verdict in
[`AGENTS.md`](AGENTS.md).  After minimum focused correctness, freeze the
implementation, run the smallest decisive end-to-end comparison, report, and
stop.  Infrastructure-only stages instead use emitted-image equivalence and
assembler processing-time/memory gates.

## Clean starting point and recovery

- Working branch: `codex/perf3-rxas-flow-infrastructure`.
- Exact starting commit:
  `2cabe88cd94e42d95281dadb8f8269b7673ca606` (`perf: add trace-safe
  component conversion flow`).
- Starting tree: clean; the branch contains the five accepted local PERF3
  commits above `origin/develop` and nothing from the provisional dense
  component solver.
- Complete pre-refactor investigation checkpoint:
  `codex/perf3-pre-flow-refactor-checkpoint` at `70719fc2d`.  It retains all
  221 code, test, worklist and evidence files from the abandoned layered PoCs
  and is for comparison/selective recovery, not a production base.
- Five untracked generated lifecycle `lifecycle_probe.rxbin` files were
  deleted with Adrian's authorization before the checkpoint.  They were not
  source or retained evidence and can be regenerated.
- Existing PERF3/PERF2 timing remains the historical runtime baseline.  Do not
  overwrite it or silently replace it with a run affected by remote-terminal
  host load.  Stage 0 audits its validity; a same-session drift control or
  baseline rerun occurs only when the governance rules require one.

No fresh build or test result is claimed by this planning checkpoint.

## Architectural invariants

1. **Procedure ownership.** Build and solve one procedure at a time.  Inlining
   can enlarge a procedure but must not turn the analysis into a whole-image
   matrix.
2. **Immutable analysis epoch.** Build a `FlowProcedure`, run cached analyses
   and proofs against that version, collect a rewrite plan, then apply the plan
   and explicitly rebuild/invalidate.  Proof code must not mutate the graph it
   is traversing.
3. **Sparse write-once identities.** Storage, component values, contexts,
   signal policy and effects use immutable definition identities plus phi
   definitions at joins.  Do not retain a node-by-all-registers-by-all-
   components table.
4. **Signals are first-class control and state.** A generic `may throw` bit or
   one coarse effect generation is insufficient.  Signal continuations,
   handler policy, write phase and observable signal data have dedicated
   representations and query APIs.
5. **Conservative base graph, refining analyses.** The graph contains every
   semantically possible edge.  A proof may establish that a conservative
   edge is infeasible at a point, but absence of proof never removes it.
6. **Storage is not a register number.** `LINK`, `SWAP`, `UNLINK`, call windows
   and reference operations are interpreted through symbolic storage
   identities.  Component SSA is layered on those identities.
7. **Metadata drives generic transfer.** Opcode-specific semantics live in
   validated metadata or narrowly reviewed transfer helpers.  Consumer code
   must not accumulate independent opcode switch statements.
8. **Program observations remain exact; optimised TRACE follows its documented
   profile.** Signal payload, interrupted address, handler order, call/unwind
   state and ordinary side effects constrain code motion and deletion.
   Optimised TRACE events may be omitted, combined or relocated when their
   producing work is removed or moved; every retained event must read the
   proved value at a reached boundary and retained same-boundary events remain
   ordered without requiring `CNOP` separators. Compiler and assembler no-opt
   mode owns source-correspondent trace.
9. **Bounded failure is safe.** Allocation failure, unsupported metadata,
   irreducible control, dynamic signal uncertainty or a proof-work budget
   exhaustion disables the candidate proof; it cannot miscompile the input.
10. **Deterministic diagnostics.** Graph IDs, definition IDs, phi inputs and
    debug dumps are stable for the same procedure so old/new analysis can be
    compared mechanically.

## Target internal layers

The implementation may adjust physical filenames to match existing build
conventions, but code ownership should follow these layers:

1. **Flow orchestration/pass manager** — owns the procedure epoch, cached
   analyses, invalidation and ordered rewrite pipeline.
2. **Flow graph** — instructions, basic blocks, source-order mapping, typed
   edges and synthetic entry/exit/handler roots.
3. **Structural analyses** — reachable reverse-postorder, predecessor sets,
   dominators, dominance frontiers, strongly connected components and loop
   forest.
4. **Signal analysis** — opcode signal contracts, mutable handler-policy SSA,
   handler reachability and edge-specific state.
5. **Storage/component SSA** — symbolic register-to-storage mapping,
   component definitions, phis, absence/presence and context/effect versions.
6. **Proof queries** — dominance, availability, success refinement,
   speculatability, must-execute and observation-equivalence queries.
7. **Rewrite consumers** — request proofs and emit a batch rewrite plan; they
   do not own another flow solver.

The initial file split should keep the public entry points narrow.  A likely
shape is `rxas_flow.c` for orchestration, internal graph/analysis/signal/SSA
modules, and existing binutils opcode metadata for public instruction
contracts.  The exact split is chosen during Stage 2 without changing public
headers unnecessarily.

## Signal contract to model

Every potentially signalling instruction needs a validated contract with at
least:

- statically known signal set or explicit dynamic/unknown signal;
- write phase: before writes, after writes, named partial writes or unknown;
- component/storage/context reads and writes on normal and failure paths;
- signal payload and interrupted-address observability;
- permitted continuation classes: normal, skip, branch/call handler,
  fail/unwind and terminal;
- signal-policy and other effect dependencies;
- whether successful completion refines a value/domain fact; and
- whether repeating the identical operation is success-stable when all named
  operands and dependencies have identical SSA identities.

Handler policy is separate mutable frame state.  It must cover static and
dynamic signal names, installation, `sigpush`/`sigpop`, inherited caller
policy at procedure entry, callee-local changes disappearing on return or
unwind, and action-aware handler results.  Unknown asynchronous handler entry
may use a synthetic conservative root rather than dense edges from every
instruction, provided the resulting observations remain equivalent to the VM.

An instruction that can signal terminates its basic block.  Its outgoing
states are edge-specific:

- **normal** applies all successful writes and any success-refinement fact;
- **skip** applies exactly the writes visible at the recorded signal phase and
  resumes after the instruction;
- **handler** exposes the signal value, payload and interrupted address;
- **fail/unwind** reaches an explicit exit/super-exit and restores caller-owned
  state according to the VM contract.

## Proof model and initial query surface

The foundation should expose small, testable queries rather than analysis
internals.  Initial queries are:

- `dominates(def, point)` and `must_execute(block, loop)`;
- `storage_at(point, register)` and `component_value_at(point, storage,
  component)`;
- `state_on_edge(instruction, edge_kind)`;
- `handler_policy_at(point, signal)` and `continuations(instruction)`;
- `can_signal(instruction)`, `signal_phase(instruction)` and
  `is_speculatable(instruction)`;
- `available_derivation(point, key)` and `same_observation(before, after)`;
- `success_refines(instruction, fact)` and
  `repetition_is_non_signalling(first, second)`; and
- explicit alias/call/effect clobber queries.

The generic repeated-operation rule is:

> A normally completed instruction can prove a dominated identical operation
> non-signalling only when opcode metadata declares success stability and the
> operands plus every signal-relevant context/effect identity are unchanged.

This is the reusable `xtoy` rule.  `ITOS`, `FTOS`, `DTOS`, other conversions
and future non-conversion operations are consumers of the same proof.  It is
not assumed merely because two textual opcodes and register numbers match.

## Detailed execution stages

### Stage 0 — lock the clean oracle before production edits

- [x] Reconfirm branch, exact commit and clean status.
- [x] Build ordinary profiling-off Debug and Release RXAS products from the
      clean base; retain the Release `rxas` binary and hashes outside the build
      tree.
- [x] Generate and hash exact focused signal/storage fixtures plus canonical
      RexxCPS, Richards and Towers RXAS inputs.
- [x] Capture emitted RXBIN hashes, existing optimizer diagnostics and both-VM
      focused outputs as the behaviour oracle.
- [x] Capture clean-base RXAS elapsed time, peak RSS and internal size counters
      for small, large and inlined procedures with the host in the agreed idle
      state.  This is an assembler-cost oracle, not a replacement runtime
      baseline.
- [x] Record deterministic procedure metrics: instructions, blocks, edges,
      registers, symbolic storages, definitions, phis and iterations.

**Gate 0:** report the exact oracle and any pre-existing failure.  Do not
explain away a dirty baseline by changing implementation.

Gate 0 passed on 2026-08-02.  Focused correctness is 61/61; accepted generated
RXBIN identities match; the idle median assembler oracles are 51.400543 ms
Richards, 20.090103 ms Towers and 51.542520 ms RexxCPS.  Evidence:
[`2026-08-02-perf3-11-stage0-oracle`](evidence/2026-08-02-perf3-11-stage0-oracle/).
The current implementation does not expose definition, phi or fixed-point
iteration counts; their absence is recorded in the oracle and Stage 3 must add
them before the replacement scaling verdict.

### Stage 1 — signal/effect semantic inventory and executable oracle

- [x] Inventory every opcode flagged as signalling, every handler-management
      opcode, calls, reference/indirect writes and numeric-context operations.
- [x] Derive actual write phase, payload/address and continuation behaviour
      from VM/plugin source; do not infer it from the current optimizer.
- [x] Define opcode-aligned metadata validation requiring every opcode to have
      an explicit signal contract; unknown is allowed but must be deliberate.
      Implement the selected table immediately after Gate 1, before Stage 2.
- [x] Add focused executable fixtures for before-write, after-write and
      partial-write failure; normal/skip; branch and call handlers; each
      retained action-aware handler result; safe failure of the retired legacy
      retry marker; inherited handlers; `sigpush`/`sigpop`; dynamic names;
      normal return; unwind; asynchronous entry; and observable signal/TRACE/
      source location.
- [x] Classify proposed semantic changes separately as: metadata correction to
      existing VM behaviour, compatible totalisation/non-signalling change, or
      incompatible/public contract change.
- [x] Produce an instruction-by-instruction decision table for any proposed
      `DCOPY`/`ITOS`/`FTOS`/`DTOS`/other `xtoy` change.

**Gate 1:** Adrian reviews the executable contract.  Any VM/instruction signal
semantic change requires explicit selection here; the scalable graph can
proceed with conservative existing semantics if none is selected.

Stage 1 analysis completed on 2026-08-02.  The current-behaviour fixtures pass
optimized/no-opt on both VMs.  The audit proves that `RXOP_SEM_MAY_THROW` is
both over-inclusive and incomplete, so the proposed graph uses a separate
opcode-aligned signal contract.  Gate 1 offers five independently recoverable
choices, with S1-S5 recommended.  Evidence and the instruction-by-instruction
table: [`2026-08-02-perf3-11-stage1-signal-contract`](evidence/2026-08-02-perf3-11-stage1-signal-contract/).

Gate 1 implementation locked on 2026-08-02. The selected S1-S5 contracts,
permanent dual-VM optimized/no-opt fixture, plugin unit tests, aligned metadata
validation, live ledger migration and documentation are complete. Focused
correctness passes 68/68; ordinary profiling-off Release builds pass; Richards,
Towers and RexxCPS RXBIN hashes remain byte-identical to Gate 0. Evidence:
[`2026-08-02-perf3-11-stage1-contract-lock`](evidence/2026-08-02-perf3-11-stage1-contract-lock/).

### Stage 2 — immutable per-procedure `FlowProcedure`

- [x] Separate procedure parsing/record ownership from graph construction and
      rewrite execution.
- [x] Form basic blocks at entry, labels, branches, calls where required,
      signalling instructions, handler entries and exits.
- [x] Add typed normal, branch, signal-skip, handler and
      terminal/unwind edges plus synthetic roots/exits.
- [x] Preserve exact source/TRACE record identity and mapping between queued
      RXAS records, instructions and emitted addresses.
- [x] Add deterministic graph dumps and focused graph-shape tests, including
      unreachable blocks, diamonds, nested loops and irreducible control.
- [x] Install an explicit graph epoch and make stale-analysis access fail
      closed.
- [x] Run emitted-image and existing optimizer parity before any consumer uses
      the new analyses.

**Gate 2:** graph construction must be linear in instructions plus edges,
produce no ordinary RXBIN change, and remain within the clean-base assembler
cost guard before adding SSA.

Gate 2 passed on 2026-08-02. The immutable sidecar has stable record,
instruction, block and emitted-address mappings; typed normal/branch/signal/
handler/exit edges; seven synthetic roots/exits; deterministic dumps; and
fail-closed epochs/unknown control. Construction uses a hashed label index and
append-only edges. The final legacy graph hands over resolved opcode pointers
before being freed, avoiding duplicate parsing and graph-memory overlap.
Focused correctness passes 113/113 and all three Gate 0 RXBIN hashes are exact.
Thirty balanced/interleaved Release rounds put median assembler deltas at
+0.411% Richards, +0.463% Towers and -2.784% RexxCPS; RSS deltas remain below
the greater-than-5%-and-1-MiB escalation rule. Evidence:
[`2026-08-02-perf3-11-stage2-flow-graph`](evidence/2026-08-02-perf3-11-stage2-flow-graph/).

### Stage 3 — reusable structural analyses and analysis manager

- [x] Compute reachable reverse-postorder once per graph epoch.
- [x] Add dominators and dominance frontiers for sparse phi placement.
- [x] Add strongly connected components, backedge classification and a loop
      forest; source order is retained for diagnostics, not used as a proof of
      dominance.
- [x] Review post-dominance/must-execute scope. No Stage 3 consumer requires
      it, so it is deliberately not instantiated before the first such query.
- [x] Cache analyses under the owning procedure and declare preservation/
      invalidation for every pass.
- [x] Expose RPO, unique predecessors, dominance and sparse-frontier surfaces
      for later priority/worklist and point-proof consumers; no per-generator
      complete-procedure rescan is introduced.
- [x] Add per-analysis work/memory counters and a configurable internal budget
      whose exhaustion disables optimization safely.

**Gate 3:** structural tests, deterministic dumps and scaling counters pass
without changing emitted code.

Gate 3 passed on 2026-08-02. The epoch-owned, demand-driven manager caches
reachable RPO, unique predecessor sets, dominators/tree intervals, sparse
dominance frontiers, SCCs, backedges and natural/irreducible loop regions.
The original graph marked signal-retry-only loops separately; K04d1 removes
those synthetic cycles after the approved retirement. Deliberate low-budget
failure, larger-budget analysis retry, stale epochs, unreachable blocks,
diamonds, nested and
irreducible loops, parallel signal edges and deterministic dumps are permanent
contract tests. Final focused correctness passes 113/113 and all three Gate 0
RXBIN hashes remain exact.

The first ordinary implementation eagerly solved unused analyses and crossed
the Richards RSS guard by 1,155,072 bytes. It was rejected and retained in the
evidence. Analysis construction is now demand-driven: diagnostics and future
consumers request the cache, while ordinary consumer-free assembly pays no
solver allocation. Thirty balanced/interleaved final rounds versus frozen
Stage 0 measure elapsed deltas of -7.087% Richards, -1.411% Towers and -0.865%
RexxCPS. RSS deltas are +499,712, +344,064 and +253,952 bytes respectively,
all below the combined greater-than-5%-and-1-MiB escalation rule. Evidence:
[`2026-08-02-perf3-11-stage3-structural-analysis`](evidence/2026-08-02-perf3-11-stage3-structural-analysis/).

### Stage 4 — sparse signal-policy and effect SSA

- [x] Represent procedure-entry handler policy as an explicit parameter,
      preserving inherited unknown state.
- [x] Version policy changes made by handler installation and
      `sigpush`/`sigpop`; use exact signal entries where statically known and a
      conservative whole-policy clobber for dynamic/unknown names.
- [x] Model calls, normal returns and unwind so callee-local policy changes do
      not become caller state while inherited handlers remain visible in the
      callee.
- [x] Represent call/reference/context/TRACE effects with sparse versions or
      summaries rather than one global barrier when a narrower fact is proved.
- [x] Bind skip/handler/fail edges to the correct policy and observation
      versions.
- [x] Prove equivalence with all Stage 1 executable signal fixtures.

**Gate 4:** no component-value proof may consume a signal edge until this
layer returns a valid edge-specific state or explicit unknown.

Gate 4 passed on 2026-08-02. The epoch manager now caches a demand-driven
sparse signal analysis with an inherited entry-policy parameter, exact static
policy writes, whole-policy clobbers for unresolved opcodes, cyclic phi
resolution and edge-specific failure states. Numeric-context, plugin, locale,
external, reference-visible, TRACE and call effects have separate write-once
identities. Parallel normal/skip edges remain separate phi inputs even when
their predecessor block is the same.

Calls preserve caller handler policy because the callee handler table is
frame-local/copy-on-write, but call/reference effects advance because VM
arguments point at caller-owned value storage. `sigpush` leaves the active
handler unchanged; its silent allocation-failure path makes subsequent
`sigpop` restoration explicitly stack-unknown. Strict GNU90 checks pass, the
focused matrix passes 113/113, all Gate 0 images are exact and the ordinary
assembler cost/RSS gate passes. Evidence:
[`2026-08-02-perf3-11-stage4-signal-policy`](evidence/2026-08-02-perf3-11-stage4-signal-policy/).

### Stage 5 — symbolic storage and component SSA

- [x] Replace dense point-by-register environments with immutable `StorageId`
      definitions and sparse register-to-storage mapping through `LINK`,
      `SWAP`, `UNLINK`, calls and references.
- [x] Give each storage component write a write-once `ValueId`; place pruned
      phis only for live/relevant components at joins.
- [x] Represent known present, known absent, constant, derived and unknown
      values without conflating absent/null with an unavailable proof.
- [x] Version numeric and other derivation contexts and name them in derived
      facts.
- [x] Apply instruction writes separately for normal and skip/failure edges
      according to the Stage 1 contract.
- [x] Add call/alias summaries conservatively; unsupported indirect mutation
      invalidates only what cannot be proved unaffected.
- [x] Match the retained P1 storage-identity results and current accepted
      component facts before replacing their implementation.

**Gate 5:** memory must scale with relevant definitions, uses and phis rather
than `nodes x registers x components`; the canonical inlined RexxCPS procedure
must complete within the agreed assembler guard.

Gate 5 passed on 2026-08-02 under Adrian's explicit allowance that proof
analysis may take seconds rather than the roughly 50 ms ordinary-assembly
baseline.  The accepted cache uses persistent mapping/value definitions and
lazy point queries; `rxas -d` materializes actual derivation sites, not every
point/register/component combination.  Canonical diagnostic elapsed/peak RSS
is 0.21 s/10,911,744 B Richards, 0.07 s/6,406,144 B Towers and
0.28 s/18,710,528 B RexxCPS.  RexxCPS retains 13,538,512 B across five
per-procedure analyses and no procedure exhausts its work budget.

Two rejected implementations remain recorded: recursive dynamic-storage
classification reached 82.51 s before termination, and eager materialization
completed in 0.78 s but peaked at about 305 MB.  Generation-marked traversal
and derivation-site demand are the locked replacements.  Unknown storage is
distinct from valid zero-based `ValueId 0`; implicit integer-coded local
writes are definitions; two-register derivations name their actual source;
and fused mapping/value instructions fail component proofs closed until
intra-instruction ordering becomes canonical metadata.

Strict GNU90 checks pass, the focused matrix passes 113/113 and all Gate 0
RXBIN hashes are exact.  Ordinary median Release assembly is 61.113 ms
Richards (+5.895%, +3.402 ms), 19.939 ms Towers (+0.093%) and 54.526 ms
RexxCPS (+0.009%).  The Richards increase is disclosed rather than hidden;
all three remain far inside the agreed proof-analysis budget.  RSS increases
are 557,056 B, 385,024 B and 172,032 B, below the combined greater-than-5%-and-
1-MiB escalation rule.  Evidence:
[`2026-08-02-perf3-11-stage5-sparse-ssa`](evidence/2026-08-02-perf3-11-stage5-sparse-ssa/).

### Stage 6 — proof service and first authority migration

- [x] Implement the query surface above with per-epoch caching and diagnostic
      reasons for failed proofs.
- [x] Implement generic dominated-success/repetition proof using SSA identity,
      success-stability metadata and signal/effect dependencies.
- [x] Implement loop availability, must-execute and speculatability queries,
      initially failing closed for may-signal motion.
- [x] Baseline the retained old decision/image and compare the new proof on
      focused fixtures and representative generated RXAS.
- [x] Migrate accepted storage/component/trace-safe `ITOS` behaviour to the
      service and make the new proof the sole authority.
- [x] Delete the old ITOS solver after the new proof covered its safe domain;
      retain the pre-refactor checkpoint and old emitted image for replay.

**Gate 6:** the new infrastructure preserves the old solver's valid safe
domain and may prove a larger domain.  Exact decision or image parity is not a
gate: newly accepted cases require their own correctness proof and, when they
change ordinary output, the mandatory first Release verdict.

Gate 6 passed on 2026-08-02.  The old solver's 21-`ITOS` image is retained as
the baseline; the new service is the sole authority and produces 19 `ITOS`.
Five primary-procedure repetitions are proved and two fail closed with
`generator-source-unknown`.  Proof analysis completes in 0.39 s at 20.2 MB
peak RSS and ordinary RexxCPS assembly remains in the tens of milliseconds.
Evidence:
[`2026-08-02-perf3-11-stage6-proof-service`](evidence/2026-08-02-perf3-11-stage6-proof-service/).

### Stage 7 — select the first new consumer

Compare, but do not combine automatically:

- **C0:** infrastructure only; no new rewrite.
- **C1:** generic dominated-success redundant `xtoy` elimination, beginning
  with instructions whose existing signal contract is already sufficient.
- **C2:** direct destination/materialisation and temporary-copy elimination
  using component/storage SSA.
- **C3:** loop-invariant conversion motion using loop/must-execute proofs;
  total non-signalling operations first.
- **C4:** consolidated swap/swap or copy cleanup using storage identity.
- **C5:** an explicitly approved instruction totalisation/non-signalling change
  followed by one of C1-C3.

For each eligible option retain static opportunity counts, exact semantic
obligations, focused dual-VM correctness, assembler cost, dynamic-instruction
ceiling and representative runtime ceiling.  Keep rejected options replayable.

**Gate 7:** Adrian selects one bounded production consumer or retains
infrastructure only.  Do not let several PoCs layer into one candidate.

Gate 7 selected the bounded C1 expansion of the already accepted `ITOS`
consumer: let the new proof establish additional dominated repetitions, but
do not yet generalize the rewrite to other `xtoy` operations.

### Stage 8 — selected consumer and mandatory first Release verdict

- [x] Implement only the selected consumer on the accepted infrastructure.
- [x] Run minimum focused graph/metadata/optimized/no-opt dual-VM correctness.
- [x] Freeze code immediately after that minimum proof.
- [x] Build the ordinary profiling-off Release product.
- [x] Run the smallest decisive end-to-end target and unrelated-work guards
      against retained valid baseline evidence, adding only a governed drift
      control when needed.
- [x] Retain exact static rewrite decisions and image counts; no separate
      counts-only profile was required to interpret the exact two-instruction
      static delta.
- [x] Report the verdict and stop for Adrian's accept/rework/revert decision.

Adrian accepted the first Release verdict on 2026-08-02.  Against the retained
21-`ITOS` Stage 5 image, the 19-`ITOS` new-proof image improves median canonical
RexxCPS throughput by 7.469% on `rxvm` and 6.866% on `rxbvm`, with 12/12
favourable pairs for each VM and all executions correct.

### Stage 9 — accepted closeout only

- [x] Remove only tactical guards/rules whose replacement proof is exact and
      covered; list every retained guard and why it remains.
- [x] Run proportional focused, broad Debug and Release validation after
      first-verdict acceptance.  Sanitizer/install/package expansion was not
      required by the accepted scope or any observed failure.
- [x] Update RXAS technical documentation and the live roadmap; do not edit
      the dated charter.
- [x] Retain one compact checksum-closed evidence bundle with negative and
      rejected results referenced rather than duplicated.
- [x] Review the final diff and commit locally when authorized; push remains a
      separate decision.

The initial broad run exposed one incorrect advanced proof across a range-call
argument and passed 1,986/1,987.  The fix models caller-owned explicit and
range-call arguments in sparse SSA rather than reinstating a global call
barrier.  Focused validation then passes 10/10 and broad Debug passes
1,987/1,987; the accepted RexxCPS hash is unchanged by the correction.  All
other tactical optimizers and their guards remain pending the explicit
migration programme below.

### Stage 10 — remaining legacy-proof migration

This is a migration programme, not a like-for-like parity exercise.  Before
editing the next consumer:

- [x] inventory every remaining legacy proof, rewrite owner and tactical guard;
- [x] assign a stable migration ID and classify proof versus mechanical
      peephole, liveness consumer or pure emission transform;
- [x] retain its current accepted/rejected decisions on focused fixtures and
      representative Richards, Towers and RexxCPS RXAS;
- [x] record its semantic dependencies, known conservative gaps, image deltas,
      assembler cost and correctness oracle;
- [x] map M01 to the required new graph/signal/storage/value/effect query;
      retain the corresponding mapping task for each later migration, adding a
      generic query only when the current surface is insufficient;
- [x] migrate M02 repeated integer/bitwise-float loads to the generic
      component-value/equivalent-constant query, including hidden reference and
      native-payload cleanup proof; and
- [x] migrate each remaining authority one at a time, delete the superseded
      solver, then prove the old safe domain plus any separately identified
      stronger domain.

A changed decision is not a mismatch by itself.  A new acceptance requires a
positive write-once/flow proof and focused adversarial correctness; a new
ordinary output also triggers the mandatory first Release verdict.  Rejected
and superseded decisions remain replayable.  Any inability to recover an old
safe case is recorded as a regression or an explicit correctness correction,
not hidden by aggregate counts.

The locked inventory and one-authority execution ledger are in
[`PERF3-11-MIGRATION-WORKLIST.md`](PERF3-11-MIGRATION-WORKLIST.md), with its
retained decision oracle in
[`2026-08-02-perf3-11-legacy-proof-baseline`](evidence/2026-08-02-perf3-11-legacy-proof-baseline/).
M01 is complete in
[`2026-08-02-perf3-11-m01-xtoy`](evidence/2026-08-02-perf3-11-m01-xtoy/).
It recovers the one old `ITOF` acceptance and generalizes the consumer to all
metadata-admitted one-register `XTOY` derivations.  Exact metadata exists for
all 20 conversions; the focused corpus proves 12 deletions, including four
`ITOD` flow shapes, while signal, effect and same-component idempotence gaps
remain closed.  `ITOD` and `BTOD` now have the coherent total non-signalling
runtime/plugin contract required by the proof.  Canonical Richards, Towers
and RexxCPS output is unchanged, focused replay passes 51/51 and broad Debug
passes 1,989/1,989.

M02 is complete in
[`2026-08-03-perf3-11-m02-constant-write`](evidence/2026-08-03-perf3-11-m02-constant-write/).
The proof service is now sole authority for repeated exact scalar constants.
It recovers the old floor and proves equal-value phi, exact-float,
linked-storage and ordered-TRACE cases while retaining different phis, signed
zero and hidden cleanup.  Native payload is an explicit eighth component;
integer/float writes may be deleted only when both reference and native payload
are already absent.  Adrian accepted the output-neutral Release verdict and
procedure-local 30.1 MB peak-RSS boundary on 2026-08-03.  Focused replay passes
53/53, Release hidden-cleanup execution passes 4/4, canonical output is exact
and broad Debug passes 1,991/1,991.

M03 is complete in
[`2026-08-03-perf3-11-m03-absent-write`](evidence/2026-08-03-perf3-11-m03-absent-write/).
The proof service is sole authority for repeated `NULL`: known target storage
and all eight component leaves must already be absent.  It recovers the old
floor and adds equal-phi, linked-storage and ordered-TRACE deletions while
retaining scalar, reference and native-payload cleanup.  Adrian accepted the
output-neutral Release verdict on 2026-08-03.  Canonical images are
byte-identical and broad Debug passes 1,993/1,993.

M04 is complete in
[2026-08-03-perf3-11-m04-self-copy](evidence/2026-08-03-perf3-11-m04-self-copy/).
The proof service is sole authority for exact same-storage COPY, ICOPY, FCOPY,
SCOPY, DCOPY, ACOPY and BCOPY. It recovers the four old raw-register cases and
adds raw decimal/attribute/binary, LINK, agreeing-phi and TRACE-safe
deletions, while divergent and different storage remain closed. Adrian
accepted the output-neutral Release verdict on 2026-08-03; canonical images
are byte-identical and broad Debug passes 1,995/1,995.

M05 is complete in
[2026-08-03-perf3-11-m05-sparse-use-liveness](evidence/2026-08-03-perf3-11-m05-sparse-use-liveness/).
One cached per-epoch use/dependency index now owns direct component uses,
storage/cursor observations, reverse phi dependencies and edge-aware liveness.
The proof service returns atomic typed-copy operand-rewrite plans and the old
dense availability/may-reach authority is deleted. All ten old safe accepts
are recovered; `copy_before_endlife` is one stronger acceptance because the
unrelated exceptional ENDLIFE does not observe the copied value. Richards,
Towers and RexxCPS remain byte-identical to M04. Adrian accepted the bounded
ordinary Release scale of 0.16-0.17 s and 102.8 MB peak RSS for RexxCPS, and
broad Debug passes 1,995/1,995.

M06 is complete in
[2026-08-03-perf3-11-m06-producer-forwarding](evidence/2026-08-03-perf3-11-m06-producer-forwarding/).
One immutable producer-forward proof now follows `StorageId`, the producer's
exact component `ValueId`, and sparse direct/dependent uses before retargeting
an adjacent `ICOPY`/`FCOPY` producer and deleting the copy. All eleven current
M05-boundary accepts are recovered; the historical twelfth is already owned by
M05. The proof newly requires every producer-cleared reference/native payload
to be absent in both storages, closing a correctness gap in the old typed-view
liveness test. The old dense M06 solver and the last M08 semantic liveness/
availability/may-reach authority are deleted; only mechanical use/kill and
taint bitsets with live diagnostic/keyhole consumers remain.

Adrian accepted the first Release verdict on 2026-08-03. Focused and canonical
images are byte-identical to frozen M05. Paired RexxCPS assembly medians remain
0.18 s; median peak RSS rises 1.05% to 104,103,936 bytes. Focused Debug and
Release pass 8/8, and broad Debug passes 1,995/1,995 in 291.22 seconds after a
stale NR-09 expectation was updated for the intended direct-destination form.
K04a now makes matching Boolean TRACE deletion scalable and atomic. The proof
records any number of exact result `ValueId` observations between compare and
branch, revalidates their record/register/component identity, deletes them with
the compare and retains unrelated events at the fused branch boundary. A later
result event still rejects; all nine focused no-opt events remain. Focused
Debug and ordinary Release pass 5/5, and strict GNU90 passes with the one
pre-existing unused-parameter warning.

The canonical audit proves K04a alone unlocks zero RexxCPS fusions. Its five
former `trace-observed` candidates now reach the next conservative guard and
reject as `call-window-observed`; other historical cases fail earlier storage,
alias or cleanup proofs. K04a is semantically identical to the strict K04 image
at 1,250 VM instructions and 1,261 trace events, still nine instructions and
nine events above frozen M06. Runtime timing was therefore correctly skipped.

K04b replaces the procedure-global numeric range veto with a reusable
edge-aware call-window query. A constant count gives exact bounds; an
unavailable count safely reaches the highest local. The compare proof follows
the exact result `ValueId` through copy/derived values and lazily materialized
phis and rejects only when that value is visible in the resulting window or a
query fails closed. Focused positive, mixed-phi, LINK, SWAP and no-opt cases
pass 5/5 in Debug and Release. Its ordinary Release `rxas` has SHA-256
`8a42d71e630672b37312a0728490160ea252ae13645c00ec924d2a8f8769ce11`.
Canonical output remains byte-identical to K04a, so Adrian accepted K04b as a
neutral migration/consolidation result on 2026-08-03; no runtime timing was
warranted.

K04c attributes all five residual RexxCPS rejections. The two relevant counted
calls are each immediately preceded by `load r0,1`, so their real argument
window is exactly `r1`; the rejected compare results are in `r2` or `r6`.
Unknown CALL signal metadata nevertheless turns the retry-edge count into a
phi and widens the analysis window to `r1..r69`. Three candidates then find the
old result through `r2`; two fail closed while querying unrelated unknown `r43`.
None is an actual caller-window observation. VM source confirms that an
action-aware signal handler runs in its own frame and `retry` resumes at the
recorded call instruction. The count operand is reread but is not exposed to
that handler; caller-owned argument values may change by reference and remain
covered by the existing storage-identity range clobber. K04c is analysis-only
and its diagnostics were removed after capture. Strict GNU90 passes with the
one pre-existing warning, the complete Debug build passes and broad Debug
passes 1,995/1,995 in 376.48 seconds.

K04d0 reviewed instruction-level retry before locking that design. Repository
search found no production caller, standard REXX resumes a `CALL ON` trap after
the clause rather than re-executing the faulting instruction, and the first
end-to-end native retry fixture exposed an existing fused-call mapping defect.
Retry also creates a self-loop and failure-state phi at every potentially
signalling instruction and cannot safely repeat arbitrary partial writes,
by-reference mutation or external side effects. Adrian approved retirement on
2026-08-03. K04d1 removes the public factory, VM continuation, CFG edge and
retry-only loop machinery; a legacy internal marker fails safely. The coherent
propagated-call partial-state contract remains for skip, handler and unwind
analysis. Focused language, bytecode, CALL, alias and signal-lifecycle proof is
required before the first Release runtime verdict.

K04d2 passes the same 14 focused checks in Debug and ordinary profiling-off
Release. K04d3 then compared the retained M06 runtime image with the frozen
candidate using one warmup and 12 balanced/interleaved recorded rounds per VM.
The candidate is structurally smaller at 1,222 VM instructions and 1,249 TRACE
events versus M06 at 1,241/1,252, but this does not produce a material runtime
gain: median RexxCPS is +0.021% on both `rxvm` and `rxbvm`, with mixed pair
directions of 6/12 and 5/12. One retained `rxvm` candidate sample is 13% below
its pair, so that cell is formally noisy and rerun-recommended; no sample was
removed. Adrian accepted K04d1 as a neutral semantic/infrastructure improvement
on 2026-08-03 without a noisy-cell append. K04d4 closeout passes the complete
Debug build and 1,998/1,998 broad Debug tests in 297.92 seconds. The production
retirement audit finds no remaining retry enum, continuation, CFG edge, loop
classification or factory; only the deliberate negative/legacy-marker
fixtures, current retirement documentation and historical provenance remain.
K04 is closed.

K02/K03 evidence is retained in
[2026-08-03-perf3-11-k02-k03-linked-reads](evidence/2026-08-03-perf3-11-k02-k03-linked-reads/).
The migration replaces the twelve syntax-expanded duplicate direct/attribute
linked-read rules with one storage/component/path proof and atomic rewrite plan. The
new attribute-count component and interned one-based attribute path follow
owner aliases by `StorageId` while count/reference generations, signals and
dynamic or invalid slots fail closed. A sparse write index covers storage-phi
leaves, cursor state and opaque-write barriers without misclassifying writes
as observations. All six copy families are covered on direct and attribute
paths; the safety panel includes changed values/count/cursor, different
owners/slots, aliased calls, divergent phis and signal skips.

The migration found and fixed two manifestations of one cross-consumer
integration error: opaque writes initially disabled the fourteen accepted K04
RexxCPS fusions, and explicit pure writes initially disabled four retained
M05/M06 expectations in the first broad gate. The proof service now classifies
pure writes once and never treats their absent `ValueId` as an unknown read.
Focused K02/K03 passes 21/21 in Debug and Release; the combined
flow/K02/K03/K04/M04/M05/M06 panel passes 28/28 in both builds; and broad Debug
passes 2,010/2,010 in 678.89 seconds. Frozen/basic focused outputs are
byte-identical; the two relevant metadata/TRACE rows are documented stronger
acceptances.
Canonical Richards, Towers and RexxCPS have zero K02/K03 accepts and remain
byte-identical to frozen K04, so no runtime timing is warranted.

K01 is complete in
[2026-08-03-perf3-11-k01-storage-permutation](evidence/2026-08-03-perf3-11-k01-storage-permutation/).
The sparse storage-permutation proof is now the sole authority for exact
cancelling `SWAP` pairs and exact self-cancelling ordered `SWAPN`; the two old
raw-register keyholes are deleted. The proof removes its bounded queue limit,
models signal continuations and rejects relevant observations, writes, call
windows, asynchronous handlers and control splits. The migration also corrects
overlapping `SWAPN` SSA transfer to compose pairs in instruction order. The
inherited floor and both legacy orientations are byte-identical, the stronger
optimized/no-opt panel passes 9/9 in Debug and Release, the shared proof panel
passes 37/37 in both builds, and broad Debug passes 2,012/2,012 in 368.43
seconds. Canonical Richards, Towers and RexxCPS have zero K01 accepts and
byte-identical images, so runtime timing is not warranted.

K06 is complete in
[2026-08-03-perf3-11-k06-mechanical-classification](evidence/2026-08-03-perf3-11-k06-mechanical-classification/).
Full `COPY` writes the exact status component immediately repeated by
same-pair `ACOPY`; both opcodes are non-signalling and the existing rule admits
no intervening executable instruction. K06 is therefore a metadata-proved
mechanical encoding, not a semantic proof consumer. The production rule is
unchanged, its original optimized/no-opt outputs are byte-identical, the
expanded metadata/optimizer panel passes 3/3 in Debug and Release, and all
three canonical images retain zero accepts and exact hashes. Runtime timing
and broad CTest are not warranted because no production code or ordinary
output changed. K05 and the D0.2 structural consolidation are complete: M00
and all seven K05 forms share the sole immutable CFG, M07 diagnostics use
sparse SSA, and the legacy graph/dense storage matrices are deleted. D0.3 is
also complete: M01-M06 and K01-K04 now request only their declared semantic
capabilities from one monotonic per-epoch proof service, loop analysis remains
dormant, and an explicit diagnostic route preserves `-d` evidence. Focused
Debug/Release and dual-VM runtime panels pass and accepted K05 hashes remain
exact. D0.4 now migrates compatible M01-M06 and K02-K04 semantic use cases to
a sparse validated transaction while leaving K01 and CFG-changing K05 in
separate epochs. Its transaction and multi-family proofs pass, the final
Debug/Release optimizer and migrated-runtime panel passes 107/107, and
accepted canonical images remain exact. Generated `Parse.rxas` applies 63
semantic plans in 12 batches at D0.4. D0.5 closes the scale failure by
eliding redundant storage phis, adding a linear write-once/single-use local
typed-copy route, and bounding whole-procedure SSA at 262,144
block-by-register join cells. Over-bound procedures retain local and CFG
consumers while semantic analysis and diagnostic SSA/use dumps fail closed.
Generated `Parse.rxas` now peaks at 142.7-142.9 MB in ordinary Release versus
2.56 GB at D0.4, retains all 261 K05 rewrites, and explicitly retains six M05
copies whose reused raw registers need a future candidate-sliced/region proof.
The final Debug/Release optimizer/runtime panel passes 108/108, broad Debug
passes 2,021/2,021, and the three canonical same-input images remain exact.

### Stage 11 — later consumers after legacy migration

Only after the foundation and first consumer are accepted:

- add further `xtoy`/GVN/PRE and loop-invariant code-motion consumers one at a
  time with their own signal-observation proof;
- rereview RXC clause lowering under PERF3-12 and keep simple emitter fixes
  separate from RXAS analysis;
- add a final constrained register-allocation/compaction pass after all
  rewrites, preserving parameter/call windows, reference/link identity,
  signal-unwind restoration, TRACE/source operands and `.locals`; and
- consider register reuse only from interference/liveness proof, not merely
  because numeric gaps exist.

Register compaction must never be used to mask an analysis scalability
failure.  PERF3-11 first makes RXAS scale with the current register space.

#### Future capability ledger

These IDs organize already-approved future directions; they are not authority
to combine consumers or bypass each consumer's correctness and Release gate.

| ID | Capability | Required proof owner | Route |
| --- | --- | --- | --- |
| FC01 | sparse storage/component use index and edge-aware liveness | immutable CFG, SSA and signal continuations | PERF3-11 Phase B; unlocks M05/M06/K04 |
| FC02 | further XTOY, GVN and partial-redundancy elimination | value/effect equivalence | PERF3-11 after legacy migration |
| FC03 | loop-invariant code motion and representation hoisting | loop membership, must-execute, invariance and signal observation | PERF3-11 one consumer at a time |
| FC04 | direct destination/materialisation and temporary elimination | destination unobserved plus atomic use redirect | PERF3-11 M05/M06 infrastructure |
| FC05 | final register-number compaction | preserved windows, links, unwind, TRACE and `.locals` | PERF3-11 after all rewrites |
| FC06 | register reuse/allocation | interference and component/storage liveness | PERF3-11 after scalable liveness |
| FC07 | compiler clause and variable/representation hoisting | RXC lowering audit separated from RXAS proof | PERF3-12 |
| FC08 | candidate-sliced or region SSA for over-bound procedures | exact candidate boundary, signal continuations, storage/value/use closure and bounded work | PERF3 follow-on only when deferred advanced cases have material value |

#### RXC-to-RXAS architecture-transfer ledger

These are architecture activities rather than individual optimizer
capabilities. The objective is a simpler RXC that remains authoritative for
language semantics and type-correct lowering, while increasingly mature RXAS
proofs own machine-flow selection. Analysis may begin after M05 is locked, but
production ownership does not move until M05/M06 and K04 have exercised the
new use/liveness service on representative code.

| ID | Activity | Gate and ordering |
| --- | --- | --- |
| AT01 | classify every RXC optimization as semantic lowering, representation choice or machine-flow rewrite | analysis after M05; retain emitted-image and runtime oracles |
| AT02 | define intent/provenance metadata RXC must preserve for later RXAS decisions | after M05/M06 query design; no public format change without approval |
| AT03 | migrate eligible tactical RXC rewrites to RXAS one authority at a time | after M01-M06 plus K04 prove value, use, signal and call-window coverage |
| AT04 | redesign inlining ownership and cost model | PoC after M05 scale proof; select only after AT02 and K04 |
| AT05 | simplify RXC emitters and delete superseded optimizer state | only after each corresponding RXAS migration is accepted |
| AT06 | consolidate the compiler/assembler boundary and rebaseline compile, assemble, image-size and runtime costs | after staged migrations; before declaring the simplified architecture complete |

AT04 must compare at least the current early RXC inliner, late RXAS inlining,
and a hybrid in which RXC proves semantic eligibility while RXAS chooses and
lowers from actual instruction, register, signal and call-window costs. It
must also decide whether analysis runs before inlining, after inlining, or in
two bounded epochs. No inlining ownership change is selected by this ledger.

The provisional order is FC01/M05-M06, K04, AT01-AT02, the AT04 PoC/selection,
then staged AT03/AT05 migrations. Broader GVN/PRE and LICM (FC02-FC03) should
be designed against the selected inlining boundary; register compaction and
reuse (FC05-FC06) remain later consumers.

## Acceptance criteria for the infrastructure

The infrastructure is accepted only when all of the following hold:

1. every signalling opcode has a validated explicit contract or a deliberate
   fail-closed unknown classification;
2. focused signal lifecycle tests demonstrate normal, skip, handler and
   unwind state against both VM modes where applicable;
3. the graph and analyses are owned and cached per procedure with explicit
   epochs/invalidation;
4. persistent state is sparse in definitions/uses/phis rather than dense in
   all nodes, registers and components;
5. each migrated consumer preserves the legacy proof's valid safe domain, while
   stronger new decisions are separately evidenced rather than rejected for
   lack of exact parity;
6. canonical RexxCPS, Richards and Towers assembly completes inside the agreed
   clean-base elapsed/RSS guard, with deterministic internal counters retained;
7. proof APIs handle joins, loops, calls, symbolic storage and signal edge
   state without raw-register or source-order assumptions;
8. unsupported cases and work-budget exhaustion fail closed and have tests;
9. no tactical optimizer rule is deleted without equivalent or stronger new
   proof and retained replay evidence; and
10. no public ISA, RXBIN, ABI, VM signal or language semantic change is bundled
    without its separately recorded approval.

The D0 scale consolidation adds a routing invariant: a consumer may request
only the capabilities declared in `rxas_flow_pass.c`. Exact local algebra does
not construct a graph; M00/K05 construct CFG state only; current component
consumers request SSA/use facts explicitly; loop/SCC/frontier work remains
dormant until a production hoisting or PRE consumer is selected. A conservative
candidate census may overselect, but may never suppress an otherwise eligible
proof query.

## Preparation completed

- [x] Preserve all provisional work and evidence on a local checkpoint branch.
- [x] Delete only the five explicitly authorized untracked lifecycle binaries.
- [x] Create the clean dedicated infrastructure branch from the accepted tip.
- [x] Record the scalable graph/SSA/signal architecture and execution gates.
- [x] Receive Adrian's authorization to begin Stage 0 and then execute the
      staged plan.
