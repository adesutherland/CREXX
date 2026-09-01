# NR-09 large-instruction drift-controlled rebaseline

Status: **complete; the original cross-session negative verdict is superseded
by a neutral-to-slightly-positive same-session verdict. Awaiting Adrian's
decision before broad QA or a production-batch commit.**

## Why this rerun was required

The original `-2.233%`/`-2.289%` result compared three accepted-product
samples from 2026-07-17 with three batch samples from 2026-07-18. The accepted
baseline was intentionally reused rather than rerun. The focused `SETTPCALL`
review subsequently demonstrated enough runtime-image and measurement
sensitivity that the sign of a small comparison could not safely be inferred
from unmatched sessions.

This campaign therefore reruns both products in one session and adds the
missing old-RXBIN control. It does not discard or overwrite the original raw
result; it explains and supersedes its performance interpretation.

## Exact products and cells

Both products are ordinary `CMAKE_BUILD_TYPE=Release`, `-O3 -DNDEBUG`,
`CREXX_VM_PROFILING=OFF` builds on the same Apple M5 host.

- **A -- accepted product:** clean detached worktree at accepted commit
  `847e62f04`, accepted VM, accepted benchmark image and accepted library.
- **B -- infrastructure control:** current batch VM, accepted benchmark image
  and accepted library. This isolates the wider opcode infrastructure, added
  handlers and VM binary shape while executing old bytecode.
- **C -- complete batch:** current batch VM, batch benchmark image and batch
  library. This is the current fused product.

The canonical RexxCPS source SHA-256 is
`2970c3d73fe2537ec8f81295c585495c4668b442d5b9a2335b1ee453a13bbdd6`.
The reconstructed accepted compiler emits the exact accepted RXAS hash
`868fb7fdac2b387dca9ba227d0ca949431e51ea87ebb06928e874360592ee515`.
The rebuilt candidate emits RXAS hash
`373c33479086b4c9a8373f55a133052f5f9bbbc925cee867176400a254ccaa0b`,
byte-identical to the frozen first-verdict candidate. The candidate library
also retains its first-verdict hash
`d1089745eb42a076e879f6c0a2e68e64865f5a801e9636e12fa27b68ce486846`.

The exact timed VM and library binaries are retained under `artifacts/`.
`images/cell-a.rxbin` and `images/cell-c.rxbin` are the exact timed images.
The two RXAS input paths have equal length; RXBIN description metadata cannot
create a path-length imbalance between A and C.

## Lifecycle and schedule

All processes are serial. Every VM/product cell receives one full canonical
warmup. The recorded campaign has 12 rounds per VM; every round executes A, B
and C once. All six A/B/C permutations occur twice in a reverse-symmetric
schedule, every product occupies every position four times, and `rxvm` versus
`rxbvm` starting order alternates by round.

There are 72 recorded samples and six warmups. All 78 executions pass the
canonical RexxCPS 2.2d contract with effective count 100. Absolute performance
varied materially during the sustained run, including one large `rxbvm` Cell
B low outlier. It is retained. The balanced paired comparisons, not the
independent absolute medians, are the decisive statistics.

## Same-session result

Positive CPS is faster; negative elapsed time is faster. The interval shown is
the approximate two-sided 95% Student-t interval around the mean of the 12
paired percentage deltas. It is reported as an uncertainty indicator, not as
a programme regression threshold.

| Comparison | `rxvm` paired CPS median (mean 95% interval) | `rxbvm` paired CPS median (mean 95% interval) |
| --- | ---: | ---: |
| B versus A: VM/infrastructure | -0.152% (-1.217% to +0.281%) | +0.022% (-3.095% to +1.670%) |
| C versus B: fused product | +0.617% (-0.211% to +2.352%) | +0.805% (-0.736% to +3.303%) |
| C versus A: complete batch | **+0.586%** (-0.140% to +1.298%) | **+0.671%** (-0.934% to +1.874%) |

The elapsed-time paired medians agree with the complete-product CPS direction:
`-0.578%` on `rxvm` and `-0.704%` on `rxbvm`. C beats A in 9/12 paired CPS and
elapsed comparisons on `rxvm`, and 8/12 on `rxbvm`. For the same-VM fusion
comparison, C beats B in 10/12 comparisons on `rxvm` and 8/12 on `rxbvm`.

The intervals include zero. The batch is therefore not a proved material
speedup, but neither the infrastructure nor fused product reproduces a
material slowdown. The corrected verdict is **neutral-to-slightly-positive**.

## Explanation of the apparent 2.2% slowdown

The old candidate result reproduces almost exactly in Cell C, while the old
accepted baseline does not reproduce in Cell A:

| VM | Retained accepted -> Cell A | Retained candidate -> Cell C | Old cross-session result | New paired result |
| --- | ---: | ---: | ---: | ---: |
| `rxvm` | **-2.577%** | +0.165% | -2.233% | **+0.586%** |
| `rxbvm` | **-2.364%** | -0.120% | -2.289% | **+0.671%** |

Thus the former negative sign came from comparing a faster accepted-baseline
session with a later candidate session. The candidate itself is stable within
0.17% across the two captures. The apparent regression is measurement/session
drift, not an identified VM, decoder or fused-handler regression.

Cell B also supplies the previously missing same-session old-RXBIN performance
test. Its paired medians versus A are -0.152% and +0.022%, with both intervals
crossing zero. This confirms the functional old-RXBIN result: the new VM and
arbitrary-operand infrastructure do not materially slow the accepted image.

## Profile report boundary

The standard 60-form report was regenerated after the product campaign. It
still reconciles the dynamic count reduction within +16/-14 instructions and
remains useful for coverage, coherence and review prioritisation. Its global
component averages, timer floor and overlapping populations cannot replace
this unprofiled product verdict or exact per-form cells. No per-form timing
estimate is summed into the corrected batch result.

## Reproduction and retained evidence

The isolated accepted build used:

```sh
git worktree add --detach /tmp/crexx-nr09-rebaseline-847e62f04 847e62f04
cmake -S /tmp/crexx-nr09-rebaseline-847e62f04 \
  -B /tmp/crexx-nr09-rebaseline-847e62f04/cmake-build-release \
  -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_FLAGS_RELEASE='-O3 -DNDEBUG' \
  -DCREXX_VM_PROFILING=OFF -DBUILD_TESTING=ON
cmake --build /tmp/crexx-nr09-rebaseline-847e62f04/cmake-build-release \
  --target rxc rxas rxlink rxvm rxbvm library compiler_exit_bin --parallel 10
```

Run the already-built three-cell campaign from the repository root with:

```sh
performance/evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/rebaseline/run_three_cell_rebaseline.zsh
```

The runner refuses to overwrite an existing `samples.csv`. Retained evidence
includes raw stdout/stderr for every execution, `samples.csv`,
`cell-summary.csv`, `paired-deltas.csv`, `paired-summary.csv`,
`position-summary.csv`, `historical-drift.csv`, exact provenance, build logs,
timed images and binaries, the analysis script and checksums.
