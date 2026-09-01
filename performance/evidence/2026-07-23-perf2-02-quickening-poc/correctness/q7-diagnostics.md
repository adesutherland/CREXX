# PERF2-02 Q7 build-private diagnostic evidence

This directory contains build-private diagnostics only. The normal Q7 source
and product artifacts were not changed, and no result here is a formal timing
result. The diagnostic executable was built from the retained Q7 patch plus
`patches/q7-diagnostics-overlay.patch` with `PERF2_Q7_DIAGNOSTICS` defined.

## Event counts

All short runs passed in both `rxvm` and `rxbvm`, and the two VMs reported
identical event counts.

| Run | Reference execution result | COPY state result |
|---|---|---|
| Bounce, one repetition | 5,100 executions: 100 local hits, 5,000 adjacent physical-attribute hits, 0 fallbacks | 14 cold executions, all 14 disabled; 10,889 later executions used the restored canonical handler |
| Richards, one repetition | 0 reference executions | 70 cold executions: 8 specialized and 62 disabled; 32,539 specialized fast hits; 648,355 later disabled/canonical executions; 0 specialized fallbacks and 0 dequickens |
| Existing reference guard fixture | 3 executions: 1 local hit and 2 fallbacks | no COPY execution |
| Diagnostic COPY-transition fixture | no reference execution | 1 cold specialization, 1 specialized fallback/dequickening, then 1 disabled/canonical execution |

The guard fixture's physical-child case is one of its two fallbacks, not an
attribute guard hit: that fixture has a `LOAD` between `MINLINKATTR1` and
`MKREF`, while Q7 recognizes only the exact adjacent pair. The other fallback
is the linked-external case. Both VMs still produce the expected semantic PASS.
Canonical Bounce separately exercises the exact adjacent physical-attribute
guard 5,000 times with zero fallback.

`malformed_modules=0` and `allocation_failures=0` in every run. Diagnostic-mode
fallback counters are zero but unexercised because this is the ordinary
profiling-off build. Invalidation is **N/A**, not zero: Q7 has no epoch,
generation, or invalidation mechanism. Reference dequickening is also **N/A**:
Q7 never rewrites its reference sites back to the canonical handler.

## First-hit diagnostic timing

Single-event timing is not defensible here. `mach_absolute_time()` has a
`125/3` ns timebase on this host; the minimum measured timer pair is zero ticks
because individual intervals can fall below one tick. The smallest useful
bounded sample therefore repeats one-repetition Richards in 12 fresh processes
per VM, giving 96 first-specialization events, 744 first-disable events, and
390,468 steady specialized executions per VM.

These are gross handler-body readings; no timer-overhead subtraction is made:

| VM | First specialize, 96 events | First disable, 744 events | Steady specialized, 390,468 events | Timer-pair mean |
|---|---:|---:|---:|---:|
| `rxvm` | 116.319 ns/event | 2,493.000 ns/event | 7.043 ns/event | 4.019 ns |
| `rxbvm` | 18.663 ns/event | 2,426.075 ns/event | 6.318 ns/event | 3.820 ns |

The first-specialize body includes the general copy, eligibility checks, state
write, and handler patch. The `rxvm` path patches a handler pointer while the
`rxbvm` path patches an opcode, so this sample does not claim a portable common
transition cost. The first-disable body is dominated by workload-specific
general copying of non-plain values and does not isolate state-transition
overhead. Steady readings are too close to the timer-pair floor (measurement
overhead is more than half the gross interval) for a precise per-execution cost
or ratio. They only resolve the bounded architectural question: first-hit work
is observable and finite, while the steady specialized body is below this
instrumentation method's reliable fine-grained resolution. This diagnostic
timing does not alter the Q7 verdict.

Raw per-process lines are in `q7-copy-timing-raw.txt`; the exact aggregation is
in `q7-run-copy-timing-sample.sh` and `q7-copy-timing-summary.txt`.

## Deterministic state and image arithmetic

The probe and disassembly output establish:

- baseline `sizeof(module)`: 216 bytes;
- Q7 `sizeof(module)`: 240 bytes, a 24-byte allocation delta;
- `sizeof(rxvm_perf2_quick_site)`: 56 bytes;
- every process plans 144 modules and allocates site arrays for 21 modules;
- Bounce: 14 benchmark COPY + 2 benchmark MKREF + 925 library COPY + 2
  library MKREF = 943 records, so `943 * 56 + 144 * 24 = 56,264` requested
  bytes;
- Richards: 128 benchmark COPY + 925 library COPY + 2 library MKREF = 1,055
  records, so `1,055 * 56 + 144 * 24 = 62,536` requested bytes.

Normal profiling-off Q7 product image deltas versus accepted Q0 are
`+16,512` file / `+6,320` `__text` bytes for `rxvm`, and `+16,560` file /
`+8,628` `__text` bytes for `rxbvm`. Exact `stat`, `size -m`, disassembly,
structure-probe, SHA-256, and arithmetic output is retained in
`q7-state-image-raw.txt` and is reproduced by `q7-capture-static.sh`. The
diagnostic source overlay is retained as
`../prototypes/q7-diagnostic-overlay.patch`.
