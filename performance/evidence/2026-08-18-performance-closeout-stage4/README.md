# Performance closeout Stage 4 evidence

Status: **complete and green**

Date: 2026-08-18

## Outcome

Stage 4 froze a reviewed, correctness-qualified and calibrated portfolio-v3
input boundary. It did not run the formal Stage 5 scorecard.

The final cREXX source review covered all 14 AWFY lanes, Base64 v2, RexxCPS
2.2d and the retained JSON/PARSE/lifecycle and DECIMAL controls. The sources
retain their required algorithms, work, observable results and disclosed
adaptations. Havlak's flat insertion-ordered edge representation and lazy
predecessor allocation are fair Level B spellings of the same CFG/union-find
algorithm; its optimized/no-opt inversion remains visible rather than being
worked around in the benchmark.

## Correctness and runtime qualification

- full Debug CTest: 2,262/2,262 passed;
- final ordinary profiling-off Release build: passed;
- focused Release portfolio, Base64, JSON, RexxCPS, runner and structured-flow
  qualification: 56/56 passed;
- Release DECIMAL/RexxCPS integrity set: 82/82 passed;
- explicit optimized/unoptimized 14-workload cREXX matrix on `rxtvm` and
  `rxbvm`: 56/56 passed in `crexx-dual-vm-smoke.log`;
- ooRexx, Regina, genuine NetRexx and pinned canonical Java/CPython positive
  qualification: 47/47 passed in `external-runtime-smoke.log`; and
- ooRexx Mandelbrot canonical-size negative: failed as expected and retained
  as `not comparable`, not hidden as a missing result.

The external sources were built from a fresh clean AWFY checkout at
`74306fec151070fd07157cefeacf19e7e0bcdc89` and freshly compiled NetRexx
sources. Build identities are in `external-runtime-build.log`.

## Product defect found by the source review

The Havlak review exposed structured-control correctness defects rather than
a reason to weaken the benchmark. Early `RETURN`, labelled `LEAVE`/`ITERATE`,
block-expression result allocation and runtime `SIGNAL` could cross generated
loop/scope cleanup incorrectly; nested signal handlers could also remain
installed after control transferred out of a protected region.

The compiler and VM now emit/unwind the required cleanup and reserve result
registers only across real runtime-cleanup boundaries. Focused regressions and
the complete Debug/Release qualifications passed. The detailed repair record
is in
[`2026-08-18-stage4-structured-control-correctness`](../2026-08-18-stage4-structured-control-correctness/).
Proof-driven specialization of disabled-signal regions remains roadmap work;
there is no blanket compiler assumption that an ignored signal is unreachable.

## Diagnostic calibration

`performance-closeout-stage4-calibration-mac-v1.txt` ran zero warmups and two
observations for 32 cREXX cells: all 14 AWFY workloads, Base64 v2 and RexxCPS,
each on `rxtvm` and `rxbvm`. All cells passed. It is diagnostic, not formal
scorecard evidence.

The intended 1-2 second range covers most cells. CD was stable at
4.648/4.591 seconds and Havlak at 3.546/3.540 seconds for `rxtvm`/`rxbvm`.
Mandelbrot, Full Json and DeltaBlue were below 0.5 seconds; the small-sample
variability flags on Mandelbrot `rxbvm`, Full Json `rxbvm` and DeltaBlue
`rxtvm` are handled by Stage 5's ten observations. No catastrophic accidental
cREXX source overhead was found.

Files under `calibration/` retain the raw samples, outputs, capture manifest
and summary. Formal Stage 5 results must not be inferred from them.

## Frozen input

The authoritative source/capability/work contract is
[`portfolio/manifest-v3.md`](../../portfolio/manifest-v3.md). Java/JVM control
implementations are not reported as NetRexx capability, and unresolved ooRexx
or genuine-NetRexx ports remain explicit rather than being called unsupported.
