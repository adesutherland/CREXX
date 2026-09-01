# PERF2-08/09 Mac equivalence and formal-closure worklist

Status: **PERF2-08/09 Mac activity complete; stopped before PERF2-10/11**

Started: 2026-07-27

This is the resumable control plane for the Mac `PERF2-08` capability and
equivalence gate followed by the first `PERF2-09` per-benchmark closure run.
`PERF2-08` is a prerequisite: formal `PERF2-09` measurement cannot claim a
complete Tier A scorecard until every non-common row is qualified or has an
explicitly approved disposition.

## Authority and stop boundaries

- Adrian authorized Mac `PERF2-08/09` on 2026-07-27.
- This is not authorization for a new language surface, Level G work, public
  API/ABI, RXAS/RXBIN change, benchmark-specific product shortcut, or
  production performance edit.
- Stop for Adrian before any language, ownership, collection, public API/ABI,
  serialized-format or architecture decision.
- Stop after the `PERF2-08` disposition panel when an exclusion/defer or design
  decision needs approval. Start formal `PERF2-09` only after that gate.
- Do not start `PERF2-10`, `PERF2-11`, cross-platform tuning, final VM/default
  selection, Linux/Windows work or a final superiority claim in this activity.
- Preserve the accepted `PERF2-06/07` product and its explicit code-layout and
  cross-platform debt. Do not retry rejected frame reset, allocation-ledger,
  cleanup-only interpreter, pooling, slab or broad `value`-layout designs.

## Verified starting state

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch: `develop`
- Starting HEAD: `057592681c0c68e90f436bf02d8c5a116111952a`
  (`test: fix CMake regex escaping`)
- Accepted product commit: `39d3c652e27860222f5d5ed43af71147589b1121`
  (`perf: close Apple value ownership slice`)
- `39d3c652e..057592681` changes only
  `compiler/tests/perf2_04_inline_assembler_imports.cmake`; product source is
  unchanged.
- Upstream: `origin/develop` at
  `53f7757c5b21c15d405b17920d4cd7f6c554c46b`; local `develop` starts three
  commits ahead and zero behind.
- Starting worktree: clean.
- Linked worktrees were recorded with `git worktree list --porcelain`.
  Historical detached performance trees and three prunable records exist; they
  are user evidence state and are not removed by this activity.

## Retained evidence authority

- [x] `2026-07-23-perf2-01-current-baseline` source commit
  `d5b25a78fd6cd2b5b5962b45e508f3cb2bb782e6` exists locally.
- [x] Its recursive `checksums.sha256` verifies all 1,948 entries.
- [x] `2026-07-27-perf2-06-07-v1r01-r1-closeout` is versioned by accepted
  commit `39d3c652e`; its `source-product-workload.sha256` file has SHA-256
  `9390f079438019a9dd25db786c48f81d1362e753ba96faf2d4ed837fbf86ce01`
  identically at `39d3c652e`, starting HEAD and the live worktree.
- [x] Every current source, workload and Level B tool named in that closeout
  manifest matches its recorded SHA-256.
- [x] The closeout/first-verdict directories have no recursive
  `checksums.sha256`; do not describe them as recursively checksum-closed.
  Their retained Git object identity and named SHA-256 manifests are the
  available authority. The new `PERF2-08/09` bundle must be recursively
  checksum-closed.
- [x] The mutable repository `cmake-build-release` product hashes do not match
  the accepted closeout product hashes and are excluded from formal evidence.
  Build a new isolated product for `PERF2-09`.
- [x] Record exact hashes for every additional retained qualification package
  actually used by an equivalence disposition. The new qualification evidence
  root records the exact current sources, external products, supplied
  `json.cls`, generated NetRexx Java/classes and raw pilot outputs.

## Live Apple and comparator freeze

- Host: MacBook Air `Mac17,3`, Apple M5, 10 cores (4 performance, 6
  efficiency), 24 GB RAM.
