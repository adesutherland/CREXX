# PERF3-11 remaining RXAS proof migration

Status: **in progress — M01-M06 and K01-K04/K06 complete; K05 is next**

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
5. **Signals and observations are first-class.** Normal, skip, handler
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
| M06 | adjacent producer-destination forwarding | focused 11 at the M05 boundary; historical 12 before `copy_before_endlife` moved to M05; canonical 0 | exact producer result, destination/temporary liveness and observation equivalence | **complete**; SSA/use proof is sole authority, old dense liveness solver deleted, hidden cleanup proved |
| M07 | dense legacy register-storage must-analysis | diagnostic-only; 13 storage-identity fixtures | sparse SSA storage queries and proof diagnostics | retire after its oracle is expressed against new SSA |
| M08 | raw-register liveness/availability/may-reach substrate | shared by M03-M06 | graph use index plus component/storage liveness | **complete**; dense semantic authority deleted after M06; mechanical use/kill classification remains for diagnostics and legacy keyholes |

The representative canonical set currently exercises no remaining M01-M06
acceptance.  That does not make the migrations irrelevant: these solvers are
the bounded basic cases that the new proof is expected to generalize, and the
focused fixtures preserve their exact safety floor.

## Inventory: keyhole proof migration and classification gates

| ID | Rule family | Accepted baseline | Current proof weakness | New owner |
| --- | --- | --- | --- | --- |
| K01 | two `SWAP`s cancel with no relevant intervening use | focused positive plus implicit-register/barrier negatives; canonical 0 | raw-register relevance and bounded queue | **complete**; sparse storage-permutation/observation proof is sole authority and ordered overlapping `SWAPN` transfer is exact |
| K02 | duplicate `LINK`/typed-copy/`UNLINK` read | focused string/binary positives and metadata/TRACE negatives; canonical 0 | syntax shape and raw-register hazard scan | **complete**; storage/component proof and atomic rewrite replace all six direct-read keyholes |
| K03 | duplicate `LINKATTR1` read for same owner/slot | focused string/binary positives and different-slot negative; canonical 0 | syntax shape; no generic attribute identity | **complete**; interned attribute path plus component/count/signal proof replace all six attribute-read keyholes |
| K04 | integer compare plus conditional branch | legacy focused 16 includes one result-TRACE deletion; canonical RexxCPS 9 require storage/lifetime reaudit | bespoke bounded path solver for boolean-result death | **complete and accepted**; atomic event deletion, exact call windows and approved retry retirement unlock all five residual canonical false positives; focused Debug/Release passes 14/14, the runtime verdict is neutral, and broad Debug passes 1,998/1,998 |
| K05 | branch-to-conditional/dual-branch threading | canonical Richards 4, Towers 7, RexxCPS 0 | queue-local label search | immutable CFG edge rewrite and reachability |
| K06 | full `COPY` followed by same-pair `ACOPY` | focused 1; canonical 0 | adjacent mnemonic rule | **complete**; metadata and VM handlers prove exact local component subsumption, so the declarative rule is retained as mechanical |

K04 and K05 remain exercised by the representative canonical inputs. The nine
historical RexxCPS K04 fusions all discarded the matching trace event for the
eliminated Boolean. That is permitted in an optimised image under the canonical
TRACE contract, but it does not by itself prove the storage cleanup and
lifetime obligations; each case remains subject to the new proof. K05 retains
eleven branch-threading rewrites across Richards/Towers.

## Mechanical rules retained outside proof migration

The following are local encodings, not independent flow solvers: fixed-register
`INC`/`DEC` specialization; adjacent wide-`CNOP` cleanup; adjacent `SWAPN`,
call-window and `NULLN` superinstruction collection; in-place
`CONCAT`/`SCONCAT` lowering; full `COPY` plus same-pair `ACOPY` component
subsumption; and adjacent `INC`/branch or compare/branch superinstruction
selection after liveness is supplied.  They remain in the
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

- [x] Add a sparse per-procedure use index keyed by storage/component
      `ValueId`; do not reconstruct a nodes-by-register matrix.
- [x] Expose edge-aware `value_live`, `all_uses_redirectable`,
      `destination_unobserved` and atomic rewrite-plan queries.
- [x] Include TRACE/source/register metadata, async handlers, signal
      continuations, call windows and hidden lifetime/reference effects.
- [x] Prove scaling on the inlined RexxCPS procedure before selecting a
      consumer.
