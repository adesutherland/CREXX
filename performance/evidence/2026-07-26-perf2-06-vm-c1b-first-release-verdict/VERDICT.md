# Verdict

## Decision result

**Accepted by Adrian after the mandatory first profiling-off Release gate.**

VM-C1b is mathematically and mechanically sound on the bounded evidence, and
the call-heavy cells show the expected gains. The no-call `rxbvm` Sieve guard
is clearly adverse, so the automatic timing rule initially stopped the slice.
After diagnosis, Adrian explicitly accepted that known trade-off because the
call-heavy gains and faster `rxvm` Sieve justify retaining VM-C1b. The adverse
cell remains recorded as debt; it is not reclassified as neutral.

| Workload | VM | Pairs | Before median | After median | Paired median | Favorable | Mean 95% interval | Result |
| --- | --- | ---: | ---: | ---: | ---: | ---: | --- | --- |
| List 100 | `rxvm` | 12 | 83.7800 ms | 78.3055 ms | -6.900818% | 12/12 | -7.194387% to -6.060546% | clear favorable |
| List 100 | `rxbvm` | 12 | 90.6850 ms | 90.7255 ms | -0.186340% | 8/12 | -0.540938% to +0.225570% | inconclusive |
| Permute 50 | `rxvm` | 12 | 86.1610 ms | 80.4570 ms | -6.632206% | 12/12 | -7.184704% to -6.157516% | clear favorable |
| Permute 50 | `rxbvm` | 12 | 92.9985 ms | 86.5520 ms | -6.571901% | 12/12 | -7.224686% to -6.135421% | clear favorable |
| Sieve 50 | `rxvm` | 12 | 23.1395 ms | 23.0780 ms | -0.122667% | 7/12 | -0.815555% to +0.618918% | inconclusive |
| Sieve 50 | `rxbvm` | 12 | 24.7935 ms | 26.1035 ms | +5.727739% | 0/12 | +4.720473% to +6.016915% | clear adverse |

All 144 recorded executions passed their expected benchmark output.  No sample
was removed, replaced or reclassified.  The clear-adverse guard is decisive;
the governed block was not extended.

## Machine and correctness gates

- Child activation now inherits an interrupt-policy pointer and ownership bit;
  the 1,280-byte table copy is absent.
- `stack_frame` is 168 bytes, down exactly 1,264 bytes from 1,432.
- `run()` changes from 543,340 to 537,680 bytes in `rxvm` and from 536,668 to
  535,872 bytes in `rxbvm`, clearing the no-growth ceiling.
- The final OOM path is one no-inline cold helper and one common dispatch label;
  cleanup never allocates while unwinding saved handlers.
- Focused source signal, runtime signal/instrumentation, reference lifetime,
  late-load and LOADMODULE CTest blocks all passed.  The final narrowed signal
  regression also passed 52/52 after the last code-layout refinement.

## Acceptance and closeout

The follow-up diagnosis is retained under `diagnostics/` and summarized in
`CODE-LAYOUT-DEBT.md` as `PERF2-06-D01`. It excludes COW execution and one-time
root allocation as the adverse Sieve mechanism, while leaving the exact
microarchitectural split between instruction-cache and branch-predictor effects
unresolved on Apple hardware.

The accepted implementation passes a full Debug build and CTest at 1,924/1,924
and a full ordinary profiling-off Release build and CTest at 1,924/1,924. The
implementation is committed before VM-C2 begins. VM-C2 is a separate clean-base
PoC so that it can be discarded without unwinding the accepted VM-C1b slice.
No push is authorized.
