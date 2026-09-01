# PERF3-05-R3 handler code-generation analysis

This bundle closes the approved Apple Clang/GCC investigation of the R2
profile-30 slowdown. The internal handler-placement framework is retained, and
the 30% experimental panel is repaired, but no product-default or public ISA/
ABI decision is made.

## Result

R2's Clang regression was an avoidable source/code-generation defect, not
instruction search and not helper sub-inlining. A reachable pointer-rich
outlined-handler facade changed Clang's hot owner allocation even when no
public outlined call ran. Clang now snapshots values only at one shared cold
entry. Real GCC performs better with the original per-identity pointer-facade
lowering, so non-inline panels lower by compiler.

The profiler also missed hot process-private fused dispatch because it mapped
those handlers to their first canonical public opcode. Native sampling exposed
hot `PRIVATE_R1_RELINK` execution in Bounce. Both existing private fusions are
therefore inline, giving 178/590 non-reserved public-plus-private definitions
inline (30.17%).

Final geometric-mean normalized throughput versus the rebuilt all-inline
control is:

| compiler | engine | all seven | without Base64 |
|---|---:|---:|---:|
| Clang | `rxtvm` | -0.541% | -0.341% |
| Clang | `rxbvm` | +0.274% | +0.201% |
| GCC | `rxtvm` | +3.841% | +1.379% |
| GCC | `rxbvm` | +8.053% | +6.422% |

The all-inline design equivalence is confirmed. Without Base64, rebuilt R3
all-inline versus R2 is -0.220%/-0.280% for Clang `rxtvm`/`rxbvm` and
-0.127%/+0.520% for GCC. Owner lengths are exactly the R2 lengths. The first
formal verdict that did not preserve the dead all-inline facade source is
retained as a rejected control because it changed threaded performance by
roughly 5-6%.

Clang profile-30 reduces `run()` from 532,512/531,868 bytes to
205,548/205,444 bytes for `rxtvm`/`rxbvm`. GCC retains the faster legacy-shaped
owner at 547,808/549,632 bytes rather than forcing the Clang shape. The current
host has a 128 KiB L1 instruction cache; size remains evidence, not an
acceptance rule.

All 1,176 final matrix executions pass, retaining 1,008 recorded samples. Both
final profile-30 compiler trees pass the focused 14-test suite and the full
2,002-test profiling-off Release suite. Fresh complete all-outline trees also
pass the focused 14-test suite under both compilers.

## Private fusion disposition

The existing `PRIVATE_R1_RELINK` and `PRIVATE_R2_COPYATTR1` handlers are
immutable load-time quickening in a process-owned execution image. Canonical
RXBIN is unchanged, and runtime guards fall back to canonical behavior. A
shared RXAS/RXVM fusion registry, gap report and any decision to make these
normal serialized instructions is queued separately as `PERF3-05-R4`.

## Bundle map

- `final-*`: manifest, capture, raw samples/output, summaries, comparisons,
  geometric means and frozen pre/post hashes for the accepted matrix.
- `rejected-first-*`: the first formal verdict that exposed the all-inline and
  universal-GCC code-shape defects.
- `compiler-split-pilot-*`, `hot-private-pilot-*`,
  `diagnostic-relative-to-inline.csv`: bounded causal controls.
- `clang-expansion-checks.csv`, compiler selected-inline decision ledgers,
  `compiler-findings.md`, `static-shape.csv`: compact compiler evidence.
- `sample-bounce-private-*.txt`: native sample evidence for the hidden private
  fused handler.
- focused/full CTest logs and `test-summary.txt`: correctness closure.
- `host-state.txt`, `COMMANDS.md`: environment and replay details.
- `SHA256SUMS`: checksum closure for every other retained file.
