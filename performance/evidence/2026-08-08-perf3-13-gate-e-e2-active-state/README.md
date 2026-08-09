# PERF3-13 Gate E E2 active-state evidence

Date: 2026-08-08

Branch: `develop`

Source state: published E1/E1-P1 Windows-corrected base
`84d406904ece6842f6cec5a47e75d12b9d28ab16`, plus the uncommitted E2 active
state, direct interrupt slot, RXAS sanitizer repair, tests, evidence and live
PERF3 control-document updates.

## Selected E2 implementation

Each `rxvm_context` owns active RXVML/RXPA bindings, RXPA copy-out scratch, SAY
routing, a logical CREXX directory/environment and the currently published
pending-interrupt pointer. A checked TLS locator identifies the worker-owned VM
active on a thread without storing mutable VM state in TLS. The standalone
product main is the sole raw OS interrupt target. Every execution has one
direct local interrupt word; nested same-worker execution transfers and
restores pending bits. Gate F fan-out remains closed.

CREXX no longer changes the parent CWD or environment. Relative file commands
use worker-local logical state and each child receives an immutable directory
and merged environment snapshot. cREXX remains UTF-8 internally; Windows child
launch converts command, application, directory and environment boundaries to
UTF-16 and uses `CreateProcessW`.

The global reference fallback counter is removed. Context-free compatibility
cells use their allocation-lifetime address identity and are not indexed or
transferable. Ordinary VM allocation remains worker-local and lock-free.

## Accepted first ordinary-Release verdict

The direct execution-slot form was compared with the exact published E1-P1
control using retained identical RXBIN/library inputs, one warmup and 12
pairwise-balanced serial recorded rounds for Sieve and RexxCPS. All 52 timing
processes passed. Positive percentages favour E2.

| Workload | Pairs | Paired mean | 95% interval | Median | Favourable | Verdict |
| --- | ---: | ---: | ---: | ---: | ---: | --- |
| Sieve | 12 | -0.012799% | -0.332055% to +0.306456% | +0.099020% | 8 | neutral |
| RexxCPS | 12 | -1.206404% | -1.760487% to -0.652320% | -1.497618% | 1 | clear adverse, inside guard |
| Pooled two workloads | 24 | -0.609602% | -1.000309% to -0.218894% | -0.468653% | 9 | clear adverse, inside guard |

No 3% workload guard fires. Adrian accepted the bounded RexxCPS regression on
2026-08-08. The queued `PERF3-05-R1`/`PERF3-05-R2` programme owns the broader
flattened-`run()` layout and profile-selected handler cleanup.

The accepted `rxbvm` is 1,020,712 bytes, with 824,628 bytes of `__text`, and
SHA-256
`132aa8a69a1ad9e250dfce8a4ac03905daade9f5e5300f27692c9f179255c841`.
The final rebuilt Release product has the same hash, so the later RXAS repair
does not change the VM verdict artifact.

## RXAS sanitizer fault and no-malloc repair

The first broad sanitizer artifact build found a heap-use-after-free at
`assembler/rxas_flow_proof.c:4413`. Proof records pinned raw pointers to inline
`RxasFlowQueueBatchEntry.original` snapshots; growing the sparse entry array
beyond its initial capacity reallocated those snapshots and left the pins
dangling.

The selected repair retains inline snapshots and stable record IDs. Growth
reports that snapshots may have moved, and the owning flow graph resolves every
proof pin again against the relocated records before another proof query. It
adds no per-record allocation. The contract test forces 17 sparse edits to
cross the initial capacity.

A discarded stable-pointer control used one allocation per edited record. On
the 109,073-byte NR15 reproducer, two balanced ten-assembly Debug blocks were
4.25/4.24 seconds for that malloc control and 4.25/4.25 seconds for the selected
record-ID form. Both produced the same 37,464-byte RXBIN with SHA-256
`85cd2dee58d8afe2c394519bf821254f56bfeb4938ac3ac4115c24e056c45c0d`.
The prior 12-minute CTest appearance was a serial invalidated-artifact fixture,
not one hung test; prebuilding that fixture four-way reduced its included CTest
setup to 8.56 seconds.

## Accepted closeout QA

| Gate | Result |
| --- | --- |
| Pre-verdict focused Debug | 39/39 passed |
| First-verdict timing processes | 52/52 passed |
| RXAS flow contract under ASan | 1/1 passed |
| Focused E2 AddressSanitizer, fixture included | 35/35 passed in 40.19 seconds |
| Complete AddressSanitizer CTest | 1,999/1,999 passed in 727.05 seconds |
| Full ordinary Debug | 1,999/1,999 passed with `--parallel 30` in 244.79 seconds |
| Focused ordinary Release | 49/49 passed in 7.98 seconds |
| Diff hygiene | `git diff --check` passed |

No CTest was excluded or skipped in the complete sanitizer or Debug runs.
Apple LeakSanitizer is unsupported, so the supported sanitizer runs used
`detect_leaks=0`; exact Debug allocation-teardown assertions remain enabled.
Native Windows-MinGW, MSVC, Intel Linux and Linux ARM64 E2 execution proof
remains ordered portable follow-up after local review.

Host: Apple M5, 10 logical CPUs, Darwin 25.5.0 arm64. Toolchain: Apple Clang
21.0.0, CMake 4.3.2, Ninja 1.13.2. Release configuration:
`CMAKE_BUILD_TYPE=Release`, `CREXX_VM_PROFILING=OFF`.
