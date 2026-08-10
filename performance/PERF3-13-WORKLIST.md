# PERF3-13 RXVM allocator, value-shape and worker-communication worklist

Date opened: 2026-08-05

Status: **Gate E E3a accepted and Mac closeout complete; E3b-P1 A/C compatibility slice approved; Gate F closed**

## Current Gate E continuation

- Worktree: `/Users/adrian/CLionProjects/CREXX`.
- Branch: `develop`.
- Published Gate E base: `84d406904ece6842f6cec5a47e75d12b9d28ab16`
  (`fix: use compiler-correct RXVM thread locals`).
- EF-0 implementation: `642e1b697bd019a800a2bddbaea8ef7a3d75e531`
  (`perf: recover spawn I/O ownership`).
- Adrian approved the full Gate E architecture and its immediate E1
  single-worker ownership-shell slice on 2026-08-07. Each production slice
  remains subject to its focused correctness, frozen ordinary Release verdict
  and explicit acceptance stop.
- Adrian accepted E1's first ordinary Release verdict on 2026-08-07. The E1
  implementation and proportional Mac closeout are published in `b5e1d0565`.
- Adrian accepted E1-P1's core-four Release verdict and authorized full QA on
  2026-08-07. The wrapper removes the isolated RexxCPS layout regression and
  passes proportional Mac sanitizer, full Debug and ordinary Release closeout.
  It is published separately in `91282a0ac`. Windows-MinGW follow-up commits
  `7bbf32cae` and `84d406904` fix the platform include boundary and select TLS
  by compiler rather than operating system. GitHub Actions run `31206601838`
  passes all 1,999 Windows tests, including `rxvmworker_lifecycle`, and the
  complete Linux/macOS/Windows build matrix; CodeQL run `31206601827` passes.
  MSVC, Intel Linux and Linux ARM64 proof remains open.
- Adrian accepted E2's direct-slot ordinary-Release verdict and authorized full
  QA. The Mac closeout is green, including the complete 1,999-test Debug and
  AddressSanitizer suites. E3 plugin/native-instance ownership is the next
  proposed Gate E slice and requires its own approval. Gate F remains closed.
- The post-handler-refactor current control is clean synchronized `develop` at
  `6d12cd921`. Ordinary Release uses the `profile-20` panel: 118 ranked public
  handlers plus both private fused handlers are inline (120/589), with the
  rest callable. A fresh seven-workload, both-engine formal absolute baseline
  passes 168/168 initial and 40/40 governed-append executions. Base64 remains
  noise-labelled after the one permitted append. This is an absolute entry
  observation, not a substitute for E3's later same-session paired verdict.
  Evidence:
  [`2026-08-10-perf3-13-e3-current-baseline`](evidence/2026-08-10-perf3-13-e3-current-baseline/).
- Adrian approved E3a on 2026-08-10 through its first frozen ordinary-Release
  verdict. E3b is the intended following slice, but remains separate and must
  not be mixed into E3a's implementation or verdict.
- Adrian accepted E3a on 2026-08-10 and directed the programme to move to E3b.
  The accepted verdict's two-context ownership test passes, all 208/208 verdict
  processes pass, all eight paired product/guard comparisons are statistically
  inconclusive, and no 3% adverse guard fires. Product `rxbvm` paired medians
  range from -0.069720% to +0.962402%; the candidate adds 800 bytes to each VM
  file while the profile-20 hot owner is unchanged in `rxtvm` and 72 bytes
  smaller in `rxbvm`.
- E3a's shortest Mac closeout is complete. Removing the disposable reproducer
  mode leaves focused Debug 15/15 and Release ownership 1/1 green. The first
  broad build exposed one include-boundary error in auxiliary targets; changing
  the new include to its interpreter-root-relative path repairs it. The full
  Debug build then passes and CTest is 2,007/2,007 in 291.77 seconds. Rebuilt
  Release `rxtvm` and `rxbvm` retain the exact accepted-verdict hashes, so the
  verdict remains authoritative and no timing rerun is warranted. Adrian then
  approved the E3b A/C compatibility model and its P1 implementation through
  the first frozen ordinary-Release verdict. Evidence:
  [`2026-08-10-perf3-13-gate-e-e3a-first-release-verdict`](evidence/2026-08-10-perf3-13-gate-e-e3a-first-release-verdict/).

## Exact isolated base

- Worktree: `/Users/adrian/CLionProjects/CREXX-rxvm-worker-memory-gatea`.
- Branch: `codex/rxvm-worker-memory-gatea`.
- Base commit: `4813e98d1dca1ac77d5899dd6c5787e4b83f4772`.
- Base subject: `perf: close PERF3-12B with Mac scorecard`.
- Closeout branch: `codex/rxvm-default-and-base64-review` in the isolated
  scratch tree `/tmp/crexx-rxvm-inline.yvLywZ/source`.
- Gate D production branch: `codex/rxvm-gate-d-industrialise` in the clean
  isolated worktree `/private/tmp/crexx-rxvm-gated.6HZvFK/source`, created from
  `0d1fe884782ff369960b1c67c38127407ce54588` without importing research
  selectors or rejected layouts.
- `develop`, `origin/develop` and the new worktree base resolved to that commit
  when Gate A started.
- The older `/Users/adrian/CLionProjects/CREXX-rxvm-worker-memory` worktree at
  `965b461d8` is retained untouched as historical M0/replay input. Its
  instrumentation and conclusions are not imported without current review.

## Host coordination

Multiple agents may use this Mac for performance measurements. A pause request
from Adrian is immediately binding. Before any timed performance cell, this
activity must ask Adrian to clear and reserve the host explicitly. Gate A uses
source inspection and retained evidence until that reservation is granted;
correctness-only work is not presented as performance evidence.

## Decision and sequence

Adrian selected the following order:

1. establish the slab allocator with the current `value` layout and semantics;
2. route allocator-eligible RXVM allocations through that substrate and obtain
   an ordinary single-worker Release verdict;
3. measure and compare compact `value` shapes on the accepted allocator;
4. select the performance/memory sweet spot rather than the smallest layout;
5. industrialise the selected allocator and value representation; and
6. only then add worker/thread/process transfer and programmable-channel
   semantics.

This order prevents allocator effects, value-layout effects and concurrency
effects from being combined in one result. Neutral single-worker performance
is acceptable for the allocator substrate if it materially improves ownership
control and does not breach RSS, lifecycle or correctness guards. A layout
candidate must earn a separate verdict.

## Scope and boundaries

- Work is RXVM-owned. Do not change cREXX language syntax, compiler lowering,
  RXAS/RXBIN semantics, HTTP APIs or channel instructions during allocator and
  value-shape stages.
- Keep `rxvm` and `rxbvm` separate in every product verdict.
- Preserve reference identity, native-payload finalisation, stable object
  attribute addresses, signal/unwind behaviour, TRACE and plugin correctness.
- The public native/plugin exposure of `value` makes any layout change an ABI
  decision. Allocator experiments retain the current layout; compact layouts
  remain isolated PoCs until Adrian selects one and approves ABI treatment.
- Do not infer thread safety from allocator ownership. Thread/process/channel
  semantics remain closed until the selected single-worker representation is
  industrialised.

## Meaning of “remove all mallocs”

The programme removes direct allocator calls from allocator-eligible RXVM and
bundled VM-plugin ownership paths, not from the operating system itself.

- The central depot is the sole ordinary boundary that obtains and returns
  large blocks from/to the system allocator.
- Worker arenas satisfy ordinary allocations without central synchronization.
- Variable byte storage uses power-of-two capacity classes with explicit
  oversized and foreign escape paths.
- Fixed-size objects such as the unchanged 240-byte `value` and reference cells
  use typed silos carved from depot blocks rather than being blindly rounded to
  a generic power-of-two object size.
- APIs whose memory must be released by an operating-system, library or foreign
  allocator stay on that allocator and appear in an explicit exception ledger.
- No raw `malloc`, `calloc`, `realloc` or `free` remains in an eligible hot or
  lifecycle path merely for convenience. Growth becomes allocate/copy/return
  through the owning arena.

The inventory must classify frame blocks, standalone and scratch values,
globals, object attribute pointer/value storage, reference cells,
string/decimal/binary buffers, native payloads, bundled plugins, module/load
state, socket/TLS state and temporary metadata before claiming completion.

## Evidence panel

Every measured gate uses the ordinary profiling-off Release product and keeps
raw correctness, elapsed, RSS, retained/high-water, allocation/class,
fragmentation and teardown evidence distinct. The minimum panel contains:

- RexxCPS for string/decimal and first-class community coverage;
- Richards and Towers for object/value/attribute allocation;
- Base64 for string/binary buffer behaviour;
- Sieve or Permute as a scalar/control guard;
- lifecycle/API/native-payload and plugin-focused checks; and
- both `rxvm` and `rxbvm`.

Formal decisions follow `PERFORMANCE-GOVERNANCE.md`. Counts-only
instrumentation may use bounded labelled forms; wall-clock conclusions use
unprofiled Release products after exclusive-host reservation.

## M0 — baseline, allocation and ownership contract

- [x] Create a fresh dedicated worktree from the exact accepted baseline and
  preserve the older M0 worktree untouched.
- [x] Verify the retained baseline/evidence usable without new timing. The old
  and current committed production `interpreter/` sources are identical; only
  `interpreter/tests/test_rxvmstem.c` differs.
- [x] Inventory every RXVM and bundled VM-plugin allocation/free/reallocation
  site and classify it as depot-owned, worker-owned, transferable,
  foreign/OS-owned or an invalid ownership ambiguity.
- [x] Record current allocation counts, requested bytes, size distribution,
  high-water/retained bytes, lifetime, originating value/storage kind and
  teardown behaviour from retained evidence, explicitly marking unavailable
  whole-allocator live/RSS/fragmentation data for Gate B telemetry.
- [x] Define the ownership header/lookup, worker identity, failure contract,
  alignment, oversized allocation, deterministic teardown and telemetry
  interfaces without changing `value`.
- [x] Compare typed fixed-size silos plus power-of-two byte classes with a
  universal power-of-two control.
- [x] Stop for M1 implementation approval with a selected substrate design and
  exact exception ledger.

Gate A result and evidence limits:
[`PERF3-13-M0-ALLOCATOR-AUDIT.md`](PERF3-13-M0-ALLOCATOR-AUDIT.md).

## M1 — basic single-worker slabs, unchanged value

Gate B was approved by Adrian on 2026-08-05. Adrian reserved the host, accepted
the Release result and authorized the baseline commit; M1 closed on 2026-08-06.

- [x] Implement the central block depot and exactly one logical worker arena;
  create no worker threads and change no value field or semantic contract.
- [x] Provide power-of-two byte-buffer classes and typed silos for fixed-size
  VM objects, including allocation failure and alignment proof.
- [x] Convert eligible RXVM allocations in bounded ownership slices.
- [x] Convert bundled decimal/native/plugin paths through an allocator boundary
  or record a justified foreign-allocation exception.
- [x] Preserve sticky reusable capacity with an explicit oversized release/trim
  policy so one exceptional input cannot remain pinned forever.
- [x] Prove deterministic teardown and zero invalid cross-family frees.
- [x] Freeze implementation after the minimum focused correctness checks.
- [x] Run the mandatory first ordinary Release
  comparison against the unchanged-layout libc-allocation control.
- [x] Stop for Adrian. Neutral is acceptable; a correctness, material RSS,
  fragmentation, teardown or representative performance guard breach is not.

### M1 accepted implementation record

- `value` remains 240 bytes with its existing inline string and public shape.
- One 64 KiB-aligned slab class serves each power-of-two byte capacity from 16
  through 16,384 bytes. Exact typed classes serve 1/2/4/8/16/32/64 current
  values and reference cells; larger requests use separately tracked extents.
- Each RXVM context owns one logical worker arena. Ordinary local allocate/free
  paths do not take the depot lock; the central depot exchanges only whole
  empty slabs and retains at most two per class and 32 globally.
- Frames, globals, modules, runtime/graph/interface metadata, references,
  RXVML-owned vectors, sockets, spawn/redirect state, ADDRESS internals, RXPA
  scratch and plugin-framework-owned registries now use the worker/depot
  substrate. Worker entry is scoped across VM runs and existing spawn I/O
  threads before they mutate owning values.
- Logical register reset keeps owned string, decimal, binary and attribute
  capacity sticky. Attribute shrink/delete performs no hot-path reclamation.
  `reclaim_attribute_storage()` exists only as an explicit quiescent operation
  with test coverage and has no production caller. Gate C owns measurement and
  selection of reclamation/pressure policy; Gate D owns any production
  implementation. The depot retains an explicit trim operation.
- `CREXX_RXVM_MEMORY_STATS=1` emits bounded versioned allocator/class telemetry
  to stderr at teardown. Default execution has no reporting. The line includes
  failures, invalid/wrong-owner frees, cumulative requested/capacity bytes,
  live and peak storage, oversized state, retained slabs and depot traffic.
- The dynamic decimal-plugin proof rejected direct RXVM allocator calls: that
  route requires exported/shared allocator state or a new versioned
  host-services ABI. Decimal payloads, bundled plugin-private objects and
  plugin temporaries therefore remain a named native-plugin/libc family for
  this unchanged-ABI gate. Gate D owns any allocator-service ABI transition.
- Other remaining direct allocator calls are provider or foreign contracts:
  slab/extent and worker/context providers; outer CLI/RXVML contexts and public
  caller-free arrays; `exepath()`/`reg2nullstring()`/environment/spawn callback
  buffers; RXBIN/signature/graph/RXPA library results; OS/TLS handles and host
  conversion buffers; and plugin finalizers. No pointer family is guessed.
