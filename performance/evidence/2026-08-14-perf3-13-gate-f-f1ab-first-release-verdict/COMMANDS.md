# Gate F F1a/F1b verdict and closeout commands

The full raw timing and investigation transcripts are retained under `timing/`
and `investigation/`. The accepted run used one warmup and twelve balanced
recorded rounds per cell. The exact per-process invocations and outputs are in
those logs.

The final focused Debug command was:

```sh
ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure \
  -R '^(gate_f_channel_contract|rxas_optimizer_metadata|nr14_frozen_parse_contract|nr15_native_stem_contract|nr21_fixed_call_contract|gate_f_channel_local_rxbvm|gate_f_channel_local_rxtvm|rxvmmemory_allocator|rxvmworker_lifecycle|rxvmactive_isolation|program_generation_control-(rxvml|rxbvml|rxtvml)|persistent_worker_executor-(rxvml|rxbvml|rxtvml)(-doorbell-stress|-sparse-owner|-sparse-owner-stress|-sparse-progress|-native-return)?)$'
```

The concurrency-sensitive subset was then repeated one hundred times per test
on both Debug and Release:

```sh
ctest --test-dir cmake-build-debug --output-on-failure --parallel 10 \
  --repeat until-fail:100 \
  -R '^(gate_f_channel_local_(rxbvm|rxtvm)|persistent_worker_executor-(rxbvml|rxtvml)(-doorbell-stress|-sparse-owner-stress|-sparse-progress)?)$'

ctest --test-dir cmake-build-release --output-on-failure --parallel 10 \
  --repeat until-fail:100 \
  -R '^(gate_f_channel_local_(rxbvm|rxtvm)|persistent_worker_executor-(rxbvml|rxtvml)(-doorbell-stress|-sparse-owner-stress|-sparse-progress)?)$'
```

The final ordinary profiling-off Release build and focused command were:

```sh
cmake --build cmake-build-release --parallel 10

ctest --test-dir cmake-build-release --parallel 5 --output-on-failure \
  -R '^(gate_f_channel_contract|gate_f_channel_local_rxbvm|gate_f_channel_local_rxtvm|persistent_worker_executor-(rxbvml|rxtvml))$'
```

Channel correctness was rerun directly on both products:

```sh
cd cmake-build-release/interpreter
../bin/rxbvm test_gate_f_channel_benchmark
../bin/rxtvm test_gate_f_channel_benchmark
```

The final broad Debug run was:

```sh
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```

It passed 2,086/2,086 directly; no serial recovery command was required in the
post-audit run.

The sanitizer tree was refreshed, then every changed focused target was built
and the ownership/lifecycle matrix was run serially through the project runner:

```sh
cmake -S . -B cmake-build-debugasan

tools/asan-run.sh --phase build --build-leaks off --build-jobs 10 \
  --build-target test_gate_f_channel_local_bin \
  --build-target persistent_worker_executor-rxbvml \
  --build-target persistent_worker_executor-rxtvml \
  --build-target test_rxvmmemory --build-target test_rxvmworker

tools/asan-run.sh --phase ctest --leaks off --test-jobs 1 \
  --regex '^(gate_f_channel_local_(rxbvm|rxtvm)|rxvmmemory_allocator|rxvmworker_lifecycle|persistent_worker_executor-(rxbvml|rxtvml)(-doorbell-stress|-sparse-owner|-sparse-owner-stress|-sparse-progress|-native-return)?)$'
```

Leak detection was disabled only after the same runner proved that this macOS
ASan runtime aborts generated build tools with `detect_leaks is not supported
on this platform`. That failed proof is retained alongside the successful ASan
logs.

The post-audit performance verdict and unchanged confirmation use the retained
executable script with separate output directories:

```sh
performance/evidence/2026-08-14-perf3-13-gate-f-f1ab-first-release-verdict/capture-post-audit.zsh \
  /tmp/crexx-f1ab-post-audit-verdict

performance/evidence/2026-08-14-perf3-13-gate-f-f1ab-first-release-verdict/capture-post-audit.zsh \
  /tmp/crexx-f1ab-post-audit-confirm
```

Both complete raw transcripts and CSV sample sets are retained under
`timing-post-audit/`; see `POST-AUDIT-VERDICT.txt` for the separate and combined
paired results.
