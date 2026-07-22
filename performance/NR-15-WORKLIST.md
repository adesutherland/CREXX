# NR-15 end-to-end stem and string-access worklist

Status: complete; D2-hybrid accepted and post-verdict QA closed
Started: 2026-07-22
Branch: `develop`
Starting HEAD: `240b29f456e995928206f04285a7c319612ff022`
Starting local upstream: `origin/develop` at
`240b29f456e995928206f04285a7c319612ff022`
Starting remote upstream: `origin/develop` at
`240b29f456e995928206f04285a7c319612ff022`
Starting worktree: clean

## Objective and boundaries

Find the fastest semantically correct complete end-to-end implementation for
general Level B stem access. Treat source property/bracket and multi-tail
lowering, key construction and conversion, inlining and register allocation,
RXAS instruction selection and flow optimization, the Level B hash table,
string primitives, and bounded VM/runtime assistance as one system.

The hot path should keep an already suitable single-tail key in its existing
register, compute hash/bucket/index state once, traverse once, and move values
directly between the stem and final caller register without receiver copyback,
argument-copy, temporary-key or return-copy scaffolding.

This activity does not authorize a source-language syntax change, public ABI
change, serialized RXBIN change, canonical ISA change, weakened semantics or a
new architectural contract. Such candidates may be designed and compared as
guarded PoCs, but comparative evidence must be presented to Adrian for explicit
selection before production integration.

The dated programme report is read-only. This worklist and
`performance/ROADMAP.md` are the live control plane. Do not commit or push
unless Adrian explicitly asks.

## Existing behavior that is baseline, not new NR-15 work

- [x] `lib/rxfnsb/rexx/stem.crexx` already implements O(1) default assignment
      and reset through `default_generation` plus per-entry generations.
- [x] Default assignment increments one generation and scans no stored tail.
- [x] A stale entry returns the current default until selectively rewritten in
      the current generation.
- [x] The current representation is a fixed 256-bucket separate-chaining table
      with parallel `buckets`, `keys`, `vals`, `value_generations` and `next`
      arrays.
- [x] Current `get` and existing-key `set` compute one hash and traverse one
      bucket chain once.
- [x] Current hash is a codepoint polynomial with multiplier 31, `strlen`, one
      `strchar` per codepoint, and modulo `num_buckets` per codepoint.
- [x] The Level B `rxfnsb.stem` is distinct from
      `lib/rxfnsc/RexxStem.crexx` and Classic variable-pool/ADDRESS stem
      surfaces. Shared semantics and reusable primitives may be audited without
      conflating their representations.

## Exit criterion

- [x] Preserve generation, default, explicit-empty-key, omitted-tail, drop,
      conversion, compound-tail, iterator, reference, evaluation-order,
      source/TRACE and signal semantics.
- [x] Retain focused cREXX plus Regina/ooRexx semantic comparison evidence.
- [x] Demonstrate the fastest accepted complete algorithm on focused stem-heavy
      cases and at least one additional stem/string-heavy workload.
- [x] Prove optimized and `-n` behavior under both `rxvm` and `rxbvm` for the
      non-architectural fallback and existing product.
- [x] Retain exact static RXAS, dynamic instruction/call/allocation/string-op,
      image-size and preparation evidence separately from wall-clock evidence.
- [x] Accept no candidate without mathematical semantic justification, focused
      passing tests, strictly fewer relevant executable instructions, no image
      growth without an explicit justified trade-off, and fewer copies,
      allocations and calls or proof that they are necessary.
- [x] Run the governed ordinary profiling-off Release first verdict for the
      complete frozen production panel and stop for Adrian with a recommendation
      to accept, rework, select an architectural candidate or revert.

## Hard sequencing gates

- [x] Re-read root/performance instructions, live roadmap, governance, the
      historical NR-15 charter entry, Level B authoring, compiler/library/RXAS
      architecture and emitter architecture.
