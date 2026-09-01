# Replay commands

The clean control compiler was configured and built in a detached worktree:

```sh
cmake -S CONTROL_SOURCE -B CONTROL_BUILD -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_PROFILING=OFF \
  -DDSLSH_LOCAL_DIR=/Users/adrian/CLionProjects/DSL-Syntax-Highlighter
cmake --build CONTROL_BUILD --target rxc --parallel 10
```

The candidate used the ordinary `cmake-build-release/bin/rxc`; both CMake
caches were checked for `CREXX_VM_PROFILING:BOOL=OFF`.

Each artifact cell used the same current include image and exact source:

```sh
CONTROL_RXC -i cmake-build-release/bin -o CONTROL/NAME SOURCE
CANDIDATE_RXC -i cmake-build-release/bin -o CANDIDATE/NAME SOURCE
```

RXAS instructions were counted with the established source-instruction rule:

```sh
awk '/^   [a-z][a-z0-9]*([ \t]|$)/ {n++} END {print n+0}' IMAGE.rxas
```

The two affected cells were assembled, linked with the same current
`library.rxbin`, and run by the same profiling-off Release `rxvm`:

```sh
rxas -o IMAGE.rxbin IMAGE
rxlink -s -o IMAGE-linked IMAGE.rxbin cmake-build-release/bin/library.rxbin
rxvm IMAGE-linked.rxbin
```

Queens additionally received `-a 1`. Runtime stdout and exit status are in
`run-results.csv`.

Compiler timing used `/usr/bin/time -p`, one warm-up per compiler, six serial
pairs, and alternating control/candidate order. Every pair compiled
`performance/tools/run_evidence_bundle.crexx`; the resulting RXAS was checked
byte-for-byte. Raw `real` samples are in `compiler-timing.csv`.
