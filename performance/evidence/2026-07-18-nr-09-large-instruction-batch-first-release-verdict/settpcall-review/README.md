# SETTPCALL focused regression review

## Verdict

Retain `SETTPCALL_REG_FUNC_REG_REG_INT` and its existing
`RXVM_MAPPED_CALL_BODY` implementation. The instruction is a small ordinary
Release win when runtime-image alignment is averaged, and the earlier
`+15.026%`/`+6.676%` profile signal was not a like-for-like comparison.

Across all 16 offsets of this machine's 128-byte cache line, 64 balanced
5,000,000-call samples per VM give:

| VM | Expanded mean | Fused mean | Fused delta | Delta |
| --- | ---: | ---: | ---: | ---: |
| `rxvm` | 28.240575 ns/call | 28.111256 ns/call | -0.129319 ns/call | -0.458% |
| `rxbvm` | 27.757409 ns/call | 27.338984 ns/call | -0.418425 ns/call | -1.507% |

Negative is faster. Individual `rxvm` offsets range from -1.247700 to
+1.310750 ns/call. A single code layout can therefore show a small loss even
though the position-averaged instruction wins.

## Decode and image review

- `bin_code` is an eight-byte cell. `SETTPCALL` is one instruction cell plus
  five operand cells (48 bytes); `SETTP` plus `CALL` is two instruction cells
  plus five operand cells (56 bytes).
- The retained disassemblies prove code-segment sizes of `0x21` cells fused
  versus `0x22` expanded. Equal standalone RXBIN file sizes are container
  padding, not equal code sizes.
- `rxbvm` executes the canonical cells through a switch. `rxvm` copies those
  cells once when loading and replaces each instruction cell with its direct
  handler pointer. Neither runtime walks the operand-format string or runs an
  arbitrary-operand decode loop per execution.
- `VM_ADVANCE(5)` and `REG_OP(1..4)`/`INT_OP(5)` compile to fixed `pc+n`
  accesses. The wider opcode has no generic runtime decode penalty.
- Removing one cell shifts every following runtime-image cell. On this
  128-byte-line machine, matching call-header offsets gives `rxvm` a
  +0.278050 ns/call loss, while matching the post-call return-target offsets
  gives a -0.561850 ns/call saving. The full 16-offset sweep is therefore the
  appropriate decision boundary.

## Semantic and handler review

The fused handler preserves the expanded order and contracts:

1. `RXFLAGS_PUBLIC_WRITE` updates only the public status bands of operand 4.
2. Procedure resolution, unresolved-function signalling and profile metadata
   match `CALL_REG_FUNC_REG`.
3. Native calls use count register 3, the contiguous window beginning at
   `REG_IDX(3)+1`, and result register 1.
4. Bytecode calls pass the fused post-instruction `next_pc`, preserve
   `caller_arg_base`, activate the same frame and map the same argument window.
5. Signal/interrupt and return transitions use the existing mapped-call body.

The exact 20,000,000-call instrumented cells also show the expected fused
handler saving: 73 ns versus 74+11 ns rounded averages in `rxvm`, and 71 ns
versus 71+12 ns in `rxbvm`. Those timings include profiler overhead and are
supporting evidence, not the Release verdict.

A trial that copied the established `CALL_REG_FUNC_REG` source body into only
`SETTPCALL` was rejected and reverted. It worsened the matched paired cell to
+0.394 ns/call in `rxvm` and +1.468 ns/call in `rxbvm`; repeated operand
evaluation is slower than the current cached mapped-call body.

No VM production edit survives this review. Adding a dummy sixth operand would
only choose one favourable alignment, discard the smaller image, and move the
same alignment sensitivity elsewhere.

## Reporting fix

`report_nr09_macro_timings.zsh` now marks call-bearing macros
`exact-cell-required`. A global `CALL_REG_FUNC_REG` average mixes callees,
arities, frame paths and call sites and can no longer classify these forms as
`possible-slowdown` or `indicated-saving`. The numeric global estimate remains
visible for diagnosis, but the decision ledger records this matched Release
cell as the authority for `SETTPCALL`.

## Reproduction

The alignment sweep is rerunnable from the repository root:

```sh
performance/evidence/2026-07-18-nr-09-large-instruction-batch-first-release-verdict/settpcall-review/run_alignment_sweep.zsh
```

Inputs, generated images, raw outputs, all 128 recorded samples and the
per-offset/overall summary are retained beside this report. The run used
commit `32bf7e76f3fe4b776ee30eb60e85af7ca1e22b46`, ordinary profiling-off Release
VMs, Darwin arm64, and a reported 128-byte cache line.