- The final focused Debug set passed 19/19: allocator ownership/alignment/
  resize/trim/teardown, value and oversized-reuse policy, RXPA UTF, RXVML
  reentrancy, four ADDRESS/redirect paths, two `rxbvm` core/binary paths, both
  VM socket paths, and seven static/dynamic/manual decimal-plugin paths. A
  stats-enabled direct RXVM run also matched its golden stdout and ended with
  `allocations=3270`, `frees=3270`, `live_allocations=0`, `failures=0`,
  `invalid_frees=0` and `wrong_owner_frees=0`. These are correctness/counts
  observations, not timing evidence.

### M1 accepted Release verdict and closeout

- The original four-cell allocator timing capture was invalidated for formal
  pairwise use after the runner review proved that simple rotation did not
  balance every pair's relative order. The accepted closeout reran all seven
  workloads with the corrected Level B pairwise scheduler: one warmup and 12
  serial recorded rounds, ordinary profiling-off Apple Clang Release, no
  removed samples and 364/364 passing processes.
- Apple Clang's selected product lane is portable switch dispatch
  (`rxvm -> rxbvm`). Against the unchanged-layout libc control, its stable-six
  geometric performance ratio is `1.200605` (+20.060456%). Sieve, Permute,
  Bounce, Richards, Towers and RexxCPS are each clear favorable with 12/12
  favorable pairs. The direct-threaded diagnostic lane is also favorable at
  `1.136288` (+13.628788%).
- The common-five ratio is `1.269177` (+26.9177%) for `rxbvm`, but Base64
  remains materially noisy and does not select the allocator or dispatch
  policy. Carry Base64 into CAP-03 as a pure Level B library/API task: define
  the standard Level B API, reference implementation and correctness/algorithm
  tests separately from the governed codec-loop benchmark. This does not imply
  native, VM or opcode work without separate approval.
- A separate four-round pairwise RSS materiality check has no selected-product
  concern: the largest `rxbvm` median increase is 348,160 bytes (+1.914%) on
  Permute; Sieve is +163,840 bytes (+0.899%), Bounce is unchanged, and the
  other four workloads decrease by 520,192 to 1,757,184 bytes.
- Proportional closeout passed the full Clang Debug build, 7/7 focused
  allocator/value/product checks, 129/129 representative non-spawn smoke
  tests, 2/2 installed SDK consumers, a real isolated install/product check,
  and a GCC 16 Release build/run selecting `rxvm -> rxtvm`.
- Spawn tests remain deliberately excluded from this transitional baseline by
  Adrian's decision. Existing spawn must migrate to the final worker ownership
  architecture before its failures can become closure blockers.
- Retained evidence:
  [`2026-08-06-perf3-13-gate-b-closeout`](evidence/2026-08-06-perf3-13-gate-b-closeout/).

### Reclamation ownership across the remaining gates

- The accepted Gate B baseline is sticky reuse: ordinary logical reset and
  attribute shrink/delete do not reclaim on the hot path. Explicit quiescent
  attribute reclaim and depot trim exist, but have no automatic production
  caller.
- Gate C M2 measured retained dead capacity, reuse distance, pressure and
  reclaimable bytes. Adrian selected R0 for the representation baseline rather
  than spending hot-path or lifecycle complexity on an automatic policy before
  the value shape was stable. R1/R2 were not silently treated as equivalent
  evidence; they are deferred candidates.
- Gate D M4 industrialises R0 only: no automatic runtime reclamation, no
  pressure check in logical reset, one empty slab retained locally per class,
  bounded depot reserve and explicit quiescent depot trim.
- Any R1 explicit value-sidecar reclaim or R2 bounded pressure policy requires
  a separately approved Gate D-R after the stable representation. It must
  define trigger ownership, safe points, budgets/hysteresis and telemetry.
- Gate E M5 adapts the policy then in force to worker ownership. A future
  global pressure signal may request owner-local work, but must not turn
  ordinary allocation or reclamation into a central allocator-thread
  bottleneck.
- Gate F M6 does not own allocator reclamation. Channel backpressure and worker
  quiescence may expose useful signals or safe points, but the channel contract
  must not hide register mutation or an unbounded reclaim pause.

## M2 — value-shape census on the accepted allocator

Gate C was approved by Adrian on 2026-08-06. The host is reserved exclusively
for this activity. M2 opens on the committed Gate B control
`f36d2c1549f9f37f3950e42bfb89f8d32f12f3ea`; no allocator geometry or
`value` layout candidate may start until the C1 census checkpoint freezes the
panel.

- [x] Count physical values by origin: frame local, global, attribute,
  standalone, scratch and native/API value.
- [x] Record simultaneous representations and ever-acquired sticky string,
  decimal, binary, object, reference and native storage.
- [x] Record logical/capacity-class distributions, moves/swaps, grows,
  oversized allocations, retained high-water and trim events.
- [x] Quantify retained dead capacity, reuse distance and reclaimable bytes
  under the no-reclaim control and candidate quiescent/pressure conditions.
- [x] Separate operation frequency from live/physical occupancy.
- [x] Keep census instrumentation diagnostic-only: it may not change
  `sizeof(value)` or the ordinary profiling-off Release product.
- [x] Freeze the exact slab/oversize and compact-layout candidate panel and
  stop at checkpoint C1 before implementing a candidate.

### Gate C allocator-geometry panel

Geometry is isolated first on the unchanged 240-byte V0 control with automatic
reclamation disabled. The initial bounded model is:

- S0: current 64 KiB slab and 16 KiB maximum byte class. After the 64-byte
  header, that maximum class has three actual slots.
- S1: 64 KiB slab requiring at least four actual slots per standard class,
  making 8 KiB the maximum byte class.
- S2: 256 KiB slab with the four-slot rule, making 32 KiB the maximum.
- S3: 1 MiB slab with the four-slot rule, making 128 KiB the maximum.
- S3a: a three-slot/256 KiB maximum only if the census finds a hot reusable
  buffer in that band. One MiB is the no-header-change ceiling for the current
  16-bit slot count and 16-byte minimum class.

The same actual-slot rule applies to typed value blocks. A class is added only
when observed demand/reuse justifies it. Depot reserve is compared in bytes,
not an unchanged slab count, and owned, committed, live and empty-reserve bytes
remain separate. After V1 removes the inline string, the census also selects
between 16-byte and 32-byte minimum string sidecars. A provisional V0 geometry
is rechecked against the selected compact value before Gate C closes.

### C1 census outcome — complete 2026-08-06

The retained counts-only package is
[`2026-08-06-perf3-13-gate-c-c1-census`](evidence/2026-08-06-perf3-13-gate-c-c1-census/).
All six product-lane cells and three concrete-threaded identity cells passed;
focused allocator/value/RXPA/RXVML/profiler checks pass 9/9. A separate
profiling-off Release proof retains the 240-byte value, passes allocator and
Sieve smoke checks and contains no census strings.

The observations freeze these decisions before any PoC:

- Sieve's reusable 8,192-value block is a 1,966,080-byte typed extent. It does
  not fit any bounded S0-S3 slab and remains on the exact tracked oversized
  path. Towers instead makes 12,071,103 hot eight-value-block allocations in
  the exact typed silo and peaks at 455 live blocks. Do not replace typed
  classes with generic power-of-two objects.
- Advance S0, S1 and S2 as defined above and add **S1b**, a 128 KiB slab with
  16 KiB maximum byte class. S1b is the smallest bridge that retains the
  current threshold while providing seven actual maximum-class slots. Reject
  S3/S3a at C1: no hot reusable 128-256 KiB buffer was observed, the 1 MiB
  slab still cannot contain Sieve's exceptional typed extent, and its
  small-class committed-memory projection is disproportionate. Reopen only if
  S2 first proves a span-size benefit that survives memory guards.
- Keep typed classes independent of the generic byte cutoff. Do not add a
  `value128+` class from this evidence. Compare depot reserve in bytes under
  every geometry.
- Exact compiler models confirm V0/V1/V2a/V2b/V3 at
  **240/208/192/160/120 bytes**. V1 screens both 16-byte and 32-byte first
  string-sidecar classes. V2a uses byte power-of-two capacity codes with an
  oversized/foreign escape. V2b keeps binary actual length at `size_t` and
  checks managed 32-bit lengths/counts. V3 retains direct hot string, binary,
  references, type, active attributes and count, with separate lazy
  numeric/native and object-growth sidecars.
- R0 remains no runtime reclaim. R1 is explicit owner-quiescent trim. The
  first R2 pressure screen is safe-point/slow-path only, starts at 128 KiB,
  scans at most 64 values and returns at most 1 MiB per pass while allowing one
  larger object for progress. It uses high/low hysteresis and never checks
  pressure in logical reset. The measured hot <=2 KiB Base64/RexxCPS sidecars
  remain sticky under R2.

C1 was a hard stop. Adrian approved C2 on 2026-08-06 with exclusive use of the
host. C2 starts with allocator geometry only: V0 remains 240 bytes, typed
silos remain unchanged and R0 keeps automatic reclamation disabled.

### C2 first Release design-selection contract

The first C2 slice is a replayable build-time geometry screen, not a production
selection:

1. S0 is the status-quo/default 64/16 KiB control. S1 is 64/8 KiB, S1b is
   128/16 KiB and S2 is 256/32 KiB. A closed CMake selector rejects any other
   geometry so this PoC cannot silently widen the C1 panel.
2. The byte-class cutoff changes only generic byte storage. Exact typed
   1/2/4/8/16/32/64-value and reference-cell silos stay available when their
   slot size fits the selected slab; exceptional larger typed requests remain
   tracked extents.
3. Compare depot reserve in bytes. Preserve the S0 ceiling of 2 MiB globally
   and 128 KiB per class, rounded down by whole slabs but retaining at least
   one slab for an active class. The resulting count ceilings are S0/S1 32
   global and 2 per class, S1b 16/1 and S2 8/1.
4. The default S0 build must retain the existing allocator behaviour and the
   ordinary product must remain profiling-off. Focused allocator correctness
   runs for every geometry precede timing.
5. The mandatory first Release verdict is deliberately short: one warmup and
   four pairwise/position-balanced recorded rounds across all four product-lane
   geometries on the seven accepted Gate B workloads. Keep Base64 recorded but
   non-selecting. Capture a separate bounded RSS check and allocator telemetry;
   do not mix either with elapsed-time claims.
6. This screen may reject a broken or clearly regressive geometry, but cannot
   select the final geometry. Stop for Adrian after the verdict. No V1 layout,
   reclamation candidate, rxtvm finalist screen, formal 12-round comparison or
   industrialisation starts without his direction.

### C2 corrected geometry first Release verdict — complete 2026-08-06

The initial four-way capture was invalidated before retention: disassembly
showed that its selector inserted a new hot cutoff branch into S0, so S0 was
not a machine-level replay of the accepted allocator control. Those numbers
have no decision authority. The corrected selector varies the compile-time
class table. Exact `otool -tvV` comparison proves all 22 emitted
`rxvm_memory_*` text symbols in corrected S0 match the accepted control.

Retained evidence:
[`2026-08-06-perf3-13-gate-c-c2-geometry-first-verdict`](evidence/2026-08-06-perf3-13-gate-c-c2-geometry-first-verdict/).
All four ordinary profiling-off Apple Clang builds resolve `rxvm -> rxbvm` and
pass their allocator/value focused pair, 8/8 total. The valid short screen
completed 140/140 timing processes and 112/112 RSS processes; all 28 separate
telemetry cells pass with zero allocation failures, invalid frees or
wrong-owner frees.

The first verdict does not select a geometry:

- Stable-six timing medians versus S0 are S1 -0.353%, S1b +0.445% and S2
  +0.562%. These are neutral short-screen observations, not formal estimates.
- S1 reduces allocator-retained storage by exactly 196,608 bytes on every
  workload and lowers peak live capacity by 41,240-134,232 bytes. Its process
  RSS is unchanged to +1.512%; no guard fires.
- S1b reduces measured process RSS by 1.631-2.488% on every workload and has
  the best balanced timing/memory screen, but its larger locally retained
  slabs increase allocator-retained storage by 589,824-917,504 bytes.
- S2 improves Towers by +1.689% and RexxCPS by +1.289%, but it raises Sieve
  peak RSS by 966,656 bytes (+5.247%), breaching the positive 3% screen guard.
  Its locally retained slabs add 2,424,832-3,145,728 bytes. S2 must not advance
  unchanged, particularly because that local cost will multiply by worker.
- Base64 reverses direction from the invalid scratch capture and remains noisy
  and non-selecting.

Recommended direction: reject S2 and run the formal 12-round S0/S1/S1b
product-lane panel with separate RSS/allocator scorecards. Stop remains active:
no formal panel, rxtvm finalist diagnostic, V1 or reclamation PoC starts until
Adrian accepts the verdict and directs the next slice.

Adrian accepted that verdict and approved the formal survivor panel on
2026-08-06. S2 remains rejected unchanged and is excluded. The authorized
slice is the ordinary profiling-off Release `rxvm` product lane for S0, S1 and
S1b: one warmup plus 12 exactly balanced timing rounds, followed by separate
bounded RSS and allocator scorecards. The stop remains active before any
`rxtvm` diagnostic, V1 layout or reclamation PoC.

### C2 formal survivor verdict — complete 2026-08-06

Retained evidence:
[`2026-08-06-perf3-13-gate-c-c2-geometry-formal-verdict`](evidence/2026-08-06-perf3-13-gate-c-c2-geometry-formal-verdict/).
All 765 timing executions pass: 21 warmups and 744 recorded samples across the
initial formal block and the exact noise/uncertainty appends. Sieve and Base64
finish with 34 pairs; the other five workloads reach the governed 36-pair
ceiling. No sample was removed. The separate RSS panel passes 84/84.

The final timing estimates remain inside the approved guards:

- S1 has a 0.997964 stable-six median ratio versus S0 (-0.204%). It is clearly
  adverse on Sieve (-0.154% paired median) and Bounce (-0.627%); the other four
  stable rows are inconclusive at the ceiling. It saves exactly 196,608 bytes
  of allocator-retained slabs per workload, but process RSS is 0.22-1.74%
  higher.