- [x] Verify exact branch, HEAD, local/remote upstream and clean worktree.
- [x] Present a numbered plan before compiler/library/RXAS/VM production edits.
- [x] Create this worklist before the first production edit.
- [x] Build the semantic matrix and reconstruct the exact post-NR-14/post-NR-18
      baseline before selecting an algorithm.
- [x] Complete and disposition the bounded A-D design/PoC panel before formal
      performance sampling.
- [x] Freeze the complete production candidate panel before its first Release
      verdict; no follow-on tuning after freeze.
- [x] After the first production edit, run only minimum focused correctness,
      immediately build ordinary profiling-off Release, run the smallest
      decisive governed comparison, report and stop.
- [x] Do not run broad Debug CTest, sanitizer, install/package, cross-platform
      sweeps, documentation polish, commit or push before Adrian accepts the
      first verdict.

## Stage 1 - semantic matrix

Unknown semantics reject only the affected transformation/site. Preserve
existing RexxDoc blocks and update them if behavior, representation, signature
or return contracts change.

| Surface | Required cases | cREXX optimized | cREXX `-n` | Regina | ooRexx | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| Missing/default | missing tail before/after default assignment | pass | pass | pass | pass | Classic uninitialized name differs before a default by design |
| Empty/omitted | explicit empty-string tail versus omitted tail/default assignment | pass | pass | scoped | scoped | Level B explicit empty key is distinct from omitted-key set |
| Insert/update | existing and newly inserted tails | pass | pass | pass | pass | direct primitive semantic control also passes |
| Generations | repeated default resets followed by selective tail writes | pass | pass | pass | pass | generation reset is baseline behavior |
| Access syntax | direct `get`/`set`, property and bracket notation | pass | pass | n/a | n/a | exact lowering retained |
| Compound tails | single/multiple tails and exact separator placement | pass | pass | pass | pass | direct-string equivalence covered |
| Conversion | integer and dynamic tail conversion | pass | pass | pass | pass | no new conversion/evaluation rewrite selected |
| Key families | ASCII, Unicode, empty, long and collision-heavy keys | pass | pass | scoped | scoped | Classic comparison covers shared behavior; Level B covers all families |
| Drop/remove | every currently defined language/library drop surface | absent | absent | pass | pass | Level B exposes no drop/remove method; Classic `DROP item.a` yields `ITEM.A` |
| Extraction | `key`, `value`, `valueAt`, `tails`, `values` | pass | pass | n/a | n/a | insertion order and generations preserved |
| Iteration | live and snapshot iterators before/after mutation/reset | pass | pass | n/a | n/a | current representation retained by fallback |
| Aliasing | receiver, object mutation and references | pass | pass | n/a | n/a | generic array inlining fails here and is rejected |
| Observability | TRACE/source-step and signal behavior | pass | pass | unchanged | unchanged | existing source metadata/TRACE and focused signal/error tests retained |
| VM modes | `rxvm` and `rxbvm` | pass | pass | n/a | n/a | exact outputs match |

Focused semantics must include at least these operation sequences:

- [x] missing read -> default set -> missing read -> explicit tail write -> read;
- [x] explicit empty key write/read versus omitted-key default assignment;
- [x] several generation resets with old, selectively refreshed and new tails;
- [x] collision-chain head/middle/tail hit, miss and existing-key update;
- [x] direct key equal to a synthetically joined multi-tail key;
- [x] preserve side-effect/evaluation ordering by retaining the existing
      property/multi-tail lowering at the architecture stop;
- [x] iterator creation before insert/default reset/selective update;
- [x] retain existing invalid index/reference/signal paths unchanged; the only
      source candidate left in the tree changes bucket hashing inside methods.

## Stage 2 - exact baseline reconstruction

### Source/compiler/inliner path

- [x] Map `compiler/rxcp_val_trans.c` property/bracket rewrite and repeated
      multi-tail `"."` concatenation, including evaluation order and types.
- [x] Map `rxcp_inline_*`, call eligibility, receiver handling, argument and
      return binding, receiver copyback, BLOCK_EXPR scaffolding and register
      assignment for `stem.get`/`stem.set`.