- [x] Migrate **M05** and delete its old solver after its gate.
- [x] Migrate **M06** separately and delete its old solver only after its gate.

#### K04 retained floor and design selection

The frozen M06 assembler accepted 16 focused integer compare/branch shapes and
nine RexxCPS shapes. One focused case and all nine RexxCPS cases placed an
ordered TRACE event on the materialized Boolean, then the old keyhole rule
deleted both the value and its event. The initial K04 audit incorrectly treated
event-count preservation as a universal correctness obligation. The canonical
TRACE contract instead permits an optimised transform to remove the matching
event when the Boolean no longer exists; compiler and assembler no-opt mode
retain the source-correspondent operation and event. This correction does not
waive ordinary storage, cleanup, alias, liveness or control-flow proofs, so the
nine canonical cases must be re-evaluated rather than presumed safe. A new
unrelated-TRACE case remains a useful proof that fusion does not delete an
event belonging to another value.

Three implementation shapes were considered:

1. **Keep keyhole selection and call a whole-procedure proof.** The keyhole
   runs before the immutable procedure exists, so this would retain a bounded
   second authority or require an additional graph. It is rejected.
2. **Selected foundation: canonical equivalence metadata plus one immutable
   proof plan.**
   A linear whole-procedure demand filter identifies consecutive executable
   compare/branch pairs. The proof service owns opcode/signal equivalence, CFG
   block adjacency, exact source/result components, local unaliased storage,
   source/destination separation, hidden reference/native cleanup, sparse
   direct/dependent uses, caller windows and TRACE observation. K04a identifies
   any number of exact result events by `ValueId`, storage, register, component
   and record position, then includes their deletion in the same revalidated
   plan as compare deletion and branch replacement. An unrelated event remains
   present and can share the fused branch boundary without a `CNOP`.
3. **Materialize the Boolean solely for optimised TRACE.** A new result-
   producing fused opcode could do this, but it defeats the selected
   optimisation and is unnecessary for the project trace contract. It remains
   unselected; users requiring source-correspondent tracing use no-opt mode.

The approved signal correction marks the eight register/register and
register/immediate `BGT`/`BGE`/`BLT`/`BLE` forms total and non-signalling, in
line with their direct scalar VM handlers. `BEQ`/`BNE` and all source integer
comparisons were already non-signalling. The older `IGT`/`BRT` keyhole rule is
subsumed by canonical `BGT`; the shared `FGT`/`BRT` rule is retired pending an
explicit `FGTBR` signal contract and a float-component proof. Adversarial
focused cases must retain compare materialization for caller-window exposure,
later live uses, source/destination overlap, linked storage and previously
populated reference cleanup. A precisely matching result event may be removed
only with the compare; unrelated events remain. The recursive 20-record path
solver and all compare/branch keyhole rules are deleted.

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

#### M06 retained floor and design selection

The frozen M05 assembler is
`fc6def299f67df91d0b6828f78ebfd3ae9bc60e99c719acb38531256da92ea55`
(`crexx-1.0.0-beta.3+local.gbe20340f99af`). It accepts exactly eleven current
M06 cases with reason `producer-destination-forwarded`: eight in
`nr18_flow_harvest`, two in `whole_procedure_flow` and one in
`whole_procedure_panel`. Richards, Towers and RexxCPS have zero M06 accepts.
The historical twelfth case, `copy_before_endlife`, is not lost: M05 now
removes that typed copy through its stronger sparse use proof before M06 runs.

M06's machine-level ceiling is the producer writing the final local directly
and the immediately following typed copy disappearing. It adds no runtime
helper or instruction. Three implementation shapes were considered:

1. **Retain the legacy raw-register liveness solver.** This preserves the old
   shapes but allocates dense per-record register/view state, does not follow
   `StorageId`/`ValueId`, and leaves M08 alive solely for M06. It is rejected.
2. **Selected: an atomic producer-forward plan from the existing SSA/use proof
   service.** The plan names the adjacent producer and copy, their exact
   component result, local unaliased temporary/final storage, the sole
   temporary-result use, exceptional/metadata/call observations, and the
   expected producer operand rewrite. The optimizer validates the immutable
   plan before retargeting the producer and deleting the copy.
3. **Fold producer forwarding into M05 operand redirection.** M05 redirects
   uses of an existing copied `ValueId`; M06 moves the defining operation to a
   different storage and removes the temporary definition. Combining them
   would obscure different destination, cleanup, signal and address
   obligations, so the authorities remain separate.

