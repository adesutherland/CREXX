# PERF3-02-C2E2 symbolic storage-identity PoC worklist

Status: **complete — core-infrastructure candidate; production integration
awaits architecture approval**

Started: 2026-07-31

Completed: 2026-07-31

Purpose: prove whether RXAS can follow the storage cell addressed by a
register slot through cREXX `link`, `unlink` and `swap` operations, instead of
ending value-flow analysis at those operations or reasoning only from the
register number. The first PoC may add diagnostics and focused tests, but must
not rewrite emitted RXBIN or remove existing tactical optimiser rules.

## Authority and mandatory stop

Adrian approved a PoC proof of the algorithm on 2026-07-31. If the PoC is safe
and materially effective, return with a plan for making symbolic storage
identity a core RXAS flow facility and for consolidating applicable tactical
link/swap rules behind it. Do not install that production architecture or
delete tactical rules without the separate post-PoC selection gate.

Stop with one of these dispositions:

1. **core-infrastructure candidate** — transfer, join, loop and fail-closed
   behavior are mechanically tested; emitted products are unchanged; and
   current PERF3 evidence contains a material set of facts recovered beyond
   numbered-register taint;
2. **bounded follow-on needed** — the algorithm is sound but attributes,
   exceptional edges or another named missing fact prevent a representative
   effectiveness result; or
3. **reject** — storage identity cannot be represented safely at RXAS level or
   does not recover useful analysis coverage.

## Exact starting state

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch/HEAD: `develop` at
  `e38e514bf611ae3873513368c44742e2ae7332d1`
- Product-code parent: `3f43a0014be10c930a12b8a636297b60f294c0a6`
- Upstream relation: local `develop` is two commits ahead of
  `origin/develop` at `21fdcf529d0e51ea264bf0c92ccfbdc06dea8200`.
- Existing PERF3-02 and C1b documentation/evidence changes remain uncommitted.
- Five pre-existing untracked lifecycle RXBIN files remain protected and must
  not be overwritten, staged or deleted.
- The checksum-closed PERF3-02 panel remains immutable; this activity creates
  a separate evidence bundle and references the old panel.

## Falsifiable question

Can one forward, monotone, must-alias analysis assign each register slot a
symbolic storage identity at every reachable program point, transform that
identity exactly through supported mapping operations, intersect safely at CFG
joins and loops, and recover useful copy/swap facts without changing RXAS,
RXBIN, runtime behavior, TRACE/source identity or ownership semantics?

## Competing approaches

### S0 — retain tactical numbered-register rules

Keep the current keyhole swap-cancellation/fusion and duplicate-link-read
families plus the current whole-procedure global register taint. This is the
behavioral and emitted-image oracle.

### S1 — bounded symbolic storage environment

Propagate a map `register slot -> symbolic storage location` over the existing
whole-procedure CFG. Base frame slots are canonical identities; direct link
copies an identity, swap permutes identities, and unlink restores the
destination's base identity. A static attribute access path is exact only on
the operation's success edge, so the current combined success/skip fallthrough
remains unknown. A join retains an identity only when every incoming state
agrees. Unsupported dynamic aliases, references, lifetime changes and
incomplete control flow fail closed.

This is the selected diagnostic PoC because it adds no public format and can
be compared directly with S0.

### S2 — full location SSA / memory-ownership IR

Create explicit location definitions, memory versions and exceptional-edge
state, then make all value/lifetime analyses consumers. This is the plausible
long-term form if S1 succeeds, but is deliberately not selected for the first
PoC because it would combine proof, architecture migration and rewrites before
the recovered opportunity is known.

## Proof obligations

- Distinguish a numbered register slot from the `value *` storage it currently
  addresses.
- Prove entry/base, direct link, attribute-link success/skip continuation,
  swap, multi-swap, unlink and multi-unlink transfer functions against the VM
  handlers. A potentially throwing link is exact only on its success edge; a
  skip-resumed fallthrough remains unknown until exceptional edges are split.
- Use a monotone must meet: disagreement, unresolved input or unsupported
  effects produce `unknown`, never a guessed alias.
- Converge on loops and preserve exact identities only at agreeing joins.
- Treat incomplete control flow and asynchronous/same-frame signal entry as
  fail-closed.
