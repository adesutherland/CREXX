# Performance diagnostic replay commands

Commands ran serially from the repository root. `CONTROL_BUILD` is an ordinary
profiling-off Release build of commit `b6a16dc3a`; `cmake-build-release` is the
candidate at `5bede182e`.

## Task launch

```sh
GF_CONTROL_DIR=CONTROL_BUILD/bin \
GF_ACCEPTED_IMAGE=$PWD/cmake-build-release/interpreter/test_gate_f_channel_benchmark.rxbin \
GF_CANDIDATE_IMAGE=$PWD/cmake-build-release/interpreter/test_gate_f_channel_benchmark.rxbin \
performance/evidence/2026-08-15-perf3-13-gate-f-f3c1-task-binding-cache-first-release-verdict/capture-task-cache-paired.zsh \
  OUTPUT_DIRECTORY
```

The command ran once at `timing/task` and twice unchanged at
`timing/task-append-1` and `timing/task-append-2`, producing 36 balanced pairs.
`summarize_task_36.crexx` reduced the concatenated retained samples.

## Ordinary single-thread portfolio

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-16-initial-concurrency-mac-closeout/manifest.txt \
  --output-dir OUTPUT_DIRECTORY --measurement timing --warmups 1 --runs 12
```

The initial command used `timing/single-thread`. The two governed appends used
the same command, `--warmups 0 --runs 12`, and output directories
`timing/single-thread-append-1` and `timing/single-thread-append-2`.

```sh
cmake-build-release/bin/crexx \
  performance/evidence/2026-08-16-initial-concurrency-mac-closeout/summarize_paired_36.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-16-initial-concurrency-mac-closeout/paired-summary-36.csv \
  performance/evidence/2026-08-16-initial-concurrency-mac-closeout/timing/single-thread/samples.csv \
  performance/evidence/2026-08-16-initial-concurrency-mac-closeout/timing/single-thread-append-1/samples.csv \
  performance/evidence/2026-08-16-initial-concurrency-mac-closeout/timing/single-thread-append-2/samples.csv
```

The unchanged Richards `rxtvm` confirmation used the two-cell confirmation
manifest with one warmup and 12 recorded balanced pairs, then
`summarize_paired.crexx`.

## Interpretation rule

The retained power log proves battery use during the complete timing window.
Do not quote this bundle as an AC baseline, release guard verdict or replacement
for the earlier accepted performance evidence.
