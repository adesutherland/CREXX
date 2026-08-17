# Post-PERF3 performance worklist

Status: **active — POSTPERF-04 generic scalar-access proof**

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
| POSTPERF-02 | DeltaBlue and CD | complete | Both pinned upstream ports qualified with stable indexed identity/container adaptations; neither is promoted to Tier A or any aggregate |
| POSTPERF-03 | Havlak | complete | Pinned upstream port qualified after POSTPERF-02 foundations; retained as a separate large-graph lane |
| POSTPERF-04 | Generic final/concrete scalar accessor proof | active | Hand-equivalent ceiling and generic proof across the expanded suite; any production candidate gets a separate mandatory first Release verdict |
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

Status: **complete — DeltaBlue and CD qualified as separate reserve lanes**.

### Frozen source and result contracts

- Upstream: `smarr/are-we-fast-yet` commit
  `74306fec151070fd07157cefeacf19e7e0bcdc89`.
- DeltaBlue source: `benchmarks/Java/src/DeltaBlue.java`,
  `benchmarks/Java/src/deltablue/*.java` and the corresponding
  `benchmarks/SOM/DeltaBlue/*.som` implementation.
- DeltaBlue `innerBenchmarkLoop(n)` runs both `chainTest(n)` and
  `projectionTest(n)`. The chain test propagates 100 successive edit values
  through `n` equality constraints and checks the last variable each time.
  The projection test checks source-to-destination propagation at 17/1170,
  destination-to-source propagation at 1050/5, then every retained destination
  after changing scale to 5 and offset to 2000. Assertion success, rather than
  an aggregate checksum, is the upstream result contract.
- CD source: `benchmarks/Java/src/CD.java`, `benchmarks/Java/src/cd/*.java`
  and the corresponding `benchmarks/SOM/CD/*.som` implementation.
- CD runs exactly 200 frames at time `frame / 10.0`. Its accepted collision
  totals are 42 for 2 aircraft, 390 for 10, 4,305 for 100, 8,655 for 200,
  10,830 for 250 and 14,484 for both 500 and 1,000 aircraft.

### Frozen Level B adaptations

- DeltaBlue lowers the upstream constraint class hierarchy to one tagged
  concrete constraint record. Stable integer handles into planner-owned typed
  arrays preserve variable/constraint graph identity despite Level B's
  by-value object assignments. Variable adjacency lists store those handles;
  they do not treat copied `.object` values as identity-bearing references.
- CD retains a benchmark-private indexed red/black tree derived from the
  upstream algorithm. This avoids silently replacing AWFY's tree with the
  class-library AVL implementation and gives persistent state and voxel maps
  stable node/value handles.
- CD vector and motion records remain value objects. The existing `rxmath`
  plugin is the disclosed native substrate for `sin`, `cos` and `sqrt`, matching
  the already-labelled NBody math boundary.
- Both remain reserve workloads outside Tier A and every common aggregate.

### DeltaBlue mandatory first Release verdict — accepted

DeltaBlue exposed a supported-shape `rxc` defect independently reduced to an
indexed class-attribute assignment whose right-hand factory was inlined as a
`BLOCK_EXPR`. The target destination and owner registers are linked before the
right-hand block executes, but the block's internal statement boundary
returned all deferred registers. The inlined factory then reused the live owner
register and produced a cyclic/corrupt aggregate that crashed every VM while
copying it.

The candidate preserves the inline factory and the indexed assignment. It
returns the right-hand statement's own deferred temporaries while retaining
only the linked assignment target and helper registers until the outer
assignment emits `unlinkn`. The focused structural regression rejects an owner
write between `minlinkattr1` and its matching `unlinkn`.

The ordinary profiling-off Release gate is retained at
[`evidence/2026-08-17-postperf-02-deltablue-register-lifetime-first-release-verdict`](evidence/2026-08-17-postperf-02-deltablue-register-lifetime-first-release-verdict/README.md).
The clean `00188b13c` control exits 139 for both the focused reproducer and
DeltaBlue; the candidate passes optimized and unoptimized forms under `rxvm`,
`rxtvm` and `rxbvm` (12/12). Ten established workload/tool RXAS images are
byte-identical. Six alternating compiler pairs have identical 1.086667-second
means at 0.01-second timer resolution. The recommendation is to accept this
correctness repair, then complete proportional QA and DeltaBlue qualification
before starting CD.
Adrian accepted the verdict on 2026-08-17.

### DeltaBlue closeout result

