# PERF3-11 Stage 2 commands

Commands were run from `/Users/adrian/CLionProjects/CREXX` unless noted.

```sh
cmake --build cmake-build-debug --target test_rxas_flow_graph rxas --parallel 10
ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure \
  -R '^(rxas_flow_graph_contract|rxas_optimizer_.*|rxassignal.*|storage_identity_runtime_.*|redundant_itos_runtime_.*|signal_contract_runtime_.*|mc_decimal_full_tests|db_decimal_tests)$'

cmake --build cmake-build-release --target rxas --parallel 10
```

The focused matrix passed 113/113. The linked optimized-runtime fixture rebuilt
once in 168.50 seconds; the selected tests then completed normally.

Canonical image parity was checked from
`cmake-build-release/tests/benchmarks` so source/output identity matched the
build target:

```sh
../../bin/rxas -o benchmark_awfy_richards_opt benchmark_awfy_richards_opt
../../bin/rxas -o benchmark_awfy_towers_opt benchmark_awfy_towers_opt
../../bin/rxas -o benchmark_rexxcps_levelb_opt benchmark_rexxcps_levelb_opt
shasum -a 256 \
  benchmark_awfy_richards_opt.rxbin \
  benchmark_awfy_towers_opt.rxbin \
  benchmark_rexxcps_levelb_opt.rxbin
```

Deterministic graph counters came from Release `rxas -d`, with stdout/stderr
redirected to a separate `mktemp` log for each input and only `^PERF3
flow-graph` records reduced.

Assembler elapsed used the frozen Stage 0 `rxas` and current Release `rxas`
from the benchmark build directory. Each workload received two warmups per
binary and 30 balanced/interleaved records. Each record used `zsh/datetime`:

```sh
zmodload zsh/datetime
start_time=$EPOCHREALTIME
RXAS -o OUTPUT INPUT
end_time=$EPOCHREALTIME
awk -v s="$start_time" -v e="$end_time" \
  'BEGIN { printf "%.9f", e-s }'
```

Peak RSS used ten separately interleaved records per binary/workload:

```sh
/usr/bin/time -lp -o TIME_LOG RXAS -o OUTPUT INPUT
```

Host state:

```sh
uname -srm
sysctl -n hw.logicalcpu hw.memsize vm.loadavg
pmset -g batt
pmset -g custom | rg lowpowermode
pmset -g therm
```
