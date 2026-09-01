# Current performance results and release closeout report

Date: 2026-08-19

Status: **Apple pre-release scorecard complete; final Linux verification
pending**

## Executive summary

The current cREXX performance position is strong overall. All 89 tested cells
produced the expected result, and the main comparable Rexx score shows cREXX
well ahead of ooRexx. cREXX is also ahead of genuine NetRexx on the aggregate
of the four workloads that can currently be compared fairly, although NetRexx
wins Permute individually.

The broader results are deliberately not presented as “cREXX wins every
benchmark”. Modern CPython is competitive: cREXX is faster on seven of the
fourteen Python controls and CPython is faster on seven. The workloads where
cREXX is weaker provide useful evidence about object handling, containers,
ownership and some complex programs.

These findings create two kinds of further work:

- **Release activity:** complete the formal Linux check and review the most
  material weak results—CD, DeltaBlue, Towers and Havlak—to decide whether a
  safe, general and worthwhile improvement belongs in this release.
- **Next-release work:** carry broader language and runtime opportunities,
  including Storage/List ownership, NBody and Permute, into the roadmap after
  Release 1. Any release-review item that does not justify a bounded change
  also moves to this next-release queue.

The scorecard does not itself authorize another optimization programme, and
none of these performance findings is currently a correctness defect or an
automatic release blocker. A correctness defect discovered during release
work remains a mandatory fix.

## Headline scorecard

The two cREXX columns are the direct-threaded (`rxtvm`) and portable
(`rxbvm`) execution engines. A result above 1.0 means cREXX completed the
agreed work faster.

| Comparison | Direct engine | Portable engine | Reading |
| --- | ---: | ---: | --- |
| cREXX versus ooRexx, common five | **5.47x** | **5.68x** | Strong overall cREXX result across Sieve, Permute, Bounce, Richards and Base64. |
| cREXX versus genuine NetRexx, common four | **1.32x** | **1.36x** | cREXX leads overall, although NetRexx remains faster on Permute. |
| cREXX versus CPython, equivalent common four | **1.21x** | **1.25x** | cREXX has a roughly 21–25% overall advantage on the cleanest common comparison. |

## RexxCPS scorecard

RexxCPS is reported separately in its own native unit: millions of nominal
Rexx clauses per second (MCPS). Higher is better.

| Runtime | Result | Comparison with cREXX direct | Comparison with cREXX portable |
| --- | ---: | ---: | ---: |
| cREXX 2.2d, direct (`rxtvm`) | **44.494834 MCPS** | — | — |
| cREXX 2.2d, portable (`rxbvm`) | **44.155513 MCPS** | — | — |
| ooRexx, canonical 2.2 | 39.328025 MCPS | cREXX is **13.14% faster** | cREXX is **12.27% faster** |
| Regina, canonical 2.2 | 32.589867 MCPS | cREXX is **36.53% faster** | cREXX is **35.49% faster** |
| genuine NetRexx 2.2n | 46.538486 MCPS | cREXX is **4.39% slower** | cREXX is **5.12% slower** |

This is a strong result: both cREXX engines are ahead of canonical ooRexx and
Regina and are close to the genuine NetRexx result. RexxCPS nevertheless stays
outside the common-workload aggregate because cREXX 2.2d and NetRexx 2.2n are
documented language adaptations rather than one identical source form. Keeping
it separate makes the comparison visible without overstating equivalence.

## Full timing view

These are median elapsed times in seconds, so lower is better. A dash means
that no fair, qualified result is available; it does not mean zero performance
or that the language is incapable. Java and CPython are controls and do not
enter the Rexx aggregates.

| Workload | cREXX direct | cREXX portable | ooRexx | genuine NetRexx | Java control | CPython control |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Sieve | 1.283 | 1.221 | 7.719 | 2.056 | 0.086 | 1.403 |
| Permute | 1.953 | 1.919 | 16.251 | 1.055 | 0.103 | 3.274 |
| Bounce | 1.830 | 1.774 | 4.481 | 1.964 | 0.068 | 1.872 |
| Richards | 0.333 | 0.327 | 1.833 | 1.084 | 0.056 | 0.387 |
| Towers | 1.744 | 1.772 | 1.204 | — | 0.045 | 0.123 |
| Storage | 1.356 | 1.353 | 0.053 | — | 0.043 | 0.042 |
| List | 1.270 | 1.264 | 4.661 | — | 0.057 | 0.599 |
| Mandelbrot | 0.179 | 0.170 | — | — | 0.060 | 0.462 |
| Full JSON | 0.045 | 0.042 | 0.076 | — | 0.052 | 0.059 |
| DeltaBlue | 0.448 | 0.457 | — | — | 0.053 | 0.031 |
| CD | 4.797 | 4.774 | — | — | 0.068 | 0.424 |
| Havlak | 3.606 | 3.604 | — | — | 0.133 | 1.474 |
| Queens | 1.302 | 1.284 | 10.798 | 1.933 | 0.074 | 1.716 |
| NBody | 1.728 | 1.658 | 27.328 | — | 0.046 | 0.631 |
| Base64 | 2.026 | 1.884 | 14.673 | — | — | — |

