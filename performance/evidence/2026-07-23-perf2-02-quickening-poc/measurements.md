# PERF2-02 measurements

Status: complete bounded ordinary profiling-off Release evidence

All decisive cells use the frozen accepted benchmark image and library, run
serially under `caffeinate -i` on attached AC power with low-power mode `0`.
The final Bounce block used two warmups plus 12 recorded balanced rounds per VM;
each Q0/Q3b/Q4 variant occupied every position four times. Raw rows and capture
manifests are retained under `timing/`, `rss/` and `manifests/`.

## Decisive same-guard Bounce comparison

Q3b and Q4 use the same exact A-LOCAL and predecessor-derived A-ATTR route and
the same canonical reference identity/completion/fallback. Q3b decodes the
canonical predecessor at each `MKREF`; Q4 binds a private eager handler during
execution-image preparation.

| VM | Q0 median | Q3b median | Q4 median | Q0/Q3b | Q3b vs Q4 |
| --- | ---: | ---: | ---: | ---: | ---: |
| `rxvm` | 6.673949 s | 1.317381 s | 1.425490 s | 5.066075x | Q3b 7.584% faster |
| `rxbvm` | 7.004614 s | 1.505788 s | 1.502132 s | 4.651793x | Q4 0.243% faster |

Q3b reduces Q0 elapsed time by 80.261%/78.503%. Its `rxbvm` difference from Q4
is smaller than either cell's MAD and is treated as tied. The earlier broad Q3
one-level helper measured 1.495130/1.727522 s; Q4 measured
1.428062/1.512110 s and Q7 1.415905/1.511787 s in that block. Because broad Q3
and Q4 did not use identical route selection, those values are machine-ceiling
orientation only; the balanced Q3b block controls that confound.

Q7 improved Q4 by only 0.859% in `rxvm` and 0.021% in `rxbvm`, inside the
paired spread, while adding state and lifecycle work. No runtime-state form
beats the final direct control.

Q7 first-hit work was observed separately in build-private diagnostics, never
in a product-timing binary. Across 12 fresh one-repetition Richards processes
per VM, 96 first-specialize and 744 first-disable events produced gross means
of 116.319/2,493.000 ns in `rxvm` and 18.663/2,426.075 ns in `rxbvm`.
First-disable includes general complex-value copying; steady samples are close
to the timer-pair floor. No overhead subtraction, cross-mode ratio or verdict
uses these perturbed readings; the balanced elapsed/startup blocks remain
authoritative.

## Richards owner controls

| VM | Q0 median | Q1 compiler capture fold | Q7 core | Disposition |
| --- | ---: | ---: | ---: | --- |
| `rxvm` | 11.643875 s | 8.813834 s (-24.305%) | 11.585007 s (-0.120%) | compiler/inliner owns the removable copy |
| `rxbvm` | 11.778197 s | 8.956049 s (-23.961%) | 11.773399 s (+0.199%) | compiler/inliner owns the removable copy |

Q1 reduces the optimized Richards image from 128 to 117 static general-copy
sites by folding 11 proved direct receiver captures. Q7 cannot remove the
object receiver copy, disables its plain-scalar candidates, and is neutral.
These three-recorded-sample cells are a bounded owner/ceiling experiment, not a
formal production verdict.

## Guard panel

Separate profile diagnostics execute exactly zero `MKREF_REG_REG` instructions
for Sieve, Permute, Storage, Towers and Base64 in both VMs, including loaded
library execution. Thus none can take a Q3b/Q4 fast path.

The final 12-round zero-hit Sieve layout block was:

| VM | Q0 | Q3b | Q4 | Interpretation |
| --- | ---: | ---: | ---: | --- |
| `rxvm` | 1.078849 s | 1.087563 s; 1.089353 s clean append | 1.129166 s | Q3b about +1%; Q4 +4.664% |
| `rxbvm` | 1.438543 s | 1.272019 s | 1.273020 s | Q3b/Q4 tied; binary-layout improvement vs Q0 |

The original five-guard block found Q4 within about 1.1% on Permute, Storage
and Towers. Base64 remained noisy after its ten-run append. Q4's repeated Sieve
mode split is therefore disclosed as unexecuted code/layout sensitivity, not a
semantic fast-path result. Q3b materially reduces the `rxvm` penalty.

## Startup, RSS and deterministic state

Process-inclusive startup uses Bounce work `1`, so it includes launch, load,
link, preparation, first result and ordinary teardown; it does not isolate
those phases. Twelve balanced medians in milliseconds were:

| VM | Q0 | Q3b | Q4 | Q7 |
| --- | ---: | ---: | ---: | ---: |
| `rxvm` | 16.483 | 14.090 | 14.301 | 14.272 |
| `rxbvm` | 16.563 | 14.200 | 14.238 | 14.326 |

The candidate differences are loader/layout-scale; no material startup tax is
resolved. Four-position-balanced canonical Bounce peak-RSS medians were:

| VM | Q0 | Q3b | Q4 | Q7 |
| --- | ---: | ---: | ---: | ---: |
| `rxvm` | 17,784,832 B | 17,899,520 B | 17,833,984 B | 17,940,480 B |
| `rxbvm` | 17,850,368 B | 17,899,520 B | 17,907,712 B | 17,924,096 B |

Every delta is below 0.9%, 156 KiB, and the governed 5%/1 MiB rerun gate. RSS
cannot resolve Q7's roughly 55-61 KiB requested payload, so deterministic
accounting is primary.

| Variant | Persistent site/module bytes | `rxvm` file / `__text` delta | `rxbvm` file / `__text` delta |
| --- | ---: | ---: | ---: |
| Q3 broad | 0 | +64 / +228 B | +48 / +348 B |
| Q3b exact | 0 | +16,512 / +416 B | +48 / +3,964 B |
| Q4 eager | 0 | +16,512 / +8,100 B | +16,560 / +7,916 B |
| Q7 core | 56,264 B Bounce; 62,536 B Richards | +16,512 / +6,320 B | +16,560 / +8,628 B |

Q3b and Q4 reuse the unchanged eight-byte-cell execution image and add no
heap, module or site allocation. Q3b adds 16 transient bytes to each VM's
`run()` stack frame. File deltas reflect Mach-O alignment as well as text. Q7
uses a 56-byte record for all COPY/MKREF sites plus 24 bytes per
loaded module: 943 records/144 modules for Bounce and 1,055/144 for Richards,
with 21 record-array allocations. Totals exclude allocator overhead. Exact
structure probes, disassembly counts, file/`__text` sizes and arithmetic are
retained in `correctness/q7-state-image-raw.txt`.