- [x] Explain why current imported inline metadata does not produce the desired
      direct path at representative optimized `13_stems` sites.
- [x] Compare generated optimized and `-n` RXAS with a hand-written ideal
      sequence and account for every call, copy, conversion, link/unlink,
      temporary key and result movement.

### Level B representation and string path

- [x] Record exact factory/get/set/hash instruction bodies in library RXAS.
- [x] Record hash arithmetic, character traversal, modulo and collision-chain
      instruction counts for ASCII, Unicode and long keys.
- [x] Record array attribute link/copy/unlink work per bucket/key/value/
      generation/next access.
- [x] Record allocation/value/frame and relevant string-opcode counts.
- [x] Verify default reset performs no tail scan; `get` and existing-key `set`
      perform one hash and one traversal in the actual emitted product.

### Workloads and evidence provenance

- [x] Locate and validate the exact current post-NR-14/post-NR-18 retained
      baseline artifacts before reuse; do not repeat historical percentages.
- [x] Refresh the exact current profile only where retained artifacts do not
      match the starting commit/product.
- [x] Cover focused get hit/miss, existing/new set, reset/default and multi-tail
      controls across small, medium, large and collision-heavy stems.
- [x] Cover ASCII and Unicode key families.
- [x] Cover canonical RexxCPS Level B with correctness and authored
      source-clause integrity unchanged.
- [x] Add one separately named stem/string-heavy workload; do not replace or
      modify a canonical portfolio workload merely for diagnostic convenience.
- [x] Retain optimized and `-n` RXAS instruction counts and hashes, dynamic
      instruction/call/allocation/string-op counts, linked sizes and
      preparation/startup observations.

## Stage 3 - bounded design and PoC panel

Every candidate remains isolated/default-off until selected. Target-only builds
and focused tests are the normal iteration loop.

| ID | Candidate | Required comparison | Disposition |
| --- | --- | --- | --- |
| A | Optimize the existing Level B structure while retaining generation reset | direct hot loops, reduced conversions/copies/attribute links/modulo; fixed versus load-sensitive buckets; collision costs | accepted fallback: existing `RXHASH`, fixed 256 buckets; 1,024/load-sensitive forms rejected |
| B | Compiler/inliner specialization | generic method inlining versus purpose-built lowering/intrinsic; single string tail stays in its source register; result/value use final registers; no receiver copyback | rejected: optimized semantic failures and +58.8% unlinked RXBIN |
| C | Segmented multi-tail access | repeated materialized concatenation versus streamed hash/equality over components plus separators; canonical key built once only after insertion is proved | deferred into architecture choice: prejoined control saves 13,323 instructions but current ISA cannot stream compare |
| D | RXAS/VM/string assistance | native string hash, cached immutable-string hash and narrower stem operations; eager/lazy/narrow preparation; exact machine ceiling and both VMs | architecture panel complete: D2-hybrid recommended; stop for production selection |

### Candidate A proof obligations

- [x] Keep generation-based O(1) reset and do not claim it as new work.
- [x] Compare direct method bodies with helper/inlined forms from actual RXAS.
- [x] Measure fixed 256 buckets against plausible load-sensitive sizing on
      small/medium/large/collision-heavy sets, including resize cost.
- [x] Prove one hash/one traversal for get and existing-key set.
- [x] Count every attribute link, copy, conversion, modulo and allocation.

### Candidate B proof obligations

- [x] Reject the generic PoC because it does not safely pass a single array-
      bearing receiver/key through without invalid copy/link scaffolding.
- [x] Confirm generic inlining does not safely write a get result directly to
      the caller's final destination.
- [x] Confirm generic inlining does not safely consume a set value directly
      from the caller's source register.
- [x] Preserve whole-receiver/reference semantics by retaining the existing
      array-shape exclusion; the permissive PoC fails reference mutation.
- [x] Compare generic inline AST/RXAS against the native direct ceiling; generic
      source RXAS grows 67.8% and is rejected before timing eligibility.

### Candidate C proof obligations

