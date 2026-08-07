# PERF3-13 Gate B closeout commands

All commands ran from `/tmp/crexx-rxvm-inline.yvLywZ/source`.

## Ordinary Release build

```sh
cmake -S . +  -B /tmp/crexx-rxvm-inline.yvLywZ/builds/vm-default-clang-release +  -G Ninja +  -DCMAKE_BUILD_TYPE=Release +  -DCREXX_VM_PROFILING=OFF

cmake --build +  /tmp/crexx-rxvm-inline.yvLywZ/builds/vm-default-clang-release +  --parallel 10 +  --target rxvm rxbvm rxtvm
```

## Corrected pairwise allocator timing

```sh
/private/tmp/crexx-perf3-13-gateb.qFFjMa/control-build-release/bin/crexx +  performance/tools/run_cross_runtime_matrix.crexx +  --nocolour --nokeep --args +  --manifest /private/tmp/crexx-rxvm-inline.yvLywZ/source/performance/evidence/2026-08-06-perf3-13-gate-b-closeout/allocator/pairwise-manifest-v2.txt +  --output-dir /private/tmp/crexx-rxvm-inline.yvLywZ/results/gateb-closure-pairwise-v2 +  --measurement timing +  --warmups 1 +  --runs 12
```

The pairwise summary was generated with:

```sh
/private/tmp/crexx-perf3-13-gateb.qFFjMa/control-build-release/bin/crexx +  performance/evidence/2026-08-06-perf3-13-gate-b-closeout/allocator/summarize_paired_v2.crexx +  --nocolour --nokeep --args +  /private/tmp/crexx-rxvm-inline.yvLywZ/results/gateb-closure-pairwise-v2/paired-summary.csv +  /private/tmp/crexx-rxvm-inline.yvLywZ/results/gateb-closure-pairwise-v2/common-five.csv +  /private/tmp/crexx-rxvm-inline.yvLywZ/results/gateb-closure-pairwise-v2/samples.csv
```

## Bounded RSS materiality check

```sh
/private/tmp/crexx-perf3-13-gateb.qFFjMa/control-build-release/bin/crexx +  performance/tools/run_cross_runtime_matrix.crexx +  --nocolour --nokeep --args +  --manifest /private/tmp/crexx-rxvm-inline.yvLywZ/source/performance/evidence/2026-08-06-perf3-13-gate-b-closeout/allocator/pairwise-manifest-v2.txt +  --output-dir /private/tmp/crexx-rxvm-inline.yvLywZ/results/gateb-closure-rss-v2 +  --measurement rss +  --warmups 0 +  --runs 4
```

## Interpretation boundary

The timing capture is the formal allocator verdict. The four-round RSS capture
is a bounded materiality check. Dispatch/inlining evidence under `dispatch/`
selects compiler-specific concrete engines but is not a formal Linux platform
verdict. Base64 remains a recorded noisy row and is not a policy selector.
