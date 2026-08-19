# PERF3 closeout worklist

Status: **complete and historical — PERF3 retired 2026-08-17**

This is the sole control plane for retiring PERF3. It closes the detailed
programme without turning closeout into another open-ended VM campaign. The
historical evidence and activity ledger remain in `ROADMAP.md`; project-wide
successors live in `docs/ROADMAP.md`.

Concurrency qualification is independent. Do not change its frozen product or
reinterpret its host evidence from this worklist.

## Locked decisions

1. Freeze production VM work. The two existing immutable load-time private
   fusions remain eligible for retention, but adaptive quickening, dequickening,
   selector caches and new public fusion opcodes are closed.
2. Preserve every historical benchmark and scorecard. Portfolio v2 uses
   separately named sources and starts a new evidence series.
3. Keep canonical AWFY Mandelbrot and RexxCPS 2.2d unchanged. Diagnostics and
   source-hoisted controls use distinct names.
4. Use the compiler-selected `rxvm` for product verdicts. `rxtvm` and `rxbvm`
   are concrete-engine controls and must not duplicate an `rxvm` alias in an
   aggregate.
5. Ordinary profiling-off Release wall clock remains timing authority.
   Profiles, RXAS counts and fusion censuses are diagnostic evidence only.
6. No production compiler/RXAS candidate is implied by closeout. A selected
   candidate needs an explicit design comparison and the mandatory first
   Release verdict.

## Closeout stages

### CLOSE-01 — reconcile the control plane

- [x] Review current PERF3 rows against accepted evidence.
- [x] Mark stale, superseded, transferred and independently owned entries.
- [x] Remove completed transactional PARSE from the central future direction.

Reconciliation: PERF3-04 transfers as the post-PERF3 generic scalar-accessor
proof; PERF3-05-B1 remains independent build/API hygiene; PERF3-05-R1 is
superseded by the completed R2-R5 handler investigation; PERF3-05-R4 is closed
by the registry below; PERF3-12C's open production wording is superseded by the
integrated PERF3-12D exit-owned lowering; PERF3-13 and DECIMAL-01 retain their
independent control planes. Cross-platform release/concurrency qualification
does not become PERF3 closeout work.

### CLOSE-02 — portfolio v2 contract

- [x] Add the version-2 manifest without changing the retained seed manifest.
- [x] Freeze provenance, workload class, comparability and aggregate membership.
- [x] Define product and concrete-engine identities explicitly.

Contract: [`portfolio/manifest-v2.md`](portfolio/manifest-v2.md). The v2 common
aggregate retains five members but replaces only its Base64 source with
Base64-v2, so it is a new evidence series rather than a mutation of v1.

### CLOSE-03 — benchmark source closeout

- [x] Retain Base64 v1 and add idiomatic Base64 v2 for cREXX, ooRexx and
      decimal NetRexx.
- [x] Retain canonical Mandelbrot and add a source-hoisted conversion control.
- [x] Replace the ambiguous JSON label with separately named parse and query
      controls; retain the old probe for historical evidence only.
- [x] Add cREXX AWFY Queens and NBody ports from pinned upstream
      commit `74306fec151070fd07157cefeacf19e7e0bcdc89`.
- [x] Keep full AWFY Json, DeltaBlue, CD and Havlak as post-PERF3 successors.

Queens exposed a real optimized-code defect in the accepted NR-26 copy/dead-
store fixed point. The 2026-08-17 repair uses reaching-definition order to
reject only an unmaterialised intermediate register; it preserves nested
inlining and valid old-value forwarding. Adrian accepted the ordinary Release
verdict, full Debug passes 2,224/2,224, and focused Release closeout passes 7/7.
Evidence:
[`rxc flow-copy fixed-point repair`](evidence/2026-08-17-rxc-flow-copy-fixed-point-repair-first-release-verdict/).
Queens and NBody subsequently passed the complete CLOSE-05 product/concrete-
engine qualification record. They remain named Tier B controls outside every
aggregate, as frozen by the v2 contract.

### CLOSE-04 — runner and result identity

- [x] Make the convenience runner explicitly smoke/process oriented.
- [x] Accept all three executable names while labelling `rxvm` as product and
      concrete engines as controls.
