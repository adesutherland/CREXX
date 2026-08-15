# F1g-C qualification commands

Commands ran from the repository root on `develop`. Retained logs and capture
manifests are the command/output authority.

```sh
cmake --build cmake-build-debug \
  --target rxbvm rxtvm rxfnsg ts_http_pooled ts_http_policy \
  ts_http_tls_live ts_rxsocket --parallel 10
ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure \
  -R '^(ts_rxsocket_(noopt|opt)|ts_http_(pooled|policy|tls_live)_(rxbvm|rxtvm)_(noopt|opt))$'

cmake --build cmake-build-release \
  --target rxbvm rxtvm rxfnsg ts_http_pooled ts_http_policy \
  ts_http_tls_live ts_rxsocket --parallel 10
ctest --test-dir cmake-build-release --parallel 10 --output-on-failure \
  -R '^(ts_rxsocket_(noopt|opt)|ts_http_(pooled|policy|tls_live)_(rxbvm|rxtvm)_(noopt|opt))$'
CREXX_HTTP_TLS_LIVE_VERIFY=1 ctest --test-dir cmake-build-release \
  --parallel 4 --output-on-failure \
  -R '^ts_http_tls_live_(rxbvm|rxtvm)_(noopt|opt)$'
ctest --test-dir cmake-build-release -L gate_f --parallel 10 --output-on-failure
ctest --test-dir cmake-build-release --parallel 4 --repeat until-fail:100 \
  --output-on-failure -R '^ts_http_policy_(rxbvm|rxtvm)_(noopt|opt)$'

ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure

tools/asan-run.sh --phase build --build-jobs 10 --build-leaks off
tools/asan-run.sh --phase ctest --leaks off --test-jobs 1 \
  --regex '^(gate_f_.*|rxvmchannel_.*|rxvmbyteendpoint_substrate|program_generation_control-.*|testConcurrency_.*|address_.*|test_address_direct_.*|ts_address_.*|inline_nested_binary_block_owner_.*|rxvmworker_lifecycle|rxvmactive_isolation|persistent_worker_executor-.*|ts_rxsocket_(noopt|opt)|ts_http_(pooled|policy|tls_live)_(rxbvm|rxtvm)_(noopt|opt))$'
CREXX_HTTP_TLS_LIVE_VERIFY=1 tools/asan-run.sh --phase ctest --leaks off \
  --test-jobs 1 --regex '^ts_http_tls_live_(rxbvm|rxtvm)_(noopt|opt)$'

caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-15-perf3-13-gate-f-f1g-c-http-policy-first-release-verdict/manifest.txt \
  --output-dir performance/evidence/2026-08-15-perf3-13-gate-f-f1g-c-http-policy-first-release-verdict/timing \
  --measurement timing --warmups 1 --runs 12
cmake-build-release/bin/crexx \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1g-c-http-policy-first-release-verdict/summarize_paired.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1g-c-http-policy-first-release-verdict/paired-summary.csv \
  performance/evidence/2026-08-15-perf3-13-gate-f-f1g-c-http-policy-first-release-verdict/timing/samples.csv
```

For the performance control, `rxvmsock.c` was compiled from parent commit
`66f193f3f` with the recorded Release compile command and linked against the
otherwise identical current object set. Candidate binaries were copied before
the control link. Final rebuilt candidate hashes match the frozen binaries.