- S1b has a 1.003752 stable-six median ratio (+0.375%). Richards (+0.371%) and
  RexxCPS (+0.848%) are clear favourable; the other four stable rows are
  inconclusive at the ceiling. Process RSS is 1.69-2.49% lower, but locally
  retained slabs rise by 589,824-917,504 bytes per worker with no peak-live
  allocator-capacity reduction.
- Base64 remains noisy/inconclusive and non-selecting. All three `rxbvm`
  artifacts are exactly 1,000,936 bytes.

Recommendation: provisionally select S0 (64 KiB slab, 16 KiB maximum slab
class) for the later value-shape panel. S1 trades small clear throughput/RSS
costs for 192 KiB less retained allocator capacity; S1b trades 576-896 KiB
additional per-worker retention for a small neutral-band timing/RSS benefit.
S0 is the simpler balanced substrate. This is a recommendation pending Adrian,
not authority to begin `rxtvm`, V1, reclamation or Gate D industrialisation.

Adrian accepted S0 and authorized the value experiments on 2026-08-06. S0 is
now frozen for M3: 64 KiB slabs, 16 KiB maximum slab class, byte-normalized
depot limits and R0 reclamation. S1/S1b/S2 remain retained evidence, not active
value-layout dimensions.

### V1 first Release design-selection contract

V1 is an isolated representation PoC, not a production ABI selection:

1. Add a closed interpreter-build selector for V0 and V1. V0 defines no new
   preprocessor behavior and must reproduce the retained 240-byte S0 control;
   V1 alone removes `small_string_buffer[32]`, targeting exactly 208 bytes.
2. Keep `SMALLEST_STRING_BUFFER_LENGTH=32` as the minimum power-of-two string
   sidecar class. A fresh V1 value has a null string pointer and zero capacity;
   its first string preparation lazily allocates at least 32 bytes through the
   S0 worker allocator. Logical reset keeps that buffer sticky.
3. Retain the direct `string_value` pointer and every current string/UTF length
   and cache width. Do not introduce capacity codes, descriptors, cold
   sidecars, reclamation checks or a second pointer indirection in V1.
4. Move transfers an allocated string sidecar without copying. Copy retains
   the existing destination-owned buffer/reuse semantics. Physical destruction
   returns any allocated sidecar through the existing allocator family.
5. Focused proof must cover the exact 240/208-byte layouts, initial no-buffer
   state, first small allocation, sticky reset, growth, copy, zero-length
   strings, move transfer, destruction and allocator failure/ownership
   counters. The default V0 product needs a machine-level replay check.
6. After the minimum correctness proof, freeze implementation and run the
   ordinary profiling-off Release `rxvm` product against the retained S0/V0
   binary: one warmup plus four balanced rounds on the seven accepted
   workloads, with separate RSS and allocator telemetry. Stop for Adrian at
   the first verdict; no V2a, `rxtvm` or reclamation work starts automatically.

### V1 first Release verdict — complete 2026-08-06; awaiting decision

Retained evidence:
[`2026-08-06-perf3-13-gate-c-v1-first-release-verdict`](evidence/2026-08-06-perf3-13-gate-c-v1-first-release-verdict/).
V0 is exactly 240 bytes, V1 is exactly 208 bytes, and both profiling-off S0
Release builds pass the three focused tests. The fresh V0 product's complete
203,865-line normalized `__text` body is identical to retained S0 at
`253af30970ff83d616923864ce9f029eaeaa88bda3bf44fe8150573d51490235`.

The first scratch capture correctly stopped when V1 RexxCPS exposed a hidden
inline-buffer dependency: the nine-digit integer-to-decimal zero path wrote
`"0"` before acquiring storage. V1 now prepares its minimum sidecar on that
path and the focused test covers it. The V1-only correction preserves exact V0
machine identity. It also exposed separate runner debt: an abnormal child
status 139 was recorded as zero. Do not rely on that runner field for abnormal
termination until corrected; the raw correctness contract still stopped the
capture.

The authoritative short panel passes all 70 executions. V1's stable-six
geometric V1/V0 ratio is **1.010915174** (+1.09%). Paired medians are Sieve
+1.593211%, Permute -0.549362%, Bounce -1.964423%, Richards +3.589034%,
Towers +1.110094% and RexxCPS +1.774387%. Sieve, Richards and RexxCPS are clear
favorable; the other stable cells are inconclusive and no workload is clear
adverse. Base64 is -0.063439%, noisy/inconclusive and non-selecting.

Recommendation: accept the first verdict and promote V1 to the formal V0/V1
survivor comparison, including separate bounded RSS/allocator telemetry and a
simplicity scorecard. This is not a production layout selection. The mandatory
stop remains active before that formal panel, V2a, `rxtvm`, reclamation or Gate
D work.

Adrian accepted that recommendation on 2026-08-06 and authorized the formal
M3 V0/V1 survivor comparison. The frozen contract is:

1. Reuse the exact post-repair first-verdict S0/R0 profiling-off Release V0
   and V1 products, accepted Gate B optimized images/library and 14-cell
   manifest. Reprove exact product/source hashes and explicitly audit the old
   pre-repair normalized-text relationship before timing; make no production
   edit during the campaign.
2. Run one warmup and 12 serial pairwise/position-balanced recorded rounds for
   all seven workloads. Base64 remains captured but non-selecting; stable-six
   membership remains Sieve, Permute, Bounce, Richards, Towers and RexxCPS.
3. Apply the standing uncertainty rules exactly. Append ten unchanged serial
   rounds only to workloads whose absolute series exceeds 3% relative MAD or
   10% min/max span. Append balanced 12-pair blocks only where the paired mean
   interval crosses zero or a regression guard, capped at 36 total pairs. Keep
   every valid sample and retain inconclusive ceilings honestly.
4. Run peak RSS separately with zero warmups and four balanced recorded rounds
   per cell. Run each manifest cell once with
   `CREXX_RXVM_MEMORY_STATS=1`; timing, process RSS and allocator-owned memory
   remain separate scorecards.
5. Record exact value/product/hot-text size plus representation states, direct
   pointer/indirection count, hot branches, first-use allocation, sticky reset,
   move/copy behavior, failure paths and oversized/foreign exceptions in a
   V0/V1 simplicity scorecard.
6. Select or reject V1 against zero correctness failures, the 1% aggregate,
   3% workload, 5% plus 1 MiB RSS and 5% plus 4 KiB artifact guards, together
   with deterministic allocator retention and complexity. Stop for Adrian at
   that formal result. V2a, `rxtvm`, reclamation and Gate D remain closed.

### V1-L01 bounded small-string spatial-locality diagnostic — closed

This optional diagnostic answers one question before the renewed formal M3
campaign: does removing the inline buffer hurt a repeatedly reused
one-character string because its sidecar loses spatial association with the
owning `value`? It does not redesign Base64, select a prefetch instruction or
authorize a production VM change. General `run()` hot/cold restructuring is
owned by the queued `PERF3-05-R1` RXVM refactor.

1. Compile one exact Level B/RXAS microprobe whose measured kernel is repeated
   one-character register-to-register string copy plus only the necessary loop
   control. Prove the emitted RXAS/opcode shape and run the identical RXBIN on
   the repaired profiling-off S0/R0 V0 and V1 products.
2. Establish every source and destination string, including V1's sticky
   32-byte sidecars, before resetting the internal timer. Measure fixed working
   sets of 8, 128 and 512 source/destination pairs: comfortably below L1D,
   around the 64 KiB host L1D boundary, and above L1D but below L2.
3. For each size compare an ordinary copy pass with a sidecar-warm pass that
   first reads one byte from every source and destination. Report the timed
   copy-only warm result as an upper bound and separately include the complete
   pre-touch cost; do not treat preloading as free work.
4. Keep traversal sequential first. Add one deterministic permuted traversal
   only if the sequential result cannot distinguish spatial streaming from
   pointer-following locality. Record correctness, exact copy count, ordinary
   Release elapsed time and available instruction/data-cache diagnostics.
5. Stop after the bounded result. A V1 loss removed by pre-touch records a
   cache/locality concern for the value-shape scorecard; a loss even for the
   smallest already-hot set points to handler/representation cost; no material
   V0/V1 difference closes the hypothesis. Any cache-colouring, prefetch,
   immediate-string or borrowed-slice candidate requires a separate approved
   design gate.

Adrian approved V1-L01 on 2026-08-06. The private scratch generator, nine
no-optimizer RXBIN cells and six-cell pairwise-balanced capture manifest were
proved before timing. One-batch correctness was 18/18 across repaired V0 and
V1, disassembly proved the intended `SCOPY`/`STRCHAR` shapes, and V1 allocator
telemetry proved the exact 1,024 additional 32-byte sidecars for the 512
source/destination pairs. The first scratch timer design was rejected before
measurement because `MTIME` includes its own `localtime()` work after the start
timestamp; the retained probe warmed and used `XTIME "T"` process-CPU ticks and
retained runner wall elapsed separately.

The exclusively reserved formal capture closed at 588/588 passing executions.
The initial 12 recorded pairs were retained; two unchanged balanced 12-pair
blocks were appended for the complete noisy 8- and 128-pair groups, reaching
the 36-pair cap, while the already decisive 512-pair group remained at 12. The
paired V1/V0 copy-rate means and 95% intervals were:

- 8 ordinary: -0.36% (-1.46%, +0.74%); warm copy-only -0.25% (-1.33%,
  +0.83%); inclusive +0.26% (-0.55%, +1.06%);
- 128 ordinary: -0.21% (-1.43%, +1.01%); warm copy-only +3.24% (+2.05%,
  +4.43%); inclusive +3.85% (+2.43%, +5.27%);
- 512 ordinary: +1.02% (+0.90%, +1.14%); warm copy-only +0.64% (+0.45%,
  +0.84%); inclusive +0.68% (+0.45%, +0.91%).

Pre-touch did not rescue a V1 regression. At 128 pairs its apparent V1 benefit
came from V0's subsequent copy-only rate falling 3.27% while V1 stayed neutral;
the full pre-touch cost was about 7.79% V0 / 7.31% V1. At 512 pairs the full
cost was about 14.21% / 14.17%, while V1 remained favorable. The repeatedly
hot 8-pair set was neutral. The conditional permuted traversal was therefore
not opened. Raw one-character `SCOPY` sidecar distance is closed as the cause
of the repaired Base64 loss; take unchanged V1 into renewed formal M3 and keep
Base64/library redesign plus general `run()` hot/cold layout under their
separate owners. No prefetch, colouring, immediate-string or borrowed-slice
candidate is authorized by this result.

A separate future item, `PERF3-13-F1`, records a possible single-character
`SCOPY` fast path. It is post-M3 and evidence-gated: first measure incidence
and exact value shapes across the representative portfolio, then compare an
exact C ceiling with narrow forms that preserve allocation, aliasing, UTF and
all value-state semantics without imposing a material branch or code-layout
cost on ordinary copies. V1-L01 did not select this idea, Base64 alone cannot
justify it, and no implementation is authorized in the current gate.

## M3 — isolated value-shape comparison

Compare independently replayable layouts on the provisionally selected slab
substrate. Every rung remains a scratch PoC until Gate D:

- [x] V0: unchanged 240-byte `value` control; corrected C2 first verdict and
  formal geometry survivor panel complete; S0 accepted by Adrian.
- [x] V1: 208-byte formal V0/V1 timing, RSS, allocator and simplicity
  comparison complete; accepted by Adrian as the next-rung baseline.
- [x] V2a: 192-byte byte-capacity-code PoC completed and rejected at its first
  Release verdict; retained as evidence because its memory result is sound but
  its hot decode expands `run()` and breaches CPU/artifact guards.
- [x] V1C32: exact 200-byte direct-`uint32_t` capacity discriminator completed
  and rejected at its first Release verdict; its stable-six loss breaches the
  guard despite uniformly lower RSS medians.
- [x] L32S: keep every allocation capacity and binary actual length as raw
  `size_t`; narrow only string byte length, character count and two private UTF
  cache positions through checked `uint32_t` boundaries. Target exactly 192
  bytes without any capacity encoding or hot decode.
- [x] L32-D/O: closed by Adrian after the decimal audit and before build. Do
  not pursue further in-struct logical-size/count packing; the extra layout and
  ABI complexity is not justified after rejecting capacity compression.
- [x] V3: audit and move only proved-cold decimal/native and object-growth
  metadata to typed sticky sidecars allocated from the existing worker byte
  slabs. Retain direct hot string, binary, reference, `object_type`, active
  `attributes` and `num_attributes` fields. Confirm candidate sizes from the
  compiler rather than assuming the earlier 112-136-byte model. The complete
  factorial/backoff panel selected only the co-allocated decimal header.
- [x] V4: complete the maximize/backoff ceiling. The 160-byte decimal/object
  form was measured and rejected despite numerical guard clearance because it
  adds 25% Towers allocation calls for only 44,800 further peak bytes saved.
- [x] Use one sidecar lifecycle/ownership protocol: worker-owned slab
  allocation, lazy acquisition, sticky reuse, stable address and common
  init/copy/move/reset/destroy hooks. Do not require one universal sidecar
  shape or allocate during logical reset.
- [x] Compare direct-pointer sidecars with descriptor forms adding a hot
  indirection; do not assume the smallest header wins.
- [x] Select R0 (no automatic runtime reclaim) for Gate D. R1 explicit
  value-sidecar reclaim and R2 bounded pressure-triggered reclaim remain
  separately gated future work; no per-reset reclaim check enters the hot path.
- [x] Measure initialization/reset/copy/move cost, first-use allocation,
  steady-state reuse, cache/page density, RSS, fragmentation and teardown.
- [x] Record a simplicity scorecard for field/sidecar count, representation
  states, hot loads/branches/indirections, first-use allocations, compiled VM
  and hot-function size, failure paths and foreign/oversized exceptions.
