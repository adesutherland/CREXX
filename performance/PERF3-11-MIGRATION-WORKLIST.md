# PERF3-11 remaining RXAS proof migration

Status: **in progress — baseline locked; M01 `XTOY` repetition migration first**

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
| M01 | `flow_compute_available_fact()` for repeated one-register `ITOF`, then metadata-admitted `XTOY` derivations | old floor: focused `ITOF` 1, canonical 0; other conversions are new domain | generic dominated-success repetition | **first**; recover the exact old `ITOF` case, then onboard `ITOD` and the wider family by metadata and separate gates |
| M02 | repeated identical integer/bitwise-float load availability | focused 1; canonical 0 | component value/equivalent constant proof | second |
| M03 | repeated `NULL` all-view availability | focused 1; canonical 0 | storage presence plus all-component equivalence | third |
| M04 | exact full/typed self-copy deletion with address-observation scan | focused full-copy 1; canonical 0 | storage/value identity plus observation equivalence | migrate with redundant-write queries |
| M05 | typed `ICOPY`/`FCOPY`/strict-`SCOPY` availability, may-reach and operand redirection | focused 10; canonical 0 | SSA use graph, edge-aware liveness and atomic rewrite plan | requires new reusable use/liveness query |
| M06 | adjacent producer-destination forwarding | focused 12; canonical 0 | exact producer result, destination/temporary liveness and observation equivalence | migrate after M05 infrastructure |
| M07 | dense legacy register-storage must-analysis | diagnostic-only; 13 storage-identity fixtures | sparse SSA storage queries and proof diagnostics | retire after its oracle is expressed against new SSA |
| M08 | raw-register liveness/availability/may-reach substrate | shared by M02-M06 | graph use index plus component/storage liveness | retire after last consumer |

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

- [ ] **M01a:** replay the old `ITOF` decisions in diagnostic dual mode.
- [ ] Extend the generic repetition service only if the current derivation
      query cannot express the exact source/result/effect proof.
- [ ] Make the new service sole `ITOF` authority and delete the old case.
- [ ] Generalize the rewrite consumer from a named `ITOF`/`ITOS` selector to
      metadata-admitted one-register `XTOY` derivations; the proof service,
      not a mnemonic list in the consumer, remains authoritative.
- [ ] **M01b:** add complete integer-source/decimal-result and numeric/plugin
      dependency metadata for `ITOD`, then validate it as the first newly
      unlocked conversion.
- [ ] Inventory the rest of the one-register conversion family and admit each
      opcode only after its source component, result component, derivation,
      context/plugin dependencies, failure-write phase and success-stability
      contract are exact.  Record each new opcode acceptance separately.
- [ ] Replay focused opt/no-opt output and Richards/Towers/RexxCPS decisions.
- [ ] Apply the mandatory Release stop if the stronger proof changes an
      ordinary representative output.
- [ ] Repeat separately for **M02**, **M03** and **M04**.

### Phase B — reusable use graph and component liveness

- [ ] Add a sparse per-procedure use index keyed by storage/component
      `ValueId`; do not reconstruct a nodes-by-register matrix.
- [ ] Expose edge-aware `value_live`, `all_uses_redirectable`,
      `destination_unobserved` and atomic rewrite-plan queries.
- [ ] Include TRACE/source/register metadata, async handlers, signal
      continuations, call windows and hidden lifetime/reference effects.
- [ ] Prove scaling on the inlined RexxCPS procedure before selecting a
      consumer.
- [ ] Migrate **M05** and **M06** separately, deleting their old solvers only
      after each gate.

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

## Current stop point

The baseline and diagnostic identity are complete.  Migration begins with M01:
the accepted repetition service should already recover the old `ITOF` floor,
then the same metadata-driven consumer expands first to `ITOD` and later to
other proved `XTOY` derivations.  This is the smallest direct test of the
basic-to-advanced policy before new liveness infrastructure is added.
