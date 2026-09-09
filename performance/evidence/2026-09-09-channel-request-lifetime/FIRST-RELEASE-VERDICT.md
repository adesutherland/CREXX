# Channel request lifetime: first Release verdict

2026-09-09. Provisional implementation on `hotfix` in
`/Users/adrian/CLionProjects/CREXX-hotfix`, based on
`d840dc1b0730b793b145b767979b3905e6f4266d`. Awaiting Adrian's explicit memory
tradeoff decision. No commit, install, publication or downstream changes.

## Verdict and decision

The focused checks pass and the Release observations support explicit request
release and direct cleanup. Recommend accepting the increased memory cost when
many requests are deliberately retained, then proceeding with the remaining
approved validation and documentation. The memory guard is crossed; this
recommendation is not approval or release qualification.

The caller-visible operation is `.channelrequest.release()`, implemented by
`chanrelease status,channel,ticket`. It requires successful terminal observation,
reclaims the provider request and ticket, and invalidates request copies. Saved
completion values survive. Existing `start`/`wait` callers must opt in; operations
without release still reach the 65,535 retained-ticket limit. CREXX-owned byte
helpers and HTTP consumers adopt release. RAG adoption remains separate.

Structured tasks retain their scope-owned join outcomes: task copies, including
DO PARALLEL result copyback after scope close, use those outcomes after channel
tracking is reclaimed. No structured task detachment API was added.

## Observations

| Cell | Baseline | Candidate |
|---|---:|---:|
| 65,535 retained requests: submit/observe CPU | 1.169–1.266 s | 0.018–0.019 s |
| Same requests: close CPU | 13.920–14.311 s | 0.001 s |
| Same requests: peak process RSS | 18,939,904–18,956,288 B | 21,561,344–21,594,112 B |
| 200,000 explicit releases: live tickets at each checkpoint | API unavailable | 0 |
| Explicit-release native probe: peak RSS | API unavailable | 6,193,152 B |
| Optimized public API, 1,000 release cycles | API unavailable | 0.03 s real; 14,614,528 B RSS |
| Same public image, 200,000 release cycles | API unavailable | 2.93 s real; 14,680,064 B RSS |

Retained-request RSS increases about 2.51 MiB / 13.9% using paired means. This
exceeds both the 5% and 1 MiB guards. New ticket-list links and provider previous
pointers add per-retained-request storage; allocator rounding and process image
layout also influence RSS. The measurements do not allocate the whole observed
RSS difference to one source structure.

The public-API result increases peak RSS by 64 KiB between 1,000 and 200,000
cycles. This is consistent with bounded reusable bookkeeping at these sizes,
not a proof of flat memory for all workloads or indefinite execution. Existing
16-bit generation retirement and the finite capability namespace remain intact.

These are single-mechanism diagnostics, not a formal performance campaign or
representative portfolio claim. The native probe links the selected Release VM
archives but does not time execution of Rexx bytecode. The separate public image
runs the ordinary compiler-selected `rxvm` (a symlink to `rxbvm` here). RexxCPS
and cross-runtime aggregates are technically inapplicable to this bounded
channel ownership mechanism. No precise speedup factor is claimed: close is
reported only to millisecond precision by the unchanged probe.

## Sampling and provenance

Host: Apple M5, 10 logical CPUs, 24 GiB RAM, macOS 26.6.2 / Darwin 25.6.0 ARM64;
AppleClang, CMake/Ninja, Release, VM profiling OFF, native Network TLS backend.
Host/power/toolchain snapshots are in `host-before.json` and `host-after.json`.
AC power, low power off; no recorded thermal/performance warning. Tests and
builds were finished before serial measurements.

The original retained baseline is
`/tmp/crexx-channel-investigation.NMIYvV/ticket-retention-hotfix.log`:
1.203 s submit/observe CPU, 14.004 s close CPU, 18,972,672 B peak RSS. Its unchanged
executable is `ticket_retention_hotfix` in that directory, SHA256
`19c6dc7d7991dd9e3dd935da0a59934c222c86e27000aa8a2961eb31cb2753bf`.

