# cREXX performance roadmap

Last updated: 2026-07-24

Status: live planning register for the second performance programme.
Production work requires the explicit decision gates recorded in each activity;
PERF2-02 contains the first accepted production slice.

The initial sweep is closed and preserved, without rewriting its accepted or
rejected history, in
[`ROADMAP-INITIAL-SWEEP-2026-07-23.md`](ROADMAP-INITIAL-SWEEP-2026-07-23.md).
The original dated charter remains
[`performance-programme-report-2026-07-15.md`](../docs/planning/release-1/performance-programme-report-2026-07-15.md).
This file is now the live control plane.

Status values are `queued`, `in progress`, `decision required`, `blocked`,
`deferred`, `complete`, `rejected` and `superseded`. `Complete` means the
stated exit gate and retained evidence both exist; it never means merely that a
prototype ran.

## North star

cREXX should become a materially faster Rexx implementation than ooRexx while
preserving the language, portability, debugging, linking and embeddability
advantages of the current architecture.

The programme has an intermediate threshold and a final outcome:

1. **Parity checkpoint, not completion:** on a clean exact commit and one
   governed same-host session,
   the selected default cREXX VM must be clearly faster than ooRexx on every
   qualified equal-work common Tier A workload, and the governed disclosed
   cREXX RexxCPS 2.2d diagnostic must beat same-session canonical Classic
   ooRexx RexxCPS. “Clearly” means the median is above parity and the governed
   sampling/interval disposition is favorable rather than noisy or
   inconclusive. There must be zero correctness failures and no unresolved
   regression guard.
2. **Unquestionable-superiority exit:** the selected default VM must reach at
   least 1.50x ooRexx median throughput on every qualified comparable cell and
   at least 2.00x on the common-workload geometric mean. The separately
   governed RexxCPS diagnostic must also reach at least 1.50x ooRexx. The
   alternate/non-default VM must itself be clearly faster than ooRexx on every
   qualified cell, not merely remain within a cREXX regression budget.

The numerical bands are the roadmap's working definition of “unquestionable”:
large enough that normal noise, one favorable workload or a marginal aggregate
cannot reverse the conclusion. They can be raised by Adrian, but ordinary
parity cannot be substituted for programme completion.

The product/architecture conclusion must be equally clear. The winning results
must survive the supported host/compiler matrix, retain both VM modes and the
portable canonical RXBIN, and demonstrate that new Rexx semantics can be
placed at compile, RXAS, link/load or guarded-runtime specialization without a
benchmark shortcut or a mandatory public-ISA fork. ooRexx is the comparator,
not the performance ceiling; cREXX should be the no-brainer foundation for
future Rexx enhancement work.

These thresholds apply only to semantically qualified cells. They cannot be
met by dropping a hard benchmark, changing its timed work, exploiting its
correctness check or treating a disclosed adaptation as equivalent. Every Tier
A row must finish either as a governed common comparison or with a named
capability/equivalence activity that explains what must change before it can
join the score.

Performance is not the only release dimension. Correctness, lifecycle, peak
RSS, installed/native use, artifact size, applicable RXBIN/ABI compatibility
and feature-gating, TRACE/source identity and both VM modes remain separate
gates under
[`PERFORMANCE-GOVERNANCE.md`](PERFORMANCE-GOVERNANCE.md) and
[`AGENTS.md`](AGENTS.md).

## Strategic conclusion

The cross-runtime RexxCPS review does **not** reduce to “hoist variables.”
Regina, ooRexx and NetRexx repeatedly apply a broader rule:

> Resolve, prove or compile a semantic decision at the earliest phase where it
> is stable, retain the result close to the execution representation, and keep
> a complete dynamic fallback.

cREXX already gives typed locals direct register/index placement. The larger
remaining opportunity is to hoist **semantic work**: call and BIF identity,
validation, parse/scan plans, type and representation decisions, loop
invariants, frame setup, conversions and repeated sequence selection.

The ownership rule for the new programme is:

| Facts become stable at | Preferred owner | Typical result |
| --- | --- | --- |
| Compile time | `rxc`, typed flow analysis and inlining | direct binding, invariant motion, result placement, dead scaffold removal |
| RXAS assembly time | RXAS whole-procedure effects/flow | destination forwarding, machine-level cleanup, coherent semantic instruction selection |
| Link time | `rxlink` | provider/member/procedure identity and immutable graph facts |
| Load/preparation time | private process execution image | decoded operands, runtime pointers, eagerly prepared process facts |
| First or repeated execution | guarded quickening/site cache | type- or target-stable private form with invalidation |
| Unstable or exceptional path | canonical fallback | late load, mutation, TRACE/debug, signals, unusual types and full semantics |

The earliest safe, fastest end-to-end placement wins. Runtime quickening is a
priority because the process-local execution image and recent semantic
fast-path work make it timely, not because runtime specialization should absorb
facts already provable by the compiler or RXAS.

Three further boundaries are fixed:

- Core Level B BIF work is **inlining first**. A small number of general RXAS
  or VM assists may support the irreducible semantic kernels; blanket native
  conversion is not the plan.
- More-than-three-operand RXAS support is already complete across assembler,
  RXBIN, linker, disassembler, metadata, compiler and both VMs. Width is an
  available design tool, not evidence that a wide instruction is profitable.
- Opcode operand width and procedure-call arity remain separate questions.
  `CALL1` through `CALL4` are already complete and historical evidence placed
  90.997% of calls at arity 0–4; PERF2-01 must refresh the residual census
  before any `CALL5+` or higher-arity frame work is proposed.
- VM work is first-class. Dispatch, execution-image layout, frames, values,
  conversions, interrupt state, code layout and lifecycle all receive current
  measurement rather than being treated as a residual implementation detail.

The detailed competitor evidence is retained in
[`rexxcps-runtime-source-review-2026-07-22.md`](rexxcps-runtime-source-review-2026-07-22.md).
Its mechanism findings remain useful; its pre-NR-15/16/17 gap sizes and
priority order are historical.

## Initial-sweep closeout

The closed register contains 29 activity rows plus an architecture gate:

| Closing disposition | Count | Activities |
| --- | ---: | --- |
| Complete | 21 | NR-01 through NR-06, NUMERIC-01, NR-08 through NR-11, NR-13 through NR-18, NR-21, NR-26 and NR-27 |
| Rejected | 1 | NR-07 |
| Deferred | 1 | NR-12 |
| Queued | 6 | NR-19, NR-20, NR-22, NR-23, NR-24 and NR-25 |
| Unstarted gate | 1 | architecture selection |

This closes the **initial sweep**, not the underlying open questions. Their
transfer is explicit:

| Initial item | Successor |
| --- | --- |
| NR-12 by-value/return cleanup | PERF2-03 flow-aware inlining and result/copy cleanup |
| NR-19 LTO/PGO/code layout | PERF2-10 toolchain, layout and lifecycle |
| NR-20 and NR-25 allocation/value ideas | PERF2-07 value, frame, representation and allocation work |
| NR-22 compact execution stream | PERF2-06 VM execution-engine programme |
| NR-23 quickening | PERF2-02 semantic quickening priority programme |
| NR-24 selected fusion | PERF2-02, PERF2-05 and PERF2-06 placement comparison |
| old architecture-selection footer | PERF2-11 explicit cross-platform architecture gates |