- [x] Use short balanced screens only to reject broken/clearly regressive PoCs.
  Compare all survivors in one 12-round pairwise-balanced ordinary Release
  panel; `rxvm` is the product authority and `rxtvm` is a finalist
  diagnostic on Apple Clang. The selected Apple product resolves to `rxbvm`;
  no duplicate `rxvm` cell was claimed.
- [x] Record Base64 but do not let its noisy elapsed result select the policy.
  Keep allocator memory, process RSS and timing scorecards separate.
- [x] Select the value shape, slab/oversize geometry and reclamation policy
  together on speed/memory/complexity evidence, then stop for Adrian at the
  Gate D boundary.

### M3 renewed formal verdict — complete and accepted 2026-08-06

The exact repaired V0/V1 products and 14-cell manifest were unchanged from the
accepted short verdict and V1-L01. Their SHA-256 identities are respectively
`bbdf064bdff32a074d7dbfe2c2320bea2b78a1caba4b6a1bfa80375ec57a78df`,
`ef61e3b0f54df2dfb9693d8cc406803e5908f9c56862de76ca0257664b2fa330`
and `a261b022d0cfc4f16f321c36dfd0812fe9ed5a23c181fcd6d9bdedcdde94e72a`.
The old `253af...` normalized-text identity is correctly superseded: it
describes the 203,865-line pre-repair V0, while the common stem
allocator-family repair changed both candidates. Renewed M3's repaired V0 has
204,275 normalized text lines at `69e67b...` and does not compare equal to that
obsolete body. Exact post-repair whole-product hashes are the frozen identity
proof.

Formal timing passed 298/298 executions. Base64 alone received its required
ten-round absolute-noise append. Permute and RexxCPS received two 12-pair
uncertainty blocks and remained modestly positive but inconclusive at the
36-pair cap. Final V1-favorable mean/95% interval results are Sieve +1.411139%
(+1.075320%, +1.746958%), Permute +0.411581% (-0.020554%, +0.843717%),
Bounce +1.025301% (+0.498568%, +1.552035%), Richards +4.066471%
(+3.790767%, +4.342176%), Towers +1.913820% (+1.557713%, +2.269927%)
and RexxCPS +0.343556% (-0.219944%, +0.907056%). The stable-six geometric
V1/V0 throughput ratio is **1.015820** (+1.582%). Base64 is clearly adverse at
-13.258688% (-18.362980%, -8.154396%) across 22 pairs but remains the
predeclared non-selecting CAP-03 library case.

Separate RSS passed 56/56 launches; every median V1 delta lies between -0.31%
and +0.40%, with all guards clear. Allocator telemetry passed 14/14 with zero
failures, invalid frees or wrong-owner frees. V1 never raises retained slab
bytes, lowers peak live capacity in every cell and saves one retained 64 KiB
slab in Richards. RexxCPS's runtime-calibrated printed rate accounts for a
three-allocation repeat difference, but retained topology and peak live
capacity are unchanged. The product grows only 16 bytes while `__text` falls
724 bytes and `run()` falls 2,948 bytes.

Adrian accepted V1 on 2026-08-06 as the measured baseline for the next shrink
rung, not as the final Gate C shape. V2a is now authorized as an isolated
192-byte PoC. It removes the raw string/binary capacity fields in favor of
byte power-of-two capacity codes with distinct managed-metadata and foreign
escapes,
while preserving current actual-length widths, direct pointers, sticky reset,
copy/move ownership and R0. Freeze after focused correctness and stop at its
smallest decisive V1/V2a ordinary Release verdict. V2b, `rxtvm`, reclamation,
Gate D code and `PERF3-13-F1` remain closed. Private V1 evidence is retained at
`/private/tmp/crexx-rxvm-inline.yvLywZ/results/gatec-m3-renewed`.

### V2a first Release design-selection contract

V2a is the next isolated representation PoC, with V1 as its exact control:

1. Extend the closed interpreter-build selector with V2a. V1 must still emit
   the frozen 208-byte value and reproduce its repaired product identity where
   source-neutral access macros permit; V2a alone targets exactly 192 bytes.
2. Remove the two eight-byte raw string/binary capacity fields. Store two
   one-byte capacity codes in the existing alignment space after
   `native_payload_flags`; do not repurpose public or VM-private type flags.
3. Code zero means unattached. Canonical power-of-two capacities encode their
   exponent directly, including managed oversized powers. One reserved escape
   recovers non-canonical managed capacity from allocator metadata; a distinct
   foreign escape uses logical length for existing compiler-owned read-only
   constants without probing before an external pointer. No new sidecar or
   pointer indirection is introduced.
4. Keep `size_t` string/binary actual lengths, direct data pointers, the 32-byte
   minimum class, sticky R0 reset, destination-owned copy, pointer-transfer
   move, physical destruction, native payload operations and allocator-family
   ownership unchanged. Capacity encoding/decoding must be checked and exact.
5. Route both layouts through common capacity access/set macros. V1 macros must
   compile to direct raw-field loads/stores; V2a's ordinary hot decode must not
   consult allocator metadata. Metadata lookup is escape-only.
6. Focused proof covers exact 208/192-byte layouts, zero/standard/grown/
   oversized/foreign escape codes, string and binary allocation failure,
   sticky reset, copy, move, physical destruction, stem mutation, native
   payloads and allocator correctness counters.
7. After focused correctness, freeze both profiling-off ordinary Release
   products. Run one warmup plus four balanced rounds over the accepted seven
   workloads, with Base64 recorded but non-selecting, plus separate bounded RSS
   and allocator telemetry. Stop for Adrian at that first verdict. V2b,
   `rxtvm`, reclamation and Gate D remain closed.

### V2a first Release verdict — rejected 2026-08-06

The frozen accepted V1 control and isolated V2a product passed 10/10 focused
Release checks. The balanced first verdict passed 70/70 timing executions,
56/56 RSS launches and 14/14 allocator-telemetry cells. V2a is nevertheless a
clear performance and artifact rejection:

- stable-six V2a/V1 throughput is **0.955095** (-4.490492%);
- paired V2a-favorable means are Sieve -7.369217%, Permute -11.152591%,
  Bounce -9.158157%, Richards +3.741962%, Towers +1.686154% and RexxCPS
  -3.588349%; Base64 is +1.276840% and inconclusive/non-selecting;
- `value` reaches exactly 192 bytes and peak live allocator capacity falls in
  every cell, while RSS medians remain within -0.66% to +0.82%; Towers alone
  retains one additional 64 KiB slab;
- `rxbvm` grows 66,048 bytes (+6.598518%), `__text` grows 72,688 bytes
  (+8.931134%) and flattened `run()` grows 68,636 bytes (+12.902765%).

This rejects the current byte-code/accessor form, not the general value-density
goal. The evidence points to repeated exponent/escape decode in flattened hot
paths rather than a memory-ownership defect. Adrian approved V1C32 as the next
bounded discriminator and did not authorize V2b.

### V1C32 first Release design-selection contract

V1C32 is an isolated intermediate PoC with frozen V1 as its exact control:

1. Extend the research selector with `V1C32`; preserve V1 at 208 bytes and
   target exactly 200 bytes for V1C32.
2. Remove the two `size_t` capacity fields from their original positions and
   place adjacent `uint32_t` string/binary capacities after
   `native_payload_flags`. Do not alter actual-length widths, public flags or
   direct payload pointers.
3. Store every ordinary capacity directly, without exponent decode. Reserve
   explicit managed-metadata and non-probing foreign sentinels only for values
   not representable directly; ordinary access must not consult metadata.
4. Preserve the 32-byte minimum class, power-of-two allocation policy, sticky
   R0 reuse, failure atomicity, destination-owned copy, pointer-transfer move,
   physical destruction, stems, native payloads and allocator ownership.
5. Focused proof covers exact 208/200-byte layouts, zero/standard/grown/
   oversized capacities, managed and foreign exceptions, reset/copy/move/
   destroy, stems, native payloads and allocator counters.
6. After focused correctness and a fresh exclusive-host confirmation, run the
   same one-warmup/four-round seven-workload timing panel, separate four-round
   RSS and one-shot allocator telemetry. Stop at that verdict before V2b,
   `rxtvm`, reclamation or Gate D.

### V1C32 first Release verdict — rejected 2026-08-06

The exact 200-byte candidate passed 5/5 focused checks, 70/70 balanced timing
executions, 56/56 RSS launches and 14/14 allocator-telemetry cells. It is
rejected because stable-six V1C32/V1 throughput is **0.988526** (-1.1474%),
just outside the 1% selection guard. Paired candidate-favorable means were
Sieve +0.175%, Permute -1.597%, Bounce -0.452%, Richards -1.717%, Towers
-2.005% and RexxCPS -1.853%; Base64's +17.57% result remains noisy and
non-selecting. All RSS medians improved by 0.13-0.65%, peak allocator capacity
fell in every cell, and Towers retained one additional 64 KiB slab. The
product grew 3.299%, `__text` 4.497% and flattened `run()` 6.483%.

This closes allocation-capacity compression in Gate C. Adrian explicitly
rejected a V1C32 recovery rung: allocation capacities stay raw `size_t`, with
no codes, sentinels, relocation or exceptional decode in the selected design
path. Frozen 208-byte V1 remains the exact control.

### L32S first Release design-selection contract

L32S is the next isolated representation PoC, derived directly from frozen V1:

1. Extend the research selector with `L32S`; preserve V1 at 208 bytes and
   target exactly 192 bytes for L32S.
2. Keep string and binary allocation capacities as direct `size_t` fields.
   Keep binary actual length, decimal lengths, attribute counts, pointers,
   flags and ownership semantics unchanged. There is no compressed allocation
   size, sentinel, metadata fallback or capacity decode.
3. Place direct string capacity before one adjacent four-field group:
   `uint32_t string_length`, `string_chars`, `string_cache_byte_pos` and
   `string_cache_char_pos`. The fields store actual values, not codes.
4. Perform allocation arithmetic and untrusted length ingress in `size_t`.
   Check exactly once where a string can enter or grow beyond the 32-bit
   contract, before buffer or field mutation. Recoverable validated ingress
   returns failure; impossible internal overflow branches to one outlined cold
   failure target. Finishing, copies, truncation, UTF counts/cache movement and
   other invariant-preserving stores are guard-free and directly assigned.
5. Preserve the 32-byte minimum class, power-of-two slabs, oversized handling,
   sticky R0 reset, copy/move/destroy behavior, stems, native payloads and
   allocator-family ownership.
6. Focused proof covers exact 208/192-byte layouts, native-width capacity and
   binary-length fields, maximum accepted logical metrics, maximum-plus-one
   failure atomicity, validated ingress, UTF/cache invariants, sidecar growth,
   reset/copy/move/destroy, stems, native payloads and allocator counters.
7. After focused correctness, freeze ordinary profiling-off Release products.
   Request a fresh exclusive-host reservation, then run the same smallest
   decisive seven-workload timing/RSS/allocator panel and stop at its first
   verdict. Decimal/object narrowing, `rxtvm`, reclamation and Gate D remain
   closed.

The reduced-guard L32S candidate passed the focused Release set 5/5:
`rxvmstem_storage`, `rxvmstem_allocator_family`, `rxvmmemory_allocator`,
`rxpa_utf_validation` and `ts_regvalue_tester`. Proof includes the exact
192-byte layout, raw native-width string/binary capacities and binary length,
maximum and maximum-plus-one failure-atomic metric boundaries, validated
ingress, and UTF seeks above `INT_MAX`. The ordinary Apple Clang `rxbvm` is
frozen at SHA-256
`2d3dc6c8df5e260cf6100413ee11d0bf4d497ba6ca775e12ebb9494abf138174`.
Against frozen accepted V1, the product is only 48 bytes larger, `__text` is
540 bytes smaller and flattened `run()` is 768 bytes smaller. Disassembly has
one shared call to the outlined overflow target; no per-read decode or guard
exists. Timing remains closed pending a fresh exclusive-host confirmation.

### L32S first Release verdict — accepted 2026-08-06

The frozen products passed 5/5 focused Release checks, 70/70 balanced timing
executions, 56/56 RSS launches and 14/14 allocator-telemetry cells. Stable-six
L32S/V1 median throughput is **1.011762** (+1.1762%). Candidate-favorable
paired means and 95% intervals are Sieve +1.718862% (+0.826086%, +2.611638%),
Permute +0.375459% (-0.245542%, +0.996460%), Bounce -3.009290%
(-6.960093%, +0.941513%), Richards +4.475931% (+2.317500%, +6.634362%),
Towers +3.409316% (+3.047610%, +3.771022%) and RexxCPS +0.345867%
(+0.151833%, +0.539901%). Base64 is -6.140682%
(-17.072814%, +4.791450%), noisy and predeclared non-selecting.

Median RSS improves by 0.175-0.396% in six cells; RexxCPS is +0.0858%.
Peak live allocator capacity falls in all seven cells. Retained slab bytes are
unchanged in six; Towers holds one additional 64 KiB slab while lowering peak
live capacity 5.1862% and system slab acquisitions from 35 to 32. All cells end
with zero live allocations and report zero failures, invalid frees and
wrong-owner frees.

The candidate reduces `value` by 16 bytes (7.6923%). Against frozen V1 the
whole product is +48 bytes (+0.0048%), while `__text` is -540 bytes (-0.0663%)
and flattened `run()` -768 bytes (-0.1444%). The first verdict therefore
passes and the recommendation is to accept L32S as the next Gate C baseline.
Stop here for Adrian: decimal/object narrowing, reclamation, `rxtvm`, Gate D
and broad closeout remain closed.

Adrian accepted L32S on 2026-08-06 as the frozen Gate C baseline. The governing
rule is now explicit: do not encode, compress, mask, shift or decode allocation
sizes or capacities. Keep capacities and allocation arithmetic as raw
`size_t`. Adrian subsequently closed further in-struct logical-field packing;
remaining shape work is limited to cohesive cold typed sidecars whose hot-path
and ABI costs are explicitly proved.