The initial candidate observation is `retained-release-1.log`. Its RSS guard
hit justified a same-session drift/tradeoff check. Two balanced diagnostic pairs
were run serially in AB/BA order, with no discarded samples or separate warmup:
`paired-a1.log`, `paired-b1.log`, `paired-b2.log`, `paired-a2.log`. Each command
returned zero, meaning the retained-ticket exhaustion behavior was reproduced.
That is not acceptance of repaired sustained use. These two pairs do not meet
the formal 12-pair campaign minimum and are not presented as formal estimates.

`released-release-1.log` is the explicit-release variant. Its zero exit requires
200,000 successful operations and successful close. `lifetime-release.log` is
the native lifecycle regression. `typed-1000-release.log` and
`typed-200000-release.log` require the public test's PASS output and exit zero.
The public image includes its fixed preliminary alias/rehash/cancellation checks
in both process measurements. Source and TRACE metadata are retained; no strip
or alternate optimization path was used.

`source-freeze.json` hashes all source inputs at the freeze. `candidate.patch`
records the tracked diff at that point; the two new test files are in the
worktree and covered by the manifest. Full rehash after measurement found no
changes. Only the two worklist/status Markdown files were updated afterward.
`artifact-provenance.json` records candidate binary/image hashes and sizes;
`results.json` retains parsed metrics alongside every raw log.

## Reproduction commands

Run these from the hotfix worktree. All products and logs stay in this scratch
directory; the old baseline build and executable remain unchanged.

```sh
cmake -S /Users/adrian/CLionProjects/CREXX-hotfix -B /tmp/crexx-channel-release.1v4pFt/release -G Ninja -DCMAKE_BUILD_TYPE=Release -DCREXX_VM_PROFILING=OFF -DCREXX_ALLOW_NETWORK_DOWNLOADS=OFF
cmake --build /tmp/crexx-channel-release.1v4pFt/release --parallel 6 --target rxvm rxvml testChannelRequestRelease test_rxvmchannel_lifetime rxfnsg rxdas
```

The exact native compiler/link argv and working directories are retained in
`probe-build-commands.json`; sources are `ticket_retention_candidate.c` (same
main as the baseline) and `ticket_released_candidate.c` (200,000 cycles with
explicit release). Run `/usr/bin/time -l` against either executable, redirecting
both output streams to its named log. The baseline command is the same, using
the unchanged executable named above.

```sh
/tmp/crexx-channel-release.1v4pFt/release/bin/rxlink -o /tmp/crexx-channel-release.1v4pFt/release_cycles /tmp/crexx-channel-release.1v4pFt/release/lib/classlib/tests_functional/testChannelRequestRelease_rxbvm_opt.rxbin /tmp/crexx-channel-release.1v4pFt/release/bin/library /tmp/crexx-channel-release.1v4pFt/release/bin/classlib
/usr/bin/time -l /tmp/crexx-channel-release.1v4pFt/release/bin/rxvm /tmp/crexx-channel-release.1v4pFt/release_cycles -a 1000
/usr/bin/time -l /tmp/crexx-channel-release.1v4pFt/release/bin/rxvm /tmp/crexx-channel-release.1v4pFt/release_cycles -a 200000
```

## Correctness evidence and remaining work

28 distinct focused Debug checks pass using the final relevant inputs: six
native/toolchain checks in `focused-debug-2.log`, followed by all 22 typed/library
checks in `focused-debug-3.log`. The C/toolchain inputs were unchanged by the
subsequent task-cache correction. Tests cover capacity recovery, 200,000 reuse
cycles crossing generation retirement, stale/wrong-owner capabilities, copied
requests, saved completions, cancellation/deadlines, failed materialization,
retry after provider destruction failure, mixed-channel cleanup, asynchronous
byte I/O, child/process paths, opcode metadata/feature round-trip, optimized and
unoptimized library paths on both VMs, HTTP streaming and server failures, and
structured task copies after close.

Earlier build/test failures are retained. The new code initially had factory and
lexical-scope mistakes; the focused runs also exposed a real compatibility
regression in task-result access after close. These were corrected before the
freeze. No compiler workaround or optimization bypass was used.

Pending after Adrian's decision: remaining provider/lifecycle edge coverage,
the agreed broad QA, documentation and public compatibility guidance, maintained
sanitizer runs, and Linux/Windows thread/process cleanup evidence. macOS evidence
does not close Linux LSan or Windows qualification. No installation, commit or
publication has been performed or claimed. No RAG files, corpus, workers or live
provider services were changed or used.
