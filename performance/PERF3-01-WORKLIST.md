# PERF3-01 current-HEAD Mac evidence and baseline-validity worklist

Status: **complete — evidence/ranking package accepted by Adrian on 2026-07-31; no production edit made**

Started: 2026-07-31

Purpose: reconcile the exact current Apple ARM64 product with the accepted
PERF2 Mac scorecard and the retained Linux/Windows attribution, refresh only
the evidence required for current candidate ranking, and present a ranked
PERF3-02/03/04/05 mechanism panel to Adrian. This activity does not authorize a
production performance edit, candidate installation, architecture selection,
commit of later evidence, or push.

## Decision gate and mandatory stop

PERF3-01 ends when Adrian receives and accepts:

1. the exact current-product and retained-evidence validity boundary;
2. any smallest governed current-HEAD Mac refresh required to restore ranking
   authority;
3. a gap/mechanism ledger with operation counts, bytes, native footprint,
   machine ceiling, semantic risks and earliest safe owner; and
4. a ranked PERF3-02/03/04/05 panel with explicit accepted, rejected,
   evidence-gated and deferred routes.

**Stop point:** report the evidence package and recommendation, then stop for
Adrian. Do not implement a PERF3-02/03/04/05 candidate, edit production
compiler/assembler/VM/library code, run broad closeout, commit the PERF3-01
results or push unless separately requested.

## Exact starting state

### Repository

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch: `develop`
- Start HEAD: `3f43a0014` (`docs: archive PERF2 and approve PERF3 roadmap`)
- Upstream at start: `origin/develop` at `21fdcf529`; relation `+1/-0`
- Approved transition commit: local only; no push requested or performed
- Pre-existing worktree scope: five untracked generated lifecycle RXBIN files;
  no tracked change at activity start

| Existing file | SHA-256 at start |
| --- | --- |
| `performance/evidence/2026-07-15-nr-02-portfolio-expansion/lifecycle/crexx/lifecycle_probe.rxbin` | `447350ecd62f33d441b1ffe82600fa50acc23f318724d87e4ec079a207c187ed` |
| `performance/evidence/2026-07-20-nr-10-formal-baseline/lifecycle-decimal/crexx/lifecycle_probe.rxbin` | `19216070c4921764404dc48d2c018ad54c3e67e401cd27cd5084f501e219b2df` |
| `performance/evidence/2026-07-23-nr-16-17-closeout/final-baseline/lifecycle/crexx/lifecycle_probe.rxbin` | `19216070c4921764404dc48d2c018ad54c3e67e401cd27cd5084f501e219b2df` |
| `performance/evidence/2026-07-23-perf2-01-current-baseline/04-lifecycle-rss/lifecycle/crexx/lifecycle_probe.rxbin` | `19216070c4921764404dc48d2c018ad54c3e67e401cd27cd5084f501e219b2df` |
| `performance/evidence/2026-07-27-perf2-06-07-selection-panel/raw/lifecycle/crexx/lifecycle_probe.rxbin` | `19216070c4921764404dc48d2c018ad54c3e67e401cd27cd5084f501e219b2df` |

These files are preserved in place. Their presence and hashes are part of the
starting state; PERF3-01 must not delete, normalize or overwrite them.

### Host and toolchain

- Capture: `2026-07-31T12:45:28Z`
  (`2026-07-31T13:45:28+0100`, BST)
- Host: MacBook Air `Mac17,3`, Apple M5, Apple ARM64
- OS: macOS 26.5.2 build 25F84; Darwin 25.5.0 arm64
- CPU: 10 physical / 10 logical CPUs
- Memory: 25,769,803,776 bytes (24 GiB)
- Power: AC attached; battery 80%; not charging
- Start load average: `2.67 1.91 1.54`
- Apple clang: 21.0.0 (`clang-2100.1.1.101`)
- CMake: 4.3.2; Ninja: 1.13.2; Git: 2.50.1

### Existing build trees

The main checkout contains existing `cmake-build-release`,
`cmake-build-profile`, `cmake-build-debug` and `cmake-build-debugasan` trees,
plus historical NR-14 trees. Cache configuration reports:

| Tree | Configuration | Profiling |
| --- | --- | --- |
| `cmake-build-release` | Release | OFF |
| `cmake-build-profile` | Release | ON |
| `cmake-build-debug` | Debug | OFF |
| `cmake-build-debugasan` | Debug | OFF |

Their existence does not prove current-HEAD product identity. PERF3-01 must
rebuild the exact required targets or use an isolated tree and hash every
accepted product before timing.

## Authority and preserved evidence

Read and apply:

- [`ROADMAP.md`](ROADMAP.md) — approved PERF3 scope and transfer ledger;
- [`AGENTS.md`](AGENTS.md) and
  [`PERFORMANCE-GOVERNANCE.md`](PERFORMANCE-GOVERNANCE.md) — standing rules;