- [x] Preserve direct-string versus component-joined key equivalence.
- [x] Preserve separator placement, conversion and evaluation order.
- [x] Compare streamed hash/equality without key allocation on hit/miss after
      Adrian authorized the architecture contract.
- [x] On insertion, materialize the canonical key once only after the lookup
      proves it is absent.
- [x] Record the break-even point and code/image cost for single and multiple
      components in the selected D1/D2 representation.

### Candidate D proof obligations

- [x] Measure exact inline native controls for string hashing, lookup, update
      and reset as applicable before integrated opcode measurements.
- [x] Reject a general cached mutable-string hash from this panel: it needs an
      invalidation/ownership contract; retain byte-exact Unicode equality and
      make that contract explicit before any future cache.
- [x] Prefer the measured narrower purpose-built control over eager/lazy
      general string-cache preparation at this decision gate.
- [x] Include complete control setup and observed allocations in profiles; do
      not claim a production lifecycle/image result before D1/D2 integration.
- [x] Avoid search, allocation and general selector dispatch on fast-path
      success unless evidence proves it unavoidable.
- [x] Stop for Adrian before production integration because the winning design
      changes RXBIN, ISA, public ABI or architecture.

## Candidate acceptance ledger

| Candidate | Semantic proof | Focused tests | Static executable delta | Dynamic delta | Calls/copies/allocations | Image delta | Status/reason |
| --- | --- | --- | ---: | ---: | --- | ---: | --- |
| Status quo | exact starting product | pass | 0 | retained exact baseline | retained calls/copies | 0 | retained comparator |
| A | public hash surface and all stem semantics retained | 17/17 final focused plus Classic | access RXBIN -8 bytes | -35.973% to -92.541% representative non-reset cells | no added call/allocation; call scaffolding remains | library -16 bytes | accepted safe fallback |
| B | disproved in optimized mode | fail | source RXAS +67.8% | not eligible | fewer calls but invalid array/receiver semantics | RXBIN +58.8% | rejected |
| C | exact direct/compound equivalence retained by no production edit | matched controls pass | no integrated image | prejoined control -13,323 instructions (-19.8%) vs multi-tail | avoids 4,320 concats in control | n/a | architecture-dependent defer |
| D | focused primitive semantics pass both VMs; full integration not attempted | PoC pass | provisional opcodes only | further -16.094% to -66.031% vs A | removes method calls/argument/return copies on measured op | not production-integrated | choose architecture; stop required |
| D1 | exact common contract including segments, iteration, copy and move | dual-VM pass | same call-site widths as D2/D2-hybrid | fastest miss/update/reset/segmented pilots | highest growth/copy allocation cost | common control only | safe architecture comparator; not recommended if one general layout must cover growth/lifecycle |
| D2 | exact common contract including segments, iteration, copy and move | dual-VM pass | same call-site widths | fastest new insertion and deep copy; mutable update/reset/long-key losses | smallest object graph; one binary payload | common control only | retain as frozen/snapshot or copy-heavy ceiling; not recommended as sole mutable layout |
| D2-hybrid | exact common contract including segments, iteration, copy and move | dual-VM pass | same call-site widths | get-hit pilot winner; near D1 steady access; about 2.3x D1 new insertion | intermediate object graph and copy cost | common control only | **recommended mutable production selection** |

## Machine-level ceiling ledger

| Operation/key class | Existing complete path | Hand-written ideal | Native/control ceiling | Integrated candidate | Notes |
| --- | ---: | ---: | ---: | ---: | --- |
| get hit / ASCII, 16 / 1,000 | 105,696 | 16,465 | 16,465 | 48,470 (A) | D is 66.031% below A |
| get miss / ASCII / 1,000 | 168,125 | 12,418 | 12,418 | 32,423 (A) | D is 61.700% below A |
| existing set / ASCII / 500 | 64,904 | 11,917 | 11,917 | 32,254 (A) | D is 63.053% below A |
| new set / ASCII / 100 | 28,849 | 6,421 | 6,421 | 12,806 (A) | allocation-sensitive pilot noisy; exact work is lower |
| default reset / 1,000 + read | 33,225 | deferred narrow intrinsic | not required for decision | 31,471 (A) | no tail scan; call scaffolding remains |
| get / Unicode / 500 | 91,232 | 13,045 | 13,045 | 29,818 (A) | byte hash/equality semantics pass |
| collision traversal / 200 | 812,438 | 57,448 | 57,448 | 71,814 (A) | D remains chain-length dependent |
| multi-tail / 1,000 | 140,457 | 53,874 prejoined bound | architecture pending | 67,197 (A) | D2 must stream segments, materialize only on insert |