- Do not infer full-value copy ownership, reference-payload equivalence,
  destructor safety or TRACE/source equivalence from storage identity alone.
- Keep canonical RXAS, RXBIN and the public ABI unchanged.
- Bound analysis memory and report a skipped procedure rather than allowing an
  unbounded compile-time cost.

## Effectiveness questions

1. How many link/unlink/swap transfers are exact versus unknown in Richards,
   Towers and the retained CRI-13 source?
2. How many full-copy sites rejected by procedure-wide numbered-register
   taint have an exact base or exact aliased storage identity at the copy?
3. Does the analysis explain the C1a-R1 stale-alias path and the restoring
   `unlinkn` edge without source-level knowledge?
4. Can the same environment detect pure swap/swapn permutation round trips
   beyond the current adjacency-specific rules?
5. Which tactical rules become candidates for later replacement by shared
   identity/value-fact consumers, and which remain instruction-selection
   rules?

## Work stages

### Stage A — control and semantic inventory

- [x] Freeze the starting commit, dirty scope and protected evidence.
- [x] Record S0/S1/S2 and the mandatory post-PoC architecture stop.
- [x] Reconcile opcode-effect metadata with the VM mapping handlers.
- [x] Inventory existing tactical link/swap rules and their focused tests.

### Stage B — diagnostic identity engine

- [x] Add a bounded storage-identity environment to the existing RXAS CFG.
- [x] Implement exact direct link, link-argument, swap/swapn/fused-swap and
      unlink/unlinkn transfers; mark throwing attribute/reference links
      success-only until exceptional edges are split.
- [x] Fail closed for unmodelled alias/reference/lifetime/opaque effects.
- [x] Emit debug-only aggregate diagnostics; perform no queue mutation.

### Stage C — focused mathematical and runtime proof

- [x] Add positive tests for straight-line, agreeing-join and stable-loop
      identity.
- [x] Add negative tests for disagreeing joins, dynamic aliases, references,
      lifetime mutation and signal-handler entry.
- [x] Add a pure swap-permutation round-trip diagnostic proof.
- [x] Prove optimized and no-opt runtime behavior remains unchanged on both
      VMs for the focused fixture.

### Stage D — representative effectiveness replay

- [x] Capture diagnostic summaries for Richards and Towers; reproduce the
      exact relevant CRI-13 instruction shape from its retained trace because
      the historical temporary input is no longer available.
- [x] Compare exact-at-point results with C2-E1's tainted/effect rejections.
- [x] Prove ordinary and debug assembly emit byte-identical RXBIN for every
      replayed input.
- [x] Publish exact counts and the interpretation boundary; no timing claim is
      made for a diagnostic-only analysis.

### Stage E — gate result

- [x] Map tactical rules to shared-analysis consumers or retained local
      instruction-selection rules.
- [x] State compile-time space/time bounds and unsupported cases.
- [x] Update the live roadmap and evidence index.
- [x] Present the gate result and stop before production integration or rule
      deletion.

## Gate result

Disposition: **core-infrastructure candidate**.

The PoC recovers materially more exact state than the current global
numbered-register taint without altering emitted bytecode. Richards has
171,203/176,414 exact state cells and all 55 globally tainted full-copy sites
are exact/base at the copy. Towers has 39,040/39,858 exact cells and 13/56
globally tainted full-copy sites are exact/base. These are necessary facts for
later value/ownership rewrites, not authorization to remove a copy.

The focused proof detects a non-adjacent unobserved `swapn` round trip and
rejects it when an affected mapping is written. Richards and Towers contain no
residual round trip after the current tactical optimizer, so the shared route
is proved but has no measured workload benefit in this slice.

The important safety finding is that throwing `linkattr*` and `linkref`
operations cannot produce an exact successor identity in the current CFG.
`SIGCALLA skip` resumes after the signal, merging success with an unchanged or
partially changed mapping. Production integration must split normal and
signal/skip mapping edges before attribute-path facts can drive rewrites.

The retained evidence and proposed integration sequence are in
[`2026-07-31-perf3-02-c2e2-storage-identity`](evidence/2026-07-31-perf3-02-c2e2-storage-identity/).
The mandatory post-PoC stop remains in force: no production consumer and no
tactical-rule deletion is selected by this result.
