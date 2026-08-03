# PERF3-11 remaining RXAS proof migration

Status: **in progress — M01-M05 complete; M06 producer forwarding next**

Authorized: 2026-08-02

Purpose: replace every remaining legacy RXAS semantic proof with the accepted
per-procedure graph/signal/storage/component proof infrastructure.  The target
is not like-for-like decision parity.  The old implementation defines a
retained minimum safe-capability baseline; the new write-once proof service may
establish a larger safe domain when that expansion is separately evidenced.

No push is authorized.

## Clean baseline

- Foundation commit: `6cdc3733597e1e2a719b1a425fcd0d6127455114`
  (`perf: add reusable RXAS proof service`).
- Frozen Stage 6 Release `rxas`:
  `bb79eb375bdc4e95fe52b604d76b45f469d0154b5f97f3aea441eeca230ff9ab`.
- Stage 6 proof authority: repeated one-register `ITOS`; its old solver is
  already deleted and is not part of this remaining inventory.
- Baseline evidence:
  [`2026-08-02-perf3-11-legacy-proof-baseline`](evidence/2026-08-02-perf3-11-legacy-proof-baseline/).
- Focused optimizer oracle passes **49/49**.
- Richards, Towers and RexxCPS diagnostic assemblies complete with no disabled
  graph, signal, SSA or proof analysis.

The baseline adds one output-neutral diagnostic: each applied old keyhole rule
prints a deterministic FNV-1a signature of its complete input/output shape
under `rxas -d`.  The signature survives unrelated rule deletion or reordering
and does not participate in matching.  Identical focused RXBIN hashes before
and after the diagnostic prove output neutrality.

## Migration rules

1. **One authority at a time.** Do not layer several solver replacements into
   one candidate or one correctness verdict.
2. **Old safe domain is the floor.** Replay every accepted focused case and its
   negative guards.  A lost old case is a regression unless the old result is
   positively proved incorrect.
3. **New proof is authoritative.** The old solver does not veto a positive new
   result and is deleted after migration; it may run only as a diagnostic until
   that deletion.
4. **Stronger results are explicit.** Record new acceptances separately from
   recovered old cases, with exact proof reasons and emitted-image delta.
5. **Signals and observations are first-class.** Normal, skip, retry, handler
   and unwind state; TRACE/source events; payload/address observation; and
   caller-owned arguments are proof inputs, not blanket mnemonic guards.
6. **Write-once identities, not register numbers.** Proofs follow `StorageId`,
   component `ValueId`, effects and actual call/link/swap mappings.
7. **Fail closed and scale.** Unsupported state, budget exhaustion, incomplete
   control and allocation failure reject only the affected proof.  Canonical
   RexxCPS remains inside the accepted seconds-scale analysis budget.
8. **Mandatory output gate.** A newly accepted ordinary output freezes
   implementation after minimum focused correctness and triggers the required
   profiling-off Release verdict before migration continues.
9. **Replay remains available.** The foundation commit, old signatures,
   fixtures, decision CSVs and rejected cases are retained after solver
   deletion.

## Inventory: whole-procedure legacy graph

