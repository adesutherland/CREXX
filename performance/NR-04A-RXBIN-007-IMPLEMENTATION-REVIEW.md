# NR-04A RXBIN 007 implementation review and repair options

Status: complete — the rejected milestone-1 shape was replaced by C3-style
process views, process-wide callable/factory bindings, site caches, and selected
complete-section compression; correctness and performance gates pass

Date: 2026-07-16

## Verdict

The T6 semantic objective remains valid: RXAS and RXLINK need one coherent view
of types, classes, interfaces, dynamically selected members, factories, and
portable procedure targets, while RXVM needs a process-bound view of the same
relationships. The milestone-1 physical and runtime implementation was not an
acceptable performance implementation of that objective. The directed R1
runtime repair now meets the focused hot-path and integrated performance gates.
The selected complete-section compression repair removes the gross serialized-
size defect without changing the graph representation.

The review found two substantially independent milestone-1 failures:

1. The hot runtime representation does not materialize assignability or bind
   portable procedure references. Successful type support allocates and walks
   the graph; method/factory resolution repeatedly scans VM modules and
   procedures. These operations should be inline equality, bit tests, and
   direct target loads.
2. RXBIN 007 stores the formerly compressed constant/metadata pool expanded,
   then adds graph facts and indexes containing semantic text and callable
   records already represented elsewhere. Loss of base-pool compression is the
   largest cause of whole-file growth; over-broad graph contents and duplicate
   representation are separate material costs.

The first failure is repaired by the selected compact runtime type descriptor,
precomputed assignability, one-time callable/factory binding, and bounded site
caches. Adrian selected the closest portable repair: preserve the broad,
complete, re-linkable six-section seed and compress each finished section
through shared `rxbin`. Compact or split graph seeds remain measured optional
refinements rather than a prerequisite for the selected repair.

## Directed C3-style/R1 comparator outcome

Adrian selected the closest repair to the existing integration: retain the
frozen six-section 007 seed and current producers/consumers, replace the
process-local `rxbin` build/access shape, wire it into the current VM, take the
Release baseline immediately, and only then consider VM-specific target
binding. This is C3-style descriptor ownership applied as the R1 rescue
comparator; it does not yet implement the compact serialized C3 seed described
below.

The candidate materializes dense assignability, `uint32_t` dispatch/factory
views, provider ranges and immutable `RxGraphTypeRef` descriptors after graph
build or deserialization. Object values store one descriptor pointer. On the
retained real images, descriptor support/dispatch is 0.92-1.07 ns and
`sizeof(value)` is 248 bytes. The first integrated `rxvm` interface result is
within +5.3% retained/+7.0% stripped of exact pre-007 evidence, while canonical
RexxCPS is +31.7% versus its retained clean O3 comparator. The remaining method
delta is +9.9%/+13.2%; `runtime_graph_procedure()` still scans modules and
procedures for every selected callable by design in this first comparator.

The dense views raise retained graph memory to 50,703 bytes for the interface
image and 150,048 bytes for RexxCPS, but leave the already oversized serialized
007 images unchanged. Full raw evidence and the stop boundary are in
`evidence/2026-07-16-nr-04a-c3-candidate/`.

## Directed callable binding, cache and factory-remap outcome

Adrian selected the closest continuation of the repaired implementation:
retain the `rxbin` process-local descriptor/views, add one dense
`CallableId -> proc_runtime *` table per graph, materialize already-bound
factory/provider rows, and add VM-owned generation-guarded site caches. The
portable graph remains free of process pointers. `runtime_graph_procedure()`
did not exist in 006; it was introduced by milestone 1. It is now retained only
as the cold `runtime_graph_procedure_unbound()` rebuild helper rather than
running on every selection.

The method cache is two-way and keyed by the immutable type descriptor. Factory
sites cache their bound bucket and use a direct target when the bucket has one
provider and no user `match`; argument-sensitive multi-provider buckets still
run every required `match` and preserve score/tie-break semantics. A semantic
generation increment invalidates both cache shapes after link/late load.

