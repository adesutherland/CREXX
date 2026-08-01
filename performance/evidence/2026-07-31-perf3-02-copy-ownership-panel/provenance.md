# PERF3-02 provenance

All diagnostic and PoC worktrees were clean detached copies of product commit
`3f43a0014be10c930a12b8a636297b60f294c0a6`. The main checkout remained on
`develop` at evidence-only commit `e38e514bf611ae3873513368c44742e2ae7332d1`.

| Variant | Detached source root | Build | Retained source diff |
| --- | --- | --- | --- |
| C0 | `/private/tmp/crexx-perf3-01.jetu7C` | ordinary Release, profiling off | accepted PERF3-01 product |
| diagnostic | `/private/tmp/crexx-perf3-02-analysis.Pk5z1L/src` | Release, profiling on | `diagnostic/source.diff` |
| C1a-R1 | `/private/tmp/crexx-perf3-02-c1a-r1.1XABpy/src` | ordinary Release, profiling off | `poc/c1a-r1/source.diff` |
| C1a-R2 | `/private/tmp/crexx-perf3-02-c1a-r2.wDliEF/src` | ordinary Release plus diagnostic profile | `poc/c1a-r2/source.diff` |
| C1c-R1 | `/private/tmp/crexx-perf3-02-c1c-r1.34OsFa/src` | ordinary Release plus diagnostic profile | `poc/c1c-r1/source.diff` |
| C2-E1 | `/private/tmp/crexx-perf3-02-c2-e1.TqLL2w/src` | ordinary Release assembler | `poc/c2-e1/source.diff` |

Every retained source diff passed reverse-apply validation against its own
worktree. `artifacts.csv` records the exact cache/product/image hashes. C1a-R2
and C1c-R1 no-opt Richards/Towers RXAS and RXBIN are byte-identical to C0; only
C1a-R2 optimized Richards and C1c-R1 optimized Towers change. The C2-E1 and C0
assemblers produce byte-identical images to each other under the same retained
invocation. Those reassemblies differ from the original CMake-built RXBIN, so
the claim is deliberately limited to within-experiment identity.

The diagnostic-only VM change adds RXSEQ site decoding and per-site generic
full-copy payload accounting. It was never used as a timing product. RXSEQ
captures used `N=2`; count profiles used the accepted smoke work. Both VMs
agreed on every deterministic count.

Both first timing campaigns used the ordinary profiling-off Release products,
AC power, low-power mode off, serial workload rotation, one warmup and 12
recorded rounds. No build, CTest or benchmark overlapped either campaign.
Adrian subsequently identified an active remote terminal with significant Mac
impact, so those wall-clock rows are provisional despite their internally
paired design. They remain in `summary/timing-effects.csv`, and their raw
schedules, samples, outputs and host boundaries remain under each candidate's
`timing/`.

Adrian confirmed the remote terminal was off before the authoritative rerun.
The host remained on AC at 80% battery, low-power mode off, with no recorded
thermal/performance/CPU-power warning and only the console login. All 36
artifact rows and 11 unique timing inputs were hash-verified; the PERF3-01
bundle replayed 101/101 checksums. Every timing cell used the exact C0 VM
binaries so only the compiler-generated main/library images varied.

The clean-host campaign retained 376 recorded samples and 36 warmups across
five serial blocks: initial 144+12, absolute-noise append 80+8, guard append 01
96+8, Richards cap 8+4 and Towers guard append 02 48+4. All 412 executions
passed correctness; every driver stderr is empty. The absolute-noise append
retained all six triggered cells plus two matched C0 partners. Zero-crossing
guard intervals then received balanced pairs to exactly 36 per guard. Target
decisions use 22 pairs except Towers `rxbvm` C1c-R1, whose clean initial 12 did
not trigger an append. `rerun/run-v1-final-summary/summary.csv` combines every
raw file for absolute statistics; `rerun/paired-effects.csv` pairs only cells
present in the same block and round.

The retained CRI-13 projection is referenced from
`../2026-07-29-crexx-rag-integration-ledger/CRI13-R1-RXAS-TRACE.md`. It is not
copied, merged into current Mac timing, or presented as a current workload.