- OS: macOS 26.5.2 (25F84), Darwin 25.5.0, Apple ARM64.
- Power at freeze: AC connected, low-power mode off; battery 80%, not charging.
- Thermal at freeze: no recorded thermal, performance or CPU-power warning.
- Load at initial freeze: `3.89 12.25 9.36`; this is not accepted formal timing
  state. Wait for a quiet pre-capture state and record pre/post state again.
- Toolchain: Apple clang 21.0.0 (`clang-2100.1.1.101`), CMake 4.3.2, Ninja
  1.13.2, 10 logical CPUs.
- ooRexx: 5.1.0 r12973 at
  `/Users/adrian/.local/opt/oorexx/5.1.0-12973/bin/rexx`, SHA-256
  `42b631e871a70f3da782af8c1045f7ed4167c3ed58834e7e4dcfd86b514c2ee8`.
- Regina: 3.9.7 at `/opt/homebrew/bin/regina`, SHA-256
  `662034cfd9f84c27be597969a97ed7789676e55dd079d23cbbca95ae3a19c555`;
  RexxCPS only.
- NetRexx: 5.10-GA; runtime JAR SHA-256
  `3285e5daa1128474278babdcb6896df6ad5c7ba3a4d9371ed0a19ed3b854af7c`;
  Java is Temurin/OpenJDK 26.0.1+8.
- NetRexx compiler JAR SHA-256:
  `3d57cbd486f90d6db4e4a5ff45defead9442e9d6fb63710a89b2befa7ceb9095`.
- [x] Freeze generated NetRexx Java/classes and the qualification JVM argv.
  Formal arguments and repetitions remain deliberately unfrozen until Adrian
  approves matrix membership.
- [x] Clean detached source identity:
  `/private/tmp/crexx-perf2-0809.uMsNxm/src` at starting HEAD, zero dirty
  paths.
- [x] Independent build identities configured at
  `/private/tmp/crexx-perf2-0809.uMsNxm/build-release`
  (`Release`, `CREXX_VM_PROFILING=OFF`) and
  `/private/tmp/crexx-perf2-0809.uMsNxm/build-profile`
  (`Release`, `CREXX_VM_PROFILING=ON`). Native sampling will use only the
  uninstrumented Release product and a separate capture directory.
- [x] Record qualification CMake options and hashes for compilers, products,
  libraries, sources and workloads. RXAS/RXBIN/linked-image identities are a
  formal-cell freeze obligation after matrix approval; no timing image was
  selected or sampled at this gate.

## Equivalence rule

A row qualifies only when all implementations perform the same mathematical
algorithm, consume the same logical work input, expose an equivalent observable
result and retain the mechanism whose cost the workload is intended to cover.
Language-required spelling and representation changes are allowed only when
they do not remove or add material timed work. Correct output alone is not
equivalence. A disclosed adaptation may remain a useful diagnostic without
entering a common aggregate.

Valid dispositions are:

- `qualified`: eligible for its governed score lane;
- `diagnostic`: correct and useful, but materially adapted and excluded from a
  common comparison;
- `approved exclusion`: no honest equivalent cell is available in this scope;
- `defer`: a named later language/API/architecture owner is required; or
- `decision required`: Adrian must select among materially different semantic
  or product surfaces.

No missing, failing, adapted or non-comparable cell is imputed.

## Stage 0 — exact freeze and reusable controls

- [x] Record live Git, upstream, dirty scope and linked worktrees.
- [x] Verify the retained PERF2-01 recursive checksums and the accepted
  PERF2-06/07 Git/hash authority actually used.
- [x] Record initial host, power, thermal, load, compiler, CMake, Ninja and
  comparator state.
- [x] Create and hash the isolated source/build/product identities. Initial
  profiling-off Release hashes are `rxc 2f74aed9ec33`, `rxas 689de0b66924`,
  `rxlink f3c16ac1df12`, `rxvm 931c75e18530`, `rxbvm 62efe6a725ad` and
  `library.rxbin 61984fd0a7f7`; retain full hashes in the qualification bundle.
- [x] Re-run only the bounded benchmark correctness smoke needed to establish
  the new product: 39/39 pass from the isolated Release build. No formal
  samples were taken.