The new proof deliberately strengthens correctness beyond the old typed-view
liveness test. VM scalar producers such as `LOAD` and integer comparisons use
`set_int`/`set_float`, which release reference and native payloads, while
`ICOPY`/`FCOPY` write only the scalar field. Retargeting is therefore allowed
only when every producer-cleared payload is already absent in both the
discarded temporary and final destination. A fresh local entry is an empty VM
value, but argument/global, aliased, previously populated or unknown storage
fails closed. The temporary result must have only the exact copy as a sparse
direct/dependent use; call windows, TRACE/register metadata, opaque uses and
registered asynchronous handlers remain observations. The producer must be a
classified, non-signalling, context-neutral single exact kill which does not
read the final storage through any register mapping. Source/TRACE records
immediately after the deleted copy retain the old address-observation guard.

### Phase C — keyhole semantic authorities

- [x] **K04a:** atomically delete only exact matching optimized-away Boolean
      events. Focused Debug/Release pass 5/5; canonical output is semantically
      identical to strict K04, so runtime timing is not yet warranted.
- [x] **K04b:** replace the procedure-global numeric call-window veto with an
      edge-aware query which rejects only when the compare-result `ValueId` or
      one of its dependent phis is actually visible in that call window.
      Accepted as neutral migration/consolidation after focused 5/5 Debug and
      Release and byte-identical canonical output.
- [x] **K04c:** attribute the five residual canonical rejections. Both calls
      have exact source window `r1`, while compare results are `r2`/`r6`; the
      current unknown CALL signal contract alone widens retry state to
      `r1..r69`. VM retry semantics confirm the count operand is preserved
      except through already-modelled argument-storage aliasing.
- [x] **K04d1:** retire the unused, non-standard instruction-level
      retry factory, VM continuation, CFG self-edge and retry-only loop
      machinery. Keep the coherent propagated-call partial-state contract for
      skip/handler/unwind paths and preserve exact argument-window, alias and
      destination effects.
- [x] **K04d2:** pass the same 14 focused language, bytecode, CALL, alias,
      optimizer and signal-lifecycle checks in Debug and ordinary Release.
- [x] **K04d3 first verdict:** retain one warmup and 12 balanced/interleaved
      RexxCPS rounds per VM. Median CPS is neutral at +0.021% on both VMs;
      pair directions are mixed and the `rxvm` candidate contains one retained
      low sample which triggers the formal rerun recommendation.
- [x] **K04d disposition:** Adrian accepted the neutral semantic/infrastructure
      cleanup without a noisy-cell append on 2026-08-03.
- [x] **K04d4 closeout:** complete Debug build and 1,998/1,998 broad Debug tests
      pass; the retirement audit finds no obsolete production retry machinery.
- [x] **K02/K03-0 retained baseline:** preserve the focused Debug/Release
      9/9 floor before migration. The frozen legacy decision signatures are
      K02 `scopy=a00a9575`, K02 `bcopy=998fd3d5`, K03
      `scopy=8ab441ee`, and K03 `bcopy=3bd65c72`; the representative Release
      Richards/Towers/RexxCPS set has zero K02/K03 accepts.
- [x] **K02/K03-1 metadata and path SSA:** distinguish attribute count from
      value status flags, correct the proven-total `SETATTRS` and `BCOPY`
      signal contracts, and intern one-based attribute paths by owner
      `StorageId`, count `ValueId`, reference-effect generation and slot.
      Dynamic slots and unknown/merged counts fail closed.
- [x] **K02/K03-2 proof and atomic rewrite:** replace the twelve duplicate
      `LINK`/`LINKATTR1` keyholes with one immutable plan which validates both
      exact link/copy/unlink triples, proves component equivalence for the
      complete typed-copy read mask, proves any removed `LINKATTR1` in range,
      rewrites the second link record to the copy and deletes its original
      copy/unlink only after the whole plan is revalidated.
- [x] **K02/K03-3 safety panel:** retain different owner/slot, changed count,
      changed source/detached value, calls/aliases, branch/phi and signal-skip
      negatives; record metadata/TRACE reads that remain semantically valid as
      stronger SSA acceptances rather than weakening the old floor.
- [x] **K02/K03-4 verdict:** compare focused signatures and RXBIN hashes,
      rerun the canonical diagnostic census, and apply the mandatory first
      Release verdict only if an ordinary representative output changes.
