# Performance workspace instructions

These instructions apply to the `performance/` subtree. The repository-root
`AGENTS.md` continues to apply everywhere.

## Read first

Before performance work, read only the sources relevant to the activity, but
always start with:

1. `performance/ROADMAP.md` for current status, ownership, notes and captured
   ideas;
2. `docs/planning/release-1/performance-programme-report-2026-07-15.md` for the
   programme terms, evidence and design gates; and
3. the relevant technical guide named in `performance/README.md`.

For Level B benchmark or runner edits, the root instruction requiring
`docs/ai-context/CREXX_LEVELB_AUTHORING.md` is mandatory.

## Workspace boundaries

- Keep programme status, idea triage, manifests and retained evidence here.
- Keep portable, correctness-gated language workloads in `tests/benchmarks/`.
- Keep focused cREXX/VM/compiler/library comparisons and microbenchmarks in
  `tests/performance/`.
- Keep user-facing profiler documentation in
  `docs/books/crexx_programming_guide/profiling.md`.
- Treat dated reports under `docs/planning/` as snapshots. Add live status and
  later observations to `performance/ROADMAP.md` or a new evidence bundle.

Do not create a second copy of a benchmark merely to retain results. Evidence
bundles point to the exact versioned workload and record the commit used.

## Roadmap discipline

- Every performance idea goes in the idea ledger in `ROADMAP.md` before or as
  it is investigated, even if it is speculative.
- Give an idea a stable ID. Record the hypothesis, affected surfaces, semantic
  risks, evidence needed and disposition. Do not silently delete rejected or
  negative ideas; mark the outcome and retain the reason.
- Update an activity's status and dated notes in the same change that starts,
  completes, pauses or invalidates it.
- `complete` means the activity's exit criterion in the programme report is
  met. A useful first slice remains `in progress`.
- Link related ideas to the `NR-*` activity they may inform. Capturing an idea
  does not approve a language, ISA, ABI or architecture change.

## Evidence discipline

Correctness is a prerequisite for timing. A retained evidence bundle must say:

- source commit, branch and dirty-worktree scope;
- host, operating system, CPU, toolchain, generator and build options;
- exact workload/image set, VM, optimized/unoptimized mode and lifecycle;
- whether source/TRACE metadata was retained or stripped;
- exact commands, warmups, recorded-run count and serial/parallel policy;
- raw benchmark-native metrics and output, kept distinct from harness/process
  elapsed time;
- correctness result and raw samples, including negative or noisy results; and
- the interpretation boundary: observation, inference, upper bound or claim.

Keep raw samples. Summary statistics never replace them. Run benchmark samples
serially unless an activity explicitly measures concurrency. Do not compare a
process-startup-inclusive measurement with a steady-state kernel measurement,
or retained metadata with stripped metadata, as if they were the same mode.

Canonical comparison workloads stay unchanged. Optimizer-resistance,
no-TRACE, opaque-input and similar diagnostics use separately named variants
and cannot silently replace the canonical score.

## Performance governance

`PERFORMANCE-GOVERNANCE.md` is the normative authority for portfolio
aggregation, formal sampling, uncertainty, regression budgets and release
claims. Apply these standing rules to future performance work:

- Keep the 12-item Tier A coverage portfolio distinct from the five-workload
  common CREXX/ooRexx/NetRexx aggregate: Sieve, Permute, Bounce, Richards and
  Base64.
- Treat RexxCPS as a first-class governed community lane in every
  multi-workload representative sampling set. Formal baselines, candidate
  verdicts, compiler/layout screens, diagnostic profile subsets and native
  PMU/sample subsets must include a RexxCPS result and its applicable qualified
  comparators. Formal timing, candidate-verdict and native-PMU cells use the
  canonical-default cREXX 2.2d command. Counts-only instrumentation may use a
  versioned bounded smoke form when canonical execution is disproportionate,
  but must label that form and link it to the retained canonical result. Keep
  RexxCPS separately reported from the common-five aggregate because cREXX and
  NetRexx are disclosed adaptations.
  A single-mechanism or single-workload experiment may omit RexxCPS only when
  it is explicitly labelled non-representative; record any technical
  inapplicability rather than silently dropping the row.
- Keep Regina RexxCPS-only and Java/native C as labelled controls. Exclude
  missing, failing, `not comparable` and materially adapted cells from common
  aggregates without imputation.
- For a canonical NetRexx common cell, require `options nobinary decimal` and
  NetRexx `Rexx` decimal arithmetic/state in the timed kernel. The generated
  Java and default HotSpot JIT are part of the normal NetRexx substrate; label
  `options binary` or primitive-Java numeric ports as controls and exclude them
  from Rexx aggregates. Disclose any necessary host-Java storage separately.
- Report `rxvm` and `rxbvm` separately. Keep throughput, lifecycle, peak RSS
  and artifact size in separate scorecards.
- Formal absolute baselines require two warmups and ten recorded serial samples
  per cell. Formal before/after decisions require at least one warmup per cell
  and 12 paired, balanced/interleaved recorded rounds.
- Run formal measurements on AC with low-power mode off and capture pre/post
  host, power, thermal and load state. Do not infer regressions from unmatched
  sessions without a same-session accepted-product drift control.
