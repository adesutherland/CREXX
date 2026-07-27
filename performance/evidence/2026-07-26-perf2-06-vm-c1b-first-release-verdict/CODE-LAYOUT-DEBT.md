# PERF2-06-D01 - accepted `rxbvm` code-layout debt

Status: accepted known debt; mandatory revisit at the cross-platform
performance gate

Accepted VM-C1b implementation commit:
`a39608426e2c1bb84d5fc0c4f767f4c9492339a9`

## Observation

- The governed Sieve 50 cell is clearly adverse in `rxbvm`: paired median
  `+5.727739%`, mean `+5.368694%`, 95% interval
  `[+4.720473%, +6.016915%]`, and 0/12 favorable pairs.
- Optimized Sieve executes no bytecode calls and performs no handler-table
  mutation. It cannot execute child-frame inheritance or the copy-on-write
  helper introduced by VM-C1b.
- A separately named work-500 diagnostic scales the effect: baseline median
  129.392 ms, candidate median 141.053 ms, paired median `+9.181412%`, mean
  `+10.730774%`, 95% interval `[+7.810599%, +13.650949%]`, 0/12 favorable.
  This excludes one-time root interrupt-table allocation/startup as the cause.
- The helper bodies are instruction-identical but their addresses move. In the
  approximately 536 KiB flattened `rxbvm` `run()` body, Apple Clang changes
  prologue/register allocation and broad stack-access/basic-block placement.
  Static disassembly changes include stack-relative `ldr` sites 5,992 to
  3,808, stack-relative `ldp` sites 105 to 933, stack stores 672 to 813, and
  branches 30,522 to 31,248. These are static sites, not dynamic counts.
- A 48-byte diagnostic pad restored the suspected `clear_value_contents`
  alignment but did not recover Sieve. Candidate versus padded-candidate
  work-500 paired median was `+0.4586%` with an interval crossing zero. The
  single-helper alignment hypothesis is rejected.
- Native work-6000 samples retain more candidate attribution below the same
  clear/reset/trim helpers (1,316 versus 1,168 samples), but Apple hardware
  counters needed to separate instruction-cache, iTLB and branch-predictor
  effects are unavailable.

## Current hypothesis and limit

The high-confidence cause class is global code layout, register allocation and
basic-block placement perturbation produced by Apple Clang across the
monolithic `RX_FLATTEN` switch interpreter. The exact microarchitectural split
is unresolved. This is a compiler/layout debt, not evidence that shared/COW
interrupt policy executes on Sieve or that the frame design is semantically
wrong.

The diagnostic work-500 timing, native samples and pad control are retained
under `diagnostics/`. They do not alter the original 12-pair verdict block.

## Reinforcing zero-work controls

Two later rejected investigations make this debt a standing guard against
unmeasured cleanup, not merely a VM-C1b anomaly:

- C2R01's fixed-core reset candidate moved `rxbvm` Sieve by `+1.801135%` and
  its static-flag form by `+2.284914%`, although optimized Sieve reused no frame
  and R1 executed no reset or classifier work. Smaller `run()`/text did not
  produce a stable portfolio win.
- C3R01's changed-only numeric-context synchronization candidate likewise
  cannot execute its intended child-entry/return saving in optimized Sieve,
  which has zero bytecode calls. Over 12 balanced pairs Sieve nevertheless
  moved `+1.440812%` in `rxvm` and `+0.936526%` in `rxbvm`; all C3R01 workload
  intervals crossed zero. The candidate was discarded immediately.

Consequently, do not treat source simplification, fewer duplicated statements,
smaller text or an operation-local machine saving as a performance result in
the flattened interpreter. Every future candidate needs an independently
measured mechanism plus a zero-work drift control in both VMs. Do not repeat
C2R01, exact reset lists, quickened clearing or C3R01 on the current Apple
product without a materially changed ownership/representation contract or the
cross-platform counter matrix below.

## Required revisit matrix

Before final VM architecture selection, compare the exact parent of the
accepted implementation commit against that commit using profiling-off Release
products and identical canonical inputs:

| Platform | Compilers | Required status |
| --- | --- | --- |
| Apple ARM64 | Apple Clang | Reproduce current observation and retain as reference |
| Linux ARM64 | GCC and Clang | Required |
| Linux x86-64 on Intel | GCC and Clang | Required and decision-critical |
| Windows x86-64 | supported MSYS2 GCC | Required by the supported matrix |

For both `rxvm` and `rxbvm`, run canonical Sieve 50, the named Sieve work-500
diagnostic, the common Tier-A portfolio and call-heavy List/Permute guards.
Retain exact compiler versions, flags, product hashes, `run()`/text size and
balanced raw timing. On Linux also retain `perf stat` cycles, instructions,
branches, branch misses, cache references/misses and available L1 instruction
cache/iTLB events, plus a focused `perf record/report` when the regression
reproduces.

Reopen `PERF2-06-D01` if a cell regresses by more than 3%, if the effect is
compiler-specific, or if code-layout variance reverses the accepted portfolio
gain. The preferred remedy direction is to stabilize or partition cold
frame/signal code away from the monolithic flattened `rxbvm` body. Do not undo
the shared-frame policy merely by assumption; compare the exact implementations
on the supported compiler/architecture matrix.
