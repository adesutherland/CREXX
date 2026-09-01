# PERF2-09 Mac scorecard

## Scope and identities

The approved common aggregate is exactly Sieve, Permute, Bounce, Richards and
Base64 (`N=5`). Towers, RexxCPS and lifecycle are separate lanes. Mandelbrot,
Storage, List and JSON have approved no-ratio dispositions from PERF2-08.

The profiling-off Release products are:

| Product | Bytes | SHA-256 |
| --- | ---: | --- |
| `rxvm` | 998,904 | `931c75e18530c4c4f80f578f64599e7cc8e31aaad76a272ec4b618853fa662ad` |
| `rxbvm` | 999,064 | `62efe6a725ad7b2a544d92611e021d11757b89635db3075a750838c41b4ebf24` |
| `library.rxbin` | 862,512 | `61984fd0a7f73736e6482a7f012bdbe2cc0ecc36cc6656f397e69df64a1a4c76` |

The exact compiler, assembler, linker, runtime, source, RXAS, RXBIN,
generated NetRexx and comparator identities are in `artifacts.csv`. Timing
started on AC with low-power mode off and no thermal/power warning at load
`1.47 1.65 1.79`; it ended with the same power/thermal state at load
`2.97 2.79 2.49`.

## Common-five throughput

Medians are normalized work per second; higher is better. cREXX cells show
`rxvm / rxbvm`. `*` remains noisy after the one permitted append.

| Workload | Equal work | cREXX | ooRexx | decimal NetRexx |
| --- | ---: | ---: | ---: | ---: |
| Sieve | 5,500 | 5,078.931 / 3,758.560 | 704.010 | 2,684.101 |
| Permute | 5,000 | 2,455.036 / 2,151.502 | 306.686 | 4,575.055 |
| Bounce | 4,200 | 3,592.595 / 2,727.942 | 920.585 | 2,103.933 |
| Richards | 20 | 2.868 / 2.835 | 10.732 | 18.175 |
| Base64 | 2,500 | 1,499.625* / 1,510.261* | 2,083.343 | 1,812.351 |

| Comparison | Geometric mean | Verdict |
| --- | ---: | --- |
| `rxvm / ooRexx` | 2.125260 | aggregate ahead; Richards and Base64 remain below parity |
| `rxbvm / ooRexx` | 1.842840 | aggregate ahead; Richards and Base64 remain below parity |
| `rxvm / NetRexx` | 0.742985 | below parity; Permute, Richards and Base64 are deficits |
| `rxbvm / NetRexx` | 0.644251 | below parity; Sieve, Permute, Richards, Bounce and Base64 remain below the 1.50 band |

## Exact per-workload ratios and remaining gain

Each ratio is cREXX/reference, higher is better. “Parity need” and “1.50 need”
are additional multiplicative gains from this product; `met` means the band is
already met.

| Workload | VM | vs ooRexx | Parity need | 1.50 need | vs NetRexx | Parity need | 1.50 need |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Sieve | `rxvm` | 7.214291 | met | met | 1.892228 | met | met |
| Sieve | `rxbvm` | 5.338790 | met | met | 1.400305 | met | 1.071x |
| Permute | `rxvm` | 8.005043 | met | met | 0.536613 | 1.864x | 2.795x |
| Permute | `rxbvm` | 7.015322 | met | met | 0.470268 | 2.126x | 3.190x |
| Bounce | `rxvm` | 3.902513 | met | met | 1.707562 | met | met |
| Bounce | `rxbvm` | 2.963270 | met | met | 1.296592 | met | 1.157x |
| Richards | `rxvm` | 0.267262 | 3.742x | 5.612x | 0.157815 | 6.337x | 9.505x |
| Richards | `rxbvm` | 0.264171 | 3.785x | 5.678x | 0.155990 | 6.411x | 9.616x |
| Base64 | `rxvm` | 0.719817 | 1.389x | 2.084x | 0.827447 | 1.209x | 1.813x |
| Base64 | `rxbvm` | 0.724922 | 1.379x | 2.069x | 0.833316 | 1.200x | 1.800x |

## Separate qualified lanes