Ideas already proved complete, rejected, superseded or subsumed remain in the
archive. Deferred `FDIVSUB`, compact TRACE-correct `ILOADSETUNLINKN`, broader
copy/propagation and metadata ideas may enter a new panel only if current
profiles give them a mechanism footprint; they are not silently re-queued.

This roadmap was written against clean `develop` at
`d5b25a78fd6cd2b5b5962b45e508f3cb2bb782e6`. PERF2-01 must freeze and record
its own exact execution baseline and upstream state rather than inheriting that
snapshot by assumption.

## Current orientation, not a new baseline

The 2026-07-23 cREXX absolute checkpoint is the newest product result. Its
competitor columns reuse the governed 2026-07-20 session, so the ratios below
are planning orientation only and must not be published as a same-session
comparison.

| Workload | Qualification | `rxvm / ooRexx` | `rxbvm / ooRexx` | Approximate gain needed to pass ooRexx | Programme disposition |
| --- | --- | ---: | ---: | ---: | --- |
| Sieve | common | 7.12x | 5.39x | already ahead | Guard the win; use as a regression control. |
| Permute | common | 2.13x | 1.99x | already ahead | Guard the win; diagnose only if a shared mechanism is selected. |
| Bounce | common | 0.319x | 0.307x | 3.13x / 3.26x | Major object, call/frame and value-path target. |
| Richards | common | 0.143x | 0.141x | 7.00x / 7.07x | Largest valid deficit; state-machine and small-operation dossier first. |
| Base64 | common | 0.712x | 0.793x | 1.40x / 1.26x | Close byte/string access, conversion and loop costs. |
| RexxCPS | governed disclosed diagnostic, not common | 0.704x | 0.654x | 1.42x / 1.53x | Visible target for BIF, inline, stable-site and representation work. |
| Mandelbrot | not common | — | — | ooRexx checksum invalid | Repair/replace the ooRexx port before a claim. |
| Towers | not common | — | — | representation mismatch | Restore equivalent object/allocation work. |
| Storage | not common | — | — | cREXX container mismatch | Resolve CAP-02 ownership/container capability first. |
| List | disclosed adaptation | — | — | ownership adaptation | Re-audit equivalence; preserve the current speed result. |
| JSON | API diagnostic | — | — | representation/API mismatch | Resolve CAP-01 common parse/result contract first. |

The cross-session five-common geomean is 0.8683 (`rxvm`) and 0.8197 (`rxbvm`)
versus ooRexx. The current cREXX RexxCPS checkpoint is 28.120M/26.119M clauses
per second versus historical ooRexx at 39.921M. These figures show that
RexxCPS is now a plausible bounded closure target while Bounce and Richards,
not RexxCPS alone, govern the “faster on every benchmark” objective.

Source:
[`2026-07-23 NR-16/NR-17 closeout`](evidence/2026-07-23-nr-16-17-closeout/README.md).

## Live activity register

| ID | Priority | Activity | Status | Dependency / next gate |
| --- | --- | --- | --- | --- |
| PERF2-00 | P0 | Close and archive the initial sweep | complete | Historical register retained; successor mappings recorded here. |
| PERF2-01 | P0 | Clean same-session baseline and complete current attribution refresh | complete | Gate A accepted 2026-07-23; refreshed selection baseline frozen. No production change authorized. |
| PERF2-02 | P0 | Stable-site semantic quickening architecture and PoC panel | complete | Adrian accepted the favorable zero-state Q3b verdict on 2026-07-24. Broad Debug, Release, ASan, isolated-install and retained-RXBIN compatibility gates pass. |
| PERF2-03 | P0 | Flow-aware inlining 2.0 and post-inline cleanup | production in progress | H and slices 1-4 approved with QA and a commit after each; mandatory pause before slice 5. Slice 1 is favorable; slice 2 is byte-identical parity. |
| PERF2-04 | P0 | Inlining-first core Level B BIF campaign | queued | PERF2-01 + PERF2-03 cleanup ceiling; profile ranks BIFs. |
| PERF2-05 | P1 | Profile-selected RXAS semantic assists and instruction improvement | queued | PERF2-01 RXSEQ plus PERF2-03/04 machine ceilings. |
| PERF2-06 | P0/P1 | VM execution-image, dispatch, stream, call and lifecycle audit | queued | PERF2-02 rejects eager/core state for Bounce; its accepted exact canonical-handler Q3b slice remains the zero-state reference baseline. |
| PERF2-07 | P1 | Value/frame/copy/representation/allocation programme | queued | PERF2-02 assigns direct reference-helper work here/with PERF2-06; Richards removable receiver copy stays compiler-owned. |
| PERF2-08 | P1 | Benchmark capability/equivalence and Level B/G decision lane | queued | Re-audit CAP-01 through CAP-04; language decisions require Adrian. |
| PERF2-09 | P0 | Per-benchmark ooRexx closure campaign | queued | Begins as dossiers in PERF2-01; production slices come from PERF2-02 through 08. |
| PERF2-10 | P2 | LTO/PGO/code layout, build and lifecycle options | queued | Time-box after dominant semantic costs; select only cross-platform wins. |
| PERF2-11 | P1 | Cross-platform architecture selection and final scorecard | queued | Accepted slices plus Apple ARM64, Linux x86-64 and Windows evidence. |
| PERF2-12 | P3 | JIT/AOT/native-backend architecture decision | deferred | Revisit only if the non-JIT programme cannot meet the unquestionable-superiority exit. |

## Execution order

The programme is deliberately evidence-first but not analysis-only:

1. **Batch 0 — closeout:** preserve the initial sweep, establish this register
   and make no product change. This batch is complete.
2. **Batch 1 — current truth:** execute PERF2-01, accept the same-session gap
   ledger and freeze the first candidate panel. No production optimization is
   selected from July 20/23 cross-session orientation alone.
3. **Batch 2 — competing PoCs:** time-box quickening, inline cleanup/BIF and the
   highest-profile VM/value alternative. Compare placement at compiler, RXAS,
   load and runtime rather than assuming one layer.
4. **Batch 3 — production slices:** take one accepted mechanism at a time
   through the mandatory first profiling-off Release verdict. Stop after each
   verdict for Adrian's direction; do not bury a regression inside a broad
   batch.
5. **Batch 4 — benchmark closure:** rerun the full scorecard after accepted
   slices, select the next largest qualified deficit and guard every existing
   win.
6. **Batch 5 — architecture selection:** complete cross-platform VM and build
   evidence, decide the default execution architecture, and publish the final
   ooRexx comparison.

The dependency shape is:

```text
PERF2-01 current profiles and same-session comparisons
├── stable-site census ───────────────> PERF2-02 quickening
├── inline/call/BIF census ───────────> PERF2-03 ──> PERF2-04
├── RXSEQ/effects/native profiles ────> PERF2-05 and PERF2-06
├── copy/frame/conversion counters ───> PERF2-07
└── capability/equivalence ledger ───> PERF2-08

PERF2-02 through PERF2-08 accepted slices
└── PERF2-09 per-benchmark closure ──> PERF2-11 architecture/final scorecard
```

## PERF2-01 — current baseline and attribution refresh

### Question

After NR-14 through NR-27, what actually consumes time and machine work in
each benchmark at current HEAD, and what is the exact same-session gap to
ooRexx?