- [x] **K02/K03-5 closeout:** distinguish value observations from sparse pure
      writes for every migrated proof consumer, restore the M05/M06 and K04
      floors, pass the 28-test shared panel in Debug and Release, and pass
      2,010/2,010 broad Debug tests in 678.89 seconds.
- [x] **K01-0 retained baseline:** freeze the accepted baseline at `45e027685`,
      preserve both legacy decision signatures and replay the six focused
      implicit-use/barrier cases in Debug and Release.
- [x] **K01-1 sparse permutation proof:** prove an exact two-`SWAP` round trip
      from restored `StorageId` bindings plus an observation/write-free
      source-linear normal/skip spine; locally installed and asynchronous
      handler observations remain closed.
- [x] **K01-2 authority migration:** make the proof plan the sole deletion
      authority and delete the two raw-register `NO_HAZARD` rules.
- [x] **K01-3 stronger safety panel:** add reversed operands, an interval beyond
      the old queue bound, explicit writes, register metadata/TRACE, opaque
      effects, call windows, mapping changes and signal/control negatives.
- [x] **K01-4 verdict:** the six inherited cases and both orientations are
      byte-identical to committed baseline `45e027685`; canonical Richards,
      Towers and RexxCPS have zero K01 accepts and byte-identical images, so
      runtime timing is not warranted.
- [x] **K01-5 closeout:** retain the evidence bundle and pass the complete
      2,012/2,012 broad Debug gate in 368.43 seconds.
- [x] **K06-0 retained baseline:** freeze committed K01 closeout `78bd7f6f5`,
      replay signature `d132e98f`, preserve the focused optimized/no-opt hashes
      and confirm zero K06 accepts in Richards, Towers and RexxCPS.
- [x] **K06-1 classification:** prove from opcode metadata and VM handlers that
      full `COPY` includes the complete status component written by `ACOPY`,
      both instructions are total, and the existing rule requires an identical
      source/destination pair with no intervening executable instruction.
- [x] **K06-2 contract floor:** lock the component-subset/non-signalling
      invariant in metadata tests and add different-target, different-source
      and executable-gap negatives to the focused optimizer fixture.
- [x] **K06-3 verdict:** compare focused and canonical ordinary Release output
      against `78bd7f6f5`; time only if an ordinary representative image changes.
- [x] **K06-4 closeout:** retain the classification evidence, document K06 as a
      mechanical adjacent encoding and remove it from the semantic-proof queue.
- [ ] Migrate **K05** when rewrite orchestration can edit immutable-CFG-derived
      plans without relying on a 20-item queue.

#### K01 retained floor and design selection

The accepted local baseline is commit `45e027685` with Debug/Release `rxas`
SHA-256 `9be497a4` and `11ea6e9b`. The inherited focused floor passes 6/6 in
both builds: relevant implicit `INC0`, numeric-register `LOAD` and `LINKARG`
uses preserve both swaps, while the corresponding unrelated-register cases
remove them. The same-order and reversed-order legacy decision signatures are
`f9d23da9` and `c82e3aa9`. Richards, Towers and RexxCPS have zero K01 accepts.

The machine-level ceiling is exact deletion of both mapping-only `SWAP`
instructions with no replacement instruction or runtime helper. Three
implementation shapes were considered:

1. **Retain the two `NO_HAZARD` keyholes and add a proof callback.** This keeps
   the 20-record streaming queue, raw-register relevance and syntax matching as
   a second semantic authority. It is rejected.
2. **Selected: pair-key demand filter plus an immutable sparse permutation
   plan.** The proof names both exact swaps and their physical registers,
   verifies dominance and pre-first/post-second `StorageId` agreement, and
   scans the shared use index for every affected-register read, write, cursor
   access, register metadata, TRACE event, call-window or opaque
   observation/effect. Only a fully revalidated plan deletes both records.
   The consumer has no queue-distance limit, so it safely recovers equivalent
   cases beyond the old window.
3. **Promote the dense M07 swap-roundtrip diagnostic or immediately delete
   arbitrary `SWAPN` permutations.** The diagnostic allocates a complete
   record-by-register matrix and is itself queued for retirement. General
   multi-permutation rewriting also overlaps NR-09 swap collection. Both are
   deferred; K01 establishes a reusable sparse proof surface with the exact
   two-`SWAP` migration as its first consumer.

