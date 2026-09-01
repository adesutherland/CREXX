# PERF3-13 EF-0 spawn I/O ownership recovery

Date: 2026-08-07
Status: **accepted and locally complete; publication pending Adrian approval**

## Decision

Retain the private single-shot redirect completion implemented in
`interpreter/rxspawn.c`. Spawn I/O threads allocate only in independent libc
capture storage. They receive no RXVM worker, register, reference cell,
attribute tree or raw `value *`. Input is snapshotted before the writer starts;
stdout and stderr publish independent completions; join is the synchronization
boundary; and the receiver worker performs the only conversion into Rexx
string or line-array registers.

This is the first bounded vertical slice of Gates E and F, not the full worker
or channel programme. It creates no public RXAS/RXBIN channel instruction. The
future Rexx-visible value model remains a logical register image with a typed
scalar/binary payload and optional ordered child-register images, never an
internal VM `value`.

## Baseline and defect classification

- Worktree: `/Users/adrian/CLionProjects/CREXX`.
- Branch: direct `develop`; no implementation branch or worktree.
- Base: `9e2e51c20133ece39d12d3b4e113d130b74b2af8`
  (`perf: integrate RXVM value and slab architecture`).
- The base was one commit ahead of `origin/develop` and was preserved without
  reset, rebase or rewrite.
- A fresh 30-way Debug sweep passed 1,965/1,990. The 25 failures were a moving
  manifestation of the same parallel redirect race.
- A serial union/control run passed 7/30 and failed 23/30. Every failure
  reported one to five live RXVM allocations after its functional output.
- The exact 22 handover failures were reproduced independently. Both
  `-nocompile` preparation invocations produced their expected artifact and
  then failed during process teardown; neither had a separate preparation
  defect.
- The direct-VM dynamic-interface test and the direct capture controls passed,
  isolating the defect to paths that crossed the `crexx`/spawn redirect layer.

The audit found that the old redirect thread context copied the parent
`memory_worker` and a live source/destination `value *`. Concurrent stdout and
stderr readers could therefore enter one worker arena and mutate receiver
registers concurrently. Input writers traversed live receiver registers and
attributes from a foreign thread.

## Design selection

1. Per-I/O-thread VM workers were rejected for EF-0. A worker-owned capture
   value would still require serialization into receiver-owned storage, while
   adding worker registration and teardown unnecessary for byte-stream I/O.
2. A reusable receiver mailbox with generation checks was rejected for EF-0.
   It adds queue, reservation, generation and cancellation machinery to a
   single-shot endpoint, yet still needs independently owned payload bytes.
3. The independent private completion was selected. It is the narrowest shape
   that enforces ownership and has an explicit publish/consume lifecycle that
   can later become one provider beneath Gate F's versioned envelope.

## Implemented contract

- Each endpoint owns its handle, byte buffer, diagnostics, transfer mode,
  terminal state and consumed state.
- Input string/array registers are flattened into immutable bytes before pipe
  and thread creation.
- stdout and stderr use separate buffers and may finish in either order.
- Each helper publishes exactly one success or failure terminal state.
- Output readers continue draining after capture allocation failure so a full
  child pipe cannot deadlock cleanup.
- Consume and discard occur only after a proven join; the active receiver
  worker is verified before register materialization.
- Broken pipe, non-zero/signalled child, partial output, allocation/launch/
  thread failure, direct-child cleanup and finalizer paths use the same
  join-before-destruction lifecycle.
- POSIX completion descriptors are close-on-exec. Windows uses private
  non-inheritable handle duplicates and the same logical state contract.
- The ordinary worker allocator and its lock-free local allocation path are
  unchanged.

## Focused fixture

`ts_address_spawn_transfer.crexx` and its native child fixture cover:

- simultaneous stdout and stderr capture, 24,480 bytes per stream;
- multiple 4 KiB read/capture growth operations and content checks;
- scalar and line-array output, including empty output;
- scalar and line-array input;
- simultaneous input, stdout and stderr;
- non-zero exit with retained partial output;
- early child exit/broken pipe;
- twelve repeated spawn/cleanup cycles; and
- nested `ADDRESS CREXX`/`crexx` driver use.

CTest registers default, `rxbvm` and `rxtvm` variants in optimized and no-opt
modes where the concrete VM is supported.

## Validation

| Gate | Result | Evidence |
|---|---:|---|
| Focused Debug clean product | 14/14 | redirect controls and all six new VM/mode cells |
| Exact handover failures plus fixture, Debug | 23/23 | all 22 original failures recovered |
| First focused ordinary Release verdict | 15/15 | accepted by Adrian on 2026-08-07 |
| Exact handover failures plus fixture, Release | 23/23 | all original paths green |
| Ordinary VM neutrality controls, Release | 4/4 | RexxCPS opt/no-opt and register integrity |
| Focused Apple AddressSanitizer | 34/34 | serial, leak detection disabled |
| Full Debug CTest | **1,996/1,996** | required `--parallel 30`, 370.26 seconds |
| Combined ordinary Release closeout | **38/38** | affected set plus controls, 7.70 seconds |
| Portable selected products | pass | `rxbvm`, `rxbvml`, `rxbvme`, `rxvm` |

The first ASan build attempt used leak detection and stopped because this Apple
ASan runtime reports `detect_leaks is not supported on this platform`. The same
full build and focused suite pass through the repository runner with leak
detection disabled. Exact Debug worker teardown counters remain enabled and the
full suite has no live-allocation assertion. No MinGW cross compiler is
installed on the host, so native Windows compile/run validation remains open;
the Windows implementation follows the same private completion contract.

No timing cell was run. There is no retained spawn timing baseline, the host
was not declared reserved, and the change does not touch ordinary VM dispatch
or worker allocation. The bounded ordinary-VM Release controls are green.

## Exact handover failures recovered

- `dynamic_interface_load_driver`
- `ts_address_crexx_noopt`
- `crexx_headerless_simple_smoke`
- `crexx_headerless_rexx_levelc_smoke`
- `crexx_explicit_header_smoke`
- `crexx_args_smoke`
- `crexx_rexxscript_smoke`
- `crexx_status_diagnostics`
- `crexx_spaced_source_smoke`
- `crexx_crxc_noargs_smoke`
- `crexx_tool_output_path_policy`
- `crexx_diagnostic_plumbing`
- `crexx_source_root_compile_only`
- `crexx_rxas_import_compile_only`
- `crexx_nocompile_exec_smoke`
- `crexx_native_nocompile_smoke`
- `crexx_native_explicit_header_smoke`
- `crexx_native_address_crexx_smoke`
- `crexx_native_default_link_strip`
- `crexx_native_linkmap_keep_source`
- `crexx_native_link_keep_inline`
- `crexx_native_nokeep_cleanup`
