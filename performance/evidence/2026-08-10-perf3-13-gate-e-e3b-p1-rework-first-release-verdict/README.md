# PERF3-13 Gate E E3b-P1 inline-dispatch rework verdict

Date: 2026-08-10

Branch: `develop`

Control source: accepted E3a commit
`29ef1975ec0190bdd1b246a76211f727fa720dce`; the exact retained E3a VMs
match the accepted closeout hashes.

Candidate: the frozen, uncommitted E3b-P1 candidate after Adrian approved the
bounded inline-dispatch rework prompted by the first failed verdict.

Status: **failed hot-primitive verdict; a new design decision is required**.
The candidate is not accepted or committed.

## Bounded rework

The RXPA public contract, catalogue/DSO ownership, capability word and legacy
recursive-lock semantics are unchanged. The rework removes the out-of-line
policy frame. `rxvm_call_native_procedure()` is an always-inline Release helper
whose hot call sites load the procedure capability and branch directly to the
former adapter for process-reentrant plugins or the outlined legacy wrapper.

Debug and Release concurrency/ownership panels each pass 7/7. Disassembly
shows direct `bl _rxvm_callfunc_direct` and `bl _rxvm_callfunc` arms and no
`rxvm_call_native_procedure` symbol. The old control adapter and candidate
direct adapter each compile to the same 556-byte body.

## Paired Release verdict

The exact control and candidate are ordinary `Release`, profiling-off,
`profile-20` builds. Every pair shares the same RXBIN, library and plugin
images. One warmup and 12 pairwise-balanced recorded rounds ran serially per
cell.

All 312/312 processes passed: 24 warmups and 288 recorded executions.
Positive elapsed percentages are adverse; positive RexxCPS rates are
favorable.

| Workload | VM | Paired mean | Mean 95% interval | Result | Guard |
| --- | --- | ---: | ---: | --- | --- |
| RXPA process-reentrant, 20M calls | `rxbvm` product | +14.501217% | +12.983946% to +16.018488% | clear adverse | hit |
| RXPA process-reentrant, 20M calls | `rxtvm` guard | +14.782493% | +12.317548% to +17.247438% | clear adverse | hit |
| RXPA legacy, 20M calls | `rxbvm` product | +14.750908% | +13.341563% to +16.160253% | clear adverse | hit |
| RXPA legacy, 20M calls | `rxtvm` guard | +13.924160% | +11.812487% to +16.035833% | clear adverse | hit |
| Reentrant one-call lifecycle | `rxbvm` product | +0.280000% | -0.600081% to +1.160082% | inconclusive | clear |
| Reentrant one-call lifecycle | `rxtvm` guard | +0.488296% | -0.241435% to +1.218026% | inconclusive | clear |
| Legacy one-call lifecycle | `rxbvm` product | +0.721916% | +0.143155% to +1.300676% | clear adverse | clear |
| Legacy one-call lifecycle | `rxtvm` guard | +0.792556% | +0.040662% to +1.544451% | clear adverse | clear |
| Sieve | `rxbvm` product | -0.449170% | -1.538679% to +0.640340% | inconclusive | clear |
| Sieve | `rxtvm` guard | -0.279062% | -1.039632% to +0.481508% | inconclusive | clear |
| RexxCPS rate | `rxbvm` product | -0.767747% | -1.265670% to -0.269823% | clear adverse | clear |
| RexxCPS rate | `rxtvm` guard | -0.117280% | -0.817767% to +0.583206% | inconclusive | clear |

The median incremental process-reentrant costs are 9.319325 ns/call for
`rxbvm` and 8.720900 ns/call for `rxtvm`. Legacy median increments are
9.271900 and 8.110275 ns/call respectively. Removing the extra frame improves
the first failed candidate but does not reach the raw native-call ceiling.

The one-call rows combine startup, plugin load, one call and teardown. Their
median deltas are 0.0375-0.1515 ms, below the lifecycle dual threshold.

## Cause and next decision

The direct adapter body is identical in size to the control adapter. The
remaining measured hot-path difference is the capability byte load,
conditional branch and split direct/legacy call target emitted at every native
call site. The instruction-loop owner changes by only +332 bytes on `rxbvm`
and -216 bytes on `rxtvm`, but the repeated policy decision still costs
approximately 8-9 ns per call.

Another production edit is not authorized by this evidence. A further design
must remove the per-call conditional policy selection. Plausible controls are
a load-time selected, predictably indirect invoker and a safely transitioned
single-legacy-executor direct mode. Those forms must be compared in isolation
before selecting another production candidate; moving the same branch into a
different wrapper is not supported by this result.

## VM shape

| VM | Control file | Candidate file | Delta | Control `__text` | Candidate `__text` | Delta | Hot-owner delta |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `rxbvm` | 1,113,112 | 1,131,096 | +17,984 (+1.62%) | 895,980 | 900,932 | +4,952 | +332 |
| `rxtvm` | 1,113,240 | 1,131,256 | +18,016 (+1.62%) | 901,504 | 905,908 | +4,404 | -216 |

Both `__TEXT` segments are 983,040 bytes versus 966,656 in the control. File
growth remains below the artifact dual threshold.

## Host and interpretation boundary

Host: Mac17,3, Apple M5, 10 logical CPUs, Darwin 25.5.0 arm64. Toolchain:
Apple Clang 21.0.0, CMake 4.2.2 and Ninja 1.13.2. The formal run started at
15:59:28 UTC and completed at 16:03:49 UTC on AC power with low-power mode
disabled. No thermal, performance or CPU-power warning was recorded before or
after capture; no build, test or second benchmark runner overlapped it.

This is a same-host rejected-candidate decision result, not a release claim.
The timing is measured paired evidence; the assembly attribution is direct
inspection; the proposed branch-free controls are unmeasured hypotheses.
