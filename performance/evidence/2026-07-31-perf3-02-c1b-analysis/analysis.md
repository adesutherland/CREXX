# C1b analysis and decision

## Decision

Advance only `C1b-R1 detached receiver-guard snapshot` as a **bounded isolated
PoC candidate**. Do not revive the broad C1a-R1 rule and do not change the
general `IF` emitter in the first experiment.

This decision is analysis-only. It does not select C1b for production and does
not displace the independently correct C1c-R1 or C1a-R2 candidates.

## Exact opportunity

The material remainder is exactly two Richards call sites:

1. `runTask()` line 218 calls `isWaitingWithPacket()` and materializes
   `§this`; 65,790 dynamic copies account for 9,671,064 recursive operations
   and 76,842,192 bytes.
2. `schedule()` line 300 calls `isHoldingOrWaiting()` and materializes
   `§this`; 106,604 dynamic copies account for 15,670,674 recursive operations
   and 124,512,560 bytes.

Together they are 172,394 top-level copies, 25,341,738 recursive operations
and 201,354,752 bytes. They are about 5.19 times the top-level and 5.18 times
the recursive work removed by the correct C1a-R2 experiment. That makes the
slice material enough to investigate, but not enough to infer a speedup from
operation ratios.

## Why C1a-R1 produced `0/1`

A real method call has a separate frame. Its local registers start mapped to
frame-owned storage, receiver attribute reads temporarily redirect registers
to object attributes, and returning or unwinding discards that frame. In the
inlined form, returns are rewritten to `LEAVE_WITH` branches inside the
caller's frame, so a redirected temporary must be restored before every such
branch.

The current `IF` emitter evaluates the predicate, emits `brf`, emits the true
body and places predicate cleanup only at the convergence or false label. A
true body containing rewritten `LEAVE_WITH` therefore jumps around that
cleanup.

The C0 private receiver copy accidentally provides the required containment:
any stale alias points into the private copied object. C1a-R1 removes that
copy, so the same stale alias points into the real scheduler object.

The invalid RXAS proves four unsafe taken edges:

- `isWaitingWithPacket()` line 171 leaves `r7` linked to
  `task_holding[identity]` and `r5` linked to its outer attribute. In the
  continuing `runTask()` body, `igetunlink r7,r3` reuses `r7`; when the stale
  mapping is present its integer store targets the scheduler Boolean element.
- `isHoldingOrWaiting()` lines 162-164 each leave an outer/nested pair linked
  on the true return. For example, line 163 leaves `r7` linked to
  `task_pending[identity]`; a later loop iteration can execute `load r7,1`
  before that register is relinked, writing through the stale alias.

The comparison guards at `isWaitingWithPacket()` lines 169-170 are safe
because conversion/comparison lowering emits `unlinkn` before `brf`. The two
final returns are also balanced, giving four safe and four unsafe return paths.
The exact eight-path ledger is in `exit-link-ledger.csv`.

This explains both the mechanism and the severity of the observed optimized
smoke result: expected queue/hold counts `23246/9297`, actual `0/1`.

## Strategy findings

### B1 — common-exit normalization is already present and insufficient

The inliner already rewrites all explicit returns to `LEAVE_WITH` and one
`bexprend` label. The unsafe predicate cleanup is control-dependent and occurs
before that epilogue, so common-exit normalization alone cannot know which
temporary alias pairs are still live. Hoisting all possible cleanup to the
epilogue would also need emitted-register liveness and exceptional-edge facts.

### B2 — private per-exit copyback has no ceiling here

These methods do not write the receiver. Keeping private receiver storage and
adding per-exit copyback therefore removes none of the two full copies. The
existing copyback wrapper is useful for mutating methods but is not a
performance route for this slice.

### B3 broad — rejected

The no-write summary used by C1a-R1 says nothing about live receiver-derived
aliases. It is permanently insufficient as a direct-binding gate.

### B3 narrow — bounded via a detached scalar guard

The finite candidate is to make alias lifetime explicit in the inline AST:

1. Revalidate the callee body and require a method receiver, exact scalar
   Boolean result, multiple explicit returns and no fallthrough.
2. Require zero calls, receiver writes, references, assembler, loops or signal
   blocks.
3. Require every receiver use to be an indexed exact-scalar Boolean read
   wholly contained in an `IF` predicate; require each non-final branch body to
   terminate immediately in a scalar return and the final statement to be a
   scalar return.
4. Reject if the call site is inside a same-procedure signal-handler region.
5. Before binding the receiver directly, rewrite every receiver-derived
   predicate as a generated scalar assignment followed by an `IF` on that
   scalar. Preserve the original predicate's source/TRACE anchors and keep the
   generated temporary out of the user surface.
6. Re-run the structural proof after cloning and fail closed on any mismatch.

Assignment lowering copies the scalar value and emits its right-hand-side
cleanup within the assignment statement. The following `IF` therefore reads
an owned scalar register, not an attribute alias. Every `LEAVE_WITH` is reached
with no live receiver-derived alias.

This route is candidate-local compiler work. It needs no RXAS/RXBIN opcode,
runtime ABI, object layout or language change. It may recompute the structural
fact from the validated local/imported AST template, so an inline-summary
schema change is not required for the first PoC. A later production-quality
generalization may choose to serialize an explicit versioned fact, but must
reconstruct and compare it on import.

## Signal and unwind boundary

Array link/index instructions can signal. In a real call, a branch handler in
the caller causes the callee frame to be unwound and discarded. After inlining,
a handler installed in the same caller frame would not discard that frame and
could observe stale aliases if a signal occurs before scalar-snapshot cleanup.

Neither exact callee nor containing Richards procedure contains a signal
block. An inherited handler in an outer frame still unwinds and discards the
current frame, matching the separate-call behavior. The first PoC must
nevertheless reject call sites inside a same-frame handler region. Supporting
those sites later requires an explicit exceptional-edge cleanup proof and is
outside this bounded slice.

## Correctness-valid machine ceiling

For the two exact sites, a successful ceiling has:

- no full receiver `copy`, destination allocation, recursive traversal or
  ownership search;
- a generated scalar snapshot for each receiver-derived guard, with all outer
  and nested alias registers restored before the `IF` branch;
- the existing common `LEAVE_WITH` result epilogue;
- unchanged receiver, formal, return and source/TRACE behavior;
- unchanged no-opt behavior unless the candidate is intentionally enabled
  there for proof; and
- canonical Richards output `23246/9297` in optimized and non-optimized
  images on both `rxvm` and `rxbvm`.

The ceiling removes the retained full-copy counts in `site-ledger.csv` while
allowing bounded scalar-copy and `unlink`/`unlinkn` overhead. It is not yet a
measured or correctness-demonstrated result.

## Required isolated PoC gate

If Adrian authorizes the next step, keep it isolated and correctness-first:

1. implement only the structural recognizer, detached scalar-guard rewrite and
   direct-binding gate described above;
2. add focused local and imported-template tests for the two positive shapes
   and fail-closed controls for calls, writes, non-guard reads, loops,
   references, nested blocks and same-frame signal handlers;
3. prove emitted cleanup precedes every rewritten return and that the two full
   receiver copies disappear while unrelated images remain unchanged;
4. run optimized/no-opt Richards on both VMs and require exact `23246/9297`;
5. stop and report correctness and deterministic instruction/count deltas
   before any timing or production selection.

Only after that gate passes should Adrian decide whether C1b competes with or
follows C1c-R1/C1a-R2. A failed gate retains B0 and closes this route without
weakening correctness.
