# Post-PERF3 performance worklist

Status: **complete — POSTPERF-01 through POSTPERF-05 closed locally**

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
| POSTPERF-04 | Generic final/concrete scalar accessor proof | complete | G1 plus four-family RXAS guard proof accepted; 266/266 verdict processes and 2,249/2,249 final Debug tests pass; accepted assembler lifecycle trade-off retained |
| POSTPERF-05 | Bounded hoisting, register finalisation and late inlining | complete | H1-T20 bounded late-profitability accepted; 2,251/2,251 final Debug tests and 56/56 focused Release tests pass; exact accepted images retained |

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

### Numbered proof plan

1. Recreate the CRI-13 concrete packed-wrapper getter/setter on the current
   compiler and retain current call/inlining/generated-code attribution.
2. Add normal `.int`/`.float` scalar-attribute controls so the result is not
   binary-, JSON- or numeric-width-specific. Compare ordinary accessors with an
   exact hand-equivalent direct ceiling.
3. Prove the semantic matrix: receiver initialization and evaluate-once
   identity, direct versus computed receiver, concrete versus interface
   dispatch, writable receiver copyback and by-value isolation, bounds and
   signals, source/TRACE identity, optimized/no-opt, source/binary imports and
   both concrete VMs.
4. Retain current `<at..i64>`/`<at..f32>` direct and wrapper forms as controls
   for generic accessor overhead. Record their current byte-addressed cost and
   generated code without redesigning binary storage, indexing or syntax in
   this stage.
5. Rank the generic production approaches from retained correctness,
   generated-code and ordinary profiling-off Release evidence. Stop before a
   production edit if no approach reaches the machine ceiling or if public
   syntax, RXBIN, ABI or architecture must change.

### Generic accessor design selection

Compare these approaches before any production compiler edit:

1. **G0 — status quo.** Preserve the ordinary resolved method call or current
   generic inliner output. This is the correctness baseline.
2. **G1 — exact accessor lowering.** For a mathematically recognized
   monomorphic scalar getter/setter, lower the already-proved receiver and
   attribute operation directly while preserving initialization, evaluate-once
   and copyback/signal/TRACE contracts. The proof must be type-generic; the
   operation may not name JSON, vectors or a numeric width.
3. **G2 — generic inliner/result finalisation.** Improve receiver/formal/result
   placement so the ordinary inliner reaches the same direct ceiling without a
   separate accessor rewrite. Prefer this if it is equally fast and its proof
   remains bounded.
4. **G3 — new public or private accessor opcode.** Retain only as a comparison
   if existing attribute/link/copy operations cannot reach the ceiling. No
   opcode is selected, and any RXBIN or architecture change requires a separate
   Adrian decision.

### Opening proof and provisional selection

The current optimized scalar-control image is 130,880 bytes (about 757 RXAS
instructions), versus 118,945 bytes (about 707 instructions) without compiler
optimization. The ordinary eight-pass inliner reaches the hot `.int` getter
and setter and the hot `.float` getter, but exhausts its traversal before the
hot `.float` setter and both packed controls. The inlined integer getter emits
`linkattr1` plus `igetunlink`; the float getter emits
`linkattr1`/`fcopy`/`unlink`; the integer setter reaches `isetattr1`; and the
float setter remains an ordinary call. This is a traversal-budget result, not
evidence that endian-aware packed access is critical.

A bounded G2 comparison raised the global pass ceiling from 8 to 32 in an
isolated compiler. It removes the later hot calls using the existing generic
machinery, but grows the image to 161,493 bytes (about 924 instructions),
continues to inline unrelated bodies and still leaves non-hot calls. That G2
form is rejected: one counter currently conflates nesting depth with progress
over independent sites.

G1 is the provisional production choice. Recognize only exact monomorphic
register-scalar methods: a getter is one final return of one receiver-owned
scalar attribute; a setter is one required by-value scalar formal assigned to
the same-typed receiver attribute followed by a void return. Boolean, integer
and float share the rule; the POSTPERF-04 verdict must cover integer and float
reads and writes together. Exact accessors receive a dedicated terminating
rewrite lane before the ordinary bounded inliner. Every successful rewrite
removes one exact accessor call and its body introduces no call, so this lane
does not need a raised global fixed-point ceiling. It deliberately reuses the
existing call-site validation, receiver capture, initialization, copyback,
signal, source/TRACE and source/binary-template machinery. Packed `i64`/`f32`,
strings, decimals, binary values, arrays, references, objects and non-exact
method bodies remain on the ordinary path.

