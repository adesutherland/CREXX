# Qualification commands

The complete profiling-off Release language-benchmark target was rebuilt
first. Each cREXX cell then used the corresponding executable and image:

```sh
cmake-build-release/bin/VM \
  cmake-build-release/tests/benchmarks/BENCHMARK_MODE.rxbin \
  cmake-build-release/bin/library.rxbin -a 1
```

NBody inserted `rx_rxmath` before `library.rxbin`. Its full reference used
argument `250000` under each executable.

The legacy Base64-v2 cells were qualified with:

```sh
/Users/adrian/.local/opt/oorexx/5.1.0-12973/bin/rexx \
  tests/benchmarks/cross-runtime/oorexx/base64_roundtrip_v2.rex 1

# In a temporary directory containing the source:
/Users/adrian/.local/opt/netrexx/5.10-GA/bin/NetRexxC.sh \
  base64_roundtrip_v2.nrx -keep
/usr/bin/java -cp TEMP:/Users/adrian/.local/opt/netrexx/5.10-GA/runlib/NetRexxR.jar \
  base64_roundtrip_v2 1
```
