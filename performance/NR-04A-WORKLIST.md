# NR-04A resumable worklist: kind-index runtime metadata and scan counters

Status: complete — RXBIN 007, process-local graph views/bindings, site caches,
section compression, retained Release evidence, cross-image provider behavior,
and the full 1,846-test Debug closeout all pass

Started: 2026-07-16

## Verified starting state

- Branch: `develop`
- Starting HEAD / completed NR-05 commit: `7a599906b` (`Complete NR-05 call-path census`)
- Required NR-04 ancestor: `6a064499f` (`Complete NR-04 opcode effects inventory`)
- Fetched `origin/develop`: `552bb76c4`
- Relationship after fetch: ahead 2, behind 0
- Starting worktree: clean
- Roadmap: NR-02, NR-03, NR-04, and NR-05 complete; NR-04A queued
- Available builds:
  - `cmake-build-debug`: Debug, profiling OFF
  - `cmake-build-release`: Release, profiling OFF
  - `cmake-build-profile`: Release, profiling ON
  - `cmake-build-debugasan`: not configured at start; configure through the
    repository ASan workflow before sanitizer validation
- Starting profiler format: schema 4, 24-column CSV

The dated programme report is a historical charter and is not edited by this
activity. Do not push the final commit.

## Original candidate A implementation plan (superseded)

1. Record every `meta_head` traversal and define counter semantics.
2. Retain a clean baseline keyed to `7a599906b`, including exact product VMs,
   retained/stripped images, serial raw timings, profiles, and provenance.
3. Add a runtime-only per-module kind index and lifecycle management.
4. Add shared indexed/full-chain cursors and schema-5 traversal accounting.
5. Migrate hot runtime consumers without changing canonical full-chain source,
   TRACE, debug, or introspection behavior.
6. Add structural, semantic, reconciliation, late-load/native/shared-pool, and
   teardown tests; update current documentation and evidence tooling.
7. Produce the paired exact-image evidence and run focused, sanitizer, Release,
   and broad Debug validation.
8. Complete the roadmap only if every exit criterion passes, remove this
   worklist, and make one focused local commit.

## Design-selection correction (2026-07-16)

The original plan moved from the traversal inventory directly to one eager
per-module index design. That was insufficient for the first performance-
programme change to ordinary production runtime code. The implementation below
is retained as candidate A evidence, not as the selected production design.
Its eager runtime index, cursor migrations, and schema-5 profiling code were
removed from the production tree after T6 was selected. Do not restore or
commit that prototype as the production solution, and do not mark NR-04A
complete from its results.

Adrian's selection criterion is maximum end-to-end performance, not minimum
blast radius. Correctness is mandatory; production/file-format scope and
compatibility are comparison costs rather than reasons to discard a
plausibly faster architecture. As of 2026-07-16 Adrian selected T6 and approved
an atomic RXBIN 007 transition with no 006 compatibility path. The canonical
implementation design is `docs/ai-context/RXBIN_007_SEMANTIC_GRAPH.md`. Bounded
measurements now choose the physical layout inside that architecture; the
retained candidate A/A-prime/C work remains comparator evidence and is not the
production fallback.

### Historical options considered