Signal modelling remains explicit. A known static signal may split a
source-linear normal/skip spine when policy is still inherited from the caller:
resumption reaches the restoring swap, while other actions discard the current
frame mapping. Locally installed/merged handlers, asynchronous-handler uses,
unknown signals and ordinary control splits reject. Register-backed
observations and all writes through either permuted physical register reject
even when the component is otherwise dead. Access through a different
register already aliasing the same storage remains valid because that alias
names the same storage with or without the temporary physical permutation.

NR-09 may first consolidate adjacent or metadata-separated source swaps into
one ordered four-operand `SWAPN`. The same proof deletes an exact repeated
pair by permutation composition. K01 also corrected sparse SSA to compose all
overlapping `SWAPN` pairs in operand order; the direct contract covers both a
three-register overlap and the self-cancelling form. General larger
permutation rewriting remains deferred rather than being inferred from K01.

#### K06 retained floor and design selection

The committed K06 baseline is K01 closeout `78bd7f6f5`. Its focused optimized
and no-opt RXBIN SHA-256 values are `ef28be93` and `5ef002b9`; the optimized
decision retains legacy signature `d132e98f`. Richards, Towers and RexxCPS have
zero K06 accepts and retain hashes `92cda4c0`, `11b14061` and `96bc599b`.

The machine-level ceiling is deletion of the second adjacent `ACOPY`, with no
replacement instruction or runtime helper. The metadata and VM handlers make
the equivalence local and exact: full `COPY` reads and writes
`RXOP_COMPONENT_ALL`, including `RXOP_COMPONENT_ATTRIBUTES`, while `ACOPY`
only reads and writes that status component. `copy_value()` assigns
`status.all_type_flags`; the `ACOPY` handler immediately repeats the same
assignment. Both opcodes are non-signalling, and the rule-table captures
require the same physical source and destination on both instructions with no
intervening executable instruction.

Three dispositions were considered:

1. **Selected: retain the declarative adjacent rule as mechanical.** Add a
   metadata contract assertion and exact negative fixture rows, but keep the
   production rule unchanged. No CFG, SSA, storage identity, liveness or
   signal-continuation query can strengthen the proof for this exact adjacency.
2. **Move the pair into the immutable proof service.** This would build and
   query procedure analysis to rediscover an operand-equality and component-
   subset fact already fixed by instruction metadata. It is rejected as an
   unnecessary semantic authority and assembler-cost regression risk.
3. **Generalize to every adjacent narrower copy subsumed by full `COPY`.** This
   may be valid, but it expands beyond the retained K06 floor into typed payload
   and native/reference lifetime cases. It remains a separately evidenced
   future mechanical capability rather than being inferred from `ACOPY`.

Because the selected disposition changes no production rule or ordinary
output, K06 does not trigger a runtime-performance verdict. Any future opcode
metadata or handler change that breaks the locked subset/non-signalling
contract must fail the metadata test and reopen the classification.

K06 closeout is retained in
[2026-08-03-perf3-11-k06-mechanical-classification](evidence/2026-08-03-perf3-11-k06-mechanical-classification/).
The metadata/optimizer panel passes 3/3 in Debug and Release. Replaying the
original focused source with its exact relative path is byte-identical in
optimized and no-opt modes, and the expanded fixture preserves the sole
`d132e98f` acceptance while locking three negative shapes. Release `rxas` is
unchanged at SHA-256 `a1c0e1ef`; Richards, Towers and RexxCPS retain exact
hashes and zero K06 accepts. No production code or ordinary output changed, so
runtime timing and broad CTest are not warranted.

#### K02/K03 retained floor and design selection

The frozen K04 assembler has twelve syntax-expanded rules: six copy families
for a repeated direct `LINK` read and the same six for an immediate one-based
`LINKATTR1` read. They cover `COPY`, `BCOPY`, `ICOPY`, `SCOPY`, `FCOPY` and
`DCOPY`; `ACOPY` has no duplicate-read rule. The old engine accepts or rejects
by captured raw register numbers plus a bounded hazard scan. Its focused
Debug/Release floor is 9/9 and the frozen RXBIN hashes are:

- K02 string `bfa4420`, binary `a8c522f`, metadata relevant `a5b459d`,
  metadata unrelated `0f6e61b`, TRACE relevant `2fb62e9`, and TRACE unrelated
  `37f8e3e`;
- K03 different-slot `f44907a`, string `97d3092`, and binary `a482b7c`.

