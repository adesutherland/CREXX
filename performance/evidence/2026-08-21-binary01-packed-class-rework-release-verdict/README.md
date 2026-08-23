# BINARY-01 packed numeric class rework Release verdict

## Scope and interpretation

This is the profiling-off Release verdict for the approved exact-getter
rework. It uses the same program, stripped linked image, workload, cell order
and sample count as the first Level G verdict, so the earlier wrapper/direct
gap can be compared without changing the benchmark contract.

The rework is compiler-generic rather than class-specific:

- a single direct final packed load donates its scalar register to the inlined
  `BLOCK_EXPR` result and falls through to the immediately following end label;
- an exact scalar accessor omits its method-entry `assertinitialized` only for
  a direct factory receiver or a non-aliased local with one dominating factory
  binding; and
- unknown, argument, exposed, reference, replacement-written, labelled and
  dynamic-`SIGNAL` receiver shapes retain the runtime guard.

This remains a narrow mechanism comparison, not a whole-application claim.
Allocation, initialization and final checksum validation are outside the timed
item loops.

## Provenance

- Source revision: `8f0dc5195b38bec3a2fe97b100e2373504b71a07` on
  `develop`, with the complete BINARY-01 dirty-worktree scope recorded in
  `provenance/pre-run.txt`.
- Host: Apple M5, Darwin arm64, macOS 26.5.2, Apple clang 21.0.0, CMake 4.3.2,
  Ninja 1.13.2.
- Power/state: AC power, low-power mode off, and no thermal or performance
  warning before or after the run. Exact state is retained in `provenance/`.
- Build: ordinary `Release`; `CREXX_VM_PROFILING=OFF`.
- Program: `tests/performance/packed_numeric_class_compare.crexx`, compiled in
  normal optimized mode and linked with `rxlink -s`.
- VMs: the ordinary Release `rxbvm` and `rxtvm` concrete products, reported
  independently.

Build command:

```text
cmake --build cmake-build-release \
  --target performance_packed_numeric_class rxlink rxbvm rxtvm --parallel 10
```

Cell command template:

```text
cmake-build-release/bin/<rxbvm|rxtvm> \
  performance/evidence/2026-08-21-binary01-packed-class-rework-release-verdict/provenance/packed_numeric_class_compare_linked.rxbin \
  -a 200000 10 <variant>
```

## Sampling and correctness

Each cell performs 2,000,000 timed item operations. The run used one serial
warmup per cell followed by 12 serial recorded rounds. Even rounds reverse all
20 cells, so every cell occupies both its forward and reverse position six
times.

All 20 warmups and 240 recorded executions passed. Every variant retained its
expected checksum, and no sample was removed. Focused Debug tests independently
cover optimized source import, RXAS/RXDAS/reassembly and binary-only import,
both concrete VMs, the live binary reference contract, and the retained
`OBJECT_NOT_INITIALIZED` signal for an uninitialized receiver.

## Result

`summary.csv` reports medians and the median of 12 paired
comparator-time/candidate-time ratios. A ratio above 1 means the candidate is
faster; below 1 means it is slower.

### Level G wrapper versus direct Level B packed access

| VM | Kernel | Wrapper median | Direct median | Wrapper throughput | Paired ratio | Wrapper wins |
|---|---:|---:|---:|---:|---:|---:|
| `rxbvm` | float read | 11,688.5 us | 11,585.5 us | 171.1 M items/s | 1.007x | 8/12 |
| `rxtvm` | float read | 10,408.5 us | 10,568.5 us | 192.2 M items/s | 1.015x | 8/12 |
| `rxbvm` | integer read | 11,699.5 us | 12,075.0 us | 171.0 M items/s | 1.033x | 8/12 |
| `rxtvm` | integer read | 10,360.5 us | 10,469.0 us | 193.0 M items/s | 1.004x | 8/12 |
| `rxbvm` | float write | 19,472.5 us | 37,233.5 us | 102.7 M items/s | 1.900x | 12/12 |
| `rxtvm` | float write | 23,263.5 us | 38,372.5 us | 86.0 M items/s | 1.652x | 12/12 |
| `rxbvm` | integer write | 11,828.5 us | 19,460.0 us | 169.1 M items/s | 1.668x | 12/12 |
| `rxtvm` | integer write | 10,002.5 us | 21,772.0 us | 200.0 M items/s | 2.169x | 12/12 |

The read wrappers are now at practical parity with direct Level B access:
paired throughput is 0.4%-3.3% higher, with each wrapper winning eight of 12
pairs. Relative to the first verdict, wrapper median elapsed time fell by
14.6%-59.4% depending on type and VM. Writes retain their prior decisive
advantage in all 48 pairs.

### Direct packed integer versus encoded i64

| VM | Kernel | Packed median | Encoded median | Packed throughput | Paired speedup | Packed wins |
|---|---:|---:|---:|---:|---:|---:|
| `rxbvm` | integer read | 12,075.0 us | 21,369.5 us | 165.6 M items/s | 1.806x | 12/12 |
| `rxtvm` | integer read | 10,469.0 us | 23,594.5 us | 191.0 M items/s | 2.233x | 12/12 |
| `rxbvm` | integer write | 19,460.0 us | 36,417.5 us | 102.8 M items/s | 1.871x | 12/12 |
| `rxtvm` | integer write | 21,772.0 us | 37,899.0 us | 91.9 M items/s | 1.779x | 12/12 |

The native packed primitive therefore retains its machine-format advantage
after the wrapper rework.

## Verdict and decision boundary

The verdict is **accepted: retain the exact-getter rework and treat the initial
`.packedfloat` / `.packedint` Level G accessors as performance-qualified**.
The optimized hot read path is one `pgetf`/`pgeti` feeding its consumer. The
negative control proves that this does not globally discard method receiver
initialization semantics. Adrian accepted this baseline on 2026-08-22.

Broad Debug, sanitizer, install, startup/RSS/artifact and cross-platform
closeout remain consolidated at the end of RCC-5 and are not claimed here.
