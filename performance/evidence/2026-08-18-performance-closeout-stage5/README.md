# Stage 5 Apple pre-release performance scorecard

Date: 2026-08-18

Status: **Apple current-product scorecard complete; exact-commit formal Linux
QA-C remains the Stage 5 exit item**

The less-technical current-results and release-disposition report is
[`performance/RESULTS.md`](../../RESULTS.md).

## Boundary

- Frozen cREXX source:
  `81f15918676d92a7d3e88954d94779ed759e9db8`.
- Fresh ordinary Release build, AppleClang 21.0.0.21000101,
  `CREXX_VM_PROFILING=OFF`.
- Apple M5, macOS 26.5.2, AC power, low-power mode off, user-confirmed clear
  host and serial execution.
- Portfolio-v3 source/work/capability contract, with canonical AWFY controls
  pinned to `74306fec151070fd07157cefeacf19e7e0bcdc89`.
- cREXX `rxtvm` and `rxbvm` are separate. Java and CPython are controls, not
  Rexx aggregate members. Primitive-Java work is not reported as NetRexx
  capability.

## Executive result

All 89 exact-work cells passed two-observation qualification. Formal timing
then retained 178 warmups and 890 initial observations; the one permitted
append added 270 observations for 27 flagged cells. All processes passed their
declared result. RSS retained 356 initial observations plus a 20-observation
append. No sample was removed.

The versioned cREXX/ooRexx common-five geometric mean is **5.467915x on
`rxtvm` and 5.679203x on `rxbvm`**. The separately named genuine-NetRexx
common-four is **1.318977x and 1.358087x**. Those aggregates do not have the
same membership: NetRexx Base64 is a Java/JVM control and is not imputed.

RexxCPS reaches **44.495/44.156 MCPS** on `rxtvm`/`rxbvm`, 13.14%/12.27%
above ooRexx and 36.53%/35.49% above Regina. It is 4.39%/5.12% below the
disclosed genuine-NetRexx 2.2n port, and therefore clears parity with ooRexx
but not the historical 1.50x aspiration.

No benchmark correctness, optimizer-resistance or capability-label defect was
found during formal timing. The lifecycle runner did expose a test-harness
defect: its historical `both` selection ran product-selected `rxvm` plus
`rxbvm`, which duplicates `rxbvm` on this build and omits `rxtvm`. The runner
now names `rxtvm|rxbvm|both`; its self-test and the corrected dual-engine
lifecycle capture pass. Product source and timed images remain the frozen
commit above.

## Port-status wording

`failed` and `unresolved` are different states. `failed` means a completed port
attempt reached build, execution or qualification but did not pass its declared
gate. `unresolved` means no correctness-qualified port was completed and the
review did not prove a language/runtime prohibition. The current heavy rows are
all **unresolved ports**, not failed ones:

- ooRexx CD is expressible in maths and objects, but no complete qualified
  red/black-tree port was finished;
- genuine-NetRexx Full Json has no qualified genuine port, because a parser
  whose material state and work use Java collections is a Java/JVM control;
- ooRexx/NetRexx DeltaBlue and Havlak likewise have no completed qualified
  full port, rather than a proved language limitation.

By contrast, ooRexx Mandelbrot is `not comparable`: an executable attempt
exists, but the decimal/XOR adaptation does not preserve the canonical
size-500/750 results. NetRexx Towers also runs, but its binary/primitive-Java
work makes it a Java/JVM control, not NetRexx capability.

## Named Rexx comparisons

Ratios are oriented so higher is better. The exact inputs and per-workload
ratios are retained in `named-comparisons.csv`.

| Comparison | `rxtvm` | `rxbvm` | Membership |
| --- | ---: | ---: | --- |
| cREXX / ooRexx | 5.467915x | 5.679203x | Sieve, Permute, Bounce, Richards, Base64-v2 |
| cREXX / genuine NetRexx | 1.318977x | 1.358087x | Sieve, Permute, Bounce, Richards |

