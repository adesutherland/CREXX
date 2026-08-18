# Linux harness repair validation commands

Commands ran from the repository root on Linux x86-64. `BUILD` denotes the
external MinSizeRel Ninja tree and `PREFIX` denotes a fresh external install
prefix.

## Build and focused scheduling proof

```sh
cmake -S . -B BUILD
cmake --build BUILD --parallel 4

ctest --test-dir BUILD --parallel 8 --repeat until-fail:5 \
  --output-on-failure -L '^server$'
```

The generated CTest metadata was also inspected to confirm `RUN_SERIAL=true`
for all eight `ts_http_server_*` and `ts_http_server_failures_*` cases.

## Maintained concurrency gates

```sh
ctest --test-dir BUILD --parallel 10 --output-on-failure \
  -L '^concurrency'

CREXX_HTTP_TLS_LIVE_VERIFY=1 \
CREXX_HTTP_TLS_LIVE_HOST=example.com \
CREXX_HTTP_TLS_MISMATCH_HOST=wrong.host.badssl.com \
ctest --test-dir BUILD --parallel 1 --output-on-failure -V \
  -R '^ts_http_tls_live_'

ctest --test-dir BUILD --parallel 10 --output-on-failure \
  --repeat until-fail:20 -L '^concurrency-stress$'
```

## Parser and complete regression gates

```sh
ctest --test-dir BUILD --parallel 10 --output-on-failure \
  -L '^syntax_highlighting$'

ctest --test-dir BUILD --parallel 10 --output-on-failure \
  --no-tests=error
```

## Install and package proof

```sh
cmake --install BUILD --prefix PREFIX

PREFIX/bin/rxc -i PREFIX/bin -x -o concurrency_basic \
  concurrency_basic.crexx
PREFIX/bin/rxas -o concurrency_basic concurrency_basic
PREFIX/bin/rxlink -s -o concurrency_basic_linked \
  concurrency_basic.rxbin PREFIX/bin/library PREFIX/bin/classlib
PREFIX/bin/rxbvm concurrency_basic_linked.rxbin
PREFIX/bin/rxtvm concurrency_basic_linked.rxbin
```

The build payload was archived with `zip`, checked with `zip -T`, packaged with
`scripts/package-linux-deb.sh`, extracted with `dpkg-deb -x`, and replayed
through the same both-VM installed-toolchain smoke.

## Formal qualification boundary

These logs validate the patched working tree before its commit identity
existed. The authoritative platform result must therefore be produced after
this evidence and the harness repair are committed:

```sh
concurrency/qa/run-linux-qualification.sh \
  EXPECTED_FULL_COMMIT FRESH_BUILD_DIRECTORY FRESH_EVIDENCE_DIRECTORY
```

That runner requires a clean exact commit and is the only source of the formal
QA-C `RESULT.txt` and closed `SHA256SUMS`.