| Candidate | Construction and lookup | Expected strength | Cost/risk to measure |
| --- | --- | --- | --- |
| A: eager all-kind per-module index (current prototype) | Scan every module once after preparation; retain grouped `(offset, ordinal)` entries for every `META_*` kind | One validation pass; stable ordered single/multi-kind cursors; late-load lifecycle already integrated | Adds setup work and retained memory to every module, including programs that never use the indexed paths; may perturb unrelated hot-path layout/performance |
| B: lazy/on-demand per-module kind spans | Keep canonical metadata unchanged; build and retain only a requested kind (or requested kind set) on its first indexed query | Avoids setup and memory for unused kinds/modules; preserves fast repeated hot lookups | First-hit latency; repeated full-chain passes when several kinds become hot; synchronization/rebuild/failure state and late-load behavior |
| C: eager narrow runtime-kind index | Validate once, but retain entries only for production semantic kinds used by ordinary callable/contract/registry paths; keep source/TRACE/introspection on canonical traversal | Removes source/TRACE and REG/CLEAR/CONST storage while retaining predictable runtime lookup speed | Must prove the selected kind set is complete; multi-kind ordering; profiler/tool consumers; still pays one validation scan at load |
| D: purpose-built contract/registry caches | Cache resolved contract kind/implements and registry products rather than generic metadata kinds | Potentially lowest lookup cost and storage for the measured workload | Broader invalidation/ownership design, duplicate/first-match semantics, and overlap with global hash/caching redesign; retain as a design option but require a separate approval if it changes ownership architecture |
| E: link-produced runtime-critical table | `rxlink` emits a compact per-module kind/offset/ordinal table beside the canonical metadata; the VM borrows or lightly decodes it | Reuses work the linker already performs; no runtime discovery scan for linked images; can omit debug-only kinds | A clean separate section/header pointer requires coordinated RXBIN 007 work; direct modules, legacy images, and native plugins need fallback |
| E4: link-produced direct semantic tables | Linker/assembler emits direct hash/sorted tables for callable, contract, implements, and member queries, optionally as a whole-image runtime directory | Highest plausible lookup ceiling: direct probes rather than per-kind scans, with construction moved out of runtime for linked images | Requires explicit duplicate/order/collision/validation rules and a merge/fallback path for late/native modules; coordinated format/tool work |
| F: constant-pool seed index | Linker (or assembler plus linker rewrite) stores a compact index payload in the constant pool and gives each module a direct seed pointer | Can share the pool lifetime and avoid a second runtime allocation/copy; potentially smaller loader change than a general section directory | A clean pointer/new constant kind is still serialized format; a reserved 006 `META_CONST` convention would leak into introspection and compatibility and is not assumed safe |
| G: split runtime/debug metadata | Linker emits a runtime-critical chain/table separately from source/TRACE metadata while preserving the canonical metadata surface for tools | Ordinary runtime consumers never cross source/TRACE and may not need a general per-kind index | Duplicated descriptors or a merge view for generic introspection; larger linker/format change; direct/native fallback |
| H: hybrid link seed plus runtime fallback | Consume a link/assembler seed when present; build lazily or eagerly only for legacy 006, direct modular, late-loaded, or native modules | Optimizes packaged linked images without dropping compatibility/dynamic loading | Two paths to validate and keep semantically identical; file-size/build-time cost; fallback policy can hide mode-specific regressions |
| Status quo | Canonical chain scans only | No new setup, allocation, ownership, or unrelated-path effect | Retained source/TRACE records dominate contract/type scans and the focused interface factory path |

The detailed surface map and wider option set are in
`performance/NR-04A-DESIGN-OPTIONS.md`. Adrian approved the clean serialized
form as RXBIN 007 and selected T6; the table remains the decision record rather
than an open alternatives gate. The holistic semantic model and
polymorphic/late-binding options are in
`performance/NR-04A-RUNTIME-TYPE-DISPATCH-DESIGN.md`. They supersede a generic
metadata index as the performance target while retaining A/C as comparators.

### Completed selection record

- [x] Preserve candidate A in an isolated exact build and freeze its VM hash.
- [ ] Prototype B without changing RXBIN, public ABI, or canonical links
  (optional comparator only; no longer a selection prerequisite).
- [x] Prototype C without changing RXBIN, public ABI, or canonical links.
- [x] Select link-produced direction: T6 with separate RXBIN 007 graph/index
  sections, numeric operands, sealed image seed, and late/native overlay.
- [ ] Record why D is rejected or retain a bounded implementation if it remains
  plausibly competitive after B/C.
- [ ] Run identical exact retained/stripped images on baseline, A, B, and C.
- [ ] Compare normalized visits and zero source/TRACE visits on ordinary lookup.
- [ ] Compare the existing runtime-interface lookup benchmark and fixed-count
  RexxCPS metadata-active diagnostic.
- [ ] Compare stripped JSON/List regression checks and the current optimized
  portfolio distribution; retain neutral and negative outcomes.
- [ ] Compare load-to-first-result medians, index build work, retained bytes,
  late-loaded/native module behavior, teardown, and both `rxvm`/`rxbvm`.
- [x] Select one production approach: T6; retain A/A-prime/C as measurements,
  reject text-only hot edges and a constant-pool seed as the primary shape.

### T6 implementation milestone 1

- [x] Freeze the fixed-width little-endian 007 header and six-section directory,
  with one-module, concatenated, linked multi-module, 006-rejection, and corrupt
  fixtures.
- [x] Add the dedicated compiled `rxbin` library for the checked 007 codec,
  signature handling, semantic graph construction, and immutable indexes;
  keep OS-specific services in `platform`.
- [x] Build the same semantic facts in RXAS and rebuild/re-ID/reindex the selected
  image in RXLINK, retaining canonical metadata for generic introspection.
- [x] Add numeric type/member/factory operands, runtime graph/type identity, and
  indexed relationship, declaration, dispatch, and provider queries in both VMs.
- [x] Keep language selection policy outside the structural graph: the cREXX view
  admits signature-eligible candidates and the VM retains factory scoring and
  tie-break decisions.
- [x] Add generation-guarded method/factory instruction-site caches over the
  sealed graph, with cache side tables owned by the runtime module.
- [x] Preserve late/native correctness by rebuilding and coherently publishing
  complete process-local callable/factory bindings after a successful late
  link, with canonical-text compatibility across graph identities. An
  append-only incremental overlay is a possible future optimization, not an
  NR-04A completion requirement.
