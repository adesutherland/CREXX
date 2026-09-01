# PERF3-13 Gate E E3a first ordinary-Release verdict

Date: 2026-08-10

Branch: `develop`

Control: exact clean `6d12cd921cdbf9cb2098df2a4c8ae6eee75e4a7f`.

Candidate: the same commit plus the frozen, uncommitted E3a implementation,
ownership test, evidence and live performance-control updates.

Status: **accepted; neutral, guard-clean verdict and Mac closeout complete**.

## Frozen E3a implementation

E3a replaces RXVM's process-global live provider instance with a synchronized
process catalogue of provider descriptors and a provider-instance set owned by
each `rxvm_context`. Decimal construction occurs once after worker-affinity
entry and before execution; frames borrow only their VM context's instance.
Instruction execution performs no catalogue lookup and takes no catalogue
lock.

Dynamic registration is an explicit transaction. It stages every descriptor,
publishes atomically, rolls back failure, treats exact duplicates as
idempotent, and retains a generation-counted dynamic-library handle until the
last descriptor/context reference is released. The ambient
`current_loading_handle` path is gone. The legacy public descriptor API remains
available through a compatibility adapter; product VM execution does not use
that shared adapter.

The ownership test first reproduced the pre-E3a shared decimal collision, then
proved the final form with two simultaneously live OS-thread-owned VM contexts:
distinct provider/private contexts, independent numeric and signal state,
catalogue clear while both instances remain live, reverse teardown, retained
dynamic-library reachability, and exact zero live instances at shutdown.

RXPA descriptor replay and native-module handle/state ownership are deliberately
unchanged. They remain the separate E3b slice.

## First ordinary-Release verdict

The exact control and candidate used ordinary profiling-off Release builds with
the unchanged `profile-20` panel. Both VM binaries consumed the same clean
control `library.rxbin` and four workload images, so the only paired executable
difference was the VM candidate. One warmup and 12 pairwise-balanced recorded
rounds ran for each control/candidate cell. Positive percentages favour E3a.

All 208/208 processes passed: 16 warmups and 192 recorded executions. Every
paired interval crosses zero and no workload trips the 3% adverse guard.

| Workload | VM | Pairs | Paired median | Paired mean | Mean 95% interval | Verdict |
| --- | --- | ---: | ---: | ---: | ---: | --- |
| Sieve | `rxbvm` product | 12 | -0.069720% | +0.447215% | -0.419646% to +1.314076% | noisy/inconclusive; guard clear |
| Richards | `rxbvm` product | 12 | +0.217535% | +0.189217% | -1.532651% to +1.911085% | noisy/inconclusive; guard clear |
| Towers | `rxbvm` product | 12 | +0.494118% | +0.432081% | -0.838570% to +1.702733% | noisy/inconclusive; guard clear |
| RexxCPS | `rxbvm` product | 12 | +0.962402% | +0.455906% | -0.795316% to +1.707128% | noisy/inconclusive; guard clear |
| Sieve | `rxtvm` guard | 12 | +0.250532% | +0.393973% | -0.320224% to +1.108170% | noisy/inconclusive; guard clear |
| Richards | `rxtvm` guard | 12 | +0.428579% | +0.965661% | -2.082167% to +4.013489% | noisy/inconclusive; guard clear |
| Towers | `rxtvm` guard | 12 | +0.464717% | +0.486173% | -0.193434% to +1.165781% | noisy/inconclusive; guard clear |
| RexxCPS | `rxtvm` guard | 12 | +0.487191% | +0.681973% | -0.395267% to +1.759214% | noisy/inconclusive; guard clear |

The product's paired medians span only -0.069720% to +0.962402%. This is no
evidence of a material single-worker regression. Adrian accepted E3a on
2026-08-10 and directed the programme to move to E3b.

## VM shape

| VM | Control file | Candidate file | Delta | Control `__text` | Candidate `__text` | Delta | Hot owner delta |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `rxbvm` | 1,112,312 | 1,113,112 | +800 | 893,596 | 895,980 | +2,384 | -72 |
| `rxtvm` | 1,112,440 | 1,113,240 | +800 | 899,048 | 901,504 | +2,456 | 0 |

The Mach-O `__TEXT` segment remains 966,656 bytes in all four binaries. The
selected panel remains 120/589 definitions inline (118 ranked public handlers
plus two private fused handlers). The E3a setup/teardown code is outside the
instruction loop; `rxvm_run_owned_core` is unchanged for `rxtvm` and 72 bytes
smaller for `rxbvm`.

## Accepted closeout

The disposable pre-fix reproducer mode was removed after acceptance; its
observed output remains retained in `VALIDATION.md`. The rebuilt focused Debug
panel passes 15/15 and Release ownership passes 1/1.

The first complete Debug build exposed one integration defect: auxiliary
targets include `rxvmintp.h` through the interpreter root but did not add the
RXVM-plugin subdirectory, so the new bare header include was unresolved.
Changing it to the interpreter-root-relative
`rxvmplugin/rxvmplugin_framework.h` repaired the build boundary without changing
runtime semantics. The complete Debug build then passed and full CTest is
2,007/2,007 in 291.77 seconds with `--parallel 30`.

The ordinary Release VMs rebuilt after that correction with the exact accepted
verdict hashes: `rxtvm`
`23a486b17ceaa8685ecb3f3606ddf7748506c7e09c549e2e3ae48d9d130e06fb`
and `rxbvm`
`9269ad65cc747a0d4770e0ff79bb2ea87f96769b704585ec616eb899df8cc2ac`.
The accepted timing evidence therefore remains authoritative and was not
rerun. The approved shortest closeout did not automatically add sanitizer,
cross-platform or install/package work.

## Host and scope boundary

Host: Mac17,3, 10 logical CPUs, Darwin 25.5.0 arm64. Toolchain: Apple Clang
21.0.0, CMake 4.3.2 and Ninja 1.13.2. Both builds used
`CMAKE_BUILD_TYPE=Release`, `CREXX_VM_PROFILING=OFF` and
`CREXX_VM_HANDLER_PANEL=profile-20`.

The formal run started at 12:42:25 UTC and completed at 12:48:08 UTC on AC
power with low-power mode disabled. No thermal, performance or CPU-power
warning was recorded before or after the capture. No build or second benchmark
runner overlapped it.

E3a is accepted and its proportional local closeout is complete. No commit or
push was inferred. E3b has a separately reviewable design plan; production E3b
edits, a parallel external-plugin capability ABI, E4, public workers/channels
and Gate F remain outside this evidence.
