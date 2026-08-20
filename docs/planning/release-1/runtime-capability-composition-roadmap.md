# Runtime Capability Composition Roadmap

Status: approved roadmap in progress. RCC-1 through RCC-4 and RCC-5A are
complete. RCC-5B implementation and its first Release verdict are accepted,
with focused mathematical-contract coverage still open. RCC-5C remains in
progress. Later RCC-5 partitions and RCC-6 through RCC-8 remain unimplemented
unless separately noted.

Date: 2026-08-20.

Decision state on 2026-08-19: the RXBIN format-boundary rule, opcode disposition
list, once-per-mutable-module-instance initializer contract, the
`name: initialiser` source spelling, and the related RCC-1 through RCC-3
implementations are accepted. Optional finalizers remain a separate decision.

## Why this is not only a deployment roadmap

"Deployment" describes how an already-designed component reaches a machine.
The decisions here start earlier: which capabilities belong to the VM, which
belong to an always-available or optional native provider, how compiled modules
declare those providers, when executable modules initialize, and which RXAS
operations remain instructions. **Runtime capability composition** covers that
complete boundary while still including build, install, native-package, and
runtime discovery.

This roadmap answers four questions:

1. whether `rx_hash` is core;
2. which parts of `system`, `rxmath`, and adjacent modules should be core,
   standard, or optional;
3. which RXAS instruction families are credible candidates for ordinary typed
   native calls; and
4. how explicit RXBIN module initialization fits the same load lifecycle.

It uses the independent role and delivery axes already proposed by the
[Release 1 component catalogue](component-catalogue/provisional-classification.md).
In particular, **core** means required language, runtime, or toolchain closure;
**standard/default** means a supported user-facing capability installed in the
normal product without making it part of the bootstrap closure. "Native" and
"always easy to find" are implementation and delivery properties, not reasons
by themselves to call something core.

Level, implementation language, and availability are independent axes. Level
B is the bootstrap-capable language subset and will increasingly host
bootstrap implementations; Level G is the normally installed standard product
profile. A library can be authored entirely in Level B and callable from a
Level B program without being guaranteed by the Level B bootstrap closure. In
that case it is **Level-B-authored, Level-G-standard**. Conversely a native
provider can expose Level-B-compatible signatures while remaining guaranteed
only in Level G. “Works when present” must not be confused with “available in
the bootstrap profile.”

The approved provider and package ID is `rx_hash`; its public namespace is
`rxhash`, and the first callable is `rxhash.sha256()`. The namespace and the
existing RXAS `rxhash` mnemonic are distinct contracts. Provider metadata uses
the stable provider ID, never a platform filename.

### Current hash names are not one facility

- Existing RXAS `rxhash` / opcode 513 implements 32-bit FNV-1a over a selected
  byte length of a UTF-8 string. The public Level B `rxfnsb.fnv()` procedure is
  a Rexx wrapper around this instruction.
- Existing RXPA `rxmath.fnv1a()` is a separate historical native FNV-1a
  implementation inside the mixed `rxmath` plugin.
- Production provider `rx_hash` publishes
  `rxhash.sha256(data = .binary) = .binary`. It is the selected native home for
  SHA-256 and a possible later consolidation point for explicitly named
  digest, checksum, and non-cryptographic hash APIs.

The roadmap's instruction-performance warning concerns only replacement of the
existing RXAS FNV-1a opcode. It does not suggest delaying the selected native
SHA-256 function.

## Recommended decisions