## Architecture selection stop - 2026-07-22

The complete evidence and recommendation are in
`evidence/2026-07-22-nr-15-first-release-verdict/ARCHITECTURE-DECISION.md`.
Candidate D wins the exact machine-work comparison but crosses the ISA/RXBIN
and representation boundary. Adrian's register-binary storage suggestion is
recorded as D2, alongside D1 current arrays and D2-hybrid binary metadata plus
VM value slots. The provisional opcodes are evidence only and are removed from
production source before this stop. Candidate A remains the safe fallback
production-source delta. No governed paired Release verdict can begin until
Adrian selects the production architecture.

## Approved D1/D2/D2-hybrid architecture panel - 2026-07-22

Adrian approved the bounded architecture panel. This approval authorizes
isolated design and PoC comparison; it does not select a production ISA,
serialized RXBIN contract, public ABI or final stem representation.

The three candidates share one logical operation contract:

- single-string get hit/miss;
- existing-key and new-key set;
- O(1) default reset with generation semantics;
- two-segment get hit/miss and set, hashing/comparing `left || "." || right`
  without materializing on hit/miss and materializing once after insertion is
  proved;
- byte-exact UTF-8 equality for empty, ASCII, Unicode and long keys;
- deterministic collision chains, insertion order and exact missing/default
  behavior;
- receiver, key and value alias handling, allocation-failure atomicity,
  source/TRACE/signal boundaries and both VM modes as production obligations.

Candidate layouts:

| ID | Physical representation | Ownership/lifecycle hypothesis | Status |
| --- | --- | --- | --- |
| D1 | Current fixed-bucket parallel `value.attributes[]` arrays | Existing VM-owned key/value/next/generation slots; native operation removes method/copy/link scaffolding | active comparator |
| D2 | Receiver `binary_value` contains header, bucket heads, fixed-width entry records and key/value arena | One ordinary VM-owned binary buffer; offsets survive realloc/move; updates reuse or append arena bytes | approved PoC |
| D2-hybrid | Receiver `binary_value` contains bucket/chain/hash/generation metadata; VM attributes own keys/default/values | Binary metadata improves locality while normal VM value slots retain string/reference ownership | approved PoC |

Every candidate must be measured with identical population and timed inputs.
Retain separately: executed instruction counts, operation counts, frame/value/
binary/attribute allocations and high-water bytes, linked image size, initial
construction, growth/rehash, repeated update, reset, copy/move and teardown
cost. Profiling-off Release pilots are tie-break evidence only; formal governed
sampling remains after production selection and integration.

Panel stop condition: remove every provisional opcode/VM integration edit,
retain reconstructable PoC source/evidence, update the comparative disposition,
and stop for Adrian to choose D1, D2, D2-hybrid, a revised combination, the
safe Candidate A fallback or revert.

## Architecture panel result - 2026-07-22

The panel is complete and retained under
`evidence/2026-07-22-nr-15-architecture-panel/`. The linked semantic control
and every 36-cell common smoke case pass in both VMs. Eighty-four final profiles
have valid result/allocation/call/census tracking; small ordinary-Release
pilots are tie-break evidence only.

- D1 wins most steady-state miss, existing-update, reset and segmented-access
  pilots, but 20,000 new inserts allocate 32,660,360 value-slot bytes plus
  4,203,584 attribute-pointer bytes and take about 2.3 times the D2/D2-hybrid
  elapsed time.
