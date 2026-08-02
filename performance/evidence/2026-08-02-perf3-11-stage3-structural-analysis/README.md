# PERF3-11 Stage 3 structural analysis

Status: **complete — Gate 3 passes**

This bundle closes the consumer-free structural-analysis stage. It retains the
first RSS-rejected eager integration as well as the accepted demand-driven
form. It is an assembler correctness/scaling gate, not a new VM runtime
benchmark baseline.

## Provenance

- Branch: `codex/perf3-rxas-flow-infrastructure`
- Stage 2 base: `c15f29419` (`perf: add immutable RXAS flow graph`)
- Stage 3 source: the base above plus the Stage 3 code, tests, documentation
  and this evidence bundle; the resulting local commit is authoritative in Git
  history because a commit cannot contain its own hash.
- Frozen comparator: Gate 0 profiling-off Release `rxas` at
  `/tmp/crexx-perf3-11-stage0.35IBzf/base-binaries/rxas`
- Host: Darwin 25.5.0 ARM64, Apple M5, 10 logical CPUs, 24 GiB RAM
- Power: AC; low-power mode off
- Thermal state: no recorded thermal, performance or CPU-power warning
- Build: CMake/Ninja Release, `CREXX_VM_PROFILING=OFF`

## Structural result

The immutable procedure epoch now owns an optional cached structural result.
It builds sparse successor-edge and unique-predecessor CSR views, multi-root
reachable RPO, immediate dominators and tree intervals, sparse dominance
frontiers, SCCs, dominance-classified backedges and natural/irreducible loop
regions. Signal-retry-only cycles are explicitly marked so they cannot be
confused with source-language loop candidates.

Every query requires the epoch. A deliberately exhausted work budget, invalid
graph or allocation failure returns no usable analysis. A failed small-budget
request may be retried with a larger budget. Graph destruction invalidates and
frees the cache. Post-dominance remains deliberately deferred until a consumer
requires a must-execute query.

The successful `-d` reductions contain no unavailable procedure. Retained
bytes are the sum of per-procedure cached structural arrays, not simultaneous
process RSS.

| Workload | Procedures | Work / budget | Retained bytes | Reachable / unreachable blocks | Preds | Frontiers | SCCs / irreducible | Backedges | Loops / memberships | Max depth |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Richards | 24 | 322,781 / 1,695,232 | 304,248 | 1,190 / 52 | 4,227 | 4,131 | 1,086 / 0 | 753 | 752 / 856 | 2 |
| Towers | 13 | 34,169 / 541,952 | 100,419 | 426 / 33 | 1,239 | 1,311 | 372 / 0 | 203 | 203 / 265 | 3 |
| RexxCPS | 5 | 250,212 / 1,126,400 | 218,825 | 900 / 10 | 2,941 | 3,461 | 406 / 0 | 493 | 488 / 1,345 | 6 |

## Correctness and image result

- Final focused Debug matrix: **113/113 passed**. It includes the structural
  graph contract, every RXAS optimizer test, signal lifecycle cases, both VMs'
  optimized/no-opt signal/storage/conversion fixtures and both decimal
  plugins.
- Strict C90 syntax checking of the new graph/analysis sources passes with
  `-Wall -Wextra -Wconversion -Wsign-conversion`.
- Ordinary profiling-off Release `rxas` builds.
- Canonical Richards, Towers and RexxCPS RXBIN hashes remain exactly equal to
  Gate 0 under the canonical working-directory/basename contract.
- Stage 3 has no rewrite consumer and changes no emitted instruction.

## Assembler-cost result

The first implementation eagerly requested the structural result for every
procedure even though no optimizer consumed it. Its elapsed medians were
guard-clean, but Richards median peak RSS rose by 1,155,072 bytes (+13.454%),
crossing the rule that requires both greater than 5% and greater than 1 MiB.
That integration was rejected rather than waived.

The accepted manager is demand-driven. Tests, `rxas -d` and future consumers
request and cache the analysis; ordinary consumer-free assembly does not solve
or retain unused facts. The final same-session comparison used two warmups and
30 balanced/interleaved elapsed rounds per workload. Peak RSS used ten
separately interleaved samples per binary/workload. Pre/post load was
`{ 1.52 2.76 2.80 }` / `{ 1.52 2.72 2.79 }`.

| Workload | Frozen median | Stage 3 median | Elapsed delta | Frozen median RSS | Stage 3 median RSS | RSS delta |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Richards | 0.059923053 s | 0.055676103 s | -7.087% | 8,568,832 B | 9,068,544 B | +499,712 B (+5.832%) |
| Towers | 0.019303918 s | 0.019031525 s | -1.411% | 4,636,672 B | 4,980,736 B | +344,064 B (+7.420%) |
| RexxCPS | 0.055335998 s | 0.054857612 s | -0.865% | 9,764,864 B | 10,018,816 B | +253,952 B (+2.601%) |

No elapsed regression is present. The Richards and Towers RSS percentages are
above 5%, but their byte deltas are far below 1 MiB, so the combined RSS
escalation condition is not met.

## Gate 3 verdict

Gate 3 passes. The structural foundation is sparse, deterministic, bounded,
epoch-safe, cached on demand and output-neutral. Stage 4 may add mutable
signal-policy/effect versions; no component-value proof may consume a signal
edge until that stage supplies a valid edge-specific state or explicit
unknown.