### Direct decimal narrowing audit — closed before build

The audit distinguished `decimal_value_length` from the allocation-capacity
field: the discarded sketch kept `decimal_buffer_length` and every allocation
operation as raw `size_t` and proposed no bit encoding. It could save only one
eight-byte alignment unit, while rearranging the native/plugin-visible `value`
ABI. Adrian closed that direction on simplicity grounds before any candidate
build, correctness run, artifact or timing result. No L32D evidence exists and
none should be inferred.

Further shape work used one rule: keep hot fields direct and move a proved-cold
cohesive field family behind typed sticky storage. Sidecars and variable
payload buffers use the existing worker-owned power-of-two slab allocator and
raw `size_t` capacities; no second allocator, compressed sizes or decoded hot
representation was introduced.

### Typed-sidecar maximise/backoff verdict — selected and accepted 2026-08-07

Adrian authorized a maximise-first factorial and directed the work to continue
without another approval stop until every material option had timings. The
complete D/N/O factorial covered 192 through 152 bytes. Conservative DP/O1
backoffs, a co-allocated decimal header and the two remaining decimal/object
interactions then isolated the implementation costs.

The selected representation is **L32SDH at 176 bytes**:

- decimal payload pointer remains hot and direct;
- raw `size_t` length and capacity form a fixed header immediately before the
  payload;
- header and payload are one sticky worker-slab allocation;
- object growth and native metadata remain direct in `value`.

Two independent 12-pair core-four blocks give L32SDH +0.847% with a 95%
interval of +0.398% to +1.296%. Sieve is +0.474%, Richards -0.342%, Towers
+2.893% and RexxCPS +0.363%; no selected workload crosses its guard. Peak live
allocator capacity falls 0.462-3.597% in all six measured cells, retained slab
bytes are unchanged and direct RSS medians move only +0.043% to +0.221%.
Against frozen L32S, `value` is 8.333% smaller, the file is +48 bytes, `__text`
is +2,104 bytes and flattened `run()` is +2,276 bytes.

The 160-byte L32SDHO maximum is numerically guard-clean but is rejected as the
wrong simplicity/per-worker trade. Its formal core-four result is +0.590%
(-0.454%, +1.633%) and Towers is -1.077%. Towers performs 12,071,103 extra
object-descriptor allocations (+25.0% allocator calls) to save only another
44,800 bytes of peak capacity beyond L32SDH; `__text` grows 7,152 bytes and
`run()` 6,300 bytes. L32SDHO1 crosses the Towers guard at -3.004% in screening.

Native sidecars are dominated: eight bytes cost about 20 KiB text and repeated
regressions. The separate DP decimal descriptor is also rejected: RexxCPS
reaches 10.74M tracked allocations and 66.2 MB cumulative internal
fragmentation. Co-allocation removes that second descriptor allocation.

Base64 remains non-selecting. It changes sign from -7.194% for L32SDH to
+3.204% for nearby L32SDHO with wide intervals, reinforcing the separate
CAP-03 Level B library/API task. JSON remains neutral.

The retained decision report is
[`2026-08-06-perf3-13-gate-c-m3-sidecar-decision`](evidence/2026-08-06-perf3-13-gate-c-m3-sidecar-decision/).
Adrian accepted this Gate C decision on 2026-08-07 and opened Gate D. Gate D
reimplements only L32SDH cleanly, syncs legacy decimal fixtures to value
lifecycle ownership and discards all research-only N/O/DP selectors and
compatibility macros. The retained performance report remains the authority for
the rejected alternatives; they are not production build options.

## M4 — selected representation industrialisation

Adrian approved Gate D on 2026-08-07. M4's Mac local closeout is complete after
its accepted mandatory first Release verdict; ordered cross-platform ABI
validation remains.

- [x] Reimplement only the selected allocator/layout cleanly in production.
- [x] Industrialise R0 only: sticky logical reset, no automatic or pressure
  reclamation on the hot path, one local empty slab per class, bounded depot
  reserve and explicit quiescent depot trim. R1/R2 remain a separate Gate D-R.
- [x] Define the `rxvml`/RXPA/plugin ABI transition as internal version 2 and a
  rebuild-together contract. Decimal engines reserve co-allocated storage
  through the host service and never free the payload pointer directly.
- [x] Preserve oversized and foreign/native payload handling.
- [x] Run the mandatory first ordinary Release verdict and stop before closeout.
  Adrian accepted the +0.968% pooled core-four result on 2026-08-07.
- [x] Publish enduring RXVM design documentation and retain Gate C selected and
  rejected-option evidence plus the Gate D first-verdict bundle.
- [x] Complete proportional Mac correctness, sanitizer, lifecycle and isolated
  install/package validation after the accepted first verdict.
- [ ] Complete rebuild-together ABI validation in order on Intel Linux, Linux
  ARM64 and same-machine Windows before closing Gate D globally.
- [x] Retain telemetry, explicit trim and deterministic teardown; remove
  disposable candidates from production while retaining their evidence.

### M4 Mac local closeout

The local industrialisation candidate is frozen. The final focused Debug and
ordinary Release sets pass 15/15. The full 30-way Debug sweep passes
1,887/1,909; serial rerun clears two parallel transients and leaves exactly 20
spawn-dependent failures. Each remaining test reports only allocator teardown
live allocations and crosses the `crexx`/ADDRESS CREXX spawn path. Diagnostic
instrumentation proved that `crexxcmd_run_argv()` lets its stdout and stderr
reader threads enter one `memory_worker`: concurrent 32-byte sidecar
allocations can receive the same worker-local slot. The child VM tears down
cleanly. This is the accepted spawn migration gap, not a general L32SDH
lifecycle defect; Gate E must give every allocating OS thread a distinct
worker arena and copy/transfer results into receiver-owned storage.

Broad testing also found and fixed a real UTF correctness defect:
`POSCHAR_REG_REG_REG` used byte length as its character-index bound. It now
uses `string_chars` in UTF builds and retains `string_length` for `NUTF8`.
The formerly failing Arabic-digit `datatype` case passes under both `rxbvm` and
`rxtvm`. AddressSanitizer then found that the test-injected external allocator
path tried to recover slab capacity from a libc allocation. That path now uses
the exact requested size; the production worker allocator branch is unchanged.
The final affected ASan panel passes 12/12 plus 1/1 Unicode no-opt, with macOS
LeakSanitizer unavailable. An independent ASan linked-runtime setup failure is
a pre-existing `rxas_flow_proof.c` use-after-free outside this RXVM diff.

Two fresh post-`POSCHAR` ordinary-Release blocks pass 208/208 processes. Their
combined 96-pair core-four result is +1.205% with a 95% interval of +0.748% to
+1.663% and 80/96 favorable pairs. Sieve is +1.573%, Towers +2.717%, RexxCPS
+1.229% and Richards a neutral -0.699%; no 3% regression guard is hit. The
final artifact remains 1,001,048 bytes with 815,420-byte `__text` and a
533,440-byte flattened `run()`. An isolated install selects `rxvm -> rxbvm`
and passes a real installed-product Sieve run. Retained evidence:
[`2026-08-07-perf3-13-gate-d-local-closeout`](evidence/2026-08-07-perf3-13-gate-d-local-closeout/).

### M4 develop integration

The selected Gate D product was composed over clean `develop` base
`086d6edc6a26475d65323e264cf9b6118a8c1de3` as one production change. The nine
Gate B/C/D research commits are not imported as a maintained alternative
history. Their measurements, decisions and rejected-option rationale remain in
this worklist and the checksum-closed evidence bundles.

The integration preserves the current decimal-provider work, including the
optimizer safeguards from `20cc10c38` for repeated decimal formatting: the
prepared register is retained while sign facts are used, decimal symbols are
not propagated into repeatedly parsed immediates, and the RexxCPS decimal
register-integrity guard remains. Decimal-plugin temporary string storage now
uses the VM host allocator services, so the newer formatting path and the
selected sidecar ownership model use one allocator family.

Fresh pre-EF-0 Debug and ordinary profiling-off Release builds both passed the
complete Decimal Gate 1 qualification set (81/81) and the combined
allocator/value, decimal, datatype and canonical RexxCPS focused set (17/17).
The pre-EF-0 broad Debug sweep was not globally green: it initially passed
1,967/1,990 and a serial rerun cleared one transient, giving an effective
1,968/1,990. The exact 22 survivors all produced their expected result and
then reported live allocations at VM teardown through a `crexx`/spawn driver
path. The direct-VM dynamic-interface test passed; only its `crexx` driver form
shared this ownership gap. EF-0 below closes that accepted Gate E migration
gap without changing the selected value layout or ordinary allocator path.

Production retains only the selected implementation. Disposable inlining and
flattening experiment controls have been removed; their negative evidence is
retained in the Gate B dispatch study. `reclaim_attribute_storage()` remains
as an explicit quiescent operation with no automatic hot-path caller, which is
part of selected R0 rather than a second reclamation policy.

## EF-0 — spawn I/O ownership and transfer recovery

Adrian changed the Gate E/F sequence on 2026-08-07. Gates E and F opened
together only for the minimum worker-ownership and transfer semantics needed
to restore the existing spawn/`crexx` redirect paths to green. EF-0 is the
first bounded vertical slice of the coherent multithreading architecture: it
establishes an ownership-safe provider/completion boundary that full Gate E
worker execution and full Gate F programmable channels can extend. Adrian
accepted its first Release verdict on 2026-08-07. The local closeout is green
and was published on `develop` in `642e1b697`; the synchronized continuation
base is `19802842e`. Adrian approved the full M5 architecture and immediate E1
slice on 2026-08-07. Full M6 remains closed until Gate E's worker model is
selected.

The unmodified `9e2e51c20133ece39d12d3b4e113d130b74b2af8` baseline was
rebuilt before production edits. A fresh 30-way Debug sweep passed
1,965/1,990. Its 25 failures moved around the same redirect race: the full run
added `ts_address_capture` variants and the Level C driver smoke while the
late-interface driver happened to pass. A serial union/control run then passed
7/30 and failed 23/30; the direct-VM late-interface control and all four serial
`ts_address_capture` controls passed, while the `crexx` driver, both
`ts_address_crexx` modes and 20 driver smokes aborted after functional output.
Every serial failure reported one to five live allocations at VM teardown.
The two `-nocompile` tests fail during their preparation invocation only
because that completed `crexx -noexec` process then reports the same one/two
live allocations; neither exposes a separate preparation or artifact defect.

The audit confirms that `prepare_redirect_thread_context()` copies a raw
destination/input `value *` and the parent VM's `memory_worker` into every I/O
thread context. Concurrent stdout and stderr readers consequently enter one
worker arena and append directly to receiver registers. Input writers likewise
read live receiver registers/attribute trees from a foreign thread. This
violates worker exclusivity on success and on several cleanup paths.

### EF-0 design selection

1. **A — per-I/O-thread VM worker plus a worker-owned capture value.** This can
   enforce allocator exclusivity, but a worker-owned `value` still cannot be
   handed to the receiver. It would need a second byte serialization step,
   worker registration/destruction and failure-atomic handoff before the
   temporary worker can be destroyed. That is valid future Gate E machinery
   but unnecessary for byte-stream redirect threads.
2. **B — receiver-reserved mailbox/completion with generation checks.** A
   private unpublished payload and publish/acquire/consume lifecycle fits Gate
   F, but reserving receiver storage or introducing a reusable queue now adds
   synchronization, generation and cancellation machinery that a single-shot
   redirect endpoint does not need. A foreign thread must not mutate the
   receiver reservation, so this also reduces to an independently owned byte
   payload for EF-0.
3. **C — single-shot private redirect completion with an independent capture
   domain. Selected.** Each endpoint owns one non-VM completion containing only
   its pipe handle, byte payload, mode, diagnostics and terminal state. Input
   is copied from string/array registers into an immutable completion-owned
   byte snapshot before thread creation. Output/error reader threads grow only
   completion-owned byte buffers. They never enter an RXVM worker and never
   receive a `value *`. Thread join supplies the release/acquire synchronization
   boundary; the receiver VM thread then consumes the published bytes exactly
   once and performs string append or line-array conversion through its own
   worker. Endpoint identity makes a generation counter unnecessary in this
   single-shot slice; the state machine still makes exactly one success/failure
   publication and one optional consume/discard explicit.

Design C is the smallest design that enforces ownership on every path. Its
private completion is intentionally shaped as the first spawn provider payload
for the later versioned Gate F envelope, but is not that public envelope. The
preferred Rexx-side evolution is a logical register image: one typed scalar or
binary payload plus zero or more ordered child-register images. Small messages
can materialize directly, while large binary content can use immutable chunks
or a bounded stream capability beneath the same register-centric surface. This
logical `ChannelValue` must never be a live internal RXVM `value`; every
receiver materializes it into receiver-owned registers. The final encoding,
stream policy and public interface remain Gate F decisions.

The Level B ADDRESS request remains the orchestration layer. EF-0 may
re-engineer the existing `REDIR2STR`, `REDIR2ARR`, `STR2REDIR`, `ARR2REDIR` and
`SPAWN` native boundary; the selected first implementation does not need a new
public RXAS/RXBIN instruction. Adrian has explicitly allowed a later narrower
native-to-cREXX instruction if implementation evidence requires it. General
`chanstart`/`chanwait` instructions remain closed.

### EF-0 numbered implementation plan

1. Retain a focused failing Debug reproducer that concurrently captures stdout
   and stderr beyond one read chunk and beyond the 16 KiB standard-allocation
   ceiling, with repeated buffer growth.
2. Replace the copied `REDIRECT` thread context with a private single-owner
   completion/state object that contains no RXVM worker and no `value *`.
3. Snapshot string and array input into immutable completion-owned bytes before
   starting the writer thread, with overflow/allocation failure cleanup.
