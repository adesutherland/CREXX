# PERF3-02-C1B receiver-link ownership analysis worklist

Status: **complete — C1b-R1 correctness PoC succeeds; timing and production
selection await approval**

Started: 2026-07-31

Purpose: determine whether the material correctness-rejected C1a-R1 ceiling
contains a bounded, general C1b compiler proof for attribute-reading or
link-owning multi-return receivers. This worklist began as an analysis-only
gate. Adrian subsequently approved the exact C1b-R1 detached receiver-guard
correctness PoC recorded below. Timing, production selection, a broader rule,
commit and push remain outside this authority.

## Decision gate and mandatory stop

Return to Adrian with one of these dispositions:

1. **bounded PoC candidate** — exact receiver/link facts, every relevant
   normal/return/fallthrough/signal/unwind obligation and a correctness-valid
   machine ceiling are finite and testable;
2. **defer** — the opportunity remains material, but the minimum proof requires
   broader alias, exception, metadata or runtime-ownership architecture; or
3. **reject** — the private materialized receiver is semantically required and
   no narrower general transformation survives the proof.

The analysis stop was satisfied and Adrian approved the bounded C1b-R1
correctness PoC on 2026-08-01. That approval covers only the structural
recognizer, detached scalar guard snapshots, direct-binding gate, focused
positive/fail-closed tests, emitted-storage verification and the canonical
Richards correctness matrix. It does not authorize timing, production
selection, a broader receiver-sharing rule, combination with C1a-R2/C1c-R1,
commit or push.

Initial analysis disposition: **bounded PoC candidate**, limited to `C1b-R1 detached
receiver-guard snapshot`. The broad C1a-R1 no-write rule remains rejected.
See
[`evidence/2026-07-31-perf3-02-c1b-analysis/`](evidence/2026-07-31-perf3-02-c1b-analysis/).

## Exact starting state

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch/HEAD: `develop` at
  `e38e514bf611ae3873513368c44742e2ae7332d1`
- Product-code parent: `3f43a0014be10c930a12b8a636297b60f294c0a6`
- Upstream relation: local `develop` is two commits ahead of
  `origin/develop` at `21fdcf529d0e51ea264bf0c92ccfbdc06dea8200`
- Existing PERF3-02 documentation/evidence changes remain uncommitted.
- Five pre-existing untracked lifecycle RXBIN files remain protected and must
  not be overwritten, staged or deleted.
- The 287-row checksum-closed
  `evidence/2026-07-31-perf3-02-copy-ownership-panel/` bundle is immutable and
  is referenced rather than amended.

## Known material boundary

The two C1a-R1-only Richards sites execute 172,394 top-level copies,
25,341,738 recursive copy operations and 201,354,752 logical bytes. The broad
shortcut is correctness-invalid: the optimized smoke expected queue/hold
counts `23246/9297` and produced `0/1`.

This is an upper bound, not a speed forecast. No valid C1a-R1 timing exists.

## Falsifiable question

Can `rxc` avoid the private full-value receiver copy at these sites while
preserving every receiver-owned attribute link, unlink, alias, value and source
metadata obligation across all rewritten exits, without a new RXAS/RXBIN,
runtime ABI, value-layout or language contract?

## Invariants

- Preserve by-value receiver isolation and observable attribute values.
- Preserve receiver-owned link/unlink balance and destruction ownership.
- Preserve argument/reference identity and all alias/generated-storage guards.
- Preserve evaluate-once behavior and source/TRACE projection.
- Preserve normal return, multiple explicit returns, fallthrough, signal,
  error and unwind behavior.
- Fail closed for unresolved calls, callbacks, native/plugin boundaries,
  dynamic aliases and incomplete exit summaries.
- Keep the C1c-R1 and C1a-R2 proofs independent; do not combine candidates.
- Treat removed operation counts as a ceiling until correctness passes and an
  ordinary profiling-off Release comparison is separately authorized.

## Strategies to compare

### B0 — current materialized receiver

Retain the private receiver copy and current cleanup/copyback behavior. This is
the semantic control and remains the fallback at every unproved site.