- [x] Freeze the versioned
  `performance/evidence/2026-07-27-perf2-08-qualification` evidence root and
  recursive checksum manifest.

## Stage 1 — PERF2-08 capability dispositions

| Gap | Current fact to re-prove | Bounded choices | Decision/stop |
| --- | --- | --- | --- |
| CAP-01 JSON parse-once/indexed document | Current cREXX path/count parser and ooRexx/NetRexx DOM paths build different result models. | Qualify a shared parse-once/result/access contract; otherwise retain all current JSON timings as diagnostics and defer a new library/runtime surface. | A new public JSON API, opaque handle ABI or language model is decision required. |
| CAP-02 owned heterogeneous/nested containers | Current cREXX Storage uses `StorageNode` plus `.object[]`; ooRexx/NetRexx use one logical nested array/object allocation. | Prove an existing ownership-safe Level B form or approve exclusion/defer to post-Release 1 Level G. | Any new ownership/container/type rule is decision required. |
| CAP-03 standard Base64 surface | The unchanged arithmetic codec workload is already a qualified common cell; a reusable product API is absent. | Keep benchmark qualification separate; defer a pure Level B library API product track unless selected independently. | Native/SIMD or a new public API is outside this activity. |
| CAP-04 load-only lifecycle boundary | Public CLIs expose combined load-to-first-result, not a common load-only phase. | Retain the honestly named combined phase; approve exclusion of any pure-loader comparison. | A new CLI/runtime API is outside this activity. |

- [x] Re-audit the current selector/module inventory only far enough to confirm
  whether a listed capability can already be expressed efficiently at Level B.
- [x] Prove zero current mechanism or unavailable ownership where that rejects
  a proposed existing-surface closure.
- [x] Record exact semantic, ownership, lifecycle, fallback and platform blast
  radius for every disposition.
- [x] Prepare CAP-01 through CAP-04 as one bounded approval panel; do not
  implement a decision-required surface.

### CAP-01 through CAP-04 decision panel

| Gap | Current proof | Recommended PERF2-08 disposition | Ownership / fallback / blast radius |
| --- | --- | --- | --- |
| CAP-01 JSON parse-once/indexed document | Repository and `rxjson` API audit found only the current string/path operations. Each `jsoncount` request converts/parses the value and path and recursively walks the document; it does not retain a parsed document. ooRexx builds its supplied `json.cls` DOM and NetRexx builds `LinkedHashMap`/`ArrayList` objects. | **Diagnostic + defer.** Exclude JSON from a common timing score. Do not manufacture equivalence from the shared result `8`. | Current inputs remain caller-owned strings and current APIs/failure behavior stay unchanged. A Level B hierarchy or opaque handle would introduce document lifetime, invalidation, error and public API/ABI decisions; defer to an independently selected library/runtime design. |
| CAP-02 owned heterogeneous/nested containers | Level B typed arrays and weak references exist, but nested reference containers are explicitly excluded and arrays are not ordinary `.object` values. Storage therefore allocates one `StorageNode` and one `.object[]` per logical node: 5,461 + 5,461 containers versus 5,461 array nodes in ooRexx/NetRexx. | **Diagnostic + defer to post-Release 1 Level G.** Exclude cREXX Storage from the cross-runtime allocation score. | The arena/wrapper owner remains responsible for lifetime; no fallback can remove 5,461 wrappers using the current surface. Any owned-array/container rule changes type, ownership, compiler and runtime contracts across both VMs and needs Adrian's architecture decision. |
| CAP-03 standard Base64 API | Repository audit still finds no reusable Base64 library service. The existing benchmark ports nevertheless execute the same RFC 4648 codec and validate the same 1,024-byte round trip, encoded length, decoded bytes and checksum. | **Keep the benchmark qualified; defer the library API.** | Current benchmark-owned buffers and error checks remain unchanged. A pure Level B API is a separate library feature; native/SIMD or ABI work is outside PERF2-08/09. Absence of an API is not a reason to exclude the equal-work benchmark. |
| CAP-04 load-only boundary | The public cREXX, ooRexx and NetRexx command paths expose compile/translate and combined load-to-first-result, but no common public load-without-execution phase. | **Keep the honestly named lifecycle phases; approve exclusion of pure load-only comparison.** | Existing process/CLI ownership and failure exits remain unchanged. A new `rxvm` mode or public VM counter changes an exposed interface and is deferred; no phase is imputed or renamed as pure load. |

