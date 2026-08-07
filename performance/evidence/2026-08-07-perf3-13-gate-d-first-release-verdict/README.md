# PERF3-13 Gate D first ordinary-Release verdict

Date: 2026-08-07  
Status: **accepted — clean 176-byte L32SDH industrialisation passes**

## Decision

Adrian accepted this mandatory first Release verdict on 2026-08-07. Continue
Gate D closeout with the clean L32SDH implementation:

- fixed 64 KiB S0 worker slabs and a 16 KiB maximum standard byte class;
- 176-byte normal UTF-8 `value` with no research layout selector;
- direct decimal payload pointer and one co-allocated 16-byte raw-`size_t`
  header immediately before the payload;
- raw `size_t` allocation capacities and binary/decimal native lengths;
- 32-bit direct string logical metrics with checked ingress only; and
- R0 sticky reuse, with no automatic or pressure-triggered reclamation on
  logical reset.

This verdict authorizes documentation/evidence and proportional validation
closeout. It does not open worker execution, `rxtvm` selection, process/host
transport, channel/RXAS semantics or a new reclamation policy.

## Source and host

- Worktree: `/private/tmp/crexx-rxvm-gated.6HZvFK/source`.
- Branch: `codex/rxvm-gate-d-industrialise`.
- Base: `0d1fe884782ff369960b1c67c38127407ce54588` (`performance:
  complete PERF3-13 Gate C census`) plus the reviewed Gate D production diff.
- Research control: frozen Gate C L32S binary, SHA-256
  `2d3dc6c8df5e260cf6100413ee11d0bf4d497ba6ca775e12ebb9494abf138174`.
- Candidate: clean Gate D L32SDH binary, SHA-256
  `36e3458a7408b87efacc2d7796e8ab4ed9bc0ee12d1faef108b7cb7e17867126`.
- Host: Apple M5, Darwin arm64 25.5.0, macOS 26.5.2 (25F84), ten logical
  CPUs.
- Toolchain: Apple Clang 21.0.0, CMake 4.3.2, Ninja 1.13.2, Release
  `-O3 -DNDEBUG`, profiling off.
- Power: AC attached at 80%, low-power mode off before the build/run and AC
  still attached at 80% afterwards. The host was reserved exclusively.
- Apple Clang selects `rxvm -> rxbvm`; the concrete selected product was
  measured once as `rxbvm`, not duplicated as an independent `rxvm` cell.

## Correctness prerequisite

The focused Debug set passed 11/11. The ordinary Release product and all
required decimal harnesses then built; the final focused Release set passed
12/12, including value layout/lifecycle, allocator family/ownership, stems,
static/dynamic/manual decimal engines, full decimal fixtures and the archive
link consumer. An earlier narrow invocation reported four tests as `Not Run`
because their executable targets had not been requested; after those exact
targets were built, all four ran and passed. This was not a runtime failure.

## Decisive performance result

The corrected Level B runner executed one warmup and 12 recorded serial,
pairwise-balanced rounds for frozen L32S and clean L32SDH on Sieve, Richards,
Towers and canonical RexxCPS. All 104 processes passed their correctness
markers. Positive percentages favor the industrialised candidate.

| Workload | Pairs | Paired mean | 95% interval | Paired median | Favorable |
|---|---:|---:|---:|---:|---:|
| Sieve | 12 | +0.533% | -0.375% to +1.440% | +0.773% | 9/12 |
| Richards | 12 | -0.825% | -2.621% to +0.970% | -0.504% | 5/12 |
| Towers | 12 | +3.045% | +2.602% to +3.489% | +3.396% | 12/12 |
| RexxCPS | 12 | +1.120% | +0.521% to +1.720% | +0.977% | 10/12 |
| Pooled core four | 48 | **+0.968%** | **+0.342% to +1.594%** | **+1.301%** | **36/48** |

No workload hits the 3% regression guard. The pooled result is independently
positive and consistent with Gate C's two-block L32SDH result of +0.847%
(+0.398%, +1.296%). No sample was removed and no append was needed.

## Representation and artifact proof

The compile-time layout test fixes the normal 64-bit UTF-8 value at 176 bytes
and the co-allocated decimal header at 16 bytes. The clean product and selected
research candidate have identical file, `__text` and flattened `run()` sizes.
Against L32S:

| Metric | L32S | Clean L32SDH | Change |
|---|---:|---:|---:|
| `value` | 192 | 176 | -16 (-8.333%) |
| `rxbvm` file | 1,001,000 | 1,001,048 | +48 |
| `__text` | 813,332 | 815,436 | +2,104 |
| flattened `run()` | 531,180 | 533,456 | +2,276 |

The whole-file hash differs from the scratch candidate because this is a clean
rebuild from production-only source, but the selected artifact-size contract
is unchanged. The retained Gate C decision report remains the authority for
the rejected 152-184-byte factorial/backoff alternatives.

## Interpretation boundary

This is a same-host causal ordinary-Release comparison of clean L32SDH against
the exact frozen L32S binary and exact previously compiled benchmark images.
It supports the single-worker selected-product performance verdict only. Gate C
retains the memory/RSS selection evidence; this bounded first verdict did not
repeat valid RSS, automatic-reclamation, `rxtvm` or multi-worker experiments.
