# Performance scorecard: NR-10 formal baseline, 2026-07-20

Status: **observation** — formal same-host absolute baseline, not a release claim

## Scope

- Source commit: `1596d7c8cbbfd360b66d50423ef4ea80320fd885` on `develop`.
- Dirty scope: performance governance, Level B measurement tools, manifests,
  evidence and their CMake self-test registration only; no product source edit.
- Platform: Apple M5, macOS 26.5.2 / Darwin 25.5.0 arm64, 10 logical CPUs.
- Build: ordinary CMake `Release`, `-O3 -DNDEBUG`, `CREXX_VM_PROFILING=OFF`.
- Product: optimized RXBIN plus current separate `library.rxbin`; `rxvm` and
  `rxbvm` are never averaged together.
- NetRexx substrate: NetRexx 5.10-GA generated Java on Temurin OpenJDK 26.0.1
  64-bit Server VM, default mixed-mode HotSpot JIT; common sources use
  `options nobinary decimal` and `Rexx` numeric state.
- Common membership: Sieve, Permute, Bounce, Richards and Base64; manifest v3
  uses one equal argument per workload across all four runtime cells.
- Session: AC power, low-power mode off, no recorded thermal/performance
  warning before or after.
- Boundary: absolute cross-runtime observation. It has no same-session accepted-
  product comparator and therefore establishes no regression claim.

## Executive result

All 500 corrected canonical timing, RSS and lifecycle executions/phase rows
passed their correctness gate: 240 initial timing rows plus 30 policy-append
rows, 60 initial RSS rows plus 10 policy-append rows, and 160 lifecycle rows.
The 24 native-C control rows also passed. No observation was removed.

The equal-work throughput geometric means are 0.883021 (`rxvm/ooRexx`),
0.872707 (`rxbvm/ooRexx`), 0.327772 (`rxvm/NetRexx`) and 0.323944
(`rxbvm/NetRexx`). Larger is better. Thus `rxvm` is 88.30% of ooRexx and
32.78% of decimal NetRexx on this balanced score; conversely decimal NetRexx
is about 3.05 times `rxvm` throughput. cREXX exceeds ooRexx on Sieve and
Permute and exceeds decimal NetRexx on Sieve, while the other cells pull the
aggregates below one.

The first capture used NetRexx `options binary` and yielded 0.006220/0.006149
against `rxvm`/`rxbvm`. Those values are withdrawn as Rexx aggregate results:
they measured primitive-Java arithmetic under the JIT. The raw version-1 files
remain only as a labelled binary/JVM audit and for their separate non-common
diagnostics; they are not canonical inputs.

## Correctness and comparability

| Workload | CREXX | ooRexx | NetRexx numeric mode/substrate | Regina/control | Aggregate disposition |
| --- | --- | --- | --- | --- | --- |
| Sieve | equivalent port, pass | equivalent port, pass | decimal `Rexx` array/arithmetic on default JIT, pass | out of scope | included |
| Permute | equivalent port, pass | equivalent procedural port, pass | decimal `Rexx` object/array state on default JIT, pass | out of scope | included |
| Bounce | equivalent port, pass | equivalent object port, pass | decimal `Rexx` object state on default JIT, pass | out of scope | included |
| Richards | common state-machine, pass | common state-machine, pass | decimal `Rexx` state-machine on default JIT, pass | out of scope | included |
| Base64 | equivalent `.binary`, pass | equivalent byte-string, pass | decimal `Rexx` arithmetic plus disclosed Java `byte[]` storage on default JIT, pass | out of scope | included |
| RexxCPS | disclosed 2.2d, pass | canonical 2.2, pass | disclosed 2.2n, pass | Regina canonical 2.2, pass | excluded: adaptations/native rate |
| Mandelbrot | equivalent port, pass | known checksum failure from NR-02 | binary-typed arithmetic-XOR control, pass | out of scope | excluded: not comparable |
| Towers | equivalent object port, pass | procedural diagnostic, pass | binary-typed object/JVM control, pass | out of scope | excluded: ooRexx representation and NetRexx numeric mode |
| Storage | node-wrapper diagnostic, pass | equivalent object port, pass | binary-typed object/JVM control, pass | out of scope | excluded: cREXX representation and NetRexx numeric mode |
| List | weak-reference arena, pass | equivalent object port, pass | binary-typed object/JVM control, pass | out of scope | excluded: adaptation review and NetRexx numeric mode |
| JSON | path/count diagnostic, pass | supplied DOM diagnostic, pass | binary-typed Java DOM control, pass | out of scope | excluded: different native surfaces and NetRexx numeric mode |

