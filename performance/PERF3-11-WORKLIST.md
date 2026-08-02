# PERF3-11 scalable RXAS flow and signal-proof infrastructure

Status: **in progress — Stage 5 locked; Stage 6 proof service/parity next**

Architecture approved: 2026-08-02

Purpose: replace the growing collection of candidate-by-candidate RXAS flow
solvers with one scalable, per-procedure analysis foundation.  The foundation
must model symbolic storage, component values, calls, mutable signal policy and
normal/skip/retry/unwind continuations accurately enough to support reusable
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
changing skip/retry behaviour is a separate semantic decision.  PERF3-11 may
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
8. **Observable ordering is preserved.** TRACE/source events, signal payload,
   interrupted address, handler order and call/unwind state constrain code
   motion and deletion.
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
- component/storage/context reads and writes on normal, skip and retry paths;
- signal payload and interrupted-address observability;
- permitted continuation classes: normal, skip, retry, branch/call handler,
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
- **retry** applies the failure-phase state and returns to the instruction
  without inventing a successful result;
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
      partial-write failure; normal/skip/retry; branch and call handlers;
      each action-aware handler result; inherited handlers; `sigpush`/`sigpop`;
      dynamic names; normal return; unwind; asynchronous entry; and observable
      signal/TRACE/source location.
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
- [x] Add typed normal, branch, signal-skip, signal-retry, handler and
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
Signal-retry-only loops are marked separately. Deliberate low-budget failure,
larger-budget retry, stale epochs, unreachable blocks, diamonds, nested and
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
- [x] Bind skip/retry/handler/fail edges to the correct policy and observation
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
- [x] Apply instruction writes separately for normal, skip and retry edges
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

### Stage 6 — proof service and dual-analysis parity

- [ ] Implement the query surface above with per-epoch caching and diagnostic
      reasons for failed proofs.
- [ ] Implement generic dominated-success/repetition proof using SSA identity,
      success-stability metadata and signal/effect dependencies.
- [ ] Implement loop availability, must-execute and speculatability queries,
      initially failing closed for may-signal motion.
- [ ] Run old and new proofs side by side in a diagnostic/test mode and compare
      decisions on focused fixtures and representative generated RXAS.
- [ ] Migrate existing accepted storage/component/trace-safe `ITOS` behaviour
      to the service without expanding its rewrite coverage.
- [ ] Delete an old solver only after decision, emitted-image and scaling
      parity are retained; keep the pre-refactor checkpoint for replay.

**Gate 6:** the new infrastructure reproduces accepted behaviour and assembly
performance before any broader consumer is selected.

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

### Stage 8 — selected consumer and mandatory first Release verdict

- [ ] Implement only the selected consumer on the accepted infrastructure.
- [ ] Run minimum focused graph/metadata/optimized/no-opt dual-VM correctness.
- [ ] Freeze code immediately after that minimum proof.
- [ ] Build the ordinary profiling-off Release product.
- [ ] Run the smallest decisive end-to-end target and unrelated-work guards
      against retained valid baseline evidence, adding only a governed drift
      control when needed.
- [ ] Retain exact static rewrite decisions and dynamic instruction counts.
- [ ] Report the verdict and stop for Adrian's accept/rework/revert decision.

### Stage 9 — accepted closeout only

- [ ] Remove only tactical guards/rules whose replacement proof is exact and
      covered; list every retained guard and why it remains.
- [ ] Run proportional focused, broad Debug, sanitizer and Release/package
      validation after first-verdict acceptance.
- [ ] Update RXAS/VM technical documentation and the live roadmap; do not edit
      the dated charter.
- [ ] Retain one compact checksum-closed evidence bundle with negative and
      rejected results referenced rather than duplicated.
- [ ] Review the final diff and commit locally when authorized; push remains a
      separate decision.

### Stage 10 — later consumers, not part of the first infrastructure verdict

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

## Acceptance criteria for the infrastructure

The infrastructure is accepted only when all of the following hold:

1. every signalling opcode has a validated explicit contract or a deliberate
   fail-closed unknown classification;
2. focused signal lifecycle tests demonstrate normal, skip, retry, handler and
   unwind state against both VM modes where applicable;
3. the graph and analyses are owned and cached per procedure with explicit
   epochs/invalidation;
4. persistent state is sparse in definitions/uses/phis rather than dense in
   all nodes, registers and components;
5. current accepted optimizer decisions and ordinary RXBIN outputs are
   unchanged before a new consumer is selected;
6. canonical RexxCPS, Richards and Towers assembly completes inside the agreed
   clean-base elapsed/RSS guard, with deterministic internal counters retained;
7. proof APIs handle joins, loops, calls, symbolic storage and signal edge
   state without raw-register or source-order assumptions;
8. unsupported cases and work-budget exhaustion fail closed and have tests;
9. no tactical optimizer rule is deleted without equivalent new proof; and
10. no public ISA, RXBIN, ABI, VM signal or language semantic change is bundled
    without its separately recorded approval.

## Preparation completed

- [x] Preserve all provisional work and evidence on a local checkpoint branch.
- [x] Delete only the five explicitly authorized untracked lifecycle binaries.
- [x] Create the clean dedicated infrastructure branch from the accepted tip.
- [x] Record the scalable graph/SSA/signal architecture and execution gates.
- [x] Receive Adrian's authorization to begin Stage 0 and then execute the
      staged plan.
