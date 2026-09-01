# PERF3-12B Mac scorecard replay commands

Commands were run from `/Users/adrian/CLionProjects/CREXX`.

## Clean product

```sh
git worktree add --detach SCRATCH/src 44d8b6a7ecd7800979b5db992c14bc7182aa89dd
cmake -S SCRATCH/src -B SCRATCH/build-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCREXX_VM_PROFILING=OFF
cmake --build SCRATCH/build-release --parallel 10 --target \
  crexx rxvm rxbvm rxc rxas rxlink rxdas \
  benchmark_awfy_sieve_opt_artifact \
  benchmark_awfy_permute_opt_artifact \
  benchmark_awfy_bounce_opt_artifact \
  benchmark_awfy_richards_opt_artifact \
  benchmark_base64_roundtrip_opt_artifact \
  benchmark_awfy_towers_opt_artifact \
  benchmark_rexxcps_levelb_opt_artifact
```

The exact scratch root retained in manifests and artifact identities is
`/private/tmp/crexx-perf3-12b-scorecard.IRzxpf`.

## Focused gate

The checked-in Release CTest tree passed the optimized seven-workload cells,
linked-artifact build and both matrix-driver self-tests: 10/10.

## Formal matrix

```sh
caffeinate -i SCRATCH/build-release/bin/crexx \
  SCRATCH/src/performance/tools/run_cross_runtime_matrix.crexx \
  --nokeep --args \
  --manifest performance/manifests/perf3-12b-current-mac-v1.txt \
  --output-dir EVIDENCE/timing/initial \
  --measurement timing --warmups 2 --runs 10
```

All 348 processes pass. Since every initial summary row says
`rerun_recommended=no`, no append manifest or append execution exists.

## Artifact tools

```sh
SCRATCH/build-release/bin/crexx \
  SCRATCH/src/performance/tools/inventory_performance_artifacts.crexx \
  --nokeep --args --manifest EVIDENCE/artifact-inputs-v1.txt \
  --output EVIDENCE/artifacts.csv

SCRATCH/build-release/bin/crexx \
  SCRATCH/src/performance/tools/summarize_perf2_artifacts.crexx \
  --nokeep --args \
  --manifest EVIDENCE/static/perf3-12b-artifact-summary-inputs-v1.txt \
  --output EVIDENCE/static/perf3-12b-artifact-summary.csv
```

Both tools also pass their Level B `--self-test` modes.

## Closeout QA

```sh
/usr/bin/cc -Icmake-build-debug/generated -Iassembler \
  -Icmake-build-debug/assembler -Iplatform -Iavl_tree -Iutf8 \
  -Ibinutils/include -I. -std=gnu90 -Wall -Wextra -Wconversion \
  -Wsign-conversion -fsyntax-only \
  assembler/rxas_flow.c assembler/rxas_flow_analysis.c \
  assembler/rxas_flow_batch.c assembler/rxas_flow_pass.c \
  assembler/rxas_flow_proof.c assembler/rxas_flow_ssa.c \
  assembler/rxas_flow_use.c binutils/rxopmeta.c

cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure

tools/asan-run.sh --build-dir cmake-build-debugasan --phase build \
  --build-target test_rxas_flow_graph --build-target test_rxop_metadata \
  --build-target rxas --build-jobs 4 --build-leaks off --no-live-tail
tools/asan-run.sh --build-dir cmake-build-debugasan --phase build \
  --build-target rxdas --build-jobs 4 --build-leaks off --no-live-tail
tools/asan-run.sh --build-dir cmake-build-debugasan --phase ctest \
  --regex '^(rxas_optimizer_metadata|rxas_flow_graph_contract|rxas_optimizer_semantic_batch_flow|rxas_optimizer_joined_key_private_local|rxas_optimizer_joined_key_private_local_noopt)$' \
  --leaks off --test-jobs 1 --no-live-tail
```

The retained first runner attempt proves why leak detection is disabled:
Apple AddressSanitizer aborts with `detect_leaks is not supported on this
platform`. The final five tests retain AddressSanitizer with
`ASAN_OPTIONS=detect_leaks=0`.

After the warning-only unsigned capability-mask correction, rebuilding the
clean scorecard tree's `rxas` target and comparing SHA-256 before/after gives
an exact match.
