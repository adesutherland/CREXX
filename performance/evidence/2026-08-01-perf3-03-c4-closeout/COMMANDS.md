# Replay commands

Commands were run from `/Users/adrian/CLionProjects/CREXX`.

## Full Debug

```sh
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
```

## Sanitizer

```sh
tools/asan-run.sh --phase build \
  --build-target rxvm --build-target rxbvm --build-jobs 10

tools/asan-run.sh --phase ctest \
  --regex '^(06_logic_(noopt|opt)|rxaslogicflowtests(-rxbvm)?|rxasconversiontests(-rxbvm)?)$' \
  --leaks on --test-jobs 1

# macOS reported detect_leaks unsupported, so the identical ASan gate was run:
tools/asan-run.sh --phase ctest \
  --regex '^(06_logic_(noopt|opt)|rxaslogicflowtests(-rxbvm)?|rxasconversiontests(-rxbvm)?)$' \
  --leaks off --test-jobs 1
```

The first CTest command is retained as an unsupported-host result. Only the
second command is the passing ASan result; no LSan pass is claimed.

## Complete Release and install

The already accepted ordinary Release tree was completed and installed into an
isolated prefix:

```sh
cmake --build /tmp/crexx-perf3-03-first-verdict.xCMBN2/build-candidate \
  --parallel 10
cmake --install /tmp/crexx-perf3-03-first-verdict.xCMBN2/build-candidate \
  --prefix /tmp/crexx-perf3-03-closeout.VNMmVS/install

INSTALL/bin/rxvm BASE64_RXBIN INSTALL/bin/library.rxbin -a 1
INSTALL/bin/rxbvm BASE64_RXBIN INSTALL/bin/library.rxbin -a 1
```

The disposable paths are retained for provenance. Recreate them from the
production source and CMake options rather than assuming those directories
remain available.