The first bound-factory result exposed a separate producer defect. An
instruction descriptor returned source-short type `.lookup24`, while the
interface declaration used canonical type
`.runtime_interface_lookup_compare..lookup24`. The builder made a duplicate
factory member and the linker encoded both real factory-selection sites with
the duplicate providerless bucket. The VM consequently missed its graph path
and ran the legacy descriptor resolver every time. The graph builder now
matches those return-type spellings semantically and operand resolution prefers
the provider-backed factory. The rebuilt linked graph has 60 types, 42 members,
24 factories and 24 providers, versus 61/43/25/24 before the fix.

This also exposed a harness-contract gap. The original `rxgraph_bench` selected
a known-valid graph factory and measured the bucket/provider primitive, so its
approximately 1-ns result was correct but did not validate executable operand
remapping. The enhanced harness audits every graph-bearing instruction operand.
It reports two factory sites and two providerless references in the defective
image, and the same two sites with zero providerless references after rebuild.
Providerless sites are reported rather than rejected generically because a
late/native provider may be legal; the fixed linked-image test contract decides
whether zero is required.

The authoritative final profiling-off Release medians are:

| VM / image | Process | Method | Factory region |
| --- | ---: | ---: | ---: |
| `rxvm`, retained | 48.965 ms | 37,044 us | 7,905 us |
| `rxvm`, stripped | 50.106 ms | 38,003 us | 8,059 us |
| `rxbvm`, retained | 48.560 ms | 36,727 us | 7,895 us |
| `rxbvm`, stripped | 48.004 ms | 36,438 us | 7,917 us |

Against exact pre-007 `rxvm`, the retained process/method/factory-region changes
are -76.05%/-18.75%/-94.83%; stripped changes are
-70.54%/-14.58%/-93.38%. The factory region contains selection, construction,
factory execution and a subsequent method, so it is an end-to-end upper bound,
not an isolated selector. Instrumented attribution puts `SRCMETHODSEL` and
`SRCFPROCSEL` at about 14 ns each; 006 recorded 21 ns and 448 ns respectively.
The final canonical RexxCPS smoke is 1,132,602 CPS, and focused Debug validation
passes 82/82. Full evidence is in
`evidence/2026-07-16-nr-04a-bound-cache/`.

The runtime design therefore passes this focused gate. It does not repair file
size: current retained/stripped interface images are 126,884/105,572 bytes,
4.474x/4.638x their 006 forms. Removing the duplicate factory saved only 140
bytes. Canonical-pool compression and semantic seed/scope remain the next
independent design decision.

## Meaning of effectively zero nanoseconds

No operation takes literally zero time. For `ISTYPE` and successful
`ASSERTTYPE`, the production requirement is that type checking has no
separately meaningful service cost beyond the opcode's register/result work:

- exact class: one pointer or dense-ID equality;
- interface/ancestor: one precomputed word load and bit test;
- no allocation, graph traversal, name conversion, hash/binary search,
  portable-reference resolution, or out-of-line function call;
- failure-only name and diagnostic construction remains cold; and
- the isolated result must be statistically indistinguishable from the same
  inline equality/bit-test control, with compiler output inspected to confirm
  the intended loads and branch.

Method resolution has a similar contract: an already numeric member slot must
produce an already-bound target using an indexed load, not a graph search plus
module/procedure scan. Factory bucket access must be direct; invoking required
user `match` functions remains real language work and is not part of the
structural-lookup target.

## Measurement evidence

### End-to-end first Release gate

The focused benchmark runs 1,000,000 method selections and 100,000 factory
selections. Its retained medians expose the size of the added work:

| Timed region | Pre-007 | Current 007 | Added per iteration |
| --- | ---: | ---: | ---: |
| method | 45,591 us | 89,251 us | 43.660 ns |
| factory loop | 152,985 us | 286,454 us | 1.335 us |

The factory loop also constructs objects, calls the selected factory and then
calls a method, so it is not an isolated factory lookup measurement. The
whole-process retained result regressed 81.7%; the stripped result regressed
76.9%. Canonical RexxCPS was 14.0% below the retained older canonical result,
with the documented commit-distance caveat.

### Isolated production graph primitives

The disposable `rxgraph_bench` target links only the production `rxbin`
library. It loads real RXAS/RXLINK images and measures graph APIs without
building or running CREXX. The following are serial, 1,000,000-iteration,
seven-sample medians from the retained early-gate images:

| Primitive | Interface image | RexxCPS image | Assessment |
| --- | ---: | ---: | --- |
| loop control | 2.164 ns | 1.066 ns | measurement control |
| exact type support | 1.179 ns | 0.936 ns | near the required equality cost |
| positive transitive support | 32.316 ns | 37.572 ns | rejected |
| negative transitive support | 48.952 ns | 72.811 ns | rejected |
| precomputed assignability bit | 0.846 ns | 0.976 ns | PoC: effectively control cost |
| numeric dispatch to callable ID | 3.073 ns | 4.003 ns | useful comparator, not final target |
| dispatch plus portable `ProcRef` view | 3.898 ns | 4.894 ns | still not a bound VM target |
| binary-search dispatch plus bound array | 2.939 ns | 3.922 ns | PoC: binding alone is insufficient |
| direct bound-target load | 0.936 ns | 0.937 ns | PoC: effectively control cost |
| factory bucket | 4.090 ns | 4.330 ns | direct structural cost is small |
| first provider | 3.210 ns | 4.473 ns | direct structural cost is small |

The bitset and direct-target rows are deliberately small scratch layouts, not
production implementations. They prove that the required success paths can be
reduced to effectively control-cost operations. The isolated graph does not
include the production `runtime_graph_procedure()` module/procedure scans.
Consequently, it proves that those scans and the relationship walk—not the mere
existence of numeric IDs—are the dominant design defects to remove.

### Graph population and retained size

| Measurement | Interface image | RexxCPS image |
| --- | ---: | ---: |
| types | 61 | 57 |
| relationships | 24 | 20 |
| members | 43 | 145 |
| callables | 72 | 323 |
| dispatch rows | 41 | 188 |
| factories/providers | 25 / 24 | 2 / 6 |
| serialized graph facts/indexes | 21,591 / 4,576 B | 69,776 / 9,548 B |
| retained graph heap/allocations | 27,023 B / 20 | 81,576 B / 20 |
| graph-local string bytes | 12,763 B | 46,720 B |
| internally unique string bytes | 12,660 B | 43,890 B |

Interning graph strings only against other graph strings would save 103 bytes
for the interface image and 2,830 bytes for RexxCPS. The useful saving is
therefore not a more elaborate graph-local interner; it is avoiding a second
copy of semantic text already represented by canonical constant/metadata
records.

The 323 RexxCPS callable nodes are a scope warning. The builder currently adds
every `META_FUNC`, even though ordinary position-addressed procedures do not
need a runtime semantic node. Runtime graph population must be restricted to
entities genuinely used by dynamic selection, relationship queries, factory
selection, late binding, or an explicitly supported reflective/name surface.

## Milestone-1 root-cause audit (historical)

### Hot relationship and dispatch paths

- The original `rx_graph_type_supports()` allocated a `seen` bitmap and queue on
  every call, then breadth-first traversed implements, class-inheritance,
  interface-extension, and alias edges. This explained the 35-103 ns isolated
  relationship costs. The process-local descriptor now reaches a precomputed
  bitset through an inline accessor.
- Milestone-1 `runtime_graph_procedure()` converted a portable callable
  reference by scanning every context module for the graph/module pair and then
  scanning that module's procedures for the instruction offset on every method
  or provider selection. The current VM runs the equivalent work only during a
  semantic-generation binding rebuild.
- Milestone-1 `resolve_runtime_graph_factory()` repeated that procedure scan for
  every factory and optional match callable in the provider bucket. Current
  factory/provider rows contain bound targets.
- Milestone-1 `rx_graph_dispatch()` performed a binary search and returned a
  callable ID. The descriptor now supplies a dense inline callable-ID load and
  the VM indexes its bound target array.
- Cross-graph compatibility falls back to canonical names. That is acceptable
  as a temporary cold compatibility path, not as the identity model for a
  context containing late/native definitions.

### Unrelated hot-path footprint

The milestone-1 T6 commit added `object_type_graph` and `object_type_id` to every
`value`. On this host `sizeof(value)` grew from 256 to 272 bytes. `value_zero()`,
copy, and move also gained graph/ID stores. Every VM register/value allocation
and many generic operations paid that layout and cache cost even when no class
or interface feature was used.

The selected repair replaced that collection with one canonical runtime-type
pointer. The pointed-to descriptor provides the name and length for `TYPEOF`
and diagnostics and the hot relationship/dispatch data. It also makes exact
type identity pointer equality. Current `sizeof(value)` is 248 bytes, below the
006 size of 256 bytes.