G3 is not selected. Existing attribute/link/copy operations are sufficient to
test the generic call-removal result without changing RXAS, RXBIN, the VM ABI
or public language semantics. Any later single-instruction float-attribute
form would be a separately approved architecture candidate, not part of this
stage.

The expected machine ceiling for a getter is one proved receiver-owned scalar
load into the consumer result; for a setter it is one proved scalar store plus
only the copyback required by the language's receiver ownership. Allocation,
selector lookup, a per-element call, general recursive copy, repeated receiver
initialization or attribute search fails the ceiling.

### Qualification checklist

- [x] Retain the current CRI-13 wrapper reproduction and a current live target
      in the expanded benchmark suite.
- [x] Retain normal scalar and packed f32/i64 hand-equivalent ceilings.
- [x] Complete the receiver/dispatch/copyback/signal/TRACE/import semantic
      matrix in optimized and no-opt forms under both concrete VMs.
- [x] Retain the current packed `i64`/`f32` direct-versus-wrapper control and
      transfer its separate typed-binary design questions to `BINARY-01`.
- [x] Retain generated-code, dynamic-count where useful, exact commands, raw
      Release samples and interpretation boundaries.
- [x] Select, reject or defer G1/G2/G3 before any production edit.
- [x] If a production edit is selected, run minimum focused correctness and
      the mandatory first profiling-off Release verdict, report it to Adrian
      and stop for direction.
- [x] After an accepted verdict only, complete proportional QA, documentation
      and a local commit before POSTPERF-05.

### Packed controls and transferred `BINARY-01`

POSTPERF-04 uses the existing `<at..i64>` and `<at..f32>` byte-addressed forms
only as direct-versus-wrapper controls for the generic scalar-access proof. It
may diagnose repeated range/conversion/offset cost, but it does not change the
binary-memory language, storage invariant, instructions or runtime in this
stage.

The broader decision has transferred to central-roadmap item `BINARY-01` and
does not block POSTPERF-04 or POSTPERF-05. Its decided direction is that
`.binary` storage bases are aligned for supported native scalar payloads and a
normal native typed route is indexed in whole elements. A separate raw route
owns byte positions, fixed encodings and endian conversion.

The byte/interchange route is separate and explicit. It owns zero-based byte
positions, fixed encoded widths, selected endianness, conversion and boundary
checks for files, protocols and wire formats. Encoded `.f32` remains distinct
from native `.float`: it necessarily converts between binary32 storage and the
VM's binary64 value.

Resolve the following `BINARY-01` language decisions as one coherent decision
before a production edit:

1. **Element origin.** Select zero-based typed indexing, matching C and the
   present raw-offset origin, or one-based typed indexing, matching cREXX arrays
   and the rule that index 2 naturally denotes the second value. Element-size
   scaling is decided; the origin is not.
2. **Syntax ownership.** Compare retaining `<at..type>` for raw byte access and
   adding a native-element spelling with repurposing `<at..type>` as the normal
   native-element form and introducing a deliberately explicit raw/endian
   spelling. The second option is a source-breaking change but may produce the
   more coherent language.
3. **Migration policy.** If `<at..type>` changes meaning, decide between a hard
   pre-release break, a version/options-controlled transition, or a diagnostic
   migration period. Do not silently reinterpret existing source.
4. **Raw format expression.** The raw route must make byte position, encoded
   width and endian policy unambiguous. Exact spelling remains open; the current
   zero-based canonical-little-endian behaviour is the semantic compatibility
   control, not a commitment to retain its syntax.

Current packed-control hypothesis: `<at..i64>` sequential reads lose to normal
`.int[]` because `BGETI64` repeats byte-range validation and raw signed
conversion while the source loop advances a separate byte offset. The current
canonical little-endian contract is compile-time selected on this Mac and does
not perform a runtime endian test. POSTPERF-04 records that evidence only.

### Closure verdict

Adrian accepted G1 plus the four-family RXAS guard proof. All 266 first-verdict
processes pass; no integrated accessor median reaches the 3% adverse guard and
float writes improve by 32.75-35.41%. The final selected RXBIN is 42,518 bytes,
2.07% above G0 and below both artifact guards. Complete Debug qualification is
2,249/2,249 and the accepted Release RXBIN reproduces byte-for-byte.

