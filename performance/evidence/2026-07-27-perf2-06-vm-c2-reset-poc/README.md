# PERF2-06 VM-C2 reset countermeasure PoC

Verdict: reject both R1 and R2. Neither fixed-core reset specialization is a
production candidate.

R0 is the exact accepted product at
`ab9eef54576388121cdb02e285bd99981e68d8b3`. R1 replaces the recycled-frame
local/global reset loops with one contiguous pointer copy. R2 performs that
copy only when a canonical, fail-closed procedure-static proof finds a
possible mapping change. All products are ordinary profiling-off,
diagnostics-off Apple-Clang Release builds. All timing cells consume the same
R0-built optimized RXBIN files and `library.rxbin`.

## Correctness and machine identity

R1 and R2 each pass:

- 78 frame, reference, call and argument tests;
- 57 signal, TRACE, breakpoint and instrumented-VM tests; and
- 8 external re-entry, dynamic/late-load and lifecycle tests.

That is 143/143 focused tests per variant in diagnostics-on, profiling-off
Debug products. The logs are in `correctness/`.

The retained count profiles prove exact R0/R1/R2 instruction, call, branch and
value-operation rows for optimized Permute 50, List 100 and Sieve 50 in both
VMs. See `profiles/operation-identity.csv`.

## Reset work

The exact-work diagnostic rows in `machine-stats/` are identical between
`rxvm` and `rxbvm`.

| Workload | Reused activations | R1 reset slots | R2 skipped activations | R2 skipped slots |
| --- | ---: | ---: | ---: | ---: |
| Permute 50 | 432,944 | 9,957,712 | 0 | 0 |
| List 100 | 572,457 | 13,312,071 | 3,099 | 9,297 |
| Base64 500 | 499 | 19,461 | 499 | 19,461 |
| Sieve 50 | 0 | 0 | 0 | 0 |

For List, R2 skips only 0.541% of reused activations and 0.070% of eligible
slots. The classifier proves 161-164 loaded procedures clean, selects the
fallback for 506-514 mapping-capable procedures and fails closed for 36
unknown procedures. Explicit cross-procedure targets, fallthrough and
unresolved indirect branches cannot be proved clean.

R1 copies 79,661,696 bytes for Permute, 106,496,568 for List and 155,688 for
Base64. R2 copies 79,661,696, 106,422,192 and zero bytes respectively; both do
zero reset-byte work for Sieve. The byte totals and VM parity are generated
directly from the retained diagnostic rows in `machine-stats/reset-summary.csv`.

## Profiling-off Release verdict

The formal capture retains 34 balanced/interleaved pairs: the required 12,
the prescribed 10-sample noise extension, and one final 12-pair extension for
crossing intervals/regression guards. No sample was removed. Base64 remains
materially noisy at the 34-pair cap; its large opposite-direction VM swings
are retained as layout/startup-sensitivity evidence, not promoted as stable
wins.

The absolute-cell gate also flags the Sieve R0 `rxvm` span (24.241%) and R1
`rxbvm` span (10.009%). The latter R1/R0 paired `rxbvm` interval nevertheless
remains wholly adverse; the noise caveat travels with the conclusion.

Positive percentages are slower than R0.

| Workload | VM | R1 mean % [95% interval] | R2 mean % [95% interval] |
| --- | --- | ---: | ---: |
| Permute | `rxvm` | +0.043 [-0.338, +0.424] | +0.303 [-0.032, +0.638] |
| Permute | `rxbvm` | -0.502 [-0.844, -0.159] | +0.849 [+0.430, +1.268] |
| List | `rxvm` | +0.953 [+0.539, +1.367] | +1.266 [+0.902, +1.629] |
| List | `rxbvm` | +1.098 [+0.799, +1.396] | +0.834 [+0.558, +1.111] |
| Base64 | `rxvm` | +18.581 [+14.783, +22.378] | +0.167 [-2.731, +3.064] |
| Base64 | `rxbvm` | -3.484 [-7.180, +0.212] | -13.684 [-17.087, -10.280] |
| Sieve | `rxvm` | -1.445 [-2.619, -0.270] | +0.924 [-0.361, +2.209] |
| Sieve | `rxbvm` | +1.801 [+1.095, +2.507] | +2.285 [+1.621, +2.949] |

The decisive control is Sieve. It executes one fresh frame and zero reused
frames, so neither variant does changed reset work. R1 has no classifier, so
its clear `rxbvm` regression isolates native code layout/register allocation.
R2 additionally scans 55,104 instructions at startup; its regression can be
classifier startup cost, layout, or both, but not reset work. Both variants
also fail List in both VMs. R2's static guard has too little hot coverage to
repay its classifier and layout effects. R1 therefore strengthens
`PERF2-06-D01`: revisit layout-sensitive VM decisions on supported Intel
x86-64 with both GCC and Clang before final architecture selection.

## Product size

| Variant | VM | File bytes | Mach-O `__text` | `run()` bytes |
| --- | --- | ---: | ---: | ---: |
| R0 | `rxvm` | 982,392 | 806,136 | 537,680 |
| R1 | `rxvm` | 982,392 | 802,820 | 534,364 |
| R2 | `rxvm` | 998,936 | 804,900 | 534,516 |
| R0 | `rxbvm` | 982,552 | 805,004 | 535,872 |
| R1 | `rxbvm` | 982,552 | 801,652 | 532,520 |
| R2 | `rxbvm` | 999,144 | 803,736 | 532,676 |

R1 removes about 3.3 KiB from both `run()` and total text. R2 retains about a
3.2 KiB `run()` reduction, but the loader classifier reduces the total text
gain to about 1.2 KiB and grows the files by about 16.5 KiB. The elapsed result
shows that smaller aggregate text alone is not a layout win.

## Post-verdict value-storage direction

Current same-procedure frame recycling preserves the internal capacity of a
logical register across later calls in the same `run()`. A strict universal
LIFO value stack would lose this affinity when another procedure reused the
same physical cells.

The strongest future design is a linear segmented control stack pointing to a
separate, non-moving, procedure-affine value-slab arena. Each activation slab
contains procedure locals plus `a0`. The control record, or a compact sidecar,
must still own the mutable register mapping: direct local pointers, module
global pointers, embedded `CALL1...CALL4` argument pointers, a separate vector
for higher arity, and any `LOAD`/`SWAP`/`LINK*`/`UNLINK*` overlay needed for
unwind and restoration. Separating values does not eliminate that semantic
state.

Return performs the existing signal cleanup and the existing
`has_reference_lifetimes`-gated scan of procedure locals plus `a0`, then
returns the unchanged slab to the same procedure. This avoids an unconditional
value scan but does not pretend the required reference scan is free. Native
payloads retain current lifetime semantics: they stay in the slab until
overwritten or until run teardown clears every initialized slab and invokes
the relevant finalizers.

That retains the useful "register 3 gets its old capacity back" behavior
without moving register innards, transferring each payload on return or adding
a hot mapping ledger. Slabs must remain context/thread local. Reference
identities, native payload resources and live object links are not reusable
capacity. If slab high-water retention later proves excessive, test capacity
capsules only for ordinary strings and non-native binaries before considering
harder payloads.

This design is recorded for later architecture selection only. Adrian closed
the activity as a rejection and authorized a results-only closeout. No fourth
variant, production integration, broad QA, implementation commit or push
follows. The control/mapping split remains PERF2-06 architecture work; any
later payload-capacity capsule or representation policy belongs to PERF2-07.
