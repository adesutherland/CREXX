# PERF2-05 R1a first Release verdict

Status: **accepted; R1a and PERF2-05 closeout complete**

This package retains the mandatory first ordinary Release verdict for the
corrected R1a exact-reference-relink implementation. It compares the accepted
R2a product with a fresh R1a product on the exact retained optimized List RXBIN
at work 100.

The verdict is **clear favorable on both VMs** at the governed 36-pair cap:

- `rxvm`: paired median `-1.151991%`, 32/36 favorable, mean 95% interval
  `[-2.337441%, -0.187062%]`;
- `rxbvm`: paired median `-3.022743%`, 36/36 favorable, mean 95% interval
  `[-3.204016%, -2.814217%]`.

Adrian accepted the verdict on 2026-07-26. The full Debug and ordinary
profiling-off Release products then rebuilt successfully, and broad CTest
passes 1,924/1,924 in both configurations. R1a and the wider PERF2-05 activity
are closed green. R2b and the neutral B1 control remain unselected future
points with explicit reopen evidence; neither keeps PERF2-05 open.

The earlier direct-signal candidate was superseded after semantic review found
that its invalid/non-reference path did not preserve canonical `LINKREF`
instrumentation attribution. Its timing package was excluded from this verdict
and retained only outside the repository under the bounded `/tmp` workspace.

## Package map

- `VERDICT.md`: accepted result, noise reading and closeout boundary.
- `PROVENANCE.md`: source, product, input and host identity.
- `COMMANDS.md`: reproduction commands and stop boundary.
- `paired-summary*.csv`: paired reductions after 12, 24 and 36 pairs.
- `timing/`: all raw samples, outputs and runner summaries; no sample removed.
- `correctness/`: fresh Release focused and compatibility output.
- `build/`: fresh Release configure and build logs.
- `summarize_paired.crexx`: evidence-local Level B paired reducer.
- `checksums.sha256`: integrity ledger for every other retained file.
