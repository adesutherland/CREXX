# PERF3-11 Stage 4 commands

Commands were run from `/Users/adrian/CLionProjects/CREXX` unless noted.

```sh
/usr/bin/cc \
  -Icmake-build-debug/generated -Iassembler -Icmake-build-debug/assembler \
  -Iplatform -Iavl_tree -Iutf8 -Ibinutils/include \
  -std=gnu90 -Wall -Wextra -Wconversion -Wsign-conversion \
  -fsyntax-only assembler/rxas_flow_signal.c \
  assembler/rxas_flow_analysis.c assembler/rxas_flow_graph.c

cmake --build cmake-build-debug \
  --target test_rxas_flow_graph test_rxop_metadata rxas --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure \
  -R '^(rxas_flow_graph_contract|rxas_optimizer_.*|rxassignal.*|storage_identity_runtime_.*|redundant_itos_runtime_.*|signal_contract_runtime_.*|mc_decimal_full_tests|db_decimal_tests)$'

cmake --build cmake-build-release --target rxas --parallel 10
```

The focused matrix passed 113/113.

Canonical image parity was run from
`cmake-build-release/tests/benchmarks`:

```sh
../../bin/rxas -o benchmark_awfy_richards_opt benchmark_awfy_richards_opt
../../bin/rxas -o benchmark_awfy_towers_opt benchmark_awfy_towers_opt
../../bin/rxas -o benchmark_rexxcps_levelb_opt benchmark_rexxcps_levelb_opt
shasum -a 256 \
  benchmark_awfy_richards_opt.rxbin \
  benchmark_awfy_towers_opt.rxbin \
  benchmark_rexxcps_levelb_opt.rxbin
```

One Release `rxas -d` invocation per canonical RXAS input supplied the sparse
analysis counters. Both stdout and stderr were redirected to separate `/tmp`
logs; only `^PERF3 flow-signal-analysis ` rows were aggregated. No disabled
analysis row was present.

Assembler elapsed compared the frozen Gate 0 binary with the current ordinary
Release binary. Each workload received two warmups per binary and 30
balanced/interleaved records. Each record used `zsh/datetime` around one
ordinary `rxas -o OUTPUT INPUT` invocation. Peak RSS used ten separately
interleaved records per binary/workload with `/usr/bin/time -lp`.

Host state was captured immediately before and after the measurement session:

```sh
uname -srm
sysctl -n hw.logicalcpu hw.memsize vm.loadavg
pmset -g batt
pmset -g custom | rg lowpowermode
pmset -g therm
```