Older profiles remain valuable historical controls, but the accepted parse,
stem, TRACE/ADDRESS, direct-call and flow changes are large enough that they
cannot rank the next production work.

### Capture plan

1. Freeze a clean exact commit and exact compiler, library, RXBIN and VM hashes.
2. Build an ordinary profiling-off Release product for timing authority and a
   separate optimized profiling build. Record compiler flags and confirm that
   `CREXX_VM_PROFILING=OFF` is real, not a runtime-disabled instrumented build.
3. Run all 11 optimized Tier A steady-state workloads in both `rxvm` and
   `rxbvm`, serially, with the existing correctness, warmup, recorded-sample,
   rotation, append and noise rules. Retain lifecycle and RSS as separate
   results.
4. In the same session, rerun all five qualified common cells for cREXX,
   ooRexx and decimal NetRexx. Capture canonical Classic RexxCPS for ooRexx,
   Regina and NetRexx, and report cREXX's disclosed 2.2d adaptation separately.
   RexxCPS never enters the common aggregate. Retain qualified lifecycle, RSS
   and artifact-size lanes separately, and record runtime versions rather than
   inheriting the July 20 labels.
5. Capture optimized diagnostic profiles for all 11 workloads in both VM
   modes. Use no-opt only as attribution: all 11 if affordable, otherwise at
   minimum RexxCPS, Bounce, Richards, Base64 and one already-winning control.
6. Capture RXSEQ N=2/3/4 from exact images and module sets in both VM modes.
   Treat straight-line windows as candidate evidence, not as loop- or
   semantic-unit truth; calls and taken branches terminate current windows.
7. Run native system sampling/counters on the uninstrumented product: cycles,
   retired instructions, branches/misses, instruction-cache/iTLB evidence and
   sampled/annotated hot stacks where the host supports them. Cover the full
   portfolio once and repeat the largest gaps and noisy hotspots.
8. Produce one dossier per workload and one cross-workload mechanism census.

### Required telemetry

Schema 4 already provides opcode, transition, procedure, call, frame,
allocation and RXSEQ evidence. The refresh must also provide deterministic,
counts-first attribution for the missing domains below. Extend profiling only
where existing data cannot answer the question, and keep instrumentation edits
separate from product optimization.

| Domain | Required observation |
| --- | --- |
| Values | copy, typed copy, move, clear, reset and destroy counts by payload shape and bytes |
| Representations | string/numeric/decimal conversion, materialization, normalization and retained-cache hit/miss counts |
| Calls/frames | fresh/reused frame, local reset work, argument/result copies, interrupt-state inheritance and numeric-context synchronization |
| Sites | static site identity, observed types/targets, cache hits/misses, generation and invalidation |
| Control | branch taken/fallthrough, loop-backedge counts and exceptional exits |
| Strings/binary | scan/slice/append/access counts and bytes, including native temporary conversion buffers where observable |
| Loader | link/bind, execution-image copy/preparation, plugin initialization, first execution and teardown phases |
| Compiler artifacts | static instructions, operands/cells, RXAS/RXBIN bytes, locals/register ceiling, inline sites and rejection reasons |

A counts-only profile mode is preferred for full-portfolio census. Per-opcode
clock reads and profile elapsed time remain diagnostic; ordinary Release timing
is authoritative.

The added fields require a versioned profiling-schema revision (schema 5 or an
explicit equivalent), stable row/field definitions, per-domain
overflow/degraded status, backward handling of schema 4, updated evidence
summarizers and profiling documentation, and focused CTest coverage. Prove that
the ordinary `CREXX_VM_PROFILING=OFF` generated path remains compile-time
empty. Counts-only output never becomes product timing evidence.

For Bounce, Richards, Storage and any selected allocation outlier, complement
VM request counters with targeted system heap/allocation profiles: alloc/free
counts, retained and high-water bytes, size classes, reuse and allocator call
stacks. Keep system lifetime evidence distinct from VM allocation-request
counters and RSS.

### RexxCPS family controls

Retain cREXX 2.2d's disclosed 100 × 100 adaptation as the published cREXX
diagnostic score; ooRexx, Regina and NetRexx retain the canonical Classic
external workload where qualified. Add exact-hash cREXX diagnostic variants
that remove or replace one timed family at a time—BIFs, internal
calls/argument parsing, TRACE/ADDRESS, stems, decimal/string loops and PARSE—to
estimate attributable ceilings. These controls must preserve nominal
clause-accounting provenance and never replace either published form.

### Deliverables and exit

- a checksum-closed evidence bundle;
- an updated
  [`benchmark-median-summary.md`](evidence/benchmark-median-summary.md) that
  includes the current checkpoint and same-session external run;
- a per-benchmark gap ledger with cREXX/ooRexx ratio and gain-to-target;
- top procedure/opcode/transition/native-stack tables;
- call/frame, copy/conversion, allocation/RSS, BIF and site-stability tables;
- an explicit mechanism footprint and owner decision for every candidate that
  enters Batch 2; and
- no degraded/overflowed profile accepted without a named limitation.

PERF2-01 completes only when Adrian accepts the refreshed ledger as the
selection baseline. It does not itself authorize a production change.

Gate A was accepted by Adrian on 2026-07-23. The accepted selection baseline is
`performance/evidence/2026-07-23-perf2-01-current-baseline/`; that acceptance
does not authorize a PERF2-02 implementation or any other production change.

## PERF2-02 — stable-site semantic quickening

Started and completed its bounded design/PoC exit on 2026-07-23. Adrian
approved the exact Q3b production slice, then accepted its favorable mandatory
first Release verdict on 2026-07-24. Broad QA and closeout are complete. The
resumable control plane is
[`PERF2-02-WORKLIST.md`](PERF2-02-WORKLIST.md), and the pre-implementation
semantic/design comparison is
[`PERF2-02-ARCHITECTURE.md`](PERF2-02-ARCHITECTURE.md). No stateful quickener
was selected; the accepted direct reference path retains no learned site state,
public format change or invalidation lifecycle.

### Bounded PoC result

The final identical-guard control makes runtime site state unnecessary. The
zero-state canonical-handler Q3b reduces Bounce elapsed time by
80.261%/78.503% (`rxvm`/`rxbvm`), beats eager Q4 by 7.584% in `rxvm`, and is
tied with it in `rxbvm`. Q7 is tied on Bounce, neutral on Richards, adds
56,264/62,536 requested state bytes and retains lifecycle gaps. The one
PERF2-02 recommendation is **direct value/reference helper work belongs first
in PERF2-07/PERF2-06**. Richards' separate Q1 control reduces elapsed by about
24% and assigns its removable receiver capture to the compiler/inliner.

The smallest proposed slice is the exact A-LOCAL/A-ATTR guard in canonical
`MKREF_REG_REG`, with no persistent state or public change. Adrian approved
that slice on 2026-07-24. The first production verdict retains 12 `rxvm` pairs
and 22 `rxbvm` pairs after the required noise append: paired elapsed medians are
-80.596%/-78.464%, every pair is favorable and both mean 95% intervals are
wholly favorable. Full Debug, ordinary Release and supported macOS ASan CTest
each pass 1907/1907, and the isolated installed tree passes native compilation
plus retained pre-change RXBIN execution in both VMs.

### Priority and scope

Quickening is the first architecture priority after PERF2-01. Here it means a
guarded private execution form that remembers a semantic decision stable at a
particular site. It does not mean merely joining adjacent opcodes or replacing
computed-goto dispatch.