Some very short-running measurements remained noise-labelled under the formal
rules. They are retained honestly in the detailed evidence and should not be
used to justify small performance claims. The broad conclusions above do not
depend on removing any sample.

## Questions arising from the scorecard

### What do “failed” and “unresolved” mean?

They are different:

- **Failed** means a completed port reached building, execution or formal
  checking but did not pass its declared gate.
- **Unresolved** means no qualified port was completed, while the review also
  found no evidence that the language could not support it.

All current heavy missing ooRexx and NetRexx ports are unresolved, not failed.
For example, ooRexx can express the mathematics and objects needed by CD, but
no complete, checked red/black-tree port was finished. Genuine NetRexx Full
JSON is also unresolved: using Java collections to perform the main work would
make it a Java/JVM control rather than evidence of NetRexx language capability.

This is different from ooRexx Mandelbrot, where an executable attempt exists
but the arithmetic adaptation does not preserve the canonical result. That
cell is therefore “not comparable”. NetRexx Towers also executes, but its main
work uses primitive Java representations, so it is reported as a Java/JVM
control rather than genuine NetRexx capability.

### Is CPython the same as Python, and should Python be expected to be slow?

Python is the language; CPython is the particular implementation measured.
The control was an optimized CPython 3.14.6 build with profile-guided and
link-time optimization. It used the normal interpreter rather than the
experimental JIT.

The old assumption that “Python is slow” is not a useful reading of these
results. cREXX and CPython each have the lower median on seven of the fourteen
controls. cREXX leads on the clean common-four comparison and on Mandelbrot,
Full JSON and Queens. CPython leads particularly strongly on several
object-and-container workloads.

That pattern is informative. Python's ordinary lists and object references
map naturally onto those benchmark algorithms. Current cREXX sometimes needs
additional owner objects, arenas or numbered handles to express the same
relationships. Those differences are real product findings, but they also
mean that combining all fourteen rows into one “language speed” number would
be misleading.

### Where does cREXX currently struggle?

| Workload | Plain-language finding | Intended horizon |
| --- | --- | --- |
| CD | Its indexed map and object work is much slower than the Python and Java controls. The exact balance of copying, calls, allocation and mathematics still needs measuring. | Review during release; fix only if a safe, general and material change is identified. |
| DeltaBlue | Current value behaviour requires numbered handles and separate planning arrays for a complex object graph. | Review during release; otherwise carry forward. |
| Towers | Earlier evidence showed unusually high object copying, clearing and allocation. The current product needs confirming measurements. | Review during release; otherwise carry forward. |
| Havlak | This complex graph workload remains slower than Python, and optimized code is unexpectedly slower than the unoptimized form. | Review during release; preserve the honest benchmark. |
| Storage | The current language surface requires an extra owner object around each logical child array. This is known additional work. | Next release, with the broader ownership/container design. |
| List | Weak references require an extra ownership arena to keep list nodes alive. | Next release, with the broader ownership/container design. |
| NBody | cREXX is much faster than the ooRexx adaptation but behind CPython; floating-point, object and native-math costs have not yet been separated. | Next-release investigation. |
| Permute | cREXX beats ooRexx and CPython but remains behind genuine NetRexx on this recursive workload. | Next-release call/value investigation. |

The first four are **release reviews**, not promises to change production code
before release. The decision rule is deliberately conservative: make a
pre-release change only when current evidence identifies a general problem and
a bounded solution whose correctness and portfolio performance can be proved.
Otherwise preserve the finding and address it after Release 1.

## Release disposition

Before the performance closeout can be called complete:

1. run the formal exact-commit Linux verification against candidate
   `81f15918676d92a7d3e88954d94779ed759e9db8`;
2. perform the bounded release reviews above and decide whether each item is a
   release change or a next-release roadmap item;
3. finish the documentation and retained-evidence clean-down; and
4. keep the scorecard frozen unless a correctness or comparison problem
   requires a clearly versioned rerun.

The source candidate is beta 3 work in progress. The latest completed release
tag remains `v1.0.0-beta.2`; these results must not be described as a released
beta 3 result until the corresponding tag and release assets exist.

## Detailed sources

- [Stage 5 scorecard and retained evidence](evidence/2026-08-18-performance-closeout-stage5/README.md)
- [Portfolio and comparison contract](portfolio/manifest-v3.md)
- [Performance roadmap](ROADMAP.md)
- [Performance closeout plan](PERFORMANCE-CLOSEOUT-PLAN.md)
