# F1g-D qualification commands

Commands ran from the repository root on `develop`. Retained logs, raw samples
and capture manifests are the output authority.

```sh
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure

cmake --build cmake-build-release --parallel 10
ctest --test-dir cmake-build-release --parallel 20 \
  --output-on-failure -L gate_f

tools/asan-run.sh --phase build --build-jobs 4 --build-leaks off
tools/asan-run.sh --phase ctest --test-jobs 8 --leaks off \
  --regex '^(gate_f_(levelg|taskwork_import|task_method_import)_(rxbvm|rxtvm)_(noopt|opt)|testConcurrency_(noopt|opt)|ts_http_(pooled|streaming|crexx_rag|codec|policy|tls_live)_(rxbvm|rxtvm)_(noopt|opt))$'
```

After the final RexxDoc and public HTTP diagnostic correction, the affected
targets and seven concurrency/streaming cells were rebuilt and rerun in Debug,
Release and Apple AddressSanitizer configurations. The retained
`qa/*-final-docfix-*.log` files are the command and result authority.

The final source audit then made compressed framing fail closed on bytes after
the final DEFLATE block and added `LINKED_ALL_MODES` to the codec matrix. The
affected codec target and all four runtime/mode cells were rebuilt and rerun in
Debug, Release and Apple AddressSanitizer configurations; retained
`qa/*-final-codec-*.log` files are the command and result authority.

The paired task command used the exact committed F1g-C products and the
byte-identical sealed benchmark image:

```sh
GF_CONTROL_DIR=/tmp/crexx-gate-f-f1g-d-control-cd2453e35 \
GF_ACCEPTED_IMAGE=$PWD/cmake-build-release/interpreter/test_gate_f_channel_benchmark.rxbin \
GF_CANDIDATE_IMAGE=$PWD/cmake-build-release/interpreter/test_gate_f_channel_benchmark.rxbin \
caffeinate -i \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1g-d-streaming-integration-first-release-verdict/capture-task-paired.zsh \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1g-d-streaming-integration-first-release-verdict/timing/task-launch-final

cmake-build-release/bin/crexx \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1g-d-streaming-integration-first-release-verdict/summarize_task_paired.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1g-d-streaming-integration-first-release-verdict/timing/task-launch-final/summary.csv \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1g-d-streaming-integration-first-release-verdict/timing/task-launch-final/samples.csv
```

The exact-final ordinary-path verdict used:

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-15-perf3-13-gate-f-f1g-d-streaming-integration-first-release-verdict/manifest.txt \
  --output-dir performance/evidence/2026-08-15-perf3-13-gate-f-f1g-d-streaming-integration-first-release-verdict/timing/single-thread-final \
  --measurement timing --warmups 1 --runs 12

cmake-build-release/bin/crexx \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1g-d-streaming-integration-first-release-verdict/summarize_paired.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1g-d-streaming-integration-first-release-verdict/paired-summary-final.csv \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1g-d-streaming-integration-first-release-verdict/timing/single-thread-final/samples.csv
```

The initial adverse task runs and unchanged confirmation are retained, as are
the two rework runs and their 24-pair combined Level B reduction. No sample was
deleted or filtered.
