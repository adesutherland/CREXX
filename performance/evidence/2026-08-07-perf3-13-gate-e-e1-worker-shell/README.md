# PERF3-13 Gate E E1 single-worker ownership shell

Date: 2026-08-07

Status: **accepted and locally complete; publication pending Adrian approval**

## Decision

Retain the E1 runtime/worker ownership shell. An internal `rxvm_runtime` owns
the allocator memory context and synchronized whole-slab depot. Each
`rxvm_context` embeds one thread-affine `rxvm_worker`, which owns its allocator
arena and explicit lifecycle (`idle`, depth-counted `running`, `draining`,
`stopped`). The current CLI and RXVML API still create one runtime and one
worker; E1 starts no worker threads and changes no VM, language, RXAS/RXBIN or
public channel semantics.

`run()` now verifies worker ownership before entering the arena. Nested calls
on the same owner thread are depth-counted, while foreign-thread execution,
allocator entry and teardown are rejected. Teardown drains the worker,
destroys its arena with the existing live-allocation count, then destroys the
runtime only after its worker count reaches zero. Ordinary worker allocation
remains lock-free.

## Base and product identity

- Worktree: `/Users/adrian/CLionProjects/CREXX`.
- Branch: direct `develop`; no implementation branch or worktree.
- Published pre-E1 base: `19802842e0655b2f2ae011f911e909a2ded7233b`.
- Both control and candidate were ordinary profiling-off Release builds
  (`CMAKE_BUILD_TYPE=Release`, `CREXX_VM_PROFILING=OFF`).
- Control `rxbvm`: 1,001,128 bytes,
  `f74aff6e1110e6a22defa91daa4e7893fb9e96bdab19ce2bc5d36495cc138fa6`.
- Timed E1 `rxbvm`: 1,002,568 bytes,
  `f76fd9a48c64c1cb3bc698007c07ffe7439e00aed98a430055f78bba8bc3d8b0`.
- Final closeout rebuild `rxbvm`: 1,002,568 bytes,
  `c9af8dfa2f8cbcc6a292621b93736c194bb88f782fa47f2d285a32690c014f67`.
- Artifact growth is 1,440 bytes (0.144%); the retained RXBIN and library
  inputs are identical for both products.

## Accepted first Release verdict

Adrian cleared the Mac for timing and accepted the first verdict on
2026-08-07. The pairwise-balanced serial matrix used one warmup and 12 recorded
pairs for each of four workloads. All 104 processes passed: eight warmups and
96 recorded executions. Positive percentages favor E1.

| Workload | Pairs | Paired mean | 95% interval | Median | Favorable |
|---|---:|---:|---:|---:|---:|
| Sieve | 12 | -0.654505% | -1.167787% to -0.141223% | -0.673506% | 2/12 |
| Richards | 12 | +0.924804% | -0.577911% to +2.427518% | +0.420318% | 8/12 |
| Towers | 12 | +1.146596% | +0.203398% to +2.089794% | +0.581948% | 11/12 |
| RexxCPS | 12 | -2.013479% | -3.370835% to -0.656122% | -1.322141% | 0/12 |
| Pooled core four | 48 | -0.149146% | -0.780096% to +0.481804% | -0.313533% | 21/48 |

The pooled result is neutral and no cell reaches the 3% guard. Sieve and
RexxCPS are clear adverse individual observations, Towers is clear favorable,
and Richards is inconclusive. The individual observations remain retained for
later Gate E scale work rather than being hidden by the pooled decision.

## Closeout validation

| Gate | Result | Evidence |
|---|---:|---|
| Focused Debug before freeze | 13/13 | lifecycle, load/run/unload, RXVML/ADDRESS and both concrete VMs |
| First focused Release correctness | 6/6 | reported with the timing verdict |
| Focused Apple AddressSanitizer | 3/3 | allocator, worker lifecycle and nested/reentrant execution; serial |
| Full Debug CTest | **1,997/1,997** | required `--parallel 30`, 319.75 seconds |
| Final Debug focused rebuild | **13/13** | final compiled source after an indentation-only closeout correction |
| Release linked-runtime generation | pass | 808 final build steps |
| Combined Release closeout | **14/14** | worker/allocator, archive link, late load, ADDRESS and both VMs |
| Portable/concrete products | pass | `rxbvm`, `rxtvm`, `rxvml`; dispatch contracts pass for both VMs |

The new cross-thread test has POSIX and Windows thread implementations. It
proves that a foreign OS thread owns neither the VM worker nor its allocator,
cannot begin execution, and cannot perturb the idle state. The owner proves
nested execution, deterministic drain, one terminal stopped state, worker
unregistration and zero live allocations.

## Sanitizer classification and platform limits

Apple's sanitizer runtime rejects `detect_leaks=1` with `detect_leaks is not
supported on this platform`; the passing focused run therefore retains
AddressSanitizer with leak detection disabled. Debug teardown retains the
allocator's exact live-allocation assertion, and the full Debug suite is green.

While building the much broader ASan linked-runtime fixture, `rxas` found a
heap-use-after-free at `rxas_flow_proof.c:4413` in
`rxas_flow_prove_compare_branch_fusion` while generating
`nr15_stem_semantics_rxvm_opt`. The allocation came from
`rxas_flow_queue_batch_edit` and was freed by another batch-edit reallocation.
This is classified as a separate pre-existing assembler defect: E1 changes no
assembler source, the exact assembler diff is empty, focused E1 ASan is green,
and Debug/ordinary Release linked-artifact generation passes. The raw report is
retained for separate correction; it was not masked or fixed inside E1.

The final closeout rebuild changed the executable hash while preserving its
exact size; the timed artifact identity is therefore retained separately from
the final product identity. The only intervening compiled-source edit was an
indentation-only correction to an existing `rxvmload.c` call, followed by the
green final Debug 13/13 and Release 14/14 controls above.

No Windows cross-toolchain is installed on this Mac, so native Windows
compile/run proof remains open. Intel Linux, Linux ARM64 and same-machine
Windows evidence remain later Gate E platform work. E2 does not open until
Adrian accepts E1 publication and explicitly approves the next slice.
