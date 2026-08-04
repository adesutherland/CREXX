# PERF3-05 compiler, layout and private-stream worklist

Status: **complete — retain L0; no production VM edit**

Approved: 2026-08-01

Purpose: determine whether the current Base64/common deficit and accepted
`PERF2-06-D01` layout debt have a supported compiler/layout remedy, or justify
a later private execution representation, without preselecting a flag,
changing canonical RXBIN or hiding semantic work inside a VM-layout claim.

## Authority and stop boundaries

Adrian approved PERF3-05 after accepted P1A closeout on 2026-08-01. This
activity may freeze current-product evidence, build isolated compiler/layout
comparators and implement disposable external PoCs. It must not install a
production source change, make a private stream the default, change canonical
RXBIN, select a cross-platform architecture, push, or begin PERF3-03 silently.

Establish the semantic Base64 string/copy ceiling before interpreting LTO,
PGO, hot/cold or private-stream results. Stop for Adrian at design selection
before any architectural production edit. Any later selected production edit
receives its own mandatory first ordinary Release verdict.

## Exact starting point

- Branch/HEAD: local `develop` at
  `4a3940395980dc40ea45917d71d99caa080e89bb`, three commits ahead of
  `origin/develop`.
- Accepted uncommitted source scope: P1A A1 demand-driven RXAS storage
  attachment, whose rebuilt Release binary exactly matches retained hash
  `7cec2f1245da24cf24ffd37ec94faba0309dfc41f088d5b94c382522c0e9258d`.
- P1A closeout passes 24/24 focused and 1,972/1,972 broad Debug tests. Its
  source and evidence remain frozen while PERF3-05 uses isolated products.
- Host: Darwin 25.5.0 ARM64, Apple M5, 10 logical CPUs, macOS 26.5.2;
  Apple clang 21.0.0, CMake 4.3.2 and Ninja 1.13.2.
- Ordinary Release is `-O3 -DNDEBUG` with `CREXX_VM_PROFILING=OFF`.
- Current in-tree runtime artifacts before the fresh PERF3-05 freeze are
  `e18c5076...` for `rxvm` (998,840 bytes) and `20ed3719...` for `rxbvm`
  (999,016 bytes). Stage A must rebuild and hash an isolated exact baseline;
  these paths alone are not retained authority.
- Five protected untracked lifecycle RXBIN files remain outside scope and must
  not be overwritten, deleted or staged.

Retained PERF3-01 timing was built from clean detached `3f43a0014` before the
accepted PERF3-02 compiler changes. It remains mechanism/ranking authority but
is not an exact current-product timing baseline for this activity. PERF3-05
must use the current C1abc+A1 source and current generated images.

## Entry evidence and falsifiable questions

- Current Base64 executes 46,723,864 VM instructions, only 1,694 allocations
  and 1,376 clear/reset/destroy operations; retained native samples place
  92-93% in `run()`. This makes it the primary semantic/layout discriminator.
- Sieve executes no bytecode calls and remains the mandatory zero-work/layout
  guard. The accepted D01 `rxbvm` regression was +5.728% on Sieve 50 and
  +9.181% on work-500 despite the selected frame policy not executing there.
- RexxCPS remains the required representative front-end/conversion lane.
  Richards and Towers protect the accepted C1abc gains and ownership-heavy
  paths from a layout-only regression.
- Apple hardware counters cannot resolve the D01 microarchitectural split.
  Mac evidence may select a supported build mechanism or justify a later
  cross-platform PoC; it cannot close the required Linux/Windows D01 matrix.

The panel answers:

1. How much Base64 time remains after benchmark-equivalent string/copy work is
   removed in a separately named semantic ceiling?
2. Does a supported whole-program compiler mode improve Base64 in both VMs
   without moving the Sieve zero-work guard or accepted Richards/Towers lanes
   beyond governance limits?
3. If compiler modes do not suffice, do static size/disassembly and paired
   results support hot/cold partitioning rather than global layout luck?
4. Only if a persistent front-end or instruction-footprint mechanism remains,
   can a private prepared representation beat current direct operands after
   including preparation, memory, lifecycle and late-load costs?

## Compared designs

### L0 — accepted current product

Ordinary profiling-off Release `rxvm` and `rxbvm`, canonical RXBIN and current
eager per-module `execution_image`. This is the correctness, timing, lifecycle,
RSS and artifact control.

### L1 — semantic Base64 ceiling

Use a separately named, exact-output diagnostic that removes proved generated
string/copy work while preserving the same RFC 4648 round trip, bounds and
checksum. This is not a benchmark replacement or production candidate. It
sets the maximum benefit that native layout cannot own.

### L2 — supported compiler modes

Compare isolated ordinary Release products with:

- CMake/Apple-Clang interprocedural optimization or the supported LTO form;
- representative instrumentation-based PGO with exact training provenance;
  and
- an unchanged-source rebuild/drift control.

The current CMake file contains commented target IPO hints but no supported
project option. A comparator must prove that the actual object-library VM core
and final executables participate; a flag accepted but optimized away is not a
candidate. Record build time, peak build memory, binary sections, symbols and
both VM results.

### L3 — hot/cold interpreter partition

Move genuinely cold signal/frame/OOM/diagnostic work outside the monolithic
flattened `run()` path or use a measured supported partitioning mechanism.
Compare source ordering, compiler outlining and narrow explicit helper forms.
The hot path may pay no new branch, lookup or allocation. Static text shrinkage
alone is insufficient: Base64 and Sieve paired results must identify the
mechanism in both VMs.

### L4 — canonical RXBIN plus private prepared representation

Keep `segment.binary` immutable and compare three plausible ownership forms:

