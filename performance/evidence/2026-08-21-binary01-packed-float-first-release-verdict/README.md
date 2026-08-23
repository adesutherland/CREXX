# BINARY-01 packed-float first Release verdict

## Scope and interpretation

This is the mandatory first profiling-off Release verdict for the new
host-native `<packed..float>` access path. It is an intentionally narrow,
non-representative mechanism comparison, not a whole-program or portfolio
claim. The comparator is the existing canonical `<at..f64>` encoded binary
field path in the same frozen product build. Both variants address the same
zero-based logical items; the encoded variant performs its required byte-index
scaling and little-endian binary64 access.

The claimed block is the product-linked block in `raw/`. An earlier balanced
block using the compiler-optimized program plus separately loaded Release
library is retained under `unlinked-diagnostic/` and excluded from the verdict.

## Provenance

- Source revision: `8f0dc5195b38bec3a2fe97b100e2373504b71a07`
  on `develop`, with the complete BINARY-01 implementation and tests present as
  the dirty-worktree scope recorded in `provenance/pre-run.txt`.
- Host: Apple M5, Darwin arm64, macOS 26.5.2, Apple clang 21.0.0, CMake 4.3.2,
  Ninja 1.13.2.
- Power/state: AC power, low-power mode off, no thermal or performance warning
  before or after the run. Exact pre/post state is retained in `provenance/`.
- Build: ordinary `Release`; `CREXX_VM_PROFILING=OFF`.
- Program: `tests/performance/binary_packed_float_compare.crexx`, compiled in
  normal optimized mode and assembled as
  `binary_packed_float_compare_opt.rxbin`.
- Product link: `rxlink -s` over the program and Release `library.rxbin`.
  Source/TRACE debug metadata is therefore stripped. The linked image SHA-256
  is retained in `verification.txt`.
- VMs: the ordinary Release `rxbvm` and `rxtvm` concrete products, reported
  independently.

Build command:

```text
cmake -S . -B cmake-build-release -DCMAKE_BUILD_TYPE=Release -DCREXX_VM_PROFILING=OFF
cmake --build cmake-build-release --target rxc rxas rxlink rxbvm rxtvm performance_binary_packed_float --parallel 10
```

Link command:

```text
cmake-build-release/bin/rxlink -s \
  -o performance/evidence/2026-08-21-binary01-packed-float-first-release-verdict/provenance/binary_packed_float_compare_linked \
  cmake-build-release/tests/performance/binary_packed_float_compare_opt.rxbin \
  cmake-build-release/bin/library.rxbin
```

Cell command template:

```text
cmake-build-release/bin/<rxbvm|rxtvm> \
  performance/evidence/2026-08-21-binary01-packed-float-first-release-verdict/provenance/binary_packed_float_compare_linked.rxbin \
  -a 200000 10 <packed-read|encoded-read|packed-write|encoded-write>
```

## Sampling and correctness

Each cell performs 2,000,000 timed item operations after untimed buffer
allocation and initialization. The run used one serial warmup per cell followed
by 12 serial recorded rounds. Odd/even rounds reverse VM, read/write, and
packed/encoded order. `verification.txt` records the exact alternating
positions: each packed/encoded pair occupies both first and second position six
times.

All 8 warmups and 96 recorded executions passed. All 104 retained outputs have
the expected pass marker, none has a failure marker, and every paired checksum
matches: `1000000` for reads and `200019` for writes. No sample was removed.

## Result

`summary.csv` contains medians and the median of the 12 per-round encoded-time
to packed-time ratios:

| VM | Kernel | Packed median | Encoded median | Packed throughput | Paired speedup | Packed wins |
|---|---:|---:|---:|---:|---:|---:|
| `rxbvm` | read | 11,641.5 us | 22,996.0 us | 171.8 M items/s | 1.950x | 12/12 |
| `rxbvm` | write | 19,423.5 us | 38,333.5 us | 103.0 M items/s | 1.973x | 12/12 |
| `rxtvm` | read | 10,225.0 us | 25,841.0 us | 195.6 M items/s | 2.522x | 12/12 |
| `rxtvm` | write | 23,108.0 us | 42,196.5 us | 86.6 M items/s | 1.830x | 12/12 |

Observation: the selected packed primitive is 1.830x to 2.522x faster than the
encoded binary64 route in these four mechanism cells and wins all 48 paired
comparisons.

Inference boundary: this decisively supports retaining the host-native
primitive for numerical kernels. It does not yet qualify `.packedfloat`, packed
`rxstats`, startup/RSS/artifact costs, integer packed access, or representative
application performance; those remain later BINARY-01/RCC-5F gates after this
first verdict is accepted.