The genuine-NetRexx view is favourable overall but not uniformly: cREXX
Permute is only 0.540488x/0.549880x of NetRexx, while Sieve, Bounce and
Richards are favourable. That deficit remains visible rather than being hidden
by the geometric mean.

## Current elapsed-time scorecard

Values are process elapsed medians in seconds for the declared v3 work.
`*` means the cell still meets the formal noise trigger after the permitted
append and is retained as noisy/inconclusive; starred cells have `n=20`, all
others `n=10`. A dash is an honestly missing, failed, unresolved or
not-comparable capability cell, never an imputed result.

| Workload | cREXX `rxtvm` | cREXX `rxbvm` | ooRexx | genuine NetRexx | Java control | CPython control |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Sieve | 1.283 | 1.221* | 7.719 | 2.056 | 0.086* | 1.403 |
| Permute | 1.953 | 1.919 | 16.251 | 1.055* | 0.103* | 3.274 |
| Bounce | 1.830 | 1.774 | 4.481 | 1.964 | 0.068* | 1.872 |
| Richards | 0.333 | 0.327* | 1.833 | 1.084 | 0.056* | 0.387 |
| Towers | 1.744 | 1.772 | 1.204 | — | 0.045* | 0.123* |
| Storage | 1.356 | 1.353 | 0.053* | — | 0.043* | 0.042* |
| List | 1.270 | 1.264 | 4.661 | — | 0.057* | 0.599 |
| Mandelbrot | 0.179 | 0.170* | — | — | 0.060* | 0.462 |
| Full Json | 0.045* | 0.042* | 0.076* | — | 0.052* | 0.059* |
| DeltaBlue | 0.448* | 0.457 | — | — | 0.053* | 0.031* |
| CD | 4.797 | 4.774 | — | — | 0.068* | 0.424 |
| Havlak | 3.606 | 3.604* | — | — | 0.133 | 1.474 |
| Queens | 1.302 | 1.284 | 10.798 | 1.933 | 0.074* | 1.716 |
| NBody | 1.728 | 1.658 | 27.328 | — | 0.046* | 0.631 |
| Base64-v2 | 2.026 | 1.884 | 14.673 | — | — | — |

Java/CPython are pinned controls with their own runtime and process-lifecycle
costs. Separate/adapted rows are not evidence of general language superiority.

## What the CPython result says

The measured runtime is specifically Homebrew **CPython 3.14.6**, built with
PGO (`--enable-optimizations`) and LTO. It is the normal GIL build; its
experimental JIT is unavailable/disabled, and it was not configured with the
optional tail-call interpreter. Calling the column simply “Python” would hide
material implementation and build facts.

The expectation that Python must be slow is not supported by this scorecard.
cREXX has the lower median on seven of the fourteen canonical CPython controls,
and CPython has the lower median on seven. On the directly equivalent common
four (Sieve, Permute, Bounce and Richards), cREXX is **1.214718x faster on
`rxtvm` and 1.250737x on `rxbvm`** geometrically. cREXX also leads on
Mandelbrot, Full Json and Queens.

CPython's large wins cluster in object/container-heavy work: Towers, Storage,
List, DeltaBlue, CD and Havlak, plus the floating-point NBody control. Its
ordinary lists, object references and allocator map directly to those upstream
programs, and operations such as list allocation/fill and `math` functions use
the mature C runtime. Several cREXX rows instead require extra ownership
objects, weak-reference arenas or stable integer handles into VM-value arrays.
Those are real product/capability findings, but they also explain why an
all-fourteen “language speed” aggregate would be misleading. The common-four
comparison is the clean answer to the general CPython question; the other rows
are individual capability and implementation diagnostics.

## Honest release findings

The scorecard exposes several candidates for a final-release performance
review. They are not benchmark correctness failures and must not be repaired
by changing required work:

1. CD is 11.3x slower than the CPython control and about 70x slower than the
   Java control. The cREXX row preserves the 200-frame algorithm through an
   indexed red/black representation and native `rxmath`. POSTPERF-05 already
   recovered about 47% by stopping unprofitable expansion, so current profiling
   must now separate indexed-map/value traffic, calls, allocation and native
   maths rather than assume one cause.
