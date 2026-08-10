# PERF3-13 E3b bundled-plugin qualification first Release verdict

Date: 2026-08-10

Branch: `develop`

Control source: accepted E3b-P1 commit
`57a0553a225d8103327d4d3842b2f459bf8dae31`, using the exact frozen
pre-change `rx_getpi.rxplugin`.

Candidate: the frozen bundled-plugin qualification slice. `getpi` replaces
the process-global `rand` state with a caller-local SplitMix64 stream seeded by
a short synchronized sequence and declares the audited plugin process
reentrant.

Status: **accepted without a guard hit**. Adrian accepted the result on
2026-08-10 and authorized the separately measured P2 session/ODBC slice. No P2
session implementation or ODBC change was made before this verdict.

## Decisive ordinary-Release comparison

The same profiling-off, profile-20 candidate VMs, optimized RXBIN and library
image compare only the frozen control and candidate plugin binaries. The
startup-included workload performs one million Leibnitz terms, one million
Monte Carlo points and the constant path. One warmup and 12 pairwise-balanced
recorded rounds ran under each concrete VM.

All 52/52 processes passed: four warmups and 48 recorded executions. Every
output completed, the invariant Leibnitz and constant results matched, and all
Monte Carlo results were in the observed sane range 3.138552 to 3.145720.
Negative elapsed percentages are favorable.

| VM | Control median | Candidate median | Paired mean | Mean 95% interval | Favorable | Guard |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| `rxbvm` product | 26.2555 ms | 20.2185 ms | -23.502260% | -24.628395% to -22.376125% | 12/12 | clear |
| `rxtvm` guard | 26.1795 ms | 20.1225 ms | -23.346622% | -23.731576% to -22.961669% | 12/12 | clear |

The repair is decisively favorable on this representative affected plugin.
The result covers the complete plugin load, three calls and teardown rather
than a PRNG-only microbenchmark.

## Interpretation boundary

This slice changes no VM execution source. The already accepted E3b-P1
branch-free call-kernel evidence remains the authority for the permanent
process-reentrant direct path and the sticky legacy compatibility transition.
The present paired result is an end-to-end algorithm and lifecycle guard for
the most material repair in the bundled qualification set. It is not a
cross-platform claim, a whole-plugin-catalogue aggregate, or evidence for the
reserved P2 session ABI or ODBC design.

The other qualified plugins are covered by focused two-context/two-thread
Release calls; `cipher`, `stack` and `id` additionally execute through the
static replay path. Those correctness tests are not treated as timing data.
