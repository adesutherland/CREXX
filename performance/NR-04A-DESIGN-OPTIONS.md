# NR-04A metadata access design options

Status: kind-index assessment complete; superseded by holistic type/dispatch decision

Date: 2026-07-16

This note corrects the missing alternatives/PoC gate before the first ordinary
production-runtime performance change. The current eager runtime index is
candidate A only. The dated programme report remains a snapshot and is not
rewritten.

The selection objective is the best end-to-end performance architecture, not
the smallest implementation or compatibility blast radius. Correctness remains
mandatory. Format scope, migration, maintenance, and fallback complexity are
costs to expose and measure, not automatic vetoes on a stronger design.

## Current surfaces and constraints

- RXBIN format `006` is a record stream with exactly three accepted record
  types: local module, shared constant pool, and shared-backed module.
- `module_header` has instruction/constant sizes and `proc_head`,
  `expose_head`, and `meta_head`; it has no header size, extension directory,
  optional section descriptor, or runtime-index pointer. Readers reject unknown
  versions, record types, section flags, and inconsistent section sizes.
- `rxlink` builds one shared constant pool, rewrites every selected module's
  canonical metadata chain, and writes one shared-pool record followed by one
  shared-backed module record per module. It therefore already knows each
  rewritten metadata offset and original module ordering.
- A shared pool can contain metadata for many modules. Any prebuilt index must
  retain per-module spans even when offsets address the same pool allocation.
- The loader expands packed sections and shared-backed modules borrow the
  expanded pool. A seed inside that pool can share its lifetime; a new separate
  section would need reader ownership/borrowing rules or a wider mapping design.
- Direct local `.rxbin` modules do not pass through `rxlink`. Native plugins
  synthesize strings and metadata in memory. Runtime `METALOADMODULE` can add
  local/linked images after execution begins. Any link-only design therefore
  needs a fallback unless seed generation moves into RXAS as well.
- Canonical chain ordering, duplicate/first-match semantics, source/TRACE
  diagnostics, and `METALOADDATA` generic introspection must remain available.

## What candidate A currently does

Candidate A is not built at link time and is not a resolved-result cache.
`prep_and_link_module()` invokes it after ordinary or native metadata is
complete. It validates the full canonical chain, creates one heap allocation
grouped by all twelve `META_*` kinds, and stores `size_t offset` plus original
`size_t ordinal` per record. Lookups still examine each candidate record of the
requested kind(s); they no longer cross unrelated source/TRACE records. The
allocation is freed with the runtime module. Interface method/factory
registries remain separate context-level resolved caches with dirty rebuilds.

For the retained optimized RexxCPS image candidate A retains 6,634 entries and
106,144 bytes across 16 modules; the stripped form retains 4,220 entries and
67,520 bytes. Most of the retained-image bytes are source/TRACE, REG, and CLEAR
entries that ordinary contract lookup does not request.

## Option families

### 0. Status quo canonical scans

Keep only `meta_head`/`next`/`prev`. This has zero new setup, file, ownership,
or retained-memory cost and is the compatibility fallback. It is not a viable
hot implementation for retained images: contract operations repeatedly visit
large source/TRACE populations.

### 1. Runtime-built generic indexes

1. **A: eager all-kind grouped index** — current prototype. One validation and
   construction pass per loaded module; predictable lookup; highest setup and
   retained-byte footprint.
2. **B1: lazy whole index** — build A on the first indexed request. This avoids
   work for modules/programs that never query metadata but moves the whole cost
   onto the first request. A production form must not silently defer invalid
   image rejection or race concurrent bridge callers.
3. **B2: lazy per-kind spans** — build only a requested kind, or an ordered
   requested kind set, on first use. It minimizes unused storage but can rescan
   the chain several times and needs per-kind unbuilt/building/ready/failed
   state plus teardown and late-load rules.
4. **B3: adaptive promotion** — use canonical scans for the first N queries and
   promote hot kinds. It can avoid allocations for one-off lookups, but adds a
   branch/state update to every pre-promotion lookup and needs an evidence-based
   promotion policy. It is unlikely to be a no-regrets first production choice.