cREXX has unusually good substrate for this work:

- canonical RXBIN remains immutable and re-linkable;
- both VM modes already own a process-local execution-image copy;
- stable direct calls already bind process-local `proc_runtime *` operands;
- graph/member/provider generations and existing method/factory site caches
  supply relevant invalidation experience; and
- late-load refresh, source metadata, profiling and two execution modes already
  provide the boundary conditions a quickener must respect.

### Candidate selection

PERF2-01 must identify the sites. Candidate families, in likely evaluation
order, are:

1. residual stable BIF or small-helper sites that cannot be removed statically,
   selected only after the relevant PERF2-03/04 cleaned-inline ceiling;
2. generic type/conversion operations with a strongly stable observed shape;
3. dynamic selector, member, factory or call sites not already closed by direct
   binding or the existing site caches;
4. validation or prepared-plan objects whose process representation is cheaper
   than repeating the semantic setup; and
5. profile-selected semantic sequences whose decisive fact is not known to the
   compiler, assembler or linker.

Indexed local-variable access, direct static calls and already-frozen PARSE do
not become quickening work merely because competitors cache them; cREXX has
already moved those facts earlier.

### PoC panel

For each candidate semantic unit compare the same exact workload and fallback:

| Variant | Placement | Purpose |
| --- | --- | --- |
| Q0 | current canonical path | baseline and full semantic fallback |
| Q1 | compiler-owned result-only lowering | machine ceiling when compiler proof makes intermediate temporaries unobservable |
| Q2 | assembler-visible static RXAS lowering/rule | public authored-sequence control with every observable intermediate effect |
| Q3 | canonical runtime-only bytecode form | test a stable portable VM form without automatically exposing authored RXAS syntax |
| Q4 | eager load/preparation specialization | cost/benefit when process facts are known before execution |
| Q5 | lazy first-hit specialization | avoid preparing cold sites and measure first-hit cost |
| Q6 | guarded threshold/tiered specialization | test whether observed stability/hotness justifies mutation |

Private quickened forms should be prototyped before assigning canonical opcode
numbers. Placement decisions distinguish a private form, compiler-owned
result-only lowering, a canonical runtime-only bytecode form and an
assembler-visible public RXAS instruction. Authored RXAS fusion must preserve
observable intermediate effects; a compiler-owned result-only form may omit an
intermediate temporary only when compiler proof makes it unobservable. Public
RXAS is considered only when authored assembly benefits and the static form
beats private specialization without losing compatibility.

### Architectural requirements

- Quickened state belongs to the process-local image or an explicit
  process-owned side table, never serialized handler pointers or mutated
  canonical RXBIN.
- The guard must encode the smallest fact that proves the fast path: type,
  target, generation, representation or context. A miss executes the complete
  semantic fallback and updates, replaces or disables the site according to a
  documented state machine.
- Preparation and mutation must be safe for the actual VM/process concurrency
  model; do not assume a single writer without proof.
- Dequickening or refresh must cover late load/provider generation change,
  dynamic mutation, TRACE/debug/source stepping, signals/unwind, plugin/native
  boundaries and any semantic context used by the specialization.
- Cold `prepare_only`/`rxvm_prepare()` must not learn execution-only facts.
  Quickened state must be proved valid or reset across a later `run()`, repeated
  runs on one context, embedded/RXVML entry and changes of TRACE, debug or
  profiling mode. Cover prepare-only, re-entry and late-load fixtures in both
  VM modes.
- `rxvm` quickening must preserve the computed-goto label-owner invariant:
  `run()` currently owns the threaded labels through `RXVM_LABEL_OWNER`
  (`noinline`/`noclone`). Keep that single stable owner or prove an equivalent,
  and test prepared handler addresses under every supported compiler/configuration.
- A fused private form must preserve every canonical exception, retirement,
  interrupt-poll, TRACE/breakpoint and profiler boundary. If equivalence at an
  intermediate boundary cannot be proved, dequick/de-fuse before execution;
  fusion must not silently reduce delivery or observation points.
- Profiling and RXSEQ must retain canonical opcode/site identity while also
  exposing quickened state, hits, misses, replacements and invalidations.
- `rxvm` and `rxbvm` must implement the same semantics. Different private
  layouts are allowed only when the evidence explains the difference.
- Startup, load-first-result, RSS, private-image size and cold-site preparation
  are measured alongside steady-state throughput.

### Adoption and exit

A quickened candidate advances only when:

1. PERF2-01 shows a repeated semantic cost and site stability;
2. the variant beats the current path in an ordinary profiling-off Release
   comparison; it also beats the best safe static form when one exists, or
   documents why no static form can consume the runtime fact;
3. the complete-product result is clearly favorable on a target workload or
   the common aggregate, with no unexplained portfolio guard;
4. invalidation, fallback, TRACE/source, signal, late-load and dual-VM fixtures
   pass; and
5. the code/image/RSS/startup trade-off is explicit.

The first PERF2-02 deliverable is an approved quickening design and a bounded
PoC panel, not a broad opcode family. The first production slice then follows
the mandatory first Release verdict and remains provisional until Adrian
accepts it. BIF/helper quickening additionally waits for the PERF2-03/04 cleaned
static ceiling so it cannot pre-empt the inlining-first policy.

## PERF2-03 — flow-aware inlining 2.0

Status: **production in progress** — Adrian approved H and production slices
1-4 with QA and an independent commit after each. Slice 1's receiver
transaction is favorable and slice 2's gate infrastructure is byte-identical
parity; execution must pause before slice 5 reference/object work. See
`PERF2-03-WORKLIST.md`.

### Current evidence

Inlining already runs before typed flow analysis, so flow sees the expanded
tree. It does not yet supply pre-inline summaries or a profitability decision,
and its current facts do not fully model block-expression result equivalence,
compiler-temporary ownership or inline formal/result coalescing.

The retained NR-12/21 comparison found a small helper at 16 instructions after
inlining versus 13 in the hand-equivalent lowering, with two extra copies, one
extra branch/register and 412 extra RXAS bytes. A literal case remained nine
instructions versus three manually. This is concrete evidence that frame
removal and semantic inlining are not enough without cleanup.

### Stage A — current inline census and cost model

For every hot or size-significant inline site, record:

- callee identity, imported/local body, call arity and execution count;
- eligibility/rejection reason and structural node size;
- call versus inline versus hand-equivalent dynamic instructions;
- formal/default/result copies, branches, temporaries and initialization;
- maximum locals/registers, temporary footprint and call-window effect;
- RXAS, standalone RXBIN and linked-image bytes; and
- complete-product timing contribution where measurable.

The output is a ranked panel with explicit `inline`, `do not inline` and
`cleanup required` cases. Code size and register pressure are part of the cost
model, not after-the-fact caveats.

### Stage B — analysis architecture

Compare a small set of coherent designs:

1. lightweight pre-inline callee summaries for mutability, escape, return and
   block-result behavior, followed by the existing post-inline full analysis;
2. clone first, then extend NR-26 facts over formal bindings, block results,
   compiler temporaries and inline exits before final lowering; and
3. a bounded fixed point in which accepted cleanup exposes constants/dead
   paths, without repeatedly cloning or destabilizing source identity.