2. DeltaBlue is about 14.4x slower than CPython and 8.4x slower than Java in
   the disclosed stable-indexed constraint-graph adaptation. Object assignment
   copies values, forcing stable handles into planner-owned typed arrays;
   POSTPERF-05 recovered about 82%, but residual handle lookup, tagged dispatch,
   copying and call/frame cost still need current attribution.
3. Towers is about 45% slower than the qualified ooRexx object port and about
   14x slower than CPython. Retained Linux profiling identified 26.8 million
   value copies, 31.3 million clear/reset/destroy operations and 5.86 GB of
   allocation requests; a current profile must confirm how much remains.
4. Havlak is about 2.45x slower than CPython and 27x slower than Java. Its
   source has already passed the fair cREXX review, and its optimized image
   remaining slower than no-opt is preserved as a compiler investigation
   candidate rather than worked around in the test. Stable graph handles,
   ordinary VM-value `.int[]`, vector/set traffic and residual generated-code
   expansion are attribution candidates, not yet a single proved cause.
5. Storage is roughly 25x slower than the ooRexx/Java/Python controls and uses
   the already-disclosed wrapper required by the current nested-reference
   container surface. It is a capability-adaptation signal, not a common-work
   aggregate result.
6. List is about 2.1x slower than CPython. Its extra typed owning arena is
   required because Level B references are weak; this is known extra ownership
   work and belongs with the future ownership/container decision, not a
   benchmark-specific shortcut.
7. NBody is 2.6-2.7x slower than CPython despite both rows using native square
   root. Current profiling must separate float/value and attribute traffic from
   the small native-math boundary before selecting any mechanism.

NBody is also behind the Java/Python controls, but it is about 16x faster than
the qualified ooRexx decimal/native-math adaptation. Permute is the material
genuine-NetRexx common-lane deficit. These findings should drive any bounded
pre-release investigation; the current scorecard itself remains frozen.
The resulting roadmap-only attribution queue is recorded in
[`performance/ROADMAP.md`](../../ROADMAP.md); no production optimization is
authorized by recording it.

## RexxCPS and cREXX controls

| Runtime | Median native rate | n | Disposition |
| --- | ---: | ---: | --- |
| cREXX `rxtvm` | 44.494834 MCPS | 10 | stable |
| cREXX `rxbvm` | 44.155513 MCPS | 10 | stable |
| ooRexx 2.2 | 39.328025 MCPS | 10 | stable |
| Regina 2.2 | 32.589867 MCPS | 10 | stable |
| genuine NetRexx 2.2n | 46.538486 MCPS | 10 | stable disclosed adaptation |

The separate cREXX capability controls are stable:

| Control | `rxtvm` | `rxbvm` |
| --- | ---: | ---: |
| JSON legacy parser | 1.328 s | 1.360 s |
| JSON construct/validate | 1.343 s | 1.314 s |
| JSON parse-once/query | 1.187 s | 1.166 s |
| PARSE frozen | 0.415 s | 0.415 s |
| PARSE generic | 2.997 s | 3.008 s |

## Noise disposition

After the single append, 62 timing cells are stable at `n=10` and 27 remain
flagged at `n=20`. Most flagged cells are subsecond process-startup controls.
The material aggregate members needing the caveat are Sieve `rxbvm` (0.50%
relative MAD but one slow outlier), genuine-NetRexx Permute (0.62% relative
MAD) and Richards `rxbvm` (2.20% relative MAD). No sample was discarded and no
second append was taken.

## Lifecycle/startup diagnostic

The corrected lifecycle runner retained 20 samples per phase after one append.
Every phase remains flagged by the strict short-process rule, so this table is
diagnostic and is not pooled with steady-state timing.

| Runtime/VM | Phase | Median |
| --- | --- | ---: |
| cREXX shared | compile | 0.110678 s |
| cREXX shared | assemble | 0.015415 s |
| cREXX `rxtvm` | load/first result | 0.015714 s |
| cREXX `rxbvm` | load/first result | 0.014081 s |
| ooRexx | translate | 0.015823 s |
| ooRexx | load/first result | 0.015362 s |
| genuine NetRexx | compile | 0.440327 s |
| genuine NetRexx | load/first result | 0.031035 s |