Broad QA also fixed an exponential signal-policy reverse resolver with a
bounded worklist. The corrected assembler retains a formally adverse
`httpcodec` lifecycle median of +8.104172% (about 17.5 ms) versus the retained
pre-guard assembler. Adrian explicitly accepted that compile-time trade-off to
retain the runtime result and close the stage. Evidence:
[`2026-08-18-postperf-04-generic-scalar-access-first-release-verdict`](evidence/2026-08-18-postperf-04-generic-scalar-access-first-release-verdict/README.md).

## POSTPERF-05 — bounded hoisting, register finalisation and late inlining

Status: **complete; H1-T20 bounded hybrid late-profitability accepted and
qualified**.

This was executed as one significant, evidence-ranked optimizer stage rather
than a sequence of tiny edits. The expanded portfolio and retained PERF3
lessons selected a coherent bounded RXC late-profitability candidate after
considering hoisting, register finalisation and late-inlining interactions.
The implementation kept RXC and RXAS ownership explicit, preserved
signal/TRACE/debug identity, and stopped at the mandatory ordinary-Release
verdict before broad closeout.

### Opening evidence and ownership

Fresh schema-5 counts on the qualified DeltaBlue and CD reserve lanes show that
the current optimized images execute fewer bytecode instructions and calls than
their no-opt controls, but admit much more inline binding/copy and register
storage:

- DeltaBlue at bounded size 10 falls from 142,316 to 134,086 executed
  instructions and 6,138 to 2,066 calls, while value slots rise 3,970 to
  32,360 and the largest frame block rises 6,632 to 40,232 bytes;
- CD at bounded size 10 falls from 10,178,179 to 9,396,634 instructions and
  539,518 to 114,175 calls, but adds 320,294 generic copies, 193,667 float
  copies and 192,336 integer copies.

The current detached inline transaction costs the final candidate, but it uses
that cost only to prove candidate-local cleanup improved the pre-cleanup
candidate. It does not compare the final expanded site with the retained call
plus the validated callable body. This makes the qualified optimized/no-opt
reversal an inlining placement/profitability problem first. Pure register-number
compaction cannot remove live copied values, and wholesale late RXAS inlining
would change ownership before a bounded RXC proof has been exhausted.

RXC therefore owns the first candidate. Existing pre-inline constant folding
and semantic summaries provide the early/hoisting side of the decision; the
detached transaction provides the late site-specific cleanup and fallback;
register allocation occurs only after a candidate has passed the final cost
gate. RXAS remains responsible for its existing mechanical and proved flow
rewrites and gains no inlining or public-format responsibility in H1.

### Design selection

| Design | Benefit | Principal cost/risk | Disposition |
| --- | --- | --- | --- |
| H0 current early inlining | Retains all current supported inline shapes | Final expanded site is never compared with the executable call-plus-body reference; qualified reserve lanes expand badly | status-quo control |
| H1 bounded hybrid late-profitability | Retains early semantic eligibility and constant work, then compares the cleaned detached candidate with original call plus the validated callable summary before register allocation | Conservative static multi-metric model may retain calls whose dynamic context could win | **selected isolated proof** |
| H2 final register-number compaction only | Can remove numeric gaps after all rewrites | Cannot remove live inline copies or high simultaneous storage; current compiler allocation is already reuse-aware | audit as a zero/secondary result, not the first production candidate |
| H3 late RXAS inlining or two RXAS epochs | Can use exact RXAS instruction, signal and call-window facts | Requires transported semantic intent, cloning and metadata/source ownership plus an architecture selection | defer unless H1 cannot recover the qualified lanes |
| H4 disable general inlining | Establishes a useful scalar-only ceiling/control | Discards accepted Richards/List/accessor wins and is not a production policy | isolated control only |

H1's reference is the sum of the original call-site AST cost and the callable
summary body cost. The final detached candidate must be no worse in structural
nodes, assignments, branches, nested calls or inline temporaries, and must be
strictly better in at least one dimension. The ordinary call remains attached
until this proof succeeds, so a rejection is site-local rather than a
whole-callable ban. Exact scalar accessors retain their dedicated proved path;
the general gate may not undo POSTPERF-04.

The isolated comparison selected a 20-node validated-body floor. A callable at
or below that floor retains the current early inline path because the complete
body is within the bounded call/frame cost envelope; a larger callable must
pass H1's final site-specific call-plus-body comparison. The zero-floor H1
control was rejected because it lost the accepted List path and moved RexxCPS
adversely. A 40-node floor was also rejected: it recovered materially less on
DeltaBlue, CD and Richards and moved RexxCPS adversely.