5. **C: eager critical-kind index** — validate the complete chain once but
   retain only `META_FUNC`, `META_CLASS`, `META_INTERFACE`,
   `META_IMPLEMENTS`, and `META_MEMBER` (subject to inventory proof). Source,
   TRACE, REG, CLEAR, CONST, ATTR, and INLINE stay on canonical full traversal.
6. **Compact representation variant** — use fixed-width pool offsets/ordinals
   where validated limits allow, or per-kind 32-bit offset vectors plus a
   cross-kind ordering mechanism. Candidate A's two `size_t` fields consume 16
   bytes per entry on this host; representation choice is independent of eager
   versus lazy construction.

### 2. Purpose-built runtime caches

Build sorted/hash tables for callable symbols, contract kind, class/interface
relationships, or resolved method/factory products rather than a generic kind
index. These can beat per-kind scans and retain less irrelevant metadata, but
they duplicate existing registry responsibilities and make duplicate,
first-match, invalidation, late-load, and native-provider semantics central.
This is a broader ownership/caching decision, not an automatic NR-04A fallback.

### 3. Link-produced serialized tables

1. **E1: separate runtime-critical index section/table** — `rxlink` emits
   per-module kind spans and compact `(offset, ordinal)` data beside canonical
   metadata. The VM validates/borrows or decodes it. A clean implementation
   needs RXBIN 007 because 006 has no optional section directory/pointer.
2. **E2: separate runtime metadata records** — emit compact semantic records
   (callables, contracts, implements, members) rather than generic offsets.
   This reduces runtime dereferencing but duplicates data and makes linker
   semantics more authoritative.
3. **E3: physical kind grouping in the shared pool** — place critical metadata
   contiguously and store per-module spans. This may remove per-entry offset
   vectors, but the current linker recursively interleaves rewritten structured
   records and referenced leaves; a two-pass layout is needed. Canonical chain
   links and generic order still need preservation.
4. **G: split runtime/debug metadata** — keep a runtime-critical table/chain
   separate from source/TRACE while retaining a canonical combined view for
   tools. Avoiding duplication requires a merge/index contract; duplicating
   critical descriptors trades file size for simple runtime access.
5. **E4: link-produced direct semantic lookup tables** — emit stable-hash or
   sorted lookup structures for callable names, contract kinds, implements
   relationships, and member ownership, with offsets into canonical metadata
   only where the complete record is required. The VM probes the serialized
   structure directly rather than scanning a kind vector. This has the highest
   plausible steady-state ceiling in the current option set and can also remove
   runtime discovery/allocation for linked images. It needs precise collision,
   duplicate/first-match, module-order, encoding, and hostile-image validation
   rules. A whole-image directory is simplest for a completed linked image;
   late-loaded and native modules need a mergeable runtime table or fallback.

### 4. Constant-pool seed designs

1. **F1: new header seed pointer plus payload** — a new header field points to
   a compact constant-pool payload. Shared-backed modules can borrow the same
   pool lifetime with distinct pointers. This is a straightforward RXBIN 007
   candidate.
2. **F2: new metadata/constant kind** — append a typed index record and point to
   it from the header. This still requires coordinated readers, linker rewrite,
   assembler/disassembler support, and a version decision.
3. **F3: reserved 006 `META_CONST` plus `BINARY_CONST` convention** — avoids a
   header-layout change in name only. The VM must discover the reserved record,
   old tools expose or rewrite it as ordinary metadata, a symbol namespace is
   reserved, and malformed payload behavior is ambiguous. Retain only as a
   compatibility thought experiment; do not assume it is safe.
4. **F4: assembler-produced seed** — RXAS emits a seed for every local module;
   `rxlink` rewrites/merges it. This covers direct modules as well as linked
   images but widens the change across assembler, linker, loader, disassembler,
   tests, and the format contract.

### 5. Hybrid and external forms

1. **H1: link seed plus runtime fallback** — linked 007 images borrow the
   seed; local/legacy 006/native modules build a narrow eager or lazy index.
2. **H2: assembler seed plus native fallback** — local and linked bytecode use
   serialized seeds; only native metadata is runtime-built.