- [x] Migrate RXAS, RXLINK, RXDAS, RXVM/RXBVM, compiler imports, RXSEQ, tests,
  native archives, and the installed `crexx`/`crxc`/`ccomp` native-link surfaces
  atomically to 007 with no 006 product fallback.
- [x] Rebuild every generated Debug RXBIN, pass focused format/graph/link/runtime
  coverage and all 1,846 Debug CTests, and prove an installed `crexx -native`
  link finds `librxbin.a` and executes its result.

### T6 post-milestone measurement and closeout slices

- [x] Correct the split-read `MTIME` measurement defect and make the CTest-only
  RexxCPS count override explicit and noncanonical.
- [x] Take an early end-to-end RXBIN 007 measurement against audited retained
  baseline evidence; stop for direction before physical-layout, cache, overlay,
  or graph-harness refinement.
- [x] After Adrian requested attribution, build the standalone `rxgraph_bench`
  target over production `rxbin`, exercise retained linked images and a fresh
  RXAS image, and retain serial primitive/size measurements.
- [x] Review graph population, codec/load copies, VM `ProcRef` binding,
  relationship/dispatch/factory hot paths, generic `value` growth, and 006/007
  section-size attribution.
- [x] Record C1/C2/C3 `TYPE_CONST` versus descriptor-table layouts and R0-R4
  repair options in `NR-04A-RXBIN-007-IMPLEMENTATION-REVIEW.md`.
- [x] Obtain Adrian's selection before changing the production representation:
  retain RXBIN 007 and the existing call sites, use a C3-style process-local
  descriptor/view owned by `rxbin`, and take an immediate baseline before
  changing VM procedure binding.
- [x] Implement Adrian's selected one-time dense callable binding plus caches:
  bind every portable callable ID to `proc_runtime *` during semantic-generation
  rebuild, materialize bound factory/provider rows, use a two-way method-site
  cache, and cache factory buckets/direct single-provider targets.
- [x] Extend the isolated harness from selecting a known-valid graph factory to
  auditing every graph-bearing operand in a real executable image. It exposed
  two providerless factory operands in the first 007 image.
- [x] Fix source-short versus canonical factory return-type remapping in the
  graph builder/linker. The rebuilt image has 24 provider-backed factories
  rather than 25 buckets / 24 providers, and the operand audit reports zero
  providerless sites.
- [x] Run the immediate focused Release gate. Final `rxvm` retained process,
  method and factory-region medians are 48.965 ms, 37,044 us and 7,905 us,
  versus 204.432 ms, 45,591 us and 152,985 us at exact pre-007 commit
  `7a599906b`. Focused Debug coverage is 82/82 green.
- [x] Audit runtime-relevant changes from 006 to current. The rejected
  per-query `runtime_graph_procedure()` scan was new in 007; 006 stored bound
  procedure pointers in its registries. The current scan is cold rebuild-only,
  graph hot helpers are inline, and `value` is 248 bytes versus 256 in 006.
- [x] Prototype canonical-section compression in isolation using the former
  006 LZSS codec. On fresh linked retained images it reduces interface from
  126,972 to 56,316 bytes and RexxCPS from 869,908 to 349,292 bytes without
  changing the graph representation; compressing every current section reaches
  37,458 and 273,858 bytes respectively.
- [x] Prototype and semantically validate a compact/narrow graph seed. The
  linkable full form plus compressed non-graph sections reaches 31,880 bytes for
  interface and 259,144 for RexxCPS; the terminal dynamic form reaches
  31,624/254,832. Both rebuild the production graph and keep its control-cost
  hot view, but dynamic generic-builder reconstruction costs 267 us and 3.349
  ms on those linked images (full costs 279 us/3.913 ms).
- [x] Prototype and semantically validate a minimal link-resolved runtime seed
  with constant-pool string references plus inline fallback, direct
  relationship/declaration walks, dynamic procedure references, assignability,
  dispatch, ordered factory/provider data, direct walk ranges and name indexes.
  It reaches 29,384 bytes for interface and 247,848 for RexxCPS, materializes in
  5 us/28 us including seed unpack, and preserves approximately 0.9 ns
  dispatch/factory/type primitives. Relationship bucket lookup is 0.86-0.90 ns
  and positive type-name search is 15-26 ns.
- [x] Measure the unrefined full-general-plus-resolved combination. It reaches
  33,340 bytes for interface and 262,762 for RexxCPS while allowing the runtime
  to decode only the 5-us/28-us resolved section. This is an upper bound because
  it duplicates runtime facts; a selected split design would need a narrower
  tooling/policy residual PoC.
