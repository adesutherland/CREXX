# PERF2-01 candidate ownership map

These are bounded PERF2-02/03/04/06/07 selection panels, not implementations.
Every candidate must first preserve exact correctness and then win ordinary
profiling-off Release timing against this bundle.

## Recommended first PoC: value/reference transfer panel

Start with the PERF2-07/PERF2-06 boundary, not selector quickening. Measure one
small implementation at a time against the current path:

1. Bounce reference construction/storage: a proven direct reference-storage
   path for `MKREF_REG_REG` that avoids repeated general value-tree discovery
   when the source shape establishes the same lifetime semantics.
2. Richards typed/scalar copy: bypass recursive general `copy_value` only when
   the opcode/site proves a non-owning scalar or already-established reference
   shape; preserve the current generic fallback.
3. Guard panel: Storage and Towers for recursive ownership/allocation safety;
   Sieve and Permute for existing-win regression; Base64 for an unrelated
   string-path control.

Why first: Bounce spends about 263 ms in `MKREF` and over four fifths of stable
native samples in reference-tree storage. Richards spends about 0.84-0.85 s in
`COPY_REG_REG`, with 96.1 million copy operations / 762.8 MB. The mechanisms
are concrete, repeated across both VMs and distinct from lifecycle noise.

## PERF2-02: stable-site quickening panel

- Candidate: guarded private specialization for the exact reference/attribute
  sites exposed by the first PoC, with type/shape checks and generic fallback.
- Evidence tie: Bounce attribute/link counts and reference-storage samples;
  Richards attribute/copy counts.
- Gate: do not start from `srcmethodsel`/`srcfprocsel` caching. Current accepted
  profiles execute zero selector attempts, so selector targets/hit rates do not
  support a first quickening choice.
- Guards: Sieve, Permute, Base64, all RXSEQ boundaries, complete site-status
  rows and ordinary-product timing.

## PERF2-03: compiler flow and inlining panel

- Candidate: bounded direct-call inlining/call-shape cleanup for Richards
  `runtask`/`queuepacket`, Permute recursion and the high-frequency List/JSON
  call sites, without changing public call semantics.
- Evidence tie: 119k Richards calls, 433k Permute calls, 4.396M List calls and
  745k JSON calls; reused-frame counts match those call volumes.
- Gate: instruction reduction and correctness first, then ordinary Release
  timing. Richards frame entry itself is only ~6.4 ms in the bounded profile,
  so a frame-only rationale is insufficient.

## PERF2-04: BIF and language-operation panel

- Candidate: `upper`/string conversion on RexxCPS and string-position/copy on
  Base64, evaluated as separate candidates.
- Evidence tie: RexxCPS `upper` 79-84 ms, call opcode 44-47 ms, conversions
  47-48 ms; Base64 `SETSTRPOS` 180-207 ms and decoder 1.45-1.47 s.
- Controls: exact-hash BIF, call-arg, TRACE/ADDRESS, stems, decimal-string and
  PARSE family probes. TRACE inactive cost must remain near zero.

## PERF2-06: execution-stream and RXAS panel

- Candidate: examine whether the proven Bounce reference path and Base64
  string-position path merit an RXAS/private execution form only after a
  value-helper PoC wins. Keep rxbvm and rxvm variants separately visible.
- Evidence tie: explicit opcode timing identifies `MKREF` and `SETSTRPOS`;
  Sieve and Permute show that broad dispatch change is not automatically useful.
- Gate: no public RXBIN/ABI change, no broad dispatch rewrite and no selection
  from RXSEQ frequency alone.

## PERF2-07: value/frame panel

- Candidate A: direct reference storage for proven Bounce shapes.
- Candidate B: typed/scalar copy bypass for proven Richards shapes.
- Candidate C: only after A/B, trim/reset lifetime cleanup guarded by Storage
  and Towers allocation evidence.
- Evidence tie: the mechanism census gives exact operations, bytes, timing,
  native samples, allocation high water and RSS.
- Gate: reject any candidate that trades time for retained memory, changes
  reference lifetime, or worsens the already-winning Sieve/Permute cells.

## Deferred selection

- Generic selector caches: no executed sites in this portfolio.
- Loader/binder work: ~2.7 ms load-first-result is separate and does not own
  the steady-state gaps.
- Mandelbrot, Towers, Storage, List and JSON cross-runtime optimization claims:
  capability/equivalence gates precede common-score use.
- LTO/PGO, pooling, representation redesign, public opcodes and benchmark
  shortcuts: outside this activity and unsupported as a first measured panel.
