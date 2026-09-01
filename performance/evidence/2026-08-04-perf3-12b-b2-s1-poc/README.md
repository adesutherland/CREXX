# PERF3-12B B2 isolated S1 segmented-stem PoC

Date: 2026-08-04

Status: complete; retain for the B4 route comparison, no production selection

## Outcome

The isolated RXAS PoC proves and rewrites four of the five generated
`"Key Bee." || lvar` native-stem sites. It replaces three `STEMGET` operations
and one `STEMSET` with the existing two-segment forms, deletes the four paired
joined-key concatenations and their exact `C` TRACE events, and retains the
fifth site.

The retained site is a useful safety result, not a proof gap. A later
failure-atomic `STEMGET` writes the same joined-result register on success but
preserves its earlier value on the signal-skip continuation. The later
user-visible TRACE can therefore observe the old compound tail after a failed
get. Removing that concat would change observable state, and the immutable use
proof rejects it as `trace-observed`.

The four selected sites remove 1,960,000 hot concatenation dispatches. Current
RXC output does not retain the unpunctuated left segment, so the PoC adds one
loop-local `LOAD "Key Bee"`, executing 280,000 times and increasing `.locals`
from 103 to 104. The isolated net is therefore 1,680,000 dispatches. Applied
to the latest accepted X1 fixed-work total of 52,839,051, with B1 already
proved hot-work neutral, that gives a derived S1 total of 51,159,051
(-3.179467%) under either VM. This is an exact instruction-count derivation,
not a new profiled or wall-clock result.

The selected concats would write 18,340,000 bytes of joined-tail payload; the
new stable-left loads write 1,960,000 bytes, for 16,380,000 net temporary
payload bytes avoided, excluding terminators and value-structure writes. These
small strings remain inline, so the handler and representation audit predicts
neutral heap-allocation count. That allocation result is an inference to be
confirmed by the B4 counts product, not a measured claim.

## Scope and provenance

- Accepted B1 base: `83de3db3c2d42f3dfcd61a1242e37d3ef196437b` on
  `codex/perf3-12b-compound-tail`.
- Isolated PoC: `888fa94eb0d5b14aa4d3a9a028117a424c6358be` on
  `codex/perf3-12b-s1-poc`.
- Canonical source remains unchanged. The generated accepted RXAS is modified
  only by the replay patch to provision the candidate's stable-left register.
- Product: ordinary profiling-off Release RXAS. Diagnostic `-d` assembly is
  used only for proof and scale evidence.
- Runtime authority: the unchanged accepted profiling-off Release `rxvm`,
  `rxbvm` and common `library.rxbin` recorded in `artifacts.csv`.
- Host: Apple M5, Darwin 25.5.0 arm64, macOS 26.5.2, 10 logical CPUs.
- Toolchain: CMake 4.3.2, Ninja 1.13.2, Apple clang 21.0.0.

## Common foundation delta

S1 needs exact string-component metadata for the CONCAT family. Adding that
metadata also lets the already accepted X01 proof remove one unrelated
`DCOPY`/TRACE pair in the unmodified accepted RXAS. This common F1 effect is
not counted as S1: the same-shaped control and candidate use the same PoC RXAS
binary, and differ by exactly the four S1 selections. B3/B4 must either carry
F1 in every route or factor it out explicitly.

## Correctness and scale

- The final focused native-stem selector passes 16/16. The immutable graph and
  opcode-metadata pair also passes 2/2 after the final diagnostic cleanup.
- Same-shaped control, optimized candidate and optimizer-disabled candidate
  each print PASS with zero stderr under both `rxvm` and `rxbvm`: six cells.
- The optimized candidate retains `.locals=104`, removes four static
  instructions and four TRACE records, and reduces the code segment from
  `0xf5b` to `0xf4f` (-12 bytes).
- Matched one-sample ordinary assembly is 0.32 s/132,890,624 bytes maximum RSS
  for the control and 0.31 s/132,710,400 bytes for S1. Diagnostic assembly is
  0.57 s for both, at 132,726,784/132,612,096 bytes. First-epoch SSA retained
  bytes are identical at 83,902,520. This is mechanism/scale evidence, not a
  benchmark timing verdict.

## Evidence map

- `analysis/selection.csv`: per-site proof outcome and exact dynamic weights.
- `analysis/foundation-and-work.md`: dispatch, payload, allocation and common
  metadata accounting.
- `replay/`: exact generated-RXAS input patches; no canonical source clone.
- `validation/focused-correctness.txt`: final focused and dual-VM results.
- `validation/release-structure.txt`: emitted instruction/image structure.
- `validation/assembler-scale.txt`: matched raw one-sample scale observations.
- `artifacts.csv`: source, tool, input, image and runtime identities.
- `checksums.sha256`: recursive evidence closure excluding itself.

## Interpretation boundary and next gate

B2 demonstrates that segmented compound-tail selection is both useful and
strictly proof-gated on this slice. It does not install S1 in the production
branch, establish a wall-clock improvement, select S1 over H1, or authorize a
combined route. The next item is `B3 — isolated H1 loop-scoped joined-key reuse
PoC`, separately replayed from the accepted B1 base after Adrian's approval.
