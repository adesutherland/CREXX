# NR-07 first Release verdict

Status: **first timing result not reproduced; no practical win; production candidate rejected and removed**.

## Scope and provenance

- Source branch/HEAD: `develop` at `5e5e3b397`, with NR-06 scalar copying
  removed and only the NR-07 direct integer compare-to-branch lowering active
  in production code. The focused tests and both verdict bundles remain
  uncommitted.
- Host: Darwin 25.5.0 arm64, Apple M5, 10 logical CPUs.
- Toolchain: Apple clang 21.0.0, CMake 4.3.2, Ninja 1.13.2,
  `CMAKE_BUILD_TYPE=Release`, `CREXX_VM_PROFILING=OFF`.
- Product build: ordinary full incremental profiling-off Release build,
  `cmake --build cmake-build-release -j 32`, completed successfully.
- Workload: canonical `tests/benchmarks/rexxcps_levelb.crexx`, optimized,
  default `100 x 100` contract with no argv, linked and stripped.
- Lifecycle: serial process runs; one warmup and three recorded samples per
  image/VM cell. `rxvm` ran baseline then candidate; `rxbvm` ran candidate then
  baseline as an order control.
- Correctness: all 16 executions exited zero and contained both
  `REXXCPS-PROVENANCE contract=canonical-default argv_count=0` and
  `PASS: RexxCPS 2.2c cREXX port`.

The retained baseline RXAS was generated before the batch by the pre-change
Release compiler. The current frozen Release compiler reproduces the earlier
validated NR-07 candidate RXAS byte-for-byte. The baseline contains 138
materialized integer comparisons and no direct compare branches; the candidate
contains 20 materialized comparisons and 118 direct compare branches.

Both main modules were assembled and linked with the same current Release
`rxas`, `rxlink`, and `library.rxbin`. The common library therefore cancels
from this paired comparison, which isolates the 118 changed RexxCPS-module
sites rather than claiming a full historical-product comparison. The isolated
result is already materially adverse, so the first-verdict gate did not spend
another build cycle reconstructing an old library.

## Exact product and images

| Artifact | SHA-256 |
|---|---|
| Release `rxc` | `12fe5319431a6aedd83be0a8a4939e31fefcc331063a8fc784f475e1a5eb5845` |
| Release `rxas` | `b3b4976dbb1be6542f14a769f26b0f5871d14d46af0ff7cf502f9d0dac3279dc` |
| Release `rxlink` | `bce9dca81eebde348de51b0c3933d79509cdc42f86a03b64afb70b14c42aa771` |
| Release `rxvm` | `0175e2ecf78ca21bbb82bd340c47ac7b407550e14f37a51a349403271afe4125` |
| Release `rxbvm` | `db32ca8512257e23659daf235b10665c8cb9773429c00135ea48668196c00973` |
| Release `library.rxbin` | `eb2cade3004704eb0425ff3769cc2a06314a3566b944208b89831d2448cc1c7c` |
| Baseline RXAS | `075eb421a177a8e2ff371a5c7d650cdbda2ed806cffc2e5ce7221a74f5136109` |
| Candidate RXAS | `85353aa284352672d0fe6ae815e41b0926d8167018680d8cdef3c2eecd80fe0b` |
| Baseline linked image | `6ef29e4135f67752d1b36c74e5215413294f8610f8e06af9d42f425e3f65b423` |
| Candidate linked image | `d1f87d2a735f0bc504a057818b7f98403946aaa5b0a53ad1fda48ff701367d8f` |

## Release timing result

Higher benchmark-native CPS is better. Lower process elapsed time is better.

| VM | Baseline median CPS | Candidate median CPS | CPS change | Baseline range | Candidate range |
|---|---:|---:|---:|---:|---:|
| `rxvm` | 1,003,222 | 891,473 | -11.139% | 936,360-1,036,220 | 825,144-898,616 |
| `rxbvm` | 805,410 | 760,524 | -5.573% | 768,694-820,215 | 753,936-791,258 |

| VM | Baseline median elapsed | Candidate median elapsed | Elapsed change | Baseline range | Candidate range |
|---|---:|---:|---:|---:|---:|
| `rxvm` | 9,977.491 ms | 11,229.718 ms | +12.551% | 9,660.370-10,689.528 ms | 11,138.989-12,130.473 ms |
| `rxbvm` | 12,427.957 ms | 13,160.919 ms | +5.898% | 12,204.107-13,021.750 ms | 12,650.309-13,277.259 ms |

The candidate is slower in both VM modes, including the reversed-order
`rxbvm` control. NR-07 is therefore not an accepted performance win in its
current form. No broad CTest, sanitizer, portfolio expansion, opcode-handler
investigation, or follow-on rework was performed after this verdict.

A later round-interleaved comparison did not reproduce this large regression,
but found no practical gain from either direct conditions or the specialist
loop extension. Adrian rejected the added compiler complexity. The production
lowering and its dedicated test registrations were subsequently removed; the
exact rejected production patch is retained here.

Raw stdout/stderr, exact argv, timestamps, versions, native metrics and process
elapsed samples are retained under `cells/`. `provenance.txt`,
`sha256sums.txt`, `release-build.log`, `build-images.log`, and the exact RXAS
and RXBIN images retain the build and comparison boundary.