Three implementation shapes were considered:

1. **Retain the keyholes and add proof callbacks.** This leaves syntax shape
   and queue traversal as a second semantic authority and cannot give future
   consumers a reusable attribute address identity. It is rejected.
2. **Selected: memory-path SSA plus one duplicate-read proof plan.** A direct
   link names its source `StorageId`. An immediate `LINKATTR1` names an
   interned child path keyed by owner storage, exact attribute-count value,
   reference-effect generation and slot. The proof compares write-once
   component values, not register numbers, and separately proves that removing
   a candidate attribute validation cannot suppress a signal. The linear
   consumer is only a demand filter; all mutation is atomic and epoch checked.
3. **Model only the current `SETATTRS`/`LINKATTR1` syntax pair.** This recovers
   the focused fixture but creates another tactical special case and cannot
   survive owner aliases, intervening writes or future attribute-path users.
   It is rejected.

The VM handlers for `SETATTRS` and two-register `BCOPY` do not raise language
signals; allocation failure aborts rather than entering the signal mechanism.
Their fail-closed unknown sidecars therefore obscure real normal-flow facts and
are corrected to non-signalling metadata without changing VM behavior. K03
does not infer safety from ordinary dominance of an earlier `LINKATTR1`: the
candidate link itself must have an exact positive slot and an exact known
attribute count at least that large. Any count/reference generation change or
merged unknown path rejects the rewrite.

#### K02/K03 result

Evidence is retained in
[2026-08-03-perf3-11-k02-k03-linked-reads](evidence/2026-08-03-perf3-11-k02-k03-linked-reads/).

The storage/component proof service is now the sole authority for all twelve
former direct/attribute duplicate-read rules. The original string/binary and
different-slot outputs remain byte-identical to frozen K04. The formerly
guarded relevant metadata and TRACE cases are stronger safe acceptances:
their retained events still read the unchanged first detached value while the
second executable read is removed. All six copy families are covered for both
direct and attribute paths. Owner aliases resolve by `StorageId`; different
owners or slots, changed count/source/detached value/cursor, aliased calls,
divergent phis and unproved signal paths reject.

The first canonical comparison exposed an integration regression rather than
a K02/K03 acceptance: newly indexed writes were represented as opaque
observations, disabling all fourteen accepted K04 RexxCPS compare/branch
fusions. The first broad gate then exposed the same conceptual boundary in
M05/M06: explicit pure writes with no `ValueId` were mistaken for unknown
reads, disabling four retained optimizer expectations. The use service now
distinguishes observations, read/write uses and pure explicit/opaque/cursor
writes once for every proof consumer. Writes remain sparse barriers for
component-interval proofs without pretending to observe the overwritten
value. The combined 28-test proof panel passes in Debug and Release, and broad
Debug passes 2,010/2,010 in 678.89 seconds. The final Richards, Towers and
RexxCPS images are byte-identical to frozen K04 at SHA-256 `92cda4c0`,
`11b14061` and `96bc599b`. The canonical K02/K03 census is zero, so runtime
timing is not warranted.

### Phase D0 — scalable routing, consolidation and consumer migration

Adrian approved D0.1 through D0.5 on 2026-08-03, with each infrastructure
stage required to migrate its affected production consumers, pass a
proportional validation gate and land as a separate local commit before the
next stage starts. The trigger is an unsupported scale boundary on generated
`Parse.rxas`: committed K06 requires approximately 1.87 GB peak RSS with
optimisation enabled versus approximately 26 MB with `-n`; the provisional
batched K05 path reaches approximately 2.47 GB. These are assembler-process
measurements, not benchmark runtime evidence.

The routing rule is now executable rather than an informal convention:

| Route | Current owners | Permitted facts |
| --- | --- | --- |
| local mechanical scan | fixed `INC`/`DEC`, `CNOP`, adjacent `SWAPN`/call/`NULLN` collection, in-place concat, K06 and adjacent loop branch packing | exact opcode, operand and bounded local metadata only |
| immutable CFG | M00 and K05 | records, blocks, typed edges, reachability and exact branch algebra |
| component SSA | M01-M04 | dominance, signal/effect, storage and component value identity |
| component SSA plus sparse use index | M05, M06 and K01-K04 | the preceding facts plus exact uses, observations and liveness |
| diagnostic-only | M07 until D0.2 | requested only by explicit diagnostics; never an ordinary-production reason to build analysis |

