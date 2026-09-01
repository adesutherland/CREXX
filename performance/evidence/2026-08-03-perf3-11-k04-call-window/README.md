# PERF3-11 K04 call-window proof and attribution

Status: **K04 complete and accepted; K04d4 closeout green**

K04b replaces the integer compare/branch consumer's procedure-global numeric
call-window veto with a reusable SSA query. A constant argument count gives an
exact local-register window; an unavailable count conservatively reaches the
highest local. The proof follows the exact compare-result `ValueId` through
copy/derived values and lazily materialized phis and rejects only when that
identity is visible in the window or a query fails closed.

Focused positive, mixed-phi, LINK, SWAP and no-opt cases pass 5/5 in Debug and
ordinary Release. The Release `rxas` SHA-256 is
`8a42d71e630672b37312a0728490160ea252ae13645c00ec924d2a8f8769ce11`.
Canonical RexxCPS remains byte-identical to K04a at 1,250 VM instructions and
1,261 TRACE events, still +9/+9 versus frozen M06. Adrian accepted this as a
neutral migration/consolidation result on 2026-08-03; no runtime timing was
performed for an identical image.

## K04c attribution

Temporary diagnostics captured the first dependency witness for each of the
five residual `call-window-observed` decisions in `__rxtrace_handler`:

| Compare record | Result | Call instruction | Conservative window | First witness |
| ---: | --- | ---: | --- | --- |
| 10 | `r6` | 774 | `r1..r69` | unrelated unknown `r43`; query unavailable |
| 502 | `r2` | 774 | `r1..r69` | unrelated unknown `r43`; query unavailable |
| 509 | `r2` | 219 | `r1..r69` | old result `ValueId` visible through `r2` |
| 523 | `r2` | 219 | `r1..r69` | old result `ValueId` visible through `r2` |
| 533 | `r2` | 219 | `r1..r69` | old result `ValueId` visible through `r2` |

The two relevant calls are at RXBIN addresses `0x7d6` and `0xece`. Each is
immediately preceded by `load r0,1` and uses `r0` as its count register, so the
actual caller-owned argument window is exactly `r1`. The compare results in
`r2` or `r6` are not actual arguments. All five rejections are therefore
conservative false positives.

The cause is the counted CALL family's current `RXSC_UNKNOWN` contract. Its
retry edge applies a dynamic failure clobber, turns the count into an unknown
phi and forces the otherwise exact query to open the range to `r1..r69`.

VM source confirms that an action-aware signal handler runs in a child frame
whose explicit argument is the signal object. Returning the `retry` action
sets execution back to the recorded interrupted call; the caller count operand
is then reread but was not exposed to the handler. Caller-owned arguments may
change by reference, and LINK/SWAP aliases can make such changes visible under
other register names. The existing storage-identity call-range clobber already
models those effects, including the case where an argument aliases the count
storage. The count register must not additionally be treated as an arbitrary
CALL write.

K04c changed no production semantics. Its diagnostics were removed after the
witness capture, the Release hash returned exactly to the K04b value above and
the canonical K04b/K04a images compare equal.

## Validation and next gate

- strict GNU90 syntax passes with one pre-existing unused-parameter warning;
- focused Debug and Release checks pass 5/5;
- the complete Debug build passes;
- broad Debug passes 1,995/1,995 in 376.48 seconds; and
- `git diff --check` passes.

K04d0 subsequently reviewed instruction-level retry itself. Repository search
found no production caller; standard REXX condition traps do not re-execute the
faulting instruction; and the first end-to-end native retry fixture exposed an
existing fused-call mapping defect. The continuation also creates one CFG
self-edge and failure-state phi for every potentially signalling instruction
and cannot safely repeat arbitrary partial writes or external side effects.

Adrian approved K04d1 retirement on 2026-08-03. The public factory, VM
continuation, CFG edge and retry-only loop machinery are removed; the legacy
internal marker fails safely. Canonical propagated-call partial-state metadata
remains for skip, handler and unwind paths, retaining caller-owned argument
clobbers, alias effects and exact destination write phase.

K04d2 passes the same 14 focused checks in Debug and ordinary profiling-off
Release. The matrix covers the retired public API diagnostic, legacy-marker
fail-safe handling in both VMs, signal functional tests, opcode metadata, flow
and compare/branch contracts, and dynamic/static native unwind mappings. Strict
GNU90 syntax also passes with the pre-existing unused-parameter warning in
`rxas_flow.c`.

The frozen K04d3 candidate has 1,222 VM instructions and 1,249 TRACE events,
versus 1,241/1,252 in the retained M06 runtime image. This combines the five
newly unblocked compare/branch fusions with removal of the unused retry surface.
The first measurement start was deliberately deferred because `avconferenced`
was persistently consuming roughly one CPU core; no contaminated timing sample
was recorded.

## K04d3 first ordinary Release verdict

After the remote/conferencing load was stopped, the host was on AC with low
power mode off and no thermal or performance warning. The checked-in Level B
runner executed one warmup and 12 balanced/interleaved recorded rounds per cell
using the same current Release `rxvm`, `rxbvm` and `library.rxbin` for both
images. All 52 executions passed the canonical RexxCPS correctness and
provenance markers. No sample was removed.

| VM | M06 median CPS | K04d median CPS | Delta | Favourable pairs | Noise verdict |
| --- | ---: | ---: | ---: | ---: | --- |
| `rxvm` | 46,221,991.5 | 46,231,723.5 | **+0.021%** | 6/12 | rerun recommended |
| `rxbvm` | 45,149,051.0 | 45,158,611.5 | **+0.021%** | 5/12 | no rerun |

One `rxvm` candidate sample in round 12 recorded 40,194,186 CPS, 12.99% below
its paired M06 sample. It is retained without exclusion and expands the
candidate cell span to 14.13%; the runner therefore marks that cell for a
rerun. The median result is nevertheless independently neutral on the clean
`rxbvm` cell, and pair directions are mixed on both VMs. The evidence does not
demonstrate a material runtime improvement from K04d. Its proven outcomes are
the simpler signal contract, removal of synthetic retry CFG/SSA state and the
smaller canonical image. Adrian accepted K04d1 as a neutral
semantic/infrastructure improvement without a noisy-cell append on 2026-08-03.

## K04d4 closeout

The complete Debug build passes. Broad Debug then passes 1,998/1,998 tests in
297.92 seconds at `--parallel 30`. The final retirement audit finds no retry
enum, VM continuation, CFG edge, retry-only loop classification or public
factory in production code. The only current-source occurrences are the
deliberate negative compiler fixture, the legacy-marker fail-safe VM fixture
and documentation of the retirement; older C2E2 text remains historical
provenance. `git diff --check` passes. K04 is closed.
