# NUMERIC-01 first profiling-off Release verdict

Status: **complete; Adrian accepted candidate C and closeout passed**

This bundle retains the bounded A/B/C verdict for the approved native Level B
numeric surface and the candidate RexxCPS typing correction. Correctness,
Classic REXX fidelity and measured performance are separate verdicts. The A,
B and C sources are different benchmark revisions and are not one formal
cross-runtime baseline.

## Starting point and build

- Repository: `/Users/adrian/CLionProjects/CREXX`
- Branch and source commit: `develop` at
  `e23d44e58ce597720bb4437b1b2ee5aef6be4eee`, exactly equal to
  `origin/develop` before the spike
- Dirty scope at measurement: the provisional Level B functions, focused
  tests, benchmark-purity fixes, live roadmap/worklist and this evidence
- Product: ordinary `CMAKE_BUILD_TYPE=Release`,
  `CREXX_VM_PROFILING=OFF`, retained source/TRACE metadata
- Build command: `cmake --build cmake-build-release --parallel 10`
- Result: pass, full Release build completed

[`host-and-build.txt`](host-and-build.txt) retains the host, CPU, toolchain,
generator, cache options and exact dirty-worktree scope.

## Candidate cells

| Cell | Source contract | Purpose |
| --- | --- | --- |
| A | current canonical cREXX RexxCPS, 18-digit generated context | exact-hash control |
| B | explicit 9 digits plus honest variable/expression types | isolate the Classic-fidelity/type correction |
| C | B plus `floattrunc` and two `floatformat` calls | isolate the approved typed BIF substitutions |

Canonical and opaque sources were generated and checked as a pair in every
cell. [`candidate-sources.patch`](candidate-sources.patch) reconstructs B from
the versioned A sources and C from B. [`artifact-hashes.csv`](artifact-hashes.csv)
records all source, optimized/unoptimized RXAS and RXBIN SHA-256 values.

The optimized canonical images timed below were:

| Cell | Source SHA-256 | RXBIN SHA-256 |
| --- | --- | --- |
| A | `91fd5380346bd2ce247b73c9373783cea9d26ee4c8cc23bce9dec6d5c15425f8` | `3af6ad415dfbba2f45e1ede1e9963defc19b18a95a10a2279e1e22a3478e19a1` |
| B | `7ab584607948cd95766d342c24938ae65f69ee610f0f4869dc7df55b71dd90ad` | `1afdde6051c12f5bb6187d5036d0de9c22d62bd5ca2e49de074db324d6afcfb0` |
| C | `f234b6f1da71c207c0ef157232a8831eaa78030c6ad8c0ac399bb48abd606e76` | `ed63e083ae1c36b52dd62c8f184a48641ff9145dffb5c29de9e009452541da57` |

## Minimum correctness proof

- `cmake --build cmake-build-release --target library-rexx --parallel 10`:
  pass.
- Direct optimized and unoptimized `ts_typed_numeric`: pass.
- `ctest --test-dir cmake-build-release --output-on-failure -R
  '^ts_(abs|min|max|min_max|sign|trunc|format|typed_numeric)_(noopt|opt)$'`:
  17/17 pass, covering the existing decimal contracts and every new signature.
- `ctest --test-dir cmake-build-release --output-on-failure -R
  '^(benchmark_awfy_bounce_(noopt|opt)|nr03_evidence_tool_selftest_(noopt|opt))$'`:
  5/5 pass including the fixture.
- B and C canonical and opaque images, optimized and unoptimized, passed smoke
  on both `rxvm` and `rxbvm`. Canonical calibration selected effective count
  12 from initial count 1. Opaque A observed `A|1|69|1.22694`, matching the
  retained Classic/Regina result in the NR-02 evidence.
- The new typed function RXAS has the declared integer/float signatures,
  native operations and zero decimal operations or conversions. Tests include
  `INT64_MIN`, signed zero, NaN/infinities, homogeneous variadics, control
  errors and optimized/unoptimized behavior.