The bounded five-observation/three-observation pilot is orientation, not the
mandatory Release verdict. On product `rxvm`, H1-T20 moved DeltaBlue size 500
from a 2.51 s median to 0.44 s, CD size 10 from 0.15 s to 0.08 s and Richards
size 20 from 2.13 s to 0.32 s. List RXAS was byte-identical and canonical
RexxCPS retained a 1.38 s median. H1-T40 recorded 1.89 s, 0.14 s, 1.94 s and
1.39 s respectively, so it is not the selected fallback.

### H1 execution and verdict gate

1. Compare H0, H1 and H4 on identical DeltaBlue/CD inputs and established
   inline-win/zero-change guards; audit RXAS instructions, copies, calls,
   `.locals`, image size and deterministic profile counts.
2. Add focused structural and opt/no-opt runtime regressions for an admitted
   small site and a rejected expansion-heavy site.
3. After the minimum focused checks pass, freeze H1, build the ordinary
   profiling-off Release product and run the smallest decisive end-to-end
   comparison against retained valid H0 evidence, with the accepted product as
   a same-session control where required.
4. Report and stop. Broad CTest, documentation closeout and commit follow only
   if Adrian accepts the first Release verdict.

### Accepted verdict and closeout

Adrian accepted H1-T20 after the capped paired Release comparison. Median
elapsed changes versus exact H0 were -82.3060%/-82.4190% for DeltaBlue,
-47.2985%/-47.1455% for CD, -83.9321%/-84.5985% for Richards and
-2.5006%/-5.0658% for List on `rxtvm`/`rxbvm`. Sieve
(+0.1756%/-0.0459%) and RexxCPS (+0.2316%/-0.0882%) remained
noisy/inconclusive at the approved 36-pair ceiling, with no paired median at
the 3% adverse guard.

The selected policy preserves the exact POSTPERF-04 scalar-accessor path and
ordinary validated callables of at most 20 structural nodes. Larger supported
call sites must be no worse in structural nodes, assignments, branches,
nested calls and inline temporaries, and strictly better in at least one of
those dimensions, than the executable original call plus callable-body
summary. A loss retains the ordinary call locally; it does not ban the
callable or change the supported semantic shape.

Static Release products confirm that this removes expansion rather than
moving work into the VM: DeltaBlue falls from 7,009 to 2,111 RXAS
instructions, CD 5,593 to 2,304, Richards 1,872 to 883, Havlak 2,730 to
1,428 and RexxCPS 1,403 to 865. List remains byte-identical; Sieve grows by
321 RXBIN bytes, below the 4 KiB guard. The compiler-produced library falls
from 907,207 to 719,809 bytes.

Closeout uncovered and corrected two independent RXC defects rather than
weakening the selected policy:

- binary and decimal constants returned from retained calls now keep an
  allocated register and emit `load` before `ret`; only RXAS-supported direct
  boolean/integer/float/string control operands bypass a register;
- the NR-26 copy fixed point now preserves a deeper read substitution when its
  reaching copy store was deliberately skipped, instead of replacing it on a
  later pass with an older physical-target equality. The optimized HTTP
  reassignment regression proves that `read_request` consumes the newly
  accepted client.

Structural tests were updated to accept the intentional site-local ordinary
call fallback while retaining their receiver, result, lifetime and decimal
integrity proofs. Final closeout passes 36/36 combined focused Debug tests,
the full Debug suite at 2,251/2,251, and 56/56 focused Release tests. A fresh
post-defect Release rebuild is byte-identical to all 17 frozen H1 benchmark
RXAS/RXBIN and library artefacts, so the accepted timing verdict remains
applicable. Evidence:
[`2026-08-18-postperf-05-bounded-late-profitability-first-release-verdict`](evidence/2026-08-18-postperf-05-bounded-late-profitability-first-release-verdict/README.md).

## Restart rule

On restart, read this file, `performance/AGENTS.md`,
`performance/PERFORMANCE-GOVERNANCE.md`, and the current `git status`. The
approved five-stage post-PERF3 sequence has no active item after POSTPERF-05;
new production performance work requires a separately selected roadmap gate.
Do not infer new authorization from an unretained pilot or from source that
merely compiles.
