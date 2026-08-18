# Performance closeout Stage 4 worklist

Status: **complete - portfolio-v3 frozen and green; Stage 5 authorized**

Authority: [`PERFORMANCE-CLOSEOUT-PLAN.md`](PERFORMANCE-CLOSEOUT-PLAN.md),
Stage 4

Started: 2026-08-18

## 1. Boundary

Stage 4 prepares, calibrates and correctness-qualifies the complete pre-release
test set. It may add or correct benchmark ports, improve cREXX benchmark
sources where the same work has an ordinary fair spelling, fix capability
labels, retain source provenance, qualify optimizer-integrity gates and run
small diagnostic pilots. It must not produce or publish the
two-warmup/ten-run pre-release scorecard; that is Stage 5.

The Mac concurrency replay is not part of this worklist. It was waived as
redundant for the unchanged qualified implementation and reopens only for a
relevant source change, a concrete evidence inconsistency or a new governed
performance question.

## 2. Gates

| Gate | Deliverable | Status |
| --- | --- | --- |
| S4-01 | Freeze pinned AWFY Java/Python provenance and qualify all 14 canonical controls | complete |
| S4-02 | Complete the final fair/idiomatic cREXX source review for all 14 AWFY lanes plus Base64, RexxCPS and capability controls | complete |
| S4-03 | Give every ooRexx and NetRexx cell an honest capability/result classification; add feasible missing ports | complete |
| S4-04 | Prepare RexxCPS whole-product, native CPS, decimal-string and compiler/RXAS optimizer-integrity gates | complete |
| S4-05 | Calibrate Stage 5 work and screen cREXX for catastrophic accidental source overhead; correct and requalify only fair source issues | complete |
| S4-06 | Run optimized/unoptimized cREXX correctness, both concrete VMs, cross-runtime port smoke tests and runner self-tests | complete |
| S4-07 | Freeze `portfolio/manifest-v3.md`, compact evidence and the exact Stage 5 input boundary | complete |

## 3. Canonical AWFY controls

The upstream source is `smarr/are-we-fast-yet` commit
`74306fec151070fd07157cefeacf19e7e0bcdc89`. The upstream Java and CPython
harnesses provide Bounce, CD, DeltaBlue, Havlak, Json, List, Mandelbrot,
NBody, Permute, Queens, Richards, Sieve, Storage and Towers.

All 28 Stage 4 correctness cells passed on 2026-08-18: one Java and one
CPython process per benchmark, with `inner=1` except CD's smallest published
case `inner=2`. These are qualification executions, not scorecard timings.
Stage 5 must use a new clean checkout at the same commit, record its source
hashes and keep Java and CPython out of every Rexx aggregate.

## 4. Cross-runtime disposition inventory

`qualified` below means a runnable correctness-qualified source is present;
it does not imply aggregate equivalence. `Java/JVM control` is never a
NetRexx capability result.

| Workload | ooRexx Stage 4 disposition | NetRexx Stage 4 disposition | Java/Python control |
| --- | --- | --- | --- |
| Sieve | qualified Classic procedural port | qualified NetRexx capability, decimal `Rexx` state | both canonical |
| Permute | qualified Classic procedural port | qualified NetRexx capability, decimal `Rexx` state | both canonical |
| Bounce | qualified ooRexx object port | qualified NetRexx capability, decimal `Rexx` state and NetRexx object surface | both canonical |
| Richards | qualified ooRexx state-machine port | qualified NetRexx capability, decimal `Rexx` state | both canonical |
| Towers | qualified ooRexx object port | not NetRexx capability - binary Java/JVM control | both canonical |
| Storage | qualified ooRexx object/array port; diagnostic adaptation | not NetRexx capability - binary Java/JVM control | both canonical |
| List | qualified ooRexx object port; cREXX remains a disclosed arena adaptation | not NetRexx capability - binary Java/JVM control | both canonical |
| Mandelbrot | ooRexx attempted; not comparable because its decimal/XOR adaptation does not retain the canonical 500/750 results | not NetRexx capability - binary Java/JVM control | both canonical |
| Json, full AWFY | qualified ooRexx `json.cls` DOM adaptation added in Stage 4 | failed/unresolved genuine NetRexx cell; any Java-collection parser is a Java/JVM control | both canonical |
| DeltaBlue | failed/unresolved; source/representation review found no language prohibition, but no qualified full port | failed/unresolved genuine NetRexx cell; canonical Java is not a substitute | both canonical |
| CD | failed/unresolved; ooRexx can express the maths and objects, but no qualified full red/black-tree port | failed/unresolved genuine NetRexx cell; canonical Java is not a substitute | both canonical |
| Havlak | failed/unresolved; ooRexx can express the graph algorithm, but no qualified full port | failed/unresolved genuine NetRexx cell; canonical Java is not a substitute | both canonical |
| Queens | qualified ooRexx object port added in Stage 4 | qualified NetRexx capability, decimal `Rexx` arrays, added in Stage 4 | both canonical |
| NBody | qualified ooRexx decimal/native-`rxmath` adaptation added in Stage 4 | failed/unresolved genuine NetRexx cell; canonical Java is not a substitute | both canonical |
| Base64 v2 | qualified ooRexx byte-string port | not NetRexx capability - primitive Java `byte[]`/`String` storage is material timed work | no upstream AWFY control |
| RexxCPS | canonical ooRexx 2.2 | qualified disclosed NetRexx 2.2n capability, reported at its native metric and never pooled | not applicable; Regina is the separate canonical Classic control |

