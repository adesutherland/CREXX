# Replay commands

## Exact products and images

H0 was built from commit `51c93c421bf444de28bc52eb9a0faedc99d1ae27`
before the candidate rebuild. H1 was built from the frozen candidate source.
Each compiler produced its own library and benchmark RXAS/RXBIN set. Both sets
were then timed with the same current profiling-off Release `rxtvm` and
`rxbvm`. Exact identities are in `hashes-and-sizes.txt` and
`artifact-summary.csv`.

The ordinary candidate product build was:

```sh
cmake --build cmake-build-release \
  --target rxc rxas rxlink rxtvm rxbvm linked_opt_runtime_artifacts \
  --parallel 10
```

The cache was checked for `CMAKE_BUILD_TYPE=Release` and
`CREXX_VM_PROFILING=OFF`.

## Focused correctness

The exact gate included:

```sh
ctest --test-dir cmake-build-debug --output-on-failure -R \
  '^(rxc_binary_literal_load|rxc_binary_literal_load_(opt|noopt)_run|inline_late_profitability_guard|inline_scalar_accessors_import_opt|inline_test1_(opt|noopt)|inline_test_class_methods_(opt|noopt)|benchmark_awfy_(sieve|list|richards|deltablue|cd|havlak|json)_(opt|noopt)|benchmark_rexxcps_levelb_(opt|noopt))$'
```

`inline_late_profitability_guard` compiles optimized and no-opt source, checks
that `addone()` is absent from optimized RXAS while the expansion-heavy
`updateandsum()` call remains, assembles both images, and runs both under
`rxtvm` and `rxbvm`.

## Formal timing

The maintained Level B runner was built/self-tested and then used with the
retained manifests:

```sh
cmake-build-release/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx \
  --nocolour --nokeep --args \
  --manifest initial-manifest.txt \
  --output-dir VERDICT_DIR \
  --measurement timing --warmups 2 --runs 12
```

The initial capture stopped only after all pre-RexxCPS groups because its
optional metric prefix did not match. No completed observation was discarded.
`rexxcps-completion-manifest.txt` supplied the corrected elapsed-only contract.
`noise-append-manifest.txt` ran the required ten-record whole-panel append.
`decision-append-manifest.txt` ran twice, adding twelve pairs per run for Sieve
and RexxCPS. Every capture used two warmups and passed its correctness oracle.

For each matched round the percentage is:

```text
(H1-T20 / H0 - 1) * 100
```

The paired decision excludes the absolute-noise append and uses 12 pairs for
the clear cells and 36 for Sieve/RexxCPS. The Level B analyzer is replayed with
the initial, RexxCPS completion and two decision-append sample files. It uses
R-7 quartiles and the correct two-sided 95% Student-t critical value for each
sample count: 2.200985 for 12 pairs and 2.030108 for 36 pairs.

## Closeout qualification

After Adrian accepted the first Release verdict, the closeout commands were:

```sh
cmake --build cmake-build-debug --parallel 10
ctest --test-dir cmake-build-debug --parallel 30 --output-on-failure
cmake --build cmake-build-release --parallel 10
```

The final Release focused set comprised 56 named tests: the original mandatory
gate above; the seven adjusted structural contracts (`nr09`, PERF2-04,
three receiver-placement proofs, the fixed-point inline-result proof and the
new late-profitability proof); the thirteen regenerated optimized golden
contracts; indexed-attribute lifetime shape; six decimal RXAS integrity
checks; binary optimized/unoptimized runtime; all four HTTP runtime modes; and
the skipped-copy reassignment shape regression. The exact expanded test names
are retained in `closeout-release-ctest.log`.

After the NR-26 fixed-point correction, the exact accepted product target was
rebuilt again:

```sh
cmake --build cmake-build-release \
  --target rxc rxas rxlink rxtvm rxbvm linked_opt_runtime_artifacts \
  --parallel 10
```

Each rebuilt optimized RXAS/RXBIN for DeltaBlue, CD, Richards, Havlak, JSON,
List, Sieve and RexxCPS was compared with `cmp -s` against its frozen H1 file;
`bin/library.rxbin` was compared the same way. The resulting 17-file proof is
`release-artifact-identity.log`.

Exact results are recorded in `VALIDATION.md` and the closeout logs.
