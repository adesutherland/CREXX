# PERF3-13 Gate E E3b-P1 first ordinary-Release verdict

Date: 2026-08-10

Branch: `develop`

Control source: accepted E3a commit
`29ef1975ec0190bdd1b246a76211f727fa720dce`; the exact retained E3a VM
binaries embed their pre-commit equivalent version
`g6d12cd921cdb.dirty` and match the accepted closeout hashes.

Candidate: the same commit plus the frozen, uncommitted E3b-P1 RXPA
compatibility/capability implementation, tests, benchmark kernels and live
performance documentation.

Status: **failed hot-primitive verdict; rework or revert direction required**.
The candidate is not accepted or committed.

## Frozen E3b-P1 candidate

The candidate preserves the legacy `_initfuncs(rxpa_initctxptr)` and
`rxpa_libfunc` ABI. Static registrations become an owned synchronized process
catalogue replayed into every VM; each successful dynamic plugin handle is
owned by its VM through final teardown. An absent or invalid optional version-1
manifest selects a process-wide recursive compatibility lock. An audited
plugin may assert `RXPA_PLUGIN_PROCESS_REENTRANT` to bypass that lock. Native
payload copy/finalize callbacks remain serialized in P1.

Focused Debug proved simultaneous static replay, legacy serialization,
process-reentrant overlap, recursive re-entry, manifest validation/fail-closed
behavior and two-context dynamic DSO lifetime. The same seven concurrency and
ownership tests pass in the frozen ordinary Release build.

## First ordinary-Release verdict

The exact control and candidate are ordinary `Release`, profiling-off,
`profile-20` builds. Every pair shares the same RXBIN, library and plugin
images. The matrix ran serially with one warmup and 12 pairwise-balanced
recorded rounds per cell.

All 312/312 processes passed: 24 warmups and 288 recorded executions.
Positive elapsed percentages are adverse; positive RexxCPS rates are
favorable.

| Workload | VM | Paired mean | Mean 95% interval | Result | Guard |
| --- | --- | ---: | ---: | --- | --- |
| RXPA process-reentrant, 20M calls | `rxbvm` product | +20.382448% | +18.473761% to +22.291134% | clear adverse | hit |
| RXPA process-reentrant, 20M calls | `rxtvm` guard | +14.387105% | +13.315358% to +15.458851% | clear adverse | hit |
| RXPA legacy, 20M calls | `rxbvm` product | +19.792936% | +18.039807% to +21.546065% | clear adverse | hit |
| RXPA legacy, 20M calls | `rxtvm` guard | +16.032255% | +14.704983% to +17.359528% | clear adverse | hit |
| Reentrant one-call lifecycle | `rxbvm` product | +1.104973% | -0.013726% to +2.223673% | inconclusive | clear |
| Reentrant one-call lifecycle | `rxtvm` guard | +0.641824% | -0.147413% to +1.431060% | inconclusive | clear |
| Legacy one-call lifecycle | `rxbvm` product | +0.454440% | -0.282523% to +1.191404% | inconclusive | clear |
| Legacy one-call lifecycle | `rxtvm` guard | +0.547015% | -0.045363% to +1.139393% | inconclusive | clear |
| Sieve | `rxbvm` product | -0.358407% | -0.770632% to +0.053818% | inconclusive | clear |
| Sieve | `rxtvm` guard | +1.485589% | +0.175391% to +2.795786% | clear adverse | clear |
| RexxCPS rate | `rxbvm` product | +0.033457% | -0.453125% to +0.520039% | inconclusive | clear |
| RexxCPS rate | `rxtvm` guard | -0.037650% | -1.116279% to +1.040978% | inconclusive | clear |

The median incremental bypass costs are 12.513850 ns/call for `rxbvm` and
8.911175 ns/call for `rxtvm`. Legacy median increments are 12.453550 and
9.493175 ns/call respectively. The similarity shows that the uncontended
recursive lock is not the dominant loss in this candidate.

The one-call rows are a combined startup, plugin load, one native call and
teardown observation, not separate named-phase measurements. Their median
deltas are only 0.0365-0.1965 ms, below the lifecycle dual threshold.

## Cause and decision boundary

Mach-O disassembly shows that the old handler called the RXPA adapter once.
The candidate first calls the new out-of-line
`rxvm_call_native_procedure()`, which creates a 64-byte register-save frame,
loads the capability and branches before tail-calling the former adapter.
That frame is also paid by the asserted-reentrant bypass and explains the
clear kernel loss. The instruction-loop owner itself shrank, so this is call
boundary overhead rather than renewed hot-loop code growth.

A bounded rework could keep the capability test in the existing hot
handler/helper surface, make one direct adapter call for a process-reentrant
procedure and outline only the legacy compatibility path. This evidence does
not authorize that edit. The mandatory gate stops here for Adrian to select
rework, revert or an explicit trade-off.

## VM shape

| VM | Control file | Candidate file | Delta | Control `__text` | Candidate `__text` | Delta | Hot-owner delta |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `rxbvm` | 1,113,112 | 1,131,160 | +18,048 (+1.62%) | 895,980 | 900,640 | +4,660 | -240 |
| `rxtvm` | 1,113,240 | 1,131,304 | +18,064 (+1.62%) | 901,504 | 905,552 | +4,048 | -852 |

Both `__TEXT` segments grow from 966,656 to 983,040 bytes. File growth exceeds
4 KiB but remains below 5%, so the artifact dual threshold does not fire.

## Host and interpretation boundary

Host: Mac17,3, Apple M5, 10 logical CPUs, Darwin 25.5.0 arm64. Toolchain:
Apple Clang 21.0.0, CMake 4.2.2 and Ninja 1.13.2. The formal run started at
15:18:53 UTC and completed at 15:23:18 UTC on AC power with low-power mode
disabled. No thermal, performance or CPU-power warning was recorded before or
after capture; no build, test or second benchmark runner overlapped it.

This is a same-host first-candidate decision result, not a release claim. The
native-call loss is a measured paired result. The assembly attribution is a
direct inspection of the frozen binaries. The proposed rework is an inference
that needs its own focused correctness and first-Release verdict.
