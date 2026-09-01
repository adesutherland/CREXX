# PERF3-11 Stage 6 commands

Commands were run from `/Users/adrian/CLionProjects/CREXX` unless noted.

## Build and focused validation

```sh
/usr/bin/cc \
  -Icmake-build-debug/generated -Iassembler -Icmake-build-debug/assembler \
  -Iplatform -Iavl_tree -Iutf8 -Ibinutils/include \
  -std=gnu90 -Wall -Wextra -Wconversion -Wsign-conversion \
  -fsyntax-only assembler/rxas_flow_proof.c assembler/rxas_flow_ssa.c \
  assembler/rxas_flow_signal.c assembler/rxas_flow_analysis.c \
  assembler/rxas_flow_graph.c binutils/rxopmeta.c

cmake --build cmake-build-debug \
  --target test_rxas_flow_graph test_rxop_metadata rxas --parallel 10

ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure \
  -R '^(rxas_linked_artifact_fixture|inline_summary_version_fallback_binary_opt|rxop_metadata_contract|rxas_flow_graph_contract|rxas_optimizer_redundant_itos_flow|rxas_optimizer_redundant_itos_flow_noopt|redundant_itos_runtime_rxvm_noopt|redundant_itos_runtime_rxvm_opt|redundant_itos_runtime_rxbvm_noopt|redundant_itos_runtime_rxbvm_opt)$'
```

The focused matrix passed 10/10 after the call-window correction.

## Broad Debug gate

```sh
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```

The first run passed 1,986/1,987 and exposed the range-call argument defect.
After the SSA correction the complete rerun passed 1,987/1,987 in 231.62 s.

## Release image and proof diagnostic

```sh
cmake --build cmake-build-release --target rxas --parallel 10

cmake-build-release/bin/rxas \
  -o /tmp/perf3-stage6-current \
  cmake-build-release/tests/benchmarks/benchmark_rexxcps_levelb_opt
shasum -a 256 /tmp/perf3-stage6-current.rxbin

/usr/bin/time -lp cmake-build-release/bin/rxas -d \
  -o /tmp/perf3-stage6-proof \
  cmake-build-release/tests/benchmarks/benchmark_rexxcps_levelb_opt \
  >/tmp/perf3-stage6-proof.log 2>/tmp/perf3-stage6-proof-time.log
```

Diagnostics were redirected because `rxas -d` is a mandatory temp-log case.
The accepted image hash is
`beeba2fefc0496e2f17dbb2ee48619c0f80ae69ed5f3aaa41f4ffe6c4fa40467`.

## First ordinary Release verdict

The retained Stage 5 image and current Stage 6 image were executed with the
same current binaries.  The Level B runner consumed this four-cell manifest:

```text
rexxcps-stage5-rxvm|RexxCPS|crexx|rxvm|Stage5-old-solver-21-ITOS|A|no|no|1000-clauses|PASS: RexxCPS 2.2d cREXX port|Performance:|RXVM;STAGE5_RXBIN;LIBRARY;-a
rexxcps-stage6-rxvm|RexxCPS|crexx|rxvm|Stage6-new-proof-19-ITOS|A|no|no|1000-clauses|PASS: RexxCPS 2.2d cREXX port|Performance:|RXVM;STAGE6_RXBIN;LIBRARY;-a
rexxcps-stage5-rxbvm|RexxCPS|crexx|rxbvm|Stage5-old-solver-21-ITOS|A|no|no|1000-clauses|PASS: RexxCPS 2.2d cREXX port|Performance:|RXBVM;STAGE5_RXBIN;LIBRARY;-a
rexxcps-stage6-rxbvm|RexxCPS|crexx|rxbvm|Stage6-new-proof-19-ITOS|A|no|no|1000-clauses|PASS: RexxCPS 2.2d cREXX port|Performance:|RXBVM;STAGE6_RXBIN;LIBRARY;-a
```

```sh
cmake-build-release/bin/rxvm \
  performance/tools/run_cross_runtime_matrix.rxbin \
  --manifest MANIFEST --output-dir OUTPUT --warmups 1 --runs 12 --balanced
```

The retained `first-verdict-samples.csv` and
`first-verdict-summary.csv` are the unmodified runner outputs.
