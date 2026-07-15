# cREXX performance workspace

This is the operational home for the cREXX performance programme. It keeps the
live roadmap, idea backlog, portfolio contract and retained evidence together
without turning test directories into a planning system.

The programme charter and 2026-07-15 evidence review remain in
[`docs/planning/release-1/performance-programme-report-2026-07-15.md`](../docs/planning/release-1/performance-programme-report-2026-07-15.md).
That dated report defines the `NR-*` activities and their exit criteria.
[`ROADMAP.md`](ROADMAP.md) is the live status overlay and the one place to
capture ideas that otherwise risk being lost.

## Directory map

| Location | Purpose |
| --- | --- |
| `performance/ROADMAP.md` | P0/P1 status, work notes, decisions and idea ledger |
| `performance/portfolio/manifest.md` | Versioned seed workload and measurement contract |
| `performance/portfolio/cross-runtime-plan.md` | Coverage targets and ooRexx/Regina/NetRexx/Java execution matrix |
| `performance/evidence/benchmark-median-summary.md` | Master per-date/run median comparison with explicit exclusions and comparability markers |
| `performance/evidence/` | Dated provenance, commands, raw samples and summaries |
| `performance/tools/run_cross_runtime.crexx` | Level B serial capture tool for one workload/runtime cell |
| `performance/tools/run_lifecycle.crexx` | Level B compile/translate and cold load-to-first-result capture across the three portfolio runtimes |
| `performance/capability-gaps.md` | Audited missing surfaces and candidates uncovered by portfolio ports |
| `tests/benchmarks/` | Portable, correctness-gated language workloads and runner |
| `tests/performance/` | Focused internal microbenchmarks and implementation comparisons |
| `docs/books/crexx_programming_guide/profiling.md` | Supported VM profiling and RXSEQ workflow |

Add future automation under `performance/tools/` only when it coordinates more
than one existing benchmark/profiler tool. Test sources remain under `tests/`.

`tools/run_cross_runtime.crexx` is the Level B cREXX orchestration layer for a
single workload/runtime cell. It executes serial process warmups and recorded
runs, requires an observable correctness string, retains stdout/stderr for
every sample, keeps process elapsed time separate from an optional
benchmark-native metric, and writes the exact argv and cREXX version to
`manifest.json`.

`tools/run_lifecycle.crexx` is also Level B cREXX. It keeps lifecycle phases
outside the steady-state aggregate and emits one CSV row per runtime, phase and
sequence. The final phase is named `load_first_result` because the public CLIs
do not expose a consistent loaded-but-not-executed boundary.

Run it through the Release driver, placing the workload command after `--`:

```bash
cmake-build-release/bin/crexx performance/tools/run_cross_runtime.crexx \
  --nokeep --args \
  --workload sieve --runtime oorexx --warmups 1 --runs 3 \
  --expect "PASS: AWFY Sieve Classic port" \
  --output-dir performance/evidence/sieve/oorexx/pilot -- \
  /path/to/rexx tests/benchmarks/cross-runtime/classic/awfy_sieve.rex 50
```

## Working loop

1. Select or capture an item in `ROADMAP.md`; state the question and exit
   criterion.
2. Establish correctness and a dated baseline using the portfolio manifest.
3. Collect the least instrumentation needed to explain the cost.
4. Make one bounded change or prototype.
5. Repeat the same correctness and unprofiled measurements, then update the
   roadmap with the result, including a neutral or negative result.

The first active slice is `NR-01`: the existing five language workloads are a
seed portfolio, the runner can retain serial samples, and the first retained
bundle is under `performance/evidence/2026-07-15-seed-portfolio/`. NR-01 stays
in progress until the portfolio and separate startup/steady-state reporting
meet the charter's full exit criterion.

Cross-runtime work is deliberately staged rather than deferred to one final
comparison: NR-01 fills the workload coverage matrix; NR-02 ports, qualifies
and runs the selected portfolio on CREXX, ooRexx and NetRexx (starting with
RexxCPS), with Regina limited to RexxCPS; and NR-10 records formal same-host
results as each workload becomes comparison-ready. See
`portfolio/cross-runtime-plan.md`.

## Technical pointers

- Compiler/emitter and register allocation:
  `docs/ai-context/CREXX_ARCHITECTURE.md` and
  `compiler/docs/emitter_architecture.md`
- Language argument semantics:
  `docs/books/crexx_language_reference/procedures_and_arguments.md`
- Assembler optimisation: `docs/ai-context/RXAS_ASSEMBLER.md`
- VM calls, signals and execution: `docs/ai-context/RXVM_INTERPRETER.md`
- Level B authoring: `docs/ai-context/CREXX_LEVELB_AUTHORING.md`
- Existing benchmark use and provenance: `tests/benchmarks/README.md`
