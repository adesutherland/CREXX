# BINARY-01 packed numeric class first Release verdict

## Scope and interpretation

This is the mandatory first profiling-off Release verdict for the initial
Level G `.packedfloat` and `.packedint` owners. It compares their imported and
inlined `get(index)` / `set(index, value)` surface with direct Level B packed
access in the same frozen product build. It also compares direct packed integer
access with encoded `<at..i64>` access as the machine-format ceiling control.

This is a narrow mechanism comparison, not a representative application or
whole-program claim. Allocation, initialization and final checksum validation
are outside the timed item loops.

## Provenance

- Source revision: `8f0dc5195b38bec3a2fe97b100e2373504b71a07` on
  `develop`, with the BINARY-01 implementation and focused tests present as
  the dirty-worktree scope retained in `provenance/pre-run.txt`.
- Host: Apple M5, Darwin arm64, macOS 26.5.2, Apple clang 21.0.0, CMake 4.3.2,
  Ninja 1.13.2.
- Power/state: AC power, low-power mode off, and no thermal or performance
  warning before or after the run. Exact state is retained in `provenance/`.
- Build: ordinary `Release`; `CREXX_VM_PROFILING=OFF`.
- Program: `tests/performance/packed_numeric_class_compare.crexx`, compiled in
  normal optimized mode.
- Product link: `rxlink -s` over the program, `library.rxbin`,
  `classlib.rxbin`, and `rxfnsg.rxbin`. Source/TRACE metadata is stripped from
  the timed image.
- VMs: the ordinary Release `rxbvm` and `rxtvm` concrete products, reported
  independently.

Build command:

```text
cmake --build cmake-build-release \
  --target performance_packed_numeric_class rxlink rxbvm rxtvm --parallel 10
```

Link command:

```text
cmake-build-release/bin/rxlink -s \
  -o performance/evidence/2026-08-21-binary01-packed-class-first-release-verdict/provenance/packed_numeric_class_compare_linked \
  cmake-build-release/tests/performance/packed_numeric_class_compare_opt.rxbin \
  cmake-build-release/bin/library.rxbin \
  cmake-build-release/bin/classlib.rxbin \
  cmake-build-release/bin/rxfnsg.rxbin
```

Cell command template:

```text
cmake-build-release/bin/<rxbvm|rxtvm> \
  performance/evidence/2026-08-21-binary01-packed-class-first-release-verdict/provenance/packed_numeric_class_compare_linked.rxbin \
  -a 200000 10 <variant>
```

## Sampling and correctness

Each cell performs 2,000,000 timed item operations. The run used one serial
warmup per cell followed by 12 serial recorded rounds. Even rounds reverse all
20 cells, so every cell occupies both its forward and reverse position six
times.

All 20 warmups and 240 recorded executions passed. All 260 retained outputs
contain the expected pass marker, none contains a failure marker, and every
variant has the expected stable checksum. No sample was removed.

Focused compiler evidence established before timing that source import and
RXBIN-only import (after RXDAS/reassembly) both remove the accessor call. The
packed operation names the receiver register directly, with no binary
`bcopy`, `link`, or `unlink`. A getter retains one scalar result copy plus its
receiver-initialization guard; a setter writes the receiver directly.

## Result

`summary.csv` reports medians and the median of 12 paired
comparator-time/candidate-time ratios. A ratio above 1 means the candidate is
faster; below 1 means it is slower.

### Level G wrapper versus direct Level B packed access

| VM | Kernel | Wrapper median | Direct median | Wrapper throughput | Paired ratio | Wrapper wins |
|---|---:|---:|---:|---:|---:|---:|
| `rxbvm` | float read | 22,432.5 us | 11,460.0 us | 89.2 M items/s | 0.512x | 1/12 |
| `rxtvm` | float read | 25,623.5 us | 10,334.5 us | 78.1 M items/s | 0.403x | 0/12 |
| `rxbvm` | integer read | 15,839.5 us | 11,921.0 us | 126.3 M items/s | 0.738x | 0/12 |
| `rxtvm` | integer read | 12,131.5 us | 10,325.5 us | 164.9 M items/s | 0.848x | 0/12 |
| `rxbvm` | float write | 20,751.0 us | 36,823.5 us | 96.4 M items/s | 1.752x | 12/12 |
| `rxtvm` | float write | 24,368.0 us | 38,303.0 us | 82.1 M items/s | 1.571x | 12/12 |
| `rxbvm` | integer write | 13,190.5 us | 19,121.5 us | 151.6 M items/s | 1.448x | 12/12 |
| `rxtvm` | integer write | 10,925.0 us | 21,437.5 us | 183.1 M items/s | 1.970x | 12/12 |

The inliner repair removes the prior catastrophic full-container
materialization: wrapper reads are now bounded scalar operations rather than
thousands of times slower. The remaining read cost is nevertheless material.
Relative to direct Level B packed reads, wrapper throughput is 48.850% lower
for `rxbvm` float, 59.683% lower for `rxtvm` float, 26.171% lower for `rxbvm`
integer, and 15.217% lower for `rxtvm` integer. Wrapper writes are faster than
the direct source comparator in all 48 paired write comparisons.

### Direct packed integer versus encoded i64

| VM | Kernel | Packed median | Encoded median | Packed throughput | Paired speedup | Packed wins |
|---|---:|---:|---:|---:|---:|---:|
| `rxbvm` | integer read | 11,921.0 us | 21,259.5 us | 167.8 M items/s | 1.802x | 12/12 |
| `rxtvm` | integer read | 10,325.5 us | 23,468.0 us | 193.7 M items/s | 2.279x | 12/12 |
| `rxbvm` | integer write | 19,121.5 us | 36,524.5 us | 104.6 M items/s | 1.910x | 12/12 |
| `rxtvm` | integer write | 21,437.5 us | 37,179.0 us | 93.3 M items/s | 1.744x | 12/12 |

This confirms that the packed integer primitive retains the same decisive
machine-format advantage previously established for packed float.

## Verdict and decision boundary

The underlying packed primitives, the no-copy class storage model, imported
setter lowering, and reference-lifetime contract are supported. The initial
Level G owners are functionally viable and their writes are performance-safe.

The recommended verdict is **rework the getter result path before calling the
Level G wrappers performance-qualified**. The emitted getter still performs a
receiver `assertinitialized`, `pget*`, scalar `icopy`/`fcopy`, and block-result
branch for every item. The next bounded experiment should test exact accessor
result-register coalescing and a proved redundant receiver-initialization guard
elision/hoist. Production code remains frozen pending Adrian's decision.

Broad Debug, sanitizer, install, startup/RSS/artifact and cross-platform
closeout remain consolidated at the end of RCC-5 and are not claimed here.