1. eager per-module compact/decoded execution image during `rxvm_prepare()`;
2. lazy per-procedure materialization on first execution, with coherent
   late-load invalidation; and
3. a narrower hot-operand sidecar while retaining the current image.

The machine ceiling is a direct bound operand/handler load. Per-instruction
translation, search, allocation or representation checks fail the design gate.
Record startup/preparation, first-hit and steady-state cost, RSS, teardown,
late-load/plugin behavior, artifact identity and both VM modes. No private
representation can be adopted from Apple-only evidence; representative benefit
on at least one second architecture is required later under PERF3-08.

## Selection and regression gates

- Correct output and exact current-source/image provenance precede timing.
- Base64 is primary, but no form is selected from one workload. Include Sieve,
  RexxCPS, Richards and Towers in every candidate-verdict set, both VMs.
- Use at least one warmup and 12 paired balanced rounds, extending only under
  the governed noise rule. Keep the accepted product as a same-session drift
  control.
- Apply the 3% per-workload guard and keep lifecycle, RSS, artifact and build
  costs separate. A Sieve drift greater than 3% stops for explicit trade-off.
- Remove no outlier without an independently demonstrated fault.
- A compiler/flag result without a causal static or profile mechanism remains
  a control. A private stream without direct hot-path access remains rejected.
- Preserve every rejected comparator and its reason; do not delete replay
  ability when narrowing the panel.

## Work stages

### Stage A — freeze current product and semantic ceiling

- [x] Create an isolated exact C1abc+A1 ordinary Release baseline; record
      commit, accepted source diff, toolchain/cache, build state and hashes.
- [x] Build the exact current Base64, Sieve, RexxCPS, Richards and Towers
      RXAS/RXBIN set; prove expected outputs on both VMs.
- [x] Reconcile retained dynamic counts/native samples with current images and
      rerun only evidence invalidated by changed streams.
- [x] Implement and prove the separately named Base64 semantic ceiling without
      changing the canonical workload.
- [x] Record current binary/run text, symbols, preparation/lifecycle, RSS and
      artifact controls.

### Stage B — isolated compiler/layout screen

- [x] Build unchanged-source drift, supported LTO/IPO and representative PGO
      products in separate directories; reject ineffective or unreproducible
      configurations explicitly.
- [x] Run the bounded five-workload/both-VM correctness and paired screen.
- [x] Attribute any surviving result with build, section, disassembly and
      retained/native evidence before treating it as a mechanism.
- [x] Decide whether L3 hot/cold partition receives one disposable PoC.
- [x] Diagnose the reported VM-library link cost separately from runtime
      layout, including archive granularity, CMake propagation and exports.

### Stage C — architecture comparator only if justified

- [x] Compare L3 against L0/L2 with no new hot-path work.
- [x] Apply the L4 entry gate: do not open it because neither a persistent
      cross-VM instruction/front-end mechanism nor a new direct-load ceiling
      is demonstrated.
- [x] Record eager, lazy and narrow-sidecar disposition as deferred without an
      implementation comparator because the L4 entry gate is not met.

### Stage D — selection stop

- [x] Retain one compact checksum-closed evidence bundle referencing prior
      forensics rather than copying them.
- [x] Select, reject or defer every L0-L4 row with exact reasons.
- [x] Update the roadmap and report to Adrian.
- [x] Stop before a production architecture edit, PERF3-03 or push.

## Selection record

- Adrian accepted this disposition on 2026-08-01 and authorized progression to
  the already approved PERF3-03 evidence/design gate.
- Retain L0. Fresh and unchanged-source drift products are byte-identical and
  all exact workload/VM correctness cells pass.
- Retain L1 only as a semantic ceiling. The exact Base64 position/control form
  is 2.691x/2.959x faster and the arithmetic form 10.691x/10.354x faster for
  `rxvm`/`rxbvm`; these controls prove generated work, not native layout.
- Reject ThinLTO as a default despite 6.3-6.5% lower `__text`: it regresses
  Sieve `rxvm` 4.443% and Base64 `rxbvm` 11.351% with VM reversals.
- Reject merged and per-VM PGO. The representative profile improves Richards
  but regresses Sieve 33.7-40.4% and Base64 27.7-48.8%.
- Reject the L3 no-flatten PoC as runtime layout. It cuts focused build time
  18.4% and build RSS 9.8%, but regresses non-noisy `rxbvm` Sieve 3.639% and
  has no stable cross-VM representative gain.
- Do not open L4. Current eager private execution images already bind handlers
  and operands, while no stronger direct private representation ceiling is
  demonstrated by the supported-mode or L3 evidence.
- Carry VM-library link cleanup as separate build/API hygiene. The current Mac
  does not reproduce a large linker bottleneck: relevant clean-build links are
  61-71 ms. Export leakage is real (367 globals versus 16 intended) but export
  filtering leaves the median dylib link unchanged. The actionable findings
  are the `PUBLIC` propagation of implementation archives (35.095 ms versus
  21.706 ms for a dylib-only consumer link) and the static phase-API object
  pulling the 805,296-byte interpreter object even for create/destroy-only use.

Evidence: [`2026-08-01-perf3-05-compiler-layout-panel`](evidence/2026-08-01-perf3-05-compiler-layout-panel/).

## Resumption rule

PERF3-05 is closed. Reopen only for new evidence that demonstrates a stable
cross-VM runtime mechanism, or under separately approved PERF3-05-B1 build/API
hygiene. Preserve the retained baseline, LTO/IPO, PGO and hot/cold configuration
records. If source, compiler, power state or canonical input hashes change,
revalidate the affected comparison rather than silently reusing it.
