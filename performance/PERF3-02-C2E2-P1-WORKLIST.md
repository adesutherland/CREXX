# PERF3-02-C2E2-P1 core storage identity and signal continuations

Status: **locked and accepted — P1A complete; no RXAS rewrite consumer
selected**

Approved: 2026-07-31

Purpose: promote the successful C2E2 symbolic storage-identity PoC into
reusable RXAS flow infrastructure and explicitly represent normal,
signal-skip and signal-retry continuations. Preserve every existing rewrite
and tactical rule until a later consumer is separately selected and passes the
mandatory first ordinary Release verdict.

## Authority and boundary

Adrian approved production Phase 1 and explicitly asked that the previously
identified signal/skip continuation limitation also be addressed. This phase
may refactor RXAS CFG/storage analysis and add focused semantic/runtime tests.
It must not remove copies, install a swap rewrite, delete tactical rules,
change RXAS/RXBIN/ABI semantics or push.

The first rewrite consumer is a later gate. If this infrastructure-only slice
changes an ordinary emitted RXBIN, stop and treat that as an unintended
production behavior change.

## Exact starting point

- Branch/HEAD: `develop` at
  `e38e514bf611ae3873513368c44742e2ae7332d1`.
- Existing PERF3-02/C1b/C2E2 code, worklists and evidence remain uncommitted.
- Baseline ordinary Release `rxas` was preserved at
  `/tmp/crexx-c2e2-p1-baseline.VosQ4p/rxas-baseline`, SHA-256
  `af80b2c9f664a2ed1b7ec7a4cca5a419718f2f5384e3cfb880f86c7a06ad7422`.
- Five pre-existing untracked lifecycle RXBIN files remain protected.
- The checksum-closed C2E2 PoC bundle remains immutable; P1 creates separate
  evidence.

## VM continuation contract

`RXOP_SEM_MAY_THROW` identifies instructions that can enter signal handling.
For storage mapping, the reusable flow model distinguishes:

1. **normal** — the instruction completed and its complete mapping transfer is
   visible;
2. **signal-skip** — an action-aware handler returned `__rxsignal_skip`, so
   execution resumes after the signal point with the mapping state present at
   that point;
3. **signal-retry** — the handler returned `__rxsignal_retry`, so execution
   resumes at the throwing instruction with the signal-point state;
4. **signal-handler** — same-frame branch/call-branch entry, retained as an
   explicit conservative target with unknown mapping; and
5. **terminal** — halt, fail or return leaves no procedure-local successor.

Simple `linkattr*` and `linkref` signal before changing the destination, so
their skip/retry state is the input mapping. Fused mapping instructions can
resize attributes or install an earlier link before a later signal; each gets
a named partial-state transfer. Unsupported partial effects fail closed.

Normal and skip may target the same following instruction. They remain
distinct edges, but a non-path-sensitive fact at that instruction must meet
them. Therefore edge-local normal success can be exact while the joined point
correctly remains unknown.

## Considered implementations

### P0 — retain the PoC's merged unknown transfer

Safe but not reusable: it records no successful edge identity and cannot
support later path-sensitive consumers or make retry/partial state explicit.

### P1 — typed continuation edges plus a graph-owned storage service

Add typed normal/skip/retry edges to the current CFG, keep same-frame handler
targets explicit, attach one bounded storage analysis to the final graph, and
provide edge-sensitive transfer/query helpers. Existing rewrite analyses
continue to consume normal CFG facts unchanged in this slice.

Selected because it addresses the semantic gap without bundling a rewrite.

### P2 — immediately migrate every liveness/value consumer

Ultimately desirable, but it could alter existing rewrites before the edge
model has independent equivalence proof. Queue it after P1, one fact family at
a time, with emitted-image diffs and the mandatory Release gate.

## Work stages

### Stage A — design and baseline

- [x] Reconcile `SIGCALLA` skip/retry and branch handler behavior with VM code
      and the interpreter guide.
- [x] Preserve the ordinary Release baseline assembler outside the repository.
- [x] Record P0/P1/P2 and select typed continuation edges without a consumer.

### Stage B — core CFG and storage service

- [x] Add typed normal, signal-skip and signal-retry CFG edges.
- [x] Preserve handler targets as explicit conservative continuation entries.
- [x] Split normal and signal-point mapping transfers, including fused partial
      mapping operations.
- [x] Retain the bounded graph-owned storage service and attach it on demand.
      The debug identity report is its current consumer; a selected rewrite
      consumer must attach the same complete service before querying it.

### Stage C — proof

- [x] Prove simple attribute/reference normal edges exact and skip/retry edges
      unchanged.
- [x] Prove fused partial-signal state fails closed.
- [x] Prove joined successors remain unknown when normal and skip disagree.
- [x] Execute a real `SIGCALLA skip` runtime fixture on both VMs, optimized and
      no-opt.

### Stage D — infrastructure verdict

- [x] Rebuild Debug and ordinary Release `rxas`.
- [x] Run the complete focused RXAS optimizer/runtime set.
- [x] Prove baseline/candidate ordinary assemblies are byte-identical for the
      focused fixture, Richards and Towers.
- [x] Measure bounded Release assembly overhead against the preserved baseline;
      this is assembler infrastructure, not a runtime benchmark claim.
- [x] Publish separate checksum-closed P1 evidence before final closeout. The
      three-way P1A verdict retains pre-P1, locked-P1 and candidate raw samples,
      identities and exact diagnostic equivalence in
      [`2026-08-01-perf3-02-c2e2-p1a-first-release-verdict`](evidence/2026-08-01-perf3-02-c2e2-p1a-first-release-verdict/).

The first verdict found byte-identical output and a bounded assembly-time cost:
Richards `+5.3%` median and Towers `+0.5%` median (`+2.0%` mean), over 30
balanced pairs. Adrian accepted retaining P1 while assembler-cost rework is
queued as `PERF3-02-C2E2-P1A` on the near roadmap. The subsequent R1 panel
freezes this exact source/binary identity and uses its storage summaries across
all compiler masks. A synthetic residual swap proof was considered and rejected
before implementation because Richards and Towers contain no residual round
trip. C1abc is a compiler-owned recommendation and does not make an RXAS
storage consumer safe: no rewrite consumer or tactical-rule deletion is yet
selected.

P1A subsequently retained demand-driven A1 and rejected feature-gated A3 after
the governed same-session rework panel. Accepted closeout reproduces the A1
Release binary exactly and passes 24/24 focused plus 1,972/1,972 broad Debug
tests. The P1 typed-continuation/storage infrastructure remains locked; only
dead ordinary storage attachment was removed.