- [`PERF2-09 Mac closure`](evidence/2026-07-27-perf2-09-mac-closure/) — last
  formal same-session Apple scorecard;
- [`PERF2-10/11 Linux x86-64`](evidence/2026-07-28-perf2-10-11-intel-linux/) —
  schema-5 and native-PMU mechanism authority;
- [`PERF2-11 Windows`](evidence/2026-07-29-perf2-11-windows-x86-64/) and its
  compiler controls — platform observations, not Mac before/after baselines;
- [`profiling.md`](../docs/books/crexx_programming_guide/profiling.md) and
  [`RXVM_INTERPRETER.md`](../docs/ai-context/RXVM_INTERPRETER.md) — current
  profiling and VM contracts.

The PERF2 Mac scorecard used detached clean source `057592681`, whose accepted
product parent was `39d3c652e`; the intervening commit was test-only. Its
ordinary Release products and library are checksum-bound. It remains a
historical same-session observation until the delta audit below proves which
parts are valid for exact current-HEAD ranking.

## Scope and invariants

- [x] Preserve canonical workload sources, arguments, correctness observations
      and common-five membership.
- [x] Keep RexxCPS first-class in any representative multi-workload set and
      separate from the common geometric mean.
- [x] Keep ordinary profiling-off Release wall clock as timing authority;
      profiles, native samples and RXSEQ remain diagnostics.
- [x] Report `rxvm` and `rxbvm` separately.
- [x] Do not compare unmatched historical and current sessions as a
      before/after regression.
- [x] Reuse retained evidence when exact hashes and product scope remain valid;
      do not rerun a formal baseline merely for freshness.
- [x] Preserve source/TRACE metadata, canonical RXBIN, late-load/plugin paths,
      signals, references and public ABI/language contracts.
- [x] Keep all maintained orchestration in cREXX Level B. No new tool is
      currently planned.
- [x] Keep Linux x86-64 and Windows immutable evidence as inputs; no remote or
      platform rerun belongs to PERF3-01.
- [x] Make no production compiler, RXAS, VM or library performance edit.

## Numbered execution plan

1. Verify the approved transition commit, branch/upstream relation, dirty
   scope, host, power, toolchain and build configuration.
2. Replay the checksum manifests actually used by the PERF2 Mac selection and
   closeout; record any absent, ignored or reconstructed artifact explicitly.
3. Audit every product-affecting commit from the accepted Mac product through
   PERF3-01 start and classify its possible effect on compiler output, RXBIN,
   runtime, library, workload, comparator or evidence tooling.
4. Compare exact current source/workload/tool hashes with the PERF2 artifact
   inventory and decide which historical timing, attribution and comparator
   cells remain usable for ranking.
5. Build and smoke the exact current ordinary Release product; build the
   diagnostic profile product only if retained diagnostic products cannot
   answer the selected questions.
6. Choose the smallest governed current-HEAD refresh. Prefer same-session
   drift controls and exact target/guard cells; run a full common-five absolute
   baseline only if aggregate authority is actually needed.
7. Refresh counts/native samples only where the retained Mac/Linux evidence
   cannot distinguish owner or mechanism. The expected focus is Richards,
   Towers, Base64, RexxCPS and Sieve as a zero-work/layout guard, in both VMs.
8. Produce a compact checksum-closed evidence bundle and gap/mechanism ledger,
   update the roadmap/worklist, present the ranking and stop.

## Resumable stage ledger

### Stage A — control plane and exact start

- [x] Approved PERF2 archive/PERF3 roadmap committed locally as `3f43a0014`.
- [x] Root and performance instructions reread.
- [x] Current branch/upstream/dirty scope recorded.
- [x] Five pre-existing lifecycle RXBIN hashes recorded and protected.
- [x] Host, power, load, toolchain and build-cache configuration recorded.
- [x] PERF3-01 worklist created and roadmap status changed to `in progress`.

### Stage B — retained evidence integrity

- [x] Verify the PERF2-09 Mac closure recursive checksum manifest.
- [x] Verify the PERF2-08 qualification bundle used by the closure.
- [x] Verify the relevant PERF2-01 selection-baseline manifest or record the
      exact narrower inputs used instead.
- [x] Verify retained PERF2-06/07 accepted-closeout evidence needed for the
      post-baseline product history.
- [x] Record every ignored/untracked generated lifecycle artifact and whether
      it is already checksum-bound by a retained bundle.

Integrity replay on 2026-07-31:

- PERF2-09 Mac closure: `54/54` recursive checksum rows pass; checksum-file
  SHA-256 `5591ae327f246e2d0918a0ebd760fb5dc475302ea06662bc5b73b6903e4af677`.