The design should make analysis facts explicit rather than matching one AST
shape. Handwritten RXAS remains RXAS's responsibility; no compiler-only
annotation is required for ordinary machine cleanup. Every structural rewrite
must either preserve a declared set of flow facts or invalidate and rebuild the
CFG/def-use overlay before another transform or final emission. Because current
inlining is destructive, make the profitability decision before irreversible
cloning or retain an untouched original call tree that can be emitted when the
cleaned inline loses.

### Initial transformation panel

- direct formal binding for proved read-only actual/formal pairs;
- dead formal/default initialization removal;
- constant propagation through formal bindings and inline block results;
- formal-to-result or return-result placement only with separate block-result
  equivalence, no harmful aliasing and exact return/cleanup ownership proof;
- dead inline-exit, block-result, branch and temporary removal;
- join-safe copy propagation and last-use moves where ownership is proved; and
- one final profitability check after cleanup, with a non-inline fallback when
  expansion still loses.

### Correctness boundaries

The fixture matrix must cover writable by-value isolation, `.ref`, optional
and default arguments, omitted/status arguments, repeated actuals, aliasing,
returns, joins, zero-trip loops, recursion, signals/unwind, inherited numeric
context, TRACE/source identity, imported inline metadata and optimized/no-opt
behavior. Register lifetime must be verified from the final typed instruction
stream.

### Exit

PERF2-03 completes when accepted inline fixtures contain no avoidable
formal/result copy, initialization, branch or compiler temporary; the output
approaches the hand-equivalent instruction/register/image footprint; a measured
profitability policy rejects losing sites; and a target workload confirms value
beyond static instruction reduction in the smallest decisive profiling-off
Release verdict. Report that verdict and stop for Adrian's acceptance before a
full-portfolio Release refresh.

## PERF2-04 — inlining-first core Level B BIF campaign

### Objective

Make the hot, bootstrap-safe Level B BIF surface execute as the simplest
semantic machine path while retaining the maintainable Level B source as the
complete fallback and documentation of behavior.

The component catalogue identifies a measured Level B bootstrap closure rather
than “make every function native.” PERF2-01 must refresh the exact library
module/selector inventory and rank it by product calls and self/child cost.
Known RexxCPS timed controls are `LENGTH`, `SUBSTR` and `WORD`; formatting BIFs
outside its timed kernel must not be presented as RexxCPS causes.

### Per-BIF ladder

Every candidate moves through the same ladder:

1. **Clean source inline:** compile the current Level B body with PERF2-03
   cleanup and existing primitives.
2. **Hand-equivalent ceiling:** express the simplest known semantically correct
   lowering to quantify remaining scaffold/scan/copy cost.
3. **General assist control:** prototype one narrow RXAS/VM semantic kernel only
   if cleaned source cannot reach the ceiling.
4. **Native/intrinsic control:** use a direct runtime implementation to bound
   overhead, not as the automatic production answer.
5. **Placement decision:** choose Level B inline, compiler lowering, public
   RXAS assist or private quickening using the shared PERF2-02/05 gate.

### Seed panel

| BIF/family | First question | Possible assist only after cleanup evidence |
| --- | --- | --- |
| `LENGTH` | Can result initialization/copy scaffolding around existing `strlen` disappear completely? | None initially. |
| `SUBSTR`, `LEFT`, `RIGHT` | What remains after validation, optional/padding and result cleanup? | Non-mutating codepoint slice/span operation. |
| `WORD`, `WORDS`, `WORDPOS` | Is repeated scanning/cursor/slice setup dominant across workloads? | General word-span/count/extract plan or assist. |
| `POS` and related search | Does the current primitive already dominate, or does wrapper/setup remain? | General codepoint search only if reused. |
| typed conversion BIFs | Are representation crossings still material after NUMERIC-01? | Representation-preserving conversion path, preferably private until stable. |

The final panel is selected from current profiles, not frozen by this seed.

### Semantic and adoption gates

Preserve validation and signals, Unicode/codepoint behavior, 1-based indexing,
padding, optional/default/status semantics, numeric context, empty/boundary
cases, references and TRACE/source behavior. Test direct, imported inline,
unoptimized and both-VM forms.

A new assist advances only if it is general beyond one benchmark, occurs at
multiple static/product sites, beats the fully cleaned inline form, reduces
machine work, and is demonstrably better as public RXAS than a compiler-owned
combination or private quickened form. A BIF may complete with no new opcode.

## PERF2-05 — RXAS semantic assists and instruction improvement

### Starting point

NR-09 and NR-18/27 established the machinery: arbitrary operand signatures,
opcode effects, compiler exact-template combination, RXAS local and
whole-procedure flow, dual-VM support and RXSEQ evidence. The broad NR-09 batch
also showed the risk: many legal wide forms were neutral or were withdrawn,
while the accepted complete product gained only about 1.4%/2.9% RexxCPS.

The next RXAS programme therefore targets **semantic units**, not operand
count or mnemonic volume.

### Candidate generation and placement

1. Refresh N=2/3/4 RXSEQ across all current images and both VMs.
2. Rank sequences using dynamic executions, static sites, distinct modules,
   retired-instruction reduction and native profile footprint. Do not add
   overlapping sequence counts as though they were independent savings.
3. Prove effects, liveness, alias/reference behavior, intermediate-write
   observability, signal/throw order, TRACE anchors and register pressure.
4. Compare distinct owners/forms: compiler-owned result-only lowering, RXAS
   effect-clean authored rule, canonical runtime-only bytecode, private
   quickened form and assembler-visible public RXAS instruction.
5. Prototype candidates as a bounded panel with exact mathematical correctness
   and instruction-reduction gates before formal timing.

Likely sources are PERF2-04 string/word kernels, PERF2-03 result placement,
PERF2-07 payload-aware copy/conversion work and PERF2-02 stable private forms.
Legacy `FDIVSUB`, `ILOADSETUNLINKN` and deferred propagation ideas remain
inspection candidates only when the new profile selects them.

### Public-form gate

A canonical instruction must:

- have a coherent, documented semantic contract independent of one compiler
  template;
- preserve all intermediate effects that language/RXAS authors can observe
  when the form is assembler-visible; compiler-owned result-only and
  runtime-only forms require their separately proved contract;
- reduce exact machine work in representative linked products with no
  instruction growth elsewhere;
- win ordinary Release time clearly in the selected/default VM and remain
  within accepted regression guards in the other, without unacceptable image,
  handler or instruction-cache growth;
- have complete mechanical opcode effects, assembler/linker/disassembler
  round-trip where public, profiling/RXSEQ visibility and an explicit
  public-source versus runtime-only classification;
- retain old-RXBIN execution and new-feature gating as required; and
- receive Adrian's explicit ISA/RXBIN approval before production assignment.

Otherwise retain the optimization in the earliest private layer that owns the
facts, or reject it with evidence.

## PERF2-06 — VM execution-engine programme

### Objective

Determine and remove the remaining VM-owned cost after compiler, BIF and RXAS
work are attributed. This includes private instruction representation,
dispatch/fetch, calls and frames, values, context/interrupt maintenance,
runtime helpers, code layout and preparation. It is not a mandate to rewrite
the dispatcher.

### Current substrate and constraints

- `rxvm` uses computed-goto dispatch; `rxbvm` uses switch dispatch.
- Both modes now prepare an owned process-local execution image. `rxvm` stores
  handler cells and both modes can hold process-local bound function operands.