Focused Debug and ordinary profiling-off Release selections pass 8/8 each.
The finalized optimized/unoptimized by `rxvm`/`rxtvm`/`rxbvm` matrix passes
6/6, explicit maintained-runner checks pass 4/4, and full Debug passes
2,236/2,236 in 311.69 seconds. Final qualification evidence is at
[`evidence/2026-08-17-postperf-02-awfy-deltablue-qualification`](evidence/2026-08-17-postperf-02-awfy-deltablue-qualification/README.md).

The bounded size-500 process pilot records a material current optimizer
boundary: optimized source RXAS is 4.250319x the no-opt instruction count, and
optimized median process time is 18.323543x no-opt for product `rxvm` and
16.810123x for `rxtvm`. The result is retained for the later generic scalar-
access and bounded late-inlining/register-finalisation stages. DeltaBlue
remains a reserve lane and changes no aggregate.

### CD closeout result

The exact 200-frame CD port preserves value records, moving-point collision
maths, recursive voxel reduction and benchmark-private indexed red/black
trees. All seven published totals pass in no-opt; optimized executions pass at
2, 10 and 100 aircraft. The bounded Debug/Release optimized/unoptimized
three-VM matrix passes 12/12, focused Debug and Release CTest pass 3/3 each,
and the explicit maintained runner passes all four pilot cells. Qualification
evidence is at
[`evidence/2026-08-17-postperf-02-awfy-cd-qualification`](evidence/2026-08-17-postperf-02-awfy-cd-qualification/README.md).

CD independently confirms the optimizer boundary: optimized source RXAS is
2.868166x the no-opt instruction count, and optimized median process time is
5.155663x no-opt for product `rxvm` and 5.414906x for `rxtvm` at bounded size
10. A passed stripped-linked size-100 orientation is 33.96 seconds optimized
versus 0.43 seconds no-opt. The source and contract remain unchanged; these
results transfer to generic scalar-access and bounded late-inlining/register-
finalisation. POSTPERF-02 changes no aggregate.

### Qualification checklist

- [x] Freeze pinned Java/SOM source and exact deterministic result contracts.
- [x] Audit Level B object, collection, identity and native-math boundaries.
- [x] Freeze the stable-handle DeltaBlue and indexed-red/black-tree CD design.
- [x] Implement and qualify DeltaBlue opt/no-opt smoke execution.
- [x] Implement and qualify CD opt/no-opt smoke execution.
- [x] Pass both products under `rxvm`, `rxtvm` and `rxbvm` in Debug and
      profiling-off Release.
- [x] Retain source/RXAS/RXBIN hashes and structural/adaptation observations.
- [x] Take a bounded product/concrete pilot only after correctness; add neither
      workload to the v2 common aggregate.
- [x] Update benchmark, third-party, portfolio and roadmap documentation.
- [x] Run focused Release QA, required broad Debug CTest and `git diff --check`.
- [x] Close POSTPERF-02 and commit locally before opening POSTPERF-03.

## POSTPERF-03 — Havlak

Status: **complete — qualified as a separate stable-indexed CFG reserve lane**.

### Frozen source and result contract

- Upstream: `smarr/are-we-fast-yet` commit
  `74306fec151070fd07157cefeacf19e7e0bcdc89`.
- Preserve `LoopTesterApp.main(innerIterations, 50, 10, 10, 5)`: the simple
  CFG, dummy recognitions into the persistent loop graph, 5,213-node large
  CFG, one persistent recognition, 50 recognitions into fresh discarded loop
  graphs and final nesting calculation.
- Preserve the published loop/node pairs: 1 -> 1,605/5,213; 15 ->
  1,647/5,213; 150 -> 2,052/5,213; 1,500 -> 6,102/5,213; and 15,000 ->
  46,602/5,213.
- The direct benchmark defaults to 50 discarded recognitions. A disclosed
  second argument controls only that count: CTest uses `1 0`; the maintained
  process-smoke runner uses `1 1`. Neither bounded form replaces the exact
  upstream timing workload.

### Representation and fastest-feature audit

- Stable integer handles lower Java identity for CFG blocks, loop records and
  union-find nodes. Ordered vectors preserve graph/work-list order and
  insertion-ordered sets preserve uniqueness.
- Normal `.int[]` is a typed array of VM values, not packed integer storage.
  The selected 64-bit product uses 176-byte `value` records plus attribute
  indirection; retain this explicitly as the normal-value representation lane.
- Packed `.binary` with `<at..i64>` is a distinct storage cost model. The
  current optimized 200,000-cell control records array read/write at
  2,523/5,256 us and packed i64 read/write at 3,614/3,492 us. Packing reduces
  footprint and improves writes but loses this read-heavy sequential control,
  so it does not silently replace the equivalence lane.