3. **H3: seed validation plus runtime materialization** — link/assembler data
   avoids discovery but the VM copies into its preferred representation. This
   reduces format/runtime coupling at the cost of allocation and copy time.
4. **Sidecar file** — keep RXBIN unchanged and ship a hash-bound adjacent index.
   This adds packaging, discovery, integrity, missing/stale-file, and embedded
   runtime problems; it is likely inferior but remains an explicit rejected
   option rather than an unspoken omission.

## PoC selection before more implementation

Only bounded measurements that discriminate between the option families proceed
before Adrian's selection; production implementation and selection remain
paused. The PoCs discriminate performance ceilings rather than prefer
candidates with smaller code or format scope.

Do not implement every row. First estimate them from the retained counts and
current loader/linker structure, then build the smallest discriminating set:

- status quo baseline;
- A, already frozen at VM hash
  `c4a7be443b8655dc9f6db0565c814544527ee453a4e787cc17e61637e79c6804`;
- C, because it tests whether most setup/memory cost comes from indexing kinds
  ordinary runtime lookups never request;
- one lazy form (B1 or B2) if lifecycle/unrelated cells indicate setup is the
  issue;
- one link-produced seed model, initially as an in-memory/mock loader input if
  that can measure lookup/load/file-size trade-offs without prematurely
  defining RXBIN 007; and
- one purpose-built direct semantic table (runtime-built D or link-produced E4)
  because generic kind vectors cannot establish the maximum lookup-performance
  ceiling.

## Performance-first decision groups

1. **Highest-ceiling architecture:** E4, preferably combined with G and an
   H-style fallback. Link/assembly produces a separate runtime-critical direct
   lookup directory; source/TRACE remains a distinct diagnostic surface; the VM
   probes semantic tables rather than metadata vectors.
2. **Format-neutral semantic comparator:** D. Build the equivalent semantic
   hash/relationship tables at runtime. This measures the lookup ceiling and
   isolates the benefit of moving construction to link time, but retains load
   work and heap ownership.
3. **Generic-index comparator:** C (and compact representation). This should
   retain A's observed improvement with substantially less memory, but lookup
   remains linear within each requested kind and therefore does not establish
   the best possible steady-state result.
4. **Construction-policy variants:** A/B1/B2. These answer when generic index
   work should occur; they are secondary once the direct semantic-table ceiling
   is known.
5. **Compatibility baseline:** status quo and H fallbacks. These are needed for
   exact comparison and legacy/native coverage, not as the default performance
   target.

The consumer audit subsequently showed that this kind-oriented option set is
only one layer of the real problem. Dynamic polymorphism needs a coherent type,
relationship, method-dispatch, factory, and late-binding architecture. See
`performance/NR-04A-RUNTIME-TYPE-DISPATCH-DESIGN.md`; its T2-T6 options supersede
a generic kind index as the highest-performance candidates.

## Common comparison matrix

Every retained candidate must report:

- exact VM and exact retained/stripped image hashes;
- normalized per-operation/per-kind visits and zero source/TRACE visits for
  ordinary contract/callable/ADDRESS lookup;
- runtime-interface factory/method benchmark and fixed-count RexxCPS;
- all eleven canonical optimized portfolio cells, with JSON/List stripped
  repeats retained rather than explained away;
- load-to-first-result, build/seed validation work, temporary and retained heap
  bytes, allocation count, and teardown;
- linked file size, shared-pool size, link time, and decompression/load cost for
  serialized candidates;
- direct local, linked shared-pool, legacy image, late load, dynamic/static
  native plugin, shared pool, teardown/reload, `rxvm`, and `rxbvm` behavior;
- format/tool compatibility (`rxas`, `rxlink`, `rxdas`, embedded/wrapped
  runtimes) and 32/64-bit encoding assumptions; and
- implementation complexity, ownership, invalidation, failure timing, and
  rejected-option reasons.

No portfolio aggregate or regression threshold is invented here. The design
recommendation must expose every neutral and negative cell and separate
steady-state benefit from startup, file-size, and retained-memory costs.