- Canonical RXBIN code cells remain immutable and portable.
- Each opcode and operand in the current runtime image occupies an eight-byte
  cell; wide forms can reduce dispatch while increasing fetch/image footprint.
- The VM polls for interrupts at every retired instruction and preserves
  signals, TRACE, numeric context, references, native/plugin calls and late
  load. These are language/runtime semantics, not optional benchmark overhead.

Documentation that still says `rxbvm` executes only the canonical stream must
be reconciled with current code as part of the activity.

### VM-A — native current-HEAD attribution

PERF2-01 must identify, per workload and per VM:

- top C handlers/helpers and their caller stacks;
- retired native instructions, branch/mispredict and instruction-cache costs;
- dispatch versus handler/helper/body proportion;
- runtime-image bytes/cells touched and hot code/text footprint;
- call/frame entry and return subphase costs;
- value copy/move/clear/conversion bytes; and
- interrupt, TRACE, numeric-context, allocation and loader/preparation costs.

Only these results rank the following PoCs.

### VM-B — execution stream and fetch layout

Compare, without changing canonical RXBIN:

1. current wide-cell private image;
2. a compact switch-oriented stream or compact operand overlay;
3. decoded hot/cold forms that keep rare metadata away from the hot fetch path;
4. PERF2-02 quickened private forms; and
5. only RXSEQ-selected fused semantic units.

Measure decode/preparation time, steady-state cycles, branch and i-cache data,
private-image/RSS size, source/profile mapping and both VM modes. A compact
stream is adopted only with representative portfolio evidence on at least two
architectures; code density alone is not a speed result.

### VM-C — residual call and frame path

Run this after PERF2-03/04 removes avoidable small calls. Frame recycling
already exists, but remaining bytecode calls reset local mappings, inherit the
interrupt table, copy/synchronize numeric context and later restore context.
Higher-arity fixed calls are not assumed: refresh their dynamic callee/cost
population after inlining and quickening, and distinguish product use from the
deliberately high-arity JSON fallback control.

Profile and compare:

- shared or copy-on-write inherited interrupt state with a per-frame overlay;
- numeric-context activation only when the effective context changes;
- proved-leaf/lightweight frame activation;
- argument/result placement coordinated with compiler flow; and
- targeted frame pooling or reset specialization by frame shape.

Gates include recursion, repeated calls, all defined signal codes,
reserved-code bounds and the `RXSIGNAL_MAX` sentinel, handler push/pop,
branch-handler ownership, signal call/unwind, writable inputs,
references/aliases, plugin/native calls, decimal-plugin modes, TRACE and late
load. Measure the inherited 32-entry table copy separately.

### VM-D — interrupt, TRACE and cold path separation

Do not remove per-instruction interrupt semantics. First measure individual
poll components and hot/cold code layout. A split synchronous/asynchronous poll
or cold outline is a design candidate only if it preserves observable delivery
and handler behavior, proves the remaining hot test, and wins outside a single
microbenchmark.

Inactive TRACE should remain near-zero-cost, but source coordinates and
debug/profiler identity must survive quickening/fusion. Signals, unwind and
late-load repair paths may be cold-outlined only after complete coverage.

### VM-E — cross-platform dispatch completion

The previous dispatch investigation remains useful negative evidence but did
not complete native Linux x86-64 counters and Windows x86-64 timing. Re-run the
current product on:

- Apple ARM64 with Apple clang and native counters/sampling available on the
  host;
- Linux ARM64 release-build/correctness/timing coverage, with native counters
  where available;
- Linux x86-64 with native branch/cache/perf evidence under supported GCC and
  Clang versions where available; and
- Windows x86-64 timing and code/artifact evidence under the supported Windows
  toolchain.

Preserve early next-target resolution and compare the actual modern
execution-image implementation, not stale pre-NR-16/17 prose. Record exact
compiler versions, `run()` text size and branch/instruction-cache counters;
dispatch layout is compiler-dependent.

### VM-F — lifecycle and preparation

Establish the CAP-04 load-only boundary before optimizing it. Attribute
canonical image verification/copy, semantic graph rebuild, function binding,
quickening preparation, plugin initialization, first frame and teardown.
Current load-first-result is around 2.7 ms on the July 23 host, so this remains
below the large steady-state deficits unless refreshed lifecycle or embedded
use evidence changes the ranking.

### Explicit non-candidates unless new evidence overturns them

| Prior idea | Current disposition |
| --- | --- |
| Remove interrupt polling | Rejected as a semantic break; the measured ceiling was only about 1–5%. |
| Lockstep dispatch cursor | Rejected after prior 15–30% regressions. |
| Force globally separate computed-goto dispatch sites | Deprioritized: mixed results with substantial code growth. |
| Serialize handler pointers or mutate canonical RXBIN | Rejected by portability, process ownership and compatibility. |
| Replace the dispatcher as the first-order answer | Unsupported by competitor evidence and current architecture. |
| Add broad superinstruction families from raw RXSEQ counts | Rejected without effects, overlap, footprint and complete-product proof. |
| Remove `RX_FLATTEN` | Rejected: it reduced text but was neutral/slower. |
| Mark the interrupt path globally `unlikely` | Rejected after severe Apple-clang layout regressions. |

Opcode-indexed/switch dispatch remains a safe portable comparison and fallback,
not a rejected design.

### Exit

PERF2-06 completes with accepted/rejected VM PoCs, a cross-platform
recommendation, current documentation, dual-VM correctness and ordinary
Release evidence that any selected VM change improves real product workloads
without hiding startup, RSS, image or compatibility costs. PERF2-11 Gate E
owns the final default/private execution-architecture decision.

## PERF2-07 — value, frame, representation and allocation work

### Question

Which parts of cREXX's general `value` and frame semantics still create repeated
copy, conversion, reset, cache or allocation work, and what is the smallest
safe intervention?

The current `value` is approximately 248 bytes and can carry scalar, decimal,
string, binary/native, reference, type and object-attribute state. Whole-value
copy can recursively duplicate populated representations/attributes, while
move transfers ownership after clearing the destination. Those facts make
both avoidable copies and a premature global layout rewrite risky.

### Required order

1. Verify current sizes/layouts on each target ABI; do not inherit historical
   Linux or pre-NR-15 profiles.
2. Count operations and bytes by payload shape, caller, procedure and
   benchmark, including conversions/materializations and cache hits.
3. Use compiler/RXAS flow first to eliminate the operation or choose a typed
   copy/move when ownership is proved.
4. Compare payload-shape fast paths and retained representation validity.
5. Only then test frame/value pooling or hot/cold representation changes.

### Candidate ladder

| Level | Candidate | Principal risk |
| --- | --- | --- |
| V1 | eliminate dead/full copies; direct result placement; typed copy/move | aliases, hidden payload release, join/exception ownership |
| V2 | payload-shape fast copy/clear/reset | stale type/attribute/native state |
| V3 | retain validated string/numeric/decimal representations | mutation invalidation, numeric-context semantics, memory growth |
| V4 | share inherited frame state and reset only live slots | signal/context ownership and recursion |
| V5 | size/shape-targeted frame/value pools | teardown, plugins, sanitizer visibility, memory retention |
| V6 | hot/cold `value` split | ABI/layout breadth, cache trade-off, widespread code complexity |

