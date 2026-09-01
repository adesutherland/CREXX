# PERF3-12B B5 replay commands

Commands were run from `/Users/adrian/CLionProjects/CREXX`.

## Focused correctness

```sh
ctest --test-dir BUILD --output-on-failure \
  -R '^(rxas_optimizer_metadata|rxas_flow_graph_contract|rxas_optimizer_semantic_batch_flow|rxas_optimizer_joined_key_private_local|rxas_optimizer_joined_key_private_local_noopt)$'
```

The expression passed 5/5 against both `cmake-build-debug` and the ordinary
profiling-off `cmake-build-release` tree.  The focused native-stem/runtime
selector expression passed 19/19, followed by dual-VM smoke execution.

## Exact production image

```sh
cmake-build-release/bin/rxas \
  -o OUTPUT/rexxcps-production.rxbin \
  performance/evidence/2026-08-04-perf3-12b-b4-comparative-panel/artifacts/s0/rexxcps.rxas
```

The output option precedes the source operand because RXAS stops option parsing
at the source file.

## First Release matrix

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest MANIFEST --output-dir OUTPUT/timing \
  --measurement timing --warmups 1 --runs 6
```

The temporary four-cell manifest used the retained B4 S0 RXBIN and the exact
production RXBIN above with the same current Release `rxvm`, `rxbvm` and
`library.rxbin`.  It was removed after capture.  All raw identities needed to
reconstruct it are retained in `identities.sha256`, the capture manifest and
the B4 S0 evidence.
