# Commands and sampling contract

All relative paths are from `/Users/adrian/CLionProjects/CREXX`.

The ordinary profiling-off Release product was built with terminal output
redirected to `/tmp/nr26-release-build.XXXXXX.log`:

```sh
cmake --build cmake-build-release --parallel 10 \
  > /tmp/nr26-release-build.XXXXXX.log 2>&1
```

The cache was checked for `Release`, `-O3 -DNDEBUG` and
`CREXX_VM_PROFILING=OFF`. The accepted baseline compiler is the retained
`cmake-build-nr14-candidate` product whose `rxc` hash matches accepted NR-14
evidence. Baseline and candidate benchmark/library inputs were linked
separately with their matching products:

```sh
cmake-build-nr14-candidate/bin/rxlink -s \
  -o /tmp/nr26-release-verdict.FZfZRZ/baseline-rexxcps-linked.rxbin \
  cmake-build-nr14-candidate/tests/benchmarks/benchmark_rexxcps_levelb_opt.rxbin \
  cmake-build-nr14-candidate/bin/library.rxbin

cmake-build-release/bin/rxlink -s \
  -o /tmp/nr26-release-verdict.FZfZRZ/candidate-rexxcps-linked.rxbin \
  cmake-build-release/tests/benchmarks/benchmark_rexxcps_levelb_opt.rxbin \
  cmake-build-release/bin/library.rxbin
```

Both linked images ran under the same current Release VMs to remove VM drift.
The initial formal block was:

```sh
cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest performance/evidence/2026-07-21-nr-26-first-release-verdict/input-manifest.txt \
  --output-dir performance/evidence/2026-07-21-nr-26-first-release-verdict/timing \
  --measurement timing --warmups 1 --runs 12
```

The approved noise rules required two unchanged-condition balanced append
blocks. Each used the same command with `--warmups 0 --runs 12` and output
directories `timing-append-01` and `timing-append-02-retry`. The first attempt
at the second append was interrupted before samples because the pre-start host
check proved a 281% CLion CPU spike; `timing-append-02` is excluded in full.

The repository Level B runner produced the capped absolute summary by merging
the three valid sample files with `--summary-only`. Paired reduction uses all
36 pairs per VM, R-7 quartiles, per-pair
`(candidate / baseline - 1) * 100`, and a two-sided Student-t interval around
the arithmetic mean with critical value 2.030108 for 35 degrees of freedom.
No sample was removed from a valid block.