## Stage 2 — PERF2-08 non-common Tier A qualification

| Workload | Starting status | Required distinguishing work | Provisional safe outcome |
| --- | --- | --- | --- |
| RexxCPS | cREXX 2.2d and NetRexx 2.2n are disclosed adaptations; canonical ooRexx/Regina 2.2 is separately retained. | Re-prove version/provenance, observable contract and native-rate meaning. | Keep as a separately labelled closure target; never add it to the common five. |
| Mandelbrot | ooRexx decimal arithmetic fails canonical 500/750 checksums. | Audit whether any ordinary ooRexx numeric mode can reproduce the same mathematical/rounding contract without imported native work; retain negative checks. | Exclude as not comparable if the numeric contract cannot be matched honestly. |
| Towers | ooRexx procedural numeric-node/stem port removes intended object/allocation work. | Compare the current port with the upstream/cREXX/NetRexx allocation graph; test a bounded ordinary ooRexx object-equivalent form if expressible. | Qualify only an allocation/object-equivalent port; otherwise retain the procedural cell as diagnostic. |
| Storage | cREXX wrapper doubles the logical ownership/allocation shape. | Prove exact logical allocations/objects and current Level B ownership limitation under CAP-02. | Exclude/defer unless an existing ownership-safe equal-work form is proved. |
| List | cREXX weak-reference arena owns nodes while other ports use object links. | Prove whether arena allocation/lifetime is material intended work or an adaptation, including exact nodes, links and lifecycle. | Qualify as disclosed equal-work only if ownership work is materially equivalent; otherwise diagnostic exclusion. |
| JSON | All three cells parse the same payload but expose different result/access models. | Prove parse count, result construction, accesses and ownership under CAP-01. | Diagnostic exclusion unless a shared contract is approved and implemented separately. |

- [x] Freeze all current source hashes and provenance.
- [x] Run opt/no-opt and runtime perturbation correctness only where retained
  evidence is not current or a distinguishing candidate must be tested.
- [x] Record exact algorithm, logical work, allocation/object/reference model,
  observable output and timed boundary for every implementation.
- [x] Retain candidates that fail equivalence; do not time them formally.
- [x] Prepare the complete qualification/exclusion panel and stop for Adrian's
  decisions before `PERF2-09`.

### Non-common Tier A decision panel

| Workload | Current distinguishing evidence | Recommended PERF2-08 disposition |
| --- | --- | --- |
| RexxCPS | cREXX 2.2d retains a zero-net-source-clause disclosed typed adaptation. ooRexx and Regina ran the checksum-identical canonical 2.2 source; NetRexx ran the disclosed 2.2n timed-kernel adaptation and emitted its explicit PASS marker. The reported rates are native implementation rates, not an equal-source common-five cell. | **Qualified separate closure target.** Report cREXX `rxvm`/`rxbvm` and canonical ooRexx separately; retain Regina and NetRexx as labelled controls. Never add RexxCPS to the common-five aggregate. |
| Mandelbrot | cREXX and binary NetRexx produce 128/191/50 for sizes 1/500/750. The current ooRexx source produces 128/255/128. An ordinary `NUMERIC DIGITS` sweep at 9, 15, 16, 17, 20, 34 and 50 never produces the binary64 size-500/750 contract. `NUMERIC FORM` changes notation, not the decimal arithmetic model. | **Approved exclusion / not comparable.** Keep all source and negative checks visible; publish no cross-runtime ratio. Any imported binary arithmetic would measure a different external mechanism. |
| Towers | The prior ooRexx numeric-node/stem form removed the intended object allocation and dispatch. The bounded replacement now creates one benchmark object and 14 disk objects per repetition, uses object methods for size/next/link mutation, runs the same recursive moves and observes 8,191. It passes repetitions 1 and 2. | **Advance the object-equivalent ooRexx port as qualified.** Retain the removed procedural source only through historical evidence; formal timing must use source SHA-256 `bb081b76306ce1d360f4e739e480e3e89ebceb31028326bc93910c8daa0267b9`. |
| Storage | All ports build the same depth-7 four-way logical tree and observe 5,461 nodes. cREXX necessarily adds 5,461 owner objects around 5,461 child arrays, while ooRexx/NetRexx allocate one array per logical node. | **Diagnostic exclusion + CAP-02 defer.** Correctness is retained; no common allocation ratio. |
| List | All ports create exactly 31 nodes (15 + 10 + 6) and observe result length 10. cREXX additionally creates a `ListArena` and typed owning array because links are weak references; ooRexx/NetRexx object links own their referents. The extra allocation and ownership traffic is material to this object/reference workload. | **Diagnostic exclusion.** Keep visible cREXX and external native timings if useful, but publish no cross-runtime ratio and do not mislabel the arena as equal work. |
| JSON | All ports parse the same payload and observe eight operations, but cREXX reparses a string/path query while ooRexx/NetRexx construct different DOMs. Result construction, allocation, access and retained representation are not common. | **Diagnostic exclusion + CAP-01 defer.** No common ratio until an independently approved shared parse/result/access contract exists. |

