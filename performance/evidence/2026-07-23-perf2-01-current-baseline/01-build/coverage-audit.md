# Profiler and evidence-tool coverage audit

This table was first recorded in `performance/PERF2-01-WORKLIST.md` before the
schema-5 implementation. It is reproduced here with final disposition.

| Required domain | Initial disposition | Final evidence / decision |
| --- | --- | --- |
| Opcode, transition, procedure, call and RXSEQ | available now | schema-4 rows retained; schema 5 remains backward-readable; RXSEQ N=2/3/4 captured for all 44 opt/no-opt VM entries |
| Fresh/reused frames and entry subphases | missing and required | schema-5 frame-entry phase rows, counts and selected timing; fresh/reused disposition retained |
| Copy, typed-copy, move, clear, reset and destroy by shape/bytes | missing and required | schema-5 value rows at common helpers and typed handlers; nested helper attribution is explicit |
| Conversion/materialization/cache | derivable now | explicit conversion/value/buffer rows summarized; the current eager value model has no general representation cache, so generic hits/misses were not invented |
| Branch direction and backedges | missing and required | stable module/index/opcode branch-site rows with taken/fall-through/cross-module/backedge counts |
| Stable sites/types/targets/cache | not justified | current optimized modules execute zero selector attempts; existing aggregate selector status retained and reported as not exercised |
| Lifecycle phases | derivable now | profiling-off process lifecycle, first-frame profile rows, artifact sizes and native load stacks kept in separate lanes; no perturbing inner loader clocks added |
| Allocation lifetime/stacks | available now | VM request counts/bytes plus macOS `sample`, `heap`, `vmmap` and `malloc_history` for selected outliers |
| Static/dynamic image shape | derivable now | exact `rxdas` disassembly, artifact inventory, profile counts and RXSEQ module records summarized by cREXX tools |

Schema 5 adds explicit per-domain `complete`, `degraded` and `overflow` state.
All 44 accepted optimized/no-opt count profiles and all selected timing profiles
are complete with no degradation or overflow. Counts-only mode writes zero
timing fields by contract and is byte-deterministic for deterministic workloads.
RexxCPS self-calibrates from wall time, so its executed count differs between
VMs/runs even when the counts profiler itself is deterministic.

The maintained control plane is cREXX Level B:

- `performance/tools/run_evidence_bundle.crexx`
- `performance/tools/summarize_perf2_profiles.crexx`
- `performance/tools/summarize_perf2_artifacts.crexx`
- `performance/tools/build_perf2_gap_ledger.crexx`
- the existing cREXX timing, lifecycle and inventory tools

No Python analysis or orchestration program was introduced. Shell is used only
to invoke builds and native host utilities.
