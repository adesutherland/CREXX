# PERF3-12B compound-tail representation and reuse

Status: in progress — B1 exact baseline and contract audit

Started: 2026-08-04

Purpose: compare two general ways to remove repeated materialization of native
stem compound tails: existing two-segment stem operations and loop-scoped reuse
of an equivalent joined key. Select neither route until isolated, replayable
Release PoCs establish correctness, actual dynamic work removed, assembler
scale and end-to-end effect.

## Authority and stop boundaries

- Branch: `codex/perf3-12b-compound-tail`.
- Published starting commit: `965b461d813f6042063ee786d8d00cea870da096`
  on `develop`. The accepted X1 implementation is `4a480bbfa`; the later merge
  integrates only René's RexxDoc formatter work.
- Retained runtime authority: the accepted PERF3-06/K04e Mac scorecard. The
  disturbed-host PERF3-12A wall-clock follow-up remains separate and is not a
  reason to rerun a valid baseline before a candidate exists.
- Mechanism/count authority: the checksum-closed
  [`PERF3-12 clause rereview`](evidence/2026-08-04-perf3-12-rexxcps-clause-rereview/)
  plus the accepted cursorless/X1 product. Audit identity before reusing the
  retained counts; recapture only a drift control needed for an exact
  candidate comparison.
- Canonical source: `tests/benchmarks/rexxcps_levelb.crexx`. Do not change it,
  clone it as a replacement benchmark, or add benchmark-specific recognition.
- Existing `STEMGET2`/`STEMSET2` opcodes and the accepted sparse CFG/SSA/use
  services are available. This slice does not authorize a new opcode, public
  RXAS/RXBIN form, language rule, runtime representation or RXC optimizer.
- Approach PoCs must remain independently replayable. Do not layer segmented
  stem selection and loop reuse before each route has its own exact ceiling,
  negative panel and ordinary profiling-off Release comparison.
- After the comparative PoC panel, stop for Adrian to select B0, B1, B2 or a
  bounded follow-on. A selected production edit then receives the mandatory
  first Release verdict and another stop before broad closeout.
- `CAP-06` per-worker arenas plus a central block depot is a deferred RXVM
  memory/threading lane. It is not part of PERF3-12B.

## Retained status quo and ceiling

The accepted optimized fixed-work profile executes 2,520,002
`CONCAT_REG_STRING_REG` instructions. Exact source/RXAS weighting assigns
2,240,000 to five `"Key Bee." || lvar` sites and 280,000 to the distinct
`"1.0" || lvar` default-tail site. The five target sites have weights
`1 + 1 + 2 + 2 + 2 = 8` over 280,000 hot `lvar` iterations.

Current lowering has the form:

```text
ITOS      right
TRACE     right
CONCAT    joined,"Key Bee.",right
TRACE     joined compound tail
STEMGET   result,stem,joined
```

The read/modify/write clause emits two independently materialized equal joined
keys, one for `STEMGET` and one for `STEMSET`. `key1` itself is constant-folded
to metadata/string operands, so there is no stable runtime register containing
the left segment `"Key Bee"` for an immediate `STEMGET2`/`STEMSET2` rewrite.

The existing two-segment handlers stream `left || "." || right` for hashing
and comparison. `STEMSET2` materializes the canonical key only on a proved new
insertion. Replacing every target concat gives B1 an upper bound of 2,240,000
removed dispatches (4.131225% of the pre-X1 optimized profile). Materializing
one joined key per `lvar` generation gives B2 a maximum 1,960,000 removed
dispatches (3.614822%). These are overlapping ceilings and must not be added.

## Design comparison

### B0 — retain current joined-key lowering

The control preserves current RXC constant folding, ordinary `CONCAT`, user
TRACE events and one-part native-stem operations. Retain B0 if neither route
can prove its semantics or if end-to-end benefit does not repay proof,
register, signal or assembler cost.

### B1 — segmented native-stem selection