### Build, serialization, and load work

- Builder type/member/callable discovery is substantially linear; completion
  contains nested relationship/declaration/dispatch loops; provider sorting is
  insertion based. These are RXAS/RXLINK costs, not steady-state opcode costs,
  but they impede large-image scaling and fast experimentation.
- `rx_graph_serialize_sections()` creates a combined graph image, allocates a
  second facts buffer, copies facts, and creates a new index envelope.
- `rx_graph_deserialize_sections()` allocates and joins the two sections before
  the normal deserializer allocates the retained arrays.
- the 007 reader first allocates and reads/copies the entire container, then
  materializes native pools and graph storage before freeing the image.

None of these lifecycle costs justifies a slow opcode path. They should be
measured separately while selecting compact-seed versus direct-table storage.

## Why the linked images grew

The prior 006 image compressed its combined constant/metadata pool. The frozen
007 base stores constants and metadata uncompressed, then adds graph sections.

| Image | 006 file | 006 expanded pool / stored pool | 007 constants + metadata | 007 graph | 007 file |
| --- | ---: | ---: | ---: | ---: | ---: |
| interface retained | 28,360 | 94,176 / 21,859 | 94,272 | 26,167 | 127,024 |
| interface stripped | 22,762 | 72,840 / 16,261 | 72,960 | 26,167 | 105,712 |
| RexxCPS retained | 259,518 | 687,552 / 174,874 | 707,328 | 79,324 | 869,956 |
| RexxCPS stripped | 201,173 | 475,256 / 116,531 | 490,056 | 79,324 | 652,308 |

The 007 constants-plus-metadata sizes are close to the old expanded pool. That
proves that loss of compression is the main whole-container expansion. The
graph remains material: its 26,167-byte interface representation is about 92%
of the entire former retained 006 file.

Compression and semantic representation are therefore separate required
repairs:

1. restore section compression or an equivalently compact record encoding for
   the canonical pool; and
2. stop duplicating broad callable/signature/name data in the runtime semantic
   seed.

## Required semantic scope

The runtime structure should contain only data used to answer semantic runtime
questions efficiently:

| Included | Purpose |
| --- | --- |
| built-in, class, interface, language and required opaque type identities | exact identity, signatures and diagnostics |
| direct implements/inherits/extends/alias facts | link/load validation and interface enumeration |
| precomputed assignability closure | `ISTYPE`, `ASSERTTYPE`, policy hand-off |
| dynamically selected method/member slots | class/interface dispatch |
| interface factory buckets and provider/match targets | cross-class factory discovery and selection |
| only callables referenced by those tables or an explicit late/reflection surface | process target binding |

The following do not acquire graph nodes merely because they occur in the
constant or metadata pool:

- source steps, TRACE events, register/clear/constant metadata and RexxDoc;
- ordinary constants and inline diagnostic payloads;
- position-addressed procedures not used by dynamic dispatch, factories, late
  binding or supported reflection; and
- metadata order or instruction-position information already consumed through
  the canonical sections.

Parameter and return types remain flexible without text-only hot edges. Each
signature uses a dense type identity; that identity names one canonical type
record whose text covers classes, interfaces, `.float` and other built-ins,
and unresolved/opaque names. The text is read for link/load/diagnostic policy,
not on ordinary type or dispatch operations.

Both forms of relationship access are useful and should coexist:

- compact direct adjacency lists answer “which interfaces does this class
  directly declare?” and support tooling/policy walks; and
- an assignability closure answers “is this object compatible with this
  contract?” with one bit test.

## Candidate runtime representation

A context-owned runtime type descriptor provides stable identity across the
sealed image and late/native overlays:

```c
typedef struct RxRuntimeType {
    uint32_t id;
    const char *name;
    uint32_t name_length;
    uint32_t kind;
    const uint64_t *assignable_words;
    const RxBoundTarget *dispatch_row;
    const uint32_t *direct_interfaces;
    uint32_t direct_interface_count;
} RxRuntimeType;
```

For the measured 61-type interface universe, a complete dense assignability
matrix is only `61 * 8 = 488` bytes because every row fits in one 64-bit word.
For RexxCPS it is 456 bytes. An inline first word plus overflow words is a
plausible late-growth layout; a plain dense matrix is the performance
comparator.

