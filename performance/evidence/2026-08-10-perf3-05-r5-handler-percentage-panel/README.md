# PERF3-05-R5 handler percentage panel

This bundle retains the approved Apple percentage screen and the default-
selection stop for the RXVM handler-placement framework. It adds an auditable
never-inline class and nominal 5%, 10%, 15%, 20%, 30% and practical-maximum
panels without changing the public RXAS/RXBIN ISA or plugin ABI.

## Result

Every one of the 651 semantic handler definitions has exactly one placement
tier. The owner-only `INTERRUPT` pseudo-op has a separate always-inline tier.
The practical maximum keeps 56 host-bound instructions, 62 reserved slots and
the `INULL`/`IUNKNOWN` sentinels callable. It therefore places 531 of the 589
policy-controlled non-reserved public-plus-private definitions inline
(90.15%), or 531/587 (90.46%) when the two sentinels are excluded as well.

Literal `all-inline` normalized preprocessing is byte-identical to the R3
starting commit under both concrete engines. All requested shapes built under
Apple Clang 21.0.0 and real GCC 16.1.0. GCC builds retain R3's explicit
`CREXX_ENABLE_TLS=OFF` limitation.

The balanced one-warmup/four-recorded screens contain 112 cells each. Every
one of the 560 executions per compiler passed its exact output oracle. The
decision-grade Clang contender matrix contains 56 cells, two warmups and 12
recorded rounds: 784/784 executions and 672/672 recorded samples passed. Its
20% result versus literal all-inline is:

| engine | all seven | common five | worst workload |
|---|---:|---:|---:|
| `rxtvm` | +3.857% | +5.475% | Towers -0.915% |
| `rxbvm` | +3.152% | +4.697% | Richards -1.714% |

No Clang 20% guard fires. Formal 15% still fires on `rxtvm` Base64 (-4.545%)
and RexxCPS (-3.821%), and on `rxbvm` RexxCPS (-4.424%). Formal 30% fires the
`rxtvm` common-five guard (-1.089%).

The decisive GCC all-inline/20% matrix contains 28 cells and the same formal
sampling: 392/392 executions and 336/336 recorded samples passed. Correctly
treating RexxCPS as a higher-is-better benchmark rate, 20% is:

| engine | all seven | common five | worst workload |
|---|---:|---:|---:|
| `rxtvm` | +3.175% | +3.974% | Bounce **-10.072%** |
| `rxbvm` | +9.646% | +12.464% | Towers -0.428% |

The GCC `rxtvm` Bounce guard is decisive. The GCC screen shows a Bounce guard
at every requested non-inline percentage; even `max-eligible` remains -4.693%
there and also loses 5.604% on Base64. Therefore no single common percentage
is guard-clean across Apple Clang and GCC. The default remains `all-inline`,
Linux/Windows selection work has not started, and the programme is stopped for
Adrian's explicit trade-off decision.

Both complete 20% compiler trees pass the 14/14 focused dispatch, signal,
interrupt, breakpoint, worker, reentrancy and late-load suite.

## Size and build-cost result

Clang 20% reduces the owner from 532,512/531,868 bytes to 146,824/145,608
bytes for `rxtvm`/`rxbvm` (about 72.4%) and the first two-VM target build from
40.02 s to 7.42 s (81.5%). GCC 20% reduces the owner from
1,493,900/1,478,368 bytes to 438,816/442,304 bytes (about 70%) and the
diagnostic two-VM target build from 310.30 s to 39.84 s (87.2%).

`max-eligible` provides limited upper-panel headroom. Clang builds it in 35.73
s and leaves a roughly 511-515 KiB owner; GCC builds it in 233.04 s and leaves
a roughly 1.386-1.388 MiB owner. Total text does not track owner size: Clang's
outlined panels grow total product text because callable wrappers remain,
whereas GCC 20% reduces total text. `static-shape-and-build.csv` keeps these
metrics separate.

These are single clean first-target build measurements. They establish compiler
effort, not yet build-time or binary-layout predictability under arbitrary
future edits; that requires a controlled perturbation series after selection.

## Bundle map

- `clang-pilot/`, `gcc-pilot/`: raw requested-percentage screens, summaries,
  outputs and capture manifests.
- `clang-formal/`, `gcc-formal/`: raw decision-grade contender samples,
  summaries, outputs and capture manifests.
- `*-comparisons.csv`, `*-aggregates.csv`: corrected lower/higher-is-better
  comparisons and governed aggregate point estimates.
- `handler-policy-ledger.csv`: all 651 handlers plus owner pseudo-op and tier.
- `static-shape-and-build.csv`: owner, product text/file, handler-prefixed
  linked symbols and compiler build cost.
- `*-pre-state.txt`, `*-post-state.txt`: host/power/thermal/process identity and
  frozen hashes. Formal pre/post hashes are identical and no thermal warning
  was recorded.
- focused CTest and runner logs: zero failures and empty runner stderr.
- `COMMANDS.md`: exact replay boundary and the all-inline expansion proof.

R3 remains the authority for the compiler-specific outlined-call lowering and
the causal code-generation investigation. This bundle does not repeat those
forensics.