Recognize a joined key produced only for one exact `STEMGET` or `STEMSET`,
prove its left and right segments, and replace the pair with existing
`STEMGET2` or `STEMSET2`. The proof must own the concat result's complete use
set and the stem operation's signal/indirect-write boundary.

The PoC must compare stable-left provisioning rather than hiding its cost:

1. reuse an already available stable left-segment register when one exists;
2. synthesize one procedure- or loop-dominating constant register, explicitly
   accounting for its `LOAD`, `.locals`, lifetime, metadata and assembler
   proof; and
3. retain a hand-equivalent exact machine ceiling as a control only.

A new constant-operand stem opcode is outside the selected surface and may be
recorded only as a rejected fallback if existing forms cannot approach the
ceiling for a demonstrated reason.

`STEMGET` and `STEMGET2` currently have unknown signal metadata even though the
shared handlers expose concrete pre-write failure paths. B1 cannot rely on
unknown-equals-unknown. Audit the four one-/two-segment get/set contracts from
the shared handler, correct metadata coherently in the isolated PoC if proved,
and preserve failure-visible writes exactly.

### B2 — loop-scoped joined-key reuse

Materialize a joined key once for a stable operand generation and redirect
later equivalent uses within the loop region. Request loop capability only for
matching candidates; do not build an eager whole-procedure loop/value layer.

Compare two safe placements:

1. lazy first-use materialization, which does not speculate a signal or TRACE
   event into a path that did not previously execute it; and
2. preheader placement only where non-signalling construction, dominance,
   must-execute and ordered-observation proofs make it equivalent.

The proof must use `ValueId`/effect identity and storage/component invariance,
not register numbers or source order. Operand writes, mapping changes,
references, indirect writes, context changes, calls, signal continuations and
phis terminate or reject a reuse generation.

### B3 — simple RXC hoisting control

An RXC emitter can retain or materialize the tail in a compiler temporary.
Use this only as an exact ceiling/control. Select it for production only if
compiler-only semantic knowledge is essential and the required emitter change
is demonstrably simpler and less entangled than preserving intent for RXAS.

### B4 — combined segmented selection and loop reuse

Deferred. The routes remove overlapping work and impose different register,
TRACE and signal obligations. Consider composition only after independent B1
and B2 results show a residual material target and define an unambiguous owner.

## Required semantic proof

- **UTF-8:** one-part lookup validates the joined string, while two-part lookup
  validates each segment. Arbitrary byte fragments can therefore differ even
  when their concatenation is valid. B1 requires independently proved valid
  segments or an exact failure-equivalence proof; otherwise reject.
- **Signals and writes:** preserve `UNICODE_ERROR`, `INVALID_ARGUMENTS` and
  `FAILURE`, their phase, destination writes and stem mutation. Corrupt native
  state, allocation failure and new insertion are explicit negatives.
- **Stem state:** cover hit, miss/default, insert, update, reset/generation,
  insertion order, empty/dotted segments and key equality.
- **Storage and aliases:** reject changed components, links/references,
  overlapping result/key/value storage, call-window exposure and indirect
  writes unless the proof service establishes equivalence.
- **Loops:** cover zero, one and many iterations; conditional first use;
  nested/reducible joins; operand mutation; break/iterate; signal exits and
  handler re-entry. Irreducible or over-budget regions fail closed.
- **TRACE:** classify the joined-tail `C` event before deleting or moving it.
  Optimized trace may vary under T1, but user-variable/value events and ordered
  event draining remain semantic observations. Do not call an event internal
  merely because its producing instruction disappears.
- **Registers and scale:** stable-left or cached-key registers must have
  explicit lifetime and `.locals` accounting. Do not bring forward final
  register compaction/reuse from PERF3-12D or recreate dense register scans.

## Execution plan

### B0 — product-neutral control plane

- [x] Re-read repository/performance instructions and the live roadmap.
- [x] Verify clean published `develop` at `965b461d8` before branching.
- [x] Reconcile the retained implementation queue with current RXAS, metadata,
  VM handlers and generated RexxCPS shape.