### B1 — common-exit normalization

Rewrite eligible explicit returns to one compiler-owned epilogue so receiver
copyback/link cleanup occurs exactly once. The analysis must prove result-value,
source/TRACE, signal and fallthrough equivalence and show whether the inliner
already has a suitable block/`LEAVE WITH` representation.

### B2 — path-complete per-exit copyback

Keep private receiver storage but synthesize the same copyback/link cleanup on
every cloned normal exit. The proof must enumerate exits after fixed-point
cloning and establish exactly-once cleanup under nested blocks and early
returns. This may reduce control restrictions but does not itself remove the
initial full copy; its machine ceiling must therefore be stated honestly.

### B3 — direct binding with link-lifecycle retargeting

Bind the inlined receiver directly to the actual only when the compiler can
retarget every receiver-owned attribute link and corresponding unlink/cleanup
to the actual without changing alias visibility or lifetime. This is the only
listed strategy that could meet the no-copy ceiling at the failed sites, and it
must fail closed if ownership cannot be summarized locally.

The bounded result is narrower than general retargeting. For the exact two
callees, every receiver read is an indexed scalar Boolean guard. Snapshotting
each such guard into a compiler-owned scalar statement makes its alias cleanup
complete before the following `IF` and therefore before every rewritten
`LEAVE_WITH`. The proposed first PoC must reject calls, writes, non-guard
receiver reads, loops, references, assembler, callee/caller-local signal
blocks and any post-clone shape mismatch. It does not require an RXAS/RXBIN,
runtime ABI, object-layout or language change.

## Required evidence

- exact source, optimized RXAS and AST/metadata identity for both sites;
- callee summaries: receiver/formal reads, writes, calls, returns, escapes and
  attribute-link operations;
- path ledger covering every exit and cleanup event before/after inlining;
- explanation of the observed `0/1` failure from concrete emitted operations;
- comparison of B1/B2/B3 proof facts, implementation surface, residual copies,
  semantic risk and expected test burden;
- correctness-valid machine-ceiling definition or an explicit reason none is
  bounded; and
- recommendation: bounded PoC, defer or reject.

## Work stages

### Stage A — control plane

- [x] Freeze repository/evidence identity and protected dirty scope.
- [x] Preserve the checksum-closed PERF3-02 panel unchanged.
- [x] Record the three analysis strategies and mandatory stop.

### Stage B — site reconstruction

- [x] Identify both failed source call sites, callees and optimized RXAS copy
      sites exactly.
- [x] Reconcile their 172,394 dynamic executions and recursive payload counts.
- [x] Compare C0, invalid C1a-R1 and safe C1a-R2 emitted paths.

### Stage C — ownership and exit proof

- [x] Inventory receiver-owned link/unlink and attribute access for each callee.
- [x] Enumerate normal/explicit-return/fallthrough/signal/unwind paths.
- [x] Explain the `23246/9297` to `0/1` correctness collapse mechanistically.
- [x] Identify the minimum additional structural facts; no first-PoC metadata
      schema change is required because the validated AST template can be
      rechecked locally and after import.

### Stage D — strategy and ceiling comparison

- [x] Evaluate B1 common-exit normalization: already present and insufficient
      because taken returns bypass predicate cleanup before the epilogue.
- [x] Evaluate B2 path-complete per-exit copyback: reject for this slice because
      keeping private receiver storage removes none of the target copies.
- [x] Evaluate B3: reject broad direct binding; retain only the detached
      receiver-guard snapshot as a bounded PoC candidate.
- [x] State the exact correctness-valid machine ceiling and residual scalar
      snapshot/cleanup work.

### Stage E — decision package

- [x] Publish a compact evidence-local analysis without copying the closed
      PERF3-02 raw bundle.
- [x] Update the live roadmap and this worklist with the disposition.
- [x] Run structural/whitespace/link/checksum checks and verify protected RXBIN
      hashes.
- [x] Present the result to Adrian and stop before any edit or timing.

## Completed outcome

