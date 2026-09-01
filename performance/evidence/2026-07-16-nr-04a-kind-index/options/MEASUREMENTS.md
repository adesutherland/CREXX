# NR-04A option-discriminating measurements

Status: bounded design input; no production candidate selected

All product timings are serial samples from exact ordinary profiling-off
Release VMs and exact image hashes retained by each bundle. Negative deltas mean
the candidate completed faster. Compare each row only with its paired baseline;
cross-bundle absolute medians show material run-to-run noise.

## Frozen VMs

| VM | SHA-256 | Meaning |
| --- | --- | --- |
| starting baseline | `2e39a5f89e4a8b639f0a269073ebaa15856c92e5769ef42bdacbd5325e9ddc15` | starting `7a599906b` ordinary Release VM |
| candidate A | `c4a7be443b8655dc9f6db0565c814544527ee453a4e787cc17e61637e79c6804` | eager all-kind index plus cursor-based `METALOADDATA` inside `run()` |
| candidate A-prime | `4c5be2d09eb4c3eecbf9dd1e2181696db6fc9df56bb5ae83b2d781dbd0877319` | A with canonical raw `METALOADDATA` traversal restored |
| candidate C | `46cccfe4f453290d8ff22c0d4b70cf997326bb919b37c928ca852e5ddd56706c` | full validation scan, retain only FUNC/CLASS/INTERFACE/IMPLEMENTS/MEMBER |

## Results

| Comparator | Exact cell | Baseline median | Candidate median | Delta |
| --- | --- | ---: | ---: | ---: |
| A | focused interface, retained | 204.432 ms | 106.384 ms | -47.961% |
| A | focused interface, stripped | 170.090 ms | 107.155 ms | -37.001% |
| A | stripped List repeat | 263.453 ms | 273.951 ms | +3.985% |
| A | stripped JSON repeat | 352.745 ms | 383.215 ms | +8.638% |
| A-prime | stripped JSON repeat | 343.494 ms | 335.704 ms | -2.268% |
| C | focused interface, retained | 221.466 ms | 98.789 ms | -55.393% |
| C | focused interface, stripped | 202.647 ms | 108.491 ms | -46.463% |
| C | tiny lifecycle, retained | 3.589 ms | 3.929 ms | +9.473% (+0.340 ms) |
| C | tiny lifecycle, stripped | 3.634 ms | 3.710 ms | +2.091% (+0.076 ms) |
| C | stripped JSON repeat | 403.927 ms | 387.765 ms | -4.001% |

Candidate A lifecycle repeats were unstable in absolute-small cells: retained
was 0.000 ms and +0.085 ms; stripped was +0.190 ms and +1.371 ms. They establish
that startup cost must be measured, not a stable percentage estimate.

## Interpretation

- A reduced normalized contract-kind visits from about 2,733.7 to 3.05 per
  query and implements visits from about 3,642.3 to 17.99 per query, eliminating
  source/TRACE records from ordinary semantic lookups.
- A's unrelated JSON regression was a code-layout confound. Moving generic
  `METALOADDATA` traversal into the already-large flattened `run()` function
  grew it by 3,952 bytes. A-prime was only 48 bytes larger than baseline and the
  paired JSON result changed direction.
- C retains 434 semantic entries (6,944 bytes) for optimized RexxCPS instead of
  A's 6,634 entries (106,144 bytes) in the retained image. Retained/stripped C
  are identical in semantic-entry count.
- C is still a same-kind linear scan. In the focused retained interface profile,
  100,001 negative class/interface relationship queries examined 2,400,024
  `META_IMPLEMENTS` entries. This is why a direct type/assignability map remains
  a required ceiling measurement.
- The generic index measurements justify separating diagnostic metadata and
  validate the semantic scope, but they do not select the final polymorphic
  dispatch architecture.

## Retained bundles

- `focused-runtime-lookup/`: candidate A versus baseline.
- `stripped-regression-check/`: candidate A List/JSON repeats.
- `lifecycle/` and `lifecycle-b/`: candidate A startup repeats.
- `options/aprime-json/`: A-prime code-layout discriminator.
- `options/candidate-c/`: narrow generic semantic-index discriminator.

The next design choices and proposed measurements are in
`performance/NR-04A-RUNTIME-TYPE-DISPATCH-DESIGN.md`.