- [x] Regenerate standalone and retained linked fixtures through real RXAS and
  RXLINK invocations and retain exact hashes, sizes, commands, section results,
  decoder equivalence checks and hot measurements in
  `evidence/2026-07-16-nr-04a-rxbin-size-options/`.
- [x] Obtain Adrian's physical-format selection before changing production
  serialization. Adrian selected the closest current shape: keep complete,
  equivalent, re-linkable RXAS/RXLINK images and add transparent compression
  to the shared 007 section codec. Compact/split graph seeds remain optional
  follow-on designs rather than this slice.
- [x] Implement deterministic per-section compression in shared `rxbin`, retain
  a raw section when compression is not smaller, validate bounded exact
  expansion, and cover mixed/corrupt directory and stream cases.
- [x] Rebuild real RXAS/RXLINK images, prove linked output can be linked again
  byte-for-byte, run both VMs, and take the immediate Release gate. Retained
  interface falls from 126,972 to 37,458 bytes and RexxCPS from 869,908 to
  273,858 bytes with no end-to-end timing regression.
The authoritative bound/cache evidence is
`evidence/2026-07-16-nr-04a-bound-cache/`; the subsequent independent physical
layout comparison is
`evidence/2026-07-16-nr-04a-rxbin-size-options/`. The selected production
compression result is in
`evidence/2026-07-16-nr-04a-rxbin-007-compression/`. The runtime and gross
serialized-size gates pass. Compact graph seeds and an incremental append-only
overlay remain evidence for separately scheduled optional refinements, not
unfinished NR-04A work.

## `meta_head` traversal inventory before migration

### Executing VM and bridge

| File / current location | Consumer | Classification | Intended disposition |
| --- | --- | --- | --- |
| `interpreter/rxvmintp.c:1124` | `resolve_runtime_procedure()` (`META_FUNC`) | production hot runtime callable lookup | indexed `META_FUNC` cursor |
| `interpreter/rxvmintp.c:1306` | `runtime_lookup_contract_kind()` (`META_CLASS`, `META_INTERFACE`) | production hot runtime contract/type lookup | ordered multi-kind cursor |
| `interpreter/rxvmintp.c:1347` | `runtime_class_implements_interface()` (`META_IMPLEMENTS`) | production hot runtime contract/type lookup | indexed `META_IMPLEMENTS` cursor |
| `interpreter/rxvmintp.c:1728` | `runtime_proc_matches_signature()` (`META_FUNC`) | production hot callable/signature lookup | indexed `META_FUNC` cursor |
| `interpreter/rxvmintp.c:1838` | `resolve_runtime_source_context()` | source/debug lookup in address order | retain canonical full-chain cursor |
| `interpreter/rxvmintp.c:2119`, `:2145` | method-registry rebuild (`META_IMPLEMENTS`, `META_MEMBER`) | link/registry rebuild | indexed kind cursors |
| `interpreter/rxvmintp.c:2319`, `:2345` | factory-registry rebuild (`META_IMPLEMENTS`, `META_MEMBER`) | link/registry rebuild | indexed kind cursors |
| `interpreter/rxvmintp.c:4655` | `METALOADDATA` | generic metadata introspection in canonical order | retain the canonical raw chain outside the giant interpreter-loop cursor machinery; A-prime showed this avoids unrelated code-layout growth |
| `interpreter/rxvml.c:227` | member procedure lookup (`META_FUNC`) | production hot RXVML/ADDRESS-dependent callable lookup | indexed `META_FUNC` cursor |
| `interpreter/rxvml.c:259` | procedure/signature lookup (`META_FUNC`) | production hot RXVML/ADDRESS-dependent callable lookup | indexed `META_FUNC` cursor |
| `interpreter/rxvml.c:320` | class implements interface (`META_IMPLEMENTS`) | production hot RXVML/ADDRESS contract lookup | indexed `META_IMPLEMENTS` cursor |
| `interpreter/rxvml.c:386` | callable signature match (`META_FUNC`) | production hot RXVML/ADDRESS callable lookup | indexed `META_FUNC` cursor |
| `interpreter/rxvml.c:1719`, `:1722` | class discovery (`META_CLASS`) | generic runtime metadata discovery | indexed `META_CLASS` cursor |
| `interpreter/rxvmprofile.c:743` | callable kind classification (`META_CLASS`) | profiler/tooling-only lookup | indexed `META_CLASS` cursor |
| `interpreter/rxvmprofile.c:780` | profile callable catalog (`META_FUNC`) | profiler/tooling-only lookup | indexed `META_FUNC` cursor |

`interpreter/rxvmload.c` copies the serialized head into the runtime module and
constructs native plugin chains before `prep_and_link_module()`; these are
module preparation/lifecycle sites, not lookup scans. NR-04A adds index build at
that preparation point after native metadata is complete.

### Outside the executing VM

