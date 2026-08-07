# PERF3-13 Gate C2 corrected first-verdict commands

All commands ran from `/private/tmp/crexx-rxvm-inline.yvLywZ/source`.

## Candidate builds

For `S0`, `S1`, `S1B` and `S2`, substituting the lowercase geometry in the
build-directory name:

```sh
cmake -S . \
  -B /private/tmp/crexx-rxvm-inline.yvLywZ/builds/gatec-c2-<geometry> \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_PROFILING=OFF \
  -DCREXX_RXVM_MEMORY_GEOMETRY=<GEOMETRY>

cmake --build \
  /private/tmp/crexx-rxvm-inline.yvLywZ/builds/gatec-c2-<geometry> \
  --parallel 10 \
  --target rxvm test_rxvmmemory ts_regvalue_tester
```

Focused tests for every build:

```sh
ctest --test-dir \
  /private/tmp/crexx-rxvm-inline.yvLywZ/builds/gatec-c2-<geometry> \
  -R '^(rxvmmemory_allocator|ts_regvalue_tester)$' \
  --output-on-failure
```

## S0 machine-identity proof

`otool -tvV` output was split at every emitted `_rxvm_memory_*` label and
compared exactly between the retained ordinary C1 S0 binary and corrected C2
S0. All 22 text symbols match and none differs. The symbol list and binary
hashes are retained in `s0-disassembly-identity.txt`.

## Pairwise scheduler proof

```sh
/private/tmp/crexx-perf3-13-gateb.qFFjMa/control-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args --self-test
```

## Valid timing screen

```sh
/private/tmp/crexx-perf3-13-gateb.qFFjMa/control-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest /private/tmp/crexx-rxvm-inline.yvLywZ/results/gatec-c2-geometry-manifest.txt \
  --output-dir /private/tmp/crexx-rxvm-inline.yvLywZ/results/gatec-c2-geometry-timing-v3 \
  --measurement timing --warmups 1 --runs 4
```

## Valid RSS screen

```sh
/private/tmp/crexx-perf3-13-gateb.qFFjMa/control-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest /private/tmp/crexx-rxvm-inline.yvLywZ/results/gatec-c2-geometry-manifest.txt \
  --output-dir /private/tmp/crexx-rxvm-inline.yvLywZ/results/gatec-c2-geometry-rss-v2 \
  --measurement rss --warmups 0 --runs 4
```

## Allocator telemetry

Each manifest command was run once with `CREXX_RXVM_MEMORY_STATS=1`; stdout and
stderr are retained separately. The command form was:

```sh
CREXX_RXVM_MEMORY_STATS=1 \
  /private/tmp/crexx-rxvm-inline.yvLywZ/builds/gatec-c2-<geometry>/bin/rxvm \
  <accepted-gate-b-workload.rxbin> \
  /private/tmp/crexx-perf3-13-gateb.qFFjMa/control-build-release/bin/library.rxbin \
  -a <workload-arguments>
```

## Interpretation boundary

Timing, process peak RSS and allocator-owned/retained telemetry are separate
scorecards. The four-round panel is a first-verdict rejection screen, not a
formal selection. Base64 is correctness and memory evidence only for policy
selection. No `rxtvm`, compact value or reclamation candidate was run. The
earlier hot-branch-confounded capture was invalidated before retention and is
not an input to this verdict.
