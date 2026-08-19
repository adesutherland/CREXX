# POSTPERF-01 Full AWFY Json qualification

Date: 2026-08-17

Status: **qualified cREXX reserve lane; no aggregate or cross-language claim**.

## Result

The separately named `awfy_json.crexx` benchmark parses the exact 25,820-byte
minified RAP payload from `smarr/are-we-fast-yet` commit
`74306fec151070fd07157cefeacf19e7e0bcdc89`. Every iteration verifies a JSON
object root, object member `head`, array member `operations` and exactly 156
operations. The complete optimized/unoptimized by product/concrete-VM
correctness matrix passes 6/6.

The cREXX source uses the supported indexed `.jsondocument` implementation
rather than reproducing AWFY's minimal-json object graph. It is therefore a
`standard-library-indexed-document` adaptation and remains outside the v2
common aggregate and every cross-runtime aggregate.

## Source and generated-code boundary

- fixture: 25,820 bytes, SHA-256
  `8f84f5fdc609a6d7179089249212a39588030852719d951db2d178820b70a7d8`;
- optimized source RXAS: 194 instructions, three `stobin` and three inlined
  `blen` operations, no retained `binlength()` call;
- unoptimized source RXAS: 173 instructions, three normal `stobin` plus
  retained `binlength()` calls;
- optimized source retains one `.jsondocument` factory call and seven indexed
  document method calls; the library implementation is the benchmarked
  substrate, not hidden setup;
- the stripped linked image contains 11,796 instructions across the benchmark
  and library, with the benchmark main still carrying its three `stobin` and
  three `blen` operations.

No opaque or optimizer-resistant variant is needed: the exact payload is read
from a runtime path, its document construction occurs inside the repeated
loop, and every repeated result is observed. The immutable fixture is loaded
once before that loop, matching the benchmark boundary while process timing
still includes file loading and VM startup.

## Bounded Release pilot

The maintained Level B process-smoke runner was extended with the explicit
non-default name `awfy-json`. Each sample executed 50 complete parse/verify
iterations. One warm-up and five serial recorded samples were taken per cell:

| Runtime | Role | Median | Range |
| --- | --- | ---: | ---: |
| `rxvm` | product, selected `rxbvm` | 97.783 ms | 96.693–99.269 ms |
| `rxtvm` | concrete-engine control | 99.979 ms | 98.471–101.011 ms |

`rxbvm` was not timed again because it is byte-identical to the selected
`rxvm`. These five-sample, sequential cells are bounded orientation only; they
are not a formal baseline, are not used for an engine claim, and contribute no
aggregate. Raw warm-up and recorded rows are retained under `pilot/`.

## Compiler repair discovered during qualification

The first optimized build exposed a compiler defect in conversion-bearing
inline argument binding. The accepted repair and its mandatory first ordinary
Release verdict are retained separately in
[`2026-08-17-postperf-01-awfy-json-compiler-repair-first-release-verdict`](../2026-08-17-postperf-01-awfy-json-compiler-repair-first-release-verdict/README.md).
The repair preserves inlining, restores the required string-to-binary
conversion and leaves ten established representative RXAS images byte-exact.

## Evidence map

- `artifacts.csv`: source, fixture, RXAS, RXBIN, linked-image and tool hashes;
- `fixture-validation.txt`: exact file and observable JSON contract;
- `linked-image-inspection.txt`: optimized/unoptimized and final-linked shape;
- `correctness-results.csv`: opt/no-opt by product/concrete-VM results plus
  runner coverage;
- `pilot/`: maintained-runner raw and summary CSVs;
- `provenance.txt`: host, power, build and controlled dirty-scope facts;
- `COMMANDS.md`: replay commands and interpretation boundaries;
- `VALIDATION.md`: post-acceptance closeout QA;
- `checksums.sha256`: recursive evidence hashes.