Mutable object/string copy-on-write and a general allocator replacement do not
enter the first panel. They require explicit alias/reference proof and current
evidence that narrower work cannot close the cost.

### Exit

Each accepted slice must reduce exact operations/bytes or demonstrated native
cost, pass sanitizer and ownership/lifecycle fixtures proportional to the
changed ownership surface, and improve a target
workload without breaching the common-portfolio guard. PERF2-07 closes only
after all high-cost shapes have an accepted optimization or an evidence-backed
defer/reject decision. Pooling decisions additionally require the targeted
alloc/free lifetime, retained/high-water, size-class, reuse and allocator-stack
evidence defined by PERF2-01; request counts and RSS alone are insufficient.

## PERF2-08 — capability, equivalence and Level B/G decision lane

Performance comparisons have exposed missing capabilities as well as slow
paths. These require separate treatment so a language design is neither
smuggled into an optimization nor blocked by an optimizer-only worklist.

### Existing capability gaps

| Gap | Performance relevance | Default route | Decision boundary |
| --- | --- | --- | --- |
| CAP-01 JSON parse-once/indexed document | Current JSON cells build/use different result models. | Library/runtime handle or indexed object model first. | Language change only if the library/runtime surface cannot express ownership/use safely. |
| CAP-02 owned heterogeneous/nested containers | cREXX Storage performs materially different allocation/object work. | Specify ownership, nested references and lifecycle; build an equivalent control. Post-Release 1 Level G is the default source-language route. | A Level B source surface requires a separate explicit scope decision after proving a library/runtime solution insufficient; Adrian approval required. |
| CAP-03 standard Base64 surface | Base64 is a valid common cell but lacks one standard portable cREXX product API. | Keep the current common codec-loop benchmark unchanged. Develop a pure Level B API as a separate product track; measure before native/SIMD. | A new API benchmark is separate unless every runtime receives an equivalent workload. Native/SIMD is an optional backend/control, not the semantic API. |
| CAP-04 load-only lifecycle boundary | Current public CLIs combine load and first result. | Measurement/runtime lifecycle boundary. | Promote to public API only with a product use case and API approval. |

### Level B performance closure

Re-audit the provisional selector/module inventory against current HEAD and
identify the bootstrap-critical scalar text, word/parse, arrays, binary
conversion, call/link, classlib collection and runtime-control surface. For
each hot selector choose and document one of:

- clean inline Level B;
- inline plus a general RXAS/VM assist;
- native/runtime intrinsic with a portable Level B fallback;
- cold/not selected; or
- blocked by a named capability/language decision.

Strict read versus grow-on-write and indexed attribute-array access are design
questions only if current semantics prevent an efficient common path. Do not
invent new syntax when flow or a runtime assist can prove the same operation.

### Level G boundary

The planned object/interface collection lowering and nested collection model
remain post-Release 1 Level G work. Interface-led collection contracts,
equality/hash/order and owned heterogeneous containers may ultimately improve
expressiveness and comparable benchmark forms, but they are not near-term
speed patches. Existing `arraydrop` and `objectarraydrop` remain Release 1
compatibility surfaces.

Any syntax, type-system, ownership, collection or public ABI change pauses at
`decision required` with an options paper and explicit Adrian approval before
implementation.

### Equivalence closure

- investigate whether a correct equivalent ooRexx Mandelbrot port can be built;
  retain `not comparable` if equivalence cannot be established;
- construct an object/allocation-equivalent Towers port;
- resolve Storage's owned nested-container mismatch through CAP-02;
- re-audit List's weak-reference/arena adaptation against the intended work;
- agree a common JSON parse/result/access contract through CAP-01; and
- keep every non-common timing visible as a diagnostic, never in the common
  aggregate until qualification passes.

PERF2-08 completes when every Tier A non-common row has joined the qualified
matrix or has a final, approved capability/language disposition.

## PERF2-09 — per-benchmark ooRexx closure campaign

PERF2-09 is the outcome lane. It consumes general mechanisms from PERF2-02
through PERF2-08; it does not authorize benchmark-specific shortcuts.

Each dossier contains exact source/image/runtime hashes, comparability status,
same-session cREXX/ooRexx throughput, gain to parity and strong band, optimized
and diagnostic static/dynamic work, top native/procedure/opcode/call paths,
copies/conversions/allocations/RSS, selected mechanism, machine ceiling and
accepted/rejected verdicts.

The hypotheses below are routing assumptions only until PERF2-01 replaces
them:

| Workload | Current planning hypothesis | Candidate owners | Closure gate |
| --- | --- | --- | --- |
| Sieve | Already a large cREXX win; sensitive to generic loop/value regressions. | Guard only; compiler/VM regression control. | Both VMs remain ahead with no guard. |
| Permute | Already ahead; recursion/call/inlining remains a useful control. | PERF2-03, residual PERF2-06 call path. | Preserve win and image size while shared call work lands. |
| Bounce | Small object methods, references, dispatch, frames and value/allocation traffic likely dominate. | PERF2-02, 03, 06 and 07. | Qualified same-session parity checkpoint, then 1.50x exit. |
| Richards | State-machine calls, branches, object/value state and frame/context work likely dominate. | PERF2-02, 03, 05, 06 and 07. | Largest gap closed without weakening semantics; 1.50x exit. |
| Base64 | Byte/string access, conversion and scan/loop costs in the existing common codec workload; a standard API is a separate product track. | PERF2-04, 05, 07 and CAP-03. | Preserve the unchanged common workload; default reaches 1.50x and alternate clearly wins, with no opaque native-only claim. |
| RexxCPS | Timed BIFs, inline scaffold, stable-site setup, loops, conversions and residual frames. | PERF2-02 through 07. | Disclosed cREXX rate reaches at least 1.50x same-session canonical Classic ooRexx. |
| Mandelbrot | Invalid ooRexx comparator blocks the claim; arithmetic/value path remains diagnostic. | PERF2-08 then 07. | Valid checksum/equal work before any ratio. |
| Towers | Current ports do different object/allocation work. | PERF2-08, then 02/06/07. | Equivalent port and qualification before the superiority target. |
| Storage | Missing owned/nested container shape creates radically different work and RSS. | CAP-02 decision, then 07. | Equivalent ownership/allocation contract and governed result. |
| List | The current cross-session diagnostic appears faster, but ownership differs. | PERF2-08 equivalence, then guard. | Qualification or final disclosed exclusion. |
| JSON | Different parser/result APIs dominate interpretation. | CAP-01 plus PERF2-04/07. | Common parse-once/result-access contract, then the superiority target. |

After every accepted production slice, run the smallest decisive target and
guard comparison first, report it and stop. A full portfolio refresh follows
only after the verdict is accepted. The next slice is selected by the largest
qualified remaining deficit with an attributable general mechanism, not by
which benchmark is easiest to improve.

## PERF2-10 — toolchain, code layout, build and lifecycle

This is a bounded optional lane, not a substitute for semantic work.

### Experiments

- C/C++ link-time optimization and interprocedural optimization;
- profile-guided optimization using a disclosed representative training set;
- hot/cold handler/helper outlining and source layout;
- compiler-specific flags only where supported and maintainable;
- shared VM-core build impact and text duplication;
- eager versus lazy execution-image preparation after CAP-04 attribution; and
- package/install/native artifact and startup consequences.

