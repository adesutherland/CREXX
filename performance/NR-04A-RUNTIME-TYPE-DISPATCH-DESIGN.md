# NR-04A runtime type and dispatch architecture options

Status: complete — T6/RXBIN 007, C3-style process-local views, process-wide
callable/factory binding, generation-guarded site caches, and complete-section
compression pass the selected correctness and performance gates

Date: 2026-07-16

## Design correction

The production problem is broader than finding `META_*` records by kind. The
hot semantic surface is a runtime type and dispatch system: callable identity,
class/interface relationships, assignability, dynamic method selection, and
interface factory selection. A generic metadata-kind index is useful as a
measured comparator and legacy fallback, but it is not the highest-performance
abstraction for polymorphism.

The selection objective is the best end-to-end performance design. File-format,
ISA, loader, and implementation scope are measured costs, not reasons to prefer
a slower design. Adrian selected T6 on 2026-07-16 and approved a coordinated,
non-backward-compatible RXBIN 007 transition. The canonical implementation
design is [RXBIN_007_SEMANTIC_GRAPH.md](../docs/ai-context/RXBIN_007_SEMANTIC_GRAPH.md).
This note retains the option comparison and measurement rationale.

The first directed repair keeps the six-section 007 seed and current consumers
but materializes C3-style process-local descriptors and dense support/dispatch
views inside `rxbin`. It replaces four object type fields with one descriptor
pointer. Isolated descriptor support/dispatch is about 0.92-1.07 ns and the
first integrated evidence is retained under
`evidence/2026-07-16-nr-04a-c3-candidate/`. Portable callable-to-runtime
procedure binding remains outside `rxbin`, as required, but is now materialized
once in a VM-owned dense table for each semantic generation. Bound
factory/provider rows, a two-way method-site cache, and factory
bucket/direct-target site caches complete the selected sealed-image hot shape.
Evidence is retained under `evidence/2026-07-16-nr-04a-bound-cache/`.

## Current language contract and deliberate future boundary

Current Level B provides:

- classes implementing one or more interfaces;
- abstract interface methods and final/default interface method bodies;
- default and named interface factories;
- runtime provider selection through class-side `match`, including score and
  alphabetical tie-break rules;
- exact-class and implemented-interface tests/casts; and
- concrete runtime type introspection.

The documented current model has no class-superclass surface and explicitly
does not implement interface inheritance. The runtime representation should
nevertheless admit future class and interface graph edges without choosing
future inheritance, linearization, override, default-conflict, or visibility
semantics here. Those remain language-design decisions for Adrian.

## Current execution and link paths

### Position-addressed and already separate

- Bytecode dispatch uses instruction addresses directly.
- Procedure execution uses runtime procedure arrays built from `proc_head`.
- Imported/exported callable linkage uses `expose_head` and exposed-symbol
  trees.
- Registers, constants, clears, attributes, and inline metadata are not name
  lookup inputs in the executing VM.
- Source/panic context is instruction-position based: find the closest
  preceding `META_SOURCE_STEP`. TRACE and generic `METALOADDATA` remain
  diagnostic/introspection surfaces in canonical order.

These surfaces do not justify an all-kind runtime index.

### Name/relationship-addressed and performance critical

- `SRCMETHODSEL_REG_REG_STRING` now receives a numeric member ID, reads the
  receiver's `RxGraphTypeRef`, obtains a dense callable ID, and indexes a
  VM-owned bound-target table. Its two-way instruction-site cache avoids even
  that fallback for repeated receiver types.
- `SRCFPROCSEL_REG_STRING_REG` now receives a numeric factory ID and caches the
  VM-bound provider bucket. A single-provider/no-match bucket returns a direct
  bound target; other buckets still invoke each required user `match` and
  select by score/tie-break.
- `ISTYPE` and successful `ASSERTTYPE` use descriptor identity or a precomputed
  assignability bit. Name normalization and relationship traversal are cold
  cross-graph/diagnostic fallbacks.