| ID | Recommendation | Reason |
|---|---|---|
| RCC-D1 | Classify `rx_hash` as **B+G standard/default**, not core. | SHA-256 is broadly useful and the measured native path is decisive, but neither the compiler nor a minimal VM needs the public hash API to compile and execute an ordinary program. A `crexx-rag` dependency does not make it CREXX bootstrap closure. |
| RCC-D2 | Ship `rx_hash` in the normal distribution and make its loading declarative and automatic. | Default delivery removes operational friction without weakening the meaning of core. Ordinary users should not need `rxvm -p`, a wrapper, or a remembered native link list. |
| RCC-D3 | Do not promote the current `system` or `rxmath` plugin wholesale. Split each by contract, lifecycle, quality, and delivery role. | Both bundles mix unrelated functions and maturity levels. Their directory boundaries are historical, not an appropriate product architecture. |
| RCC-D4 | Add provider dependency metadata to the compiled module path: `rxc` -> RXAS -> RXBIN -> `rxlink` -> runtime/native packager. | Compile-time signature discovery already comes directly from RXPA `ADDPROC`. The missing information is the runtime provider identity and resolution contract. |
| RCC-D5 | Add an explicit RXBIN module-initializer lifecycle after provider resolution, linking, and preparation, but before `main`. | Provider registration and Rexx module initialization are different jobs. Initializers must not be used to guess or trigger undeclared native dependencies. |
| RCC-D6 | Start opcode-to-call work with the 14 file operations. Treat existing RXAS `rxhash` and host utilities as measured follow-ons, not automatic removals. | File operations are cold host I/O with raw-pointer handles. FNV-1a hashing is library-shaped but can be hot; clock/environment/random utilities need stable contracts first. |
| RCC-D7 | Do not add a generic `HOSTCALL` instruction. | The normal typed procedure call, linker, and RXPA machinery already provide the required dispatch. A second generic call route would duplicate binding and weaken signatures. |