### Qualification-only execution result

- Isolated cREXX Release product: 39/39 benchmark-label smoke tests passed.
- Selected non-common cREXX matrix: 48/48 PASS across `rxvm`/`rxbvm`,
  optimized/no-opt, including Unicode-independent deterministic results.
- Current NetRexx generated classes: 11/11 selected Mandelbrot, Towers,
  Storage, List and JSON correctness cells passed.
- ooRexx: 8/8 selected Towers, Storage, List and JSON cells passed; Mandelbrot
  size 1 passed and sizes 500/750 failed with the retained expected decimal
  results 255/128.
- RexxCPS qualification pilots exited successfully on ooRexx, Regina and the
  disclosed NetRexx 2.2n adaptation. Their observed rates are smoke output and
  are not formal samples.
- At this gate no formal time, aggregate, lifecycle, RSS or artifact-size
  verdict had been taken. Adrian subsequently approved the exact panel and the
  completed Stage 3 evidence is recorded below.

## Stage 3 — PERF2-09 formal Mac capture (after PERF2-08 approval only)

- [x] Adrian approved the complete PERF2-08 panel exactly as proposed on
  2026-07-27: common `N=5`; separate qualified Towers, RexxCPS and lifecycle
  lanes; no formal timing rows or ratios for Mandelbrot, Storage, List or JSON;
  CAP-01 through CAP-04 public/language surfaces deferred as recorded.
- [x] Freeze the approved matrix membership in
  `performance/manifests/perf2-09-mac-closure-v1.txt`; never mutate an
  earlier formal manifest in place.
- [x] Run the common five (Sieve, Permute, Bounce, Richards, Base64) across
  `rxvm`, `rxbvm`, ooRexx and decimal-semantics NetRexx with equal work,
  two warmups and ten recorded serial, rotated samples per cell.
- [x] Run cREXX RexxCPS 2.2d under both VMs and canonical Classic ooRexx 2.2
  as a separately disclosed native-rate comparison; keep Regina/NetRexx
  labelled controls where retained.
- [x] Run every other approved Tier A row in its exact qualified or diagnostic
  lane. Exclusions stay visible and receive no fabricated ratio.
- [x] Apply the governed one-append noise rule without removing samples.
- [x] Capture lifecycle separately from steady-state timing.
- [x] Capture peak RSS separately with zero timing warmups and its own sample
  set.
- [x] Capture source, RXAS, RXBIN/linked image, executable, library and
  generated NetRexx artifact sizes/hashes separately.
- [x] Record pre/post AC, low-power, thermal and host-load state. Stop formal
  timing on a thermal/power warning or unresolved competing load.
- [x] Report `rxvm` and `rxbvm` separately; publish the four common aggregate
  ratios with exact membership and `N=5`.