- RXVML duplicates callable, signature, class-discovery, and implements
  queries rather than consuming one shared runtime semantic service.
- Profiling classifies callables from the same semantic metadata, but this is
  tooling setup rather than a reason to put generic metadata traversal on the
  production hot path.

### Work already repeated across the linker and VM

`rxlink` discovers interfaces, implementations, members, callable
symbols, factory/match functions, and signature compatibility while validating
the selected linked image. After load, `rxvm_link()` builds one process-bound
callable view per graph and process-wide factory buckets that include compatible
providers from every loaded graph. Late-loaded and native modules cause a
complete coherent binding/legacy-registry rebuild and semantic-generation
increment. An append-only incremental overlay is an optional future optimization,
not a correctness or NR-04A completion dependency.

## Logical runtime model

The following is an encoding-independent model. The same model can be built in
the VM for a format-neutral PoC, serialized by RXAS/RXLINK, or split between a
link seed and loader binding.

### Identities

- **TypeId**: dense runtime identity for a text-backed built-in, class,
  interface, opaque/external, or language-defined type node. Image-local IDs
  may be remapped when images/modules join one VM context.
- **MemberId**: identity for a canonical method/factory name plus signature.
  Name alone is insufficient if later language levels add overloads.
- **FactoryId**: an interface/factory member bucket identity. A provider result
  is not generally cacheable because `match` can depend on call arguments.
- **ProcRef**: portable serialized `(module, procedure)` or pool-relative
  reference, bound by the loader to `proc_runtime *` for execution.
- **Universe generation**: changes only when the type/dispatch universe changes,
  not on ordinary calls.

Stable process-global IDs are not required. Dense context-local IDs give better
tables and allow late loaders to append/remap without exposing addresses in the
file format. Canonical text remains on every type node; parameter and return
records refer to those nodes by ID, so text flexibility does not require
text-only hot edges. `.float` and the other built-ins are preseeded ordinary
nodes.

### Core tables

1. **Type directory** — canonical name/hash to TypeId and class/interface kind.
2. **Type graph** — typed edges for current `implements` plus reserved graph
   categories for future superclass and superinterface relationships.
3. **Assignability data** — exact-type identity plus precomputed or incrementally
   maintained transitive membership. Candidate encodings include per-type
   bitsets, sorted TypeId vectors, or small-inline-plus-overflow sets.
4. **Callable directory** — callable symbol/MemberId to ProcRef and canonical
   signature. This replaces repeated `META_FUNC` scans for runtime and RXVML.
5. **Class dispatch rows** — `(concrete TypeId, MemberId) -> ProcRef`, including
   final/default interface bodies after the language resolution rules have been
   applied.
6. **Interface dispatch views** — an itable-like view from a class/interface
   contract to dense member slots. It can be materialized directly or represented
   by class dispatch rows plus per-interface slot maps.
7. **Factory buckets** — `FactoryId -> ordered provider[]`, each provider holding
   class TypeId, factory ProcRef, optional match ProcRef, and bound signature.
   Runtime selection still executes required `match` functions and preserves
   score/tie-break semantics.

Canonical metadata remains the authoritative introspection/debug surface. The
runtime tables are derived semantic products, not a replacement for source,
TRACE, RexxDoc/tooling, or generic metadata order.

## Fast paths

### Type tests and casts

Resolve the target contract operand to TypeId once. Store the concrete TypeId
on object values (possibly alongside the existing type-name pointer during a
transition). Then:

- exact-class test: integer equality;
- interface/ancestor test: bit or small-set membership; and
- `typeof`: TypeId-to-name table lookup.

This removes per-test allocation, name normalization, contract-kind lookup, and
relationship scans.

### Dynamic method selection

Use two levels:

1. **Per-instruction-site cache** keyed by universe generation and concrete
   TypeId. Start monomorphic, admit a small polymorphic cache, then mark the site
   megamorphic when appropriate.
