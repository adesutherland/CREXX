# PERF3-11 K04e Mac scorecard validation

- Clean detached product: `c4470635048e497417c4db92c03ecbcd79eaa750`,
  `Release`, `-O3 -DNDEBUG`, `CREXX_VM_PROFILING=OFF`.
- Focused optimized workload and matrix self-test panel: `10/10` pass. The
  initial `9/10` attempt merely lacked the narrow no-opt self-test build
  prerequisite; after building that generated artifact, the unchanged panel
  passed `10/10` in 1.51 seconds.
- Initial formal matrix: 29 cells, 58 warmups, 290 recorded observations and
  `348/348` executions pass with zero stderr.
- Initial noise rule selects exactly `permute-netrexx`, `base64-rxvm` and
  `base64-rxbvm`; the append manifest contains exactly those three cells.
- Governed append: 30 recorded observations and `30/30` executions pass with
  zero stderr. Samples removed: zero. Second append: forbidden and not run.
- The merged summary contains 20 observations for each appended cell and ten
  for every other cell. `permute-netrexx`, `base64-rxvm` and `base64-rxbvm`
  remain noise-labelled after the append.
- All four common-five geometric means and 20 component ratios are generated
  from the merged sample set and independently recompute exactly at six decimal
  places. Common membership is exactly Sieve, Permute, Bounce, Richards and
  Base64 (`N=5`).
- Artifact inventory: 79 SHA-256-bound rows. All 61 benchmark-source,
  generated-product and comparator runtime/compiler identities match the
  retained PERF3-06 scorecard.
- Static summary inventories the exact seven optimized RXBINs plus the shared
  library. Relative to PERF3-06 there is no static instruction or image-size
  increase in any workload; K04e's independently proved change is RexxCPS
  `1,222 -> 1,221`.
- `rxvm`, `rxbvm` and `library.rxbin` are SHA-256 bound. The executable sizes
  are unchanged from PERF3-06 and the library is 464 bytes smaller, so the
  deterministic artifact-size guard is clear.
- Apple M5 host remained on AC with low-power mode off and no thermal or
  performance warning. Capture was serial-rotated.
- The repository manifests hash to
  `391e80be6ff2fcd97924ba41ecda1256f15618ac9f3f4c6772000819207a23db`
  and `d5b81f8f9778e1595cbb3753f21228e9c4b2c31b78a3cd7aee2d0da5f610595b`.

The recursive checksum file was generated after all evidence files were
frozen and independently verified.