- Remove no outlier without an independently demonstrated fault. Follow the
  approved noise/rerun rules and retain inconclusive results honestly.
- Enforce the approved regression guards: 1% per common geometric mean, 3%
  per comparable Tier A workload, plus the separate lifecycle, RSS and artifact
  guards in `PERFORMANCE-GOVERNANCE.md`. A guard hit stops for Adrian's explicit
  trade-off decision.
- Any new or changed performance tool must be cREXX Level B. Read
  `docs/ai-context/CREXX_LEVELB_AUTHORING.md` before editing it; do not replace
  the Level B path with Python.
- Keep formal evidence compact: one final consolidated bundle, prior forensics
  referenced rather than copied, and no committed calibration, superseded
  scratch run or reproducible duplicate build output.

## Implementation gates

### Mandatory first Release verdict after a production performance edit

This is a hard sequencing gate, not a late closeout check. A selected
production implementation remains provisional and revertable until this gate
is reviewed:

1. Run only the minimum focused build and correctness checks needed to show the
   changed path is safe and correct enough to benchmark. Do not wait for full
   Debug CTest, sanitizer, install/package proof, cross-platform work, polished
   documentation, or loose-end cleanup.
2. Immediately freeze implementation work and build the ordinary,
   profiling-off Release product.
3. Run the smallest decisive end-to-end Release performance cells against
   already-retained valid baseline evidence. Audit and reuse the baseline; do
   not rerun it unless it is invalid or a bounded drift control is genuinely
   required.
4. Report the Release verdict to Adrian and stop for direction. Until Adrian
   accepts the result or selects the next design step, do not continue with
   representation tuning, harness improvements, follow-on PoCs, caches,
   overlays, broad portfolio/validation sweeps, documentation polish, or other
   completion work.
5. If the result is neutral, negative, materially noisy, or exposes an
   unrelated regression, treat rework or revert as live outcomes. Do not spend
   further time completing an implementation whose design has not survived its
   first Release performance gate.

### Approved closeout path

After Adrian accepts the Release verdict, keep closeout to the shortest path
needed for the agreed scope: remove disposable PoCs and obsolete production
code, rebuild the affected product, run focused checks plus the required broad
CTest, retain the decisive benchmark evidence, update the live roadmap, review
the diff, and commit when requested. Do not automatically add another 006
audit, rerun valid baselines, expand the benchmark portfolio, repeat isolated
harness measurements, rewrite historical reports, or add sanitizer,
cross-platform, install/package, alternate-layout, or overlay work unless the
selected scope, a failure, or Adrian explicitly requires it.

- Prefer a minimal reproducer and dynamic-count/profile evidence before
  changing a hot path.
- Before the first production-code performance edit for an activity, add an
  explicit design-selection section to its worklist or decision record. Start
  with the status quo and enumerate at least two plausible implementation
  approaches when two exist; if only one is viable, record why the alternatives
  were rejected before coding.
- Time-box lightweight prototypes in isolated builds before selecting the
  production approach. Compare the same exact inputs and ordinary Release
  binaries, and include steady-state benefit, startup/load cost, memory,
  teardown, late-load/plugin behavior, both VM modes, and effects on unrelated
  hot paths. A functionally complete first candidate is still a prototype until
  this comparison is retained.
- Define the machine-level ceiling for every new hot primitive before
  integration. If the intended operation is pointer/ID equality, a precomputed
  bit test, or a direct bound-target load, build and measure that exact inline
  control in the isolated harness. Allocation, graph traversal, name lookup,
  portable-reference binding or a search on the success path fails the design
  gate even when it improves substantially over the old implementation. Prove
  the primitive at control cost first, then measure the integrated opcode and
  finally the end-to-end workload.
- Any new runtime cache, index, allocation, or preparation pass must explicitly
  compare eager, lazy/on-demand, and narrower purpose-built forms where they are
  plausible. Record ownership, invalidation/rebuild, failure, and cross-platform
  costs for each option.
- Select the production design from the comparative evidence and retain both
  the chosen and rejected options with reasons. If a candidate exposes a new
  portfolio or lifecycle regression, pause production closeout and investigate
  it; do not let sunk implementation effort substitute for the selection gate.
- Optimize for the activity's stated performance objective, not for the
  smallest production-code, file-format, or subsystem blast radius. Treat
  implementation scope, compatibility, migration, and maintenance as measured
  decision costs; do not use them to exclude a plausibly faster architecture
  before it is compared. Architectural or format changes still require Adrian's
  explicit selection before production implementation.
- Confirm instrumented findings with paired, unprofiled Release wall-clock
  measurements; profiling elapsed time is not a benchmark result.
- Preserve existing ISA/ABI/signal/reference/debug contracts for P0 no-regrets
  work. Pause for Adrian before any language, serialized RXBIN, public ABI or
  architectural decision.
- Validate focused semantics first. Add wider portfolio, Debug/Release,
  sanitizer and cross-platform checks in proportion to the changed surface.
- Record workload-specific, neutral and negative outcomes; do not optimize a
  single headline score at the expense of the portfolio.