- Loop-parent and loop-body insertions are mathematically unique: every loop
  receives one parent and the Havlak node pool contains each body handle once.
  Those internal set insertions may append without a redundant duplicate scan;
  generic non-back-predecessor sets retain membership checks.

### Qualification checklist

- [x] Retain pinned Apache-2.0 provenance and source notice.
- [x] Preserve the CFG, loop finder, union-find and loop-forest algorithms with
      stable-handle representation adaptations documented.
- [x] Build optimized and unoptimized forms in Debug and profiling-off Release.
- [x] Pass the bounded opt/no-opt product/concrete six-cell Release matrix at
      1,605 loops and 5,213 nodes.
- [x] Exercise one complete discarded-loop-forest pass in optimized and
      unoptimized product cells.
- [x] Complete and retain the full published reference-result qualification:
      execute 1, 15, 150 and 1,500; retain the exact `1,602 + 3n`
      construction proof and disclose that 15,000 was not an executed pass.
- [x] Retain source/RXAS/RXBIN hashes, generated-code observation, exact
      commands, bounded runner output and interpretation boundary.
- [x] Run focused Debug/Release CTest, full Debug CTest and `git diff --check`.
- [x] Close and commit POSTPERF-03 locally before opening POSTPERF-04.

### Closeout result

The final-source Release opt/no-opt by `rxvm`/`rxtvm`/`rxbvm` matrix passes
6/6, the maintained runner passes both one-discarded-recognition product cells,
and the published 1, 15, 150 and 1,500 results execute exactly. Two deliberately
bounded 15,000 attempts were interrupted at 916.56 and 1,567.48 seconds, so
46,602/5,213 is retained through the audited `1,602 + 3n` construction and is
not called an executed pass. Focused Debug and profiling-off Release CTest pass
3/3 each; full Debug passes 2,240/2,240 in 370.34 seconds.

Optimized source RXAS is 2.243313x the no-opt instruction count, with aggregate
locals rising from 442 to 678 and stripped-linked instructions from 1,160 to
2,536. The single runner samples are integration evidence only. Final evidence
is at
[`evidence/2026-08-17-postperf-03-awfy-havlak-qualification`](evidence/2026-08-17-postperf-03-awfy-havlak-qualification/README.md).

POSTPERF-03 changes no aggregate and makes no production compiler, RXAS or VM
edit. POSTPERF-04 now owns the generic scalar proof and aligned `rxinteger`
decision input.

## POSTPERF-04 — generic scalar access and packed-read controls

The generic final/concrete scalar-access proof remains the stage owner. Havlak
adds the following bounded decision input; it is not production authorization.

### `POSTPERF-04-PACKED-01` — aligned native `rxinteger` read

Hypothesis: current `<at..i64>` sequential reads lose to normal `.int[]`
because `BGETI64` repeats byte-range validation and raw signed conversion while
the source loop also advances a separate byte offset. The canonical
little-endian binary contract is compile-time selected on the current Mac; it
does not perform a runtime endian test.

Compare these forms before selecting any production edit:

1. Status quo `BGETI64` with canonical little-endian, range and signed
   conversion contracts.
2. Compile-time native signed-64 conversion when `rxinteger` is proved
   8-byte two's-complement, retaining ordinary range validation.
3. Compiler-selected aligned-native access when buffer-base alignment,
   `offset mod 8 = 0` and host representation are proved. Treat bounds proof
   and loop hoisting as a separate, composable fact.
4. An explicitly selectable aligned `rxinteger` accessor or view. Check base
   alignment, total extent, element bounds and compatible signed-64 host
   representation once, then admit direct indexed reads/writes without
   per-element endian or signed conversion. Its backing store may be `.binary`
   or a future packed-integer container; the proof must not assume the storage
   decision. Public syntax remains open.
5. An unchecked public native-endian/native-width array is not selected. It
   would require a separate language/safety decision.

The machine-level ceiling is one proved aligned 64-bit load into the result
register plus loop arithmetic; the C implementation must remain alias-safe
(for example a `memcpy` form that the compiler lowers to that load). Preserve
the existing portable fallback and signal behavior for every unproved site.
Adrian selected the aligned `rxinteger` path for inclusion in the proof on
2026-08-17, but not its syntax or implementation. Any production candidate
must first pass the mandatory focused correctness and ordinary profiling-off
Release verdict; canonical RXBIN, public ABI and source semantics remain
unchanged unless Adrian separately approves an architecture decision.

## Restart rule

On restart, read this file, `performance/AGENTS.md`,
`performance/PERFORMANCE-GOVERNANCE.md`, and the current `git status`. Resume
the first unchecked item in the active stage. Do not infer completion from an
unretained pilot or from source that merely compiles.