| Lane | cREXX median | Comparator median | Ratio / disposition |
| --- | ---: | ---: | --- |
| Towers, elapsed for 100 repetitions | 3,631.729 / 3,707.652 ms | ooRexx 1,191.427 ms | `0.328060 / 0.321343`; needs 3.048x/3.112x to parity; NetRexx 35.204 ms is an explicitly excluded binary/JVM startup control |
| RexxCPS native rate | 37.929 / 35.546 MCPS | canonical ooRexx 38.091 MCPS | `0.995754 / 0.933193`; needs 1.004x/1.072x to parity and 1.506x/1.607x to the strong band |
| RexxCPS controls | — | Regina 32.158; NetRexx 46.030 MCPS | labelled controls only; the cREXX 2.2d and NetRexx 2.2n adaptations are not common-source aggregate cells |

## Lifecycle, RSS and artifacts

Lifecycle medians are 77.541 ms cREXX compile, 7.312 ms assemble,
2.986/2.826 ms `rxvm`/`rxbvm` load-to-first-result, 4.479 ms ooRexx
translate, 8.918 ms ooRexx load-to-first-result, 447.054 ms NetRexx compile
and 30.128 ms NetRexx load-to-first-result. Only cREXX compile clears the
post-append noise thresholds. These phases are not steady-state ratios and no
phase is relabelled as pure load.

Peak-RSS medians for cREXX stay between 16.59 and 18.20 MiB across the seven
timed workloads. ooRexx stays between 17.05 and 17.25 MiB. The comparable
NetRexx/JVM medians range from 165.89 to 528.09 MiB, with Towers' binary/JVM
control at 46.58 MiB; four NetRexx series remain noisy. RSS is descriptive and
is not blended into throughput.

Current optimized cREXX source/RXAS/RXBIN sizes are:

| Workload | Source | RXAS | RXBIN |
| --- | ---: | ---: | ---: |
| Sieve | 983 | 9,774 | 4,864 |
| Permute | 1,313 | 29,684 | 11,773 |
| Bounce | 2,230 | 45,362 | 17,175 |
| Richards | 9,403 | 255,035 | 78,742 |
| Base64 | 4,437 | 114,978 | 38,233 |
| Towers | 3,529 | 81,065 | 28,647 |
| RexxCPS | 12,180 | 195,576 | 68,446 |

## Approved exclusions

| Workload/capability | Final PERF2-08 disposition |
| --- | --- |
| Mandelbrot | approved exclusion: ordinary ooRexx decimal modes do not reproduce the binary64 500/750 checksums; no ratio |
| Storage / CAP-02 | diagnostic exclusion and defer to an explicit post-Release 1 Level G ownership/container decision |
| List | diagnostic exclusion: the cREXX weak-reference arena adds material ownership work |
| JSON / CAP-01 | diagnostic exclusion and defer: parse/result/access models are not common |
| CAP-03 | common Base64 benchmark remains qualified; a reusable API is a separate deferred product track |
| CAP-04 | retain honestly named lifecycle phases; pure load-only comparison excluded pending a separately approved interface |

## Ranked ooRexx-closure handover

1. **Richards** is the largest qualified common deficit in both VMs and to
   both reference runtimes. Accepted V1R01-R1 removed 22.38% of exact
   copy operations/bytes at its proof cell and delivered about 21% Release
   improvement, but the current product still needs 3.74–3.79x to ooRexx
   parity. Its residual general owner must be re-attributed before any new
   candidate; this scorecard does not re-authorize rejected reset, ledger,
   pooling, slab or broad-layout designs.
2. **Towers** is 3.05–3.11x from ooRexx parity in the now-qualified object
   lane. It is not a common-five aggregate member. No new allocation/value
   candidate is inferred without current shape/lifetime attribution.
3. **Base64** needs 1.38–1.39x to ooRexx parity and remains timing-noisy. Its
   known exact work is string copy/length materialization; the deferred
   reusable API is not a benchmark-speed authorization.
4. **RexxCPS** is near ooRexx parity but still needs 1.51–1.61x to reach the
   separately governed strong band. Retained conversion/materialization work
   remains a later general-mechanism input, not a selected candidate.

The secondary common NetRexx deficit order is Richards, Permute, Base64, then
the `rxbvm` strong-band gaps in Bounce and Sieve. Permute needs 1.86–2.13x to
NetRexx parity while remaining 7–8x ooRexx; it therefore stays an accepted
direct-placement guard until residual current work is independently
re-attributed.

The exact next candidate is deliberately unselected. PERF2-10/11 and the
cross-platform/final VM decisions remain outside this activity.
