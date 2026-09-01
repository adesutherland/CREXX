# F1g-B qualification commands

Principal first-verdict commands were run from the repository root on
`develop` with the F1g-B implementation frozen.

```sh
cmake --build cmake-build-debug --target testConcurrency ts_http_pooled -j10
ctest --test-dir cmake-build-debug --parallel 6 --output-on-failure \
  -R '^(testConcurrency_(noopt|opt)|ts_http_pooled_(rxbvm|rxtvm)_(noopt|opt))$'

cmake --build cmake-build-release \
  --target rxc rxas rxlink rxbvm rxtvm classlib rxfnsg \
           ts_http_pooled testConcurrency -j10
shasum -a 256 cmake-build-release/bin/rxbvm \
  cmake-build-release/bin/rxtvm
stat -f '%N %z bytes' cmake-build-release/bin/rxbvm \
  cmake-build-release/bin/rxtvm
ctest --test-dir cmake-build-release --parallel 6 --output-on-failure \
  -R '^(testConcurrency_(noopt|opt)|ts_http_pooled_(rxbvm|rxtvm)_(noopt|opt))$'
ctest --test-dir cmake-build-release -L gate_f \
  --parallel 10 --output-on-failure
```

```sh
cmake --build cmake-build-release --target testrxfnsg -j10
ctest --test-dir cmake-build-debug --parallel 4 --output-on-failure \
  -R '^ts_llm_(ollama|providers)_(noopt|opt)$'
ctest --test-dir cmake-build-release --parallel 4 --output-on-failure \
  -R '^ts_llm_(ollama|providers)_(noopt|opt)$'

ctest --test-dir cmake-build-release --parallel 4 \
  --repeat until-fail:100 --output-on-failure \
  -R '^ts_http_pooled_(rxbvm|rxtvm)_(noopt|opt)$'

ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure

tools/asan-run.sh --phase build --build-jobs 10 --build-leaks off
tools/asan-run.sh --phase ctest --leaks off --test-jobs 1 \
  --regex '^(gate_f_.*|rxvmchannel_.*|rxvmbyteendpoint_substrate|program_generation_control-.*|testConcurrency_.*|ts_http_pooled_.*|ts_llm_(ollama|providers)_.*|address_.*|test_address_direct_.*|ts_address_.*|inline_nested_binary_block_owner_.*|rxvmworker_lifecycle|rxvmactive_isolation|persistent_worker_executor-.*)$'
```

After correcting the chunked terminal-section boundary, the affected path was
rebuilt and rerun proportionately:

```sh
cmake --build cmake-build-debug --target rxfnsg ts_http_pooled -j10
ctest --test-dir cmake-build-debug --parallel 4 --output-on-failure \
  -R '^ts_http_pooled_(rxbvm|rxtvm)_(noopt|opt)$'

cmake --build cmake-build-release --target rxfnsg ts_http_pooled -j10
ctest --test-dir cmake-build-release --parallel 4 --output-on-failure \
  -R '^ts_http_pooled_(rxbvm|rxtvm)_(noopt|opt)$'
ctest --test-dir cmake-build-release --parallel 4 \
  --repeat until-fail:100 --output-on-failure \
  -R '^ts_http_pooled_(rxbvm|rxtvm)_(noopt|opt)$'

tools/asan-run.sh --phase build --build-jobs 10 --build-leaks off \
  --build-target rxfnsg --build-target ts_http_pooled
tools/asan-run.sh --phase ctest --leaks off --test-jobs 1 \
  --regex '^ts_http_pooled_(rxbvm|rxtvm)_(noopt|opt)$'
```