- [x] Log `CAP-06` as deferred without broadening this slice.
- [x] Commit this worklist and roadmap status as the product-neutral baseline.

### B1 — exact baseline and contract audit

- [ ] Verify the retained PERF3-12/X1 evidence checksums and product identities.
- [ ] Rebuild only artifacts needed to prove current optimized/no-opt RXAS,
  RXBIN and both VMs match the accepted baseline or explain bounded drift.
- [ ] Inventory every target static site, its operand provenance, TRACE record,
  stem consumer, loop membership and exact dynamic weight.
- [ ] Audit `STEMGET`, `STEMSET`, `STEMGET2` and `STEMSET2` handler/effect/
  component/signal contracts; add adversarial metadata/runtime fixtures before
  using a corrected contract in a candidate.
- [ ] Record assembler time/RSS and graph/SSA/use size for the unchanged input.

### B2 — isolated segmented-route PoC

- [ ] Build a replayable B1 candidate in an isolated tree/patch with no change
  to canonical benchmark source.
- [ ] Prove exact concat/stem pairing, segment provenance, individual UTF-8
  validity, complete joined-result uses, signal/write equivalence and TRACE
  disposition.
- [ ] Measure stable-left setup cost and `.locals`; reject any design whose hot
  path merely trades `CONCAT` for another per-use setup instruction.
- [ ] Run focused native-stem positives/negatives and opt/no-opt dual-VM
  equivalence, then capture exact static/dynamic work, buffer/copy/allocation
  effects and assembler scale.

### B3 — isolated loop-reuse PoC

- [ ] Build a separately replayable B2 candidate using capability-lazy loop and
  value/effect facts.
- [ ] Compare lazy first-use with preheader placement; keep only variants that
  preserve zero-iteration and ordered signal/TRACE behavior.
- [ ] Run loop/storage/reference/call/signal/TRACE adversarial fixtures plus
  opt/no-opt dual-VM equivalence.
- [ ] Capture exact static/dynamic concat removal, materializations retained,
  buffer/copy/allocation effects and assembler time/RSS.

### B4 — comparative Release panel and selection stop

- [ ] Build ordinary profiling-off Release B0, B1 and B2 products from exact
  recorded sources with both VM modes and no profiling instrumentation.
- [ ] Run the smallest decisive balanced/interleaved RexxCPS comparison plus
  native-stem and no-candidate guards. Keep raw outputs and negative results.
- [ ] Compare correctness, instruction/copy/allocation removal, runtime, RSS,
  image/register growth and assembler scale without summing overlapping
  ceilings.
- [ ] Retain replay material for both candidates and report B0/B1/B2 to Adrian.
- [ ] **Stop for route selection. Do not combine or install a production
  candidate before approval.**

### B5 — selected production route and mandatory first Release verdict

- [ ] Reimplement only the selected proof/consumer cleanly in production.
- [ ] Run minimum focused correctness needed for safe measurement.
- [ ] Freeze implementation, build the ordinary profiling-off Release product,
  run the accepted target/guard comparison and report exact counts plus runtime.
- [ ] **Stop for Adrian's first Release verdict.** Broad Debug, sanitizer,
  cross-platform, documentation polish and combined follow-ons wait.

### B6 — proportional closeout after acceptance

- [ ] Remove disposable PoCs while retaining replayable patches/evidence for
  selected and rejected options.
- [ ] Run focused plus required broad correctness and any selected sanitizer or
  supported-platform checks justified by the changed surface.
- [ ] Update RXAS/VM/compiler documentation and the live roadmap.
- [ ] Commit and publish only when requested.

## Immediate next step

`B1 — exact baseline and contract audit`: checksum the retained evidence,
prove current artifact identity, enumerate the five generated sites, and make
the one-/two-segment native-stem signal contracts exact before either PoC can
claim semantic equivalence.