- D2 is the insertion, compactness and lifecycle ceiling. At 64 entries its
  exclusive profiled deep copy is 86/87 ns versus D1's 3,066/3,143 ns, but its
  packed mutable value/default path loses update, reset and long/Unicode cells.
- D2-hybrid wins the get-hit Release pilot, stays closer to D1 on the remaining
  access cells, removes most of the D1 insertion penalty and halves much of the
  parallel-array object graph. It is the recommended single mutable layout.

All candidates prove allocation-free two-segment hit/miss, one canonical key
construction on insert, insertion-order extraction, generation-aware values,
deep-copy isolation and ordinary VM move transfer. Call-site RXBIN widths are
identical. The current profiler does not expose deallocation counters, so
failure-injection/destructor/sanitizer proof remains a production obligation.

The provisional opcodes and VM includes are removed before this stop;
`provisional-integration.patch`, helper/handler source and raw evidence retain
the reconstruction. Adrian must now select D2-hybrid (recommended), D1, D2, a
revised combination, Candidate A fallback or revert before Stage 4.

## Stage 4 - frozen production panel and first Release verdict

Adrian selected **D2-hybrid** for production implementation on 2026-07-22.
The production form is the panel's private version-1 receiver layout: a
fixed-width little-endian header, 256 bucket heads and 16-byte
hash/next/generation records in the receiver binary; ordinary VM attributes
own the insertion-ordered keys, values and current default. The serialized
RXBIN change is the canonical instruction family and feature declaration, not
serialization of the receiver's ephemeral hash-table bytes.

Direct compiler lowering is limited to a proved concrete `rxfnsb.stem`
receiver in simple storage. Complex/computed receiver shapes retain the normal
method path, whose bodies use the same native operations. This preserves
receiver copyback and reference behavior where a direct mutation has not been
proved. Single materialized keys are the mandatory first production path;
two-segment streaming is enabled only where conversion, evaluation order and
TRACE observation remain exact, otherwise the existing canonical-key
construction remains the fallback.

Rejected/deferred alternatives remain as recorded above: D1 loses insertion,
copy and object-graph cost; fully packed D2 loses mutable update/reset and
long/Unicode access; Candidate A is the non-architectural rollback; a frozen
packed D2 snapshot remains separate future work.

- [x] Record the complete selected production algorithm and why every rejected
      or deferred alternative lost.
- [x] Record the exact compatibility, ownership, invalidation, exception,
      source/TRACE and dual-VM contract.
- [x] Freeze implementation after the complete selected panel is present.
- [x] Run only minimum focused Debug/Release correctness needed for safe timing.
- [x] Build the ordinary profiling-off Release product.
- [x] Audit and reuse retained valid baseline evidence; add a same-session
      accepted-product drift control where governance requires it.
- [x] Run at least one warmup and 12 paired balanced/interleaved recorded rounds
      per decisive before/after cell; apply governed appends if uncertainty or
      a regression guard requires them.
- [x] Report `rxvm` and `rxbvm` separately.
- [x] Keep correctness, static reduction, dynamic work, benchmark-native rate,
      process elapsed, lifecycle, RSS/allocation and artifact size separate.
- [x] Update this worklist and `performance/ROADMAP.md` with exact evidence and
      recommendation.
- [x] Stop for Adrian before broad closeout, cleanup, further PoCs,
      documentation polish, commit or push.

## First-verdict evidence bundle checklist

- [x] source branch/commit and exact dirty scope;
- [x] host/OS/CPU/power/thermal/load and build/toolchain options;
- [x] exact commands, order, warmups, samples and serial policy;
- [x] semantic matrix and all raw correctness outputs;
- [x] exact before/after optimized and `-n` RXAS plus hashes;
- [x] copy/allocation/call/string-op ledger;
- [x] machine-level ceiling comparison;
- [x] dynamic instruction/procedure/allocation profiles;
- [x] canonical RexxCPS plus independent stem/string-heavy workload;
- [x] linked artifact size and preparation/startup observations;
- [x] raw paired ordinary Release samples and uncertainty summary;
- [x] recursive checksums and concise interpretation/recommendation.

