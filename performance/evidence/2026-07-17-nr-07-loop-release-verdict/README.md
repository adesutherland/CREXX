# NR-07 specialist-loop Release verdict

Status: **rejected; specialist-loop production changes removed**.

## Scope and provenance

- Source branch/HEAD: `develop` at `5e5e3b397`, with the uncommitted NR-07
  direct-condition lowering plus the approved specialist-loop extension.
- Product build: ordinary full incremental Release build with
  `CREXX_VM_PROFILING=OFF`; it completed successfully.
- Focused Release correctness: 35/35 loop, condition, parser, structural and
  linked-runtime tests passed, including optimized/no-opt output and both VMs.
- Workload: canonical optimized `tests/benchmarks/rexxcps_levelb.crexx`, using
  its default 100 x 100 contract with no argv.
- Images: the retained original image, retained direct-condition-only image,
  and current full loop candidate were all linked against the same preserved
  `library.rxbin` (`eb2cade3004704eb0425ff3769cc2a06314a3566b944208b89831d2448cc1c7c`).
- Runtime: the same current Release `rxvm` and `rxbvm` binaries executed every
  image. Each image/VM received one warmup and five recorded samples.
- Ordering: variants rotated within each round; `rxbvm` reversed the `rxvm`
  order. This gives 30 interleaved recorded runs plus six warmups.
- Correctness: all 36 executions exited zero and contained the canonical
  provenance and `PASS: RexxCPS 2.2c cREXX port` marker.

## What survived assembly

Relative to the direct-condition-only compiler, raw compiler RXAS removes 26
additional static instruction sites. Normal RXAS optimization already folds
all 12 `INC; BR` pairs into `BCTP`, however, so direct rxc emission of `BCTP`
does not change the assembled product.

The assembled RexxCPS delta is therefore only 14 instruction sites:

- 12 `IGT; BRT` pairs become 12 `IGTBR` instructions;
- one `FGT; BRT` pair becomes one `FGTBR`; and
- one positive-count `BCF` plus back-edge `BR` becomes one `BCT`.

The isolated no-opt opcode ceilings were positive: typed direct branches were
22.7% faster in `rxvm` and 25.5% in `rxbvm`; the three-register counted-loop
form was 18.5% and 15.4% faster respectively. The ordinary product result
below shows that those local savings do not create an end-to-end RexxCPS win.

## Release timing result

Higher benchmark-native CPS is better; lower process elapsed time is better.

| VM | Variant | Median CPS | CPS range | Median elapsed | Elapsed range |
|---|---|---:|---:|---:|---:|
| `rxvm` | original | 1,184,117 | 1,179,818-1,193,745 | 8,452.973 ms | 8,384.897-8,484.083 ms |
| `rxvm` | direct only | 1,176,964 | 1,151,703-1,186,005 | 8,507.322 ms | 8,439.615-8,690.637 ms |
| `rxvm` | full loops | 1,175,795 | 1,170,768-1,189,279 | 8,513.463 ms | 8,416.426-8,549.366 ms |
| `rxbvm` | original | 1,180,836 | 1,153,600-1,191,639 | 8,476.645 ms | 8,400.156-8,679.133 ms |
| `rxbvm` | direct only | 1,174,014 | 1,162,284-1,180,837 | 8,528.025 ms | 8,476.303-8,611.912 ms |
| `rxbvm` | full loops | 1,170,578 | 1,164,211-1,183,146 | 8,551.077 ms | 8,460.290-8,597.823 ms |

Median-of-cell changes:

| VM | Comparison | CPS change | Elapsed change |
|---|---|---:|---:|
| `rxvm` | direct only vs original | -0.604% | +0.643% |
| `rxbvm` | direct only vs original | -0.578% | +0.606% |
| `rxvm` | full loops vs direct only | -0.099% | +0.072% |
| `rxbvm` | full loops vs direct only | -0.293% | +0.270% |
| `rxvm` | full loops vs original | -0.703% | +0.716% |
| `rxbvm` | full loops vs original | -0.869% | +0.878% |

Round-paired medians agree that the loop extension is neutral-to-adverse:
full versus direct-only is -0.025% CPS/+0.025% elapsed in `rxvm` and -0.294%
CPS/+0.270% elapsed in `rxbvm`.

The earlier blocked-order result of -11.139%/-5.573% for direct-only versus
original is not reproduced. The exact same retained images are close under
interleaving, so the earlier material result was dominated by time-window
drift or host load. The new result still provides no positive Release evidence
for either the direct-only or specialist-loop candidate.

Adrian rejected the extension because the ordinary product showed no practical
performance improvement and therefore did not justify the additional compiler
complexity. The specialist-loop production and dedicated test changes were
removed; the evidence is retained.

The later NR-06 causal audit led Adrian to retain that separate bounded
register-placement optimization because it removes verified swaps without
adding runtime instructions. It did not change this NR-07 decision: the
remaining direct-condition production lowering and its dedicated tests were
also removed because neither NR-07 path demonstrated a practical gain. Its
exact rejected patch is retained in the first-verdict evidence directory.

No broad CTest, sanitizer or portfolio expansion was performed before the
decision. Raw manifests, stdout/stderr, chronological
samples, paired deltas, exact RXAS/RXBIN/disassembly images and build/test logs
remain in this directory.