The failure is an alias-lifetime error, not evidence that the full receiver
copy is semantically mandatory. In C1a-R1, four direct Boolean guard return
edges branch to the inline epilogue before `unlinkn` restores the outer and
nested attribute aliases. Later register reuse writes through those stale
aliases into the real scheduler object. C0 contains those writes inside the
private receiver copy; safe C1a-R2 retains both material copies.

The bounded C1b-R1 route is to detach all six receiver-derived guard
expressions in the two callees into generated scalar assignments before their
`IF` statements, then allow direct receiver binding only under the complete
structural gate recorded above. Assignment lowering already completes
right-hand-side cleanup within the statement. The resulting machine ceiling
removes two static and 172,394 dynamic full receiver copies, 25,341,738
recursive operations and 201,354,752 logical bytes while retaining small
scalar snapshot and cleanup operations.

Signal behavior remains explicitly bounded. An outer-frame handler unwinds
and discards the current frame as a real call would; an enclosing handler in
the same caller frame would not. The first PoC must therefore reject the latter
shape rather than claiming a general exceptional-edge proof. The implemented
PoC enforces that rejection.

The separately approved isolated correctness PoC succeeded. The next activity,
only if Adrian approves it, is the mandatory smallest decisive ordinary
profiling-off Release timing verdict against retained C0 evidence. Timing,
production selection, broadening the rule and combination with C1c-R1/C1a-R2
remain unauthorized.

## Approved correctness PoC gate (2026-08-01)

- [x] Preserve an ordinary profiling-off Release compiler control before the
      first production edit.
- [x] Implement only the exact detached receiver-guard structural recognizer.
- [x] Snapshot each accepted receiver-derived Boolean predicate into generated
      scalar storage before its `IF`, then revalidate the cloned shape.
- [x] Admit direct receiver binding only when both validations succeed; all
      other shapes retain the current materialized receiver.
- [x] Add local/imported positive coverage and fail-closed cases for calls,
      writes, non-guard reads, loops, references, nested blocks and same-frame
      signal handling.
- [x] Independently verify emitted exit storage/alias state and that the two
      target receiver copies disappear while unrelated images remain stable.
- [x] Require `23246/9297` for optimized and no-opt Richards on both `rxvm` and
      `rxbvm`.
- [x] Report deterministic correctness and count deltas, then stop before any
      timing.

## Correctness PoC outcome

The PoC passed its mandatory correctness stop on 2026-08-01. The retained C0
compiler is SHA-256
`caec40f5d7d6304e29e2a678c5604a5d78e42172bc1ee0bf82b52a130f5f99d1`;
the unchanged P1 assembler is
`09ea8e49c54dfc839833a1fa40dfa5809467aa53c730dc050ae0a2b95f73669e`.
The final C1b compiler is
`0323b70207c22486896d6883a80ca4388f6a4c164d03737bcefc3a3a8ed6be52`.

Optimized C0 and optimized/no-opt C1b-R1 all produce the canonical Richards
`23246/9297` result on both `rxvm` and `rxbvm`. The generated Richards image
contains 69 rather than 71 static `copy` instructions: the source-anchored
receiver copies in `Scheduler.runTask` and `Scheduler.schedule` are absent,
and the independent procedure verifier attributes one fewer full-copy event to
each target. Six detached scalar guard snapshots remain. Towers and the
class-method control stay byte-identical between C0 and C1b-R1.

The independent P1 storage-identity verifier reports unchanged exact
link/unlink balance (`126/126` in `runTask`, `14/14` in `schedule`), reduces
unknown join state to zero in both procedures, and removes exactly one
full-copy event from each target procedure. The focused receiver/import suite
passes 9/9 and the Richards benchmark correctness suite passes 4/4. The local,
imported and fail-closed cases cover calls, writes, non-guard reads, loops,
reference formals, nested-return blocks and same-frame signal continuations.

No timing was performed. Exact matrices, product identities, deterministic
image deltas and verifier counts are retained in
[`evidence/2026-08-01-perf3-02-c1b-correctness/`](evidence/2026-08-01-perf3-02-c1b-correctness/).