### NetRexx implementation boundary

The five common implementations contain 16,431 bytes of authored `.nrx`, which
the NetRexx compiler expands to 35,743 bytes of retained generated Java and
25,191 bytes of class files. This is not a useful percentage split between two
independent implementations: all benchmark logic is authored in NetRexx, all
execution uses the JVM/JIT, and the generated Java is compiler output rather
than a hand-written Java benchmark. Sieve, Permute, Bounce and Richards keep
their timed logic in `netrexx.lang.Rexx` values. Base64 additionally uses Java
`byte[]`, `String`/`StringBuilder`, `Byte.toUnsignedInt` and `Arrays.equals` for
the disclosed binary-storage/observation surface while its indices, byte
arithmetic, loops and checksum remain decimal `Rexx` work.

## Canonical equal-work throughput

Rates are process-inclusive work units/second. Every runtime receives the same
work count within a workload, so the ratios are also exactly the corresponding
inverse elapsed-time ratios. Native RexxCPS rates are separate.

| Workload | Equal work | `rxvm` median (n) | `rxbvm` median (n) | ooRexx median (n) | decimal NetRexx median (n) | Noise disposition |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| Sieve | 5,500 | 5,044.90 (10) | 4,980.14 (10) | 713.56 (10) | 2,709.44 (10) | stable |
| Permute | 5,000 | 647.13 (10) | 642.32 (10) | 315.29 (10) | 4,354.91 (20) | NetRexx noisy after append |
| Bounce | 2,200 | 328.01 (10) | 322.99 (10) | 993.49 (10) | 2,009.95 (10) | stable |
| Richards | 20 | 1.72817 (10) | 1.72031 (10) | 11.4375 (10) | 17.7025 (10) | stable |
| Base64 | 2,500 | 1,579.68 (20) | 1,550.92 (20) | 2,130.12 (10) | 1,840.57 (10) | both CREXX VMs noisy after append |

## Common aggregate

| Comparison | Geometric mean | N | Exact membership |
| --- | ---: | ---: | --- |
| `rxvm / ooRexx` | 0.883021 | 5 | Sieve; Permute; Bounce; Richards; Base64 |
| `rxvm / NetRexx` | 0.327772 | 5 | Sieve; Permute; Bounce; Richards; Base64 |
| `rxbvm / ooRexx` | 0.872707 | 5 | Sieve; Permute; Bounce; Richards; Base64 |
| `rxbvm / NetRexx` | 0.323944 | 5 | Sieve; Permute; Bounce; Richards; Base64 |

Ratios are `CREXX normalized rate / reference normalized rate`; larger is
better. Nothing is imputed. Exact per-workload ratios are in
`timing-equal-work/ratios.csv`.

## RexxCPS native rate

| Runtime/VM | Variant | Median clauses/s | n | MAD % | Span % |
| --- | --- | ---: | ---: | ---: | ---: |
| CREXX `rxvm` | disclosed 2.2d | 1,227,312.5 | 10 | 0.270 | 1.495 |
| CREXX `rxbvm` | disclosed 2.2d | 1,219,940.0 | 10 | 0.362 | 1.838 |
| ooRexx | canonical 2.2 | 39,920,638.0 | 10 | 0.264 | 1.786 |
| Regina | canonical 2.2 | 33,214,478.5 | 10 | 0.470 | 2.009 |
| NetRexx | disclosed 2.2n | 48,067,752.0 | 10 | 1.867 | 6.067 |

