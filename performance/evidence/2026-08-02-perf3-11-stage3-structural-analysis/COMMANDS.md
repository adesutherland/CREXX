# PERF3-11 Stage 3 commands

Commands were run from `/Users/adrian/CLionProjects/CREXX` unless noted.

```sh
/usr/bin/cc \
  -Icmake-build-debug/generated -Iassembler -Icmake-build-debug/assembler \
  -Iplatform -Iavl_tree -Iutf8 -Ibinutils/include \
  -std=gnu90 -Wall -Wextra -Wconversion -Wsign-conversion \
  -fsyntax-only assembler/rxas_flow_analysis.c assembler/rxas_flow_graph.c

cmake --build cmake-build-debug --target test_rxas_flow_graph rxas \
  --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure \
  -R '^(rxas_flow_graph_contract|rxas_optimizer_.*|rxassignal.*|storage_identity_runtime_.*|redundant_itos_runtime_.*|signal_contract_runtime_.*|mc_decimal_full_tests|db_decimal_tests)$'

cmake --build cmake-build-release --target rxas --parallel 10
```

The final focused matrix passed 113/113. The linked optimized-runtime fixture
rebuilt once; the selected tests then completed normally.

Canonical image parity was run from
`cmake-build-release/tests/benchmarks` using the exact canonical basename:

```sh
../../bin/rxas -o benchmark_awfy_richards_opt benchmark_awfy_richards_opt
../../bin/rxas -o benchmark_awfy_towers_opt benchmark_awfy_towers_opt
../../bin/rxas -o benchmark_rexxcps_levelb_opt benchmark_rexxcps_levelb_opt
shasum -a 256 \
  benchmark_awfy_richards_opt.rxbin \
  benchmark_awfy_towers_opt.rxbin \
  benchmark_rexxcps_levelb_opt.rxbin
```

Structural counters came from one Release `rxas -d` invocation per canonical
RXAS input. Both stdout and stderr were redirected to separate `mktemp` logs;
only `^PERF3 flow-analysis ` rows were reduced into `analysis-metrics.csv`.

Assembler elapsed compared the frozen Gate 0 binary with the current ordinary
Release binary. Each workload received two warmups per binary and 30
balanced/interleaved records. Each record used `zsh/datetime`:

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

Host state was captured immediately before and after both measurement
sessions:

```sh
uname -srm
sysctl -n hw.logicalcpu hw.memsize vm.loadavg
pmset -g batt
pmset -g custom | rg lowpowermode
pmset -g therm
```

The initial eager session is retained because its Richards RSS result rejected
that integration. The final session measures the accepted demand-driven form.
