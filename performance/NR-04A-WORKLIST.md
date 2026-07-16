# NR-04A resumable worklist: kind-index runtime metadata and scan counters

Status: T6/RXBIN 007 milestone 1 implemented and Debug-green; profiling and
physical-layout PoCs next

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
- [ ] Add the late/native append-only overlay and generation-guarded site caches.
  Milestone 1 retains canonical-text compatibility across graph identities.
- [x] Migrate RXAS, RXLINK, RXDAS, RXVM/RXBVM, compiler imports, RXSEQ, tests,
  native archives, and the installed `crexx`/`crxc`/`ccomp` native-link surfaces
  atomically to 007 with no 006 product fallback.
- [x] Rebuild every generated Debug RXBIN, pass focused format/graph/link/runtime
  coverage and all 1,846 Debug CTests, and prove an installed `crexx -native`
  link finds `librxbin.a` and executes its result.

### T6 post-milestone measurement and closeout slices

- [ ] Compare candidate name-hash, adjacency, declaration, callable, dispatch,
  and provider layouts on construction time, lookup/traversal, and retained and
  serialized bytes.
- [ ] Measure 007 file size, RXAS/RXLINK time, validation/binding time, and
  load-to-first-result against retained A/A-prime/C evidence.
- [ ] Compare assignability and resolved dispatch-view encodings without moving
  inheritance/override/default/provider selection rules into the graph library.
- [ ] Compare cold numeric lookup plus monomorphic, bimorphic,
  small-polymorphic, and megamorphic site caches in both VM modes.
- [ ] Measure the late/native overlay's publish/rebuild cost, generation
  invalidation, first miss, and refilled steady state.
- [ ] Run sanitizer, Release, lifecycle, exact-image, full portfolio, and
  cross-platform validation before production performance closeout.

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
