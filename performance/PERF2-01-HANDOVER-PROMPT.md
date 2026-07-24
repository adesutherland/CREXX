# PERF2-01 new-session handover prompt

Recommended setting: **Very High** for this PERF2-01 evidence activity. Reserve
**Ultra** for PERF2-02 quickening architecture and its first correctness-sensitive
PoC after PERF2-01 is accepted.

Copy the prompt below into a new session.

---

You are working in `/Users/adrian/CLionProjects/CREXX` on the first executable
activity in the second cREXX performance programme: **PERF2-01 — current
baseline and attribution refresh**.

Your task is to complete PERF2-01, not merely plan it. Build the resumable
control plane, audit/extend the diagnostic coverage where required, collect the
current evidence, produce the per-benchmark gap/cost dossiers and stop for
Adrian's acceptance of the refreshed selection baseline. Do **not** implement
quickening or any other production optimization in this session.

## Current orientation

At handover preparation time:

- branch: `develop`;
- HEAD: `d5b25a78fd6cd2b5b5962b45e508f3cb2bb782e6`;
- `origin/develop` matched that HEAD when this handover was finalized;
- the working tree contains intentional uncommitted performance documentation,
  including the replacement roadmap and initial-sweep archive.

Treat those facts as orientation only. Verify the exact live branch, HEAD,
upstream and worktree before doing anything. Preserve every pre-existing user
change. Do not reset, stash, overwrite, commit or push unrelated changes. Do
not push anything without explicit Adrian approval.

The current accepted planning comparison is not a governed current
cross-runtime baseline:

- common-five cross-session cREXX/ooRexx geometric mean:
  0.868336 (`rxvm`) and 0.819722 (`rxbvm`);
- cREXX RexxCPS 2.2d: 28.120M/26.119M clauses/s;
- historical ooRexx RexxCPS: 39.921M clauses/s;
- Sieve and Permute are already cREXX wins;
- Base64 is relatively close;
- Bounce and Richards are the largest qualified deficits;
- Mandelbrot, Towers, Storage, List and JSON retain comparability/adaptation
  qualifications.

Do not publish these cross-session ratios as the refreshed result.

## Mandatory reading and instruction order

Read completely before taking task actions:

1. repository `AGENTS.md`;
2. `performance/AGENTS.md`;
3. `performance/ROADMAP.md`, especially the North star, PERF2-01, PERF2-02 and
   PERF2-11 gates;
4. `performance/PERFORMANCE-GOVERNANCE.md`;
5. `performance/README.md`;
6. `performance/portfolio/manifest.md` and
   `performance/portfolio/cross-runtime-plan.md`;
7. `performance/evidence/2026-07-23-nr-16-17-closeout/README.md`;
8. `performance/rexxcps-runtime-source-review-2026-07-22.md`;
9. `docs/books/crexx_programming_guide/profiling.md`;
10. `docs/ai-context/RXVM_INTERPRETER.md`;
11. the existing evidence tools/manifests and current profiler implementation
    relevant to the capture.

Use the live roadmap over historical worklist prose when they differ. Preserve
the dated charter and closed initial roadmap as history.

## Required working method

Before the first implementation or evidence edit:

1. provide Adrian with a numbered execution plan;
2. create `performance/PERF2-01-WORKLIST.md` using `apply_patch`;
3. record exact live repository state, host/power state, compiler/toolchain,
   build configuration, installed external runtime versions and the intended
   artifact/evidence layout;
4. split the worklist into resumable stages with explicit commands, outputs,
   completion boxes and blockers; and
5. state the exact Gate A exit and the point at which you will stop for
   acceptance.

Keep commentary updates concise and frequent. For verbose builds, profiles or
tool output, use a `mktemp` log and inspect focused slices; do not stream
unbounded output.

Use target-only/focused builds first. All maintained performance orchestration
must be cREXX Level B under `performance/tools/`. Temporary native host tools
may be used for system profiling, but they do not become the maintained control
plane.