- [x] Make the formal aggregate tolerate a product-only cREXX aggregate and
      omit absent legacy concrete-engine aggregates rather than fabricating
      cells.

The maintained Level B matrix self-test now covers a product-only aggregate.
Focused Debug validation passes runner and reducer in both optimized and
unoptimized modes, and a direct `rxtvm` smoke result is labelled
`concrete-engine-control,process-smoke`.

### CLOSE-05 — correctness qualification

- [x] Build optimized and unoptimized images for every new Level B source.
- [x] Run deterministic smoke checks under the product and both concrete VMs
      where the runtime substrate applies.
- [x] Record any language/runtime adaptation; never promote a provisional port
      merely because it compiles.

Qualification evidence:
[`portfolio-v2 qualification`](evidence/2026-08-17-perf3-closeout-portfolio-v2-qualification/).
All 36 cREXX process cells pass; NBody's full reference passes under all three
executable names; Base64-v2 also compiles/runs under ooRexx and decimal
NetRexx with the same length/checksum contract. This is correctness evidence,
not timing.

### CLOSE-06 — fusion registry and gap report

- [x] Inventory RXAS-owned static fusions and both VM private fusions.
- [x] Record canonical shapes, proof owner, fallback and observability contract.
- [x] Recount static eligible sites on portfolio v2.
- [x] Reconcile retained dynamic evidence and state what the current profiler
      deliberately attributes to canonical public instructions.
- [x] Retain or retire each fusion explicitly, then close quickening as an
      active programme.

Registry: [`PERF3-FUSION-REGISTRY.md`](PERF3-FUSION-REGISTRY.md). The corrected
post-RXAS census finds 1,248 public serialized sites and 34 exact private sites
across 17 optimized portfolio-v2 images. Both accepted private fusions remain
immutable load-time specializations. Adaptive quickening, dequickening,
selector caches and new public fusion opcodes are closed.

### CLOSE-07 — current Mac baseline

- [x] Confirm controlled source, quiet AC power, low-power mode off and low
      host load. The uncommitted approved closeout scope is recorded as base
      commit plus exact hashes because no publication commit was authorized.
- [x] Build a fresh ordinary profiling-off Release product.
- [x] Pass the benchmark correctness gate before timing.
- [x] Run two warmups and ten serial recorded samples per formal cell, rotating
      runtime order and retaining all samples.
- [x] Keep RexxCPS native CPS, lifecycle, RSS and artifact results separate.
- [x] Treat Java and CPython AWFY runs as labelled controls, never Rexx
      aggregate members. No direct Java/CPython language ports enter this
      baseline; NetRexx's normal generated-Java substrate remains disclosed.

Baseline:
[`portfolio-v2 Mac closeout`](evidence/2026-08-17-perf3-closeout-current-mac-v2/).
The formal matrix passes 516/516 initial and 150/150 governed-append
processes, retains every sample, and closes 87 artifact identities. Product
common-five geometric means are 4.897751 versus ooRexx and 1.543319 versus
decimal NetRexx. Richards remains below both references and Permute remains
below NetRexx. Base64-v2 makes this a new evidence series, not a v1 trend.

### CLOSE-08 — retire PERF3

- [x] Publish the current scorecard or record a precise blocked/inconclusive
      boundary.
- [x] Move durable successors to `docs/ROADMAP.md`.
- [x] Mark this worklist and the detailed PERF3 ledger complete/historical.
- [x] Review the complete diff, run `git diff --check`, and commit only on
      Adrian's publication instruction.

Final audit: the combined fresh Release replay passes 26/26, the evidence
bundle verifies 52 recursive checksums, and `git diff --check` passes. The
closeout remains uncommitted and unpushed pending Adrian's publication
instruction.

## Post-PERF3 order

1. Full AWFY Json.
2. DeltaBlue and CD, sharing any honest container/identity foundations.
3. Havlak after those foundations are proven.
4. Generic final/concrete scalar-accessor proof against the expanded suite.
5. Broader bounded hoisting, register finalisation and late inlining, selecting
   at most one production candidate at a time from current evidence.
6. At release-candidate freeze, regenerate the VM handler profile and review
   low-level `rxvm` layout/default-engine shape with platform evidence.