2. **Global dispatch fallback** keyed by `(TypeId, MemberId)`, using dense slots
   or a compact hash/table. It supplies the cache and handles megamorphic sites.

A side table indexed by module/instruction position avoids self-modifying
bytecode and keeps read-only/shared images possible. The hot monomorphic path is
generation compare, TypeId compare, and a bound ProcRef/pointer load.

### Dynamic factory selection

Resolve the descriptor/selector to FactoryId once per instruction site. Cache
the provider bucket and bound signatures. The selected provider cannot usually
be cached across calls because user `match` results depend on arguments; all
required matches still execute. Single-provider/no-match buckets can have a
direct fast path while retaining the specified scoring rule where a match
exists.

### Callable and signature lookup

Runtime and RXVML use the shared callable directory. Procedure execution itself
continues to use existing runtime procedure arrays and linked exposures; the
new table serves only genuinely dynamic symbolic/signature requests.

## Link, load, and late binding

### Closed linked image

RXAS can emit module-local symbolic seeds. RXLINK already has the complete
selected-module view and can:

- assign compact image-local TypeId/MemberId values;
- validate and flatten current implements/member/factory relationships;
- produce dispatch rows and factory buckets;
- precompute assignability for the closed image; and
- emit portable ProcRefs and string/metadata offsets rather than process
  pointers.

The loader validates bounds and semantic invariants, maps image-local IDs into
the context universe, and binds ProcRefs to runtime procedure pointers. A
serialized direct table minimizes discovery work; a compact seed plus runtime
materialization may produce better in-memory tables.

### Late-loaded/native/open world

Late binding is intentionally a fast mutable layer over the closed seed:

- append or remap new TypeId/MemberId identities;
- validate new graph edges and providers;
- update only affected assignability/dispatch/factory rows where practical;
- increment a universe generation after publishing a coherent update; and
- let per-site caches miss and refill on generation mismatch.

The first PoC may rebuild the small semantic universe to establish a performance
ceiling, but the production design should measure incremental update versus
full rebuild. Native definitions feed the same builder; there is no legacy
RXBIN 006 reader or fallback. Concurrency/publication rules for nested
or concurrent RXVML/native calls must be explicit before production adoption.

## Performance-first architecture options

### T0 — Current metadata scans

Compatibility baseline only. It has no setup cost but repeatedly performs
string and mixed-chain work and is not a plausible polymorphic hot path.

### T1 — Generic critical-kind index (candidate C)

Retain `FUNC`, `CLASS`, `INTERFACE`, `IMPLEMENTS`, and `MEMBER` offsets after one
validation scan. This is a useful lower-bound comparator and builder
aid. It removes unrelated metadata but still linearly examines same-kind records
and still parses/compares strings.

### T2 — Runtime-built semantic universe

Build the identities, graph, assignability, callable, dispatch, and factory
tables in memory from canonical metadata. Add per-site late-binding caches.
This comparator does not require RXBIN or ISA change and can measure the
semantic architecture's lookup ceiling. It pays runtime
discovery/materialization and heap cost.

### T3 — Link-seeded semantic universe, runtime-materialized

RXLINK/RXAS emits compact semantic records; the loader binds them into the T2
runtime layout. This removes repeated discovery/validation but keeps freedom to
tune the in-memory representation independently of the format. T6 adopts this
shape through the RXBIN 007 section directory.

### T4 — Link-produced direct tables

Emit dense TypeId/member slots, assignability data, dispatch rows, and factory
buckets in a layout the VM can mostly borrow after validation, binding only
ProcRefs. This minimizes load allocation/copy and can provide the best cold-start
result, but couples serialized and runtime layouts more tightly.

### T5 — Numeric runtime operands / linked-image opcode specialization

Replace or specialize string-bearing type/method/factory operands with TypeId,
MemberId, and FactoryId operands in linked images. Combined with T3/T4 and
per-site caches, this removes first-use descriptor/name resolution and has the
highest plausible dispatch ceiling. This ISA/format change is approved as part
of T6/RXBIN 007; there is no legacy decode path.