These are visible community/diagnostic rates, not a common aggregate: the
cREXX and NetRexx sources are disclosed adaptations.

## Lifecycle and startup

| Runtime/VM | Phase | Median ms | n | Min-max ms | Noise status |
| --- | --- | ---: | ---: | --- | --- |
| CREXX shared | compile | 78.098 | 20 | 76.580-80.973 | stable after append |
| CREXX shared | assemble | 7.002 | 20 | 6.629-19.160 | noisy span |
| CREXX `rxvm` | load-first-result | 2.914 | 20 | 2.727-3.121 | noisy span |
| CREXX `rxbvm` | load-first-result | 2.767 | 20 | 2.613-3.036 | noisy span |
| ooRexx | translate | 4.019 | 20 | 3.777-5.575 | noisy |
| ooRexx | load-first-result | 8.149 | 20 | 7.729-37.216 | noisy retained extreme |
| NetRexx decimal | compile | 401.986 | 20 | 389.459-506.942 | noisy |
| NetRexx decimal | load-first-result | 29.529 | 20 | 28.066-32.777 | noisy |

## Peak RSS

Medians are MiB. The authoritative CSV retains bytes, quartiles and ranges.

| Workload | `rxvm` | `rxbvm` | ooRexx | NetRexx | n/noise |
| --- | ---: | ---: | ---: | ---: | --- |
| Sieve | 16.75 | 16.11 | 17.05 | 408.83 | n=3, stable |
| Permute | 17.00 | 16.27 | 17.05 | 170.97 | n=3, stable |
| Bounce | 17.31 | 16.55 | 17.06 | 166.30 | n=3, stable |
| Richards | 19.38 | 17.89 | 17.09 | 197.16 | n=3, stable |
| Base64 | 16.88 | 16.36 | 17.05 | 204.28 | NetRexx n=13, noisy retained 172.75-521.92 MiB span; others n=3 |
| RexxCPS | 19.14 | 17.50 | 17.25 | 417.02 | NetRexx n=13, noisy; Regina 2.80 MiB n=3 |

## Artifact size

The complete 101-row hash-bound matrix is `artifacts.csv`. Selected byte sizes:

| Workload/runtime | Source | Generated Java | RXAS | RXBIN / class total |
| --- | ---: | ---: | ---: | ---: |
| Sieve CREXX | 983 | — | 9,774 | 4,864 |
| Sieve NetRexx decimal | 1,069 | 2,558 | — | 2,119 |
| Permute CREXX | 1,313 | — | 30,448 | 12,077 |
| Permute NetRexx decimal | 1,326 | 3,237 | — | 3,239 |
| Bounce CREXX | 2,230 | — | 47,525 | 17,847 |
| Bounce NetRexx decimal | 2,012 | 5,066 | — | 5,141 |
| Richards CREXX | 9,403 | — | 259,027 | 80,694 |
| Richards NetRexx decimal | 8,363 | 16,546 | — | 9,561 |
| Base64 CREXX | 4,437 | — | 115,210 | 38,273 |
| Base64 NetRexx decimal | 3,661 | 8,336 | — | 5,131 |
| RexxCPS CREXX | 12,180 | — | 225,660 | 79,405 |
| RexxCPS NetRexx | 7,602 | 13,155 | — | 8,877 |

Product artifacts are `rxvm` 866,200 bytes, `rxbvm` 866,328 bytes and
`library.rxbin` 868,952 bytes. Sizes are separate deployment observations, not
throughput scores.

## Diagnostics and controls