The instruction-family dispositions under [RXAS instruction-to-call
review](#rxas-instruction-to-call-review) are accepted as the roadmap list.
That accepts the order and evidence gates, not removal of an instruction or
reuse of an opcode number.

### When would hashing become core?

Only an actual core dependency should promote it. For example, if a future
RXBIN integrity contract, content-addressed linker cache, or package verifier
requires SHA-256 in every toolchain/runtime closure, the small internal digest
mechanism used by that feature would be core. The public hash API could still
remain standard/default: internal mechanism, public contract, and delivery
do not have to share one classification.

## Proposed capability composition

The following is the target classification, not a request to rename or move
all source immediately.

| Capability | Proposed role and delivery | Native shape | Disposition |
|---|---|---|---|
| decimal engine (`mc_decimal`) | level core / required | existing VM numeric provider | Keep core. It implements language numeric semantics and is already required by compiler/runtime closure. |
| VM control, frames, signals, typed values, references, tasks, channels | level core / required | VM instructions and internal services | Keep in the VM. These define execution or ownership semantics rather than a library algorithm. |
| socket/TLS mechanism used by the current Level G HTTP stack | Level G core mechanism / required with that runtime profile | current VM-owned registry and instructions | Keep for the first roadmap stages. Reconsider only after a session-aware core-provider model has proven equivalent ownership, worker, TLS, and failure behavior. |
| low-level `rx_io` handle and byte/text stream mechanism | level core implementation **if it replaces the current `F*` instructions** | small context/session-owned provider, statically available in required runtime profiles | Recommended first extraction. Use small integer handles with context teardown, never a `FILE *` cast into `.int`. Public filesystem conveniences remain standard. |
| `rx_hash` | B+G standard / default | small process-reentrant provider; dynamic for ordinary runtime use and static archive for native packaging | Selected home for SHA-256 and, after compatibility review, named non-cryptographic hashes/checksums. Not an all-assets native library. |
| scalar `rxfloat` (`rxmath` compatibility aliases) | Level G standard/default | process-reentrant native `rxfloat` provider backed by platform libm | Contains only coherent scalar binary-float math and constants. Its signatures are callable from Level B when the provider is installed, but it is not bootstrap closure or language numeric semantics. |
| `rxint` | Level G standard/default | Level-B-authored checked integer algorithms | Exact integer mathematics belongs in the standard math family without forcing a native dependency or claiming bootstrap availability. |
| `rxdecimal` | Level G standard/default | Level-B-authored algorithms over core `mc_decimal` arithmetic | Preserves caller-selected decimal precision; use of a core primitive does not make this higher-level library core. |
| `rxstats` | Level G standard after `BINARY-01` | separate native bulk provider over packed `rxinteger`/`rxfloat` storage | Statistics is the black-and-white Level G case: useful standard functionality, but neither language nor bootstrap closure. It must not inherit the boxed historical implementation or scalar-call shape. |
| `rxfs` | B+G standard / default | adapter over the low-level I/O/OS mechanism | Directory and file-name operations: cwd, list, test, create, remove, rename, and append. |
| `rxplatform` | standard or optional by function | narrow native provider | OS, host, user, uptime, clipboard, and beep need separate portability/privacy/UI dispositions; they must not all inherit one `system` status. |
| process and ADDRESS services | level core mechanism where required; public conveniences standard | existing structured process/channel/ADDRESS architecture | Do not revive the commented historical pipe procedures as the architecture. Use the current typed process and environment model. |
| `rxid` | optional until its randomness and contract are qualified; standard/default is a later promotion | cryptographically appropriate platform implementation | UUID generation does not belong in `rxmath`. |
| pure Rexx regex and JSON | retain their independently classified library roles | Rexx implementation, with native alternatives only when justified | Do not make native merely for consistency with this roadmap. |
| ODBC, GUI, XML, external regex, LLM/vector/database providers | integration or optional / opt-in | independently packaged providers | External service/library/platform dependencies remain outside core. |

### Partition the current `system` bundle

The registered surface currently combines directory/file functions, platform
information, clipboard/UI functions, mutable process-global values, and
developer tooling. The process/parse registrations in the source are currently
commented out, so they are not a basis for a public contract.

| Current area | Destination | Recommendation |
|---|---|---|
| `getdir`, `getCWD`, `getloadpath`, `setdir`, directory/file test/create/remove/rename/list, `append` | low-level `rx_io` plus public `rxfs` | Standard/default public API; only the low-level handle mechanism becomes core if it replaces instructions. |
| `opsys`, `userid`, `host`, `sysuptime` | `rxplatform` | Review privacy, portability, naming, and error contracts; standard or optional per function. |
| clipboard and beep | platform UI provider | Optional. They are not headless/server core. |
| `getglobal`, `setglobal` | no new core home | Deprecate or confine as legacy. Unscoped process-shared mutable state conflicts with multi-VM ownership. |
| `lmodules` | developer/reflection tooling | Developer-only unless a stable inspection API is separately designed. |
| `parse`, `parsex` | no native promotion | Keep the language/library parser authoritative; do not duplicate it in `system`. |
| historical pipe procedures | no revival | Superseded by the structured process/channel/ADDRESS work. |

The `crexx` driver currently links the broad static `system` target for a much
smaller tool need. Partitioning should let the driver depend on the narrow
provider(s) it actually uses rather than making every historical `system`
procedure appear core.

### Partition the current `rxmath` bundle

The current plugin combines libm functions, statistics, four unrelated hash or
checksum functions, UUID generation, and an `inlinec` procedure that writes,
compiles, and executes temporary C. It also contains explicitly unfinished
statistics code and at least one implementation defect, so promotion of the
bundle would promote its accidents.

| Current area | Destination | Recommendation |
|---|---|---|
| libm scalar functions and `pi`/`euler` | native `rxfloat`, canonical namespace `rxfloat`; direct `rxmath` aliases | Level G standard/default after domain, range, signal, NaN/Inf, platform, naming, and first-Release performance gates. |
| integer mathematics | Level-B-authored `rxint` | Level G standard/default. Use exact checked algorithms such as Euclid, overflow-safe integer square root, and modular exponentiation; no blanket native plugin. |
| decimal mathematics | Level-B-authored `rxdecimal` over `mc_decimal` | Level G standard/default. Inherit the caller's numeric context, use bounded guard digits, and do not round-trip through binary float. |
| mean/deviation/covariance/correlation/regression | native bulk `rxstats` after `BINARY-01` | Level G standard. Separate contract and quality gate; do not promote the historical boxed scalar-call implementation. |
| DJB2, Murmur, FNV-1a, CRC32 | the new hash provider under explicitly approved public names | Separate cryptographic digests, checksums, and table/container hashes in the API. Preserve compatibility aliases only by explicit decision. |
| UUID | `rxid` | Use a qualified random source and UUID contract; not mathematics. |
| `inlinec` | removal or developer experiment | Never standard or core. Native compilation/execution is a tool/security boundary. |

## Declarative native-provider dependencies

### Required end-to-end behavior

1. `rxc` discovers the RXPA procedure and type signature directly from
   `ADDPROC`, as it does now. No Rexx wrapper is generated merely to publish the
   signature.
2. When a compiled module retains a call to a native procedure, its RXAS carries
   the stable provider dependency as metadata. Dead imports must not create a
   runtime dependency after the normal reachability/link decisions.
3. `rxas` serializes the dependency in RXBIN metadata. The record names a
   stable provider ID which is also the artifact stem, not a path or complete
   platform filename. Required callable identities and signatures remain
   link-verifiable.
4. `rxlink` unions and deduplicates requirements, retains which modules need
   them for diagnostics, and fails on incompatible requirements.
5. `rxvm` resolves mandatory providers before normal procedure linking. It first
   accepts an already registered static provider; otherwise it searches trusted
   application/configured/install locations for `<provider-id>.rxplugin` and
   verifies the binary's manifest ID. A missing or wrong provider fails before
   any module initializer or `main` runs.
6. `crexx -native` and equivalent packagers read the same dependency records,
   select installed static archives, and retain the static RXPA registration
   anchors automatically. Users do not maintain a second provider list.
7. Explicit `rxvm -p` remains a development/override facility, not the normal
   application dependency mechanism.

The first dependency record needs a provider ID, required/optional status, and
enough callable/signature information to diagnose a mismatched implementation.
The existing RXPA manifest ABI number describes the host protocol; it is not a
provider release version. A separate provider-version scheme should be added
only when a real compatibility case requires it.

### Implemented RCC-1 record contract

The compiler emits one record immediately after the retained imported callable
signature:

```rxas
.meta "namespace.callable"=".provider" "stable-provider-id"
```

`.provider` and `.provider.required` are equivalent required spellings;
`.provider.optional` records a best-effort dependency. RXBIN 007 stores this as
`META_PROVIDER { symbol, provider, flags }`, while the matching `META_FUNC`
remains the only signature authority. `RXBIN_PROVIDER_REQUIRED` is bit 0 and no
other flag is currently valid. Provider IDs start with an ASCII letter or digit
and thereafter contain only ASCII letters, digits, `.`, `_`, or `-`.

This is a feature-gated metadata extension, not a container change. Writers set
`RXBIN007_FEATURE_NATIVE_PROVIDERS` (`1 << 4`) whenever the image contains a
provider record. Readers reject the record without that bit and reject unknown
feature bits, so the version remains RXBIN 007 under the accepted boundary.

`rxc` obtains the stable ID from the RXPA V1/V2 manifest used to import the
`ADDPROC` declaration. The DECL_ONLY static registration path carries the same
ID. Provider identity participates in duplicate-import consistency, and only a
callable declaration retained in emitted RXAS creates a dependency.

`rxlink -p requirements-file` (or `PROVIDERS` in a control file) writes:

```text
CREXX-RXPA-REQUIREMENTS 1
required<TAB>provider<TAB>callable<TAB>return-type<TAB>arguments<TAB>module
```

Rows retain requiring-module provenance for diagnostics. The linker rejects a
callable that is associated with conflicting provider IDs or signatures.

### Search and trust policy

Automatic loading does not mean an unrestricted current-directory sweep. The
implemented precedence is:

1. provider already registered in the runtime or native executable;
2. the application/container's declared local provider directory;
3. explicit runtime search paths supplied by `--provider-path`, the embedding
   API/location, or `CREXX_PROVIDER_PATH`;
4. the installed `bin/providers` directory beside `rxvm`.

The current directory should be a deliberate developer option, not a higher
priority production source. Diagnostics must report the provider ID, requiring
module, paths examined, candidate rejection reason, and whether static or
dynamic resolution was attempted.

The provider ID is the canonical artifact stem. For provider `rx_hash`, `rxvm`
opens only `rx_hash.rxplugin` in those trusted directories; it does not scan the
directory or interpret a separate sidecar. Runtime loading verifies the RXPA
binary manifest ID before `_initfuncs`, then verifies the
provider/callable/option/return/arguments metadata published by the native
module. RXPA source spelling is compared in canonical RXAS form, including
`[]` to `[*]` and insignificant signature punctuation whitespace.
`add_rxpa_provider_package()` copies the dynamic module, canonical
`rx_hash.a`/`.lib` archive, and compatibility `rx_hash_static.a`/`.lib` archive
into `bin/providers`. `crexx -native` asks `rxlink` for the same requirements,
prefers the canonical archive, falls back to the compatibility name, and uses
whole-archive/force-load semantics so registration anchors are retained.

### RXBIN format decision

The accepted boundary is structural, not merely chronological:

- remain on RXBIN 007 when provider dependencies, initializer declarations and
  their dependency edges can be encoded as feature-gated record kinds in the
  existing metadata section and existing module/graph identity machinery;
- move to RXBIN 008 if the work changes the container layout, section directory,
  header shape, module/file topology or another part of the file structure; and
- do not add a seventh section merely for these records. If the existing
  metadata home proves insufficient, that is evidence for 008 rather than a
  reason to disguise a structural change as an extension to 007.

An old 007 reader rejects the new feature bit cleanly. Any later change to the
container boundary still requires a new explicit decision.

## Explicit module initialization

This is the existing PERF3-07 `CAP-05` lifecycle item brought into the product
composition design. Three mechanisms must remain distinct:

- RXPA `_initfuncs`: registers native procedures when a provider is loaded;
- RXPA V2 session creation: creates per-VM native provider state; and
- RXBIN module initialization: executes declared Rexx/RXAS code once for one
  mutable module instance in one execution/isolate.

A module initializer must not discover that a native function is needed and
then load it. The module dependency record provides that information before the
initializer can run.

### Proposed lifecycle

```text
load RXBIN containers
  -> collect and resolve native-provider dependencies
  -> register providers and create per-VM provider sessions
  -> link all procedures/classes
  -> prepare execution images
  -> execute pending module initializers
  -> call main or the host-requested public procedure
```

### Initializer contract implemented by RCC-3

- The compiler/linker emits an explicit initializer metadata record pointing to
  the local procedure identity. The VM does not scan for a magic `_init` name
  and no wrapper `main` is required.
- A module declares zero or more initializers. They run in source declaration
  order, which is retained as metadata order when `rxlink` combines modules.
- The source form is `name: initialiser [expose variable-list]`. The name has
  the module's ordinary namespace-qualified identity for metadata and
  diagnostics, but the initializer is a private lifecycle entry point: source
  code cannot call it or export it through namespace `expose`. Its own
  `expose` clause only makes module variables visible to its body.
- An initializer takes no application arguments and returns `.void`. An
  unhandled signal or VM failure fails initialization; there is no parallel
  integer status convention.
- The unit is the **mutable module instance**, not the OS process, immutable
  RXBIN definition, program generation, submitted task label or call to a
  procedure. The initialized marker belongs to the execution/worker overlay
  beside its globals. It must never be stored in or inferred from a shared
  sealed program generation.
- Execution is serialized. Application `main`, tasks, and public host calls do
  not observe the module until its initializer has completed successfully.
- Ordering is deterministic. Unrelated modules use stable loaded/linked-module
  order. If an initializer calls an ordinary procedure in an unready module,
  the VM initializes that target module before entering the procedure. No
  separate `requires` list is needed. Re-entry through such calls detects and
  fails an initializer cycle.
- The per-instance state machine is `UNINITIALIZED -> INITIALIZING -> READY` or
  `FAILED`. The VM makes at most one initializer attempt for that instance. A
  failed instance is poisoned and uncallable; it is destroyed or excluded from
  a failed late-load transaction rather than retried over partially changed
  globals or external side effects.
- An initializer may call ordinary helpers in its own `INITIALIZING` module.
  A call into another unready module initializes that module as described
  above. An attempt to re-enter an initializer cycle fails deterministically.
- A late-loaded module follows the same transaction: resolve its providers,
  link, prepare, initialize, then publish success. A failed initializer leaves
  the new module and any new provider session unavailable rather than half
  initialized. Existing `READY` module instances are not rerun.
- Public `rxvm_run()` and `rxvm_call()` perform the lifecycle automatically;
  embedders may call the explicit idempotent `rxvm_initialize()` phase. Local
  task workers initialize their private module overlays before becoming
  eligible for requests, so shared immutable program generations never share
  mutable initialization state.
- External side effects cannot in general be rolled back. Initializers should
  therefore keep such effects idempotent and bounded even though the VM, not
  library convention, enforces the at-most-one attempt within an instance.
- Optional finalizers are a later paired design. If selected, they run only for
  successfully initialized modules, in reverse initialization order, before
  provider-session teardown. They are not required for the first initializer
  implementation.

The intended uses include assigning module globals, constructing immutable
lookup data and creating module-private singleton-like objects, caches or
resource managers. “Singleton” means one instance in each mutable module
instance: it is owned and destroyed with that execution/worker overlay. It is
not one process-global object shared by all VMs or workers. A native provider's
per-VM state remains an RXPA session, and durable application state that needs
one logical identity across workers remains a service/actor rather than a
module singleton.

### Task and worker scope

The existing task implementations do not all use the same physical lifetime,
so “once per task” is not a sufficient module-initializer rule:

| Execution site | Mutable module-instance lifetime | Initializer consequence |
|---|---|---|
| standalone `rxvm`/`rxbvm` or one embedded context | the context's module overlay | once before `main` or the first public host call |
| local task-pool worker | one worker context and overlay currently serve successive requests | once when that worker attaches or advances to the module generation, before it reports ready; not once per request |
| isolated-process task | every request creates a fresh executor and VM context even when the process is warm | once per task, because the task has fresh module instances |
| another independent embedded VM or worker | distinct overlays over any shared immutable generation | once for every distinct module instance |
| late-loaded/derived generation | existing overlays remain and only new module overlays are materialized | once for each new module instance; never rerun an existing `READY` module |

This does not weaken the task ownership boundary. Live values do not cross
executions, and task code must not depend on persistent worker-local globals.
The current local provider nevertheless reuses one execution serially, so its
physical globals can remain between requests even though that retention is not
a task API guarantee.

Merely rerunning module initializers before each local request would not create
task isolation: globals that an initializer does not overwrite, references,
native session state and external resources could still remain. If a future
local provider promises fresh state for every task, it must create or completely
reset a task-owned execution/module overlay (and any task-scoped native state).
The ordinary per-module-instance rule would then make its initializers run once
per task automatically. That is a separate executor design and performance
decision, not an exception in initializer semantics.

Per-task setup belongs in the task procedure's normal prologue. Durable mutable
state belongs to a service/actor identity. A second automatic “task initializer”
hook is not proposed.

### Failure at each execution site

- A standalone or embedded context that fails initialization never becomes
  ready and cannot enter `main` or a public procedure.
- A local worker that fails initialization never joins the eligible pool. Pool
  construction fails if its required worker set cannot be initialized; already
  created worker contexts are torn down.
- An isolated request whose fresh context fails initialization returns a task
  execution/setup failure without entering the target. The context is destroyed;
  the warm process may serve a later request through another fresh context.
- A late-load failure leaves the pre-existing ready context usable and does not
  publish the new module set.

Signals after earlier modules became `READY`, failure cleanup, initialization
cycles, embedded re-entry, both VM dispatch modes and each task-provider
lifetime need focused negative tests before publication.

The accepted profiling-off Release call-path verdict is retained in
[`2026-08-19 RCC-3 module initializers`](../../../performance/evidence/2026-08-19-rcc3-module-initializers-first-release-verdict/README.md).
Across 12 balanced call/argument pairs, `rxbvm` is -0.353% paired mean and
`rxtvm` is +0.179%; both confidence intervals include zero and remain inside
the 3% guard.

## RXAS instruction-to-call review

The current table has 655 numeric entries, including 68 reserved slots. There
is therefore no immediate numeric-ID emergency. Retiring an instruction from a
published format also does not make its number safely reusable: the old ID must
normally remain a tombstone until an explicit incompatible ISA/RXBIN boundary.
The useful goals are clearer ownership, less VM host code, fewer assembler and
metadata cases, and smaller cold handler/tooling surfaces.

### Recommended first and later candidates

| Family | Current IDs | Recommendation | Conditions |
|---|---:|---|---|
| file operations: `FOPEN` through `FERROR` | 472-485 (14 forms) | **First conversion candidate.** Replace with typed calls into low-level `rx_io`. | Preserve byte/text/UTF behavior, signals, standard streams, close-on-exec, both VMs, native packaging, and late-load. Replace raw `FILE *` integers with context-owned handles and teardown. |
| existing RXAS `rxhash` (FNV-1a string slice) | 513 | **Library-shaped, but measured migration only.** Publish a distinctly named FNV-1a call first; retain the opcode until collection/stem workloads prove no material regression. This is independent of adding SHA-256 to `rx_hash`. | It is currently used as a hot primitive. Compare call cost, whole-string behavior, embedded NUL/binary semantics, and optimizer opportunities. |
| time/version/environment/random | 458-465 (8 forms) | **Second-wave review.** Partition into clock, environment, version, and random contracts rather than one host bundle. | Define determinism, units, monotonic versus wall clock, random state/quality, error behavior, and sandbox/host policy first. |
| socket operations | 62-83 (22 forms) | **Defer, then reconsider as a group.** | A session-aware core provider must prove equivalent context ownership, TLS backends, worker behavior, cleanup, status codes, packaging, and performance. Network latency makes call overhead plausible, but the current instructions have a deliberate deployment/ownership contract. |
| metadata/load operations | 525-537 | **Keep for now.** | Several are frame-, linker-, debugger-, or canonical-RXBIN-sensitive. Revisit only behind a stable context-aware reflection/runtime API; `METALOADMODULE` must integrate transactional initialization first. |
| `SAY*` and `READLINE` | 450-457 | **Keep for now.** | They are language/exit/debug-facing and need a separate I/O/exit semantics design, not a mechanical native call. |
| signals, breakpoints, calls/returns, register/value/reference/object/binary operations, tasks/channels | multiple | **Keep as instructions.** | They directly express VM execution, control-flow, frame, ownership, or hot typed-memory semantics. |

### Required evidence before retiring any family

1. Static census of shipped Rexx/RXAS/RXBIN producers and consumers.
2. Dynamic count and timing profiles on representative applications, not only
   unit fixtures.
3. Handler and toolchain code-size measurements on Apple, Linux, and Windows.
4. Typed replacement API and exact signal/error/ownership equivalence tests.
5. Dual lowering or a migration reader for the declared compatibility window.
6. Optimized/no-opt, `rxvm`/`rxbvm`, linked, late-loaded, embedded, native-packed,
   sanitizer, install, and package proof.
7. An explicit decision to tombstone or reclaim numeric IDs at the selected
   RXBIN/ISA boundary.

## Ordered roadmap

| Gate | Work | Exit condition |
|---|---|---|
| RCC-0: decision lock | Complete the remaining role/provider-name decisions. The conditional 007/008 boundary, opcode disposition list and once-per-mutable-module-instance initializer semantics are accepted. | An approved design records what is core, what is merely default, which compatibility boundary applies, and the exact initialization unit. |
| RCC-1: provider identity and dependency path — **implemented** | Add stable provider identity to compiler import provenance, RXAS metadata, RXBIN serialization, `rxlink` union/conflict logic, diagnostics, and inspection tools. | A linked image reports its exact native requirements without loading or running them. Old/new format behavior is deterministic. |
| RCC-2: runtime and native-package resolution — **implemented** | Implement static-first trusted autoload, binary manifest-ID verification, installed/app-local lookup, failure diagnostics, and automatic static archive selection for native packaging. | The same test image runs under ordinary `rxvm`/`rxbvm` and native packaging without `-p` or a user-maintained provider list; missing/wrong providers fail before execution. |
| RCC-3: explicit module initialization — **implemented** | Implement initializer metadata, per-module-instance state, VM ready-state, deterministic ordering, at-most-one attempt, late-load transaction, embedded/task-provider behavior, signal/failure tests, and documentation. | Every declared initializer reaches `READY` once per mutable module instance after dependencies/link/preparation and before that instance is observable on both VM modes. |
| RCC-4: production `rx_hash` — **implemented** | The small process-reentrant provider publishes `rxhash.sha256(data = .binary) = .binary` directly through RXPA, ships dynamic and static forms, and uses declarative dependency autoload. | Standard vectors, embedded zeroes, boundary lengths, both VMs, native package, scratch install/package, static/dynamic concurrency, and a read-only `crexx-rag` Level G consumer contract pass on macOS. The accepted first-Release verdict found the production path equivalent to the prototype; Linux/Windows product qualification remains RCC-8 work. |
| RCC-5A: level and mathematics contract — **complete** | Record Level B bootstrap versus Level G availability as independent from implementation language, and lock the `rxfloat`, `rxint`, `rxdecimal`, and later packed `rxstats` boundaries. | The public family has explicit availability, implementation, precision, algorithm, compatibility, and performance contracts. |
| RCC-5B: native scalar float provider — **implementation/verdict accepted; focused coverage open** | Extract scalar libm functions into process-reentrant provider `rxfloat`, publish canonical `rxfloat` names and direct `rxmath` compatibility aliases, repair defects, and qualify automatic dynamic/static resolution. | Scalar binary-float math is coherent, tested, automatically loaded/packaged, and meets the accepted first-Release verdict. |
| RCC-5C: integer and decimal standard libraries — **approved, in progress** | Add Level-B-authored `rxint` exact checked algorithms and `rxdecimal` context-preserving algorithms over `mc_decimal`. | Exact integer boundaries and decimal precision/domain/convergence behavior pass focused cross-VM optimized/no-opt coverage. |
| RCC-5D+: remaining historical-bundle splits | Extract and qualify stats after `BINARY-01`, filesystem, platform, ID, hash/checksum, developer, and legacy surfaces. Update the component catalogue and packaging from actual transitive dependencies. | No broad `system` or `rxmath` status hides unrelated APIs; the `crexx` driver links only its narrow required providers. |
| RCC-6: file-instruction replacement | Add `rx_io`, dual-lower/migrate the 14 `F*` forms, prove handle ownership and behavior, measure code/startup/call effects, and select the compatibility retirement point. | The call path is equivalent and acceptable; old opcodes are retained or tombstoned according to the approved format policy. |
| RCC-7: measured instruction review | Evaluate existing RXAS `rxhash`, host utilities, then sockets/reflection only in the recorded order and as separate decisions. | Each family has a keep/convert disposition backed by use, performance, ownership, size, and compatibility evidence. |
| RCC-8: release qualification | Cross-platform build/install/package, both VMs, native/embedded/late-load, security-path, failure, concurrency, and documentation closeout. | The product can explain and mechanically report every required provider and initializer; default installations work without manual runtime lists. |

RCC-5A through RCC-5C are separately authorized; RCC-5A is complete. RCC-5D
and later work is not automatically authorized by that approval. Each later
production architecture or language/format decision remains subject to the
repository's normal approval and, where performance-sensitive,
first-Release-verdict gates.

RCC-5 uses one consolidated full-QA and documentation closeout after its final
approved subphase. Intermediate subphases run focused correctness and any
mandatory first-Release verdict only. Broader evidence gathered for an
intermediate subphase is retained, but does not require the same broad suite to
be repeated for every following subphase.

## Approval questions

The immediate decisions needed before implementation are:

1. Approve `rx_hash` as **standard/default, not core**, while requiring zero-
   configuration discovery for normal installed use.
2. Approve the rule that only required execution/toolchain closure is core, and
   that the current `system` and `rxmath` bundles must be partitioned rather
   than promoted wholesale.
3. Approve the declarative provider-dependency direction and static-first,
   trusted dynamic fallback; leave exact metadata encoding to RCC-0.

The RXBIN boundary rule, instruction-family disposition list and explicit
callable-ID, **once-per-mutable-module-instance** initializer lifecycle and
`name: initialiser` source syntax no longer need approval at this gate.
Optional finalizers remain deferred. The 14 file operations remain the first
conversion candidate, and
existing RXAS `rxhash` remains pending its separate performance verdict;
neither acceptance authorizes production work.