## Peak RSS

RSS is separate from timing. Eighty-seven cells are stable at four
observations. Genuine-NetRexx Permute and RexxCPS remain noisy after a
ten-observation append (`n=14`) and are not treated as stable memory claims.

| Cell/group | Median peak RSS | Disposition |
| --- | ---: | --- |
| common cREXX rows | 14.7-16.3 MiB | stable |
| cREXX CD | 26.8-27.0 MiB | stable |
| cREXX DeltaBlue | 41.4-41.6 MiB | stable |
| cREXX Havlak | 91.7-91.9 MiB | stable |
| cREXX Storage | 198.4-198.5 MiB | stable adaptation cost |
| genuine-NetRexx Sieve | 409.0 MiB | stable |
| genuine-NetRexx Queens | 425.2 MiB | stable |
| genuine-NetRexx Permute | 171.8 MiB | noisy; 595% span |
| genuine-NetRexx RexxCPS | 525.7 MiB | noisy; 55.6% span |

## Current artifact inventory

The current fresh build has a bounded 50-artifact size/hash inventory in
`artifacts.csv`; no recursive historical checksum forest was created.

| Artifact | Size |
| --- | ---: |
| `rxc` | 3,469,248 bytes |
| `rxas` | 779,192 bytes |
| `rxlink` | 162,568 bytes |
| `rxdas` | 212,248 bytes |
| `rxvm` / `rxbvm` | 1,379,592 bytes each, identical hash |
| `rxtvm` | 1,396,232 bytes |
| `library.rxbin` | 720,593 bytes |

The largest timed cREXX generated forms are DeltaBlue RXAS (312,155 bytes),
CD RXAS/RXBIN (302,777/109,113 bytes), DeltaBlue RXBIN (102,342 bytes) and
Havlak RXAS/RXBIN (216,382/78,310 bytes). These are current facts, not a
regression claim without a version-matched size baseline.

## DECIMAL and concurrency dispositions

DECIMAL remains a separate, already-completed current-product decision. The
current `mc_decimal` L1 arithmetic medians are 2.690296 s at Common-18 and
3.286608 s at Classic-9. Tuned decNumber showed no credible headroom;
decQuad was 44.40%/64.76% slower in adapter arithmetic; and libmpdec was
41.66%/26.28% slower in formal L1 adapter arithmetic. The decision remains
**retain `mc_decimal`**, with the negative lessons retained so the experiments
are not repeated without a new trigger.

The unchanged Mac concurrency timing replay remains explicitly waived. Mac
QA-B correctness/packaging and the retained performance disposition stand;
Windows QA-D remains the prior completed native qualification and was not
repeated. Linux has a strong local repair-validation record (2,207/2,207 plus
install/package proof), but its own evidence explicitly says it is not the
formal clean exact-commit QA-C runner result. Therefore formal QA-C on
`81f15918676d92a7d3e88954d94779ed759e9db8` remains the only uncompleted
Stage 5 exit condition; no Linux host was attached to this task.

## Evidence map

- `PRE-RUN.md` and `POST-RUN.md`: host, power and runtime provenance.
- `manifest-mac-v1.txt`: exact 89-cell command/work contract.
- `qualification/`: 178 exact-work qualification observations.
- `formal-timing/`, `noise-append/`, `combined-timing/`: timing samples and
  consolidated summaries.
- `rss/`, `rss-noise-append/`, `combined-rss/`: RSS samples and summaries.
- `lifecycle/`: corrected current dual-engine lifecycle diagnostic.
- `artifact-manifest-mac-v1.txt` and `artifacts.csv`: bounded current artifact
  inventory.
- `named-comparisons.csv`: explicit versioned aggregate and RexxCPS ratios.

This is a valid Apple pre-release observation for the frozen candidate, not a
cross-platform release claim until formal Linux QA-C completes.