| ID | Current authority | Accepted baseline | New owner | Disposition |
| --- | --- | --- | --- | --- |
| M00 | complete-control reachability and dead record cleanup | canonical: Richards 5, Towers 7, RexxCPS 0; focused 4 instructions plus TRACE/source records | immutable graph reachability and rewrite plan | migrate after value consumers; structural, not value proof |
| M01 | `flow_compute_available_fact()` for repeated one-register `ITOF`, then metadata-admitted `XTOY` derivations | old floor: focused `ITOF` 1, canonical 0; other conversions are new domain | generic dominated-success repetition | **complete**; old authority deleted, all 20 one-register conversions described by metadata, 12 focused deletions proved |
| M02 | repeated identical integer/bitwise-float load availability | focused 1; canonical 0 | component value/equivalent constant proof | **complete**; old authority deleted, old floor plus four stronger focused deletions proved |
| M03 | repeated `NULL` all-view availability | focused 1; canonical 0 | storage presence plus all-component equivalence | **complete**; old authority deleted, old floor plus equal-phi, linked-storage and TRACE cases proved |
| M04 | exact full/typed self-copy deletion with address-observation scan | focused full-copy 1; canonical 0 | storage/value identity plus observation equivalence | **complete**; old authority deleted, all seven copy families use conditional same-storage metadata and the proof service |
| M05 | typed `ICOPY`/`FCOPY`/strict-`SCOPY` availability, may-reach and operand redirection | focused 10; canonical 0 | SSA use graph, edge-aware liveness and atomic rewrite plan | **complete**; old dense solver deleted, ten-case floor plus one stronger case proved |
| M06 | adjacent producer-destination forwarding | focused 12; canonical 0 | exact producer result, destination/temporary liveness and observation equivalence | migrate after M05 infrastructure |
| M07 | dense legacy register-storage must-analysis | diagnostic-only; 13 storage-identity fixtures | sparse SSA storage queries and proof diagnostics | retire after its oracle is expressed against new SSA |
| M08 | raw-register liveness/availability/may-reach substrate | shared by M03-M06 | graph use index plus component/storage liveness | retire after last consumer |

The representative canonical set currently exercises no remaining M01-M06
acceptance.  That does not make the migrations irrelevant: these solvers are
the bounded basic cases that the new proof is expected to generalize, and the
focused fixtures preserve their exact safety floor.

## Inventory: proof-bearing keyhole rules

| ID | Rule family | Accepted baseline | Current proof weakness | New owner |
| --- | --- | --- | --- | --- |
| K01 | two `SWAP`s cancel with no relevant intervening use | focused positive plus implicit-register/barrier negatives; canonical 0 | raw-register relevance and bounded queue | storage permutation plus observation equivalence |
| K02 | duplicate `LINK`/typed-copy/`UNLINK` read | focused string/binary positives and metadata/TRACE negatives; canonical 0 | syntax shape and raw-register hazard scan | storage/component availability and atomic use redirect |
| K03 | duplicate `LINKATTR1` read for same owner/slot | focused string/binary positives and different-slot negative; canonical 0 | syntax shape; no generic attribute identity | storage path/attribute identity plus component value proof |
| K04 | integer compare plus conditional branch, including ordered TRACE form | focused 16 positive shapes and live-result/call-window negatives; canonical RexxCPS 9 | bespoke bounded path solver for boolean-result death | CFG use/liveness, signal/TRACE observation and fused-op equivalence |
| K05 | branch-to-conditional/dual-branch threading | canonical Richards 4, Towers 7, RexxCPS 0 | queue-local label search | immutable CFG edge rewrite and reachability |
| K06 | full `COPY` followed by same-pair `ACOPY` | focused 1; canonical 0 | adjacent mnemonic rule | component-write subset/equivalence proof or retained mechanical rule |

K04 and K05 are the only remaining proof-bearing keyhole families exercised by
the three representative canonical inputs: nine compare fusions in RexxCPS and
eleven branch-threading rewrites across Richards/Towers.

## Mechanical rules retained outside proof migration

The following are local encodings, not independent flow solvers: fixed-register
`INC`/`DEC` specialization; adjacent wide-`CNOP` cleanup; adjacent `SWAPN`,
call-window and `NULLN` superinstruction collection; in-place
`CONCAT`/`SCONCAT` lowering; and adjacent `INC`/branch or compare/branch
superinstruction selection after liveness is supplied.  They remain in the
keyhole engine unless migration removes their surrounding proof need or a
separate assembler-processing simplification is selected.

## Execution order and gates

### Phase A — simple write-once derivation repetition

- [x] **M01a:** replay the old `ITOF` decisions in diagnostic dual mode.
- [x] Extend the generic repetition service only if the current derivation
      query cannot express the exact source/result/effect proof.