A dense 32-bit dispatch-ID matrix would be at most 10,492 bytes for the
interface graph (`61 * 43 * 4`) and 33,060 bytes for RexxCPS
(`57 * 145 * 4`) before restricting columns to genuinely dynamic member slots.
Direct bound-pointer rows are twice those sizes on this host. Both are small
enough to measure rather than dismiss. Per-type compact/perfect rows remain a
size comparator, but sorted binary-search rows are not the zero-work target.

The sealed hot operations become:

```c
exact = actual_type == target_type;
supports = actual_type->assignable_words[target_type->id >> 6] &
           (UINT64_C(1) << (target_type->id & 63));
target = actual_type->dispatch_row[member_slot];
bucket = runtime->factory_buckets[factory_slot];
```

These expressions must be inline in both interpreter modes. Type/member name
hashing remains a load/link/late-binding/reflection service and cannot appear
on the sealed numeric opcode path.

## `TYPE_CONST` versus a separate descriptor table

Adding one new constant-pool kind is feasible and is different from enlarging
every constant. A portable `TYPE_CONST` can hold canonical name/type/kind/ID
fields. Its materialized in-memory form may contain process-local pointers that
the 007 codec never serializes.

Three layouts should be compared using the same hot representation:

### C1 — `TYPE_CONST` is the runtime type descriptor

Graph relationships and graph-bearing operands identify `TYPE_CONST` records.
The materialized type constant contains the assignability and dispatch
pointers; object values store `TYPE_CONST *`.

Strengths: one semantic object, few allocations, direct sealed-image pointer
identity, and no separate type-descriptor allocation.

Risks: heterogeneous constant-pool locality; canonical/shared pool records gain
process-local state; late images can contain a second constant for the same
context type, so pointer equality is invalid unless all uses are remapped to a
context-canonical record; mutable late publication becomes constant-pool
ownership work.

### C2 — `TYPE_CONST` owns a runtime descriptor pointer

The portable constant remains canonical. At load its runtime-only field points
to the context-canonical `RxRuntimeType`; graph-bearing runtime operands and
object values may then be bound directly to that descriptor.

Strengths: reuses constant-pool naming and avoids graph-local text while
preserving one context identity across images.

Risks: retaining `TYPE_CONST *` on the hot path would add an unnecessary hop.
The loader should patch the runtime operand directly to `RxRuntimeType *`.
Consequently the descriptor table still exists as ownership, although it is
not searched during execution.

### C3 — separate compact semantic seed and runtime descriptor table

The seed refers to canonical string/metadata IDs and the loader builds a dense
descriptor array. Runtime operands and values are bound directly to descriptor
pointers.

Strengths: best separation, locality, immutable canonical pools, simple
late/native context identity and independently tunable runtime layout.

Risks: one additional load-time allocation/directory. This is not a hot-path
lookup because operands and values are resolved once.

The meaningful comparison is therefore sealed-image build/load bytes and
late-binding ownership, not an assumed opcode penalty. C2 and C3 can have the
same one-pointer hot path as C1.

## End-to-end repair options

### R0 — revert milestone 1

Revert the 007 semantic implementation and resume from the immediate pre-T6
state if no replacement passes isolated proof. This remains the safety outcome;
the current correctness-green implementation is not accepted merely because it
exists.

### R1 — repair the current six-section graph in place

Keep the broad 007 facts/index schemas, but precompute assignability, bind all
`ProcRef`s once, create direct runtime dispatch/factory views, bind operands,
replace the value graph/name/ID collection with one type pointer, restore
compression, and remove non-dynamic callables/text duplication.

This is the shortest production route but preserves more serialized/runtime
machinery than the runtime needs. It is useful as an implementation-speed
comparator, not automatically the best design.

### R2 — compact semantic seed, runtime-materialized direct view

Use C2 or C3. RXAS/RXLINK emit only unique type/member identities, direct
relationships, dynamic callable `ProcRef`s, and factory/provider facts,
referencing canonical pool text rather than copying it. The loader builds
assignability bits, direct dispatch rows, bound targets and factory buckets.
Serialized generic search indexes are omitted unless a measured cold consumer
needs them.