| Result | Label | Median | n | Interpretation |
| --- | --- | ---: | ---: | --- |
| Initial common NetRexx binary aggregate | binary/JVM control | `rxvm` 0.006220; `rxbvm` 0.006149 | N=5 | withdrawn from Rexx aggregate; `options binary` primitive arithmetic under default JIT |
| Mandelbrot `rxvm` / `rxbvm` / NetRexx | diagnostic plus binary/JVM control | 180.379 / 159.027 / 49.688 ms | 10 | ooRexx fails checksum; NetRexx is arithmetic-adapted and binary-typed |
| Towers `rxvm` / `rxbvm` / ooRexx / NetRexx | diagnostic plus binary/JVM control | 688.802 / 698.266 / 67.505 / 28.478 ms | 10 / 10 / 20 / 20 | ooRexx procedural; NetRexx binary-typed; two cells noisy |
| Storage `rxvm` / `rxbvm` / ooRexx / NetRexx | diagnostic plus binary/JVM control | 1,752.200 / 1,774.574 / 24.258 / 28.249 ms | 10 / 10 / 20 / 10 | cREXX node wrapper; NetRexx binary-typed; ooRexx noisy |
| List `rxvm` / `rxbvm` / ooRexx / NetRexx | diagnostic plus binary/JVM control | 217.823 / 217.307 / 234.074 / 29.004 ms | 10 | cREXX weak-reference arena; NetRexx binary-typed |
| JSON `rxvm` / `rxbvm` / ooRexx / NetRexx | diagnostic plus binary/JVM control | 279.278 / 272.723 / 616.013 / 56.566 ms | 10 / 10 / 10 / 20 | different native parser surfaces; NetRexx binary-typed and noisy |
| Mechanical native C | upper-bound control | 519,512,228 nominal ops/s | 10 | scalar branch-heavy control, not RexxCPS |
| Dynamic-value native C | upper-bound control | 47,998,607.5 nominal ops/s | 10 | tagged-value control, not RexxCPS |

The C controls have runtime-input-dependent digests and volatile result sinks.
They answer only whether observed rates are physically plausible for deliberately
different native mechanisms.

## Regression guards

No regression verdict is made. This is an unmatched absolute baseline and the
governance forbids comparing it to older-session medians as a regression. Future
changes must rerun the accepted product/image as a same-session drift control and
apply the approved aggregate, per-workload, lifecycle, RSS and artifact guards.

## Provenance and reproducibility

- Corrected equal-work timing/RSS machine state:
  `provenance/pre-run-equal-work.txt` and
  `provenance/post-run-equal-work.txt`; decimal lifecycle state:
  `provenance/pre-run-decimal.txt` and `provenance/post-run-decimal.txt`.
- Runtime/source/generated/internal-form/executable hashes: `artifacts.csv`.
- Formal commands: versioned manifests under `performance/manifests/` and this
  bundle's `README.md`.
- Canonical timing raw data: `timing-equal-work/samples.csv` and
  `timing-equal-work/outputs.csv`, plus `timing-equal-work-append-1`.
- Canonical RSS raw data: `rss-equal-work/samples.csv` and
  `rss-equal-work/outputs.csv`, plus `rss-equal-work-append-1`.
- Canonical lifecycle raw data: `lifecycle-decimal/samples.csv`; sequence 1-10
  is initial and 11-20 is the append.
- Initial `timing*` and `rss*` paths without `equal-work` are the version-1
  binary-NetRexx audit/non-common diagnostic run, not canonical common inputs.
  Its superseded binary lifecycle directory is deliberately not retained.
- Recursive closure: `checksums.sha256` after final inventory review.

## Interpretation and open decisions

This evidence satisfies the NR-10 formal absolute-baseline scope on this Apple
M5/macOS host and proves the NR-11 scorecard structure. It does **not** qualify
as a release claim: it is a WIP `develop` observation, not a clean named release
candidate with a same-session accepted-product comparator. Timing, RSS and
lifecycle cells that remained noisy after the required append are published as
noisy; no extreme was removed.