## Scope and non-goals

PERF2-01 must establish current truth. It may add or repair profiling and
evidence tooling required to observe that truth, but it must not alter ordinary
product semantics or implement a performance candidate.

Do not start:

- quickening or private instruction specialization;
- inlining cleanup;
- new BIF/RXAS opcodes;
- frame/value pooling or representation redesign;
- LTO/PGO selection;
- language, Level B/G, RXBIN or public ABI changes; or
- benchmark-specific shortcuts.

If a measurement defect or correctness failure is found, isolate and report it
before treating any affected result as evidence. Ask for direction before a
material expansion beyond diagnostic infrastructure.

## Stage 0 — exact baseline and dirty-tree isolation

- Verify branch/HEAD/upstream and enumerate all pre-existing modifications.
- Confirm whether the dirty tree is documentation-only or contains product
  sources. Preserve it exactly.
- Freeze the product source under test at a clean exact commit. If the main
  worktree is documentation-dirty but product-identical to HEAD, use a clean
  detached/scratch worktree outside the repository for formal builds and record
  that relationship. Do not commit/reset the user's documentation merely to
  obtain a clean build tree.
- Verify host AC power, low-power/thermal state, logical CPUs and available
  storage before formal runs.
- Inventory current cREXX, ooRexx, NetRexx and Regina executables/versions.
- Reconfirm all benchmark correctness expectations, work counts and
  comparability labels.

## Stage 1 — profiler and evidence-tool gap audit

Audit current schema 4, evidence tools and summarizers against PERF2-01's
required telemetry:

- opcode/transition/procedure/call/RXSEQ data;
- fresh/reused frames and frame-entry subphases;
- copy, typed-copy, move, clear, reset and destroy operations by payload
  shape/bytes;
- string/numeric/decimal conversion, materialization and representation-cache
  hits/misses;
- branch direction and loop/backedge counts;
- stable-site types/targets, cache hits/misses and invalidations;
- loader/binder/execution-image/plugin/first-frame/teardown phases; and
- allocation requests plus system alloc/free lifetime, retained/high-water
  bytes, size classes and allocator stacks for selected outliers.

First write a precise coverage table: available now, derivable now, missing,
or not justified. Do not implement counters merely because the roadmap lists a
possible dimension.

Where required evidence is genuinely unavailable, implement a versioned schema
5 (or explicit equivalent) diagnostic extension with:

- stable row/field definitions;
- per-domain degraded/overflow status;
- backward handling of schema 4;
- updated Level B summarizers, documentation and focused tests;
- deterministic counts-only operation where appropriate; and
- proof that `CREXX_VM_PROFILING=OFF` compiles the instrumentation out and that
  counts/profile elapsed time never becomes product timing evidence.

Keep profiler changes diagnostic and separately attributable. Run focused
Debug/Release checks as needed; use supported sanitizer workflows only when the
changed surface warrants them.

## Stage 2 — product-authority timing matrix

Using the ordinary profiling-off Release product and exact optimized images:

1. run all 11 Tier A steady-state workloads under `rxvm` and `rxbvm`;
2. use the existing serial, balanced/rotated, correctness-gated sampling,
   warmup, append and noise rules;
3. retain exact commands, source/module/image/runtime hashes and raw outputs;
4. capture lifecycle, RSS and artifact size separately from steady-state
   throughput; and
5. reject no valid sample merely because it is an outlier.

RexxCPS must retain both benchmark-native clauses/s and process-inclusive
elapsed time. The cREXX 2.2d adaptation remains a disclosed diagnostic, not a
common-aggregate member.

## Stage 3 — same-session external comparison

In the same governed session:

- run the five qualified common workloads for cREXX, ooRexx and decimal
  NetRexx using equal work;
- run canonical Classic RexxCPS for ooRexx, Regina and NetRexx;
- report cREXX RexxCPS 2.2d separately;
- preserve all current comparability/adaptation exclusions;
- record exact external runtime versions and commands; and
- keep lifecycle, RSS and artifact results separately labelled where
  qualified.