- [x] **D0.1 routing ledger and capability census:** give every current
      optimisation family a stable owner and explicit `LOCAL`, `CFG`,
      `DOMINANCE`, `SIGNAL`, `STORAGE`, `VALUE`, `USE` or `LOOPS` requirement;
      run one conservative opcode/metadata census before whole-procedure
      optimisation; gate each legacy consumer on its declared candidate
      family; and keep loop analysis dormant because no current production
      consumer requests it.
- [ ] **D0.2 one graph and structural consumers:** migrate M00 and K05 onto
      the immutable graph, express the M07 oracle through sparse SSA and delete
      the legacy graph/dense use-kill storage.
- [ ] **D0.3 capability-lazy semantic consumers:** migrate M01-M06 and K01-K04
      so each query requests only its declared structural, signal, storage,
      value and use capabilities.
- [ ] **D0.4 batched pass manager:** collect compatible typed rewrite plans
      from one immutable epoch, apply one validated batch and rebuild only
      after the batch.
- [ ] **D0.5 full scale verdict:** run the complete proportional correctness,
      elapsed and peak-RSS review. The `Parse.rxas` design target is at most
      256 MB and the hard rejection boundary is 512 MB; retain exact canonical
      image and accepted K05 VM-instruction evidence.

### Phase D — remove superseded infrastructure

- [ ] Express the M07 storage-identity oracle against sparse SSA and delete the
      dense legacy storage analysis.
- [x] Delete M08 availability/may-reach/liveness authority after its last
      consumer. Retain the raw use/kill and taint bitsets still consumed by
      storage diagnostics, bounded-analysis accounting and legacy keyholes.
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
in 231.94 seconds.

M06 is complete in
[`2026-08-03-perf3-11-m06-producer-forwarding`](evidence/2026-08-03-perf3-11-m06-producer-forwarding/).
The proof service now owns adjacent `ICOPY`/`FCOPY` producer forwarding through
one immutable `StorageId`/`ValueId` plan. It recovers all eleven current M05
boundary accepts with reason `producer-destination-forwarded-ssa`; the
historical twelfth case remains owned by M05. The old dense register/view
liveness solver is deleted.

Two new adversarial cases prove the correctness gap found during migration:
scalar producers clear reference/native payloads that typed copies preserve,
so forwarding requires those cleanup components already absent in both the
temporary and final storage. Temporary liveness, producer signalling or view
mismatch, destination reads, source-address observation, calls, metadata,
opaque use and asynchronous-handler observations also remain closed.

Adrian accepted the first Release verdict on 2026-08-03. Frozen M05 and M06
produce byte-identical focused and canonical Richards, Towers and RexxCPS
images. Three serial paired RexxCPS assembly rounds have equal 0.18 s medians;
median peak RSS moves from 103,022,592 to 104,103,936 bytes (+1.05%). Focused
Debug and Release pass 8/8. Strict GNU90 passes with one pre-existing warning;
the full Debug build and corrected broad suite pass 1,995/1,995 in 291.22
seconds. Broad closeout corrected one stale NR-09 oracle to expect M06's direct
destination load; no production change was required. K04a then completed exact
atomic result-TRACE deletion and focused Debug/Release passed 5/5. Canonical
RexxCPS remains semantically identical to strict K04: its five newly admitted
TRACE observations all stop at the next procedure-global call-window guard.
K04b then replaced the procedure-global numeric call-window veto with exact or
conservative bounds plus dependent-`ValueId` visibility. Focused Debug and
Release pass 5/5 and canonical output remains byte-identical, so Adrian
accepted the output-neutral consolidation without runtime timing. K04c proved
all five residual canonical rejections are CALL signal-contract false
positives: both actual windows are only `r1`, but retry-edge count phis widen
them to `r1..r69`. Strict GNU90 passes with one pre-existing warning, the full
Debug build passes and broad Debug passes 1,995/1,995 in 376.48 seconds. K04d
review then found no production retry caller and exposed an existing fused-call
retry mapping defect. Adrian approved K04d1 retirement on 2026-08-03; the
propagated-call partial-state metadata remains for non-retry failure paths.
K04d2 passes focused Debug/Release 14/14. K04d3's first Release verdict is
runtime-neutral at +0.021% median CPS on both VMs, with mixed pair directions;
one retained low `rxvm` candidate sample triggers the formal rerun
recommendation. Adrian disposition is required before migration continues.
