# New Build System Vision and Migration Plan

- **Status:** Design proposal; no build implementation has been changed
- **Baseline:** `origin/develop` at `dc44d92909e706adc575932ba9ae72f2d6f05b7d`
- **Planning branch:** `temp/newbuild`
- **Date:** 2026-08-27

## 1. Purpose

The new build should make the cREXX product structure understandable to a
human, expose every real dependency to the build engine, exploit safe
parallelism from the start, and use the resulting workload to qualify cREXX's
own file, hashing, process and task facilities.

The desired end state is not a collection of faster shell scripts. It is one
declarative build graph with:

- clear product layers;
- explicit execution waves where a barrier is genuinely required;
- independently runnable actions within each wave;
- isolated work and publication locations;
- recorded source, RXAS and RXBIN dependency resolution;
- metadata treated as a first-class generated product;
- resource-aware parallel scheduling;
- layered QA with defined coverage; and
- static and dynamic checks for undeclared dependencies and file races.

CMake and Ninja should remain responsible for the host C/C++ bootstrap. Once
the necessary Level B runtime and libraries exist, a cREXX Level B builder
should become the normal orchestrator for post-bootstrap work. Both parts must
consume the same graph rather than encode two build systems independently.

## 2. Governing principles

1. **A product layer and an execution wave are different concepts.** Product
   layers explain the system. Execution waves express only the dependencies
   that prevent concurrent work.
2. **One generated path has exactly one owner.** No action may delete, replace
   or append to another action's output.
3. **Every generated input has a declared producer.** Directory order,
   timestamps and a previous build must never make a clean build succeed.
4. **Import resolution is part of the action identity.** The source roots,
   binary roots, allowed artifact kinds, search order and selected providers
   must be declared and recorded.
5. **Metadata is a semantic artifact.** Compiler, assembler, linker and
   disassembler metadata behaviour is included in correctness, dependency and
   reproducibility checks.
6. **Parallel by construction.** Serialization is permitted only for a real
   dependency, a measured resource limit or an intentionally exclusive
   facility.
7. **Tests consume an already built product.** Normal CTest execution must not
   invoke a nested build in the same tree.
8. **Clean builds are the design point.** Incremental and cached builds are
   optimisations of a correct clean graph.
9. **Self-hosting is staged and reversible.** The CMake/Ninja bootstrap remains
   a reference path until the cREXX builder proves graph, artifact and QA
   parity.
10. **Aggressive simplification is allowed on this branch.** Unnecessary
    dependencies and compatibility scaffolding may be removed, but each
    removal must be justified by the declared graph and qualified against a
    clean `develop` reference build.

## 3. Current-state findings

The present build already contains useful foundations, including shared VM
object libraries, explicit class-library and Level C dependency tables, a
private Level G HTTP staging area, and CTest fixtures. It also exposes several
forms of implicit coordination that should not survive the migration:

- Level B BIF modules are forced through a predecessor chain rather than
  expressing only their semantic dependencies.
- Several library targets delete a shared consolidated image when rebuilding
  an individual member.
- the common `bin` directory acts as both publication directory and compiler
  import root, so stale and in-progress products can influence resolution;
- broad CTest fixture use can trigger builds while tests are running;
- some generated-runtime suites are globally gated by one fixture even when
  their artifacts are independent;
- many custom targets participate in the default build, which makes the
  product boundary harder to see;
- examples, demonstrations and optional remote/parser facilities are mixed
  into ordinary configuration or build paths; and
- CMake currently defines no named job pools for high-memory compilations,
  link work or exclusive generators.

`ninja -t missingdeps all` reports no depfile-known generated-file omission in
the existing configured Release graph. That is useful but limited evidence: it
does not detect undeclared custom-command reads, two writers to the same path,
delete-versus-read conflicts, or resolution against a stale artifact.

The migration should retain the strong parts of the current graph while
replacing serialized chains, shared destructive cleanup and build-during-test
coordination.

## 4. Human-facing product layers

The documented product model should use these layers:

| Layer | Contents | Result |
| --- | --- | --- |
| C0: host foundations | platform support, generators, AVL tree, RXBIN support and native third-party inputs | native objects and generator tools |
| C1: core C toolchain | VM variants, assembler, linker, compiler, disassembler, contract tools and native packaging helpers | runnable bootstrap toolchain |
| B0: Level B bootstrap | core Level B BIF modules and the Level B library image | `library.rxbin` and module artifacts |
| X: certified exits | exit-token support and certified exit modules | certified exits image |
| B1: Level B substrate | class library, required native providers, hashing/filesystem/task primitives and build-runner substrate | self-host-capable Level B environment |
| C: core cREXX tools | Level C library, RexxScript, preprocessor, debugger and other core REXX-based tools | normal developer tool suite |
| G: Level G library | network and other Level G facilities | Level G modules/images |
| L: Level L libraries | independent specialist libraries such as TinyExpr | Level L modules/images |
| Product | installed driver, runtime composition, packages and install tree | installable cREXX product |
| Optional | examples, demonstrations, contributions, benchmarks and experiments | explicitly requested auxiliary artifacts |

This taxonomy must be visible in documentation and command names. It must not
create false barriers: for example, an independent Level L library can build
at the same time as Level G or a Level C lane if its declared prerequisites are
ready.

## 5. Proposed execution waves

The initial graph should use these coarse waves. The manifest may expose more
fine-grained dependencies within them, but no action in a later wave starts
until the stated barrier is satisfied.

```text
Wave 0  Resolve configuration, host tools and pinned external inputs
   |
Wave 1  Build host generators and native foundations
   |
Wave 2  Build and verify the core C toolchain
   |
Wave 3  Build the isolated Level B bootstrap
   |
Wave 4  Build certified exits
   |
Wave 5  Build Level B class/native substrate and the cREXX builder
   |
Wave 6  +----------------+----------------+----------------+---------------+
        | Level C lane   | Level G lane   | Level L lanes  | debugger/etc  |
        +----------------+----------------+----------------+---------------+
   |
Wave 7  Assemble/install product; optionally build examples, demos and contrib
```

Within a wave, actions run as soon as their direct prerequisites are complete.
A wave barrier is a human-readable safety boundary, not a substitute for the
fine-grained DAG. As the graph is proven, unnecessary barriers should be
removed while retaining the product-layer labels.

## 6. Dependency resolution contract

### 6.1 Existing compiler behaviour that the build must control

`rxc` has separate ordered search spaces for source and binary imports:

- source roots can provide `.crexx`, `.crx`, `.rexx` or the initial source's
  extension;
- binary roots can provide `.rxbin`, optional `.rxas`, and `.rxplugin`;
- the initial source directory is the primary source root and `-s` adds source
  roots;
- `-i` adds binary roots, while the compiler executable directory is normally
  another binary root;
- repeated `-s` and `-i` options preserve order;
- source is searched ahead of deployed binary artifacts;
- same-stem binary candidates in one root collapse to the freshest artifact,
  with `.rxbin` preferred over `.rxas` on a timestamp tie;
- `--import-rxas` enables RXAS imports; and
- `--no-exe-import` removes the compiler executable directory from the binary
  search path.

This flexibility is appropriate for interactive development, but it is too
ambiguous for a deterministic library build. A parallel build must never
present a consumer with stale source, RXAS and RXBIN candidates and allow
timestamp order to decide which one wins.

### 6.2 Hermetic build policy

Every compiler action must declare:

- ordered source roots;
- ordered binary roots;
- permitted import kinds: source, RXAS, RXBIN and/or plugin;
- whether the executable directory is visible;
- the expected provider for every non-system import;
- whether the action is bootstrap, ordinary library, tool, test or example;
  and
- the metadata preservation/stripping policy expected downstream.

The normal build should use private, immutable import roots constructed for
the action or action family. Each namespace/stem should have one eligible
provider in those roots. In particular:

- Level B bootstrap actions use only the approved bootstrap source and RXAS
  roots, with `--no-exe-import`;
- an in-progress consolidated library is never placed in a consumer's import
  root;
- old and new `library.rxbin` files never coexist as eligible providers;
- publication happens only after validation and by atomic rename; and
- timestamps never select between alternative build products.

### 6.3 Resolution evidence and depfiles

`rxc`, `rxas` and `rxlink` should gain a machine-readable dependency mode. For
each action it should emit a depfile plus a resolution record containing at
least:

- requested namespace and symbol;
- selected source/RXAS/RXBIN/plugin path;
- artifact kind and search-root index;
- content digest, size and relevant metadata digest;
- tool version and resolution flags; and
- rejected same-stem candidates, if any were visible.

The build fails if the actual resolution record differs from the manifest or
if an undeclared candidate affected the decision. This evidence also becomes
part of the cache key and reproducibility record.

### 6.4 Route-parity qualification

