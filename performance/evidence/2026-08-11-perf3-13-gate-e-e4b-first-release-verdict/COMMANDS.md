# E4b retained commands

Commands ran from `/Users/adrian/CLionProjects/CREXX` unless noted.

## Frozen controls and candidate build

```sh
control_dir=/tmp/crexx-e4b-control-99753ba54427-20260811
mkdir -p "$control_dir"
cp cmake-build-release/bin/rxbvm "$control_dir/rxbvm-e4a-control"
cp cmake-build-release/bin/rxtvm "$control_dir/rxtvm-e4a-control"

cmake --build cmake-build-release \
  --target rxbvm rxtvm program_generation_control-rxvml \
  program_generation_control-rxbvml program_generation_control-rxtvml \
  benchmark_awfy_sieve_opt benchmark_rexxcps_levelb_opt library \
  --parallel 10
```

The Release cache recorded `CMAKE_BUILD_TYPE=Release`,
`CREXX_VM_PROFILING=OFF` and `CREXX_VM_HANDLER_PANEL=profile-20`.

## Focused Release correctness

```sh
ctest --test-dir cmake-build-release \
  -R '^program_generation_control-(rxvml|rxbvml|rxtvml)$' \
  --output-on-failure
```

## Primary timing matrix

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-11-perf3-13-gate-e-e4b-first-release-verdict/manifest.txt \
  --output-dir performance/evidence/2026-08-11-perf3-13-gate-e-e4b-first-release-verdict/timing \
  --measurement timing --warmups 1 --runs 12
```

The maintained E3b-P1 Level B paired reducer has generic statistics but names
its two expected variants. The retained raw samples were not changed; a
temporary compatibility copy changed only those two variant labels before
reduction:

```sh
compat_samples=$(mktemp -t crexx-e4b-paired-samples.XXXXXX)
sed -e 's/e4a-control/e3a-29ef1975e/g' \
    -e 's/e4b-candidate/e3b-p1-branch-free/g' \
  performance/evidence/2026-08-11-perf3-13-gate-e-e4b-first-release-verdict/timing/samples.csv \
  > "$compat_samples"

cmake-build-release/bin/crexx \
  performance/evidence/2026-08-10-perf3-13-gate-e-e3b-p1-first-release-verdict/summarize_paired.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-11-perf3-13-gate-e-e4b-first-release-verdict/paired-summary.csv \
  "$compat_samples"
```

## Mechanical noise append

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-11-perf3-13-gate-e-e4b-first-release-verdict/noise-append-manifest.txt \
  --output-dir performance/evidence/2026-08-11-perf3-13-gate-e-e4b-first-release-verdict/noise-append \
  --measurement timing --warmups 0 --runs 12
```

Reduction used the same temporary-label procedure with
`noise-append/samples.csv` as input and
`noise-append-paired-summary.csv` as output.

## Lifecycle RSS

The first invocation omitted the runner-required explicit zero warmups and is
retained as `rss-runner-attempt1.*`. The corrected command was:

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-11-perf3-13-gate-e-e4b-first-release-verdict/lifecycle-manifest.txt \
  --output-dir performance/evidence/2026-08-11-perf3-13-gate-e-e4b-first-release-verdict/rss \
  --measurement rss --warmups 0 --runs 12
```

## Host and binary inspection

```sh
uname -a
sysctl -n machdep.cpu.brand_string
sysctl -n hw.logicalcpu
pmset -g batt
pmset -g custom
pmset -g therm
uptime

shasum -a 256 \
  /tmp/crexx-e4b-control-99753ba54427-20260811/rxbvm-e4a-control \
  /tmp/crexx-e4b-control-99753ba54427-20260811/rxtvm-e4a-control \
  cmake-build-release/bin/rxbvm cmake-build-release/bin/rxtvm
size -m VM_PATH
```

## Post-acceptance Mac closeout

```sh
tools/asan-run.sh --build-dir cmake-build-debug --phase build \
  --build-target program_generation_control-rxvml \
  --build-target program_generation_control-rxbvml \
  --build-target program_generation_control-rxtvml

tools/asan-run.sh --build-dir cmake-build-debug --phase ctest \
  --regex '^(linked_opt_runtime_artifacts_build|rxvmworker_lifecycle|rxasdispatchcontract-rxbvm|rxasdispatchcontract-rxtvm|reentrancy_check|program_generation_control-rxvml|program_generation_control-rxbvml|program_generation_control-rxtvml|ts_loadmodule_noopt|ts_loadmodule_opt|rxas_optimizer_barrier_metaloadmodule)$' \
  --test-jobs 1

tools/asan-run.sh --phase build --build-leaks off \
  --build-target program_generation_control-rxvml \
  --build-target program_generation_control-rxbvml \
  --build-target program_generation_control-rxtvml

tools/asan-run.sh --phase ctest \
  --regex '^program_generation_control-(rxvml|rxbvml|rxtvml)$' \
  --leaks off --test-jobs 1

cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug \
  --parallel 30 --output-on-failure

cmake --build cmake-build-release --parallel 10
ctest --test-dir cmake-build-release \
  -R '^(linked_opt_runtime_artifacts_build|rxvmworker_lifecycle|rxasdispatchcontract-rxbvm|rxasdispatchcontract-rxtvm|reentrancy_check|program_generation_control-rxvml|program_generation_control-rxbvml|program_generation_control-rxtvml|ts_loadmodule_noopt|ts_loadmodule_opt|rxas_optimizer_barrier_metaloadmodule)$' \
  --output-on-failure

shasum -a 256 cmake-build-release/bin/rxbvm \
  cmake-build-release/bin/rxtvm
```

The sanitizer runner retained its detailed logs at
`cmake-build-debugasan/asan-logs/20260811-094535-build` and
`cmake-build-debugasan/asan-logs/20260811-094905-ctest`.