4. Capture stdout and stderr independently into completion-owned byte buffers;
   continue draining after capture allocation failure so the child cannot
   deadlock on a full pipe.
5. Centralize join, terminal publication and one-time receiver consume. Perform
   final string/array conversion only while the receiver worker is entered.
6. Give POSIX and Windows the same handle ownership, broken-pipe, launch/thread
   failure, partial-output, cancellation/cleanup and finalizer contract.
7. Exercise empty/string/array input and output, simultaneous stdin/stdout/
   stderr, non-zero exit, early exit/broken pipe, repeated cleanup, nested
   `ADDRESS CREXX`/driver execution and both concrete VMs in optimized and
   no-opt forms where applicable. There is no existing spawn allocation/thread
   failure injection hook; retain that case as an explicit gap unless a narrow
   deterministic hook is justified by the implementation.
8. Prove exact Debug teardown and the existing driver controls, review the
   ownership diff, then freeze implementation.
9. Build the ordinary profiling-off Release product, run the smallest decisive
   spawn/driver verdict plus a bounded ordinary-VM neutrality check, report the
   first Release verdict to Adrian and stop. Sanitizer, full Debug closeout,
   portable builds, evidence closeout and commit follow only after acceptance.

### EF-0 accepted implementation and closeout

Design C is implemented in `rxspawn.c`. Each endpoint owns a private
`REDIRECT_COMPLETION` allocated with libc. I/O threads receive only that
completion. The receiver-side `REDIRECT` retains the destination worker and
register, which no helper thread can access. Output readers use independent
4 KiB read chunks, retain partial output on terminal error and keep draining
after capture-allocation failure. Input string/array values are flattened
before pipe and thread creation. Join precedes every consume or discard; the
receiver verifies its active worker before performing string/array
materialization.

POSIX completion descriptors are close-on-exec and Windows uses private
non-inheritable duplicates. Nested `crexxcmd_run_argv()` now forwards the
parent input redirect rather than substituting `/dev/null`.

The retained focused fixture exercises simultaneous stdout/stderr capture with
24,480 bytes per stream, multiple capture-buffer growth operations and content
verification. It also covers empty output, string/line-array output,
string/array input, simultaneous stdin/stdout/stderr, non-zero status with
partial output, early child exit/broken pipe, twelve repeated cleanup cycles
and nested `ADDRESS CREXX`. CTest registers default, `rxbvm` and `rxtvm`
variants in optimized and no-opt modes where the concrete VM is available.
There is no production allocation/thread-failure injection selector; launch,
thread-start and allocation failure paths are audited structurally and share
the same centralized cleanup.

Adrian accepted the mandatory first ordinary-Release verdict on 2026-08-07:
the focused recovery set passed 15/15, the exact 22 handover failures plus the
new fixture passed 23/23, and the bounded ordinary-VM RexxCPS/register controls
passed 4/4. Post-acceptance closeout then passed focused Apple AddressSanitizer
34/34 with leak detection disabled, full Debug 1,996/1,996 at the required
30-way parallelism, and the combined ordinary Release set 38/38. The portable
`rxbvm`, `rxbvml`, `rxbvme` and selected `rxvm` targets build. Apple
LeakSanitizer reports that `detect_leaks` is unsupported on this platform;
Debug teardown counters retain exact live-allocation coverage. No MinGW cross
compiler is installed, so the shared Windows contract is implemented but its
native compile/run proof remains a platform limitation. No timing cell was
needed: this path has no retained spawn timing baseline, and ordinary VM
allocation/execution is untouched and passes the bounded Release controls.

Compact retained evidence:
[`2026-08-07-perf3-13-ef0-spawn-recovery`](evidence/2026-08-07-perf3-13-ef0-spawn-recovery/).

### EF-0 ownership contract

- [x] No spawn I/O thread enters the receiver VM worker or any other VM worker.
- [x] No thread completion contains or dereferences a live VM `value *`,
  register, reference cell or attribute tree.
- [x] Input is immutable and thread-owned before thread creation.
- [x] Output/error bytes are endpoint-owned until terminal publication and
  join; stdout and stderr share neither payload nor allocator state.
- [x] Receiver conversion occurs exactly once under the receiver worker, or the
  completion is deterministically discarded on an abandoned request.
- [x] Success, non-zero child exit, broken pipe, allocation/thread/launch
  failure, partial output, cleanup and finalizer paths release every completion
  allocation and handle.
- [x] Each endpoint publishes exactly one terminal success/failure state.
- [x] POSIX and Windows implement the same ownership/state contract; native
  Windows compile/run validation remains outstanding.
- [x] Ordinary single-worker RXVM allocation remains lock-free and unchanged.

## M5 — worker/thread execution ownership

EF-0 is accepted and published as the recovery subset above. Adrian approved
the following full M5 architecture and immediate E1 slice on 2026-08-07. E1
introduces ownership structure only: it creates no worker threads and changes
no VM, language, RXAS/RXBIN or public pool semantics.

### Gate E design selection

1. **A — fully independent VM context and module load per worker.** This is
   ownership-safe and is retained as the first concurrent correctness control,
   but duplicates bytecode, link metadata and execution images. It is not the
   selected final sharing model.
2. **B — sealed shared runtime/program plus worker-local mutable state.
   Selected.** A runtime owns the synchronized whole-slab depot, worker
   registry, process signal broker, immutable plugin-factory/library catalogue
   and sealed program generations. Each worker owns one VM context, allocator
   arena, registers/stacks, module globals, procedure runtimes/frame recyclers,
   dynamic caches, references, plugin instances, sockets, RXVML/RXPA state,
   callbacks, scratch state and logical CREXX directory/environment. Program
   bytes, constants and metadata become shareable only after an explicit
   immutability audit and publication as a sealed generation.
3. **C — share the current mutable VM context behind locks or TLS overlays.**
   Rejected. Module globals, frame free lists, dispatch caches, native/plugin
   state, references and late-load state would either race or serialize on a
   coarse lock. Field-by-field TLS would obscure rather than enforce
   ownership.
4. **Process-per-worker.** Reserved for failure-prone or untrusted work. Gate E
   thread workers are trusted in-process engines; later warm process pools use
   the same Gate F protocol without weakening thread-worker ownership.

The current `rxvm_context` becomes the internal worker VM and initially remains
the compatibility shell used by the CLI and RXVML. A new runtime domain owns
the memory context/depot. Ordinary allocation remains worker-local and
lock-free; shared synchronization is limited to registration, program/plugin
catalogues, signal routing and whole-slab depot exchange.

Live VM `value`, reference cells, executing registers and mutable native
payloads never cross workers. Freeing is owner-only by default. A bounded
remote-free queue remains a required measured comparison but is not selected
unless an unavoidable provider/native lifecycle proves that it is needed.
Only an owning worker may return empty whole slabs at quiescent trim or
teardown.

Late loading publishes a new sealed program generation rather than mutating a
generation being executed. Existing requests retain their old generation until
completion. The product main VM is the OS interrupt target and every worker
observes only its own pending word at safe points. A later Gate F provider may
route or propagate an event by raising selected worker words; the low-level
POSIX or Windows callback never walks the worker registry. Targeted
cancellation is a cooperative worker request, not forced thread termination.

The future transport value remains a logical register image: a typed scalar or
binary payload with ordered child-register images. Gate E may use a private
copy-only subset in its concurrency harness, but no internal `value *`, public
pool/channel API or RXAS/RXBIN instruction is introduced before Gate F.

### Gate E numbered implementation plan

1. **E1 — single-worker ownership shell.** Move the memory context/depot into
   an internal runtime object, retain one allocator worker in each
   `rxvm_context`, add explicit worker lifecycle and wrong-thread ownership
   checks, and preserve the current one-worker API and execution path.
2. Freeze E1 after the smallest focused Debug load/run/unload, RXVML/ADDRESS
   and both-concrete-VM correctness set. Build ordinary profiling-off Release,
   run the smallest retained single-worker neutrality verdict, report it to
   Adrian and stop before broad closeout.
3. **E2 — explicit active state.** Move RXVML/RXPA active contexts and temporary
   pools, SAY routing, CREXX directory/environment, reference fallback state
   and interrupt delivery from process globals to checked runtime/worker
   ownership while preserving nested same-worker calls.
4. **E3 — plugin/native lifecycle.** Separate immutable factory/library
   catalogues from per-worker plugin instances and classify every bundled
   native payload as immutable, worker-affine, completion-based or not
   worker-safe.
5. **E4 — sealed program generations.** Retain independent module loads as the
   correctness control, then share only audited immutable RXBIN/constants/type
   metadata. Keep globals, procedure runtimes, frame recyclers and caches in
   worker overlays; add generation-safe late loading.
6. **E5 — private concurrency proof.** Add persistent trusted worker threads,
   fixed worker/session affinity, cooperative cancellation and a private
   copy-only logical-register request/completion harness. Expose no public pool
   API.
7. **E6 — reclamation and scale selection.** Compare owner-only freeing with a
   bounded remote-free prototype and whole-slab handback. Measure single-worker
   neutrality, 1/2/4/8-worker throughput, contention, peak/retained RSS,
   cancellation, failure isolation and deterministic shutdown.
8. Complete accepted-slice sanitizer/lifecycle checks, full Debug CTest with
   repository parallelism, Release and portable builds, then Mac, Intel Linux,
   Linux ARM64 and same-machine Windows evidence. Timed work requires Adrian to
   clear and reserve the host. Stop for worker-model selection before public
   pool or Gate F semantics.

### E1 accepted implementation and closeout

E1 introduces `rxvm_runtime` as the owner of the allocator memory context and
whole-slab depot. The compatibility `rxvm_context` embeds one
thread-affine `rxvm_worker`, which owns its allocator arena, owner-thread token,
depth-counted execution state and deterministic lifecycle. `run()` refuses a
foreign thread, nested same-owner RXVML calls remain valid, teardown requires
an idle owner to enter `draining`, and the runtime cannot be destroyed while a
worker remains registered. Allocator entry and worker destruction independently
enforce the same thread ownership. No new thread, public API, register transfer,
channel or RXAS/RXBIN semantic is introduced, and ordinary local allocation
remains lock-free.

The focused cross-thread lifecycle test has POSIX and Windows implementations.
It proves that a foreign OS thread owns neither the VM worker nor its allocator,
cannot begin execution and cannot perturb the idle state. The owning thread
proves nested execution, drain, exactly one stopped terminal state, worker
unregistration and zero live allocations.

Adrian cleared the Mac and accepted the mandatory first ordinary-Release
verdict on 2026-08-07. One warmup and 12 pairwise-balanced serial rounds across
Sieve, Richards, Towers and RexxCPS passed all 104 processes. The 48-pair pooled
mean is -0.149146% with a 95% interval of -0.780096% to +0.481804%; no 3% guard
fires. Individual retained observations are Sieve -0.654505% clear adverse,
Richards +0.924804% inconclusive, Towers +1.146596% clear favorable and RexxCPS
-2.013479% clear adverse. The candidate `rxbvm` grows by 1,440 bytes (0.144%).

Post-acceptance closeout passes focused Apple AddressSanitizer 3/3, full Debug
1,997/1,997 with `--parallel 30` in 319.75 seconds, 808 final ordinary Release
build steps, and combined Release 14/14. A final compiled-source Debug control
also passes 13/13 after an indentation-only closeout correction. Both concrete VM
dispatch contracts and the static RXVML archive link pass. Apple LeakSanitizer
does not support leak detection; Debug teardown retains exact live-allocation
assertions. The broad ASan linked-artifact build separately exposes a
pre-existing RXAS heap-use-after-free at `rxas_flow_proof.c:4413` while
generating `nr15_stem_semantics_rxvm_opt`; E1 changes no assembler source, and
the finding is retained rather than hidden or expanded into this slice. Native
Windows compile/run proof is outstanding because no Windows cross-toolchain is
installed.

Compact retained evidence:
[`2026-08-07-perf3-13-gate-e-e1-worker-shell`](evidence/2026-08-07-perf3-13-gate-e-e1-worker-shell/).

### E1-P1 flattened-core layout stabilisation

Adrian approved this bounded follow-up on 2026-08-07 after the E1 closeout.
The retained first verdict's clear RexxCPS loss was isolated against
`19802842e`, which already contains the published EF-0 spawn recovery. EF-0 is
therefore not the cause of the E1 comparison.

Machine-code inspection found that placing worker lifecycle entry, diagnostics
and exit directly inside the 500+ KiB flattened `run()` body changed its stack
frame from `0x6d0` to `0x6e0`, changed argument register allocation and changed
the body from 533,440 to 528,100 bytes. The enlarged E1 `rxvm_context` also
moves later fields by 32 bytes, but a private counterfactual retaining that
layout while moving lifecycle handling to a small wrapper restored the
flattened core's original stack/register pattern.

Across two same-artifact, pairwise-balanced canonical RexxCPS blocks totalling
60 recorded pairs, committed E1 is clear adverse against the post-EF-0 control
at -0.780187% (95% interval -1.425203% to -0.135171%). The wrapper is neutral
against the control at -0.153656% (-0.723528% to +0.416215%) and clear favorable
against committed E1 at +0.657320% (+0.154892% to +1.159749%). This selects
the wrapper and rejects context-layout redesign for this slice.

Numbered implementation plan:

1. Rename the current flattened body to a private noinline/no-clone core and
   keep its allocator enter/leave and complete VM semantics unchanged.
2. Make public `run()` the small lifecycle wrapper: begin ownership, invoke the
   flattened core, end ownership. Nested same-owner calls continue to enter the
   wrapper and use the existing depth count.
3. Run the focused Debug worker/allocator, nested RXVML, late-interface,
   ADDRESS CREXX and both-concrete-VM controls; then freeze implementation.
4. Build the ordinary profiling-off Release product and run a pairwise-balanced
   core-four comparison against exact post-EF-0 control `19802842e`, retaining
   RexxCPS as the target and Sieve/Richards/Towers as layout guards.