This has the cleanest separation between portable truth and process-optimal
layout. Load work is higher than R3 but currently measured graph/image loads
are sub-millisecond, leaving room for a compact materialization pass.

### R3 — link-produced direct policy-neutral views

RXLINK emits assignability bitsets, dynamic dispatch target IDs, direct
interface lists and factory/provider ranges. The loader validates/borrows those
tables and binds a compact target-ID array to `proc_runtime *` once.

This minimizes load construction and is the likely cold-start ceiling. It
couples 007 more closely to the runtime table layout and may serialize unused
capacity. C1 is most natural for a sealed-only form; C2 remains safer for the
late/native context overlay.

### R4 — no serialized semantic graph

RXLINK may build the graph transiently for validation/link decisions, but emit
only canonical metadata. RXVM derives the compact runtime view at load. This is
the minimum-duplication size comparator and proves the cost of a serialized
seed, but repeats discovery work and is unlikely to be the best lifecycle
choice for large images.

## Separately buildable implementation seam

The existing `rxbin` library is the correct common format/semantic-seed home;
`platform` remains OS-specific and is not appropriate. VM-specific
`proc_runtime *` binding must not leak into the portable codec.

The production seam should be:

1. `rxbin`: portable `TYPE_CONST`/seed codec, builder, validation and
   policy-neutral relationships;
2. a small interpreter runtime-view source/target: context type descriptors,
   assignability materialization, `ProcRef` resolver callback, bound dispatch
   and factory arrays; and
3. inline accessors consumed by `rxvmintp.c` and both VM modes.

The runtime-view target can link against `rxbin` and a fake resolver in the
scratch harness. A graph/layout edit then rebuilds only `rxbin`, the small
runtime view and the harness. The current first incremental build of `rxas`
plus `rxgraph_bench` completed in under one second after configuration; the
real RXAS fixture assembles in about 0.3 seconds. No CREXX compiler or full VM
rebuild is needed to compare seed/runtime layouts.

## PoC decision gate

No replacement should enter `rxvmintp.c` before the isolated harness proves:

1. C1, C2 and C3 exact/support hot expressions, including compiler output;
2. dense bitset versus inline-first-word/overflow assignability;
3. dense bound-pointer versus dense target-ID-plus-binding-array dispatch;
4. direct factory/provider range access;
5. retained and serialized bytes, allocation count, build time and load/bind
   time on the real interface and RexxCPS images;
6. a real RXAS-produced standalone image and RXLINK-produced shared image; and
7. context identity behavior for a small simulated late-image remap.

The harness is exploratory. Once a production layout is accepted, remove its
temporary storage-stat API and layout variants, retaining only small functional
and performance-smoke tests that guard the selected zero-allocation inline
paths.

## Selected hot view and physical result

The common hot view is now selected and measured independently of its
serialized seed:

- one context-canonical runtime type pointer on object values and runtime
  operands;
- dense precomputed assignability, starting with an inline 64-bit word;
- already-bound direct dispatch rows and direct factory buckets; and
- cold name/late-binding directories outside the sealed opcode path.

The disposable fast design loop measured complete-section compression and
compact/direct seed alternatives over real RXAS and RXLINK images. Adrian
selected complete-section compression behind the existing broad R1 seed. The
shared 007 writer retains LZSS only when a section shrinks; the checked reader
expands it before the unchanged graph/native-pool materialization path.

The production retained interface image is 37,458 bytes and RexxCPS is 273,858
bytes, exactly matching the PoC. Median complete interface-image load is 106 us
versus 79 us uncompressed; the descriptor/dispatch/factory hot view remains at
approximately 1 ns. Same-cell integrated timing is within run noise and
canonical RexxCPS remains neutral or better across the two VMs. A linked image
re-links byte-for-byte and executes under both VMs. Evidence is in
`evidence/2026-07-16-nr-04a-rxbin-007-compression/`.

The alternate seed PoCs have been removed from production source. R2/R3 remain
historical evidence for an optional graph-size refinement, not an open format
decision. `rxgraph_bench` remains a separately built regression harness for the
selected zero-allocation inline paths and cold load/materialization cost.

This recommendation does not select new inheritance, override, interface
default, overload, visibility or provider-scoring rules. Those remain language
policy. The runtime layout only makes already-selected relationships and
targets fast to inspect, enumerate and execute.