| Surface | Classification | Disposition |
| --- | --- | --- |
| `linker/rxlinkmain.c` metadata scans and chain copy | linker scan / canonical image construction | unchanged; outside executing runtime and serialization remains canonical |
| `linker/tests/test_linked_format.c` | format test inspection | unchanged |
| `assembler/rxasassm.c`, `assembler/rxaslib.c` | assembler chain construction | unchanged |
| `compiler/rxcpfunc.c` | compiler/import metadata scan | unchanged |
| `disassembler/rxdadism.c` | tooling/introspection scan | unchanged |
| `interpreter/rxseqfile.c` header hashing | profiler/tooling image identity | unchanged |

Breakpoint and TRACE execution primarily consume the instruction-address
metadata map built from canonical records rather than running additional
`meta_head` scans. Their behavior remains covered by focused regressions.

## Runtime index contract

- Walk each serialized module metadata chain exactly once at runtime-module
  preparation.
- Store runtime-only entries grouped by `META_*` kind. Each entry carries the
  constant-pool offset plus its original chain ordinal.
- Preserve module order, original within-kind order, and original cross-kind
  order for multi-kind queries by merging on the saved ordinal.
- Keep `module->meta_head` and every serialized `next`/`prev` link untouched.
- Validate offsets, entry kinds, next links, and cyclic/overlong chains; use the
  VM's existing fail-fast invalid-image and OOM policy.
- Empty kinds have stable zero-length spans. Shared-pool offsets remain relative
  to the owning runtime module's `segment.const_pool`.
- Build after ordinary, linked, late-loaded, dynamic native, and statically
  linked native module metadata is complete; free the one retained allocation
  in `rxfremod()`.
- No serialized structure, RXBIN version, public header ABI, module order,
  duplicate, case, or first-match semantic changes.

## Traversal counter contract (defined before optimization)

The profiler-only vocabulary will use stable lookup categories and metadata
kinds. Schema 5 records:

- `metadata_lookup`: one row per operation/category with query, hit, miss,
  invalid, and degraded counts;
- `metadata_visit`: entries examined by operation and actual `META_*` kind;
- `metadata_index_build`: indexed entries by kind, plus modules, total offsets,
  retained bytes, invalid, and degraded accounting.

Semantics:

- A query is one logical operation across the ordered module set, not one query
  per module.
- A hit is a query that returns or accepts at least one matching result; a miss
  is a completed query with no result. Enumeration/rebuild operations count as
  hits when they accept at least one entry and misses when they accept none.
- `metadata_visit` increments exactly once for every metadata offset returned
  to a consumer. The sum of per-kind visits for an operation must equal its
  examined total.
- Index-build traversal is setup accounting and never contributes to
  steady-state lookup visits.
- Invalid means a structurally invalid offset/type/link or impossible cursor
  state. Degraded means a requested index path fell back to a full scan. The
  production policy is fail-fast construction and no fallback, so valid proof
  runs must report zero for both.
- Full-chain source/debug/introspection operations may visit
  `META_SOURCE_STEP` and `META_TRACE_EVENT`. Ordinary contract, type, callable,
  interface, and RXVML/ADDRESS-dependent operations must report zero visits for
  both kinds.
- Counters and their strings/API are compiled out when
  `CREXX_VM_PROFILING=OFF`; profiling remains runtime-off unless explicitly
  selected.

Proposed operation vocabulary (adjust only to match audited call boundaries):
`callable_resolve`, `contract_kind`, `contract_implements`,
`callable_signature`, `interface_method_registry`,
`interface_factory_registry`, `rxvml_member`, `rxvml_callable`,
`rxvml_contract`, `rxvml_signature`, `rxvml_class_discovery`,
`source_context`, `metadata_introspection`, and `profile_catalog`.

## Baseline ledger

This and the following implementation/test/validation ledgers describe the
superseded candidate-A path. Their unchecked items are historical and are not
active NR-04A closeout work.

Evidence root: `performance/evidence/2026-07-16-nr-04a-kind-index/`

- [ ] repository/host/compiler/build provenance retained
- [ ] ordinary profiling-off Release `rxvm` and `rxbvm` identities and hashes
- [ ] baseline product binaries copied without changing source scope
- [ ] exact retained and stripped linked images built from identical inputs
- [ ] exact image hashes and metadata counts retained
- [ ] serial raw samples retained for all eleven optimized workloads
- [ ] representative unoptimized and lifecycle samples retained
- [ ] schema-4 baseline profiles and instruction counts retained
- [ ] baseline mixed-chain traversal evidence retained where practical
- [ ] prior NR-03 and NR-05 evidence left untouched

## Implementation ledger