- [x] Make the new service sole `ITOF` authority and delete the old case.
- [x] Generalize the rewrite consumer from a named `ITOF`/`ITOS` selector to
      metadata-admitted one-register `XTOY` derivations; the proof service,
      not a mnemonic list in the consumer, remains authoritative.
- [x] **M01b:** add complete integer-source/decimal-result and numeric/plugin
      dependency metadata for `ITOD`, then validate it as the first newly
      unlocked conversion.
- [x] Inventory the rest of the one-register conversion family and admit each
      opcode only after its source component, result component, derivation,
      context/plugin dependencies, failure-write phase and success-stability
      contract are exact.  Record each new opcode acceptance separately.
- [x] Replay focused opt/no-opt output and Richards/Towers/RexxCPS decisions.
- [x] Apply the mandatory Release stop if the stronger proof changes an
      ordinary representative output.
- [x] Repeat separately for **M02**.
- [x] Repeat separately for **M03**.
- [x] Repeat separately for **M04**.

#### M04 design selection

M04's machine-level runtime ceiling is deletion of an instruction whose VM
success path is already an address-equality no-op.  The assembler-side ceiling
is one bounded proof query for each demand-filtered candidate; there is no
runtime helper, allocation or added instruction.

1. **Retain the legacy raw-register identity plus forward TRACE scan.** This
   preserves the old floor but cannot follow LINK/SWAP storage, and treats a
   later trace event as if the no-op copy itself were observable.
2. **Selected: canonical same-storage copy metadata plus a write-once
   `StorageId` proof.** Identical physical operands prove the old floor even
   when entry storage is unknown; otherwise equal IDs or equal unique storage
   leaves prove linked and agreeing-phi aliases.  The proof service is sole
   authority and the old identity branch is deleted.
3. **Defer M04 into the M05 use/liveness rewrite service.** That service is
   needed for nonidentity typed-copy redirection, but is unnecessary for an
   instruction whose conditional VM path performs no write or signal.  It
   would also delay removal of the incorrect blanket TRACE guard.

The selected conditional metadata covers `COPY`, `ICOPY`, `FCOPY`, `SCOPY`,
`DCOPY`, `ACOPY` and `BCOPY`.  It is deliberately stronger than each opcode's
general signal contract: notably `BCOPY` remains conservative for different
storage while its same-storage VM path returns before allocation or signal.

### Phase B — reusable use graph and component liveness

- [ ] Add a sparse per-procedure use index keyed by storage/component
      `ValueId`; do not reconstruct a nodes-by-register matrix.
- [x] Expose edge-aware `value_live`, `all_uses_redirectable`,
      `destination_unobserved` and atomic rewrite-plan queries.
- [x] Include TRACE/source/register metadata, async handlers, signal
      continuations, call windows and hidden lifetime/reference effects.
- [x] Prove scaling on the inlined RexxCPS procedure before selecting a
      consumer.
- [x] Migrate **M05** and delete its old solver after its gate.
- [ ] Migrate **M06** separately and delete its old solver only after its gate.

#### M05 retained floor and design selection

The frozen M04 assembler accepts ten M05 typed-copy cases, all with reason
`all-uses-redirected`: `typed_copy_compare`, `typed_copy_join`, the first
independent copy in `complex_independent_region`, and the seven
`whole_procedure_panel` cases `typed_float_compare`, `typed_string_compare`,
`copy_loop_dominated`, `endlife_then_independent`,
`unrelated_register_metadata`, `copy_across_unobserved_throw` and
`indirect_control_flow_fail_closed`.  `typed_copy_other_view_live`,
`typed_copy_live_across_call`, `copy_loop_mixed_entry`, `copy_before_endlife`,
`relevant_register_metadata` and `copy_observed_by_throw_handler` retain the
adjacent negative floor.  M06 producer forwarding and the typed
`nr18_flow_harvest` cases are not M05 input.

Three implementation shapes were considered:

1. **Retain the dense per-candidate solver.**  This allocates four arrays over
   every record and repeatedly scans every operand for each candidate.  It is
   the scaling problem Phase B exists to remove and is rejected.
