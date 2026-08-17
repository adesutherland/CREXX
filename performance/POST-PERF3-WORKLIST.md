# Post-PERF3 performance worklist

Status: **active — POSTPERF-02 DeltaBlue and CD**

Started: 2026-08-17

Base commit: `110e298af` (`perf: close PERF3 and establish portfolio v2`)

This is the restart authority for the approved post-PERF3 sequence. Each stage
must finish its own provenance, correctness, evidence, documentation, diff
review and local commit before the next stage starts. Nothing here authorizes a
push. A production compiler, RXAS or VM performance edit still stops at the
mandatory first ordinary-Release verdict in `performance/AGENTS.md`.

## Sequence and status

| ID | Stage | Status | Exit |
| --- | --- | --- | --- |
| POSTPERF-01 | Full AWFY Json | complete | Exact upstream payload and verification contract; opt/no-opt product and concrete-VM qualification; retained source/RXAS/RXBIN evidence; docs and local commit |
| POSTPERF-02 | DeltaBlue and CD | active — source/design audit not started | Both pinned upstream ports qualified together because they share identity/container foundations; neither is silently promoted to Tier A |
| POSTPERF-03 | Havlak | pending | Pinned upstream port qualified after POSTPERF-02 foundations; retained as a separate large-graph lane |
| POSTPERF-04 | Generic final/concrete scalar accessor proof | pending | Hand-equivalent ceiling and generic proof across the expanded suite; any production candidate gets a separate mandatory first Release verdict |
| POSTPERF-05 | Bounded hoisting, register finalisation and late inlining | pending | Evidence ranks bounded consumers; select at most one production candidate at a time and apply the mandatory verdict gate |

## Standing boundaries

1. Preserve every canonical and historical benchmark. New sources and
   manifests receive distinct names and identities.
2. Correctness, provenance and semantic classification precede timing.
3. `rxvm` is the product identity; `rxtvm` and `rxbvm` are concrete controls.
   Do not duplicate an executable alias in an aggregate.
4. Reserve workloads do not enter Tier A or the common aggregate without a
   separate explicit promotion decision and versioned formal baseline.
5. Java and CPython ports are labelled language controls. NetRexx's generated
   Java/HotSpot substrate remains a Rexx-language implementation under its
   disclosed mode; it is not a direct Java source control.
6. A compiler, assembler, linker, library or VM defect exposed by a benchmark
   receives a minimal reproducer and focused regression test. Preserve the
   supported optimized shape; fail closed only when equivalence cannot be
   established mathematically.

## POSTPERF-01 — Full AWFY Json

### Frozen source and result contract

- Upstream: `smarr/are-we-fast-yet` commit
  `74306fec151070fd07157cefeacf19e7e0bcdc89`.
- Source workload: `benchmarks/SOM/Json/Json.som` and its minimal-json-derived
  parser/object model; Java source is the cross-check for zero-based string
  indexing and the harness verification boundary.
- Exact minified RAP payload: 25,820 bytes, SHA-256
  `8f84f5fdc609a6d7179089249212a39588030852719d951db2d178820b70a7d8`.
- Each benchmark iteration parses the complete payload, verifies an object
  root, verifies object member `head`, verifies array member `operations`, and
  verifies exactly 156 operations. Verification remains inside the repeated
  benchmark loop, matching AWFY's `innerBenchmarkLoop`.
- The canonical cREXX lane uses `.jsondocument`, the current supported indexed
  Level B JSON implementation. This preserves the complete input, parse and
  observable result but uses a different internal representation from AWFY's
  minimal-json object graph. Label it `standard-library-indexed-document`
  and keep it outside every cross-runtime aggregate.
- Load the immutable fixture once before the repeated kernel. File I/O is not
  repeated benchmark work; process-level observations still disclose that
  startup includes fixture loading.

### Qualification checklist

- [x] Retain the exact upstream fixture and source/hash provenance.
- [x] Add the separately named cREXX Full AWFY Json source and CMake opt/no-opt
      product.
- [x] Require deterministic byte-count, document validity, root/head/
      operations types and 156-operation result checks.
- [x] Pass optimized and unoptimized execution under `rxvm`, `rxtvm` and
      `rxbvm` where the same runtime substrate applies.
- [x] Inspect optimized RXAS; record compiler/library adaptations
      and optimizer-resistance boundary.
- [x] Take a bounded product/concrete pilot only after correctness. Do not add
      the result to the v2 common aggregate.
- [x] Retain compact evidence with exact commands, outputs, hashes and claim
      boundary.
- [x] Update benchmark, third-party, portfolio and roadmap documentation.
- [x] Run the focused Release panel, required broad Debug CTest for the changed
      library/benchmark surface, and `git diff --check`.
- [x] Close POSTPERF-01 and commit locally before opening POSTPERF-02.

### Mandatory first Release verdict — accepted

Full AWFY Json exposed a supported-shape `rxc` defect: optimized inlining of a
runtime `.string` actual into a `.binary` value formal omitted the normal
call's `stobin` conversion. A focused reproducer proved the error independently
of JSON and file I/O. The candidate preserves the inline rewrite and uses the
ordinary typed assignment emitter only for this conversion-bearing isolated
binding. Optimized/unoptimized execution and a structural `stobin` plus
inlined-`blen` contract pass.

The ordinary profiling-off Release gate is retained at
[`evidence/2026-08-17-postperf-01-awfy-json-compiler-repair-first-release-verdict`](evidence/2026-08-17-postperf-01-awfy-json-compiler-repair-first-release-verdict/README.md).
The clean `110e298af` control fails Full AWFY Json with fixture length zero;
the candidate passes the 25,820-byte and 156-operation contract. Ten existing
representative RXAS images are byte-identical. Six alternating compiler pairs
are neutral at +0.151515% candidate wall time at 0.01-second resolution. The
recommendation is to accept the correctness repair and proceed to the bounded
pilot, broad required QA, documentation closeout and local POSTPERF-01 commit.
Adrian accepted the verdict on 2026-08-17.

### Closeout result

Post-acceptance focused Debug and Release panels pass 8/8 each. Full Debug
passes 2,231/2,231 at the prescribed `--parallel 30`. The bounded 50-iteration
Release process pilot retains five recorded samples each for product `rxvm`
and distinct `rxtvm`; it is orientation only and changes no aggregate. Final
qualification evidence is at
[`evidence/2026-08-17-postperf-01-full-awfy-json-qualification`](evidence/2026-08-17-postperf-01-full-awfy-json-qualification/README.md).

## POSTPERF-02 — DeltaBlue and CD

Status: **active — no source or implementation change yet**.

Restart at the pinned upstream source/result audit for both workloads. Freeze
their object-identity, collection and deterministic verification contracts
before choosing shared cREXX foundations. Record language-required adaptations
separately, preserve distinct benchmark identities, and do not promote either
workload into Tier A as part of qualification.

## Restart rule

On restart, read this file, `performance/AGENTS.md`,
`performance/PERFORMANCE-GOVERNANCE.md`, and the current `git status`. Resume
the first unchecked item in the active stage. Do not infer completion from an
unretained pilot or from source that merely compiles.
