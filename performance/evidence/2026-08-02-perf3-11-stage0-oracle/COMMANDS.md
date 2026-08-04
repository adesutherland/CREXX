# PERF3-11 Stage 0 commands

Commands were run from `/Users/adrian/CLionProjects/CREXX`.

```sh
git status --short --branch
git log -2 --format='%H %s'
rg -n '^(CMAKE_BUILD_TYPE|CREXX_VM_PROFILING|CMAKE_GENERATOR):' \
  cmake-build-debug/CMakeCache.txt cmake-build-release/CMakeCache.txt

cmake --build cmake-build-debug \
  --target rxas rxc rxlink rxvm rxbvm --parallel 10
cmake --build cmake-build-release \
  --target rxas rxc rxlink rxvm rxbvm --parallel 10
cmake --build cmake-build-release --parallel 1 --target \
  tests/benchmarks/benchmark_awfy_richards_opt.rxbin \
  tests/benchmarks/benchmark_awfy_towers_opt.rxbin \
  tests/benchmarks/benchmark_rexxcps_levelb_opt.rxbin

ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure \
  -R '^(rxassignal.*|rxas_optimizer_(barrier_signal|storage_identity_flow|redundant_itos_flow|redundant_itos_flow_noopt)|storage_identity_runtime_.*|redundant_itos_runtime_.*)$'
```

The focused CTest command passed 61/61.  Its three-minute fixture setup rebuilt
the generated linked runtime products; the selected tests themselves then
completed normally.

The frozen Release `rxas` and inputs were copied to
`/tmp/crexx-perf3-11-stage0.35IBzf`.  Each workload received two warmups and
ten serial records using this equivalent high-resolution zsh pattern:

```sh
zmodload zsh/datetime
start=$EPOCHREALTIME
/tmp/crexx-perf3-11-stage0.35IBzf/base-binaries/rxas \
  -o /tmp/crexx-perf3-11-stage0.35IBzf/output.rxbin INPUT.rxas
end=$EPOCHREALTIME
awk -v s=$start -v e=$end 'BEGIN { printf "%.9f", e-s }'
```

Peak RSS was captured separately for each of the same inputs with ten serial
records:

```sh
/usr/bin/time -lp -a -o TIMING.txt \
  /tmp/crexx-perf3-11-stage0.35IBzf/base-binaries/rxas \
  -o OUTPUT.rxbin INPUT.rxas
```

Deterministic procedure metrics came from `rxas -d`; full verbose logs remain
in the temporary oracle directory and were reduced to
[`procedure-summary.csv`](procedure-summary.csv) for retained evidence.