- [ ] index representation and lifecycle
- [ ] validation/empty/duplicate/order/shared-pool handling
- [ ] shared indexed cursor and canonical full-chain cursor
- [ ] schema-5 state, CSV/table output, parser, fixture, and docs
- [ ] `rxvmintp.c` callable/type/contract consumers migrated
- [ ] interface method/factory registry rebuilds migrated
- [ ] `rxvml.c` callable/member/signature/contract consumers migrated
- [ ] profiler-only catalog scans migrated
- [ ] source context and `METALOADDATA` deliberately remain canonical-order
- [ ] no remaining executing-runtime hot mixed-chain scans

## Test ledger

- [ ] empty and one-of-each-kind index structure
- [ ] source/TRACE interleaving and within-kind order
- [ ] duplicate names and first-match behavior
- [ ] multiple modules/module order and shared pool
- [ ] late-loaded RXBIN and native/static plugin metadata
- [ ] teardown and repeated VM context creation
- [ ] ASSERTTYPE/ISTYPE built-ins/classes/interfaces and failures
- [ ] implementation checks, dynamic method/factory selection, signatures
- [ ] RXVML/ADDRESS discovery and dispatch
- [ ] rxvm/rxbvm and optimized/unoptimized focused coverage
- [ ] counters reconcile and ordinary lookups visit zero source/TRACE
- [ ] source/TRACE/debug/introspection counters retain legitimate visits
- [ ] panic, breakpoint, trace-event, signal-source, and METALOADDATA tests
- [ ] interface/link/RXPA/late-load registry behavior
- [ ] profiling-off symbols/strings/help and image hash stability

## Validation ledger

- [ ] affected Debug targets built
- [ ] focused ordinary Debug CTest passed
- [ ] affected profiling Release targets built
- [ ] focused profiling schema/counter CTest passed
- [ ] affected ordinary Release targets built
- [ ] paired retained/stripped instruction counts identical
- [ ] retained exact-image portfolio benefit reproducible
- [ ] stripped performance has no unexplained material regression
- [ ] lifecycle/index-build cost has no unexplained material regression
- [ ] focused Debug ASan/LSan passed via `tools/asan-run.sh`
- [ ] broad Debug CTest passed with `--parallel 30`
- [ ] `git diff --check` passed
- [ ] evidence checksums independently verified
- [ ] roadmap completed only after all exit criteria pass
- [ ] this temporary worklist removed
- [ ] one focused local commit created; nothing pushed

## 2026-07-16 historical candidate A progress checkpoint

- The isolated candidate had a runtime-only per-module kind index,
  canonical/indexed cursors, lifecycle build/free, audited hot-consumer
  migrations, and schema-5 counters. `METALOADDATA` and source/panic lookup
  used the full cursor. This code is no longer present in the selected T6
  production tree.
- Structural Debug and profile counter reconciliation tests pass. The
  interleaved all-kind/shared-pool fixture repeats build/free 100 times.
- Frozen starting-commit product/profile binaries and exact retained/stripped
  images are under `/Users/adrian/CLionProjects/CREXX-NR04A-baseline-7a599906b`.
- Checksum-closed `baseline-product`, full-scan `baseline-traversal`, indexed
  `candidate`, fixed-count, and replicated heavy A/B bundles have been
  captured. They will be regenerated once more after the final runtime edit.
- Full scan versus indexed normalized visits observed so far:
  `contract_kind` 2733.727 to 3.056 offsets/query (99.888% lower), and
  `contract_implements` 3642.250 to 17.991 (99.506% lower). Indexed ordinary
  lookups report zero source/TRACE visits and zero invalid/degraded state.
- Replicated fixed-count count=100 product A/B medians show retained metadata
  improving 25.407% and 24.963%. Stripped-image change is -1.763% and +0.625%
  across the same two alternating blocks. Retained/stripped profiled execution
  is exactly 19,362,656 instructions in both cells.
- The candidate's remaining actions were superseded by the T6 selection. Its
  retained evidence is comparator input only; current work resumes at the T6
  post-milestone measurement slices above.

## 2026-07-16 T6 milestone 1 checkpoint

- The portable RXBIN 007 container, sealed semantic graph, numeric graph
  operands, RXAS construction, RXLINK rebuild, runtime graph identities, RXDAS
  validation, and dedicated `rxbin` library are implemented. The append-only
  overlay, site caches, and representation comparisons remain open.
- The final Debug rebuild regenerated all dependent RXBINs and completed 1,182
  build steps. Focused graph/interface/runtime/wrapper coverage passed 121/121;
  the full RXLINK/RXDAS group passed 33/33; and the broad Debug suite passed
  1,846/1,846 with `--parallel 30`.
- A temporary-prefix install contained `bin/librxbin.a`; the installed
  `crexx -native` command linked with `-lrxbin`, produced the native
  `explicit_header` executable, and that executable printed `EXPLICIT`.
- The full sweep exposed and now covers two real boundary cases: imported
  factory metadata derives its return type from the owning qualified class,
  and invalid factory signatures do not enter the pre-indexed provider view.