Formal argument freeze from the current-product envelope check:

| Workload | Equal work | Fastest disposable observation | Slowest disposable observation | Disposition |
| --- | ---: | ---: | ---: | --- |
| Sieve | 5,500 | `rxvm` 1.108 s | ooRexx 7.725 s | unchanged seed accepted |
| Permute | 5,000 | NetRexx 1.070 s | ooRexx 16.393 s | unchanged seed accepted |
| Bounce | 4,200 | `rxvm` 1.147 s | ooRexx 4.256 s | increased from historical 2,200 because the accepted product made the old fastest cell 0.594 s |
| Richards | 20 | NetRexx 1.028 s | `rxvm` 6.814 s | unchanged seed accepted |
| Base64 | 2,500 | ooRexx 1.196 s | `rxbvm` 1.891 s | unchanged seed accepted |
| Towers | 100 | NetRexx control 0.036 s | `rxbvm` 3.666 s | same work retained; cREXX and the primary ooRexx comparator both exceed 1 s. The binary/JVM NetRexx control is startup-dominated and receives no common ratio. |

The disposable envelope observations are calibration only and are excluded
from formal evidence. The common five satisfy the governed one-argument
1-to-30-second rule on the current product.

Formal capture completed with 348/348 initial timing samples and 20/20
governed Base64 append samples; 87/87 initial and 40/40 governed append RSS
samples; and 80/80 initial plus 80/80 governed append lifecycle phase rows.
No correctness-passing sample was removed. cREXX Base64 timing, four NetRexx
RSS cells and seven of eight lifecycle series remain explicitly noisy after
the single permitted append. The four exact common-five geometric means are
`2.125260`, `1.842840`, `0.742985` and `0.644251` for `rxvm/ooRexx`,
`rxbvm/ooRexx`, `rxvm/NetRexx` and `rxbvm/NetRexx`, respectively.

## Stage 4 — per-benchmark closure dossiers

For each Tier A workload retain:

- exact source/image/runtime hashes and comparability status;
- same-session cREXX/ooRexx throughput or explicitly absent ratio;
- gain to parity, 1.50x strong band and remaining deficit;
- optimized and diagnostic static/dynamic work;
- top native/procedure/opcode/call paths;
- copies, conversions, allocations, lifecycle, RSS and artifact size in
  separate dimensions;
- selected general mechanism and mathematical machine ceiling; and
- accepted, rejected, deferred or decision-required next step.

- [x] Rank the largest qualified remaining deficit from current measurements,
  not historical counts.
- [x] Guard Sieve/Permute and every accepted PERF2-02 through PERF2-07 gain.
- [x] Do not install the next performance candidate from this ranking.

The current ooRexx-closure ranking is Richards, Towers, Base64, then RexxCPS;
Sieve, Permute and Bounce are already ahead. In the secondary NetRexx common
comparison the deficit order is Richards, Permute, Base64, followed by the
`rxbvm` strong-band gaps in Bounce and Sieve. Richards is the largest
qualified common deficit under either comparator at `0.267262/0.264171`
versus ooRexx and `0.157815/0.155990` versus NetRexx. The accepted
direct-placement work is a guard, not a new candidate authorization.

## Stage 5 — PERF2-09 Mac closeout and stop

- [x] Produce one compact, recursively checksum-closed evidence bundle with
  consolidated raw tables and referenced prior forensics.
- [x] Reconcile `ROADMAP.md`, this worklist, the portfolio comparability plan,
  capability ledger and scorecard without rewriting historical reports.
- [x] Record the exact unresolved deficit/debt and candidate handover ledger.
- [x] Stop before `PERF2-10/11`, any production performance edit or any
  cross-platform/final-superiority claim.

Closeout evidence:
[`2026-07-27-perf2-09-mac-closure`](evidence/2026-07-27-perf2-09-mac-closure/).
The next candidate remains deliberately unselected. The accepted code-layout
debt, `PERF2-06-D01`, architecture-owned C2R03/V6R01 decisions and required
Linux/Windows lanes remain open under their existing owners.