The standard cREXX benchmark audit also proved zero decimal metadata,
operations and calls in all non-RexxCPS workloads. Bounce's four integer
velocities now call `intabs`, and the evidence tool's two float outputs now call
`floatformat`; both changed artifacts have zero decimal paths. Separately
compiled optimized/unoptimized `run_cross_runtime.crexx` and
`run_lifecycle.crexx` also had zero decimal metadata, operations, conversions
and decimal BIF calls. Mandelbrot's binary64 arithmetic remains intentional.

## Generated-code and conversion verdict

[`numeric-contexts.txt`](numeric-contexts.txt),
[`numeric-main-metadata.txt`](numeric-main-metadata.txt),
[`rxas-opcounts.csv`](rxas-opcounts.csv) and
[`rxas-bif-calls.txt`](rxas-bif-calls.txt) retain the generated audit.

- A sets all benchmark and generated procedures to 18 digits.
- B and C set `main`, `cps_subroutine` and `fail` to 9 digits in canonical and
  opaque images. Compiler-generated trace handlers remain at 18; imported
  library modules retain their own contexts.
- The optimized A image has `ftod=4`, `ftoi=1`, `itof=2`, `stof=3`, `stoi=1`,
  `stod=4` and `dtos=3`.
- B makes `j`, its 1.1-step loop and compound arithmetic genuinely decimal,
  makes timer/rate arithmetic deliberately float, and keeps calibration counts
  integer. Its optimized image has `ftod=3`, `itof=1`, `stof=2`, `stoi=2`,
  `stod=4` and `dtos=5`; `ftoi` and `itod` are gone.
- C replaces the three remaining decimal BIF calls. Optimized and unoptimized
  C have `ftod=0`, with the same honest arithmetic as B.

Every remaining C conversion is semantic:

1. `stod`/`dtos` implement deliberate Classic compound-variable decimal
   arithmetic and decimal-to-text observations. The six additional no-opt
   `stod` operations create explicit decimal constants; optimization folds
   those constants to decimal literals.
2. `stof` converts the Classic string result of `time()` at the intentional
   cREXX binary64 timer boundary.
3. `itof` converts integral count/averaging denominators only where elapsed or
   rate division intentionally becomes binary64.
4. `stoi` converts command text to an integer and converts `floattrunc`'s
   Classic-compatible textual result to the integer calibration count.
5. Remaining `ftos`/`itos` operations are observations, output, keys or
   external text contracts, not arithmetic representation leaks.

No dynamic profile was needed: the static call sites prove the BIF
substitutions occur in calibration/output code, and the default count of 100
does not enter the `floattrunc` calibration branch.

## Release measurement method

[`run_release_verdict.zsh`](run_release_verdict.zsh) ran fresh processes
serially with zero warmups and three recorded samples per cell/VM. The three
orders give every candidate every position once and the same mean sequence
position:

- `rxvm`: `A B C`, `B C A`, `C A B`
- `rxbvm`: `C B A`, `B A C`, `A C B`

Each command was `/usr/bin/time -p VM OPTIMIZED_IMAGE
cmake-build-release/bin/library` with no benchmark arguments, preserving the
canonical default `100 x 100` workload. Benchmark-native CPS was parsed from
RexxCPS output; process `real` time was retained separately. All 18 samples
reported effective count 100 and PASS.

[`samples.csv`](samples.csv) and [`raw/`](raw/) are authoritative. The sample
CSV SHA-256 is
`b8d92bec3241ea9c78141b3c447301df458e6d57398868d809de7d4bca80923e`.
[`summary.csv`](summary.csv) is derived, not a replacement for raw data.

## Observed Release result

| VM | Cell | Native CPS mean (range) | Delta vs A | Process real mean (range) | Delta vs A |
| --- | --- | ---: | ---: | ---: | ---: |
| `rxvm` | A | 1,132,514 (1,122,694–1,146,742) | — | 8.847 s (8.74–8.92) | — |
| `rxvm` | B | 1,158,309 (1,140,532–1,175,228) | +2.278% | 8.647 s (8.52–8.78) | -2.261% |
| `rxvm` | C | 1,150,869 (1,129,725–1,180,035) | +1.621% | 8.707 s (8.49–8.87) | -1.583% |
| `rxbvm` | A | 1,095,915 (1,075,992–1,113,945) | — | 9.140 s (8.99–9.31) | — |
| `rxbvm` | B | 1,105,751 (1,098,914–1,118,543) | +0.898% | 9.060 s (8.95–9.12) | -0.875% |
| `rxbvm` | C | 1,107,633 (1,083,713–1,130,936) | +1.069% | 9.047 s (8.86–9.24) | -1.021% |