The external capture must be same-session evidence. Historical competitor
columns may remain visible only as explicitly labelled orientation.

## Stage 4 — diagnostic attribution

For all 11 workloads and both profile VMs, capture the current optimized
diagnostic profile. Use no-opt only as attribution—preferably all workloads, or
at minimum RexxCPS, Bounce, Richards, Base64 and one already-winning control.

Capture exact RXSEQ N=2/3/4 images and module sets. Remember that current RXSEQ
windows stop at calls/taken branches; do not treat overlapping straight-line
windows as whole-loop truth.

Run native system sampling/counters against the uninstrumented product where
available:

- cycles and native instructions;
- branch/mispredict evidence;
- instruction-cache/iTLB evidence;
- sampled and annotated hot stacks; and
- targeted system heap/allocation lifetime profiles for Bounce, Richards,
  Storage and any newly selected allocation outlier.

Cover the portfolio once, then repeat the largest gaps and noisy hotspots. Keep
native/system profiles diagnostic and host/tool specific.

Add exact-hash cREXX RexxCPS family controls only for attribution: BIFs,
internal calls/argument parsing, TRACE/ADDRESS, stems, decimal/string loops and
PARSE. Preserve nominal clause-accounting provenance. These controls never
replace the published cREXX 2.2d result.

## Required deliverables

Complete and retain:

1. `performance/PERF2-01-WORKLIST.md` with every stage and disposition;
2. one checksum-closed PERF2-01 evidence bundle;
3. updated `performance/evidence/benchmark-median-summary.md`;
4. a per-workload gap ledger containing:
   - comparability status;
   - `rxvm`/`rxbvm`/ooRexx throughput;
   - ratio and gain required for parity, 1.50x per-cell target and 2.00x common
     geomean target;
   - static/dynamic instructions and image footprint;
   - top procedures/opcodes/transitions/native stacks;
   - calls/frame reuse and reset cost;
   - copy/conversion operations and bytes;
   - allocations, lifetime and RSS;
   - site stability/cache data where available; and
   - selected likely owner: compiler/inliner, RXAS, link/load, VM quickening,
     value/frame work or capability/equivalence;
5. a cross-workload mechanism census;
6. a shortlist of bounded PERF2-02/03/04/06/07 candidate panels, each tied to
   measured evidence rather than source intuition; and
7. an exact verification/checksum record with no unexplained degraded or
   overflowed profile.

## Decision and stop gates

- Product timing authority is always the ordinary profiling-off Release build.
- Do not select a performance implementation merely from instruction counts,
  profile time, native samples or a microbenchmark.
- Do not silently repair or reclassify a non-comparable benchmark.
- Do not update the dated charter or closed initial roadmap.
- Do not proceed into PERF2-02 quickening in this session.
- When the evidence bundle, updated median index, workload dossiers and
  candidate ownership map are complete, present the refreshed result and stop
  for Adrian's acceptance of PERF2-01 Gate A.

The completion report must lead with:

- the new same-session common-portfolio ratio to ooRexx;
- the new separately disclosed RexxCPS ratio;
- which workloads already meet parity/1.50x and which dominate the remaining
  gap;
- the newly measured dominant cost family per workload;
- any instrumentation or comparability limitations; and
- the recommended first bounded PoC panel—without implementing it.

## Ability boundary for the next session

Very High is sufficient for this evidence programme if execution is careful and
resumable. When PERF2-01 is accepted, start PERF2-02 in a fresh **Ultra**
session. Quickening's first design/PoC must jointly prove persistent
process-local image state, invalidation/dequickening, computed-goto label-owner
stability, canonical retirement/interrupt/TRACE boundaries, late-load and
embedded re-entry behavior, profiling identity and both VM modes. After that
architecture and reference state machine are frozen, bounded quickening slices
can return to Very High.

---