The VM interpreter is a very large translation unit built in threaded and
switch variants. Record build time/memory, binary text/data size, i-cache and
branch effects, startup, steady state and both VM modes on Apple ARM64, Linux
x86-64, Linux ARM64 and Windows. Keep all supported release architectures in
scope. Never select a flag/layout from one host or from instrumented-profile
elapsed time.

Adopt only repeatable supported-platform improvements with reproducible build
inputs and no benchmark-specific PGO training claim. Otherwise retain the
result as rejected/deferred evidence.

## PERF2-11 — architecture and release gates

The old unstarted architecture footer is replaced by explicit gates.

### Gate A — refreshed truth

PERF2-01 evidence is checksum-closed, same-session comparison is accepted,
comparability labels are current, and every Batch 2 candidate has a measured
mechanism footprint.

### Gate B — placement selection

For each semantic family, compare compiler/inliner, RXAS, link/load and runtime
quickening placement. Record the selected and rejected variants, machine
ceiling, code/image/RSS/startup trade-off and why later-phase knowledge is
required if an earlier owner is not selected.

### Gate C — language, ISA and ABI decisions

Any new syntax/type/ownership rule, public RXAS opcode/RXBIN feature or public
runtime ABI change has an options paper and explicit Adrian approval. Private
PoCs do not create a de facto public contract.

### Gate D — mandatory first Release verdict

After the minimum focused correctness checks pass, freeze the production edit,
build the ordinary profiling-off Release product and run the smallest decisive
exact-hash end-to-end comparison against a retained valid baseline. Report and
stop for direction. The implementation remains provisional/revertable until
accepted. Broad QA, sanitizer, packaging and documentation closeout follow the
decision, not precede it.

The existing formal guards remain: no unexplained worse-than-3% individual
workload and no worse-than-1% common aggregate without explicit acceptance.
Instruction reduction, profile time and microbenchmarks do not replace the
complete-product verdict.

After acceptance, select broad QA, sanitizer, install/package,
RXBIN/ABI/feature-gating and documentation closeout in proportion to the
changed surface, observed failures and Adrian's direction. They follow the
first verdict; the roadmap does not mandate every broad lane for every slice.

### Gate E — cross-platform/default VM

Before default-architecture selection, retain ordinary product evidence from
at least Apple ARM64, Linux x86-64, supported Linux ARM64 and Windows timing,
with both VM modes, code/artifact size and relevant native counters. Keep all
supported release architectures in the regression matrix. Include exact
supported compiler/version coverage: Apple clang on ARM64, GCC and Clang on
native Linux x86-64 where supported/available, and the supported Windows
toolchain. Select the default/private stream using the whole scorecard rather
than one dispatch microbenchmark.

### Gate F — final external claim

- zero correctness failures;
- every Tier A cell qualified or explicitly resolved under PERF2-08;
- selected default VM reaches at least 1.50x ooRexx on every qualified common
  cell and the separately disclosed cREXX RexxCPS 2.2d diagnostic reaches at
  least 1.50x same-session canonical Classic ooRexx RexxCPS;
- common-workload geometric mean reaches at least 2.00x ooRexx;
- alternate/non-default VM is clearly faster than ooRexx on every qualified
  cell;
- both VMs, lifecycle, RSS and artifact results shown separately;
- runtime/compiler/source versions and exact hashes published; and
- no common/aggregate claim based on a cross-session ratio, adapted cell or
  excluded diagnostic. cREXX 2.2d versus canonical Classic ooRexx RexxCPS
  remains a separately disclosed, governed diagnostic comparison rather than a
  common cell.

## PERF2-12 — JIT/AOT decision

NetRexx demonstrates the ceiling available when a hot semantic graph reaches a
mature optimizing VM, but it does not prove that a cREXX JIT yields its exact
ratio. A JIT, trace compiler, native AOT backend or external optimizer is a
separate architecture programme.

Keep PERF2-12 deferred until the accepted non-JIT programme and current
cross-platform scorecard show that the unquestionable-superiority exit cannot
be reached economically. Reopening it requires:

- the residual gaps and hot semantic graph after PERF2-02 through PERF2-10;
- a comparison of native AOT, baseline JIT, tracing/quickening and existing VM
  maintenance cost;
- debugger/TRACE, signals, dynamic loading, plugins, portability, packaging,
  sandboxing and deterministic-build requirements; and
- an explicit architecture decision from Adrian.

## Worklist and evidence contract

Before the first production edit in any activity, create a resumable worklist
that records:

1. exact clean baseline commit, branch state and artifact hashes;
2. one falsifiable performance hypothesis and named mechanism footprint;
3. candidate variants, static/runtime ownership and machine ceiling;
4. semantic risks and focused correctness matrix;
5. exact profiling-off Release comparison and regression guards;
6. first Release verdict stop point;
7. proportional broad QA, sanitizer, install/package,
   RXBIN/ABI/feature-gating and documentation closeout after acceptance and
   Adrian's direction;
8. retained accepted, rejected and neutral evidence; and
9. status/update links back to this register.

Target-only builds and focused PoC loops come first. Formal baselines use the
ordinary product and existing governance. All new performance orchestration is
Level B cREXX under `performance/tools/`; temporary host-side analysis may be
used for investigation but does not become the maintained control plane.

## Authoritative references

- programme rules: [`AGENTS.md`](AGENTS.md) and
  [`PERFORMANCE-GOVERNANCE.md`](PERFORMANCE-GOVERNANCE.md)
- initial closed register:
  [`ROADMAP-INITIAL-SWEEP-2026-07-23.md`](ROADMAP-INITIAL-SWEEP-2026-07-23.md)
- dated charter:
  [`performance-programme-report-2026-07-15.md`](../docs/planning/release-1/performance-programme-report-2026-07-15.md)
- cross-runtime mechanisms:
  [`rexxcps-runtime-source-review-2026-07-22.md`](rexxcps-runtime-source-review-2026-07-22.md)
- latest current-product checkpoint:
  [`2026-07-23 NR-16/NR-17 closeout`](evidence/2026-07-23-nr-16-17-closeout/README.md)
- capability ledger: [`capability-gaps.md`](capability-gaps.md)
- compiler and VM architecture:
  [`CREXX_ARCHITECTURE.md`](../docs/ai-context/CREXX_ARCHITECTURE.md),
  [`RXAS_ASSEMBLER.md`](../docs/ai-context/RXAS_ASSEMBLER.md) and
  [`RXVM_INTERPRETER.md`](../docs/ai-context/RXVM_INTERPRETER.md)
- inlining and flow evidence:
  [`NR-12-21-WORKLIST.md`](NR-12-21-WORKLIST.md),
  [`NR-26-WORKLIST.md`](NR-26-WORKLIST.md) and
  [`NR-27-WORKLIST.md`](NR-27-WORKLIST.md)
- language/capability planning:
  historical/working
  [`levelb-language-improvement-backlog.md`](../docs/planning/beta-3/notes/levelb-language-improvement-backlog.md)
  (reconcile completed items before reuse),
  [`array-statements.md`](../docs/planning/beta-3/notes/array-statements.md) and
  [`cross-cutting-conclusions.md`](../docs/planning/release-1/component-catalogue/cross-cutting-conclusions.md)
- VM investigation history:
  [`vm-dispatch-performance-investigation.md`](../docs/planning/beta-3/notes/vm-dispatch-performance-investigation.md)
