# CRI-09 retained pre-edit Release baseline

Status: frozen before the first `CAP-01-J01` production edit

Date: 2026-07-30

## Product identity

- Source branch/HEAD: `develop` at
  `d78c6fcfa81ef03fdbea65ff9cc39ed99e8716bf` with the already retained
  integration-programme worktree.
- Pre-edit `rxjson.crexx` SHA-256:
  `82a3d16989b0c289ff64634f1b1c4c011cfab1e0c0e9796840a163b839e91880`.
- Benchmark source SHA-256:
  `9cccc73717cf1574878ce72553b4c84fb14e34f4d9fcd0fd2c993d144ccfefc0`.
- Dedicated build:
  `/tmp/crexx-cri09-release-baseline.rxGv7t/build`.
- Configuration: Ninja, `Release`, `CREXX_VM_PROFILING=OFF`,
  `ENABLE_PARSER_MODE=OFF`, `BUILD_TESTING=ON`.
- Host/compiler: Darwin arm64 25.5.0; Apple clang 21.0.0; CMake 4.3.2;
  Ninja 1.13.2.
- Linked benchmark SHA-256:
  `acd8a82fd67c65816f5b652958ce89dba34c1b4cf94cfa291e09c753888bfff4`.
- `library.rxbin` SHA-256:
  `8a74c37c7917628814cea75a1aee3cdf757a357f958ddf74c9ca1a2ff61aacd2`.
- `rxvm` SHA-256:
  `b1bd31897ac26a4378f52f9498999a7b0b1d32c91443ce908c36ecadf888cb9e`.
- `rxbvm` SHA-256:
  `92bcf6d11bb9c393b7116ac04565f641ea708e7ed9578b4dde31936c77d5538a`.

## Exact build and link commands

```sh
cmake -S /Users/adrian/CLionProjects/CREXX \
  -B /tmp/crexx-cri09-release-baseline.rxGv7t/build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCREXX_VM_PROFILING=OFF \
  -DENABLE_PARSER_MODE=OFF -DBUILD_TESTING=ON

cmake --build /tmp/crexx-cri09-release-baseline.rxGv7t/build \
  --target rxjson_parser_compare_opt_artifact rxvm rxbvm --parallel 10

/tmp/crexx-cri09-release-baseline.rxGv7t/build/bin/rxlink -s \
  -o /tmp/crexx-cri09-release-baseline.rxGv7t/rxjson_parser_compare_baseline \
  /tmp/crexx-cri09-release-baseline.rxGv7t/build/tests/performance/rxjson_parser_compare_opt.rxbin \
  /tmp/crexx-cri09-release-baseline.rxGv7t/build/bin/library.rxbin
```

The benchmark uses its maintained defaults: 60 rows, 30 iterations, and a
4,394-byte payload. Each VM received three unrecorded warmups followed by 12
recorded serial samples. Raw sample SHA-256:
`5cef9563ceec9cad4846e30a7d898c9fb70b2abd623e995e8efd1d5e422de708`.

## Retained medians

| VM | `valid` | deep get | tail get | count | members |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | 4,271.5 us | 4,487.5 us | 4,405.0 us | 7,756.0 us | 4,081.5 us |
| `rxbvm` | 5,093.5 us | 5,621.0 us | 5,527.0 us | 10,200.0 us | 5,421.5 us |

Raw logs remain outside the repository at
`/tmp/crexx-cri09-release-baseline.rxGv7t/`: `configure.log`, `build.log`,
`link.log`, `raw-samples.log`, the linked benchmark image, and the complete
dedicated build tree. No normal-prefix install or hosted service was used.

## Predeclared candidate rule

Focused correctness must pass before measurement. A retained document's parse
plus 30 representative indexed accesses must take at most 50% of the matched
pre-edit 30-call repeated-parse workload on both optimized VMs, neither VM may
show an optimizer-induced inversion, and existing one-shot compatibility
operation medians must remain within 25% of this product. Noisy container
extraction must replace the reproduced 4,161-parser loop with a structural scan
and one strict parse of the returned document. A miss is reported as a failed
or mixed first verdict; the workload may not be weakened after seeing results.