- PERF2-08 qualification: `32/32` rows pass; checksum-file SHA-256
  `83d9984887eac695c533802c2188e986542849be7ccd8f5f26f314af862bf301`.
- PERF2-01 current baseline: `1948/1948` rows pass; checksum-file SHA-256
  `fde9aa923fc451ea16edc0c4f09bf532faa2e024aa22fbfc72e35a965f78eacc`.
- PERF2-06/07 accepted closeout: its `SHA256SUMS` authority passes `53/53`.
- All 24 formal benchmark/lifecycle source rows and all current comparator
  executable/JAR hashes match the PERF2-09 artifact inventory exactly.
- Four pre-existing generated lifecycle RXBINs with SHA-256 `192160...b2df`
  are already checksum-bound in later retained bundles. The earlier NR-02
  `447350...7ed` file remains preserved as an explicit pre-existing artifact;
  none was used as current PERF3 evidence.

### Stage C — post-scorecard product delta

- [x] Classify each commit from `057592681`/`39d3c652e` through `3f43a0014`.
- [x] Identify compiler-output changes affecting common/Towers/RexxCPS images.
- [x] Identify VM/runtime/library changes affecting execution or comparators.
- [x] Identify benchmark/manifests/tool changes affecting exact execution or
      result interpretation.
- [x] Build a source/product/hash validity matrix against `artifacts.csv`.

Delta classification:

- `057592681` is the qualification test-only child of accepted product parent
  `39d3c652e`; `d5f0827ca` and the later PERF2 handover commits are retained
  evidence/control-plane work.
- `55276f151` and the RXPP sequence are outside the selected benchmark source
  and VM execution kernels; `cb283549f` changes only syntax-highlighter
  projection indexing.
- `d78c6fcfa` contains correctness/sanitizer repairs in compiler constant
  folding, binary-to-string validation, decimal/plugin code and `x2d`; these
  invalidate exact product/library identity even where a selected RXAS stream
  remains unchanged.
- `003c4dfde` adds MSVC controls plus portable VM time/date and RXPA constant
  pool handling. Its Apple hot-path intent is neutral, but executable identity
  is changed and must not be assumed performance-identical.
- `ea25d1720` is product-affecting: compiler flow and inline-binding work opens
  proved read-only binary by-value aliasing, while VM/RXPA and `rxjson` also
  change. It can affect Base64 generated code and the linked library.
- `ccb71bb1e`, `815a5a7cf`, `2ca41a61e`, `21fdcf529` and `3f43a0014` are
  test/tool/evidence/documentation work for the selected product boundary.
- The current qualified ooRexx Towers source differs from product parent `39d`
  because qualification added it after that commit, but its hash exactly
  matches the accepted PERF2-09 formal artifact. No comparator refresh is
  required.

Current clean detached Release at `3f43a0014`, compared with PERF2-09:

| Artifact class | Exact result |
| --- | --- |
| Selected sources and comparator runtimes | all match |
| Formal manifest | matches (`aa15f5...e931`) |
| Matrix driver | differs (`77cfd1...ceab` to `efb194...a06c`) |
| `rxc`, `rxas`, `rxlink`, `rxvm`, `rxbvm` | all differ |
| `library.rxbin` | differs and grows `862512` to `935917` bytes |
| Sieve/Permute/Bounce/Richards/Towers RXAS | exact matches |
| Base64 RXAS | differs, `114978` to `114864` bytes |
| RexxCPS RXAS | same size but different hash |
| All seven selected RXBINs | differ |

The matching five RXAS streams retain strong historical mechanism evidence,
but changed executables, linked library and RXBIN containers prevent treating
the PERF2 timing cells as exact current-product observations.

### Stage D — baseline-validity decision

- [x] State which PERF2 Mac absolute cells remain valid historical observations.
- [x] State which cells can rank current mechanisms without rerun.
- [x] State the exact smallest current-HEAD timing cells required.
- [x] State the exact smallest current-HEAD diagnostic cells required.
- [x] Freeze the run schedule, stop conditions and evidence output root before
      starting measurements.

Decision:

- PERF2-09 remains the valid accepted historical same-session observation at
  `39d3c652e`; Linux schema-5/native-PMU evidence remains valid attribution for
  the accepted product, and Windows remains platform-only evidence.
- Matching sources, comparator hashes and five RXAS streams keep the retained
  operation/native mechanism ranking usable as a prior, but not as an exact
  current-product timing claim.
- Because both VM executables, every RXBIN and the library differ, the smallest
  defensible current timing refresh is the complete existing formal manifest:
  common five, separate Towers, and first-class separate RexxCPS, with both
  VMs and the unchanged comparators. Run the ordinary `2` warmups/`10`
  recorded schedule, serial and rotated, and apply at most the governed one
  ten-sample append to any noisy cell.
