# PERF3-13 Gate E E3b-P1 branch-free production verdict

Date: 2026-08-10

Branch: `develop`

Control source: accepted E3a commit
`29ef1975ec0190bdd1b246a76211f727fa720dce`; the retained control VMs match
the accepted E3a closeout hashes.

Candidate: the frozen E3b-P1 A/C compatibility implementation with a
procedure-bound native invoker and the approved cold, sticky legacy
coordinator.

Status: **accepted, guard-clean, and passed Mac closeout**. Adrian accepted
the verdict and authorized QA and a local commit on 2026-08-10. P2 session
factories/default sessions, cross-platform proof, public workers/channels and
Gate F remain separate approval gates.

## Selected production form

The existing `_initfuncs` ABI and ordinary `ADDPROC` call surface remain
compatible. An audited plugin may add `RXPA_PLUGIN_PROCESS_REENTRANT`, which
exports a versioned optional capability manifest. An old host ignores the
manifest; a new host treats absent, malformed or unknown capabilities as
legacy.

Each loaded native runtime procedure stores its selected invoker. An ordinary
native call loads that invoker and performs one indirect call; it does not test
the capability word or enter a policy wrapper. Process-reentrant procedures
remain permanently bound to the direct adapter.

The first VM context that loads any unmarked procedure also binds that legacy
procedure directly. A second legacy-capable context starts a cold process-wide
transition, prevents new direct legacy execution, waits for active VM execution
boundaries to drain, and rebinds only registered legacy procedures to the
recursive locked adapter. The transition is sticky. A context that exposes
only process-reentrant procedures does not trigger it.

Static registrations are retained in a synchronized, owned catalogue and
replayed into every VM. Dynamic plugin handles are owned by the VM until values,
native payloads, modules and other plugin-code users are destroyed. P1 keeps
native-payload copy/finalize callbacks on the recursive compatibility lock.

## Paired ordinary-Release verdict

The exact control and candidate are ordinary `Release`, profiling-off,
`profile-20` builds. Every pair shares the same RXBIN, library and plugin
images. One warmup and 12 pairwise-balanced recorded rounds ran serially per
cell.

All 312/312 processes passed: 24 warmups and 288 recorded executions. Positive
elapsed percentages are adverse; positive RexxCPS rates are favorable.

| Workload | VM | Paired mean | Mean 95% interval | Result | Guard |
| --- | --- | ---: | ---: | --- | --- |
| RXPA process-reentrant, 20M calls | `rxbvm` product | +1.368096% | -0.042248% to +2.778441% | noisy inconclusive | clear |
| RXPA process-reentrant, 20M calls | `rxtvm` guard | +2.175049% | +1.417704% to +2.932394% | clear adverse | clear |
| RXPA legacy, 20M calls | `rxbvm` product | +0.104869% | -1.105497% to +1.315234% | noisy inconclusive | clear |
| RXPA legacy, 20M calls | `rxtvm` guard | -0.413910% | -1.754527% to +0.926707% | noisy inconclusive | clear |
| Reentrant one-call lifecycle | `rxbvm` product | +1.179650% | +0.232019% to +2.127282% | clear adverse | clear |
| Reentrant one-call lifecycle | `rxtvm` guard | -0.208562% | -1.182716% to +0.765592% | noisy inconclusive | clear |
| Legacy one-call lifecycle | `rxbvm` product | +0.246539% | -0.744738% to +1.237817% | noisy inconclusive | clear |
| Legacy one-call lifecycle | `rxtvm` guard | +0.486907% | -0.299868% to +1.273682% | noisy inconclusive | clear |
| Sieve | `rxbvm` product | -0.234163% | -1.133483% to +0.665157% | noisy inconclusive | clear |
| Sieve | `rxtvm` guard | -0.194978% | -1.031578% to +0.641622% | noisy inconclusive | clear |
| RexxCPS rate | `rxbvm` product | +0.624971% | -0.400931% to +1.650873% | noisy inconclusive | clear |
| RexxCPS rate | `rxtvm` guard | -0.375677% | -1.339248% to +0.587894% | noisy inconclusive | clear |

The product `rxbvm` process-reentrant result has a +1.37% adverse tendency but
is inconclusive. The `rxtvm` guard result is a measured +2.18% adverse effect,
still below the 3% hot-kernel guard. Legacy calls are neutral on both VMs. The
former 14-20% losses from the out-of-line and per-call-branch candidates are
removed.

The one-call rows include process startup, plugin load, one call and teardown.
Their median process deltas range from -0.0365 ms to +0.1625 ms and do not meet
the lifecycle dual escalation threshold. Sieve, canonical RexxCPS and artifact
guards are all clear.

## Machine-code and VM shape

There is no `rxvm_call_native_procedure` symbol in either candidate VM. Each
inspected native call site loads the preselected invoker and native function
and uses `blr`, with no capability branch. The profile-20 hot owner changes by
+412 bytes on `rxbvm` and -236 bytes on `rxtvm`.

| VM | Control file | Candidate file | Delta | Control `__text` | Candidate `__text` | Delta |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `rxbvm` | 1,113,112 | 1,132,136 | +19,024 (+1.71%) | 895,980 | 902,856 | +6,876 |
| `rxtvm` | 1,113,240 | 1,132,264 | +19,024 (+1.71%) | 901,504 | 907,732 | +6,228 |

Both candidate `__TEXT` segments are 983,040 bytes versus 966,656 in the
control. File growth remains below the artifact dual threshold.

## Mac closeout

The full Debug build and CTest pass 2,017/2,017 tests. The focused ordinary
Release concurrency/ownership panel passes 11/11, including static replay,
dynamic lifetime, invalid-manifest fallback, permanent reentrant binding,
legacy serialization and transition quiescence.

Broad QA exposed that the internal RXVML ADDRESS bridge had not declared its
already-existing process-reentrant property. Its callbacks resolve mutable
state through the active RXVML context and the established
`e2_active_context_isolation` test intentionally synchronizes two concurrent
callbacks. Registering those five internal procedures under one reentrant
static-plugin identity repairs that deadlock. The rebuilt Release `rxbvm` and
`rxtvm` retain the exact timed hashes, so no performance rerun is warranted.

## Host and interpretation boundary

Host: Mac17,3, Apple M5, 10 logical CPUs, Darwin 25.5.0 arm64. Toolchain:
Apple Clang 21.0.0, CMake 4.2.2 and Ninja 1.13.2. The formal run started at
16:55:46 UTC and completed at 17:00:00 UTC on AC power with low-power mode
disabled. No thermal, performance or CPU-power warning was recorded before or
after capture; no build, test or second benchmark runner overlapped it.

This is a same-host accepted candidate decision, not a cross-platform or final
release claim. Timing rows are measured paired evidence. The call-shape
attribution is direct assembly inspection. The reentrancy assertion remains a
plugin-author correctness contract, not a host-side proof of safety.