2. **Selected: one cached SSA use/dependency index per procedure epoch.**
   Explicit component and cursor reads, implicit reads, metadata/TRACE reads
   and observation boundaries are indexed once against write-once
   `ValueId`/`StorageId` facts.  Phi/copy dependency edges let a query visit
   only values which can contain the candidate result.  The proof service,
   rather than the index, decides equivalence and produces an immutable,
   all-or-nothing operand rewrite plan.
3. **Embed another graph walk in the M05 consumer.**  This could recover the
   old floor with less initial code but would duplicate traversal, signal and
   observation policy for M06, K04 and future liveness proofs.  It is rejected.

The service is deliberately split into an output-neutral infrastructure gate
and an M05 authority gate.  Its first public surface exposes indexed direct
uses and value dependencies, edge-aware value liveness, retained-memory/work
metrics and epoch validation.  Candidate-specific redirectability remains a
proof query.  M05 must recover the ten-case floor and records any stronger
acceptance separately; copied-view-dead deletion and producer forwarding
remain M06 work.  The first stronger case is `copy_before_endlife`: the sparse
proof distinguishes an unrelated exceptional `ENDLIFE` observation and safely
redirects the typed-copy consumer, where the old M05 blanket boundary rejected
the case and M06 later retargeted the producer.  Instruction count is unchanged
but proof ownership is more precise.  String-copy deletion additionally
requires destination cursor state to be unobserved, because `SCOPY` writes both
the string value and its cursor.

### Phase C — keyhole semantic authorities

- [ ] Migrate **K04** first because it has nine current canonical RexxCPS hits
      and exercises the reusable liveness service.
- [ ] Migrate **K02** and **K03** to storage/component identities.
- [ ] Migrate **K01** to permutation/observation equivalence.
- [ ] Reclassify or migrate **K06** after the generic component proof exists.
- [ ] Migrate **K05** when rewrite orchestration can edit immutable-CFG-derived
      plans without relying on a 20-item queue.

### Phase D — remove superseded infrastructure

- [ ] Express the M07 storage-identity oracle against sparse SSA and delete the
      dense legacy storage analysis.
- [ ] Delete M08 availability/may-reach/liveness code after its last consumer.
- [ ] Move M00 reachability cleanup to the immutable graph rewrite plan.
- [ ] Remove the legacy whole-procedure graph once no consumer or diagnostic
      owns it; keep only the small mechanical keyhole engine if still useful.
- [ ] Rebaseline assembler elapsed/RSS and complete proportional broad Debug
      closeout before the final local commit.

## M01-M04 results and current stop point

M01 is complete in
[`2026-08-02-perf3-11-m01-xtoy`](evidence/2026-08-02-perf3-11-m01-xtoy/).
The generic consumer recovers the old `ITOF` floor and proves 11 additional
focused deletions: `FTOS`, `DTOS`, `BTOD`, `BTOF`, `BTOS`, `FTOB`, `STOB` and
four `ITOD` shapes spanning adjacency, installed-handler policy, ordered TRACE
and linked storage.  Source/effect changes, seven signalling conversion
families, and same-component `BTOI`/`ITOB` normalizations remain rejected with
stable proof reasons.

The `ITOD` investigation established that both bundled `decimalFromInt`
implementations are total for every integer.  `ITOD` and the same-function
`BTOD` now clear stale backend diagnostics and have a coherent non-signalling
runtime and metadata contract; OOM remains a VM panic, not a language signal.
All 20 one-register conversion opcodes now describe their exact source,
result, derivation and context dependencies in canonical metadata.

Richards, Towers and RexxCPS remain byte-identical to the frozen Stage 6
assembler, so M01 did not trigger a new runtime verdict.  Focused optimizer
replay passes 51/51, broad Debug passes 1,989/1,989, and Release RexxCPS
diagnostic assembly completes in 0.38 s at 22,265,856 bytes peak RSS.

