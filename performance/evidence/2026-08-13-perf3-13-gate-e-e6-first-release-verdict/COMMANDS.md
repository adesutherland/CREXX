# E6 first Release verdict commands

The candidate identities and build configuration are recorded in
`ARTIFACTS.txt`. Adrian confirmed the host was clear before the formal run.

```sh
caffeinate -i /private/tmp/crexx-e6-candidate-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-13-perf3-13-gate-e-e6-first-release-verdict/reclaim-scale-manifest.txt \
  --output-dir performance/evidence/2026-08-13-perf3-13-gate-e-e6-first-release-verdict/timing \
  --measurement timing --warmups 1 --runs 12

/private/tmp/crexx-e6-candidate-release/bin/crexx \
  performance/evidence/2026-08-13-perf3-13-gate-e-e6-first-release-verdict/summarize_selection.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-13-perf3-13-gate-e-e6-first-release-verdict/selection-summary.csv \
  performance/evidence/2026-08-13-perf3-13-gate-e-e6-first-release-verdict/timing/samples.csv \
  performance/evidence/2026-08-13-perf3-13-gate-e-e6-first-release-verdict/timing/outputs.csv
```

The guard hit required the approved interactive stop. C2 and external RSS
commands were deliberately not run.

Adrian then approved an exact isolated confirmation on the still-clear host:

```sh
caffeinate -i /private/tmp/crexx-e6-candidate-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-13-perf3-13-gate-e-e6-first-release-verdict/confirmation-rxbvml-w4-manifest.txt \
  --output-dir performance/evidence/2026-08-13-perf3-13-gate-e-e6-first-release-verdict/confirmation-rxbvml-w4 \
  --measurement timing --warmups 1 --runs 12
```

The original adverse result did not reproduce. Selection remained stopped at
that point before the C2 and external RSS panel.

Adrian then approved the remaining panel:

```sh
caffeinate -i /private/tmp/crexx-e6-candidate-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-13-perf3-13-gate-e-e6-first-release-verdict/remote-free-manifest.txt \
  --output-dir performance/evidence/2026-08-13-perf3-13-gate-e-e6-first-release-verdict/remote-free-timing \
  --measurement timing --warmups 1 --runs 12

caffeinate -i /private/tmp/crexx-e6-candidate-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-13-perf3-13-gate-e-e6-first-release-verdict/reclaim-scale-manifest.txt \
  --output-dir performance/evidence/2026-08-13-perf3-13-gate-e-e6-first-release-verdict/rss \
  --measurement rss --warmups 0 --runs 4

caffeinate -i /private/tmp/crexx-e6-candidate-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-13-perf3-13-gate-e-e6-first-release-verdict/remote-free-manifest.txt \
  --output-dir performance/evidence/2026-08-13-perf3-13-gate-e-e6-first-release-verdict/remote-free-rss \
  --measurement rss --warmups 0 --runs 4
```

## Accepted C0 closeout

After Adrian selected C0, C1 and C2 were removed and the selected form was
rebuilt in fresh trees. The final focused CTest regex was:

```sh
ctest --test-dir BUILD_DIR --parallel 10 --output-on-failure \
  -R '^(rxvmmemory_allocator|rxvmworker_lifecycle|rxvmactive_isolation|reentrancy_check|program_generation_control-(rxvml|rxbvml|rxtvml)|persistent_worker_executor-(rxvml|rxbvml|rxtvml)(-.*)?)$'
```

Final sanitizer validation used the repository runner on Apple with leak
detection disabled and AddressSanitizer enabled:

```sh
tools/asan-run.sh \
  --build-dir /private/tmp/crexx-e6-c0-closeout-asan \
  --phase ctest \
  --regex '^(rxvmmemory_allocator|rxvmworker_lifecycle|rxvmactive_isolation|reentrancy_check|program_generation_control-(rxvml|rxbvml|rxtvml)|persistent_worker_executor-(rxvml|rxbvml|rxtvml)(-.*)?)$' \
  --leaks off --test-jobs 1
```

The final broad Debug command was:

```sh
ctest --test-dir /private/tmp/crexx-e6-c0-closeout-debug \
  --parallel 30 --output-on-failure
```