### T6 — Hybrid sealed seed plus open-world overlay

**Selected 2026-07-16.** Use T3/T4/T5 for RXBIN 007 linked-image contents and a
T2 incremental overlay for late-loaded 007 and native modules. Per-site caches
see one coherent universe and generation. RXAS builds a module-local graph;
RXLINK merges, reassigns IDs, rebuilds indexes, and emits one sealed image
graph. A common compiled binutils library owns the fixed-width codec, builder,
merge/remap, validation, and rule-neutral queries. Language policy owns
inheritance, override/default, assignability, and provider-selection decisions.

## Measurements that discriminate these options

Existing evidence already establishes:

- candidate A removed over 99% of unrelated metadata visits for contract-kind
  and implements queries;
- restoring canonical `METALOADDATA` traversal reduced the giant `run()` code
  growth from 3,952 bytes to 48 bytes and changed the prior stripped-JSON
  regression to a 2.268% improvement in that repeat;
- candidate C retained only 197 semantic offsets (3,152 bytes) for the focused
  interface image and improved its retained/stripped medians by 55.393%/46.463%
  versus the exact baseline in that run; and
- candidate C still examined 2,400,024 `META_IMPLEMENTS` entries for 100,001
  negative relationship queries, so it does not measure the direct-map ceiling.

The selected sealed-image runtime shape has now passed its bounded gate. The
production descriptor/dispatch/factory primitives measure approximately
control cost, final `SRCMETHODSEL`/`SRCFPROCSEL` attribution is about 14 ns per
instruction, and the exact focused `rxvm` process is 70.54-76.05% faster than
006. The operand audit also found and drove correction of a short/canonical
factory-type remap that had forced every measured factory site through the
legacy resolver. Detailed evidence is in
`evidence/2026-07-16-nr-04a-bound-cache/`.

The following are optional future refinements outside completed NR-04A:

1. T2 relationship/type directory: direct TypeId plus assignability membership
   on the existing focused exact image; compare positive and negative tests.
2. T2 method dispatch: monomorphic, bimorphic, small-polymorphic, and
   megamorphic sites; report inline-cache hits/misses and global fallbacks.
3. T2 factory buckets: one provider, several providers without matches, and
   several argument-sensitive matches. Separate descriptor/bucket overhead from
   required user match execution.
4. T3/T4 offline seed model: exact serialized bytes, link time, validation,
   materialization/allocation bytes, and ProcRef binding work for retained and
   stripped RexxCPS plus the focused interface image.
5. Late-load overlay: cold update cost, affected rows, generation invalidations,
   first post-load miss, and steady state after refill.

The isolated/runtime and both-VM focused cells discriminate the sealed hot
shape, and complete-section compression is selected and measured. NR-04A does
not carry these optional refinements or additional platform/portfolio sweeps as
unfinished closeout work.

## Decisions recorded for production implementation

1. RXAS and RXLINK move atomically to RXBIN 007; all consumers move with them,
   reject 006, and rebuild their artifacts.
2. Type, member, and factory instruction operands are graph IDs; object values
   carry one immutable runtime type-descriptor pointer.
3. Per-instruction-site caches use a VM-owned side table, not mutable serialized
   instructions.
4. Linked images are sealed seeds with a generation-rebuilt process view for
   open-world additions; an append-only overlay is a future optimization.
5. Class-inherits-class and interface-extends-interface edges are representable.
   Their semantics remain a separate language decision.
6. Canonical type text is retained once on a graph node; signatures and graph
   relationships use dense local IDs. Built-ins such as `.float` are ordinary
   special nodes.
7. The common graph builder/query/codec is a compiled binutils library shared by
   RXAS, RXLINK, the VM, and tools. It is structural and policy-neutral.
8. NR-04A retains comparator evidence. The resolved-view, sealed numeric
   operand/cache, cross-image provider, complete-section compression, and broad
   Debug gates pass. Compact graph scope and an append-only overlay are optional
   future refinements.
