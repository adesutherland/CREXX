# CONC-16 Level G HTTP server retained commands

Commands ran from `/Users/adrian/CLionProjects/CREXX`. The Step 2 product at
commit `229b3e8b4` was frozen before the first Step 3 Release rebuild under
`/tmp/crexx-httpserver-control-229b3e8b4`.

## Ordinary Release build

```sh
cmake --build cmake-build-release --parallel 10
```

The cache recorded `CMAKE_BUILD_TYPE=Release`, `CREXX_VM_PROFILING=OFF` and
`CREXX_VM_HANDLER_PANEL=profile-20`.

## Paired ordinary single-thread guard

The first and identical confirmation campaigns used:

```sh
caffeinate -i cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-16-conc-16-http-server-first-release-verdict/manifest.txt \
  --output-dir OUTPUT_DIRECTORY \
  --measurement timing --warmups 1 --runs 12

cmake-build-release/bin/crexx \
  performance/evidence/2026-08-16-conc-16-http-rationalisation-first-release-verdict/summarize_paired.crexx \
  --nocolour --nokeep --args OUTPUT_SUMMARY OUTPUT_DIRECTORY/samples.csv
```

`OUTPUT_DIRECTORY` was first `timing/single-thread`, then
`timing/single-thread-confirmation`. `OUTPUT_SUMMARY` was respectively
`paired-summary.csv` and `paired-summary-confirmation.csv`. The reducer is the
unchanged Level B reducer retained by the immediately preceding CONC-16 slice;
the new manifest deliberately preserves its control/candidate variant names.

## Focused Debug and Release QA

```sh
ctest --test-dir cmake-build-debug \
  -R '^(ts_http_(service_contract|server|pooled|streaming|crexx_rag|policy|codec)|ts_llm_(ollama|providers))' \
  --parallel 12 --output-on-failure

ctest --test-dir cmake-build-release \
  -R '^(ts_http_(service_contract|server|pooled|streaming|crexx_rag|policy|codec)|ts_llm_(ollama|providers))' \
  --parallel 12 --output-on-failure
```

The complete outputs are retained in `qa/debug-focused.log` and
`qa/release-focused.log`.

The server and negative handler paths were also built and run under Apple ASan
with leak detection disabled because this host does not support LeakSanitizer:

```sh
tools/asan-run.sh --phase build --build-target ts_http_server --build-jobs 10 \
  --build-leaks off --no-live-tail
tools/asan-run.sh --phase ctest \
  --regex '^ts_http_server_(rxbvm|rxtvm)_(noopt|opt)$' \
  --leaks off --test-jobs 4 \
  --fixture-exclude-setup linked_opt_runtime_artifacts --no-live-tail
tools/asan-run.sh --phase build --build-target ts_http_server_failures \
  --build-jobs 10 --build-leaks off --no-live-tail
tools/asan-run.sh --phase ctest \
  --regex '^ts_http_server_failures_(rxbvm|rxtvm)_(noopt|opt)$' \
  --leaks off --test-jobs 4 \
  --fixture-exclude-setup linked_opt_runtime_artifacts --no-live-tail
```

The corresponding CTest logs are under
`cmake-build-debugasan/asan-logs/20260816-131821-ctest/` and
`cmake-build-debugasan/asan-logs/20260816-132219-ctest/`; build logs are under
the adjacent `20260816-131313-build/` and `20260816-132207-build/` directories.

## Server scenario capture

CTest intentionally deletes its temporary fully linked images. The retained
server capture rebuilt the same stripped images explicitly:

```sh
mkdir -p /tmp/crexx-httpserver-release-linked
for mode in rxbvm_noopt rxbvm_opt rxtvm_noopt rxtvm_opt; do
  cmake-build-release/bin/rxlink -s \
    -o "/tmp/crexx-httpserver-release-linked/ts_http_server_${mode}" \
    "cmake-build-release/lib/rxfnsg/tests_functional/ts_http_server_${mode}.rxbin" \
    cmake-build-release/bin/library.rxbin \
    cmake-build-release/bin/classlib.rxbin \
    cmake-build-release/bin/rxfnsg.rxbin
done

cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest performance/evidence/2026-08-16-conc-16-http-server-first-release-verdict/server-manifest.txt \
  --output-dir performance/evidence/2026-08-16-conc-16-http-server-first-release-verdict/timing/server \
  --measurement timing --warmups 1 --runs 12

cmake-build-release/bin/crexx \
  performance/evidence/2026-08-16-conc-16-http-server-first-release-verdict/summarize_server.crexx \
  --nocolour --nokeep --args \
  performance/evidence/2026-08-16-conc-16-http-server-first-release-verdict/timing/server/outputs.csv \
  performance/evidence/2026-08-16-conc-16-http-server-first-release-verdict/server-summary.csv
```

All timing samples ran serially. The scenario includes a deliberate 200 ms
incomplete-request deadline, two parallel public-client requests, binary and
text handler results, protocol-negative checks and structured cleanup.

## Host and artifact inspection

```sh
uname -srm
sysctl -n machdep.cpu.brand_string
stat -f '%z' CONTROL_OR_CANDIDATE
shasum -a 256 CONTROL_OR_CANDIDATE
```