5. Report the first Release verdict and stop for Adrian. Broad closeout,
   sanitizer refresh, evidence packaging and the separate E1-P1 commit follow
   only after acceptance. E2 and Gate F remain closed.

Adrian accepted the first Release verdict and authorized full QA on
2026-08-07. One warmup and 20 pairwise-balanced serial rounds per workload
passed all 168 processes. All rows are neutral: Sieve -0.190379%, Richards
-0.286721%, Towers -0.059737%, RexxCPS -0.231775% and the pooled core four
-0.192153% (95% interval -0.632127% to +0.247821%). No 3% guard fires. The
candidate `rxbvm` is 1,264 bytes larger than the post-EF0 control and 176 bytes
smaller than committed E1.

Post-acceptance closeout passes the focused Apple AddressSanitizer worker,
allocator and reentrancy set 3/3, full Debug 1,997/1,997 with `--parallel 30`
in 210.77 seconds, and the ordinary profiling-off Release closeout 14/14.
Both concrete VM dispatch contracts, linked artifacts, nested RXVML, late
interface load, ADDRESS CREXX opt/no-opt and the static archive consumer pass.
Apple LeakSanitizer remains unsupported; exact Debug teardown assertions stay
enabled. Native Windows, Intel Linux and Linux ARM64 proof remains later Gate E
platform work. Compact evidence:
[`2026-08-07-perf3-13-gate-e-e1-p1-wrapper`](evidence/2026-08-07-perf3-13-gate-e-e1-p1-wrapper/).

### E2 explicit-active-state design selection

The opening audit identifies seven mutable process-global execution paths:
`rxvml_active_context`, `current_rxpa_context`, the RXPA copy-out pool, the
SAY exit, CREXX directory-stack and process directory/environment mutation,
the context-free reference-ID counter, and the VM `interrupts` bitset. The
RXVM plugin factory catalogue and its dynamic-library handle remain E3
catalogue/lifecycle work; module generations remain E4.

1. **A — worker-VM-owned active state with a checked thread-local locator.
   Selected.** Append an internal active-state record to `rxvm_context` so
   established context-member offsets remain stable. It owns active RXVML/RXPA
   bindings, native copy-out scratch, SAY routing, logical CREXX command state
   and the worker's live pending-interrupt pointer. A nested enter/leave locator
   identifies the currently executing or loading worker VM; the mutable payload
   remains context-owned and teardown checked. Nested same-worker calls save and
   restore their active bindings. Preserving context offsets does not promise
   identical compiler-generated stack/register layout inside the flattened
   interpreter core; E2's accepted RexxCPS result is evidence of that distinction.
2. **B — make each old global thread-local. Rejected.** TLS would prevent two
   OS threads from overwriting one another, but would not bind state to a
   worker lifecycle, distinguish nested workers on one thread, prove cleanup,
   or provide a valid target when a Windows console handler runs on a system
   thread.
3. **C — retain process globals behind a mutex. Rejected.** It would serialize
   unrelated workers, leave callbacks dependent on ambient state and make a
   lock, rather than ownership, the correctness boundary.

CREXX commands use a worker-owned logical current directory, directory stack
and environment override/tombstone map. File commands resolve relative paths
against that logical directory. Child launch receives immutable working-
directory/environment inputs and applies them in the child/CreateProcess call;
the parent process CWD and environment are never temporarily changed. A
save/apply/restore process-global lease was rejected because unrelated native
threads could observe the temporary state even if CREXX commands themselves
were serialized.

cREXX remains UTF-8 internally on every platform. Windows command line,
working-directory and environment snapshots cross the OS boundary through
UTF-8/UTF-16 conversion and Win32 `W` APIs; E2 does not change the process
console code page or locale. Child standard streams remain explicit byte
payloads, with provider-selected conversion when the child encoding is not
UTF-8. The remaining older narrow Windows filesystem built-ins require a
bounded `W`-API adapter audit during E2 portable closeout; this does not alter
the first Mac Release verdict.

The initial E2 implementation retained a separate process-global asynchronous
signal mask and made every dispatch poll both that mask and the worker-local
pending mask. Its first Release verdict was materially adverse: Sieve
`-9.736158%`, RexxCPS `-1.771985%`, and the 48-pair pooled mean `-3.293694%`;
all 104 processes passed, so correctness did not explain the loss. Adrian
rejected the resulting double read on 2026-08-07 and approved the following
bounded counterfactual before broader QA.

Each VM context has exactly one pending-interrupt word and dispatch reads only
that word. The standalone product's main VM context is the sole OS-addressable
target; the low-level POSIX or Windows callback only maps the event and sets a
bit in that word. Other VM contexts retain their own worker-local word and do
not independently consume a second process queue. Internal VM signals always
target the current active context; a context-free native raise targets the
designated product main context. Later Gate F communication may deliberately
propagate a Ctrl-C or another event by raising each selected worker's own word,
but neither worker registration/fan-out nor cancellation policy belongs in E2.
The raw OS callback must never walk a worker registry.

The frozen single-word counterfactual passes 39/39 focused Debug tests and all
52 Release timing processes. Across 12 balanced pairs, RexxCPS is neutral at
`+0.052501%` (95% interval `-0.635793%` to `+0.740794%`), proving that the
double-read regression was real. Sieve improves from `-9.736158%` to
`-5.311186%` but remains clearly adverse (95% interval `-5.779917%` to
`-4.842455%`) and still hits the 3% guard. The candidate product is 1,020,616
bytes versus 1,002,392 bytes (`+1.818051%`), while `__text` grows by 10,188
bytes. Apple-Clang code generation retains the context-word address in a stack
slot: representative dispatches load that pointer and then load the flag,
where the published global control uses `adrp` address generation followed by
one flag load. This is one logical interrupt word but still one extra data
load at many dispatch sites. No broader QA is authorized until Adrian selects
the disposition or a further bounded addressing counterfactual.

Adrian approved that bounded counterfactual on 2026-08-07. While a VM is
running, its sole pending-interrupt word is an execution-local direct slot and
the active context publishes that slot's address to native raisers. The
designated product main publishes the same address to the OS callback only for
the slot's live execution interval. Nested same-worker execution transfers the
pending bits into the nested slot and restores them to the suspended slot on
return. Dispatch reads the direct local word; there is no context word, shadow
mask, process queue or fan-out. Handler installation/removal must bracket the
published main-slot lifetime before it can be cleared.

The frozen direct-slot verdict passes the same 39/39 focused Debug tests and
all 52 Release timing processes. Sieve is restored to neutral at `-0.012799%`
(95% interval `-0.332055%` to `+0.306456%`, 8/12 favorable) with no guard hit.
RexxCPS is clearly adverse at `-1.206404%` (95% interval `-1.760487%` to
`-0.652320%`, 1/12 favorable) but remains inside the 3% guard. The 24-pair
two-workload mean is `-0.609602%` (95% interval `-1.000309%` to `-0.218894%`)
with no guard hit. Apple Clang emits one direct stack-slot load at dispatch,
and candidate `__text` is 5,556 bytes above the published control rather than
10,188 bytes above it. The candidate product is 1,020,712 bytes versus
1,002,392 (`+1.827628%`). Adrian accepted this first Release verdict and the
bounded RexxCPS loss on 2026-08-08; no additional performance timing was
required before closeout. The queued `PERF3-05-R1`/`PERF3-05-R2` work owns the
broader flattened-`run()` hot/cold and profile-selected handler layout cleanup.

Context-backed reference cells retain their existing worker-VM reference
context. The context-free compatibility helpers no longer use mutable global
ID state; their unindexed cell identity is local to the live compatibility
cell. No such cell may cross a worker boundary.

### E2 numbered implementation plan

1. Add the appended active-state record plus checked nested enter/leave and
   current-context accessors. Prove owner rejection, nested restoration and
   deterministic empty teardown without changing `rxvm_worker` or earlier
   `rxvm_context` field offsets.
2. Move RXVML callback selection, RXPA loader selection and RXPA copy-out
   scratch into that state. Cover nested RXVML callbacks, dynamic and static
   plugin loading, allocation failure and cleanup.
3. Make SAY routing context-owned for explicit RXVML users and active RXPA
   plugins. Preserve the legacy no-context setter only as a thread-local
   compatibility default; it is not shared VM execution state.
4. Replace the CREXX process-global directory stack and direct parent
   CWD/environment mutation with worker-local logical state. Resolve all
   built-in file paths against the logical directory and pass a copied
   directory/environment view to child launch on POSIX and Windows.
5. Remove the mutable fallback reference counter. Give each live VM execution
   one direct pending-interrupt word, designate the standalone product main
   context as the sole OS-event target, and keep handler registration
   process-scoped without adding a second event mask or per-dispatch read.
6. Retain focused concurrent reproducers for RXVML/SAY/RXPA isolation, logical
   CREXX directory/environment isolation, interrupt isolation, nested calls,
   both concrete VMs and exact teardown.
7. After the smallest focused Debug set passes, freeze production code, build
   ordinary profiling-off Release, compare the retained core four against the
   published E1-P1 product and report the first verdict to Adrian. Broad QA,
   sanitizer and portable closeout wait for acceptance.

### E2 accepted closeout

Adrian accepted the direct-slot verdict and authorized full QA on 2026-08-08.
The implementation remains frozen. The final ordinary profiling-off Release
product passes 49/49 focused lifecycle, loader, RXVML, RXPA, ADDRESS, both-VM,
external-consumer and crexx-driver checks. Its `rxbvm` is byte-identical to the
accepted first-verdict artifact: SHA-256
`132aa8a69a1ad9e250dfce8a4ac03905daade9f5e5300f27692c9f179255c841`,
1,020,712 bytes, with 824,628 bytes of `__text`.

Full Debug passes 1,999/1,999 with `--parallel 30` in 244.79 seconds. The
focused supported Apple AddressSanitizer E2 set passes 35/35 with its optimized
artifact fixture included, and the complete AddressSanitizer CTest passes
1,999/1,999 in 727.05 seconds with no excluded or skipped tests. Apple
LeakSanitizer is not supported, so `detect_leaks=0` is a platform capability
limit rather than a test exclusion; Debug's exact live-allocation teardown
assertions remain enabled.

The full optimized sanitizer build exposed an independent RXAS proof-snapshot
use-after-free when the sparse semantic edit array grew beyond 16 inline
records. The repair retains inline snapshots and stable record IDs; array
growth causes proof pins to be resolved again before the next proof query. It
adds no per-record allocation. A balanced same-input Debug comparison of the
discarded malloc control and selected record-ID form was 4.25/4.24 seconds
versus 4.25/4.25 seconds across two ten-assembly blocks and produced
byte-identical RXBIN. The RXAS contract, optimized reproducer, focused ASan and
complete ASan/Debug suites all pass after the repair.

Native Windows-MinGW, MSVC, Intel Linux and Linux ARM64 execution proof for E2
remains the ordered portable follow-up after Adrian reviews the local commits.
Compact evidence:
[`2026-08-08-perf3-13-gate-e-e2-active-state`](evidence/2026-08-08-perf3-13-gate-e-e2-active-state/).

### E3 plugin catalogue and worker-owned native instances — selected

E2 removed ambient loader selection but deliberately left the process-global
RXVM plugin factory list, its `current_loading_handle`, the single factory-made
`plugin_info`, static RXPA registration queues and dynamic-library lifetime for
the next slice. The current decimal instance mutates `num_context` and private
provider state, so it cannot be shared by concurrently executing workers even
when only one thread enters each worker.

1. **A — immutable process catalogue plus worker-VM-owned instances. Selected.**
   Synchronize only descriptor publication and library-handle
   lifetime. Published factory descriptors are immutable; every VM context
   constructs and owns its mutable provider/native instances, and destroys
   them before releasing its catalogue-generation references. Execution never
   locks the catalogue. Static RXPA registrations become replayable immutable
   descriptors rather than a one-shot queue consumed by the first VM.
2. **B — independently load every library in every worker. Rejected as the
   primary model.** OS loaders may coalesce a dynamic library, static
   constructors run process-wide and existing factories may expose singleton
   state, so repeated `dlopen`/`LoadLibrary` alone does not prove isolation.
   An explicitly declared private provider may still use this mode later.
3. **C — retain shared plugin instances behind a mutex. Rejected.** Decimal
   numeric context, error state and mutable native payload would remain
   cross-worker state, and ordinary instruction execution would gain a hot
   synchronization dependency.

The live 2026-08-10 entry audit confirms two related but independently
reviewable ownership surfaces:

- `rxvmplugin_factories` and `current_loading_handle` are process globals; a
  catalogue entry stores the one live `factory()` result, and every `run()`
  points that shared decimal instance at the current frame's `num_context`.
  `rxvml_create()` registers another global decimal instance for every context.
- RXPA is a wider compatibility surface: static function/metadata lists are
  consumed and freed by the first `rxldmodp()` replay, successful dynamic-load
  handles are not returned to VM ownership, and dynamic plugins retain the
  copied helper table in DSO-static `_rxpa_context` state.

To keep each production decision bounded, E3 is split into separately approved
slices. Adrian accepted E3a and therefore selected architecture A for RXVM
providers. E3b applies the same catalogue/context ownership rule to RXPA, but
its treatment of legacy plugin-private process state remains a separate design
decision below.

### E3a — RXVM provider catalogue and worker-VM decimal instances — accepted; Mac closeout complete

1. Freeze the source inventory and add a minimal two-context reproducer that
   demonstrates the present shared-instance/`num_context` collision without
   changing product behaviour. Record factory order, duplicate-name priority,
   dynamic-load failure and teardown expectations for bundled and external
   RXVM providers.
2. Replace the mutable global list entry with an immutable process-catalogue
   descriptor published by an explicit load transaction. The descriptor owns
   name/type/capability metadata, the factory and a generation-counted library
   handle reference; publication and handle lifetime are synchronized, but VM
   execution never takes the catalogue lock. Remove ambient
   `current_loading_handle` from the registration path.