- No T6 performance profile, physical-layout comparison, Release portfolio, or
  sanitizer claim is made by this checkpoint. Those are the next directed
  milestone, not prerequisites for the requested design/build/CTest gate.

## 2026-07-16 T6 early measurement checkpoint

- Fixed `MTIME` and `XTIME "U"` to derive seconds and microseconds from one
  `gettimeofday()` snapshot. RexxCPS now rejects positional overrides, marks
  `--smoke-count` as noncanonical, prints effective provenance, and the formal
  runner requires the canonical no-argument marker.
- The focused Debug build and seven focused/fixture tests passed: both `TIME`
  tests, both explicit RexxCPS smoke tests, both canonical runner tests, and the
  linked optimized artifact fixture.
- Audited existing raw evidence and found no retained 86,400-second process
  sample or 6/12-CPS RexxCPS signature. Existing pre-007 samples therefore
  remain input. No new 006 control was retained; three cells that had already
  completed when the unnecessary rerun was stopped were removed.
- The 007 `rxvm` interface benchmark regressed 81.7% retained and 76.9%
  stripped in process time; its method/factory timed regions regressed
  71.0-95.8%. Canonical RexxCPS fell 14.0% against the retained clean O3
  baseline. Linked image size grew 3.24-4.64x in the measured cells.
- Exact 007 samples, raw output, provenance, images, and interpretation are in
  `evidence/2026-07-16-nr-04a-rxbin-007-early-baseline/`. The result triggers
  the early stop/replan gate; no graph-layout harness, cache/layout PoC,
  overlay, full portfolio rerun, sanitizer sweep, or production closeout
  follows without Adrian's direction.

## 2026-07-16 RXBIN 007 implementation review checkpoint

- Adrian directed a full design/size review and a seconds-scale isolated
  harness before any further integrated work. `rxgraph_bench` now links only
  production `rxbin`, builds independently, loads real retained/RXAS 007
  images, and reports primitive latency plus serialized and retained size.
- Serial isolated measurements show exact support at 0.9-1.2 ns, but positive
  transitive support at 32-38 ns and negative support at 49-73 ns. The
  implementation allocates and breadth-first-walks relationships for every
  query. A scratch precomputed bit test measures 0.85-0.98 ns and a direct
  bound-target load 0.94 ns, proving the intended effectively-control-cost
  shape. Numeric dispatch/factory graph primitives are 3-5 ns, but the VM then
  scans modules and procedures to bind each portable callable on every
  selection.
- `value` grew from 256 to 272 bytes and gained graph/ID stores in zero/copy/
  move paths. This is an unrelated-hot-path risk consistent with the RexxCPS
  warning and must be removed or disproved.
- The 3.24-4.64x file increase is mostly explained by 007 storing the formerly
  compressed constant/metadata pool expanded. The graph is still materially
  over-broad: all `META_FUNC` records become callables and graph-local text
  repeats canonical semantic text.
- The review recommends one context-canonical type pointer, inline equality/
  assignability-bit tests, already-bound dispatch rows, and direct factory
  buckets. It retains three portable/runtime ownership choices: a
  `TYPE_CONST` that is the descriptor, a `TYPE_CONST` pointing to a context
  descriptor, or a separate compact seed and descriptor table. No choice has
  been implemented.
- Full findings, performance gates and R0-R4 repair options are in
  `NR-04A-RXBIN-007-IMPLEMENTATION-REVIEW.md`; retained exploratory results are
  under
  `evidence/2026-07-16-nr-04a-rxbin-007-early-baseline/isolated-rxgraph/`.

## 2026-07-16 C3-style/R1 candidate checkpoint

- Adrian selected the shortest repair that preserves the current 007
  integration: keep the six-section seed and existing consumers, but replace
  the `rxbin` process-local lookup shape and value identity before measuring
  any VM-specific target-binding optimization.
- `rxbin` now materializes a dense transitive assignability bitset, dense
  `uint32_t` dispatch/factory views, direct provider ranges and stable immutable
  type descriptors after build or checked deserialization. Numeric support is
  0.69-0.70 ns and the actual descriptor path is 0.92-1.07 ns in the retained
  interface/RexxCPS images. Descriptor dispatch is about 0.92-0.93 ns.
- A VM `value` now stores one `RxGraphTypeRef *`; name, length, graph and ID live
  in the descriptor. `sizeof(value)` is 248 bytes, down from the rejected 272
  bytes and below the pre-graph 256 bytes. Zero/copy/move each perform one type
  pointer store rather than four field stores.
- The unchanged RXBIN images prove this is a process-local implementation
  comparison. Runtime graph memory rises from 27,023 to 50,703 bytes for the
  interface image and from 81,576 to 150,048 bytes for RexxCPS, principally
  from the two dense type-by-member `uint32_t` views; serialized size is
  unchanged.