- Current schema-5 counts are required only for Richards, Towers, Base64,
  RexxCPS and a Sieve zero-work/layout guard, under both VMs. Retained Linux
  PMU attribution supplies native footprint; no ungoverned macOS sampler or
  remote platform rerun is justified.
- Evidence root is frozen as
  `performance/evidence/2026-07-31-perf3-01-current-mac/`. Stop on correctness
  failure, power/thermal invalidation, overlapping build/test activity, or a
  matrix-driver validation failure; retain invalid output and do not prune.

### Stage E — exact current product and correctness

- [x] Reconfigure/rebuild exact profiling-off Release targets with a temp-log
      workflow for any verbose build.
- [x] Record build flags and prove `CREXX_VM_PROFILING=OFF`.
- [x] Run the minimum correctness gate covering the selected workloads and both
      VMs before timing.
- [x] Hash `rxc`, `rxas`, `rxlink`, `rxvm`, `rxbvm`, `library.rxbin`, selected
      workload sources/RXAS/RXBIN and manifests.
- [x] Build/profile only the diagnostic targets justified by Stage D.

The exact ordinary Release product is an isolated clean detached build at
`3f43a0014` under `/private/tmp/crexx-perf3-01.jetu7C`, configured with
`CMAKE_BUILD_TYPE=Release`, Apple-clang `-O3 -DNDEBUG`, Ninja and
`CREXX_VM_PROFILING=OFF`. The focused linked-opt CTest gate passes `8/8`
(fixture plus seven selected `rxvm` workloads); a separate exact-argument
`rxbvm` smoke passes all seven selected workloads, including canonical
RexxCPS. No build/test process may overlap the formal run.

### Stage F — governed current-HEAD refresh

- [x] Capture pre-run host/power/thermal/load state.
- [x] Run all samples serially with no concurrent build/test activity.
- [x] Apply formal warmup/sample/rotation and noise rules to every claimed cell.
- [x] Preserve raw outputs, correctness, benchmark-native metrics and invalid
      observations without pruning.
- [x] Keep throughput, lifecycle, RSS and artifacts separate.
- [x] Capture post-run state and recursively checksum the compact final bundle.

Formal timing passed `348/348` initial executions. The governed one-append rule
selected `sieve-rxvm`, `permute-netrexx`, `richards-netrexx` and both Base64 VM
cells; all `50/50` append executions passed and all five remain labelled noisy
at `n=20`. No sample was removed and no second append was taken. The corrected
common-five geometric means are `2.139811x/1.818954x` versus ooRexx and
`0.779920x/0.662974x` versus decimal NetRexx for `rxvm`/`rxbvm`. An earlier
block wired to the retired PERF2 product is retained, marked invalid and
excluded. The focused schema-5 capture passed `10/10`, with all 70 domain rows
complete. No RSS, lifecycle, remote platform or ungoverned native-sampling
campaign was added.

### Stage G — ranking and stop

- [x] Produce current gap and gain-to-target rows.
- [x] Produce copy/value/conversion/accessor/layout mechanism rows with exact
      counts/bytes/native footprint and machine ceilings.
- [x] Rank PERF3-02/03/04/05 with explicit owner and entry-gate disposition.
- [x] Update the roadmap and this worklist only after evidence is verified.
- [x] Run `git diff --check`, review the complete documentation/evidence diff
      and confirm the five pre-existing RXBIN hashes remain unchanged.
- [x] Present the decision package to Adrian and stop before production work.

The proposed activity ranking is PERF3-02 first, PERF3-05 second, PERF3-03
third and PERF3-04 evidence-gated. Exact reasoning and rejected automatic
routes are in
[`decision-summary.md`](evidence/2026-07-31-perf3-01-current-mac/decision-summary.md)
and
[`gap-mechanism-ledger.csv`](evidence/2026-07-31-perf3-01-current-mac/gap-mechanism-ledger.csv).

Adrian accepted the evidence boundary and ranked panel on 2026-07-31. That
acceptance closes PERF3-01 and opens PERF3-02 for its bounded evidence/design
and PoC comparison. It does not select or authorize a production candidate.

## Expected evidence root

If Stage D requires new retained evidence, use:

`performance/evidence/2026-07-31-perf3-01-current-mac/`

Keep it compact: provenance, exact manifests/hashes, consolidated samples,
focused diagnostic tables, a gap/mechanism ledger, decision summary and one
recursive checksum file. Reference PERF2 bundles rather than copying them.

## Resumption rule

Reread root/performance instructions and the live roadmap, verify HEAD,
upstream and dirty scope, then resume at the first unchecked item. Before any
timing block recheck AC/low-power/thermal/load state and ensure no build, CTest
or benchmark process overlaps. If current state differs from the frozen state,
record the delta before continuing.