Focused tests must compile representative consumers through each supported
route:

1. provider source;
2. provider RXAS metadata;
3. provider RXBIN metadata; and
4. linked-library metadata where that route is supported.

The tests compare callable contracts, provider identity, overload choice,
inline metadata, class/interface metadata, diagnostics and resulting runtime
behaviour. Differences must be either eliminated or documented as deliberate
route semantics. This becomes a permanent regression group because the same
area has previously failed while a library was being built.

## 7. Metadata and artifact contract

Metadata is produced and transformed throughout the toolchain:

- `rxc` emits callable, provider, contract, class/interface, inline,
  initializer, source-step and trace metadata in RXAS;
- `rxas` validates and packs constants, metadata and the semantic graph into
  RXBIN;
- `rxlink` selects modules, remaps identifiers, checks provider signatures,
  rebuilds the canonical graph and preserves or strips defined metadata
  classes according to policy; and
- `rxdas` must round-trip supported metadata spellings without creating a
  different import contract.

Therefore an action's output is not just its executable instructions. Its
metadata is part of its public interface and must participate in:

- content hashes and cache keys;
- API/ABI compatibility checks;
- source/RXAS/RXBIN parity tests;
- link selection and initializer retention checks;
- strip/preserve policy tests;
- reproducibility comparisons; and
- installed-product qualification.

Each generated RXAS, RXBIN or linked image should have a sidecar artifact
manifest. It should record the producer, input digests, resolved imports,
exported providers/contracts, metadata classes present, strip policy, semantic
graph digest and final artifact digest. Metadata-only modules are valid outputs
and must not be discarded as empty work.

## 8. One declarative graph

The source of truth should be a versioned, human-readable manifest. CMake
adapters, the Level B builder, CI jobs and QA selection all consume it. Stage
scripts may provide convenient entry points, but may not recreate dependency
logic.

Every action record should contain:

| Field | Meaning |
| --- | --- |
| `id` | stable action identity |
| `product_layer` | C0, C1, B0, X, B1, C, G, L, Product or Optional |
| `wave` | coarse barrier membership |
| `tool` and `argv` | exact executable and arguments |
| `inputs` | declared source, generated and control inputs |
| `outputs` | all owned outputs and byproducts |
| `needs` | direct action dependencies |
| `import_policy` | ordered roots, allowed kinds and resolution flags |
| `metadata_policy` | required, preserved and stripped metadata classes |
| `work_dir` | action-private workspace |
| `resources` | CPU, memory, I/O and exclusive-resource weights |
| `environment` | explicitly allowed environment variables |
| `cache_policy` | cacheability and complete action-key inputs |
| `qa_tags` | tier, component, level, capability and platform coverage |

The manifest compiler should reject duplicate outputs, missing producers,
cycles, undeclared wave inversions, ambiguous import roots and references to
paths outside approved work/publication roots before executing any command.

## 9. Parallel execution and resource control

Parallelism is a scheduler setting over an already correct DAG, never the
mechanism used to discover dependencies.

The first profiles should be:

- `developer-fast`: up to 30 runnable actions on Adrian's current macOS
  development machine;
- `portable`: 5 runnable actions on slower or unknown hosts;
- `memory-constrained`: host-derived capacity with heavyweight VM compilation
  limited independently;
- `race-stress`: 30 actions plus deterministic delay injection and schedule
  variation; and
- `measurement-isolated`: one controlled benchmark/measurement workload with
  all unrelated build and QA activity quiescent.

CPU count alone is not sufficient. The graph needs named/weighted resource
pools for:

- high-memory optimized compilation, especially VM interpreter variants;
- link steps;
- disk-intensive archive or package assembly;
- parser-thread tooling that is proven exclusive; and
- external facilities such as network ports or a deliberately shared service.

CMake/Ninja should map these to Ninja job pools where possible. The Level B
runner should implement the same resource model rather than using a single
global thread count. Locks must identify the actual resource; they must not be
used to conceal output collisions.

Performance and build-time measurements must never share the general parallel
worker pool. The scheduler should treat measurement as mutually exclusive with
compilation, linking, correctness QA, stress QA and other benchmarks. Before a
measurement starts it records the host, build configuration, power state and
load, waits for build-owned work to become quiescent, and rejects or clearly
marks a run whose background load exceeds the approved threshold.

## 10. Output isolation and publication

Each action writes only beneath an action-specific directory, for example:

```text
build/work/<action-id>/...
build/stage/<layer>/<namespace>/...
build/publish/<configuration>/...
```