M02 is complete in
[`2026-08-03-perf3-11-m02-constant-write`](evidence/2026-08-03-perf3-11-m02-constant-write/).
The old raw-register repeated-load solver is deleted.  The proof service now
requires equal `StorageId`, equal integer value or exact float bits across all
reaching `ValueId` leaves, and already-absent reference/native payloads before
removing an exact total scalar constant write.  It recovers the one old floor
and proves four stronger deletions: equal-value phi, exact float, linked storage
and ordered TRACE.  Different phis, signed zero and hidden cleanup remain
closed.

M02 adds native payload as an eighth component because VM scalar assignment
can finalize host-owned binary state as well as release references.  All
migrated consumers share one proof session per fixed-point epoch, and lazy SSA
memory accounting now includes query-materialized state.  Ordinary RexxCPS
assembly retains a 0.05 s median; median RSS rises from 22,134,784 to 30,081,024
bytes and remains procedure-local inside the accepted seconds-scale budget.
Adrian accepted that first Release verdict on 2026-08-03.  Canonical images are
unchanged, focused optimizer replay is 53/53, Release hidden-cleanup execution
is 4/4, and broad Debug passes 1,991/1,991.

M03 is complete in
[`2026-08-03-perf3-11-m03-absent-write`](evidence/2026-08-03-perf3-11-m03-absent-write/).
The old repeated-`NULL` solver is deleted.  Its replacement requires known
pre-instruction storage and proves every reaching leaf of all eight components
already absent.  It recovers the old floor and additionally removes equal-phi,
linked-storage and ordered-TRACE repetitions.  Different phis, scalar writes,
reference cleanup and native-payload cleanup remain closed.  Adrian accepted
the output-neutral Release verdict on 2026-08-03; canonical images are
byte-identical and broad Debug passes 1,993/1,993.

M04 is complete in
[2026-08-03-perf3-11-m04-self-copy](evidence/2026-08-03-perf3-11-m04-self-copy/).
Canonical same-storage no-op metadata covers all seven full/typed copy
families. The proof service preserves the old raw COPY/ICOPY/FCOPY/SCOPY floor
and adds seven focused deletions: raw DCOPY/ACOPY/BCOPY, linked full/binary,
TRACE-anchored full copy and agreeing-phi decimal copy. Divergent-phi and
different-storage copies remain rejected. The old identity branch and its
blanket forward TRACE veto are deleted.

Adrian accepted the output-neutral first Release verdict on 2026-08-03.
Richards, Towers and RexxCPS are byte-identical to frozen M03. The final broad
Debug run passes 1,995/1,995.

M05 is complete in
[`2026-08-03-perf3-11-m05-sparse-use-liveness`](evidence/2026-08-03-perf3-11-m05-sparse-use-liveness/).
The cached use service indexes explicit, read/write, metadata, TRACE, cursor,
call-window and opaque observations against `ValueId`/`StorageId`, plus reverse
phi dependencies and sparse liveness. The proof service produces one immutable
all-or-nothing operand plan for exact `ICOPY`/`FCOPY`/strict-`SCOPY`; the old
per-candidate availability/may-reach arrays and repeated operand scans are
deleted.

All ten old safe M05 decisions are recovered. The separately evidenced
`copy_before_endlife` case is stronger: M05 now proves the unrelated exceptional
ENDLIFE does not observe the candidate and redirects the compare to the source;
the old product instead rejected in M05 and reached the same instruction count
through M06 producer forwarding. Metadata, TRACE, mixed-entry, other-component,
cursor, caller-window and handler-observed controls remain closed.

Adrian accepted the first ordinary Release verdict on 2026-08-03. Richards,
Towers and RexxCPS remain byte-identical to M04. Three paired scale-screen
rounds put ordinary RexxCPS assembly at 0.16-0.17 s and 102.8 MB peak RSS versus
0.05-0.06 s and 29.9 MB for frozen M04; this material but bounded cost is inside
the agreed seconds-scale budget. Focused Debug and Release pass 6/6, strict
GNU90 passes with one pre-existing warning, and broad Debug passes 1,995/1,995
in 231.94 seconds. M06 producer forwarding is next.
