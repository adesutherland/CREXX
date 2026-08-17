# Replay commands

The clean control compiler was configured and built in a detached worktree:

```sh
git worktree add --detach CONTROL_SOURCE 00188b13c
cmake -S CONTROL_SOURCE -B CONTROL_BUILD -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_PROFILING=OFF \
  -DDSLSH_LOCAL_DIR=/Users/adrian/CLionProjects/DSL-Syntax-Highlighter
cmake --build CONTROL_BUILD --target rxc --parallel 10
```

The candidate used the ordinary `cmake-build-release/bin/rxc`; both CMake
caches were checked for `CREXX_VM_PROFILING:BOOL=OFF` and `Release`.

Each established artifact cell used the same current include image and exact
source:

```sh
CONTROL_RXC -i cmake-build-release/bin -o CONTROL/NAME SOURCE
CANDIDATE_RXC -i cmake-build-release/bin -o CANDIDATE/NAME SOURCE
```

RXAS instructions were counted with the established source-instruction rule:

```sh
awk '/^   [a-z][a-z0-9]*([ \t]|$)/ {n++} END {print n+0}' IMAGE.rxas
```

The focused reproducer and DeltaBlue cells were assembled, linked with the
same current `library.rxbin`, and run by the same profiling-off Release VM
panel. The unoptimized form adds `-n` to `rxc` and `rxas`:

```sh
rxc -i cmake-build-release/bin -o IMAGE SOURCE
rxas -o IMAGE.rxbin IMAGE
rxlink -s -o IMAGE-linked IMAGE.rxbin cmake-build-release/bin/library.rxbin
rxvm IMAGE-linked.rxbin
rxvm IMAGE-linked.rxbin -a 10
rxtvm IMAGE-linked.rxbin -a 10
rxbvm IMAGE-linked.rxbin -a 10
```

The structural Release contract used:

```sh
cmake \
  -DRXC=cmake-build-release/bin/rxc \
  -DSOURCE=compiler/tests/rexx_src/inline_indexed_attr_factory_lifetime.crexx \
  -DINCLUDE_DIR=cmake-build-release/bin \
  -DWORK=SHAPE_WORK \
  -P cmake/CheckInlineIndexedAttrFactoryLifetime.cmake
```

Compiler timing used `/usr/bin/time -p`, one warm-up per compiler, six serial
pairs and alternating control/candidate order. Every pair compiled
`performance/tools/run_evidence_bundle.crexx`; its RXAS was checked
byte-for-byte. Raw `real` samples are in `compiler-timing.csv`.

Focused Debug selection:

```sh
ctest --test-dir cmake-build-debug --parallel 5 --output-on-failure \
  -R '^(inline_indexed_attr_factory_lifetime_(run_noopt|run_opt|shape)|benchmark_awfy_deltablue_(noopt|opt))$'
```
