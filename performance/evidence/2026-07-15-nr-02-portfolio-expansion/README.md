# NR-02 approved portfolio expansion

Status: qualification pilots retained; formal quiet-machine baselines remain
NR-10 work

This bundle records the six new steady-state workloads and the separate
compile/load/first-result lifecycle lane on the same macOS ARM64 development
host. Every steady-state cell has one serial warmup plus three recorded process
samples. Every lifecycle phase has three recorded samples. No passing recorded
sample was rejected as an outlier.

The cREXX images come from the Release build configured with Apple clang
`-O3 -DNDEBUG`. The source tree was intentionally dirty while the approved
ports were being implemented on top of commit
`3aee597382c90152b31a4a772e70ceac174688dc`.

| Runtime/toolchain | Qualified installation |
| --- | --- |
| cREXX | `crexx-1.0.0-beta.3+local.g3aee597382c9.dirty`; Release Apple clang `-O3 -DNDEBUG` |
| ooRexx | 5.1.0 r12973 official universal portable build under `/Users/adrian/.local/opt/oorexx/5.1.0-12973` |
| NetRexx | 5.10-GA build 18-20260320-1410 under `/Users/adrian/.local/opt/netrexx/5.10-GA` |
| Java | Temurin OpenJDK 26.0.1 used by the NetRexx compile/run lane |

## Steady-state qualification medians

Values are process milliseconds, lower is better, and are rounded to three
significant digits. These are qualification pilots, not published ratios.

| Workload / common argument | CREXX | ooRexx | NetRexx | Comparability |
| --- | ---: | ---: | ---: | --- |
| Bounce / 100 repetitions | 356 | 115 | 32.4 | equivalent object/reference ports |
| Storage / 10 repetitions | 2,070 | 31.9 | 30.6 | cREXX diagnostic only: node wrapper adds one object/allocation per logical array |
| List / 100 repetitions | 243 | 245 | 31.7 | cREXX disclosed weak-reference arena; aggregate review required |
| Richards / 1 repetition | 648 | 104 | 40.9 | same state-machine representation and canonical queue/hold result |
| JSON / 5,000 repetitions | 321 | 650 | 71.2 | diagnostic only: path/count versus supplied/Java-collection DOMs |
| Base64 / 500 repetitions | 433 | 278 | 65.3 | same RFC 4648 algorithm with native byte containers |

Storage and JSON must not enter a common aggregate. List remains visible but
requires an NR-11 decision because the cREXX arena owns weak-reference targets.
Bounce, Richards and Base64 are the initial common-score candidates after
formal NR-10 reruns.

## Lifecycle medians

| Phase | CREXX | ooRexx | NetRexx |
| --- | ---: | ---: | ---: |
| compile / translate ms | 74.7 | 4.72 | 428 |
| assemble ms | 6.90 | — | — |
| load-to-first-result ms | 3.22 | 8.79 | 28.0 |

`load_first_result` is deliberately combined: none of the three public command
paths exposes the same loaded-but-not-executed boundary. The cREXX assemble
phase uses `cmake -E chdir` solely to give `rxas` an isolated output directory,
so that small process-wrapper cost is part of the current diagnostic.

## Retained material

- `<workload>/<runtime>/pilot/manifest.json`, `samples.csv`, and per-sample raw
  stdout/stderr come from `performance/tools/run_cross_runtime.crexx`.
- `lifecycle/samples.csv` and the compiled probe artifacts come from
  `performance/tools/run_lifecycle.crexx`.
- `generated/netrexx/` retains `.nrx`, generated Java, classes, compiler logs
  and `javap` output. `generated/oorexx/` retains executable translated images.
- `generated/artifacts.sha256` inventories the retained generated forms.

The source/equivalence decisions are in `performance/NR-02-WORKLIST.md`; the
capability findings are in `performance/capability-gaps.md`.
