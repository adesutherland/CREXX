# Replay commands

The clean control compiler was configured and built in a detached worktree:

```sh
git worktree add --detach CONTROL_SOURCE 110e298af
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

The decisive Full AWFY Json cells were assembled, linked with the same current
`library.rxbin`, and run by the same profiling-off Release `rxvm`:

```sh
rxas -o IMAGE.rxbin IMAGE
rxlink -s -o IMAGE-linked IMAGE.rxbin cmake-build-release/bin/library.rxbin
rxvm IMAGE-linked.rxbin -a 1 \
  cmake-build-release/tests/benchmarks/awfy_json_rap_minified.json
```

Compiler timing used `/usr/bin/time -p`, one warm-up per compiler, six serial
pairs and alternating control/candidate order. Every pair compiled
`performance/tools/run_evidence_bundle.crexx`; the resulting RXAS was checked
byte-for-byte. Raw `real` samples are in `compiler-timing.csv`.

Focused selection commands:

```sh
ctest --test-dir cmake-build-debug \
  -R '^inline_binary_formal_string_conversion_(run_args_noopt|run_args_opt|shape)$' \
  --output-on-failure
ctest --test-dir cmake-build-debug \
  -R '^benchmark_awfy_json_(noopt|opt)$' --output-on-failure
ctest --test-dir cmake-build-release \
  -R '^(inline_binary_formal_string_conversion_(run_args_noopt|run_args_opt|shape)|benchmark_awfy_json_(noopt|opt))$' \
  --output-on-failure
```
