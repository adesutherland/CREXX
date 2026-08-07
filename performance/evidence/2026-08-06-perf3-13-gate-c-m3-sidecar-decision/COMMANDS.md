# Commands

The retained manifests in the scratch result root were executed with the
corrected Level B pairwise scheduler. Formal timing used one warmup and 12
recorded rounds:

```sh
/private/tmp/crexx-rxvm-inline.yvLywZ/builds/baseline/bin/crexx \
  performance/tools/run_cross_runtime_matrix.crexx --nokeep --args \
  --manifest MANIFEST --output-dir OUTPUT \
  --measurement timing --warmups 1 --runs 12
```

The two independent L32SDH core captures used
`gatec-sidecar-dh-confirm-manifest.txt`. L32SDHO used
`gatec-sidecar-dho-confirm-manifest.txt`. The Base64/JSON diagnostic captures
used the corresponding `*-diagnostic-manifest.txt` files.

Allocator telemetry was requested from each frozen binary with:

```sh
CREXX_RXVM_MEMORY_STATS=1 rxbvm IMAGE library.rxbin -a ARGS
```

The Level B RSS capture intermittently lost the first two `/usr/bin/time -l`
stderr rows and correctly failed instead of inventing a value. The final RSS
panels therefore used direct serial `/usr/bin/time -l` launches in alternating
control/candidate order, four runs per cell, and parsed only the explicit
`maximum resident set size` row. The failed capture is retained in scratch as
`gatec-sidecar-dh-rss-core` and is not evidence.

Focused L32SDH construction and validation used the Release build at
`/private/tmp/crexx-rxvm-inline.yvLywZ/builds/gatec-backoff-l32sdh` and targets
`rxbvm`, `rxc`, `ts_regvalue_tester`, `test_rxvmmemory`, `test_rxvmstem`,
`test_rxvmstem_allocator_family`, `mc_decimal_test_harness`,
`mc_decimal_dynamic`, `mc_decimal_dyn_test_harness`, `db_decimal_dynamic`,
`db_decimal_manual` and `test_db_decimal_archive_link_bin`.