Rules:

- an action cannot delete a shared library or directory to mark itself dirty;
- archive/link assembly is a separate single-owner action;
- consumers read immutable staged artifacts, never another action's work area;
- successful outputs are validated, fsynced where required, then atomically
  renamed into the staging or publication location;
- temporary names are unique to the action and invocation;
- cleanup removes only paths owned by the selected action or complete build
  root; and
- installed-product tests use the install tree, not accidental build-tree
  fallbacks.

These rules remove the current need for serial predecessor chains and shared
clean stamps where no semantic dependency exists.

## 11. Level B builder takeover

### 11.1 Bootstrap boundary

The Level B runner becomes eligible only after Wave 5 provides:

- a qualified VM and compiler toolchain;
- `library.rxbin` and certified exits;
- required class/native libraries;
- filesystem enumeration and atomic publication;
- cryptographic hashing;
- process execution and exit-status capture;
- parallel task primitives with cancellation; and
- deterministic manifest parsing.

Before that point, CMake/Ninja drives the build. After it, the Level B runner
orchestrates Level C, G and L libraries, REXX-based tools, optional products and
their QA preparation.

### 11.2 Human-facing stage script

The Level B interface should make barriers and concurrent actions obvious. A
conceptual form is:

```text
stage "level-c-foundations" {
  parallel {
    build "rxfnsc-core"
    build "classlib-dependent-tools"
  }
}

stage "rexx-tools" requires "level-c-foundations" {
  parallel {
    build "rexxscript"
    build "rxpp"
    build "rxdb"
  }
}
```

This is illustrative, not a proposed new language syntax. The actual form must
use established Level B syntax and requires a separate design approval before
implementation.

### 11.3 Dual-run qualification

During migration, CMake/Ninja and the Level B runner should build the same
post-bootstrap slice into separate roots. Qualification compares:

- planned DAG and selected providers;
- action and metadata manifests;
- final artifact hashes where deterministic;
- semantic/disassembly comparison where non-semantic bytes vary;
- essential and smoke QA results; and
- clean, incremental and failure-recovery behaviour.

The Level B path becomes default only after repeated clean parity at job counts
1, 5 and 30 and after injected action failures leave no publishable partial
artifacts.

## 12. QA model

### 12.1 Tiers

| Tier | Purpose | Expected use |
| --- | --- | --- |
| Graph | manifest validation, duplicate writers, cycles, import ambiguity and missing producers | every configure/plan |
| Essential | minimum compiler/assembler/linker/VM and bootstrap correctness; blocks all further work | every build and commit |
| Smoke | representative end-to-end product behaviour across major layers | normal developer and PR loop |
| Comprehensive | full component, language, library, negative and regression suites | broad CI and release preparation |
| Qualification | clean build, install/package, source-RXAS-RXBIN parity, reproducibility and supported-platform gates | scheduled and release gates |
| Stress | parallel schedule fuzzing, task load, cancellation, repeated clean/incremental builds and race detection | scheduled and focused concurrency work |
| Measurement | controlled performance/build-time evidence with retained host, configuration, power and load data | explicit isolated runs only; never concurrent with build, QA, stress or another measurement |

These tiers are orthogonal to existing compiler-internal stage labels. Test
labels should independently encode tier, component, product level, capability,
mode and platform so a user can answer both "how much QA?" and "what does it
cover?"

### 12.2 Fixture policy

All normal artifacts and fixtures are built by an explicit `qa-prep` graph
before CTest starts. Tests then:

- read immutable product/fixture artifacts;
- write only to a unique per-test directory;
- use unique ports, databases and temporary names;
- declare a resource lock only for a real exclusive facility; and
- never rebuild or delete a shared runtime image.

Multi-step test scenarios can have their own setup/run/cleanup DAG, but they
must be materialized before or outside the general parallel CTest pool. A test
failure must not invalidate another test's inputs.

### 12.3 Timeouts and heavy-load interpretation

A timeout has different meanings in normal QA and deliberate load testing:

- essential, smoke, comprehensive and qualification tests run under a defined
  resource profile and must meet their maintained timeouts there;
- race/stress runs may deliberately overload the host, and a timeout that occurs
  only beyond the supported profile is retained as a capacity/stress result
  rather than automatically treated as a functional failure;
- the same timed-out test must be rerun in isolation or under its normal QA
  profile to distinguish starvation from a correctness defect; and
