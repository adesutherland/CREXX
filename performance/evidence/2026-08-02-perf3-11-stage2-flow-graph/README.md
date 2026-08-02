# PERF3-11 Stage 2 immutable flow graph

Status: **complete — Gate 2 passes**

This bundle closes the consumer-free immutable `FlowProcedure` construction
stage. It does not select a new optimizer rewrite and does not replace the
retained runtime performance baseline.

## Provenance

- Branch: `codex/perf3-rxas-flow-infrastructure`
- Stage 1 signal-contract base: `75710c365a15637b38c0d938df474bb685838dc9`
- Host: Darwin 25.5.0 ARM64, 10 logical CPUs, 24 GiB RAM
- Power: AC; low-power mode off
- Thermal state: no recorded thermal, performance or CPU-power warning
- Builds: CMake/Ninja Debug and Release, `CREXX_VM_PROFILING=OFF`
- Frozen comparator: Stage 0 Release `rxas` retained under
  `/tmp/crexx-perf3-11-stage0.35IBzf/base-binaries/rxas`

## Construction result

The new internal `rxas_flow_graph` module snapshots one procedure into stable
record, instruction and code-block identities plus seven synthetic
root/exit blocks. It records exact pre-emission addresses and typed normal,
branch, signal-skip, signal-retry, handler, terminal, unwind and unknown
edges. Every query requires the graph epoch and fails closed when stale.

The construction path is expected linear in queued records plus edges:

1. one record/opcode snapshot pass;
2. an open-addressed label index;
3. one leader/block pass;
4. append-only edge construction, with constant-time handler-target marking;
5. no edge-list rescans or graph sorting.

While the legacy optimizer remains, its final resolved `OpInfo` pointers are
handed to the immutable builder before the legacy graph is freed. This avoids
a second opcode-table parse and avoids overlapping the two graph allocations.
The standalone builder still resolves opcodes itself for tests and future use.

Deterministic `rxas -d` output includes every block, record, instruction and
typed edge without pointer values. The permanent C contract covers unreachable
code, diamonds, nested loops, irreducible control, call boundaries, source and
TRACE mapping, every signal continuation/root, unknown-opcode fail-closed flow,
dump determinism and stale epochs.

## Correctness and image result

- Focused Debug matrix: **113/113 passed**. This includes every RXAS optimizer
  test, the new graph contract, signal lifecycle cases, both VMs' optimized and
  no-opt signal/storage/conversion fixtures, and both decimal plugins.
- Ordinary profiling-off Release `rxas` builds.
- Canonical Richards, Towers and RexxCPS RXBIN hashes remain exactly equal to
  Gate 0. The immutable graph has no rewrite consumer and ordinary emission is
  unchanged.

## Assembler-cost result

The first ten-sample check correctly rejected the initial implementation:
re-resolving every opcode after the legacy graph added 20.4% Richards, 4.1%
Towers and 9.2% RexxCPS median elapsed. That duplicate parse was removed before
the gate was rerun.

The final result uses two warmups and 30 balanced/interleaved elapsed rounds
per workload against the frozen Stage 0 binary in the same session. Peak RSS
uses ten separately interleaved samples per binary/workload. Post-run load was
`{ 1.18 2.24 2.77 }`; later capture after evidence reduction was
`{ 1.16 1.95 2.60 }`.

| Workload | Frozen median | Stage 2 median | Elapsed delta | Frozen median RSS | Stage 2 median RSS | RSS delta |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Richards | 0.059193969 s | 0.059437513 s | +0.411% | 8,626,176 B | 9,068,544 B | +442,368 B (+5.128%) |
| Towers | 0.018989921 s | 0.019077898 s | +0.463% | 4,710,400 B | 5,013,504 B | +303,104 B (+6.435%) |
| RexxCPS | 0.055488587 s | 0.053943515 s | -2.784% | 9,748,480 B | 9,912,320 B | +163,840 B (+1.681%) |

All elapsed cells remain inside the 3% per-workload guard. The two small RSS
percentages above 5% remain far below the standing RSS escalation rule, which
requires a regression greater than both 5% and 1 MiB. No Gate 2 cost guard is
hit.

## Gate 2 verdict

Gate 2 passes: construction is consumer-free, fail-closed, deterministic and
expected linear; focused correctness and exact output-image parity pass; and
elapsed/RSS remain inside guard. Stage 3 may add reusable structural analyses
under the graph epoch without changing emitted code.
