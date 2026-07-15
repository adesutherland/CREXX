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

## Implementation gates

- Prefer a minimal reproducer and dynamic-count/profile evidence before
  changing a hot path.
- Confirm instrumented findings with paired, unprofiled Release wall-clock
  measurements; profiling elapsed time is not a benchmark result.
- Preserve existing ISA/ABI/signal/reference/debug contracts for P0 no-regrets
  work. Pause for Adrian before any language, serialized RXBIN, public ABI or
  architectural decision.
- Validate focused semantics first. Add wider portfolio, Debug/Release,
  sanitizer and cross-platform checks in proportion to the changed surface.
- Record workload-specific, neutral and negative outcomes; do not optimize a
  single headline score at the expense of the portfolio.