- performance tests are not used as correctness fixtures and never run in the
  heavily parallel correctness/stress workload.

Timeouts should be declared per test class from retained normal-load evidence,
not globally inflated until overloaded runs happen to pass. Reports must state
the scheduler profile, job count, host load and whether the result is a normal
QA failure, a stress-capacity observation or an invalid performance run.

## 13. Build-race detection

### 13.1 Static graph proof

For two unordered actions `A` and `B`, safe concurrency requires:

```text
W(A) intersect (R(B) union W(B)) = empty
W(B) intersect (R(A) union W(A)) = empty
```

where `R` and `W` are the declared read and write path sets after path
normalization. In addition:

- every generated read has exactly one reachable producer;
- every output has exactly one owner;
- a directory output has an explicit ownership boundary;
- deletes are writes over the complete affected path set; and
- an action may publish only after all reads of any replaced generation are
  complete, normally avoided by immutable generation paths.

The manifest compiler can enforce most of this before a build. CMake's Ninja
generator and `ninja -t missingdeps` remain useful secondary checks for
depfile-visible generated inputs.

### 13.2 Dynamic file-access audit

Static declarations must be checked against actual execution. An audit wrapper
should record process-tree file opens, stats, directory scans, renames,
deletions and writes, then compare them with the action manifest. The exact
backend is platform-specific:

- Linux can use fanotify/eBPF or `strace`-class tracing;
- macOS can use supported Endpoint Security/DTrace-derived facilities or a
  purpose-built interposition/audit helper;
- Windows can use ETW/Process Monitor-class evidence.

The normal solution must not require weakening host security controls. A
portable compiler/toolchain-level audit is therefore also needed: rxc, rxas,
rxlink, the Level B runner and fixture helpers should directly report all files
they resolved or changed. OS tracing is the independent cross-check, not the
only source of truth.

The audit fails on:

- a read not covered by a declared input or approved system root;
- a write/delete outside the action work directory or owned output set;
- two concurrent writers to the same normalized path;
- a write concurrent with another action's read;
- an undeclared directory scan that can change resolution; or
- a published output read before its atomic commit.

### 13.3 Schedule fuzzing and reproducibility

The stress runner should execute the same clean graph repeatedly with:

- job limits 1, 5 and 30;
- seeded random readiness delays;
- reversed and randomized ready-queue ordering;
- injected tool failures before and during publication;
- filesystem timestamp equalization and perturbation; and
- repeated builds from deliberately contaminated parent directories while the
  build root remains clean.

Each successful run compares action manifests, provider selections, metadata
inventories and final hashes. A serially successful but parallel-only divergent
run is a graph defect, not a flaky-test waiver.

## 14. Migration workstreams

### Phase 0: freeze evidence and define the graph

1. Record clean `develop` Debug and Release build graphs, timings, peak memory,
   artifact inventories and QA selections on macOS and Linux.
2. Catalogue every custom target/command, output, byproduct, import root,
   cleanup path, fixture and resource lock.
3. Define the manifest schema and graph validator.
4. Add a read-only export of the current CMake graph for comparison.

**Gate:** all current generated products have an owner and provisional layer,
wave and QA classification.

### Phase 1: make the current CMake graph race-safe

1. Split work, staging and publication roots.
2. Replace shared-output deletion and append/consolidation patterns with
   single-owner assembly actions.
3. Replace predecessor chains with direct semantic dependencies.
4. Declare all custom-command outputs, byproducts, depfiles and terminal needs.
5. Add resource pools for high-memory/native-heavy work.
6. Move examples, demonstrations, contributions, downloads and experiments
   behind explicit options/targets.
7. Move all nested CTest builds into `qa-prep`.

**Gate:** clean builds pass at 1, 5 and 30 jobs; parallel execution produces no
shared-path collision and no test initiates a build.

### Phase 2: make imports and metadata deterministic

1. Add dependency/resolution reports to rxc, rxas and rxlink.
2. Give bootstrap and ordinary library actions curated immutable import roots.
3. Add sidecar action/artifact/metadata manifests.
4. Add permanent source/RXAS/RXBIN route-parity tests.
5. Add metadata preserve/strip and disassembler round-trip tests.

**Gate:** no build selection depends on timestamps, executable-directory
contents or a previous build; all selected providers are recorded and match
the manifest.

### Phase 3: introduce the Level B runner

1. Build the smallest runner over established filesystem, hash, process and
   task APIs.
