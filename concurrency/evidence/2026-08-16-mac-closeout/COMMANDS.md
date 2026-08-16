# Mac closeout replay commands

Commands ran from the repository root on `develop`.

## Stress and sanitizer

```sh
ctest --test-dir cmake-build-debug --parallel 10 --output-on-failure \
  --repeat until-fail:20 -L '^concurrency-stress$'

tools/asan-run.sh --phase build --build-jobs 10 --build-leaks off

# REGEX was the exact alternation generated from:
ctest --test-dir cmake-build-debug -N -L '^concurrency$'

tools/asan-run.sh --phase ctest --regex REGEX --leaks off --test-jobs 8
```

The exact 180 selected names can be recovered from the labelled manifest; no
test-name family approximation was used.

## Complete Debug and Release

```sh
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure

cmake --build cmake-build-release --parallel 10
cmake --build cmake-build-release --target concurrency-qa --parallel 10
```

The Release cache recorded `CMAKE_BUILD_TYPE=Release`,
`CREXX_VM_PROFILING=OFF`, `CREXX_VM_HANDLER_PANEL=profile-20` and
`CREXX_ENABLE_TLS=NETWORK`.

## Isolated install and toolchain smoke

```sh
cmake --install cmake-build-release --prefix ISOLATED_PREFIX

cd ISOLATED_WORKSPACE
ISOLATED_PREFIX/bin/rxc -i ISOLATED_PREFIX/bin -x \
  -o concurrency_basic concurrency_basic.crexx
ISOLATED_PREFIX/bin/rxas -o concurrency_basic concurrency_basic
ISOLATED_PREFIX/bin/rxlink -s -o concurrency_basic_linked \
  concurrency_basic.rxbin ISOLATED_PREFIX/bin/library \
  ISOLATED_PREFIX/bin/classlib
ISOLATED_PREFIX/bin/rxbvm concurrency_basic_linked.rxbin
ISOLATED_PREFIX/bin/rxtvm concurrency_basic_linked.rxbin
```

The temporary prefix was archived with macOS `ditto`; only its inventory,
smoke log and digest are retained.