The heavy missing ports are recorded as `failed/unresolved`, not `not
supported`. The review found no concrete ooRexx or NetRexx language/runtime
prohibition that would justify an unsupported claim. Stage 5 therefore emits
no invented result for those language cells and shows the canonical Java and
Python controls in their own columns.

## 5. Final cREXX source-review checklist

Each row must be checked against the pinned source or documented contract for
algorithm/work, observable result, current Level B facilities and any
necessary adaptation.

| cREXX source group | Required review outcome | Status |
| --- | --- | --- |
| Sieve, Permute, Bounce, Richards | common algorithms, work counts and deterministic results retained | complete |
| Towers | object/allocation algorithm retained; separate lane remains appropriate | complete |
| Storage | wrapper forced by the current nested-reference-container surface remains disclosed | complete |
| List | weak-reference arena ownership remains necessary and disclosed | complete |
| Mandelbrot | canonical source stays distinct from the hoisted diagnostic control | complete |
| Full Json | full fixture/result retained; indexed standard-library adaptation remains outside aggregates | complete |
| DeltaBlue, CD, Havlak | stable-handle/indexed adaptations retain algorithms; bounded/noncanonical test arguments remain explicit | complete |
| Queens | ordinary typed-array/recursive-search spelling retains ten searches per iteration | complete |
| NBody | object algorithm retained; `rxmath` native square root remains disclosed | complete |
| Base64 v2 | byte work, length, equality and checksum retained | complete |
| RexxCPS 2.2d | clause mix, canonical defaults, provenance, result observation and decimal/string work retained | complete |
| JSON/PARSE/lifecycle controls | identities and timing boundaries remain separate | complete |

Any source edit that changes work or the timing boundary creates a new
version. Ordinary spelling improvements that preserve both still require the
optimized/unoptimized and dual-VM gate before inclusion.

## 6. Calibration and cREXX outlier screen

Use a declared small-sample pilot to choose counts that make the fastest
formal Stage 5 cell approximately one second while bounding impractically slow
cells. Keep canonical harness arguments, adapted repetition counts and native
metrics explicit. The pilot may compare rough order of magnitude, but is not a
scorecard and must not be quoted as one.

For every adverse cREXX outlier:

1. reconfirm equal algorithm, work, result and timing boundary;
2. inspect whether current typed values, containers, references, string/JSON
   APIs, native library surface or allocation idioms remove accidental port
   overhead;
3. change only the benchmark source where equivalence is proved;
4. version any changed work or timing boundary; and
5. repeat opt/noopt, dual-VM, observable-result and calibration checks.

Do not turn a reserve adaptation into an aggregate-equivalent claim merely
because its source becomes faster. Any required production change is recorded
for the roadmap and stopped for separate approval.

## 7. RexxCPS and optimizer-integrity preparation

Stage 4 must qualify, without formal performance sampling:

1. canonical-default cREXX RexxCPS 2.2d on `rxbvm` and `rxtvm`;
2. official ooRexx RexxCPS 2.2, Regina control and disclosed NetRexx 2.2n;
3. the opaque A/B result-observation diagnostic;
4. the decimal-register integrity CTest;
5. the decimal-string family control with a bounded correctness count; and
6. equivalent optimized, optimized-compiler/RXAS-noopt and full-noopt images
   for the Stage 5 decimal scorecard.

The prepared full-work manifests remain Stage 5 inputs. Stage 4 does not run
their two-warmup/ten-observation campaigns.

## 8. Exit checklist

- [x] All cREXX reviewed sources pass optimized and unoptimized correctness.
- [x] Both concrete VMs pass the reviewed sources where their substrate is
  applicable.
- [x] All newly added ooRexx/NetRexx ports compile or run and report their
  expected result.
- [x] Every missing or control cell has an explicit honest label.
- [x] Canonical Java and CPython controls remain pinned and independently
  qualified.
- [x] Stage 5 work counts are calibrated and every catastrophic cREXX outlier
  has a source/equivalence disposition.
- [x] RexxCPS and optimizer-integrity gates pass.
- [x] Runner self-tests pass.
- [x] `portfolio/manifest-v3.md` and compact Stage 4 evidence agree.
- [x] No Stage 5 formal timing was run before the Stage 4 freeze.

Exit evidence:
[`2026-08-18-performance-closeout-stage4`](evidence/2026-08-18-performance-closeout-stage4/).