2. Parse and validate the same declarative graph used by CMake adapters.
3. Implement barriers, direct DAG scheduling, resource pools, cancellation and
   atomic publication.
4. Migrate one independent Level L module as the first proving slice.
5. Migrate Level G, then Level C/RexxScript/core REXX tools.

**Gate:** dual-run graph, resolution, metadata and QA parity across repeated
clean and incremental builds at 1, 5 and 30 jobs.

### Phase 4: consolidate the product workflow

1. Make the Level B runner the normal post-bootstrap entry point.
2. Retain a bootstrap/reference comparison target.
3. Standardize install, package, examples, demos and contributions as manifest
   selections.
4. Publish build profiles and QA tiers in developer documentation and CI.
5. Remove superseded CMake/script dependency logic only after parity evidence
   is retained.

**Gate:** a fresh checkout can build, test, install and reproduce the supported
product without undeclared tools, network access, stale build products or
manual sequencing.

## 15. Proposed user interface

The final command names can change, but the workflow should offer these
concepts consistently through CMake presets/wrappers and the Level B runner:

```text
build plan                         # validate and display layers/waves/DAG
build bootstrap                    # Waves 0-5
build product --jobs 30            # normal product, no optional material
build layer G --jobs 5             # selected product layer plus prerequisites
build optional examples,demos      # explicit auxiliary products
build qa essential                 # essential preparation and tests
build qa smoke --jobs 30           # smoke preparation and tests
build qa comprehensive             # full maintained suite
build audit-races --seed <seed>     # dynamic access audit and schedule fuzzing
build explain <action-or-artifact>  # dependency, provider and metadata reasons
```

`build explain` is important for human comprehension. It should show why an
action ran, which provider was selected, which metadata contract was consumed,
what waits for it and why a barrier or resource limit applies.

## 16. Acceptance criteria

The migration is complete only when:

- the documented product layers map to queryable manifest selections;
- all generated outputs and byproducts have one owner;
- the static graph has no undeclared producer, output or unordered path
  conflict;
- actual file access agrees with declarations in race-audit runs;
- Level B bootstrap imports cannot resolve against a stale installed or
  in-progress library;
- source, RXAS and RXBIN import routes pass their declared parity contract;
- metadata generation, transformation and stripping are explicitly tested;
- clean output is independent of ready-queue order and job counts 1, 5 and 30;
- ordinary tests never build or mutate shared fixtures;
- normal QA timeouts are qualified independently from deliberate heavy-load
  stress results;
- essential, smoke, comprehensive, qualification, stress and measurement
  suites have stated coverage and stable selection commands;
- measurement suites run only in an isolated, quiescent resource profile and
  reject or mark contaminated runs;
- the Level B runner survives the full post-bootstrap workload, failure and
  cancellation tests without partial publication;
- a clean install is tested without build-tree fallbacks; and
- CI uses deliberate job/resource profiles rather than serial execution as a
  correctness mechanism.

## 17. Decisions still requiring approval

This document establishes the architecture and migration direction, but the
following should be approved separately before implementation:

1. the concrete manifest format and versioning policy;
2. the Level B runner's user-facing API or any new language/library surface;
3. the exact boundary between CMake/Ninja and the self-hosted runner;
4. whether deterministic RXBIN byte identity is mandatory or semantic
   equivalence is sufficient for specific artifact classes;
5. the supported dynamic file-audit backends per platform; and
6. the point at which the reference CMake post-bootstrap path may be removed.

## 18. Immediate next step

Build a generated current-state catalogue from the existing CMake files. It
should list every action, target, output, byproduct, cleanup path, import root,
fixture, test label and lock, then flag ambiguous ownership and resolution. That
catalogue becomes the input to Phase 0 review and the first draft of the
declarative graph; it should not yet alter production build behaviour.

## References

- `docs/ai-context/CREXX_ARCHITECTURE.md`
- `docs/ai-context/CREXX_LIBS.md`
- `docs/ai-context/RXAS_ASSEMBLER.md`
- `docs/ai-context/RXLINK_LINKER.md`
- `docs/ai-context/RXVM_INTERPRETER.md`
- `compiler/rxcpmain.c`
- `lib/rxfnsb/rexx/CMakeLists.txt`
- `lib/classlib/CMakeLists.txt`
- `lib/rxfnsc/CMakeLists.txt`
- `lib/rxfnsg/rexx/CMakeLists.txt`
- `rexxscript/CMakeLists.txt`
- `cmake/CrexxTestModes.cmake`