- The first integrated `rxvm` interface medians are now +5.3% retained and
  +7.0% stripped versus exact pre-007 evidence, rather than +81.7%/+76.9%.
  Method time is still +9.9%/+13.2%, while factory time is +5.7%/+4.6%.
  Canonical RexxCPS reaches 1,129,206 CPS, +31.7% versus the retained clean O3
  comparator and +53.1% versus the first 007 gate.
- Release graph/direct workload checks pass. The focused Debug graph,
  interface, late-load and RXVML sweep passes 81/81. Raw serial results and
  provenance are in `evidence/2026-07-16-nr-04a-c3-candidate/`.
- The deliberate stop leaves `runtime_graph_procedure()` unchanged. The next
  decision is the process-local callable-to-`proc_runtime *` binding shape;
  no site cache, compression, graph-scope reduction or closeout work has begun.

## 2026-07-16 bound/cache and factory-remap checkpoint

- Adrian selected one-time dense callable/factory bindings plus caches. The VM
  builds those process-local tables only when a semantic generation changes;
  sealed method/factory selection no longer scans modules or procedures.
- A two-way method-site cache and factory bucket/direct-target site cache are
  VM-owned and generation guarded. Portable RXBIN data contains no process
  pointers and serialized instructions remain immutable.
- The enhanced graph harness found that both executable factory sites in the
  first 007 linked image referenced a duplicate providerless factory. A valid
  source-short return type had not matched its canonical declared class type.
  Semantic signature matching fixes the producer/remap and the rebuilt image
  reports zero providerless factory sites.
- Final profiling-off `rxvm` retained medians are 48.965 ms process, 37,044 us
  method and 7,905 us factory-region, versus 204.432 ms, 45,591 us and
  152,985 us in exact pre-007 evidence. `SRCMETHODSEL`/`SRCFPROCSEL` attribution
  is about 14 ns each; 006 was 21 ns/448 ns. Focused Debug coverage passes
  82/82 and the final canonical RexxCPS smoke reports 1,132,602 CPS.
- Runtime performance passes this focused gate. The subsequent disposable
  size PoCs prove the dominant compression recovery and compare two graph
  shapes on fresh RXAS/RXLINK fixtures. Compression plus a compact dynamic seed
  reaches 31,624 bytes for retained interface and 254,832 for linked RexxCPS;
  compression plus a minimal resolved runtime seed reaches 29,384/247,848 with
  5-us/28-us materialization and control-cost hot access. Keeping the current
  full compact seed beside it costs 33,340/262,762 before residual deduplication.
- The next action is Adrian's choice between a compact general graph and a
  separate resolved runtime plus tooling/policy representation. Do not rebuild
  the integrated compiler/VM, run broad closeout, or write final NR-04A reports
  before that format/scope selection.

## 2026-07-16 selected RXBIN 007 compression checkpoint

- Adrian selected the closest current shape: RXAS and RXLINK retain the same
  complete, executable, re-linkable six-section image, while shared `rxbin`
  compresses each finished section only when the stored form is smaller.
- Production output matches the PoC prediction exactly. Retained interface is
  37,458 bytes and retained RexxCPS is 273,858 bytes, reductions of 70.50% and
  68.52% from the fresh uncompressed 007 images. A retained interface image
  re-links byte-for-byte and runs under both VMs.
- Median complete interface-image load/materialization is 106 us versus 79 us
  uncompressed. Descriptor support/dispatch and factory/provider access remain
  approximately 1 ns. Same-cell process, method, and factory medians move by at
  most 0.85% except a 2-3% stripped `rxvm` improvement. Canonical RexxCPS is
  1,120,626 CPS for `rxvm` and 1,135,200 for `rxbvm`.
- Focused Debug format/graph/link/RXDAS/interface coverage passes 8/8. The
  obsolete size and alternate-seed PoC target/sources are removed;
  `rxgraph_bench` remains an `EXCLUDE_FROM_ALL` production-library regression
  harness.
- Raw evidence and the selected-format verdict are in
  `evidence/2026-07-16-nr-04a-rxbin-007-compression/`. The physical-format stop
  is resolved; compact graph seeds are optional future refinement evidence.
- Final closeout found that a graph-local factory binding could select a
  catch-all provider before considering a higher-scoring provider from another
  loaded 007 image. Factory bindings now aggregate the validated process-wide
  provider registry after linking. The minimal CMS reproducer, all six ADDRESS
  integration fixtures, and the LLM fixture under `rxbvm` pass.
- The final Debug rebuild succeeds and the broad suite passes 1,846/1,846.
  This closes NR-04A; no append-only overlay, alternate graph seed, sanitizer,
  cross-platform, or additional portfolio sweep is carried as required work.