3. Append a provider-instance set to `rxvm_context` so established hot context
   offsets and the accepted profile-20 owner shape are not needlessly
   disturbed. Instantiate the selected decimal provider once per VM context
   after plugin selection and before its first run. Frames borrow only that
   context-owned instance. Prefer this bounded eager point over a per-operation
   lazy branch; retain a narrower lazy form only as a measured failure-recovery
   alternative if eager construction proves materially costly.
4. Destroy every provider instance while its worker is idle and its allocator
   is still active, then release the catalogue generation/handle reference.
   Duplicate or failing registration must roll back without publishing a
   partial descriptor, leaking an instance or closing code still reachable by
   another context.
5. Define the minimum rebuild-together internal capability declaration for
   immutable/reentrant provider code versus per-context mutable instances.
   Preserve the existing external/public ABI through an adapter if it cannot
   be changed compatibly; do not infer safety from plugin type or metadata.
6. Add POSIX and Windows tests with two simultaneously live VM contexts for
   distinct decimal instances, different DIGITS/FUZZ/FORM/STANDARD state,
   independent signal/error state, nested calls, duplicate/failing factories,
   reverse teardown, retained handles and exact zero-live-allocation shutdown.
   Run the ownership tests under both concrete VMs; creating a public worker
   pool remains out of scope.
7. After the minimum focused Debug set passes, freeze implementation and build
   ordinary profiling-off Release with the unchanged profile-20 policy. Run a
   same-session, paired/interleaved 12-round comparison of the exact
   `6d12cd921` control and candidate for Sieve, Richards, Towers and canonical
   RexxCPS under product `rxbvm`, with `rxtvm` as the concrete dispatch guard.
   Retain VM file/`__text`, `rxvm_run_owned_core`, handler-placement and
   lifecycle deltas. The new absolute baseline is context only; it is not used
   as an unmatched regression comparator.
8. Report the first Release verdict and stop for Adrian. Full Debug,
   sanitizer, cross-platform closeout, E3b, E4, public workers and Gate F remain
   closed until that verdict and slice are explicitly accepted.

### E3b — RXPA replay, lifetime and optional concurrency capabilities — P1 approved

Adrian directed the programme to move to E3b after accepting E3a on
2026-08-10. He approved a backward-compatible A/C model: every existing plugin
is safe through the A compatibility lane, while an audited plugin may opt into
C capabilities. He then selected process reentrancy as the first useful
capability and authorized documentation, implementation and testing through
the P1 first ordinary-Release verdict. Per-context session factories and
per-call flags follow as P2 after P1 is accepted.

#### Current RXPA ownership audit

1. Runtime static constructors call `rxvm_addfunc()` and the metadata callbacks
   without an active VM. They prepend borrowed string/function pointers to
   unsynchronized process lists. The first `rxldmodp()` builds one native
   module, consumes and frees every list node, so a later VM cannot replay the
   same statically linked plugin set. `rxvml_create()` happens to append its
   five internal ADDRESS bridge functions again for each context; that does not
   recover other constructor registrations and races under concurrent creates.
2. A dynamic `.rxplugin` load calls its public `_initfuncs(rxpa_initctxptr)`
   entry directly into a context-owned module builder. Registration is not a
   transaction: callbacks mutate the module as the initializer runs. A
   successful `LoadLibrary`/`dlopen` handle is neither returned nor attached to
   the VM, so callable and native-payload-operation pointers remain valid only
   because the current loader leaks the successful handle.
3. Existing dynamic plugins copy the helper table into DSO-static
   `_rxpa_context`. OS loaders may coalesce repeated loads, so running the
   initializer for every VM rewrites that one table and does not create
   per-context plugin statics. The helper function addresses themselves are
   process-immutable and E2 routes their mutable RXVML/RXPA/SAY/copy-out state
   through the active `rxvm_context`; plugin-private DSO statics remain shared
   and cannot be inferred reentrant.
4. Native procedure pointers are encoded in the synthetic native module and
   copied into `proc_runtime.start`; ordinary and signal calls reach them
   without an owner/lifetime/policy descriptor. Native payload `copy` and
   `finalize` callbacks are also raw code pointers and can run after the
   originating native call.
5. Teardown currently frees each module image before its module globals, then
   drains the remaining context-owned values, references and registries. An
   explicit DSO reference therefore cannot be released at `free_module()`; it
   must survive until every frame/global/native payload and callback reachable
   from that VM has been destroyed.
6. The compiler has its own static declaration lists and also uses the existing
   `load_plugin()` contract. The installed `CREXX::RXPA` SDK exposes the
   current header-only `_initfuncs` ABI to external C and C++ plugins. E3b must
   preserve those source and binary consumers rather than requiring a new
   initializer or context userdata argument.

#### E3b selected A/C contract

1. **A is the mandatory compatibility floor.** An unmodified plugin retains
   the installed `_initfuncs(rxpa_initctxptr)` ABI and existing source/binary
   behavior. Its procedures and native-payload callbacks use one process-wide
   recursive compatibility lane because the host cannot infer whether its
   private statics are safe. Registration and catalogue synchronization remain
   off the ordinary bytecode execution path.
2. **C is optional and explicitly asserted.** A versioned, optional query
   advertises immutable plugin-level capability flags. Old hosts ignore the
   extra symbol; a new host treats a missing, malformed or unknown declaration
   as legacy A. The P1 convenience declaration is the one-line
   `RXPA_PLUGIN_PROCESS_REENTRANT` macro.
3. **Process reentrancy means concurrent calls have defined behavior.** It does
   not mean side-effect-free: synchronized I/O, atomics and calls into external
   thread-safe services remain valid. The plugin author promises that every
   procedure covered by the declaration tolerates concurrent entry, including
   its process statics, library calls, error paths and teardown assumptions.
   P1 does not infer safety from function names, metadata or plugin type.
4. **P1 is deliberately plugin-granular.** Audited process-reentrant procedures
   bypass the legacy call lock. Unmarked procedures remain serialized. Native
   payload `copy`/`finalize` callbacks remain on the compatibility lane in P1;
   a later capability can relax that only with an explicit lifetime contract.
5. **P2 extends the same negotiation surface.** A session-aware plugin may
   expose a per-VM session factory/destructor and per-call flags. Legacy entry
   points use a documented default session so old callers remain valid. P2 is
   closed until P1's ordinary-Release verdict is accepted; P1 must not smuggle
   a session pointer into `ADDPROC` options or change `rxpa_libfunc`.
6. Repeated `dlopen`/`LoadLibrary` is not isolation: OS loaders may coalesce a
   DSO and `_rxpa_context` is DSO-static. E3b therefore makes static declarations
   replayable and DSO handle ownership explicit even when the plugin opts into
   process reentrancy.

#### E3b-P1 numbered implementation plan

1. Commit the exact accepted E3a closeout so its commit and byte-identical
   Release executables are the E3b control. Record current declaration order,
   duplicate priority, compiler behavior and installed C/C++ SDK behavior.
2. Add the optional versioned capability declaration and
   `RXPA_PLUGIN_PROCESS_REENTRANT` macro without changing `_initfuncs`,
   `rxpa_libfunc`, `ADDPROC` language options, RXAS or RXBIN. Static plugins are
   rebuild-together; dynamic plugins remain loadable by old hosts.
3. Replace destructive static registration consumption with synchronized,
   owned snapshot-and-replay. Exact repeated internal registration is
   idempotent, and each VM publishes a distinct native module only after a
   complete snapshot has been built.
4. Add a runtime-private loader result that returns successful dynamic handles
   and the validated optional capability. Keep `load_plugin()` for existing
   compiler/internal callers. Each VM owns its DSO references until all frames,
   globals, references, native payloads and modules that may reach plugin code
   have been destroyed.
5. Bind the selected capability to each native runtime procedure. Route legacy
   ordinary and signal calls through one process-wide recursive lock; route an
   explicitly process-reentrant procedure directly. Keep catalogue and loader
   locks off the call path and keep native payload callbacks serialized in P1.
6. Add two-context, two-OS-thread tests covering second-context static replay,
   distinct native runtime objects, overlapping process-reentrant calls,
   serialized legacy calls, context-correct RXPA helper state, dynamic handle
   lifetime, reverse teardown and conservative handling of absent/invalid
   capability declarations. Retain compiler and installed external C/C++ SDK
   coverage.
7. Document the opt-in contract with safe/unsafe examples and the migration
   rule: add the macro only after auditing all process statics and callees.
   Document legacy serialization, P1 payload behavior and the reserved P2
   session-factory/default-session extension.
8. After the minimum focused Debug set passes, freeze production code and build
   ordinary profiling-off Release with the unchanged profile-20 panel. Compare
   the committed E3a control and candidate in a balanced native-call kernel,
   Sieve and canonical RexxCPS under product `rxbvm`, with `rxtvm` as the guard.
   Report lock/bypass cost, startup/load/teardown, VM file/`__text` and the hot
   owner separately.
9. Report the first E3b-P1 ordinary-Release verdict and stop for Adrian. Broad
   Debug/sanitizer/cross-platform closeout, P2 sessions, E4, public workers/
   channels and Gate F remain closed until P1 is explicitly accepted.

#### E3b approval boundary

Approval selects the backward-compatible A/C model and authorizes P1 steps 1-9
through the first frozen Release verdict. Adrian explicitly authorized the E3a
checkpoint commit. P1 authorizes parallel calls only for plugins that make the
process-reentrant assertion; legacy procedures remain serialized. It does not
authorize P2 session factories, a mandatory new ABI, a push or Gate F work.

- [ ] Give each worker its own execution state, stack/register sets, frame
  caches, arena and procedure-affine free lists.
- [ ] Define module-global, reference-cell, native/plugin, signal and late-load
  ownership explicitly.
- [ ] Add synchronized depot block transfer and worker registration/teardown
  without putting ordinary allocations on a central lock or allocator thread.
- [ ] Apply the selected reclamation policy through owner-local reclaim,
  bounded remote-free handling and whole-block depot handback.
- [ ] Compare owner-only frees, bounded remote-free queues and block handback.
- [ ] Prove single-worker neutrality, then multi-worker scaling, contention,
  retained RSS, cancellation and shutdown.
- [ ] Stop for worker-model selection before public pool/channel semantics.

## M6 — thread/process/host transfer and programmable channels

EF-0 is accepted and locally complete for the private spawn
completion/transfer subset above. The full transport-neutral M6 programme
remains closed until the Gate E worker model has been selected.

- [ ] Define one versioned value/message envelope with copy, move, immutable
  transfer-buffer and serialization modes.
- [ ] Make bounded queues, backpressure, deadlines, cancellation, failure and
  exactly one terminal completion part of the common contract.
- [ ] Provide one logical channel interface over in-process workers, process
  pools and cross-host transports.
- [ ] Prototype the channel worker as a Rexx class/interface before RXAS.
- [ ] Exercise compute, file/socket/HTTP, timer and child-process providers.
- [ ] Only after protocol/ownership acceptance design any
  `chanstart`/`chanwait` RXAS/RXBIN surface with complete semantics.

## Approval gates

1. **Gate A — M0 start: approved and completed 2026-08-05.** The source and
   retained-evidence audit selects a hybrid typed-silo/power-of-two-byte worker
   heap. No timed run was needed.
2. **Gate B — M1: approved 2026-08-05; completed and accepted 2026-08-06.**
   The unchanged-`value` worker/slab allocator passed the corrected formal
   Release and bounded RSS verdicts and is the committed baseline for Gate C.
3. **Gate C — M2/M3: approved 2026-08-06; completed and accepted 2026-08-07.**
   L32S remains the frozen 192-byte control. The maximise/backoff panel selects
   a 176-byte direct
   decimal pointer plus co-allocated raw-`size_t` header. Its combined
   core-four result is +0.847% and peak live capacity falls in all six cells.
   The 160-byte decimal/object form is guard-clean but rejected for a 25%
   Towers allocator-call increase, a Towers reversal and greater code/lifecycle
   complexity. Allocation-size compression, native/object sidecars and
   separate decimal descriptors are closed. `rxtvm` and automatic reclamation
   remain closed.
4. **Gate D — M4: approved 2026-08-07; first Release verdict accepted; Mac
   local closeout complete.** The clean 176-byte L32SDH product passes final
   focused Debug/Release, affected ASan and isolated install/product checks.
   Two post-correction blocks pass 208/208 executions and combine to +1.205%
   across 96 core-four pairs (95% interval +0.748% to +1.663%), with no guard
   hit. Cross-platform rebuild-together ABI validation remains before global
   Gate D closure.
5. **Gate E/F EF-0 recovery: accepted and published 2026-08-07.** The minimum
   spawn I/O ownership and receiver-side transfer slice passes its accepted
   first Release verdict, focused sanitizer, full Debug and Release closeout.
   It is published in `642e1b697` on the synchronized `19802842e` continuation
   base.
6. **Gate E — full M5 start: approved 2026-08-07; E1 through E3a accepted.**
   E1/E1-P1 are published and Windows-MinGW proven. E2's worker-owned active
   state and direct interrupt slot pass the accepted Release verdict, complete
   Mac Debug/ASan/Release closeout and the separately repaired RXAS sanitizer
   fault. The current profile-20 absolute baseline is retained. E3a RXVM
   provider/decimal ownership has an accepted neutral, guard-clean verdict and
   passes its 2,007/2,007 Mac Debug closeout. Adrian directed the programme to
   E3b; its replay/lifetime and serialized legacy-native design now awaits
   approval, while implementation remains closed. The full gate stops for
   worker-model selection before any public pool/channel semantics.
7. **Gate F — full M6 start: closed.** After Gate E selection, implement
   transport-neutral channels and only later consider public RXAS exposure.

Approval of one gate does not approve the next.