C versus B is -0.642% native CPS / +0.694% process time on `rxvm` and
+0.170% / -0.143% on `rxbvm`. These small opposing deltas and overlapping
ranges are a neutral/noisy BIF timing result, not evidence of a speedup. B's
mean improvement over A is an observation from three samples per cell; it is
compatible with removing hot conversion work, but the overlap and benchmark
revision change do not support a broader performance claim.

## Separate verdicts and recommendation

- **Semantic/correctness:** pass for the bounded focused proof.
- **Classic fidelity:** C is better than A. Genuine clause-workload arithmetic
  is decimal under explicit digits 9; integral control remains integer; timer
  and rate calculations are an explicit binary64 cREXX port adaptation. The
  opaque result matches the retained Classic result.
- **Generated types/conversions:** pass. C removes every avoidable
  float-to-decimal BIF crossing while retaining deliberate Classic
  string/decimal exercise.
- **Measured performance:** B/C versus A is a small positive observation on
  both VMs, but not a decisive speedup. C versus B is neutral/noisy because the
  substitutions are outside the default timed clause loop.
- **Independent API utility:** positive. The functions close a real Level B
  type surface, operate natively, preserve decimal contracts, and already
  remove accidental decimal paths from Bounce and benchmark evidence tooling.

Recommendation at the first gate was to keep the public typed surface and the
standard-benchmark purity fixes; accept C as the RexxCPS semantic/type direction
without claiming a typed-BIF timing win. No D cell is warranted because B/C
already isolates all three substitutions and their placement explains the null
timing result. Adrian accepted that recommendation.

## Accepted 2.2d closeout and exact-hash result

Accepted C is now the canonical and opaque RexxCPS 2.2d source revision. The
candidate version/provenance strings were replaced together with the benchmark
adaptation documentation; that expected text-only change gives the final source
and image new hashes while preserving C's arithmetic/type implementation.

| Final artifact | SHA-256 |
| --- | --- |
| canonical source | `2970c3d73fe2537ec8f81295c585495c4668b442d5b9a2335b1ee453a13bbdd6` |
| opaque source | `72a7c2d1d284900032078f19f8b92dd1d1d852300fe628492abdec208d8260a3` |
| canonical optimized RXAS | `aba5b72b9e7d9654af4d3c9407a6aab936d161157061998f3eb6ee08beb49c13` |
| canonical optimized RXBIN | `c885dcf9d6a3818119757b9cafed892ec9dd5a9a96bd6d2dd9fcc23fc68495a9` |

[`run_accepted_baseline.zsh`](run_accepted_baseline.zsh) ran one warmup plus
three recorded canonical-default samples per VM on the final optimized image.
[`accepted-c/samples.csv`](accepted-c/samples.csv) retains native CPS and
process time separately; [`accepted-c/summary.csv`](accepted-c/summary.csv) is
derived.

| VM | Native CPS median (range) | Process real median (range) |
| --- | ---: | ---: |
| `rxvm` | 1,145,721 (1,138,702–1,172,036) | 8.74 s (8.55–8.80) |
| `rxbvm` | 1,114,685 (1,098,014–1,132,910) | 8.99 s (8.84–9.12) |

Final validation:

- Release focused numeric checks: 17/17 pass.
- Release Bounce, evidence-tool and canonical/opaque RexxCPS checks: 9/9 pass.
- Final canonical and opaque optimized/no-opt RXAS: authored procedures use
  digits 9 and `ftod=0`, `itod=0`, `ftoi=0`.
- Full Debug build: pass.
- Full Debug CTest with `--parallel 30`: 1,851/1,851 pass in 285.71 seconds.
- All eleven public functions have matching Markdown, RexxDoc return contracts
  and CMake wiring.

Disposable scratch candidates and temporary audit builds were removed after
their reconstructing patch, hashes, generated audit and raw comparison samples
were retained here. Historical 2.2c and candidate A/B/C measurements remain
separate from the accepted 2.2d baseline. No commit or push was made.