## D2-hybrid first Release verdict - 2026-07-22

The frozen implementation passes focused Debug 22/22, focused Release 10/10
and opcode metadata. Formal profiling-off Release timing uses one warmup plus
12 balanced/interleaved pairs per cell against the retained Candidate A linked
images. Every result is correct and every paired mean 95% interval is wholly
favorable:

| Cell | `rxvm` paired median | `rxbvm` paired median |
| --- | ---: | ---: |
| 60,000,000 get hits, 64 entries | -76.839% elapsed | -75.438% elapsed |
| independent 400,000-pass histogram | -32.027% elapsed | -31.885% elapsed |
| canonical RexxCPS 2.2d | +10.888% CPS | +10.919% CPS |

Integrated schema-4 profiles reduce get-hit instructions 53,874 to 15,396
(-71.422%), histogram 1,851,581 to 1,117,409 (-39.651%), and canonical
RexxCPS 317,993,134 to 251,052,811 (-21.051%). Direct bytecode calls fall
1,067 to three, 21,071 to 1,004, and 8,663,479 to 6,583,479 respectively.

The process-inclusive load/construction/first-access diagnostic is neutral to
favorable and hits no lifecycle guard; its required absolute-noise append is
retained. Canonical peak RSS medians fall 81,920/98,304 bytes. The optimized
access image, canonical RexxCPS and shared library shrink 1,200, 1,192 and
1,528 bytes. No throughput, lifecycle, RSS or artifact guard is hit.

Evidence:
`evidence/2026-07-22-nr-15-first-release-verdict/production-d2h/`.
Recommendation: accept D2-hybrid and authorize the shortest post-verdict
quality closeout. The implementation remains provisional and uncommitted; no
broad CTest, sanitizer/failure injection, install/package, cleanup, commit or
push has run before this stop.

## Accepted post-verdict QA closeout - 2026-07-22

Adrian accepted D2-hybrid and explicitly asked for all QA through a local
commit. The closeout retained the frozen first-verdict timing bundle unchanged
and added a separate `qa-closeout/` evidence directory.

- [x] Add failure-atomic native-stem attribute/string growth without changing
      historical allocation behavior for unrelated VM callers.
- [x] Prove binary preparation/reserve failure, attribute/string allocation
      failure, insertion/update/reset/get atomicity, corruption/cycles,
      generation/capacity/key-length overflow, extraction bounds, segmented
      keys, aliases, deep copy, move, and destruction.
- [x] Add the permanent RXBIN feature/round-trip contract and move the NR-14
      and NR-21 deliberate unknown-feature checks to the next unsupported bit.
- [x] Document all nine native-stem mnemonics and reconcile the live surface at
      384 unique mnemonics, 591 forms, and 384 unique reference headings.
- [x] Preserve all 46 `rxfnsb.stem` RexxDoc blocks plus relevant tags.
- [x] Pass the 392-step complete Debug rebuild, focused Debug 30/30, and full
      Debug CTest 1,901/1,901 in 149.21 seconds.
- [x] Pass the complete supported Apple ASan build and focused 30/30. Apple
      rejects leak detection, so LSan is explicitly not claimed.
- [x] Pass the ordinary profiling-off Release full build and focused 30/30.
- [x] Install 133 files into a fresh prefix; prove installed native packaging,
      installed compiler/assembler/linker execution, both installed VMs, and
      exact retained pre-native-stem RXBIN compatibility.
- [x] Run an accepted-vs-final same-image drift guard. Combined paired medians
      are +0.599% `rxvm` get-hit elapsed, -2.439% `rxbvm` get-hit elapsed,
      +0.789%/+1.086% canonical RexxCPS, and -2.088%/-0.448% lifecycle elapsed;
      no 3% throughput or lifecycle guard is hit.
- [x] Retain exact logs, raw samples, runner manifests, checksums, and concise
      interpretation under
      `evidence/2026-07-22-nr-15-first-release-verdict/qa-closeout/`.

No push is authorized. The requested delivery artifact is one local NR-15
commit on `develop`.
